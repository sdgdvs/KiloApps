#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TIMER_ID 1
#define TIMER_INTERVAL 33 // ~30 FPS

// Colors
#define BG_COLOR RGB(20, 24, 33)
#define CARD_BG RGB(32, 38, 52)
#define BORDER_COLOR RGB(60, 70, 95)
#define TEXT_GOLD RGB(255, 215, 0)
#define TEXT_WHITE RGB(235, 240, 245)
#define TEXT_MUTED RGB(140, 150, 175)
#define PATH_COLOR RGB(45, 52, 70)
#define PATH_BORDER RGB(75, 85, 110)
#define CASTLE_COLOR RGB(100, 110, 130)
#define TOWER_SLOT RGB(40, 50, 70)
#define TOWER_HOVER RGB(60, 80, 110)

typedef struct {
    int x, y;
    BOOL occupied;
    int towerType; // 0 = none, 1 = archer, 2 = mage, 3 = cannon
} TowerSlot;

// Global State
static TowerSlot g_slots[12];
static int g_slotCount = 0;
static int g_gold = 100;
static int g_baseHp = 20;
static int g_maxBaseHp = 20;
static int g_wave = 1;
static BOOL g_waveActive = FALSE;
static int g_selectedSlot = -1;

void InitGameState() {
    g_gold = 100;
    g_baseHp = 20;
    g_maxBaseHp = 20;
    g_wave = 1;
    g_waveActive = FALSE;
    g_selectedSlot = -1;

    // Define 12 tower placement slots around the path
    int coords[12][2] = {
        {120, 100}, {240, 100}, {360, 100}, {480, 100},
        {120, 280}, {240, 280}, {360, 280}, {480, 280},
        {120, 440}, {240, 440}, {360, 440}, {480, 440}
    };
    g_slotCount = 12;
    for (int i = 0; i < 12; i++) {
        g_slots[i].x = coords[i][0];
        g_slots[i].y = coords[i][1];
        g_slots[i].occupied = FALSE;
        g_slots[i].towerType = 0;
    }
}

