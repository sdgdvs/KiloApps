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

#define TOTAL_ELEMENTS 56
#define TOTAL_RECIPES 52
#define TOTAL_TIERS 5
#define GRID_SIZE 10

typedef struct {
    int id;
    const char* name;
    int tier;
    int isBasic;
} Element;

static const Element g_Elements[TOTAL_ELEMENTS] = {
    // Tier 1: Basic (4)
    { 0, "Fire", 1, 1 },
    { 1, "Water", 1, 1 },
    { 2, "Earth", 1, 1 },
    { 3, "Air", 1, 1 },

    // Tier 2: Nature (12)
    { 4, "Steam", 2, 0 },
    { 5, "Lava", 2, 0 },
    { 6, "Energy", 2, 0 },
    { 7, "Mud", 2, 0 },
    { 8, "Rain", 2, 0 },
    { 9, "Dust", 2, 0 },
    { 10, "Stone", 2, 0 },
    { 11, "Plant", 2, 0 },
    { 12, "Cloud", 2, 0 },
    { 13, "Charcoal", 2, 0 },
    { 14, "Swamp", 2, 0 },
    { 15, "Tree", 2, 0 },

    // Tier 3: Metallurgy (12)
    { 16, "Metal", 3, 0 },
    { 17, "Sand", 3, 0 },
    { 18, "Glass", 3, 0 },
    { 19, "Rust", 3, 0 },
    { 20, "Blade", 3, 0 },
    { 21, "Boiler", 3, 0 },
    { 22, "Electricity", 3, 0 },
    { 23, "Wire", 3, 0 },
    { 24, "Gunpowder", 3, 0 },
    { 25, "Explosion", 3, 0 },
    { 26, "Magnet", 3, 0 },
    { 27, "Clay", 3, 0 },

    // Tier 4: Arcane (14)
    { 28, "Life", 4, 0 },
    { 29, "Golem", 4, 0 },
    { 30, "Magic", 4, 0 },
    { 31, "Mana", 4, 0 },
    { 32, "Phoenix", 4, 0 },
    { 33, "Dragon", 4, 0 },
    { 34, "Crystal", 4, 0 },
    { 35, "Rune", 4, 0 },
    { 36, "Potion", 4, 0 },
    { 37, "Elixir", 4, 0 },
    { 38, "Nether", 4, 0 },
    { 39, "Shadow", 4, 0 },
    { 40, "Light", 4, 0 },
    { 41, "Spirit", 4, 0 },

    // Tier 5: Celestial (14)
    { 42, "Sun", 5, 0 },
    { 43, "Moon", 5, 0 },
    { 44, "Star", 5, 0 },
    { 45, "Comet", 5, 0 },
    { 46, "Meteor", 5, 0 },
    { 47, "Galaxy", 5, 0 },
    { 48, "Cosmos", 5, 0 },
    { 49, "Eclipse", 5, 0 },
    { 50, "Gold", 5, 0 },
    { 51, "Starlight", 5, 0 },
    { 52, "Supernova", 5, 0 },
    { 53, "Black Hole", 5, 0 },
    { 54, "Time", 5, 0 },
    { 55, "Eternity", 5, 0 }
};

typedef struct {
    int id;
    const char* name;
    int threshold;
    COLORREF color;
} TierInfo;

static const TierInfo g_Tiers[TOTAL_TIERS] = {
    { 1, "Basic", 0, RGB(52, 152, 219) },
    { 2, "Nature", 4, RGB(46, 204, 113) },
    { 3, "Metallurgy", 12, RGB(230, 126, 34) },
    { 4, "Arcane", 24, RGB(155, 89, 182) },
    { 5, "Celestial", 38, RGB(241, 196, 15) }
};

typedef struct {
    int ingredient1;
    int ingredient2;
    int result;
} Recipe;

