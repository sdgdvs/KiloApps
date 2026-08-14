#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int dpi = 96;
#define SCALE(x) MulDiv(x, dpi, 96)

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

static double my_sqrt(double x) {
    if (x <= 0.0) return 0.0;
    double guess = x;
    for (int i = 0; i < 15; i++) {
        guess = (guess + x / guess) * 0.5;
    }
    return guess;
}

#define NUM_ITEMS 5

int values[NUM_ITEMS] = { 20, 60, 45, 80, 30 };
int target[NUM_ITEMS] = { 20, 60, 45, 80, 30 };
const char* labels[NUM_ITEMS] = { "Q1", "Q2", "Q3", "Q4", "Q5" };

#define NUM_THEMES 5
const char* themeNames[NUM_THEMES] = { "Cyber Teal", "Neon Sunset", "Emerald Forest", "Sunset Warmth", "Synthwave" };

COLORREF themes[NUM_THEMES][NUM_ITEMS] = {
    // 0: Cyber Teal
    { RGB(20, 184, 166), RGB(245, 158, 11), RGB(236, 72, 153), RGB(139, 92, 246), RGB(59, 130, 246) },
    // 1: Neon Sunset
    { RGB(244, 114, 182), RGB(56, 189, 248), RGB(250, 204, 21), RGB(74, 222, 128), RGB(192, 132, 252) },
    // 2: Emerald Forest
    { RGB(16, 185, 129), RGB(52, 211, 153), RGB(5, 150, 105), RGB(110, 231, 183), RGB(20, 83, 45) },
    // 3: Sunset Warmth
    { RGB(245, 158, 11), RGB(239, 68, 68), RGB(217, 119, 6), RGB(249, 115, 22), RGB(252, 211, 77) },
    // 4: Synthwave
    { RGB(236, 72, 153), RGB(168, 85, 247), RGB(6, 182, 212), RGB(234, 179, 8), RGB(244, 63, 94) }
};

int currentTheme = 0;

int randSeed = 42;
int MyRand() {
    randSeed = randSeed * 1103515245 + 12345;
    return (unsigned int)(randSeed / 65536) % 32768;
}

HWND hBtnRandomize;
HWND hBtnToggle;
HWND hBtnTheme;
HWND hBtnSort;
HWND hBtnHelp;
HFONT hBtnFont = NULL;

// Chart modes: 0 = Bar, 1 = Line, 2 = Area, 3 = Pie, 4 = Donut, 5 = Radar
int chartMode = 0;
const char* modeNames[6] = { "Bar View", "Line View", "Area View", "Pie View", "Donut View", "Radar View" };

int hoveredIndex = -1;
int mouseX = 0, mouseY = 0;

// Stats variables
int statTotal = 0;
int statMean = 0;
int statMedian = 0;
int statStdDev = 0;
int statMin = 0;
int statMax = 0;

void CalculateStats() {
    statTotal = 0;
    statMin = values[0];
    statMax = values[0];
    int temp[NUM_ITEMS];

    for (int i = 0; i < NUM_ITEMS; i++) {
        statTotal += values[i];
        if (values[i] < statMin) statMin = values[i];
        if (values[i] > statMax) statMax = values[i];
        temp[i] = values[i];
    }
    statMean = statTotal / NUM_ITEMS;

    // Simple bubble sort for median
    for (int i = 0; i < NUM_ITEMS - 1; i++) {
        for (int j = 0; j < NUM_ITEMS - i - 1; j++) {
            if (temp[j] > temp[j + 1]) {
                int t = temp[j]; temp[j] = temp[j + 1]; temp[j + 1] = t;
            }
        }
    }
    statMedian = temp[NUM_ITEMS / 2];

    double varSum = 0;
    for (int i = 0; i < NUM_ITEMS; i++) {
        double diff = values[i] - statMean;
        varSum += diff * diff;
    }
    statStdDev = (int)my_sqrt(varSum / NUM_ITEMS);
}

