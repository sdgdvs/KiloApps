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

static double my_atan(double z) {
    if (z > 1.0) return MY_PI / 2.0 - my_atan(1.0 / z);
    if (z < -1.0) return -MY_PI / 2.0 - my_atan(1.0 / z);
    double z2 = z * z;
    return z * (1.0 - z2 / 3.0 + (z2 * z2) / 5.0 - (z2 * z2 * z2) / 7.0 + (z2 * z2 * z2 * z2) / 9.0);
}

static double my_atan2(double y, double x) {
    if (x > 0.0) return my_atan(y / x);
    if (x < 0.0 && y >= 0.0) return my_atan(y / x) + MY_PI;
    if (x < 0.0 && y < 0.0) return my_atan(y / x) - MY_PI;
    if (x == 0.0 && y > 0.0) return MY_PI / 2.0;
    if (x == 0.0 && y < 0.0) return -MY_PI / 2.0;
    return 0.0;
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
HWND hBtnTrend;
HWND hBtnSort;
HWND hBtnHelp;
HFONT hBtnFont = NULL;

// Chart modes: 0 = Bar, 1 = Line, 2 = Area, 3 = Pie, 4 = Donut, 5 = Radar
int chartMode = 0;
const char* modeNames[6] = { "Bar View", "Line View", "Area View", "Pie View", "Donut View", "Radar View" };

// Trend modes: 0 = Off, 1 = Linear Fit, 2 = Mov Avg (3), 3 = Mean Line
int trendMode = 0;
const char* trendNames[4] = { "Trend: Off", "Trend: Linear Fit", "Trend: MovAvg", "Trend: Mean" };

double trendSlope = 0.0;
double trendIntercept = 0.0;
double trendR2 = 0.0;

static void FormatTrendEq(char* dest, double slope, double intercept, double r2) {
    int sSign = slope < 0 ? -1 : 1;
    double absSlope = slope < 0 ? -slope : slope;
    int sWhole = (int)absSlope;
    int sFrac = (int)((absSlope - (double)sWhole) * 100.0 + 0.5);
    if (sFrac >= 100) { sWhole++; sFrac = 0; }

    int iSign = intercept < 0 ? -1 : 1;
    double absInt = intercept < 0 ? -intercept : intercept;
    int iWhole = (int)absInt;
    int iFrac = (int)((absInt - (double)iWhole) * 100.0 + 0.5);
    if (iFrac >= 100) { iWhole++; iFrac = 0; }

    if (r2 >= 0.9995) {
        wsprintfA(dest, "Fit: y = %s%d.%02dx %c %d.%02d (R2=1.000)",
            sSign < 0 ? "-" : "", sWhole, sFrac,
            iSign < 0 ? '-' : '+', iWhole, iFrac);
    } else {
        int rFrac = (int)(r2 * 1000.0 + 0.5);
        if (rFrac > 999) rFrac = 999;
        if (rFrac < 0) rFrac = 0;
        wsprintfA(dest, "Fit: y = %s%d.%02dx %c %d.%02d (R2=0.%03d)",
            sSign < 0 ? "-" : "", sWhole, sFrac,
            iSign < 0 ? '-' : '+', iWhole, iFrac,
            rFrac);
    }
}

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

    // Linear Regression
    double sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;
    for (int i = 0; i < NUM_ITEMS; i++) {
        sumX += (double)i;
        sumY += (double)values[i];
        sumXY += (double)i * (double)values[i];
        sumXX += (double)i * (double)i;
    }
    double n = (double)NUM_ITEMS;
    double denom = n * sumXX - sumX * sumX;
    if (denom != 0.0) {
        trendSlope = (n * sumXY - sumX * sumY) / denom;
        trendIntercept = (sumY - trendSlope * sumX) / n;
        double ssTot = 0, ssRes = 0;
        for (int i = 0; i < NUM_ITEMS; i++) {
            double yHat = trendSlope * (double)i + trendIntercept;
            double diffTot = (double)values[i] - (sumY / n);
            double diffRes = (double)values[i] - yHat;
            ssTot += diffTot * diffTot;
            ssRes += diffRes * diffRes;
        }
        trendR2 = (ssTot > 0.0001) ? (1.0 - ssRes / ssTot) : 1.0;
        if (trendR2 < 0.0) trendR2 = 0.0;
        if (trendR2 > 1.0) trendR2 = 1.0;
    } else {
        trendSlope = 0.0;
        trendIntercept = (double)statMean;
        trendR2 = 0.0;
    }
}