static const Recipe g_Recipes[TOTAL_RECIPES] = {
    // T2 Nature (12)
    { 1, 0, 4 },   // Water + Fire -> Steam
    { 2, 0, 5 },   // Earth + Fire -> Lava
    { 0, 3, 6 },   // Fire + Air -> Energy
    { 2, 1, 7 },   // Earth + Water -> Mud
    { 1, 3, 8 },   // Water + Air -> Rain
    { 2, 3, 9 },   // Earth + Air -> Dust
    { 5, 3, 10 },  // Lava + Air -> Stone
    { 2, 8, 11 },  // Earth + Rain -> Plant
    { 3, 8, 12 },  // Air + Rain -> Cloud
    { 0, 11, 13 }, // Fire + Plant -> Charcoal
    { 7, 11, 14 }, // Mud + Plant -> Swamp
    { 11, 2, 15 }, // Plant + Earth -> Tree

    // T3 Metallurgy (12)
    { 10, 0, 16 }, // Stone + Fire -> Metal
    { 10, 3, 17 }, // Stone + Air -> Sand
    { 17, 0, 18 }, // Sand + Fire -> Glass
    { 16, 1, 19 }, // Metal + Water -> Rust
    { 16, 10, 20 },// Metal + Stone -> Blade
    { 16, 4, 21 }, // Metal + Steam -> Boiler
    { 16, 6, 22 }, // Metal + Energy -> Electricity
    { 22, 0, 23 }, // Electricity + Fire -> Wire
    { 0, 9, 24 },  // Fire + Dust -> Gunpowder
    { 24, 0, 25 }, // Gunpowder + Fire -> Explosion
    { 16, 22, 26 },// Metal + Electricity -> Magnet
    { 7, 0, 27 },  // Mud + Fire -> Clay

    // T4 Arcane (14)
    { 6, 1, 28 },  // Energy + Water -> Life
    { 28, 16, 29 },// Life + Metal -> Golem
    { 6, 3, 30 },  // Energy + Air -> Magic
    { 30, 1, 31 }, // Magic + Water -> Mana
    { 0, 28, 32 }, // Fire + Life -> Phoenix
    { 5, 28, 33 }, // Lava + Life -> Dragon
    { 10, 30, 34 },// Stone + Magic -> Crystal
    { 10, 31, 35 },// Stone + Mana -> Rune
    { 11, 31, 36 },// Plant + Mana -> Potion
    { 36, 30, 37 },// Potion + Magic -> Elixir
    { 5, 30, 38 }, // Lava + Magic -> Nether
    { 3, 30, 39 }, // Air + Magic -> Shadow
    { 0, 30, 40 }, // Fire + Magic -> Light
    { 28, 3, 41 }, // Life + Air -> Spirit

    // T5 Celestial (14)
    { 0, 40, 42 }, // Fire + Light -> Sun
    { 10, 40, 43 },// Stone + Light -> Moon
    { 42, 6, 44 }, // Sun + Energy -> Star
    { 44, 1, 45 }, // Star + Water -> Comet
    { 44, 10, 46 },// Star + Stone -> Meteor
    { 44, 12, 47 },// Star + Cloud -> Galaxy
    { 47, 30, 48 },// Galaxy + Magic -> Cosmos
    { 42, 43, 49 },// Sun + Moon -> Eclipse
    { 16, 42, 50 },// Metal + Sun -> Gold
    { 44, 40, 51 },// Star + Light -> Starlight
    { 44, 25, 52 },// Star + Explosion -> Supernova
    { 48, 39, 53 },// Cosmos + Shadow -> Black Hole
    { 48, 6, 54 }, // Cosmos + Energy -> Time
    { 54, 30, 55 } // Time + Magic -> Eternity
};

typedef struct {
    int discovered[TOTAL_ELEMENTS];
    int unlockedTiers[TOTAL_TIERS];
    int discoveredCount;
    int essence;
    int slot1;
    int slot2;
    int selectedTierFilter; // 0 = All, 1..5 = T1..T5
    int currentPage;
    int buttonElemMap[GRID_SIZE];
    char lastStatus[128];
    char searchFilter[64];
} AlchemyState;

static AlchemyState g_State;
static HWND g_hGridButtons[GRID_SIZE];
static HWND g_hTierButtons[TOTAL_TIERS + 1];
static HWND g_hSlot1Button = NULL;
static HWND g_hSlot2Button = NULL;
static HWND g_hSearchEdit = NULL;
static HWND g_hJournalEdit = NULL;
static HWND g_hPrevButton = NULL;
static HWND g_hNextButton = NULL;
static HWND g_hPageText = NULL;

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

static void PlayDiscoveryFanfare() {
    Beep(523, 70);   // C5
    Beep(659, 70);   // E5
    Beep(784, 70);   // G5
    Beep(1046, 120); // C6
}

static void PlayTierUnlockFanfare() {
    Beep(440, 60);  // A4
    Beep(554, 60);  // C#5
    Beep(659, 60);  // E5
    Beep(880, 150); // A5
}