void LayoutButtons(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int clientW = rc.right - rc.left;
    int clientH = rc.bottom - rc.top;

    if (hBtnRandomize && hBtnToggle && hBtnTheme && hBtnSort && hBtnHelp) {
        int btnW = SCALE(90);
        int btnH = SCALE(26);
        int gap = SCALE(8);
        int btnY = clientH - SCALE(38);
        if (btnY < 10) btnY = 10;
        int totalW = 5 * btnW + 4 * gap;
        int startX = (clientW - totalW) / 2;
        if (startX < 5) startX = 5;

        SetWindowPos(hBtnRandomize, NULL, startX, btnY, btnW, btnH, SWP_NOZORDER);
        SetWindowPos(hBtnToggle, NULL, startX + btnW + gap, btnY, btnW, btnH, SWP_NOZORDER);
        SetWindowPos(hBtnTheme, NULL, startX + (btnW + gap) * 2, btnY, btnW, btnH, SWP_NOZORDER);
        SetWindowPos(hBtnSort, NULL, startX + (btnW + gap) * 3, btnY, btnW, btnH, SWP_NOZORDER);
        SetWindowPos(hBtnHelp, NULL, startX + (btnW + gap) * 4, btnY, btnW, btnH, SWP_NOZORDER);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            randSeed = GetTickCount();
            hBtnRandomize = CreateWindowEx(0, "BUTTON", "Randomize",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 90, 26, hwnd, (HMENU)1, NULL, NULL);
            hBtnToggle = CreateWindowEx(0, "BUTTON", "Mode",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 90, 26, hwnd, (HMENU)2, NULL, NULL);
            hBtnTheme = CreateWindowEx(0, "BUTTON", "Theme",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 90, 26, hwnd, (HMENU)3, NULL, NULL);
            hBtnSort = CreateWindowEx(0, "BUTTON", "Sort",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 90, 26, hwnd, (HMENU)4, NULL, NULL);
            hBtnHelp = CreateWindowEx(0, "BUTTON", "Help",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 90, 26, hwnd, (HMENU)5, NULL, NULL);
            
            hBtnFont = CreateFontA(SCALE(-14), 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            SendMessage(hBtnRandomize, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessage(hBtnToggle, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessage(hBtnTheme, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessage(hBtnSort, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessage(hBtnHelp, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            
            LayoutButtons(hwnd);
            CalculateStats();
            SetTimer(hwnd, 1, 16, NULL);
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

            if (chartMode == 3 || chartMode == 4) { // Pie or Donut
                int cx = W / 2 - 50;
                int cy = (H - 80) / 2 + 35;
                int dx = mouseX - cx;
                int dy = mouseY - cy;
                int distSq = dx * dx + dy * dy;
                int outerR = (W < H - 80 ? W : H - 80) * 35 / 100;
                int innerR = chartMode == 4 ? outerR * 55 / 100 : 0;

                if (distSq >= innerR * innerR && distSq <= (outerR + 10) * (outerR + 10)) {
                    double angle = 0;
                    if (dx != 0 || dy != 0) {
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
                    for (int i = 0; i < NUM_ITEMS; i++) total += (values[i] > 0 ? values[i] : 0);
                    if (total <= 0) total = 1;

                    double currAngle = -MY_PI / 2.0;
                    for (int i = 0; i < NUM_ITEMS; i++) {
                        int v = values[i] > 0 ? values[i] : 0;
                        double sliceAngle = ((double)v / (double)total) * (MY_PI * 2.0);
                        if (angle >= currAngle && angle <= currAngle + sliceAngle) {
                            newHover = i;
                            break;
                        }
                        currAngle += sliceAngle;
                    }
                }
            } else if (chartMode == 5) { // Radar
                int cx = W / 2;
                int cy = (H - 80) / 2 + 35;
                int radius = (W < H - 80 ? W : H - 80) * 35 / 100;
                double angleStep = (MY_PI * 2.0) / NUM_ITEMS;
                int dx = mouseX - cx;
                int dy = mouseY - cy;
                int distSq = dx * dx + dy * dy;

                if (distSq <= (radius + 20) * (radius + 20)) {
                    double angle = 0;
                    if (dx != 0 || dy != 0) {
                        double absX = dx < 0 ? -dx : dx;
                        double absY = dy < 0 ? -dy : dy;
                        if (absX > absY) {
                            angle = (dy >= 0 ? 1.0 : -1.0) * (MY_PI / 2.0 - (dx >= 0 ? absY / absX : MY_PI - absY / absX));
                        } else {
                            angle = (dy >= 0 ? MY_PI / 2.0 : -MY_PI / 2.0);
                        }
                    }
                    if (angle < -MY_PI / 2.0) angle += MY_PI * 2.0;

                    for (int i = 0; i < NUM_ITEMS; i++) {
                        double spokeAngle = -MY_PI / 2.0 + i * angleStep;
                        double diff = angle - spokeAngle;
                        if (diff < 0) diff = -diff;
                        if (diff > MY_PI) diff = MY_PI * 2.0 - diff;
                        if (diff < angleStep / 2.0) {
                            newHover = i;
                            break;
                        }
                    }
                }
            } else { // Bar, Line, Area
                int chartX = 50;
                int chartY = 35;
                int chartW = W - 75;
                int chartH = H - 110;
                if (chartW < 50) chartW = 50;
                if (chartH < 50) chartH = 50;

                int maxVal = 100;
                for (int i = 0; i < NUM_ITEMS; i++) if (values[i] > maxVal) maxVal = values[i];
                if (maxVal <= 0) maxVal = 1;

                int barW = 30;
                int spacing = (chartW - (NUM_ITEMS * barW)) / (NUM_ITEMS + 1);

                for (int i = 0; i < NUM_ITEMS; i++) {
                    int bx = chartX + spacing + i * (barW + spacing);
                    if (mouseX >= bx && mouseX <= bx + barW && mouseY >= chartY && mouseY <= chartY + chartH) {
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
        case WM_KEYDOWN: {
            if (wParam == 'H' || wParam == 'h') {
                MessageBox(hwnd, "KChart Studio Help:\n\n- Use Mode button to switch charts\n- Use Theme to change colors\n- Sort button sorts data\n- Randomize generates new data", "Help", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }
        case WM_TIMER: {
            int changed = 0;
            for (int i = 0; i < NUM_ITEMS; i++) {
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
            if (changed) {
                CalculateStats();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_COMMAND: {
            SetFocus(hwnd); // Ensure the main window keeps focus for keyboard shortcuts like 'H'
            int cmdId = LOWORD(wParam);
            if (cmdId == 1) { // Randomize
                for (int i = 0; i < NUM_ITEMS; i++) {
                    target[i] = 10 + (MyRand() % 90);
                }
            } else if (cmdId == 2) { // Toggle Mode
                chartMode = (chartMode + 1) % 6;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (cmdId == 3) { // Toggle Theme
                currentTheme = (currentTheme + 1) % NUM_THEMES;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (cmdId == 4) { // Sort
                for (int i = 0; i < NUM_ITEMS - 1; i++) {
                    for (int j = 0; j < NUM_ITEMS - i - 1; j++) {
                        if (target[j] > target[j + 1]) {
                            int t = target[j]; target[j] = target[j + 1]; target[j + 1] = t;
                        }
                    }
                }
            } else if (cmdId == 5) { // Help
                MessageBox(hwnd, "KChart Studio Help:\n\n- Use Mode button to switch charts\n- Use Theme to change colors\n- Sort button sorts data\n- Randomize generates new data", "Help", MB_OK | MB_ICONINFORMATION);
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

            // Double buffer
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, W, H);
            HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

            // Dark background fill
            HBRUSH bg = CreateSolidBrush(RGB(9, 9, 11));
            FillRect(memDC, &rc, bg);
            DeleteObject(bg);

            HFONT hFont = CreateFontA(SCALE(-14), 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HGDIOBJ oldFont = SelectObject(memDC, hFont);
            SetBkMode(memDC, TRANSPARENT);

            // Title & Mode & Theme Header
            SetTextColor(memDC, RGB(244, 244, 245));
            char titleStr[128];
            wsprintfA(titleStr, "%s | Theme: %s | Press 'H' for Help", modeNames[chartMode], themeNames[currentTheme]);
            RECT titleR = { SCALE(15), SCALE(8), W - SCALE(15), SCALE(26) };
            DrawTextA(memDC, titleStr, -1, &titleR, DT_LEFT | DT_SINGLELINE);

            // Stats Suite Summary Bar
            char statsStr[256];
            wsprintfA(statsStr, "Mean: %d  Med: %d  StdDev: %d  Min: %d  Max: %d  Total: %d",
                statMean, statMedian, statStdDev, statMin, statMax, statTotal);
            SetTextColor(memDC, RGB(161, 161, 170));
            RECT statsR = { SCALE(15), SCALE(24), W - SCALE(15), SCALE(38) };
            DrawTextA(memDC, statsStr, -1, &statsR, DT_LEFT | DT_SINGLELINE);

            COLORREF* palette = themes[currentTheme];

            if (chartMode == 3 || chartMode == 4) { // Pie / Donut
                int cx = W / 2 - 50;
                int cy = (H - 80) / 2 + 35;
                int outerR = (W < H - 80 ? W : H - 80) * 35 / 100;
                int innerR = chartMode == 4 ? outerR * 55 / 100 : 0;
                if (outerR < 20) outerR = 20;

                int total = 0;
                for (int i = 0; i < NUM_ITEMS; i++) total += (values[i] > 0 ? values[i] : 0);
                if (total <= 0) total = 1;

                double startAngle = -MY_PI / 2.0;

                for (int i = 0; i < NUM_ITEMS; i++) {
                    int v = values[i] > 0 ? values[i] : 0;
                    double sliceAngle = ((double)v / (double)total) * (MY_PI * 2.0);
                    double endAngle = startAngle + sliceAngle;

                    int xr1 = cx + (int)(my_cos(startAngle) * 1000.0);
                    int yr1 = cy + (int)(my_sin(startAngle) * 1000.0);
                    int xr2 = cx + (int)(my_cos(endAngle) * 1000.0);
                    int yr2 = cy + (int)(my_sin(endAngle) * 1000.0);

                    HBRUSH sliceBrush = CreateSolidBrush(palette[i]);
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

                if (chartMode == 4) { // Cutout hole for Donut
                    HBRUSH holeBrush = CreateSolidBrush(RGB(9, 9, 11));
                    HGDIOBJ oldHoleBrush = SelectObject(memDC, holeBrush);
                    HPEN nullPen = CreatePen(PS_NULL, 0, 0);
                    HGDIOBJ oldNullPen = SelectObject(memDC, nullPen);
                    Ellipse(memDC, cx - innerR, cy - innerR, cx + innerR, cy + innerR);
                    SelectObject(memDC, oldHoleBrush);
                    SelectObject(memDC, oldNullPen);
                    DeleteObject(holeBrush);
                    DeleteObject(nullPen);
                }

                // Legend
                int legX = W - 110;
                int legY = cy - 45;
                for (int i = 0; i < NUM_ITEMS; i++) {
                    int ly = legY + i * 20;
                    HBRUSH legBrush = CreateSolidBrush(palette[i]);
                    RECT legDot = { legX, ly + 3, legX + 10, ly + 13 };
                    FillRect(memDC, &legDot, legBrush);
                    DeleteObject(legBrush);

                    char ltxt[32];
                    wsprintfA(ltxt, "%s: %d", labels[i], values[i]);
                    RECT legTxtR = { legX + 15, ly, W - 5, ly + 18 };
                    SetTextColor(memDC, hoveredIndex == i ? RGB(255, 255, 255) : RGB(161, 161, 170));
                    DrawTextA(memDC, ltxt, -1, &legTxtR, DT_LEFT | DT_SINGLELINE);
                }
            } else if (chartMode == 5) { // Radar View
                int cx = W / 2;
                int cy = (H - 80) / 2 + 35;
                int radius = (W < H - 80 ? W : H - 80) * 35 / 100;
                double angleStep = (MY_PI * 2.0) / NUM_ITEMS;

                // Web Grid
                HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(40, 40, 50));
                HGDIOBJ oldGridPen = SelectObject(memDC, gridPen);

                for (int level = 1; level <= 4; level++) {
                    int r = radius * level / 4;
                    POINT pts[NUM_ITEMS];
                    for (int i = 0; i < NUM_ITEMS; i++) {
                        double angle = -MY_PI / 2.0 + i * angleStep;
                        pts[i].x = cx + (int)(my_cos(angle) * r);
                        pts[i].y = cy + (int)(my_sin(angle) * r);
                    }
                    Polygon(memDC, pts, NUM_ITEMS);
                }

                // Spokes & Labels
                for (int i = 0; i < NUM_ITEMS; i++) {
                    double angle = -MY_PI / 2.0 + i * angleStep;
                    int sx = cx + (int)(my_cos(angle) * radius);
                    int sy = cy + (int)(my_sin(angle) * radius);
                    MoveToEx(memDC, cx, cy, NULL);
                    LineTo(memDC, sx, sy);

                    int lx = cx + (int)(my_cos(angle) * (radius + 18));
                    int ly = cy + (int)(my_sin(angle) * (radius + 18));
                    RECT lr = { lx - 20, ly - 8, lx + 20, ly + 8 };
                    SetTextColor(memDC, hoveredIndex == i ? RGB(255, 255, 255) : RGB(161, 161, 170));
                    DrawTextA(memDC, labels[i], -1, &lr, DT_CENTER | DT_SINGLELINE);
                }
                SelectObject(memDC, oldGridPen);
                DeleteObject(gridPen);

                // Data polygon
                int maxVal = 100;
                for (int i = 0; i < NUM_ITEMS; i++) if (values[i] > maxVal) maxVal = values[i];
                if (maxVal <= 0) maxVal = 1;

                POINT dpts[NUM_ITEMS];
                for (int i = 0; i < NUM_ITEMS; i++) {
                    double r = (double)values[i] / maxVal * radius;
                    double angle = -MY_PI / 2.0 + i * angleStep;
                    dpts[i].x = cx + (int)(my_cos(angle) * r);
                    dpts[i].y = cy + (int)(my_sin(angle) * r);
                }

                HBRUSH polyBrush = CreateSolidBrush(palette[0]);
                HGDIOBJ oldPBrush = SelectObject(memDC, polyBrush);
                HPEN polyPen = CreatePen(PS_SOLID, 2, palette[0]);
                HGDIOBJ oldPPen = SelectObject(memDC, polyPen);

                Polygon(memDC, dpts, NUM_ITEMS);

                SelectObject(memDC, oldPBrush);
                SelectObject(memDC, oldPPen);
                DeleteObject(polyBrush);
                DeleteObject(polyPen);

                // Points
                for (int i = 0; i < NUM_ITEMS; i++) {
                    HBRUSH ptBrush = CreateSolidBrush(palette[i]);
                    HGDIOBJ oldBrush = SelectObject(memDC, ptBrush);
                    HPEN noPen = CreatePen(PS_NULL, 0, 0);
                    HGDIOBJ oldP = SelectObject(memDC, noPen);
                    int pr = (hoveredIndex == i) ? 7 : 4;
                    Ellipse(memDC, dpts[i].x - pr, dpts[i].y - pr, dpts[i].x + pr, dpts[i].y + pr);
                    SelectObject(memDC, oldBrush);
                    SelectObject(memDC, oldP);
                    DeleteObject(ptBrush);
                    DeleteObject(noPen);
                }
            } else { // Bar, Line, Area
                int chartX = 50;
                int chartY = 45;
                int chartW = W - 75;
                int chartH = H - 95;
                if (chartW < 50) chartW = 50;
                if (chartH < 50) chartH = 50;

                int maxVal = 100;
                for (int i = 0; i < NUM_ITEMS; i++) if (values[i] > maxVal) maxVal = values[i];
                if (maxVal <= 0) maxVal = 1;

                // Grid lines & Y scale
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

                // Axes
                HPEN axisPen = CreatePen(PS_SOLID, 2, RGB(100, 116, 139));
                HGDIOBJ oldAxisPen = SelectObject(memDC, axisPen);
                MoveToEx(memDC, chartX, chartY, NULL);
                LineTo(memDC, chartX, chartY + chartH);
                LineTo(memDC, chartX + chartW, chartY + chartH);
                SelectObject(memDC, oldAxisPen);
                DeleteObject(axisPen);

                int barW = 30;
                int spacing = (chartW - (NUM_ITEMS * barW)) / (NUM_ITEMS + 1);

                if (chartMode == 0) { // Bar Chart
                    for (int i = 0; i < NUM_ITEMS; i++) {
                        int safeV = values[i] > 0 ? values[i] : 0;
                        int bh = (safeV * chartH) / maxVal;
                        int bx = chartX + spacing + i * (barW + spacing);
                        int by = chartY + chartH - bh;

                        RECT br = { bx, by, bx + barW, chartY + chartH };
                        HBRUSH brBrush = CreateSolidBrush(palette[i]);
                        FillRect(memDC, &br, brBrush);
                        DeleteObject(brBrush);

                        SetTextColor(memDC, hoveredIndex == i ? RGB(255, 255, 255) : RGB(161, 161, 170));
                        RECT lr = { bx - 10, chartY + chartH + 6, bx + barW + 10, chartY + chartH + 24 };
                        DrawTextA(memDC, labels[i], -1, &lr, DT_CENTER | DT_SINGLELINE);
                    }
                } else if (chartMode == 1 || chartMode == 2) { // Line / Area Chart
                    POINT pts[NUM_ITEMS];
                    for (int i = 0; i < NUM_ITEMS; i++) {
                        int safeV = values[i] > 0 ? values[i] : 0;
                        int bh = (safeV * chartH) / maxVal;
                        pts[i].x = chartX + spacing + i * (barW + spacing) + barW / 2;
                        pts[i].y = chartY + chartH - bh;
                    }

                    if (chartMode == 2) { // Area fill polygon
                        POINT polyPts[NUM_ITEMS + 2];
                        polyPts[0].x = pts[0].x;
                        polyPts[0].y = chartY + chartH;
                        for (int i = 0; i < NUM_ITEMS; i++) {
                            polyPts[i + 1] = pts[i];
                        }
                        polyPts[NUM_ITEMS + 1].x = pts[NUM_ITEMS - 1].x;
                        polyPts[NUM_ITEMS + 1].y = chartY + chartH;

                        HBRUSH areaBrush = CreateSolidBrush(RGB(20, 80, 90));
                        HGDIOBJ oldAB = SelectObject(memDC, areaBrush);
                        HPEN noPen = CreatePen(PS_NULL, 0, 0);
                        HGDIOBJ oldNP = SelectObject(memDC, noPen);
                        Polygon(memDC, polyPts, NUM_ITEMS + 2);
                        SelectObject(memDC, oldAB);
                        SelectObject(memDC, oldNP);
                        DeleteObject(areaBrush);
                        DeleteObject(noPen);
                    }

                    // Line connecting points
                    HPEN linePen = CreatePen(PS_SOLID, 3, palette[0]);
                    HGDIOBJ oldLinePen = SelectObject(memDC, linePen);
                    MoveToEx(memDC, pts[0].x, pts[0].y, NULL);
                    for (int i = 1; i < NUM_ITEMS; i++) {
                        LineTo(memDC, pts[i].x, pts[i].y);
                    }
                    SelectObject(memDC, oldLinePen);
                    DeleteObject(linePen);

                    // Dots & Labels
                    for (int i = 0; i < NUM_ITEMS; i++) {
                        HBRUSH ptBrush = CreateSolidBrush(palette[i]);
                        HGDIOBJ oldBrush = SelectObject(memDC, ptBrush);
                        HPEN noPen = CreatePen(PS_NULL, 0, 0);
                        HGDIOBJ oldP = SelectObject(memDC, noPen);
                        int pr = (hoveredIndex == i) ? 7 : 5;
                        Ellipse(memDC, pts[i].x - pr, pts[i].y - pr, pts[i].x + pr, pts[i].y + pr);
                        SelectObject(memDC, oldBrush);
                        SelectObject(memDC, oldP);
                        DeleteObject(ptBrush);
                        DeleteObject(noPen);

                        SetTextColor(memDC, hoveredIndex == i ? RGB(255, 255, 255) : RGB(161, 161, 170));
                        RECT lr = { pts[i].x - 20, chartY + chartH + 6, pts[i].x + 20, chartY + chartH + 24 };
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
    SetProcessDPIAware();
    HDC hdc = GetDC(NULL);
    if (hdc) {
        dpi = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);
    }
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KChartApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    RECT rect = { 0, 0, SCALE(1024), SCALE(768) };
    AdjustWindowRect(&rect, style, FALSE);

    HWND hwnd = CreateWindowEx(0, "KChartApp", "KChart Studio", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