void LayoutButtons(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int clientW = rc.right - rc.left;
    int clientH = rc.bottom - rc.top;

    if (hBtnRandomize && hBtnToggle && hBtnTheme && hBtnTrend && hBtnSort && hBtnHelp) {
        int btnW = SCALE(80);
        int btnH = SCALE(26);
        int gap = SCALE(6);
        int btnY = clientH - SCALE(38);
        if (btnY < 10) btnY = 10;
        int totalW = 6 * btnW + 5 * gap;
        int startX = (clientW - totalW) / 2;
        if (startX < 5) startX = 5;

        SetWindowPos(hBtnRandomize, NULL, startX, btnY, btnW, btnH, SWP_NOZORDER);
        SetWindowPos(hBtnToggle, NULL, startX + btnW + gap, btnY, btnW, btnH, SWP_NOZORDER);
        SetWindowPos(hBtnTheme, NULL, startX + (btnW + gap) * 2, btnY, btnW, btnH, SWP_NOZORDER);
        SetWindowPos(hBtnTrend, NULL, startX + (btnW + gap) * 3, btnY, btnW, btnH, SWP_NOZORDER);
        SetWindowPos(hBtnSort, NULL, startX + (btnW + gap) * 4, btnY, btnW, btnH, SWP_NOZORDER);
        SetWindowPos(hBtnHelp, NULL, startX + (btnW + gap) * 5, btnY, btnW, btnH, SWP_NOZORDER);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            randSeed = GetTickCount();
            hBtnRandomize = CreateWindowEx(0, "BUTTON", "Randomize",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                0, 0, 80, 26, hwnd, (HMENU)1, NULL, NULL);
            hBtnToggle = CreateWindowEx(0, "BUTTON", "Mode",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                0, 0, 80, 26, hwnd, (HMENU)2, NULL, NULL);
            hBtnTheme = CreateWindowEx(0, "BUTTON", "Theme",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                0, 0, 80, 26, hwnd, (HMENU)3, NULL, NULL);
            hBtnTrend = CreateWindowEx(0, "BUTTON", "Trend",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                0, 0, 80, 26, hwnd, (HMENU)6, NULL, NULL);
            hBtnSort = CreateWindowEx(0, "BUTTON", "Sort",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                0, 0, 80, 26, hwnd, (HMENU)4, NULL, NULL);
            hBtnHelp = CreateWindowEx(0, "BUTTON", "Help",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
                0, 0, 80, 26, hwnd, (HMENU)5, NULL, NULL);
            
            int fontHeight = -MulDiv(12, dpi, 72);
            hBtnFont = CreateFontA(fontHeight, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            SendMessage(hBtnRandomize, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessage(hBtnToggle, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessage(hBtnTheme, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessage(hBtnTrend, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
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
                    double angle = my_atan2((double)dy, (double)dx);
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
                    double angle = my_atan2((double)dy, (double)dx);
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
                int chartY = 45;
                int chartW = W - 75;
                int chartH = H - 95;
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
            if (wParam == 'H' || wParam == 'h' || wParam == VK_F1) {
                MessageBox(hwnd, "KChart Studio Help:\n\n- Mode button ('M'): switch chart views (Bar, Line, Area, Pie, Donut, Radar)\n- Theme button ('C'): switch color themes\n- Trend button ('T'): cycle Trendline & Regression overlays (Off, Linear Fit, Mov Avg, Mean Line)\n- Sort button ('S'): sort values\n- Randomize ('R'): generate new dataset\n- Hotkeys: 'H'/F1 Help, 'M' Mode, 'C' Theme, 'T' Trend, 'S' Sort, 'R' Randomize", "Help", MB_OK | MB_ICONINFORMATION);
            } else if (wParam == 'T' || wParam == 't') {
                trendMode = (trendMode + 1) % 4;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'M' || wParam == 'm') {
                chartMode = (chartMode + 1) % 6;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'C' || wParam == 'c') {
                currentTheme = (currentTheme + 1) % NUM_THEMES;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 'R' || wParam == 'r') {
                for (int i = 0; i < NUM_ITEMS; i++) {
                    target[i] = 10 + (MyRand() % 90);
                }
            } else if (wParam == 'S' || wParam == 's') {
                for (int i = 0; i < NUM_ITEMS - 1; i++) {
                    for (int j = 0; j < NUM_ITEMS - i - 1; j++) {
                        if (target[j] > target[j + 1]) {
                            int t = target[j]; target[j] = target[j + 1]; target[j + 1] = t;
                            int tv = values[j]; values[j] = values[j + 1]; values[j + 1] = tv;
                            const char* tl = labels[j]; labels[j] = labels[j + 1]; labels[j + 1] = tl;
                        }
                    }
                }
                CalculateStats();
                InvalidateRect(hwnd, NULL, TRUE);
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
            SetFocus(hwnd); // Ensure the main window keeps focus for keyboard shortcuts like 'H' / 'T'
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
            } else if (cmdId == 6) { // Toggle Trendline Overlay
                trendMode = (trendMode + 1) % 4;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (cmdId == 4) { // Sort
                for (int i = 0; i < NUM_ITEMS - 1; i++) {
                    for (int j = 0; j < NUM_ITEMS - i - 1; j++) {
                        if (target[j] > target[j + 1]) {
                            int t = target[j]; target[j] = target[j + 1]; target[j + 1] = t;
                            int tv = values[j]; values[j] = values[j + 1]; values[j + 1] = tv;
                            const char* tl = labels[j]; labels[j] = labels[j + 1]; labels[j + 1] = tl;
                        }
                    }
                }
                CalculateStats();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (cmdId == 5) { // Help
                MessageBox(hwnd, "KChart Studio Help:\n\n- Mode button ('M'): switch chart views (Bar, Line, Area, Pie, Donut, Radar)\n- Theme button ('C'): switch color themes\n- Trend button ('T'): cycle Trendline & Regression overlays (Off, Linear Fit, Mov Avg, Mean Line)\n- Sort button ('S'): sort values\n- Randomize ('R'): generate new dataset\n- Hotkeys: 'H'/F1 Help, 'M' Mode, 'C' Theme, 'T' Trend, 'S' Sort, 'R' Randomize", "Help", MB_OK | MB_ICONINFORMATION);
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

            int fontHeight = -MulDiv(12, dpi, 72);
            HFONT hFont = CreateFontA(fontHeight, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HGDIOBJ oldFont = SelectObject(memDC, hFont);
            SetBkMode(memDC, TRANSPARENT);

            // Title & Mode & Theme Header
            SetTextColor(memDC, RGB(244, 244, 245));
            char titleStr[128];
            wsprintfA(titleStr, "%s | %s | %s | Press 'H' Help / 'T' Trend", modeNames[chartMode], themeNames[currentTheme], trendNames[trendMode]);
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

                // Trendline & Regression Overlays in Cartesian Views
                if (trendMode == 1) { // Linear Trend
                    int x0 = chartX + spacing + barW / 2;
                    int x1 = chartX + spacing + (NUM_ITEMS - 1) * (barW + spacing) + barW / 2;
                    double yHat0 = trendSlope * 0.0 + trendIntercept;
                    double yHat1 = trendSlope * (double)(NUM_ITEMS - 1) + trendIntercept;
                    if (yHat0 < 0) yHat0 = 0;
                    if (yHat1 < 0) yHat1 = 0;
                    int y0 = chartY + chartH - (int)((yHat0 * chartH) / maxVal);
                    int y1 = chartY + chartH - (int)((yHat1 * chartH) / maxVal);

                    HPEN tPen = CreatePen(PS_SOLID, 2, RGB(250, 204, 21));
                    HGDIOBJ oldTPen = SelectObject(memDC, tPen);
                    MoveToEx(memDC, x0, y0, NULL);
                    LineTo(memDC, x1, y1);
                    SelectObject(memDC, oldTPen);
                    DeleteObject(tPen);

                    // Endpoints
                    HBRUSH tBrush = CreateSolidBrush(RGB(250, 204, 21));
                    HGDIOBJ oldTB = SelectObject(memDC, tBrush);
                    HPEN noPen = CreatePen(PS_NULL, 0, 0);
                    HGDIOBJ oldNP = SelectObject(memDC, noPen);
                    Ellipse(memDC, x0 - 4, y0 - 4, x0 + 4, y0 + 4);
                    Ellipse(memDC, x1 - 4, y1 - 4, x1 + 4, y1 + 4);
                    SelectObject(memDC, oldTB);
                    SelectObject(memDC, oldNP);
                    DeleteObject(tBrush);
                    DeleteObject(noPen);

                    // Formula Badge
                    char eqStr[64];
                    FormatTrendEq(eqStr, trendSlope, trendIntercept, trendR2);
                    RECT eqR = { chartX + chartW - 220, chartY + 6, chartX + chartW - 5, chartY + 24 };
                    HBRUSH bgR = CreateSolidBrush(RGB(24, 24, 30));
                    FillRect(memDC, &eqR, bgR);
                    DeleteObject(bgR);
                    SetTextColor(memDC, RGB(250, 204, 21));
                    DrawTextA(memDC, eqStr, -1, &eqR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                } else if (trendMode == 2) { // 3-Point Moving Average
                    POINT maPts[NUM_ITEMS];
                    for (int i = 0; i < NUM_ITEMS; i++) {
                        int start = i > 0 ? i - 1 : 0;
                        int end = i < NUM_ITEMS - 1 ? i + 1 : NUM_ITEMS - 1;
                        int sum = 0, count = 0;
                        for (int k = start; k <= end; k++) {
                            sum += values[k];
                            count++;
                        }
                        int maVal = sum / count;
                        maPts[i].x = chartX + spacing + i * (barW + spacing) + barW / 2;
                        maPts[i].y = chartY + chartH - (maVal * chartH) / maxVal;
                    }

                    HPEN maPen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
                    HGDIOBJ oldMAPen = SelectObject(memDC, maPen);
                    MoveToEx(memDC, maPts[0].x, maPts[0].y, NULL);
                    for (int i = 1; i < NUM_ITEMS; i++) {
                        LineTo(memDC, maPts[i].x, maPts[i].y);
                    }
                    SelectObject(memDC, oldMAPen);
                    DeleteObject(maPen);

                    // Dots
                    for (int i = 0; i < NUM_ITEMS; i++) {
                        HBRUSH ptBrush = CreateSolidBrush(RGB(56, 189, 248));
                        HGDIOBJ oldBrush = SelectObject(memDC, ptBrush);
                        HPEN noPen = CreatePen(PS_NULL, 0, 0);
                        HGDIOBJ oldP = SelectObject(memDC, noPen);
                        Ellipse(memDC, maPts[i].x - 3, maPts[i].y - 3, maPts[i].x + 3, maPts[i].y + 3);
                        SelectObject(memDC, oldBrush);
                        SelectObject(memDC, oldP);
                        DeleteObject(ptBrush);
                        DeleteObject(noPen);
                    }

                    RECT maR = { chartX + chartW - 190, chartY + 6, chartX + chartW - 5, chartY + 24 };
                    HBRUSH bgR = CreateSolidBrush(RGB(24, 24, 30));
                    FillRect(memDC, &maR, bgR);
                    DeleteObject(bgR);
                    SetTextColor(memDC, RGB(56, 189, 248));
                    DrawTextA(memDC, "MovAvg (Window: 3)", -1, &maR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                } else if (trendMode == 3) { // Mean Line
                    int yMean = chartY + chartH - (statMean * chartH) / maxVal;
                    HPEN meanPen = CreatePen(PS_SOLID, 2, RGB(236, 72, 153));
                    HGDIOBJ oldMPen = SelectObject(memDC, meanPen);
                    MoveToEx(memDC, chartX, yMean, NULL);
                    LineTo(memDC, chartX + chartW, yMean);
                    SelectObject(memDC, oldMPen);
                    DeleteObject(meanPen);

                    char mStr[32];
                    wsprintfA(mStr, "Mean Line: %d", statMean);
                    RECT mR = { chartX + chartW - 140, yMean - 18, chartX + chartW - 5, yMean - 2 };
                    HBRUSH bgR = CreateSolidBrush(RGB(24, 24, 30));
                    FillRect(memDC, &mR, bgR);
                    DeleteObject(bgR);
                    SetTextColor(memDC, RGB(236, 72, 153));
                    DrawTextA(memDC, mStr, -1, &mR, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
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