static void UpdateGrimoireGrid() {
    int matches[TOTAL_ELEMENTS];
    int matchCount = 0;

    for (int i = 0; i < TOTAL_ELEMENTS; i++) {
        if (!g_State.discovered[i]) continue;
        if (g_State.selectedTierFilter > 0 && g_Elements[i].tier != g_State.selectedTierFilter) continue;
        if (g_State.searchFilter[0] != '\0' && !StrContainsIgnoreCase(g_Elements[i].name, g_State.searchFilter)) continue;

        matches[matchCount++] = i;
    }

    int totalPages = (matchCount + GRID_SIZE - 1) / GRID_SIZE;
    if (totalPages < 1) totalPages = 1;
    if (g_State.currentPage >= totalPages) g_State.currentPage = totalPages - 1;
    if (g_State.currentPage < 0) g_State.currentPage = 0;

    int startIndex = g_State.currentPage * GRID_SIZE;

    for (int k = 0; k < GRID_SIZE; k++) {
        int matchIdx = startIndex + k;
        if (matchIdx < matchCount) {
            int elemIdx = matches[matchIdx];
            g_State.buttonElemMap[k] = elemIdx;
            char btnText[64];
            wsprintfA(btnText, "[T%d] %s", g_Elements[elemIdx].tier, g_Elements[elemIdx].name);
            SetWindowTextA(g_hGridButtons[k], btnText);
            ShowWindow(g_hGridButtons[k], SW_SHOW);
        } else {
            g_State.buttonElemMap[k] = -1;
            ShowWindow(g_hGridButtons[k], SW_HIDE);
        }
    }

    if (g_hPageText) {
        char pageStr[32];
        wsprintfA(pageStr, "%d/%d", g_State.currentPage + 1, totalPages);
        SetWindowTextA(g_hPageText, pageStr);
    }
}

static int CheckTierUnlocks() {
    int highestTier = 1;
    for (int t = 0; t < TOTAL_TIERS; t++) {
        if (g_State.discoveredCount >= g_Tiers[t].threshold) {
            highestTier = g_Tiers[t].id;
            if (!g_State.unlockedTiers[t]) {
                g_State.unlockedTiers[t] = 1;
                char msg[256];
                wsprintfA(msg, "🔓 TIER UNLOCKED! Tier %d (%s) is now accessible!", g_Tiers[t].id, g_Tiers[t].name);
                AddJournalLog(msg);
                PlayTierUnlockFanfare();
            }
        }
    }
    return highestTier;
}

