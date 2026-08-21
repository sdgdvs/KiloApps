#include <windows.h>
#include <stdio.h>
#include <string.h>

#define ID_BTN_DEST1 101
#define ID_BTN_DEST2 102
#define ID_BTN_DEST3 104
#define ID_LIST_LOG  103
#define ID_BTN_BUY_START 200
#define ID_BTN_SELL_START 210

typedef struct {
    int credits;
    int fuel;
    int maxFuel;
    int cargo;
    int maxCargo;
    int location; // Index in planets array
    int inventory[5]; // Food, Water, Ore, Tech, Meds
    int engineLevel;
    int cargoLevel;
    int weaponLevel;
    int inCombat;
    int playerShields;
    int enemyShields;
    int enemyMaxShields;
} GameState;

GameState state = { 1000, 100, 100, 0, 20, 0, {0,0,0,0,0}, 0, 0, 0, 0, 50, 30, 30 };

const char* goodNames[5] = { "Food", "Water", "Ore", "Tech", "Meds" };
int currentPrices[5];

unsigned int rngState = 0x1234;
unsigned int SimpleRand() {
    rngState = (rngState >> 1) ^ (-(int)(rngState & 1u) & 0xB400u);
    return rngState;
}

#define NUM_PLANETS 12
typedef struct {
    char name[32];
    int x, y;
    int ecoType; // 0=Agri, 1=Mining, 2=Tech, 3=Industrial, 4=Balanced
    int techLevel;
} Planet;
Planet planets[NUM_PLANETS];

const char* namePool[15] = {
    "Terra", "Ares", "Aphrodite", "Zion", "Helios",
    "Nova", "Eden", "Hades", "Atlantis", "Olympus",
    "Kronos", "Tarsus", "Vulkan", "Ryloth", "Dune"
};
const char* ecoNames[5] = { "Agri", "Mining", "Tech", "Industrial", "Balanced" };

int planetLinks[NUM_PLANETS][3];
int planetDistances[NUM_PLANETS][3];

void GenerateGalaxy() {
    for (int i = 0; i < NUM_PLANETS; i++) {
        lstrcpy(planets[i].name, namePool[i]);
        planets[i].x = SimpleRand() % 100;
        planets[i].y = SimpleRand() % 100;
        planets[i].ecoType = SimpleRand() % 5;
        planets[i].techLevel = 1 + (SimpleRand() % 5);
    }
    for (int i = 0; i < NUM_PLANETS; i++) {
        int dists[NUM_PLANETS];
        for(int j=0; j<NUM_PLANETS; j++) {
            if (i == j) { dists[j] = 999999; continue; }
            int dx = planets[i].x - planets[j].x;
            int dy = planets[i].y - planets[j].y;
            dists[j] = dx*dx + dy*dy;
        }
        for(int k=0; k<3; k++) {
            int minDist = 9999999;
            int bestJ = -1;
            for(int j=0; j<NUM_PLANETS; j++) {
                if(dists[j] < minDist) { minDist = dists[j]; bestJ = j; }
            }
            planetLinks[i][k] = bestJ;
            int r = 0;
            while(r*r <= minDist) r++;
            planetDistances[i][k] = r;
            dists[bestJ] = 999999;
        }
    }
}

void GetMarketBase(int pIdx, int basePrices[5]) {
    Planet p = planets[pIdx];
    int base[5] = {50, 50, 50, 50, 50};
    if (p.ecoType == 0) { base[0] = 20; base[1] = 25; base[3] = 90; }
    if (p.ecoType == 1) { base[2] = 20; base[0] = 80; }
    if (p.ecoType == 2) { base[3] = 30; base[4] = 40; base[2] = 80; }
    if (p.ecoType == 3) { base[2] = 30; base[3] = 40; base[0] = 80; }
    if (p.ecoType == 4) { base[0]=40; base[1]=40; base[2]=40; base[3]=40; base[4]=40; }
    base[3] += (5 - p.techLevel) * 10;
    base[4] += (5 - p.techLevel) * 10;
    for(int i=0; i<5; i++) basePrices[i] = base[i];
}

