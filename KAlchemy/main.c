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

#define TOTAL_ELEMENTS 20
#define TOTAL_RECIPES 16

typedef struct {
    int id;
    const char* name;
    int isBasic;
} Element;

static const Element g_Elements[TOTAL_ELEMENTS] = {
    { 0, "Fire", 1 },
    { 1, "Water", 1 },
    { 2, "Earth", 1 },
    { 3, "Air", 1 },
    { 4, "Steam", 0 },
    { 5, "Lava", 0 },
    { 6, "Energy", 0 },
    { 7, "Mud", 0 },
    { 8, "Rain", 0 },
    { 9, "Dust", 0 },
    { 10, "Obsidian", 0 },
    { 11, "Stone", 0 },
    { 12, "Plant", 0 },
    { 13, "Cloud", 0 },
    { 14, "Gunpowder", 0 },
    { 15, "Life", 0 },
    { 16, "Charcoal", 0 },
    { 17, "Swamps", 0 },
    { 18, "Metal", 0 },
    { 19, "Golem", 0 }
};

typedef struct {
    int ingredient1;
    int ingredient2;
    int result;
} Recipe;

static const Recipe g_Recipes[TOTAL_RECIPES] = {
    { 0, 1, 4 },   // Fire + Water -> Steam
    { 0, 2, 5 },   // Fire + Earth -> Lava
    { 0, 3, 6 },   // Fire + Air -> Energy
    { 1, 2, 7 },   // Water + Earth -> Mud
    { 1, 3, 8 },   // Water + Air -> Rain
    { 2, 3, 9 },   // Earth + Air -> Dust
    { 5, 1, 10 },  // Lava + Water -> Obsidian
    { 5, 3, 11 },  // Lava + Air -> Stone
    { 2, 8, 12 },  // Earth + Rain -> Plant
    { 3, 8, 13 },  // Air + Rain -> Cloud
    { 0, 9, 14 },  // Fire + Dust -> Gunpowder
    { 6, 1, 15 },  // Energy + Water -> Life
    { 0, 12, 16 }, // Fire + Plant -> Charcoal
    { 7, 12, 17 }, // Mud + Plant -> Swamps
    { 11, 0, 18 }, // Stone + Fire -> Metal
    { 15, 18, 19 } // Life + Metal -> Golem
};

typedef struct {
    int discovered[TOTAL_ELEMENTS];
    int discoveredCount;
    int essence;
    int slot1;
    int slot2;
    char lastStatus[128];
    char journalLog[1024];
} AlchemyState;

static AlchemyState g_State;
static HWND g_hElemButtons[TOTAL_ELEMENTS];
static HWND g_hSlot1Button = NULL;
static HWND g_hSlot2Button = NULL;

static HBRUSH hBgBrush = NULL;
static HBRUSH hPanelBrush = NULL;
static HBRUSH hCrucibleBrush = NULL;
static HFONT hTitleFont = NULL;
static HFONT hHeaderFont = NULL;
static HFONT hUIFont = NULL;
static HFONT hSlotFont = NULL;

static void InitGameState() {
    memset(&g_State, 0, sizeof(AlchemyState));
    g_State.discovered[0] = 1; // Fire
    g_State.discovered[1] = 1; // Water
    g_State.discovered[2] = 1; // Earth
    g_State.discovered[3] = 1; // Air
    g_State.discoveredCount = 4;
    g_State.essence = 100;
    g_State.slot1 = -1;
    g_State.slot2 = -1;
    lstrcpyA(g_State.lastStatus, "Transmutation Crucible Ready");
    lstrcpyA(g_State.journalLog, "Welcome Apprentice Alchemist!\r\nSelect elements from your Grimoire to combine in the Crucible.");
}

static void UpdateSlotButtonText() {
    if (g_hSlot1Button) {
        if (g_State.slot1 >= 0 && g_State.slot1 < TOTAL_ELEMENTS) {
            SetWindowTextA(g_hSlot1Button, g_Elements[g_State.slot1].name);
        } else {
            SetWindowTextA(g_hSlot1Button, "[ Slot 1 ]");
        }
    }
    if (g_hSlot2Button) {
        if (g_State.slot2 >= 0 && g_State.slot2 < TOTAL_ELEMENTS) {
            SetWindowTextA(g_hSlot2Button, g_Elements[g_State.slot2].name);
        } else {
            SetWindowTextA(g_hSlot2Button, "[ Slot 2 ]");
        }
    }
}

static void UpdateElementButtonsVisibility() {
    for (int i = 0; i < TOTAL_ELEMENTS; i++) {
        if (g_hElemButtons[i]) {
            ShowWindow(g_hElemButtons[i], g_State.discovered[i] ? SW_SHOW : SW_HIDE);
        }
    }
}

