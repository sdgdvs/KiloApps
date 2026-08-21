#include <windows.h>
#include <stdio.h>
#include <string.h>

#define ID_BTN_DEST1 101
#define ID_BTN_DEST2 102
#define ID_LIST_LOG  103
#define ID_BTN_BUY_START 200
#define ID_BTN_SELL_START 210

typedef struct {
    int credits;
    int fuel;
    int maxFuel;
    int cargo;
    int maxCargo;
    int location; // 0 = Earth, 1 = Mars, 2 = Venus
    int inventory[5]; // Food, Water, Ore, Tech, Meds
    int engineLevel;
    int cargoLevel;
    int weaponLevel;
} GameState;

GameState state = { 1000, 100, 100, 0, 20, 0, {0,0,0,0,0}, 0, 0, 0 };

const char* goodNames[5] = { "Food", "Water", "Ore", "Tech", "Meds" };
int marketBases[3][5] = {
    { 10, 15, 60, 100, 40 }, // Earth
    { 40, 50, 20, 120, 50 }, // Mars
    { 50, 40, 70,  30, 90 }  // Venus
};
int currentPrices[5];

unsigned int rngState = 0x1234;
unsigned int SimpleRand() {
    rngState = (rngState >> 1) ^ (-(int)(rngState & 1u) & 0xB400u);
    return rngState;
}

const char* planetNames[] = { "Earth", "Mars", "Venus" };
int distances[3][3] = {
    {0, 20, 15}, // Earth
    {20, 0, 25}, // Mars
    {15, 25, 0}  // Venus
};

HWND hStatCredits, hStatFuel, hStatCargo, hStatWeapons;
HWND hStatLoc;
HWND hBtnDest1, hBtnDest2;
HWND hListLog;
#define ID_BTN_UPG_CARGO 301
#define ID_BTN_UPG_ENGINE 302
#define ID_BTN_UPG_WEAPON 303
HWND hBtnUpgCargo, hBtnUpgEngine, hBtnUpgWeapon;

HWND hStatGoodName[5];
HWND hStatGoodPrice[5];
HWND hStatGoodOwned[5];
HWND hBtnGoodBuy[5];
HWND hBtnGoodSell[5];

int destTarget[2];
int destCost[2];

void GeneratePrices() {
    for (int i = 0; i < 5; i++) {
        int base = marketBases[state.location][i];
        int variance = base / 5;
        int rand_var = 0;
        if (variance > 0) rand_var = SimpleRand() % (variance * 2 + 1);
        currentPrices[i] = base - variance + rand_var;
    }
}

void LogMessage(const char* msg) {
    SendMessage(hListLog, LB_ADDSTRING, 0, (LPARAM)msg);
    int count = SendMessage(hListLog, LB_GETCOUNT, 0, 0);
    SendMessage(hListLog, LB_SETTOPINDEX, count - 1, 0);
}

void UpdateUI(HWND hwnd) {
    char buf[128];
    wsprintf(buf, "Credits: %d", state.credits);
    SetWindowText(hStatCredits, buf);
    wsprintf(buf, "Fuel: %d / %d", state.fuel, state.maxFuel);
    SetWindowText(hStatFuel, buf);
    wsprintf(buf, "Cargo: %d / %d", state.cargo, state.maxCargo);
    SetWindowText(hStatCargo, buf);
    wsprintf(buf, "Weapons: Lv %d", state.weaponLevel);
    SetWindowText(hStatWeapons, buf);

    wsprintf(buf, "Current Location: %s", planetNames[state.location]);
    SetWindowText(hStatLoc, buf);

    // Update buttons
    int btnIdx = 0;
    for (int i = 0; i < 3; i++) {
        if (i == state.location) continue;
        destTarget[btnIdx] = i;
        destCost[btnIdx] = distances[state.location][i] - state.engineLevel * 2;
        if (destCost[btnIdx] < 1) destCost[btnIdx] = 1;
        
        wsprintf(buf, "Travel to %s (%d fuel)", planetNames[i], destCost[btnIdx]);
        SetWindowText(btnIdx == 0 ? hBtnDest1 : hBtnDest2, buf);
        EnableWindow(btnIdx == 0 ? hBtnDest1 : hBtnDest2, state.fuel >= destCost[btnIdx]);
        
        btnIdx++;
    }

    int cargoCost = 500 * (state.cargoLevel + 1);
    int engineCost = 1000 * (state.engineLevel + 1);
    int weaponCost = 1500 * (state.weaponLevel + 1);

    wsprintf(buf, "Upg Cargo (%d cr)", cargoCost);
    SetWindowText(hBtnUpgCargo, buf);
    EnableWindow(hBtnUpgCargo, state.credits >= cargoCost);

    wsprintf(buf, "Upg Engine (%d cr)", engineCost);
    SetWindowText(hBtnUpgEngine, buf);
    EnableWindow(hBtnUpgEngine, state.credits >= engineCost);

    wsprintf(buf, "Upg Weapons (%d cr)", weaponCost);
    SetWindowText(hBtnUpgWeapon, buf);
    EnableWindow(hBtnUpgWeapon, state.credits >= weaponCost);

    // Update Market
    for (int i = 0; i < 5; i++) {
        wsprintf(buf, "%d cr", currentPrices[i]);
        SetWindowText(hStatGoodPrice[i], buf);
        wsprintf(buf, "Own: %d", state.inventory[i]);
        SetWindowText(hStatGoodOwned[i], buf);
        
        EnableWindow(hBtnGoodBuy[i], state.credits >= currentPrices[i] && state.cargo < state.maxCargo);
        EnableWindow(hBtnGoodSell[i], state.inventory[i] > 0);
    }
}

