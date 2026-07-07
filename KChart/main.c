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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            randSeed = GetTickCount();
            hBtnRandomize = CreateWindowEx(0, "BUTTON", "Randomize Data",
                WS_CHILD | WS_VISIBLE,
                W / 2 - 60, H - 70, 120, 24, hwnd, (HMENU)1, NULL, NULL);
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            SendMessage(hBtnRandomize, WM_SETFONT, (WPARAM)hFont, TRUE);
            SetTimer(hwnd, 1, 16, NULL); // Animation timer
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
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Double buffer
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, W, H);
            SelectObject(memDC, memBM);
            
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
            
            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
            
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
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