HWND hStatCredits, hStatFuel, hStatCargo, hStatWeapons;
HWND hStatLoc;
HWND hBtnDest1, hBtnDest2, hBtnDest3;
HWND hListLog;
#define ID_BTN_UPG_CARGO 301
#define ID_BTN_UPG_ENGINE 302
#define ID_BTN_UPG_WEAPON 303
HWND hBtnUpgCargo, hBtnUpgEngine, hBtnUpgWeapon;

#define ID_BTN_FIRE 401
#define ID_BTN_FLEE 402

HWND hStatCombatTitle;
HWND hStatCombatPlayer;
HWND hStatCombatEnemy;
HWND hBtnFire;
HWND hBtnFlee;

HWND hStatGoodName[5];
HWND hStatGoodPrice[5];
HWND hStatGoodOwned[5];
HWND hBtnGoodBuy[5];
HWND hBtnGoodSell[5];

int destTarget[3];
int destCost[3];

void GeneratePrices() {
    int bases[5];
    GetMarketBase(state.location, bases);
    for (int i = 0; i < 5; i++) {
        int base = bases[i];
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

    wsprintf(buf, "%s (%s, Lv %d)", planets[state.location].name, ecoNames[planets[state.location].ecoType], planets[state.location].techLevel);
    SetWindowText(hStatLoc, buf);

    // Update buttons
    for (int i = 0; i < 3; i++) {
        int target = planetLinks[state.location][i];
        destTarget[i] = target;
        destCost[i] = planetDistances[state.location][i] - state.engineLevel * 2;
        if (destCost[i] < 1) destCost[i] = 1;
        
        wsprintf(buf, "To %s (%d fuel)", planets[target].name, destCost[i]);
        HWND btn = (i == 0) ? hBtnDest1 : ((i == 1) ? hBtnDest2 : hBtnDest3);
        SetWindowText(btn, buf);
        EnableWindow(btn, !state.inCombat && state.fuel >= destCost[i]);
    }

    if (state.inCombat) {
        ShowWindow(hStatLoc, SW_HIDE);
        ShowWindow(hBtnDest1, SW_HIDE);
        ShowWindow(hBtnDest2, SW_HIDE);
        ShowWindow(hBtnDest3, SW_HIDE);
        ShowWindow(hStatCombatTitle, SW_SHOW);
        ShowWindow(hStatCombatPlayer, SW_SHOW);
        ShowWindow(hStatCombatEnemy, SW_SHOW);
        ShowWindow(hBtnFire, SW_SHOW);
        ShowWindow(hBtnFlee, SW_SHOW);
        
        wsprintf(buf, "Shields: %d", state.playerShields);
        SetWindowText(hStatCombatPlayer, buf);
        wsprintf(buf, "Enemy: %d / %d", state.enemyShields, state.enemyMaxShields);
        SetWindowText(hStatCombatEnemy, buf);
    } else {
        ShowWindow(hStatLoc, SW_SHOW);
        ShowWindow(hBtnDest1, SW_SHOW);
        ShowWindow(hBtnDest2, SW_SHOW);
        ShowWindow(hBtnDest3, SW_SHOW);
        ShowWindow(hStatCombatTitle, SW_HIDE);
        ShowWindow(hStatCombatPlayer, SW_HIDE);
        ShowWindow(hStatCombatEnemy, SW_HIDE);
        ShowWindow(hBtnFire, SW_HIDE);
        ShowWindow(hBtnFlee, SW_HIDE);
    }

    int cargoCost = 500 * (state.cargoLevel + 1);
    int engineCost = 1000 * (state.engineLevel + 1);
    int weaponCost = 1500 * (state.weaponLevel + 1);

    wsprintf(buf, "Upg Cargo (%d cr)", cargoCost);
    SetWindowText(hBtnUpgCargo, buf);
    EnableWindow(hBtnUpgCargo, !state.inCombat && state.credits >= cargoCost);

    wsprintf(buf, "Upg Engine (%d cr)", engineCost);
    SetWindowText(hBtnUpgEngine, buf);
    EnableWindow(hBtnUpgEngine, !state.inCombat && state.credits >= engineCost);

    wsprintf(buf, "Upg Weapons (%d cr)", weaponCost);
    SetWindowText(hBtnUpgWeapon, buf);
    EnableWindow(hBtnUpgWeapon, !state.inCombat && state.credits >= weaponCost);

    // Update Market
    for (int i = 0; i < 5; i++) {
        wsprintf(buf, "%d cr", currentPrices[i]);
        SetWindowText(hStatGoodPrice[i], buf);
        wsprintf(buf, "Own: %d", state.inventory[i]);
        SetWindowText(hStatGoodOwned[i], buf);
        
        EnableWindow(hBtnGoodBuy[i], !state.inCombat && state.credits >= currentPrices[i] && state.cargo < state.maxCargo);
        EnableWindow(hBtnGoodSell[i], !state.inCombat && state.inventory[i] > 0);
    }
}

void Travel(int btnIdx, HWND hwnd) {
    int cost = destCost[btnIdx];
    int target = destTarget[btnIdx];
    
    if (state.fuel >= cost) {
        state.fuel -= cost;
        state.location = target;
        char buf[256];
        wsprintf(buf, "> Hyperspace jump complete. Arrived at %s. Used %d fuel.", planets[target].name, cost);
        LogMessage(buf);

        if (SimpleRand() % 100 < 30) {
            int enc = SimpleRand() % 3;
            if (enc == 0) {
                int fuelLoss = 5 + (SimpleRand() % 11);
                state.fuel -= fuelLoss;
                if (state.fuel < 0) state.fuel = 0;
                char encBuf[256];
                wsprintf(encBuf, "> WARNING: Asteroid field! Evasive maneuvers cost %d fuel.", fuelLoss);
                LogMessage(encBuf);
            } else if (enc == 1) {
                int creditsGained = 50 + (SimpleRand() % 101);
                state.credits += creditsGained;
                char encBuf[256];
                wsprintf(encBuf, "> Distress signal! Helped stranded ship, received %d credits.", creditsGained);
                LogMessage(encBuf);
            } else {
                int available[5];
                int availCount = 0;
                for (int i = 0; i < 5; i++) {
                    if (state.inventory[i] > 0) available[availCount++] = i;
                }
                state.inCombat = 1;
                state.playerShields = 50 + state.cargoLevel * 10;
                state.enemyMaxShields = 20 + (SimpleRand() % 40);
                state.enemyShields = state.enemyMaxShields;
                LogMessage("> 🚨 RED ALERT: PIRATE VESSEL INTERCEPTED YOU! 🚨");
            }
        }

        GeneratePrices();
        UpdateUI(hwnd);
    } else {
        LogMessage("> Insufficient fuel!");
    }
}

void EnemyTurn(HWND hwnd) {
    int dmg = 5 + (SimpleRand() % 15);
    state.playerShields -= dmg;
    char buf[128];
    wsprintf(buf, "> Pirates fired! Your shields took %d damage.", dmg);
    LogMessage(buf);

    if (state.playerShields <= 0) {
        LogMessage("> Your shields failed!");
        int lost = (int)(state.credits * 0.2) + 50;
        if (lost > state.credits) lost = state.credits;
        state.credits -= lost;
        wsprintf(buf, "> Pirates looted %d credits and left you drifting.", lost);
        LogMessage(buf);
        state.inCombat = 0;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            rngState = GetTickCount();
            if (rngState == 0) rngState = 0x1234;
            GenerateGalaxy();
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
            hStatLoc = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 20, 130, 260, 20, hwnd, NULL, NULL, NULL);

            hBtnDest1 = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 160, 250, 30, hwnd, (HMENU)ID_BTN_DEST1, NULL, NULL);
            hBtnDest2 = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 200, 250, 30, hwnd, (HMENU)ID_BTN_DEST2, NULL, NULL);
            hBtnDest3 = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 240, 250, 30, hwnd, (HMENU)ID_BTN_DEST3, NULL, NULL);

            hStatCombatTitle = CreateWindow("STATIC", "🚨 COMBAT ENGAGED 🚨", WS_CHILD, 20, 100, 200, 20, hwnd, NULL, NULL, NULL);
            hStatCombatPlayer = CreateWindow("STATIC", "Shields: 50", WS_CHILD, 20, 130, 200, 20, hwnd, NULL, NULL, NULL);
            hStatCombatEnemy = CreateWindow("STATIC", "Enemy: 30 / 30", WS_CHILD, 20, 160, 200, 20, hwnd, NULL, NULL, NULL);
            hBtnFire = CreateWindow("BUTTON", "Fire Weapons", WS_CHILD | BS_PUSHBUTTON, 20, 190, 120, 30, hwnd, (HMENU)ID_BTN_FIRE, NULL, NULL);
            hBtnFlee = CreateWindow("BUTTON", "Attempt Flee", WS_CHILD | BS_PUSHBUTTON, 150, 190, 120, 30, hwnd, (HMENU)ID_BTN_FLEE, NULL, NULL);
            
            SendMessage(hStatCombatTitle, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hStatCombatPlayer, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hStatCombatEnemy, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnFire, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnFlee, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hStatShipyard = CreateWindow("STATIC", "Shipyard", WS_CHILD | WS_VISIBLE, 20, 280, 100, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hStatShipyard, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnUpgCargo = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 310, 180, 30, hwnd, (HMENU)ID_BTN_UPG_CARGO, NULL, NULL);
            hBtnUpgEngine = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 210, 310, 180, 30, hwnd, (HMENU)ID_BTN_UPG_ENGINE, NULL, NULL);
            hBtnUpgWeapon = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 400, 310, 180, 30, hwnd, (HMENU)ID_BTN_UPG_WEAPON, NULL, NULL);
            SendMessage(hBtnUpgCargo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnUpgEngine, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnUpgWeapon, WM_SETFONT, (WPARAM)hFont, TRUE);

            hListLog = CreateWindow("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
                20, 360, 600, 100, hwnd, (HMENU)ID_LIST_LOG, NULL, NULL);

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
            SendDlgItemMessage(hwnd, ID_BTN_DEST3, WM_SETFONT, (WPARAM)hFont, TRUE);
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
            } else if (LOWORD(wParam) == ID_BTN_DEST3) {
                Travel(2, hwnd);
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
            } else if (LOWORD(wParam) == ID_BTN_FIRE) {
                int dmg = 10 + state.weaponLevel * 15 + (SimpleRand() % 10);
                state.enemyShields -= dmg;
                char buf[128];
                wsprintf(buf, "> You fired! Pirate shields took %d damage.", dmg);
                LogMessage(buf);

                if (state.enemyShields <= 0) {
                    LogMessage("> Pirate ship destroyed!");
                    int bounty = 50 + (SimpleRand() % 100);
                    state.credits += bounty;
                    wsprintf(buf, "> Recovered %d credits from the wreckage.", bounty);
                    LogMessage(buf);
                    state.inCombat = 0;
                } else {
                    EnemyTurn(hwnd);
                }
                UpdateUI(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_FLEE) {
                if ((SimpleRand() % 100) < 50) {
                    LogMessage("> Successfully fled from the pirates!");
                    state.inCombat = 0;
                } else {
                    LogMessage("> Failed to escape!");
                    EnemyTurn(hwnd);
                }
                UpdateUI(hwnd);
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
        CW_USEDEFAULT, CW_USEDEFAULT, 650, 550,
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
