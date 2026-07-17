#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 400
#define H 300

int values[5] = { 20, 60, 45, 80, 30 };
int target[5] = { 20, 60, 45, 80, 30 };
const char* labels[5] = { "Q1", "Q2", "Q3", "Q4", "Q5" };
COLORREF colors[5] = {
    RGB(20, 184, 166), // Teal
    RGB(245, 158, 11), // Orange
    RGB(236, 72, 153), // Pink
    RGB(139, 92, 246), // Purple
    RGB(59, 130, 246)  // Blue
};

int randSeed = 42;
int MyRand() {
    randSeed = randSeed * 1103515245 + 12345;
    return (unsigned int)(randSeed / 65536) % 32768;
}

HWND hBtnRandomize;
HWND hBtnToggle;
int isLineChart = 0;
int hoveredIndex = -1;
int mouseX = 0, mouseY = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            randSeed = GetTickCount();
            hBtnRandomize = CreateWindowEx(0, "BUTTON", "Randomize",
                WS_CHILD | WS_VISIBLE,
                W / 2 - 110, H - 70, 100, 24, hwnd, (HMENU)1, NULL, NULL);
            hBtnToggle = CreateWindowEx(0, "BUTTON", "Toggle Type",
                WS_CHILD | WS_VISIBLE,
                W / 2 + 10, H - 70, 100, 24, hwnd, (HMENU)2, NULL, NULL);
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            SendMessage(hBtnRandomize, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnToggle, WM_SETFONT, (WPARAM)hFont, TRUE);
            SetTimer(hwnd, 1, 16, NULL); // Animation timer
            break;
        }
        case WM_MOUSEMOVE: {
            mouseX = LOWORD(lParam);
            mouseY = HIWORD(lParam);
            
            int chartX = 50;
            int chartY = 30;
            int chartW = W - 100;
            int chartH = H - 120;
            int barW = 30;
            int spacing = (chartW - (5 * barW)) / 6;
            
            int newHover = -1;
            for (int i = 0; i < 5; i++) {
                int bh = (values[i] * chartH) / 100;
                int bx, by, bw, h;
                if (!isLineChart) {
                    bx = chartX + spacing + i * (barW + spacing);
                    by = chartY + chartH - bh;
                    bw = barW;
                    h = bh;
                } else {
                    bx = chartX + spacing + i * (barW + spacing) + barW / 2 - 10;
                    by = chartY + chartH - bh - 10;
                    bw = 20;
                    h = 20;
                }
                if (mouseX >= bx && mouseX <= bx + bw && mouseY >= by && mouseY <= by + h) {
                    newHover = i;
                    break;
                }
            }
            if (newHover != hoveredIndex || newHover != -1) {
                hoveredIndex = newHover;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
            break;
        }
        case WM_MOUSELEAVE: {
            if (hoveredIndex != -1) {
                hoveredIndex = -1;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_TIMER: {
            int changed = 0;
            for (int i = 0; i < 5; i++) {
                if (values[i] < target[i]) {
                    values[i] += 2;
                    if (values[i] > target[i]) values[i] = target[i];
                    changed = 1;
                } else if (values[i] > target[i]) {
                    values[i] -= 2;
                    if (values[i] < target[i]) values[i] = target[i];
                    changed = 1;
                }
            }
            if (changed) InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                for (int i = 0; i < 5; i++) {
                    target[i] = 10 + (MyRand() % 90); // 10 to 100
                }
            } else if (LOWORD(wParam) == 2) {
                isLineChart = !isLineChart;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Double buffer
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, W, H);
            HBITMAP oldBM = SelectObject(memDC, memBM);
            
            HBRUSH bg = CreateSolidBrush(RGB(9, 9, 11));
            RECT full = {0, 0, W, H};
            FillRect(memDC, &full, bg);
            DeleteObject(bg);
            
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HGDIOBJ oldFont = SelectObject(memDC, hFont);
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(161, 161, 170));
            
            int chartX = 50;
            int chartY = 30;
            int chartW = W - 100;
            int chartH = H - 120;
            
            // Draw axes
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(100, 116, 139));
            HGDIOBJ oldPen = SelectObject(memDC, hPen);
            MoveToEx(memDC, chartX, chartY, NULL);
            LineTo(memDC, chartX, chartY + chartH);
            LineTo(memDC, chartX + chartW, chartY + chartH);
            SelectObject(memDC, oldPen);
            DeleteObject(hPen);
            
            int barW = 30;
            int spacing = (chartW - (5 * barW)) / 6;
            
            if (!isLineChart) {
                for (int i = 0; i < 5; i++) {
                    int bw = barW;
                    int bh = (values[i] * chartH) / 100;
                    int bx = chartX + spacing + i * (barW + spacing);
                    int by = chartY + chartH - bh;
                    
                    RECT br = { bx, by, bx + bw, chartY + chartH - 1 };
                    HBRUSH brBrush = CreateSolidBrush(colors[i]);
                    FillRect(memDC, &br, brBrush);
                    DeleteObject(brBrush);
                    
                    // Draw label
                    RECT lr = { bx - 10, chartY + chartH + 5, bx + bw + 10, chartY + chartH + 25 };
                    DrawTextA(memDC, labels[i], -1, &lr, DT_CENTER | DT_SINGLELINE);
                }
            } else {
                int prevX = 0, prevY = 0;
                for (int i = 0; i < 5; i++) {
                    int bw = barW;
                    int bh = (values[i] * chartH) / 100;
                    int bx = chartX + spacing + i * (barW + spacing) + bw / 2;
                    int by = chartY + chartH - bh;
                    
                    if (i > 0) {
                        HPEN linePen = CreatePen(PS_SOLID, 2, RGB(20, 184, 166)); // Teal line
                        HGDIOBJ oldLinePen = SelectObject(memDC, linePen);
                        MoveToEx(memDC, prevX, prevY, NULL);
                        LineTo(memDC, bx, by);
                        SelectObject(memDC, oldLinePen);
                        DeleteObject(linePen);
                    }
                    prevX = bx; prevY = by;
                    
                    HBRUSH ptBrush = CreateSolidBrush(colors[i]);
                    HGDIOBJ oldBrush = SelectObject(memDC, ptBrush);
                    HPEN noPen = CreatePen(PS_NULL, 0, 0);
                    HGDIOBJ oldP = SelectObject(memDC, noPen);
                    Ellipse(memDC, bx - 6, by - 6, bx + 6, by + 6);
                    SelectObject(memDC, oldBrush);
                    SelectObject(memDC, oldP);
                    DeleteObject(ptBrush);
                    DeleteObject(noPen);
                    
                    RECT lr = { bx - 20, chartY + chartH + 5, bx + 20, chartY + chartH + 25 };
                    DrawTextA(memDC, labels[i], -1, &lr, DT_CENTER | DT_SINGLELINE);
                }
            }
            
            if (hoveredIndex != -1) {
                char txt[32];
                wsprintfA(txt, "%s: %d", labels[hoveredIndex], values[hoveredIndex]);
                
                SIZE sz;
                GetTextExtentPoint32A(memDC, txt, lstrlenA(txt), &sz);
                
                int th = 24;
                int tw = sz.cx + 16;
                int tx = mouseX + 10;
                int ty = mouseY - 10 - th;
                
                if (tx + tw > W) tx = mouseX - tw - 10;
                if (ty < 0) ty = mouseY + 20;
                
                RECT tr = { tx, ty, tx + tw, ty + th };
                HBRUSH tbg = CreateSolidBrush(RGB(30, 30, 35));
                FillRect(memDC, &tr, tbg);
                DeleteObject(tbg);
                
                HPEN tpen = CreatePen(PS_SOLID, 1, RGB(100, 100, 110));
                HGDIOBJ oldTpen = SelectObject(memDC, tpen);
                HBRUSH hollow = (HBRUSH)GetStockObject(NULL_BRUSH);
                HGDIOBJ oldHbg = SelectObject(memDC, hollow);
                Rectangle(memDC, tx, ty, tx + tw, ty + th);
                SelectObject(memDC, oldHbg);
                SelectObject(memDC, oldTpen);
                DeleteObject(tpen);
                DeleteObject(hollow);
                
                SetTextColor(memDC, RGB(255, 255, 255));
                RECT lrt = { tx, ty, tx + tw, ty + th };
                DrawTextA(memDC, txt, -1, &lrt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SetTextColor(memDC, RGB(161, 161, 170));
            }
            
            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
            
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KChartApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KChartApp", "KChart", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
