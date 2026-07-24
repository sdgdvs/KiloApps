#include <windows.h>

#pragma function(memset, memcpy)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* p = (char*)dest;
    while (count--) *p++ = (char)c;
    return dest;
}

void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

// Game State Definitions (Scaffold Phase 1)
typedef struct {
    int discoveredCount;
    int totalElements;
    int gold;
    int essence;
    char lastTransmutation[64];
    char statusLog[512];
} AlchemyState;

static AlchemyState g_State = {
    4, 100, 50, 100,
    "Transmutation Crucible Ready",
    "Welcome to KAlchemy Laboratory! Select elements to combine."
};

static HBRUSH hBgBrush = NULL;
static HBRUSH hPanelBrush = NULL;
static HBRUSH hCrucibleBrush = NULL;
static HFONT hTitleFont = NULL;
static HFONT hHeaderFont = NULL;
static HFONT hUIFont = NULL;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hBgBrush = CreateSolidBrush(RGB(18, 18, 28));
            hPanelBrush = CreateSolidBrush(RGB(28, 30, 48));
            hCrucibleBrush = CreateSolidBrush(RGB(40, 32, 60));
            hTitleFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Georgia");
            hHeaderFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hUIFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            
            // Buttons scaffolding
            CreateWindowA("BUTTON", "🔥 Fire", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 25, 110, 100, 36, hwnd, (HMENU)101, NULL, NULL);
            CreateWindowA("BUTTON", "💧 Water", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 25, 155, 100, 36, hwnd, (HMENU)102, NULL, NULL);
            CreateWindowA("BUTTON", "🌱 Earth", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 25, 200, 100, 36, hwnd, (HMENU)103, NULL, NULL);
            CreateWindowA("BUTTON", "💨 Air", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 25, 245, 100, 36, hwnd, (HMENU)104, NULL, NULL);

            CreateWindowA("BUTTON", "✨ Transmute", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 330, 260, 120, 40, hwnd, (HMENU)201, NULL, NULL);
            CreateWindowA("BUTTON", "🧹 Clear Crucible", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 330, 310, 120, 30, hwnd, (HMENU)202, NULL, NULL);
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id >= 101 && id <= 104) {
                const char* names[] = { "Fire", "Water", "Earth", "Air" };
                wsprintfA(g_State.lastTransmutation, "Selected element: %s", names[id - 101]);
                Beep(587, 80); // D5 chime sound effect
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (id == 201) {
                lstrcpyA(g_State.lastTransmutation, "Crucible Transmutation simulated!");
                Beep(880, 120); // A5 magic sound effect
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (id == 202) {
                lstrcpyA(g_State.lastTransmutation, "Crucible Cleared");
                Beep(440, 80);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            SetBkMode(hdc, TRANSPARENT);

            // Title & Status Header
            RECT headerRect = { 20, 15, 760, 60 };
            FillRect(hdc, &headerRect, hPanelBrush);
            FrameRect(hdc, &headerRect, (HBRUSH)GetStockObject(GRAY_BRUSH));

            SelectObject(hdc, hTitleFont);
            SetTextColor(hdc, RGB(240, 195, 60));
            TextOutA(hdc, 35, 24, "🧪 KAlchemy - Fantasy Laboratory", 33);

            SelectObject(hdc, hUIFont);
            char statsStr[128];
            wsprintfA(statsStr, "Discovered: %d/%d  |  Essence: %d  |  Gold: %d", 
                g_State.discoveredCount, g_State.totalElements, g_State.essence, g_State.gold);
            SetTextColor(hdc, RGB(160, 220, 255));
            TextOutA(hdc, 480, 28, statsStr, lstrlenA(statsStr));

            // Left Panel - Element Library
            RECT leftPanel = { 20, 75, 260, 520 };
            FillRect(hdc, &leftPanel, hPanelBrush);
            FrameRect(hdc, &leftPanel, (HBRUSH)GetStockObject(GRAY_BRUSH));
            SelectObject(hdc, hHeaderFont);
            SetTextColor(hdc, RGB(220, 220, 250));
            TextOutA(hdc, 35, 85, "Elemental Grimoire", 18);

            // Center Panel - Transmutation Crucible
            RECT centerPanel = { 275, 75, 510, 520 };
            FillRect(hdc, &centerPanel, hCrucibleBrush);
            FrameRect(hdc, &centerPanel, (HBRUSH)GetStockObject(GRAY_BRUSH));
            SelectObject(hdc, hHeaderFont);
            SetTextColor(hdc, RGB(255, 215, 100));
            TextOutA(hdc, 310, 85, "Transmutation Crucible", 22);

            // Crucible Slots (Scaffold)
            RECT slot1 = { 300, 130, 380, 210 };
            RECT slot2 = { 405, 130, 485, 210 };
            FillRect(hdc, &slot1, hPanelBrush);
            FillRect(hdc, &slot2, hPanelBrush);
            FrameRect(hdc, &slot1, (HBRUSH)GetStockObject(WHITE_BRUSH));
            FrameRect(hdc, &slot2, (HBRUSH)GetStockObject(WHITE_BRUSH));

            SelectObject(hdc, hUIFont);
            SetTextColor(hdc, RGB(150, 150, 180));
            TextOutA(hdc, 315, 160, "Slot 1", 6);
            TextOutA(hdc, 420, 160, "Slot 2", 6);

            // Status message
            SetTextColor(hdc, RGB(120, 230, 180));
            TextOutA(hdc, 290, 360, g_State.lastTransmutation, lstrlenA(g_State.lastTransmutation));

            // Right Panel - Journal & Log
            RECT rightPanel = { 525, 75, 765, 520 };
            FillRect(hdc, &rightPanel, hPanelBrush);
            FrameRect(hdc, &rightPanel, (HBRUSH)GetStockObject(GRAY_BRUSH));
            SelectObject(hdc, hHeaderFont);
            SetTextColor(hdc, RGB(220, 220, 250));
            TextOutA(hdc, 540, 85, "Alchemist's Log", 15);

            SelectObject(hdc, hUIFont);
            SetTextColor(hdc, RGB(180, 180, 200));
            TextOutA(hdc, 540, 120, g_State.statusLog, lstrlenA(g_State.statusLog));

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, hBgBrush);
            return 1;
        }

        case WM_DESTROY:
            if (hBgBrush) DeleteObject(hBgBrush);
            if (hPanelBrush) DeleteObject(hPanelBrush);
            if (hCrucibleBrush) DeleteObject(hCrucibleBrush);
            if (hTitleFont) DeleteObject(hTitleFont);
            if (hHeaderFont) DeleteObject(hHeaderFont);
            if (hUIFont) DeleteObject(hUIFont);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = NULL;
    wc.lpszClassName = "KAlchemyClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA("KAlchemyClass", "KAlchemy - Fantasy Crafting & Element Discovery",
                              WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 580,
                              NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}

void MainEntry() {
    int ret = WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT);
    ExitProcess(ret);
}