void Travel(int btnIdx, HWND hwnd) {
    int cost = destCost[btnIdx];
    int target = destTarget[btnIdx];
    
    if (state.fuel >= cost) {
        state.fuel -= cost;
        state.location = target;
        char buf[256];
        wsprintf(buf, "> Hyperspace jump complete. Arrived at %s. Used %d fuel.", planetNames[target], cost);
        LogMessage(buf);
        GeneratePrices();
        UpdateUI(hwnd);
    } else {
        LogMessage("> Insufficient fuel!");
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            rngState = GetTickCount();
            if (rngState == 0) rngState = 0x1234;
            GeneratePrices();

            HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
            
            CreateWindow("STATIC", "KTrader Space Trading Sim", WS_CHILD | WS_VISIBLE,
                20, 20, 300, 20, hwnd, NULL, NULL, NULL);

            hStatCredits = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 10, 60, 140, 20, hwnd, NULL, NULL, NULL);
            hStatFuel = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 160, 60, 140, 20, hwnd, NULL, NULL, NULL);
            hStatCargo = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 310, 60, 140, 20, hwnd, NULL, NULL, NULL);
            hStatWeapons = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 460, 60, 140, 20, hwnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Navigation", WS_CHILD | WS_VISIBLE, 20, 100, 100, 20, hwnd, NULL, NULL, NULL);
            hStatLoc = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 20, 130, 200, 20, hwnd, NULL, NULL, NULL);

            hBtnDest1 = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 160, 250, 30, hwnd, (HMENU)ID_BTN_DEST1, NULL, NULL);
            hBtnDest2 = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 200, 250, 30, hwnd, (HMENU)ID_BTN_DEST2, NULL, NULL);

            HWND hStatShipyard = CreateWindow("STATIC", "Shipyard", WS_CHILD | WS_VISIBLE, 20, 240, 100, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hStatShipyard, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnUpgCargo = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 270, 180, 30, hwnd, (HMENU)ID_BTN_UPG_CARGO, NULL, NULL);
            hBtnUpgEngine = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 210, 270, 180, 30, hwnd, (HMENU)ID_BTN_UPG_ENGINE, NULL, NULL);
            hBtnUpgWeapon = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 400, 270, 180, 30, hwnd, (HMENU)ID_BTN_UPG_WEAPON, NULL, NULL);
            SendMessage(hBtnUpgCargo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnUpgEngine, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnUpgWeapon, WM_SETFONT, (WPARAM)hFont, TRUE);

            hListLog = CreateWindow("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
                20, 320, 600, 100, hwnd, (HMENU)ID_LIST_LOG, NULL, NULL);

            HWND hStatMarket = CreateWindow("STATIC", "Market", WS_CHILD | WS_VISIBLE, 300, 100, 100, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hStatMarket, WM_SETFONT, (WPARAM)hFont, TRUE);

            for (int i = 0; i < 5; i++) {
                int y = 130 + i * 22;
                hStatGoodName[i] = CreateWindow("STATIC", goodNames[i], WS_CHILD | WS_VISIBLE, 300, y, 50, 20, hwnd, NULL, NULL, NULL);
                hStatGoodPrice[i] = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 360, y, 60, 20, hwnd, NULL, NULL, NULL);
                hStatGoodOwned[i] = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 430, y, 60, 20, hwnd, NULL, NULL, NULL);
                hBtnGoodBuy[i] = CreateWindow("BUTTON", "Buy", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 500, y, 40, 20, hwnd, (HMENU)(ID_BTN_BUY_START + i), NULL, NULL);
                hBtnGoodSell[i] = CreateWindow("BUTTON", "Sell", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 550, y, 40, 20, hwnd, (HMENU)(ID_BTN_SELL_START + i), NULL, NULL);

                SendMessage(hStatGoodName[i], WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessage(hStatGoodPrice[i], WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessage(hStatGoodOwned[i], WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessage(hBtnGoodBuy[i], WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessage(hBtnGoodSell[i], WM_SETFONT, (WPARAM)hFont, TRUE);
            }

            // Set fonts
            SendDlgItemMessage(hwnd, ID_BTN_DEST1, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendDlgItemMessage(hwnd, ID_BTN_DEST2, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hListLog, WM_SETFONT, (WPARAM)hFont, TRUE);

            LogMessage("> Welcome to KTrader, Captain.");
            UpdateUI(hwnd);
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_DEST1) {
                Travel(0, hwnd);
            } else if (LOWORD(wParam) == ID_BTN_DEST2) {
                Travel(1, hwnd);
            } else if (LOWORD(wParam) >= ID_BTN_BUY_START && LOWORD(wParam) < ID_BTN_BUY_START + 5) {
                int good = LOWORD(wParam) - ID_BTN_BUY_START;
                int price = currentPrices[good];
                if (state.credits >= price && state.cargo < state.maxCargo) {
                    state.credits -= price;
                    state.inventory[good]++;
                    state.cargo++;
                    char buf[128];
                    wsprintf(buf, "> Bought 1 %s for %d cr.", goodNames[good], price);
                    LogMessage(buf);
                    UpdateUI(hwnd);
                }
            } else if (LOWORD(wParam) >= ID_BTN_SELL_START && LOWORD(wParam) < ID_BTN_SELL_START + 5) {
                int good = LOWORD(wParam) - ID_BTN_SELL_START;
                int price = currentPrices[good];
                if (state.inventory[good] > 0) {
                    state.credits += price;
                    state.inventory[good]--;
                    state.cargo--;
                    char buf[128];
                    wsprintf(buf, "> Sold 1 %s for %d cr.", goodNames[good], price);
                    LogMessage(buf);
                    UpdateUI(hwnd);
                }
            } else if (LOWORD(wParam) == ID_BTN_UPG_CARGO) {
                int cost = 500 * (state.cargoLevel + 1);
                if (state.credits >= cost) {
                    state.credits -= cost;
                    state.cargoLevel++;
                    state.maxCargo += 20;
                    char buf[128];
                    wsprintf(buf, "> Cargo Bay upgraded! Max cargo now %d.", state.maxCargo);
                    LogMessage(buf);
                    UpdateUI(hwnd);
                }
            } else if (LOWORD(wParam) == ID_BTN_UPG_ENGINE) {
                int cost = 1000 * (state.engineLevel + 1);
                if (state.credits >= cost) {
                    state.credits -= cost;
                    state.engineLevel++;
                    state.maxFuel += 50;
                    LogMessage("> Engine upgraded! Less fuel used for travel.");
                    UpdateUI(hwnd);
                }
            } else if (LOWORD(wParam) == ID_BTN_UPG_WEAPON) {
                int cost = 1500 * (state.weaponLevel + 1);
                if (state.credits >= cost) {
                    state.credits -= cost;
                    state.weaponLevel++;
                    char buf[128];
                    wsprintf(buf, "> Weapons upgraded to level %d!", state.weaponLevel);
                    LogMessage(buf);
                    UpdateUI(hwnd);
                }
            }
            return 0;
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
            
            unsigned int lfsr = 0xACE1u;
            for (int i = 0; i < 200; i++) {
                lfsr = (lfsr >> 1) ^ (-(int)(lfsr & 1u) & 0xB400u);
                int x = (lfsr * 73) % (rc.right > 0 ? rc.right : 1);
                lfsr = (lfsr >> 1) ^ (-(int)(lfsr & 1u) & 0xB400u);
                int y = (lfsr * 97) % (rc.bottom > 0 ? rc.bottom : 1);
                lfsr = (lfsr >> 1) ^ (-(int)(lfsr & 1u) & 0xB400u);
                int starIntensity = 100 + ((lfsr * 13) % 156);
                SetPixel(hdc, x, y, RGB(starIntensity, starIntensity, starIntensity));
            }
            return 1;
        }
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(0, 255, 255));
            SetBkColor(hdc, RGB(0, 0, 0));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void __stdcall MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    const char CLASS_NAME[] = "KTraderClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "KTrader",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 650, 500,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd != NULL) {
        ShowWindow(hwnd, SW_SHOWDEFAULT);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    ExitProcess(0);
}
