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
    char searchFilter[64];
} AlchemyState;

static AlchemyState g_State;
static HWND g_hElemButtons[TOTAL_ELEMENTS];
static HWND g_hSlot1Button = NULL;
static HWND g_hSlot2Button = NULL;
static HWND g_hSearchEdit = NULL;
static HWND g_hJournalEdit = NULL;

static HBRUSH hBgBrush = NULL;
static HBRUSH hPanelBrush = NULL;
static HBRUSH hCrucibleBrush = NULL;
static HBRUSH hVesselBrush = NULL;
static HBRUSH hGoldBadgeBrush = NULL;
static HPEN hVesselPen = NULL;
static HPEN hGoldPen = NULL;
static HPEN hPurplePen = NULL;
static HPEN hInnerGlowPen = NULL;
static HFONT hTitleFont = NULL;
static HFONT hHeaderFont = NULL;
static HFONT hUIFont = NULL;
static HFONT hSlotFont = NULL;
static HFONT hBadgeFont = NULL;

static void AddJournalLog(const char* text) {
    if (!g_hJournalEdit) return;
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[512];
    wsprintfA(buf, "[%02d:%02d:%02d] %s\r\n\r\n", st.wHour, st.wMinute, st.wSecond, text);

    int len = GetWindowTextLengthA(g_hJournalEdit);
    SendMessageA(g_hJournalEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageA(g_hJournalEdit, EM_REPLACESEL, FALSE, (LPARAM)buf);
    SendMessageA(g_hJournalEdit, EM_SCROLLCARET, 0, 0);
}

static void ToLowerStr(char* dest, const char* src) {
    while (*src) {
        char c = *src++;
        if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
        *dest++ = c;
    }
    *dest = '\0';
}

static int StrContainsIgnoreCase(const char* haystack, const char* needle) {
    if (!needle || !needle[0]) return 1;
    char hLower[128], nLower[128];
    ToLowerStr(hLower, haystack);
    ToLowerStr(nLower, needle);

    for (int i = 0; hLower[i] != '\0'; i++) {
        int j = 0;
        while (hLower[i + j] != '\0' && nLower[j] != '\0' && hLower[i + j] == nLower[j]) {
            j++;
        }
        if (nLower[j] == '\0') return 1;
    }
    return 0;
}

static void UpdateElementButtonsVisibility() {
    for (int i = 0; i < TOTAL_ELEMENTS; i++) {
        if (g_hElemButtons[i]) {
            int show = g_State.discovered[i];
            if (show && g_State.searchFilter[0] != '\0') {
                show = StrContainsIgnoreCase(g_Elements[i].name, g_State.searchFilter);
            }
            ShowWindow(g_hElemButtons[i], show ? SW_SHOW : SW_HIDE);
        }
    }
}

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
    g_State.searchFilter[0] = '\0';
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

static void PlayDiscoveryFanfare() {
    Beep(523, 70);   // C5
    Beep(659, 70);   // E5
    Beep(784, 70);   // G5
    Beep(1046, 120); // C6
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            InitGameState();

            hBgBrush = CreateSolidBrush(RGB(9, 9, 20));          // Deep violet/slate
            hPanelBrush = CreateSolidBrush(RGB(22, 21, 46));       // Mystical panel background
            hCrucibleBrush = CreateSolidBrush(RGB(29, 24, 61));    // Arcane laboratory crucible
            hVesselBrush = CreateSolidBrush(RGB(55, 38, 95));      // Arcane purple vessel
            hGoldBadgeBrush = CreateSolidBrush(RGB(45, 35, 12));   // Golden badge fill
            hVesselPen = CreatePen(PS_DASH, 2, RGB(155, 89, 182));
            hPurplePen = CreatePen(PS_SOLID, 2, RGB(155, 89, 182));
            hGoldPen = CreatePen(PS_SOLID, 2, RGB(243, 156, 18));
            hInnerGlowPen = CreatePen(PS_SOLID, 1, RGB(241, 196, 15));

            hTitleFont = CreateFontA(21, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hHeaderFont = CreateFontA(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hUIFont = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hSlotFont = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hBadgeFont = CreateFontA(12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

            // Grimoire Search Input
            g_hSearchEdit = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                30, 96, 225, 24, hwnd, (HMENU)401, NULL, NULL);

            // Element Grimoire Buttons (2 columns of 10)
            for (int i = 0; i < TOTAL_ELEMENTS; i++) {
                int col = i % 2;
                int row = i / 2;
                int x = 30 + col * 115;
                int y = 128 + row * 35;
                g_hElemButtons[i] = CreateWindowA("BUTTON", g_Elements[i].name,
                    WS_CHILD | BS_PUSHBUTTON,
                    x, y, 110, 31, hwnd, (HMENU)(UINT_PTR)(100 + i), NULL, NULL);
            }
            UpdateElementButtonsVisibility();

            // Crucible Slots
            g_hSlot1Button = CreateWindowA("BUTTON", "[ Slot 1 ]", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 295, 200, 90, 50, hwnd, (HMENU)301, NULL, NULL);
            g_hSlot2Button = CreateWindowA("BUTTON", "[ Slot 2 ]", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 400, 200, 90, 50, hwnd, (HMENU)302, NULL, NULL);

            // Action Buttons
            CreateWindowA("BUTTON", "✨ Transmute", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 315, 265, 155, 40, hwnd, (HMENU)201, NULL, NULL);
            CreateWindowA("BUTTON", "Clear Crucible", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 330, 315, 125, 28, hwnd, (HMENU)202, NULL, NULL);
            CreateWindowA("BUTTON", "Reset Progress", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 330, 350, 125, 26, hwnd, (HMENU)203, NULL, NULL);

            // Journal Log Edit Control (Read-Only)
            g_hJournalEdit = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_BORDER,
                530, 96, 225, 415, hwnd, (HMENU)402, NULL, NULL);

            AddJournalLog("Welcome Apprentice Alchemist!\r\nSelect elements from your Grimoire to combine in the Crucible.");
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);

            // Search filter edit box changed
            if (id == 401 && code == EN_CHANGE) {
                GetWindowTextA(g_hSearchEdit, g_State.searchFilter, sizeof(g_State.searchFilter));
                UpdateElementButtonsVisibility();
            }
            // Element selection (IDs 100 to 119)
            else if (id >= 100 && id < 100 + TOTAL_ELEMENTS) {
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
                    AddJournalLog("Place two elements into the Crucible before transmuting.");
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

                            char logMsg[256];
                            wsprintfA(logMsg, "✨ NEW DISCOVERY! You created %s by combining %s + %s! (+50 Essence)",
                                g_Elements[res].name, g_Elements[e1].name, g_Elements[e2].name);
                            AddJournalLog(logMsg);

                            PlayDiscoveryFanfare();
                        } else {
                            wsprintfA(g_State.lastStatus, "Created %s (Known)", g_Elements[res].name);
                            char logMsg[256];
                            wsprintfA(logMsg, "Created %s (%s + %s). Already recorded in Grimoire.",
                                g_Elements[res].name, g_Elements[e1].name, g_Elements[e2].name);
                            AddJournalLog(logMsg);
                            Beep(659, 120);
                        }
                    } else {
                        wsprintfA(g_State.lastStatus, "Reaction Fizzled!");
                        char logMsg[256];
                        wsprintfA(logMsg, "Reaction fizzled! No transmutation for %s + %s.",
                            g_Elements[e1].name, g_Elements[e2].name);
                        AddJournalLog(logMsg);
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
                    SetWindowTextA(g_hSearchEdit, "");
                    UpdateSlotButtonText();
                    UpdateElementButtonsVisibility();
                    SetWindowTextA(g_hJournalEdit, "");
                    AddJournalLog("Journal reset. Basic elements restored.");
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

            // Helper macro for drawing glowing panel borders
            #define DRAW_RUNE_PANEL(r, brush) { \
                FillRect(hdc, &(r), (brush)); \
                HGDIOBJ pOldP = SelectObject(hdc, hPurplePen); \
                Rectangle(hdc, (r).left, (r).top, (r).right, (r).bottom); \
                SelectObject(hdc, hGoldPen); \
                Rectangle(hdc, (r).left + 2, (r).top + 2, (r).right - 2, (r).bottom - 2); \
                SelectObject(hdc, pOldP); \
            }

            // Top Header Bar
            RECT headerRect = { 20, 10, 765, 60 };
            DRAW_RUNE_PANEL(headerRect, hPanelBrush);

            SelectObject(hdc, hTitleFont);
            SetTextColor(hdc, RGB(243, 156, 18));
            TextOutA(hdc, 35, 18, "KALCHEMY - Arcane Laboratory", 28);

            // Rank calculation
            const char* rankStr = "Apprentice";
            if (g_State.discoveredCount >= 20) rankStr = "Grand Master";
            else if (g_State.discoveredCount >= 15) rankStr = "Master Alchemist";
            else if (g_State.discoveredCount >= 10) rankStr = "Journeyman";
            else if (g_State.discoveredCount >= 6) rankStr = "Adept";

            // Golden Element Badges in Header
            SelectObject(hdc, hBadgeFont);
            HGDIOBJ oldBrush = SelectObject(hdc, hGoldBadgeBrush);
            HGDIOBJ oldPen = SelectObject(hdc, hGoldPen);

            // Badge 1: Discovered
            RECT badge1 = { 375, 18, 490, 52 };
            RoundRect(hdc, badge1.left, badge1.top, badge1.right, badge1.bottom, 8, 8);
            char b1Str[32];
            wsprintfA(b1Str, "Discovered: %d/%d", g_State.discoveredCount, TOTAL_ELEMENTS);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, b1Str, -1, &badge1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            // Badge 2: Essence
            RECT badge2 = { 498, 18, 613, 52 };
            RoundRect(hdc, badge2.left, badge2.top, badge2.right, badge2.bottom, 8, 8);
            char b2Str[32];
            wsprintfA(b2Str, "Essence: %d", g_State.essence);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, b2Str, -1, &badge2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            // Badge 3: Rank
            RECT badge3 = { 621, 18, 750, 52 };
            RoundRect(hdc, badge3.left, badge3.top, badge3.right, badge3.bottom, 8, 8);
            char b3Str[64];
            wsprintfA(b3Str, "Rank: %s", rankStr);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, b3Str, -1, &badge3, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            SelectObject(hdc, oldBrush);
            SelectObject(hdc, oldPen);

            // Left Panel - Element Grimoire
            RECT leftPanel = { 20, 68, 265, 523 };
            DRAW_RUNE_PANEL(leftPanel, hPanelBrush);

            SelectObject(hdc, hHeaderFont);
            SetTextColor(hdc, RGB(243, 156, 18));
            TextOutA(hdc, 30, 74, "Elemental Grimoire", 18);

            char countStr[32];
            wsprintfA(countStr, "(%d Elements)", g_State.discoveredCount);
            SelectObject(hdc, hUIFont);
            SetTextColor(hdc, RGB(180, 160, 220));
            TextOutA(hdc, 175, 76, countStr, lstrlenA(countStr));

            // Draw Golden Badge Frames around Discovered Element Buttons
            SelectObject(hdc, hGoldPen);
            for (int i = 0; i < TOTAL_ELEMENTS; i++) {
                if (g_State.discovered[i] && IsWindowVisible(g_hElemButtons[i])) {
                    RECT btnR;
                    GetWindowRect(g_hElemButtons[i], &btnR);
                    MapWindowPoints(NULL, hwnd, (LPPOINT)&btnR, 2);
                    InflateRect(&btnR, 2, 2);
                    HGDIOBJ pNullBrush = GetStockObject(NULL_BRUSH);
                    HGDIOBJ pOldB = SelectObject(hdc, pNullBrush);
                    Rectangle(hdc, btnR.left, btnR.top, btnR.right, btnR.bottom);
                    SelectObject(hdc, pOldB);
                }
            }

            // Center Panel - Transmutation Crucible
            RECT centerPanel = { 275, 68, 510, 523 };
            DRAW_RUNE_PANEL(centerPanel, hCrucibleBrush);

            SelectObject(hdc, hHeaderFont);
            SetTextColor(hdc, RGB(243, 156, 18));
            TextOutA(hdc, 295, 74, "Transmutation Crucible", 22);

            // Draw Outer Glowing Arcane Rune Ring & Crucible Vessel
            SelectObject(hdc, hGoldPen);
            HGDIOBJ pNullB = GetStockObject(NULL_BRUSH);
            SelectObject(hdc, pNullB);
            Ellipse(hdc, 340, 92, 445, 193); // Outer golden rune ring

            SelectObject(hdc, hVesselBrush);
            SelectObject(hdc, hPurplePen);
            Ellipse(hdc, 350, 102, 435, 183); // Core vessel ellipse

            SelectObject(hdc, hSlotFont);
            SetTextColor(hdc, RGB(241, 196, 15));
            TextOutA(hdc, 364, 135, "Crucible", 8);

            // Status message centered below crucible buttons
            SelectObject(hdc, hUIFont);
            SetTextColor(hdc, RGB(120, 230, 180));
            RECT statusRect = { 285, 390, 500, 440 };
            DrawTextA(hdc, g_State.lastStatus, -1, &statusRect, DT_CENTER | DT_WORDBREAK);

            // Right Panel - Journal & Log
            RECT rightPanel = { 520, 68, 765, 523 };
            DRAW_RUNE_PANEL(rightPanel, hPanelBrush);

            SelectObject(hdc, hHeaderFont);
            SetTextColor(hdc, RGB(243, 156, 18));
            TextOutA(hdc, 535, 74, "Alchemist's Journal", 19);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            HWND hEdit = (HWND)lParam;
            if (hEdit == g_hJournalEdit || hEdit == g_hSearchEdit) {
                SetTextColor(hdcEdit, RGB(226, 232, 240));
                SetBkColor(hdcEdit, RGB(16, 14, 32));
                static HBRUSH hEditBg = NULL;
                if (!hEditBg) hEditBg = CreateSolidBrush(RGB(16, 14, 32));
                return (INT_PTR)hEditBg;
            }
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
            if (hVesselBrush) DeleteObject(hVesselBrush);
            if (hGoldBadgeBrush) DeleteObject(hGoldBadgeBrush);
            if (hVesselPen) DeleteObject(hVesselPen);
            if (hPurplePen) DeleteObject(hPurplePen);
            if (hGoldPen) DeleteObject(hGoldPen);
            if (hInnerGlowPen) DeleteObject(hInnerGlowPen);
            if (hTitleFont) DeleteObject(hTitleFont);
            if (hHeaderFont) DeleteObject(hHeaderFont);
            if (hUIFont) DeleteObject(hUIFont);
            if (hSlotFont) DeleteObject(hSlotFont);
            if (hBadgeFont) DeleteObject(hBadgeFont);
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