void DrawRoundedRect(HDC hdc, int left, int top, int right, int bottom, COLORREF fillColor, COLORREF borderColor, int radius) {
    HBRUSH fillBrush = CreateSolidBrush(fillColor);
    HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    RoundRect(hdc, left, top, right, bottom, radius, radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(fillBrush);
    DeleteObject(pen);
}

void Render(HDC hdc, HWND hwnd) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int w = clientRect.right;
    int h = clientRect.bottom;

    // Create Backbuffer
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // Background
    HBRUSH bgBrush = CreateSolidBrush(BG_COLOR);
    FillRect(memDC, &clientRect, bgBrush);
    DeleteObject(bgBrush);

    SetBkMode(memDC, TRANSPARENT);

    // Top Header / HUD Bar
    DrawRoundedRect(memDC, 10, 10, w - 10, 60, CARD_BG, BORDER_COLOR, 8);

    HFONT hFontTitle = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(memDC, hFontTitle);

    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, 25, 22, "KFORTRESS", 9);

    HFONT hFontSub = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontSub);
    SetTextColor(memDC, TEXT_MUTED);
    TextOutA(memDC, 140, 26, "Fantasy Tower Defense & Siege Defense", 37);

    // Stats HUD
    char buf[128];
    HFONT hFontStat = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontStat);

    SetTextColor(memDC, TEXT_GOLD);
    sprintf(buf, "Gold: %d", g_gold);
    TextOutA(memDC, w - 320, 24, buf, (int)strlen(buf));

    SetTextColor(memDC, RGB(239, 68, 68));
    sprintf(buf, "Base HP: %d/%d", g_baseHp, g_maxBaseHp);
    TextOutA(memDC, w - 210, 24, buf, (int)strlen(buf));

    SetTextColor(memDC, TEXT_WHITE);
    sprintf(buf, "Wave: %d", g_wave);
    TextOutA(memDC, w - 90, 24, buf, (int)strlen(buf));

    DeleteObject(hFontTitle);
    DeleteObject(hFontSub);
    DeleteObject(hFontStat);

    // Battlefield Area
    int bfX = 10, bfY = 70, bfW = w - 220, bfH = h - 80;
    DrawRoundedRect(memDC, bfX, bfY, bfX + bfW, bfY + bfH, CARD_BG, BORDER_COLOR, 8);

    // Draw Winding Path
    POINT pathPoints[] = {
        {bfX + 30, bfY + 180},
        {bfX + 280, bfY + 180},
        {bfX + 280, bfY + 360},
        {bfX + 480, bfY + 360},
        {bfX + 480, bfY + 200},
        {bfX + bfW - 60, bfY + 200}
    };
    HPEN pathPen = CreatePen(PS_SOLID, 40, PATH_COLOR);
    HPEN oldPen = (HPEN)SelectObject(memDC, pathPen);
    Polyline(memDC, pathPoints, 6);

    HPEN pathBorderPen = CreatePen(PS_SOLID, 2, PATH_BORDER);
    SelectObject(memDC, pathBorderPen);
    Polyline(memDC, pathPoints, 6);
    SelectObject(memDC, oldPen);
    DeleteObject(pathPen);
    DeleteObject(pathBorderPen);

    // Draw Spawn Gate
    DrawRoundedRect(memDC, bfX + 10, bfY + 155, bfX + 50, bfY + 205, RGB(180, 60, 60), RGB(220, 80, 80), 6);
    SelectObject(memDC, oldFont);
    SetTextColor(memDC, TEXT_WHITE);
    TextOutA(memDC, bfX + 16, bfY + 172, "GATE", 4);

    // Draw Castle Fortress Base
    DrawRoundedRect(memDC, bfX + bfW - 80, bfY + 165, bfX + bfW - 10, bfY + 235, CASTLE_COLOR, TEXT_GOLD, 8);
    TextOutA(memDC, bfX + bfW - 75, bfY + 192, "CASTLE", 6);

    // Draw Tower Slots
    for (int i = 0; i < g_slotCount; i++) {
        COLORREF fill = (g_selectedSlot == i) ? TOWER_HOVER : TOWER_SLOT;
        COLORREF border = (g_selectedSlot == i) ? TEXT_GOLD : BORDER_COLOR;
        DrawRoundedRect(memDC, g_slots[i].x - 22, g_slots[i].y - 22, g_slots[i].x + 22, g_slots[i].y + 22, fill, border, 6);

        if (g_slots[i].occupied) {
            HBRUSH tBrush = CreateSolidBrush(TEXT_GOLD);
            HBRUSH oB = (HBRUSH)SelectObject(memDC, tBrush);
            Ellipse(memDC, g_slots[i].x - 12, g_slots[i].y - 12, g_slots[i].x + 12, g_slots[i].y + 12);
            SelectObject(memDC, oB);
            DeleteObject(tBrush);
        } else {
            SelectObject(memDC, oldFont);
            SetTextColor(memDC, TEXT_MUTED);
            TextOutA(memDC, g_slots[i].x - 4, g_slots[i].y - 8, "+", 1);
        }
    }

    // Right Control Sidebar
    int sbX = w - 200, sbY = 70, sbW = 190, sbH = h - 80;
    DrawRoundedRect(memDC, sbX, sbY, sbX + sbW, sbY + sbH, CARD_BG, BORDER_COLOR, 8);

    HFONT hFontHeader = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontHeader);
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, sbX + 15, sbY + 15, "COMMAND POST", 12);

    // Button: Start Wave
    COLORREF btnBg = g_waveActive ? RGB(60, 70, 85) : RGB(16, 185, 129);
    DrawRoundedRect(memDC, sbX + 15, sbY + 50, sbX + sbW - 15, sbY + 95, btnBg, BORDER_COLOR, 6);
    SetTextColor(memDC, TEXT_WHITE);
    TextOutA(memDC, sbX + 35, sbY + 63, g_waveActive ? "WAVE IN PROGRESS" : "START WAVE 1", g_waveActive ? 16 : 12);

    // Button: Reset Game
    DrawRoundedRect(memDC, sbX + 15, sbY + 110, sbX + sbW - 15, sbY + 150, RGB(225, 29, 72), BORDER_COLOR, 6);
    TextOutA(memDC, sbX + 45, sbY + 122, "RESET DEFENSE", 13);

    // Info Box
    DrawRoundedRect(memDC, sbX + 15, sbY + 170, sbX + sbW - 15, sbY + sbH - 15, BG_COLOR, BORDER_COLOR, 6);
    HFONT hFontBody = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontBody);
    SetTextColor(memDC, TEXT_MUTED);

    TextOutA(memDC, sbX + 25, sbY + 185, "Phase 1 Scaffold:", 17);
    TextOutA(memDC, sbX + 25, sbY + 210, "- Click + slots", 15);
    TextOutA(memDC, sbX + 25, sbY + 230, "- Prepare defenses", 18);
    TextOutA(memDC, sbX + 25, sbY + 250, "- Defend the Castle!", 20);

    DeleteObject(hFontHeader);
    DeleteObject(hFontBody);

    // Copy Backbuffer to Window DC
    BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

    // Cleanup
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        InitGameState();
        SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
        break;

    case WM_TIMER:
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        int w = clientRect.right;
        int h = clientRect.bottom;
        int sbX = w - 200, sbY = 70, sbW = 190;

        // Check button clicks
        // Start Wave
        if (x >= sbX + 15 && x <= sbX + sbW - 15 && y >= sbY + 50 && y <= sbY + 95) {
            g_waveActive = !g_waveActive;
            Beep(600, 40);
        }
        // Reset Defense
        else if (x >= sbX + 15 && x <= sbX + sbW - 15 && y >= sbY + 110 && y <= sbY + 150) {
            InitGameState();
            Beep(300, 60);
        }
        // Check slot clicks
        else {
            g_selectedSlot = -1;
            for (int i = 0; i < g_slotCount; i++) {
                int dx = x - g_slots[i].x;
                int dy = y - g_slots[i].y;
                if (dx * dx + dy * dy <= 22 * 22) {
                    g_selectedSlot = i;
                    g_slots[i].occupied = !g_slots[i].occupied;
                    Beep(800, 30);
                    break;
                }
            }
        }
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Render(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_ERASEBKGND:
        return 1; // Prevent flicker

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KFortressWindowClass";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "KFortress - Fantasy Tower Defense & Siege Defense",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