static void InitGameState() {
    memset(&g_State, 0, sizeof(AlchemyState));
    g_State.discovered[0] = 1; // Fire
    g_State.discovered[1] = 1; // Water
    g_State.discovered[2] = 1; // Earth
    g_State.discovered[3] = 1; // Air
    g_State.unlockedTiers[0] = 1; // Basic
    g_State.unlockedTiers[1] = 1; // Nature (threshold 4)
    g_State.discoveredCount = 4;
    g_State.essence = 100;
    g_State.slot1 = -1;
    g_State.slot2 = -1;
    g_State.selectedTierFilter = 0;
    g_State.currentPage = 0;
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

            // Tier Filter Buttons
            g_hTierButtons[0] = CreateWindowA("BUTTON", "All", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 30, 96, 35, 22, hwnd, (HMENU)500, NULL, NULL);
            for (int t = 1; t <= TOTAL_TIERS; t++) {
                char tStr[16];
                wsprintfA(tStr, "T%d", t);
                g_hTierButtons[t] = CreateWindowA("BUTTON", tStr, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 30 + t * 37, 96, 35, 22, hwnd, (HMENU)(UINT_PTR)(500 + t), NULL, NULL);
            }

            // Grimoire Search Input
            g_hSearchEdit = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                30, 122, 225, 24, hwnd, (HMENU)401, NULL, NULL);

            // Paginated Grid Buttons (2 columns of 5)
            for (int k = 0; k < GRID_SIZE; k++) {
                int col = k % 2;
                int row = k / 2;
                int x = 30 + col * 115;
                int y = 152 + row * 38;
                g_hGridButtons[k] = CreateWindowA("BUTTON", "",
                    WS_CHILD | BS_PUSHBUTTON,
                    x, y, 110, 34, hwnd, (HMENU)(UINT_PTR)(100 + k), NULL, NULL);
            }

            // Pagination Controls
            g_hPrevButton = CreateWindowA("BUTTON", "< Prev", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 30, 348, 65, 26, hwnd, (HMENU)601, NULL, NULL);
            g_hPageText = CreateWindowA("STATIC", "1/1", WS_CHILD | WS_VISIBLE | SS_CENTER, 100, 353, 85, 20, hwnd, (HMENU)602, NULL, NULL);
            g_hNextButton = CreateWindowA("BUTTON", "Next >", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 348, 65, 26, hwnd, (HMENU)602, NULL, NULL);

            UpdateGrimoireGrid();

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

            AddJournalLog("Welcome Apprentice Alchemist!\r\nSelect elements from your Grimoire to combine in the Crucible across 5 Tiers!");
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);

            // Search filter edit box changed
            if (id == 401 && code == EN_CHANGE) {
                GetWindowTextA(g_hSearchEdit, g_State.searchFilter, sizeof(g_State.searchFilter));
                g_State.currentPage = 0;
                UpdateGrimoireGrid();
            }
            // Tier Filter Buttons (500 = All, 501..505 = T1..T5)
            else if (id >= 500 && id <= 500 + TOTAL_TIERS) {
                g_State.selectedTierFilter = id - 500;
                g_State.currentPage = 0;
                UpdateGrimoireGrid();
                Beep(450, 40);
            }
            // Grid element buttons (100 to 109)
            else if (id >= 100 && id < 100 + GRID_SIZE) {
                int btnIdx = id - 100;
                int elemIdx = g_State.buttonElemMap[btnIdx];
                if (elemIdx >= 0 && elemIdx < TOTAL_ELEMENTS && g_State.discovered[elemIdx]) {
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
            // Page navigation
            else if (id == 601) { // Prev
                if (g_State.currentPage > 0) {
                    g_State.currentPage--;
                    UpdateGrimoireGrid();
                    Beep(400, 40);
                }
            }
            else if (id == 602) { // Next
                g_State.currentPage++;
                UpdateGrimoireGrid();
                Beep(400, 40);
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
                        int resTier = g_Elements[res].tier;
                        int reqThreshold = g_Tiers[resTier - 1].threshold;

                        if (g_State.discoveredCount < reqThreshold) {
                            wsprintfA(g_State.lastStatus, "🔒 Tier %d (%s) Locked!", resTier, g_Tiers[resTier - 1].name);
                            char logMsg[256];
                            wsprintfA(logMsg, "🔒 TIER LOCKED! Crafting %s requires Tier %d (%s) - discover %d elements!",
                                g_Elements[res].name, resTier, g_Tiers[resTier - 1].name, reqThreshold);
                            AddJournalLog(logMsg);
                            Beep(180, 150);
                        } else if (!g_State.discovered[res]) {
                            g_State.discovered[res] = 1;
                            g_State.discoveredCount++;
                            g_State.essence += 50;

                            CheckTierUnlocks();
                            UpdateGrimoireGrid();

                            wsprintfA(g_State.lastStatus, "DISCOVERY! Created %s!", g_Elements[res].name);

                            char logMsg[256];
                            wsprintfA(logMsg, "✨ NEW DISCOVERY! You created %s [Tier %d %s] by combining %s + %s! (+50 Essence)",
                                g_Elements[res].name, resTier, g_Tiers[resTier - 1].name, g_Elements[e1].name, g_Elements[e2].name);
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
                    UpdateGrimoireGrid();
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
            if (g_State.discoveredCount >= 50) rankStr = "Celestial Master";
            else if (g_State.discoveredCount >= 38) rankStr = "Grand Alchemist";
            else if (g_State.discoveredCount >= 24) rankStr = "Master Alchemist";
            else if (g_State.discoveredCount >= 12) rankStr = "Journeyman";
            else if (g_State.discoveredCount >= 4) rankStr = "Adept";

            int highestTier = CheckTierUnlocks();

            // Golden Element Badges in Header
            SelectObject(hdc, hBadgeFont);
            HGDIOBJ oldBrush = SelectObject(hdc, hGoldBadgeBrush);
            HGDIOBJ oldPen = SelectObject(hdc, hGoldPen);

            // Badge 1: Discovered
            RECT badge1 = { 360, 18, 465, 52 };
            RoundRect(hdc, badge1.left, badge1.top, badge1.right, badge1.bottom, 8, 8);
            char b1Str[32];
            wsprintfA(b1Str, "Discovered: %d/%d", g_State.discoveredCount, TOTAL_ELEMENTS);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, b1Str, -1, &badge1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            // Badge 2: Highest Tier
            RECT badge2 = { 470, 18, 565, 52 };
            RoundRect(hdc, badge2.left, badge2.top, badge2.right, badge2.bottom, 8, 8);
            char b2Str[32];
            wsprintfA(b2Str, "Tier: T%d %s", highestTier, g_Tiers[highestTier - 1].name);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, b2Str, -1, &badge2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            // Badge 3: Essence
            RECT badge3 = { 570, 18, 655, 52 };
            RoundRect(hdc, badge3.left, badge3.top, badge3.right, badge3.bottom, 8, 8);
            char b3Str[32];
            wsprintfA(b3Str, "Essence: %d", g_State.essence);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, b3Str, -1, &badge3, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

            // Badge 4: Rank
            RECT badge4 = { 660, 18, 755, 52 };
            RoundRect(hdc, badge4.left, badge4.top, badge4.right, badge4.bottom, 8, 8);
            SetTextColor(hdc, RGB(255, 215, 0));
            DrawTextA(hdc, rankStr, -1, &badge4, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

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
            if (hEdit == g_hJournalEdit || hEdit == g_hSearchEdit || hEdit == g_hPageText) {
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
