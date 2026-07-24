#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MY_PI 3.14159265358979323846

static double my_sin(double x) {
    while (x < -MY_PI) x += 2.0 * MY_PI;
    while (x > MY_PI) x -= 2.0 * MY_PI;
    double x2 = x * x;
    return x * (1.0 - x2 / 6.0 + (x2 * x2) / 120.0 - (x2 * x2 * x2) / 5040.0);
}

static double my_cos(double x) {
    return my_sin(x + MY_PI / 2.0);
}

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
HFONT hBtnFont = NULL;

// Chart modes: 0 = Bar, 1 = Line, 2 = Donut
int chartMode = 0;
const char* modeNames[3] = { "Bar View", "Line View", "Donut View" };

int hoveredIndex = -1;
int mouseX = 0, mouseY = 0;

void LayoutButtons(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int clientW = rc.right - rc.left;
    int clientH = rc.bottom - rc.top;

    if (hBtnRandomize && hBtnToggle) {
        int btnW = 100;
        int btnH = 26;
        int btnY = clientH - 45;
        if (btnY < 10) btnY = 10;
        SetWindowPos(hBtnRandomize, NULL, clientW / 2 - 110, btnY, btnW, btnH, SWP_NOZORDER);
        SetWindowPos(hBtnToggle, NULL, clientW / 2 + 10, btnY, btnW, btnH, SWP_NOZORDER);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            randSeed = GetTickCount();
            hBtnRandomize = CreateWindowEx(0, "BUTTON", "Randomize",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 100, 26, hwnd, (HMENU)1, NULL, NULL);
            hBtnToggle = CreateWindowEx(0, "BUTTON", "Toggle Type",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 100, 26, hwnd, (HMENU)2, NULL, NULL);
            
            hBtnFont = CreateFontA(14, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            SendMessage(hBtnRandomize, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessage(hBtnToggle, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            
            LayoutButtons(hwnd);
            SetTimer(hwnd, 1, 16, NULL); // Smooth animation timer
            break;
        }
        case WM_SIZE: {
            LayoutButtons(hwnd);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_MOUSEMOVE: {
            mouseX = LOWORD(lParam);
            mouseY = HIWORD(lParam);
            
            RECT rc;
            GetClientRect(hwnd, &rc);
            int W = rc.right - rc.left;
            int H = rc.bottom - rc.top;

            int newHover = -1;

            if (chartMode == 2) {
                // Donut mode hit test
                int cx = W / 2 - 40;
                int cy = (H - 60) / 2 + 25;
                int dx = mouseX - cx;
                int dy = mouseY - cy;
                int distSq = dx * dx + dy * dy;
                int outerR = (W < H - 60 ? W : H - 60) * 35 / 100;
                int innerR = outerR * 55 / 100;

                if (distSq >= innerR * innerR && distSq <= (outerR + 10) * (outerR + 10)) {
                    double angle = 0;
                    if (dx != 0 || dy != 0) {
                        // Approximate atan2 for quadrant checks
                        double absX = dx < 0 ? -dx : dx;
                        double absY = dy < 0 ? -dy : dy;
                        if (absX > absY) {
                            angle = (dy >= 0 ? 1.0 : -1.0) * (MY_PI / 2.0 - (dx >= 0 ? absY / absX : MY_PI - absY / absX));
                        } else {
                            angle = (dy >= 0 ? MY_PI / 2.0 : -MY_PI / 2.0);
                        }
                    }
                    if (angle < -MY_PI / 2.0) angle += MY_PI * 2.0;

                    int total = 0;
                    for (int i = 0; i < 5; i++) total += (values[i] > 0 ? values[i] : 0);
                    if (total <= 0) total = 1;

                    double currAngle = -MY_PI / 2.0;
                    for (int i = 0; i < 5; i++) {
                        int v = values[i] > 0 ? values[i] : 0;
                        double sliceAngle = ((double)v / (double)total) * (MY_PI * 2.0);
                        if (angle >= currAngle && angle <= currAngle + sliceAngle) {
                            newHover = i;
                            break;
                        }
                        currAngle += sliceAngle;
                    }
                }
            } else {
                int chartX = 50;
                int chartY = 30;
                int chartW = W - 75;
                int chartH = H - 95;
                if (chartW < 50) chartW = 50;
                if (chartH < 50) chartH = 50;

                int maxVal = 100;
                for (int i = 0; i < 5; i++) if (values[i] > maxVal) maxVal = values[i];
                if (maxVal <= 0) maxVal = 1;

                int barW = 30;
                int spacing = (chartW - (5 * barW)) / 6;

                for (int i = 0; i < 5; i++) {
                    int safeV = values[i] > 0 ? values[i] : 0;
                    int bh = (safeV * chartH) / maxVal;
                    int bx, by, bw, h;

                    if (chartMode == 0) {
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

                    if (mouseX >= bx && mouseX <= bx + bw && mouseY >= by - 5 && mouseY <= chartY + chartH) {
                        newHover = i;
                        break;
                    }
                }
            }

            if (newHover != hoveredIndex) {
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
                    target[i] = 10 + (MyRand() % 90);
                }
            } else if (LOWORD(wParam) == 2) {
                chartMode = (chartMode + 1) % 3;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc;
            GetClientRect(hwnd, &rc);
            int W = rc.right - rc.left;
            int H = rc.bottom - rc.top;

            if (W <= 0 || H <= 0) {
                EndPaint(hwnd, &ps);
                break;
            }

            // Double buffer memory setup
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, W, H);
            HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

            // Dark background fill
            HBRUSH bg = CreateSolidBrush(RGB(9, 9, 11));
            FillRect(memDC, &rc, bg);
            DeleteObject(bg);

            HFONT hFont = CreateFontA(13, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HGDIOBJ oldFont = SelectObject(memDC, hFont);
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(161, 161, 170));

            // Header Title & Mode
            RECT titleR = { 15, 10, W - 15, 30 };
            DrawTextA(memDC, modeNames[chartMode], -1, &titleR, DT_LEFT | DT_SINGLELINE);

            if (chartMode == 2) {
                // Donut Chart Rendering
                int cx = W / 2 - 40;
                int cy = (H - 60) / 2 + 25;
                int outerR = (W < H - 60 ? W : H - 60) * 35 / 100;
                int innerR = outerR * 55 / 100;
                if (outerR < 20) outerR = 20;

                int total = 0;
                for (int i = 0; i < 5; i++) total += (values[i] > 0 ? values[i] : 0);
                if (total <= 0) total = 1;

                double startAngle = -MY_PI / 2.0;

                for (int i = 0; i < 5; i++) {
                    int v = values[i] > 0 ? values[i] : 0;
                    double sliceAngle = ((double)v / (double)total) * (MY_PI * 2.0);
                    double endAngle = startAngle + sliceAngle;

                    int xr1 = cx + (int)(my_cos(startAngle) * 1000.0);
                    int yr1 = cy + (int)(my_sin(startAngle) * 1000.0);
                    int xr2 = cx + (int)(my_cos(endAngle) * 1000.0);
                    int yr2 = cy + (int)(my_sin(endAngle) * 1000.0);

                    HBRUSH sliceBrush = CreateSolidBrush(colors[i]);
                    HGDIOBJ oldBrush = SelectObject(memDC, sliceBrush);
                    HPEN slicePen = CreatePen(PS_SOLID, 1, RGB(9, 9, 11));
                    HGDIOBJ oldPen = SelectObject(memDC, slicePen);

                    Pie(memDC, cx - outerR, cy - outerR, cx + outerR, cy + outerR, xr1, yr1, xr2, yr2);

                    SelectObject(memDC, oldBrush);
                    SelectObject(memDC, oldPen);
                    DeleteObject(sliceBrush);
                    DeleteObject(slicePen);

                    startAngle = endAngle;
                }

                // Inner donut hole
                HBRUSH holeBrush = CreateSolidBrush(RGB(9, 9, 11));
                HGDIOBJ oldHoleBrush = SelectObject(memDC, holeBrush);
                HPEN nullPen = CreatePen(PS_NULL, 0, 0);
                HGDIOBJ oldNullPen = SelectObject(memDC, nullPen);
                Ellipse(memDC, cx - innerR, cy - innerR, cx + innerR, cy + innerR);
                SelectObject(memDC, oldHoleBrush);
                SelectObject(memDC, oldNullPen);
                DeleteObject(holeBrush);
                DeleteObject(nullPen);

                // Legend on right side
                int legX = W - 100;
                int legY = cy - 45;
                for (int i = 0; i < 5; i++) {
                    int ly = legY + i * 20;
                    HBRUSH legBrush = CreateSolidBrush(colors[i]);
                    RECT legDot = { legX, ly + 3, legX + 10, ly + 13 };
                    FillRect(memDC, &legDot, legBrush);
                    DeleteObject(legBrush);

                    char ltxt[32];
                    wsprintfA(ltxt, "%s: %d", labels[i], values[i]);
                    RECT legTxtR = { legX + 15, ly, W - 5, ly + 18 };
                    SetTextColor(memDC, hoveredIndex == i ? RGB(255, 255, 255) : RGB(161, 161, 170));
                    DrawTextA(memDC, ltxt, -1, &legTxtR, DT_LEFT | DT_SINGLELINE);
                }
            } else {
                int chartX = 50;
                int chartY = 30;
                int chartW = W - 75;
                int chartH = H - 95;
                if (chartW < 50) chartW = 50;
                if (chartH < 50) chartH = 50;

                int maxVal = 100;
                for (int i = 0; i < 5; i++) if (values[i] > maxVal) maxVal = values[i];
                if (maxVal <= 0) maxVal = 1;

                // Grid lines & Y-axis scale labels
                HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(40, 40, 48));
                HGDIOBJ oldGridPen = SelectObject(memDC, gridPen);
                SetTextColor(memDC, RGB(113, 113, 122));

                for (int i = 0; i <= 4; i++) {
                    int y = chartY + (chartH / 4) * i;
                    MoveToEx(memDC, chartX, y, NULL);
                    LineTo(memDC, chartX + chartW, y);

                    int valNum = maxVal - (maxVal / 4) * i;
                    char vstr[16];
                    wsprintfA(vstr, "%d", valNum);
                    RECT vr = { chartX - 42, y - 7, chartX - 5, y + 10 };
                    DrawTextA(memDC, vstr, -1, &vr, DT_RIGHT | DT_SINGLELINE);
                }
                SelectObject(memDC, oldGridPen);
                DeleteObject(gridPen);

                // Draw Axes
                HPEN axisPen = CreatePen(PS_SOLID, 2, RGB(100, 116, 139));
                HGDIOBJ oldAxisPen = SelectObject(memDC, axisPen);
                MoveToEx(memDC, chartX, chartY, NULL);
                LineTo(memDC, chartX, chartY + chartH);
                LineTo(memDC, chartX + chartW, chartY + chartH);
                SelectObject(memDC, oldAxisPen);
                DeleteObject(axisPen);

                int barW = 30;
                int spacing = (chartW - (5 * barW)) / 6;

                if (chartMode == 0) {
                    // Bar Chart
                    for (int i = 0; i < 5; i++) {
                        int bw = barW;
                        int safeV = values[i] > 0 ? values[i] : 0;
                        int bh = (safeV * chartH) / maxVal;
                        int bx = chartX + spacing + i * (barW + spacing);
                        int by = chartY + chartH - bh;
                        if (by > chartY + chartH - 1) by = chartY + chartH - 1;

                        RECT br = { bx, by, bx + bw, chartY + chartH };
                        HBRUSH brBrush = CreateSolidBrush(colors[i]);
                        FillRect(memDC, &br, brBrush);
                        DeleteObject(brBrush);

                        SetTextColor(memDC, hoveredIndex == i ? RGB(255, 255, 255) : RGB(161, 161, 170));
                        RECT lr = { bx - 10, chartY + chartH + 6, bx + bw + 10, chartY + chartH + 24 };
                        DrawTextA(memDC, labels[i], -1, &lr, DT_CENTER | DT_SINGLELINE);
                    }
                } else {
                    // Line Chart
                    int prevX = 0, prevY = 0;
                    for (int i = 0; i < 5; i++) {
                        int bw = barW;
                        int safeV = values[i] > 0 ? values[i] : 0;
                        int bh = (safeV * chartH) / maxVal;
                        int bx = chartX + spacing + i * (barW + spacing) + bw / 2;
                        int by = chartY + chartH - bh;

                        if (i > 0) {
                            HPEN linePen = CreatePen(PS_SOLID, 2, RGB(20, 184, 166));
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
                        int pr = (hoveredIndex == i) ? 7 : 5;
                        Ellipse(memDC, bx - pr, by - pr, bx + pr, by + pr);
                        SelectObject(memDC, oldBrush);
                        SelectObject(memDC, oldP);
                        DeleteObject(ptBrush);
                        DeleteObject(noPen);

                        SetTextColor(memDC, hoveredIndex == i ? RGB(255, 255, 255) : RGB(161, 161, 170));
                        RECT lr = { bx - 20, chartY + chartH + 6, bx + 20, chartY + chartH + 24 };
                        DrawTextA(memDC, labels[i], -1, &lr, DT_CENTER | DT_SINGLELINE);
                    }
                }
            }

            // Hover Tooltip
            if (hoveredIndex != -1) {
                char txt[32];
                wsprintfA(txt, "%s: %d", labels[hoveredIndex], values[hoveredIndex]);

                SIZE sz;
                GetTextExtentPoint32A(memDC, txt, lstrlenA(txt), &sz);

                int th = 24;
                int tw = sz.cx + 16;
                int tx = mouseX + 10;
                int ty = mouseY - 10 - th;

                // Tooltip boundary clamping
                if (tx + tw > W - 5) tx = mouseX - tw - 10;
                if (tx < 5) tx = 5;
                if (ty < 5) ty = mouseY + 15;
                if (ty + th > H - 5) ty = H - th - 5;

                RECT tr = { tx, ty, tx + tw, ty + th };
                HBRUSH tbg = CreateSolidBrush(RGB(24, 24, 30));
                FillRect(memDC, &tr, tbg);
                DeleteObject(tbg);

                HPEN tpen = CreatePen(PS_SOLID, 1, RGB(100, 100, 120));
                HGDIOBJ oldTpen = SelectObject(memDC, tpen);
                HBRUSH hollow = (HBRUSH)GetStockObject(NULL_BRUSH);
                HGDIOBJ oldHbg = SelectObject(memDC, hollow);
                Rectangle(memDC, tx, ty, tx + tw, ty + th);
                SelectObject(memDC, oldHbg);
                SelectObject(memDC, oldTpen);
                DeleteObject(tpen);

                SetTextColor(memDC, RGB(255, 255, 255));
                RECT lrt = { tx, ty, tx + tw, ty + th };
                DrawTextA(memDC, txt, -1, &lrt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
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
            KillTimer(hwnd, 1);
            if (hBtnFont) DeleteObject(hBtnFont);
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

    HWND hwnd = CreateWindowEx(0, "KChartApp", "KChart", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 320, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