static void PlayDiscoveryFanfare() {
    Beep(523, 70);  // C5
    Beep(659, 70);  // E5
    Beep(784, 70);  // G5
    Beep(1046, 120); // C6
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            InitGameState();

            hBgBrush = CreateSolidBrush(RGB(13, 15, 27));
            hPanelBrush = CreateSolidBrush(RGB(22, 24, 44));
            hCrucibleBrush = CreateSolidBrush(RGB(28, 24, 54));
            hTitleFont = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hHeaderFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hUIFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hSlotFont = CreateFontA(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

            // Element Grimoire Buttons (2 columns of 10)
            for (int i = 0; i < TOTAL_ELEMENTS; i++) {
                int col = i % 2;
                int row = i / 2;
                int x = 32 + col * 112;
                int y = 115 + row * 38;
                g_hElemButtons[i] = CreateWindowA("BUTTON", g_Elements[i].name,
                    WS_CHILD | BS_PUSHBUTTON,
                    x, y, 105, 34, hwnd, (HMENU)(UINT_PTR)(100 + i), NULL, NULL);
            }
            UpdateElementButtonsVisibility();

            // Crucible Slots
            g_hSlot1Button = CreateWindowA("BUTTON", "[ Slot 1 ]", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 300, 140, 90, 60, hwnd, (HMENU)301, NULL, NULL);
            g_hSlot2Button = CreateWindowA("BUTTON", "[ Slot 2 ]", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 400, 140, 90, 60, hwnd, (HMENU)302, NULL, NULL);

            // Action Buttons
            CreateWindowA("BUTTON", "✨ Transmute", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 320, 230, 150, 42, hwnd, (HMENU)201, NULL, NULL);
            CreateWindowA("BUTTON", "Clear Crucible", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 335, 285, 120, 30, hwnd, (HMENU)202, NULL, NULL);
            CreateWindowA("BUTTON", "Reset Progress", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 335, 325, 120, 26, hwnd, (HMENU)203, NULL, NULL);
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);

            // Element selection (IDs 100 to 119)
            if (id >= 100 && id < 100 + TOTAL_ELEMENTS) {
                int elemIdx = id - 100;
                if (g_State.discovered[elemIdx]) {
                    if (g_State.slot1 == -1) {
                        g_State.slot1 = elemIdx;
                    } else if (g_State.slot2 == -1) {
                        g_State.slot2 = elemIdx;
                    } else {
                        g_State.slot1 = elemIdx; // override slot 1
                    }
                    UpdateSlotButtonText();
                    Beep(520, 60);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            // Clear Slot 1
            else if (id == 301) {
                g_State.slot1 = -1;
                UpdateSlotButtonText();
                Beep(350, 50);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Clear Slot 2
            else if (id == 302) {
                g_State.slot2 = -1;
                UpdateSlotButtonText();
                Beep(350, 50);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Transmute action
            else if (id == 201) {
                if (g_State.slot1 < 0 || g_State.slot2 < 0) {
                    lstrcpyA(g_State.lastStatus, "Select 2 elements for Crucible!");
                    Beep(220, 100);
                } else {
                    int e1 = g_State.slot1;
                    int e2 = g_State.slot2;
                    int matchIdx = -1;

                    for (int r = 0; r < TOTAL_RECIPES; r++) {
                        if ((g_Recipes[r].ingredient1 == e1 && g_Recipes[r].ingredient2 == e2) ||
                            (g_Recipes[r].ingredient1 == e2 && g_Recipes[r].ingredient2 == e1)) {
                            matchIdx = r;
                            break;
                        }
                    }

                    if (matchIdx >= 0) {
                        int res = g_Recipes[matchIdx].result;
                        if (!g_State.discovered[res]) {
                            g_State.discovered[res] = 1;
                            g_State.discoveredCount++;
                            g_State.essence += 50;
                            UpdateElementButtonsVisibility();

                            wsprintfA(g_State.lastStatus, "DISCOVERY! Created %s!", g_Elements[res].name);
                            wsprintfA(g_State.journalLog, "✨ NEW DISCOVERY! Combined %s + %s -> %s! (+50 Essence)\r\n\r\nTotal Discovered: %d / %d",
                                g_Elements[e1].name, g_Elements[e2].name, g_Elements[res].name,
                                g_State.discoveredCount, TOTAL_ELEMENTS);

                            PlayDiscoveryFanfare();
                        } else {
                            wsprintfA(g_State.lastStatus, "Created %s (Known)", g_Elements[res].name);
                            wsprintfA(g_State.journalLog, "Created %s by combining %s + %s. Already in Grimoire.",
                                g_Elements[res].name, g_Elements[e1].name, g_Elements[e2].name);
                            Beep(659, 120);
                        }
                    } else {
                        wsprintfA(g_State.lastStatus, "Reaction Fizzled!");
                        wsprintfA(g_State.journalLog, "Reaction Fizzled! No combination for %s + %s.",
                            g_Elements[e1].name, g_Elements[e2].name);
                        Beep(180, 150);
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Clear Crucible
            else if (id == 202) {
                g_State.slot1 = -1;
                g_State.slot2 = -1;
                UpdateSlotButtonText();
                lstrcpyA(g_State.lastStatus, "Crucible Cleared");
                Beep(300, 60);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Reset Progress
            else if (id == 203) {
                if (MessageBoxA(hwnd, "Reset all discovered elements back to starter 4?", "Reset KAlchemy", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    InitGameState();
                    UpdateSlotButtonText();
                    UpdateElementButtonsVisibility();
                    Beep(400, 100);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            SetBkMode(hdc, TRANSPARENT);

            // Top Header Bar
            RECT headerRect = { 20, 12, 765, 62 };
            FillRect(hdc, &headerRect, hPanelBrush);
            FrameRect(hdc, &headerRect, (HBRUSH)GetStockObject(GRAY_BRUSH));

            SelectObject(hdc, hTitleFont);
            SetTextColor(hdc, RGB(243, 156, 18));
            TextOutA(hdc, 35, 23, "KAlchemy - Element Discovery Lab", 32);

            SelectObject(hdc, hUIFont);
            char statsStr[128];
            const char* rankStr = "Apprentice";
            if (g_State.discoveredCount >= 20) rankStr = "Grand Master";
            else if (g_State.discoveredCount >= 15) rankStr = "Master";
            else if (g_State.discoveredCount >= 10) rankStr = "Journeyman";
            else if (g_State.discoveredCount >= 6) rankStr = "Adept";

            wsprintfA(statsStr, "Discovered: %d/%d  |  Essence: %d  |  Rank: %s", 
                g_State.discoveredCount, TOTAL_ELEMENTS, g_State.essence, rankStr);
            SetTextColor(hdc, RGB(160, 220, 255));
            TextOutA(hdc, 430, 26, statsStr, lstrlenA(statsStr));

            // Left Panel - Element Grimoire
            RECT leftPanel = { 20, 72, 265, 525 };
            FillRect(hdc, &leftPanel, hPanelBrush);
            FrameRect(hdc, &leftPanel, (HBRUSH)GetStockObject(GRAY_BRUSH));

            SelectObject(hdc, hHeaderFont);
            SetTextColor(hdc, RGB(243, 156, 18));
            TextOutA(hdc, 35, 82, "Elemental Grimoire", 18);

            // Center Panel - Transmutation Crucible
            RECT centerPanel = { 280, 72, 505, 525 };
            FillRect(hdc, &centerPanel, hCrucibleBrush);
            FrameRect(hdc, &centerPanel, (HBRUSH)GetStockObject(GRAY_BRUSH));

            SelectObject(hdc, hHeaderFont);
            SetTextColor(hdc, RGB(243, 156, 18));
            TextOutA(hdc, 305, 82, "Transmutation Crucible", 22);

            // Status message centered below crucible buttons
            SelectObject(hdc, hUIFont);
            SetTextColor(hdc, RGB(120, 230, 180));
            RECT statusRect = { 290, 365, 495, 410 };
            DrawTextA(hdc, g_State.lastStatus, -1, &statusRect, DT_CENTER | DT_WORDBREAK);

            // Right Panel - Journal & Log
            RECT rightPanel = { 520, 72, 765, 525 };
            FillRect(hdc, &rightPanel, hPanelBrush);
            FrameRect(hdc, &rightPanel, (HBRUSH)GetStockObject(GRAY_BRUSH));

            SelectObject(hdc, hHeaderFont);
            SetTextColor(hdc, RGB(243, 156, 18));
            TextOutA(hdc, 535, 82, "Alchemist's Journal", 19);

            SelectObject(hdc, hUIFont);
            SetTextColor(hdc, RGB(200, 210, 230));
            RECT logRect = { 535, 115, 750, 500 };
            DrawTextA(hdc, g_State.journalLog, -1, &logRect, DT_LEFT | DT_WORDBREAK);

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
            if (hSlotFont) DeleteObject(hSlotFont);
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
                               CW_USEDEFAULT, CW_USEDEFAULT, 800, 570,
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
