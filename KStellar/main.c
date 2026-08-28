#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define ID_BTN_SET_COURSE 101

typedef struct {
    int id;
    char name[32];
    int x;
    int y;
    char desc[256];
    int food_price;
    int minerals_price;
    int tech_price;
} System;

System systems[] = {
    {1, "Sol", 50, 50, "Cradle of humanity. Highly developed planetary system with a stable economy.", 20, 40, 150},
    {2, "Alpha Centauri", 30, 70, "Primary mining colony. Resource rich, but known for pirate activity on the outskirts.", 40, 15, 200},
    {3, "Sirius", 75, 35, "Major trading hub. High tech goods are cheap here.", 30, 60, 80},
    {4, "Proxima", 20, 80, "Remote outpost. Dangerous but offers high rewards for smugglers.", 50, 30, 300},
    {5, "Vega", 80, 80, "Agricultural world. Supplies food to neighboring industrial systems.", 5, 80, 180}
};

#define ID_BTN_BUY_FOOD 102
#define ID_BTN_SELL_FOOD 103
#define ID_BTN_BUY_MINERALS 104
#define ID_BTN_SELL_MINERALS 105
#define ID_BTN_BUY_TECH 106
#define ID_BTN_SELL_TECH 107
#define ID_BTN_UPG_ENGINE 108
#define ID_BTN_UPG_CARGO 109
#define ID_BTN_UPG_WEAPON 110
#define ID_BTN_UPG_SHIELD 111

#define ID_BTN_COMBAT_ATTACK 120
#define ID_BTN_COMBAT_EVADE 121
#define ID_BTN_COMBAT_USE_TECH 122
#define ID_BTN_COMBAT_FLEE 123

#define ID_BTN_MISSIONS 130
#define ID_BTN_MISSION_ACCEPT_1 131
#define ID_BTN_MISSION_ACCEPT_2 132
#define ID_BTN_MISSION_ABANDON 133
#define ID_BTN_MISSION_BACK 134

typedef struct {
    int type; // 0=None, 1=Delivery, 2=Bounty
    int targetId;
    int reward;
} Mission;

HWND hMapArea, hInfoArea, hBtnCourse, hFuelText, hCreditsText, hCargoText, hHullText, hMissionText;
HWND hBtnBuyFood, hBtnSellFood, hBtnBuyMinerals, hBtnSellMinerals, hBtnBuyTech, hBtnSellTech;
HWND hBtnUpgEngine, hBtnUpgCargo, hBtnUpgWeapon, hBtnUpgShield;
HWND hBtnCombatAttack, hBtnCombatEvade, hBtnCombatUseTech, hBtnCombatFlee;
HWND hBtnMissions, hBtnMissionAccept1, hBtnMissionAccept2, hBtnMissionAbandon, hBtnMissionBack;

int selectedSystem = -1;
int currentSystemId = 0;
int inMissionsView = 0;

Mission activeMission = {0, -1, 0};
Mission stationMissions[2] = {{0,-1,0}, {0,-1,0}};

void GenerateStationMissions() {
    for (int i=0; i<2; i++) {
        stationMissions[i].type = (rand() % 2) + 1;
        int target = rand() % 5;
        if (target == currentSystemId) {
            target = (target + 1) % 5;
        }
        stationMissions[i].targetId = target;
        stationMissions[i].reward = 300 + (rand() % 500);
    }
}
int fuel = 100;
int credits = 1000;
int cargoFood = 0;
int cargoMinerals = 0;
int cargoTech = 0;
int cargoMax = 50;

int engineLevel = 1;
int cargoLevel = 1;
int weaponLevel = 1;
int shieldLevel = 1;

int hull = 100;
int maxHull = 100;
int enemyHull = 0;
int enemyMaxHull = 0;
int inCombat = 0;
char combatLog[1024] = "";

static HFONT hFont = NULL;
static HBRUSH hBgBrush = NULL;

void UpdateDashboard() {
    char buf[64];
    sprintf(buf, "HULL: %d/%d", hull, maxHull);
    SetWindowText(hHullText, buf);
    sprintf(buf, "FUEL: %d%%", fuel);
    SetWindowText(hFuelText, buf);
    sprintf(buf, "CREDITS: %d", credits);
    SetWindowText(hCreditsText, buf);
    int used = cargoFood + cargoMinerals + cargoTech;
    sprintf(buf, "CARGO: %d/%d TONS", used, cargoMax);
    SetWindowText(hCargoText, buf);
    
    if (activeMission.type == 1) {
        sprintf(buf, "MISSION: Deliver to %s (₭%d)", systems[activeMission.targetId].name, activeMission.reward);
        SetWindowText(hMissionText, buf);
    } else if (activeMission.type == 2) {
        sprintf(buf, "MISSION: Hunt pirate in %s (₭%d)", systems[activeMission.targetId].name, activeMission.reward);
        SetWindowText(hMissionText, buf);
    } else {
        SetWindowText(hMissionText, "");
    }
}

void ShowStationView(HWND hwnd) {
    inMissionsView = 0;
    
    ShowWindow(hBtnMissionAccept1, SW_HIDE);
    ShowWindow(hBtnMissionAccept2, SW_HIDE);
    ShowWindow(hBtnMissionAbandon, SW_HIDE);
    ShowWindow(hBtnMissionBack, SW_HIDE);
    
    ShowWindow(hBtnMissions, SW_SHOW);
    ShowWindow(hBtnCourse, SW_HIDE);
    ShowWindow(hBtnBuyFood, SW_SHOW);
    ShowWindow(hBtnSellFood, SW_SHOW);
    ShowWindow(hBtnBuyMinerals, SW_SHOW);
    ShowWindow(hBtnSellMinerals, SW_SHOW);
    ShowWindow(hBtnBuyTech, SW_SHOW);
    ShowWindow(hBtnSellTech, SW_SHOW);
    ShowWindow(hBtnUpgEngine, SW_SHOW);
    ShowWindow(hBtnUpgCargo, SW_SHOW);
    ShowWindow(hBtnUpgWeapon, SW_SHOW);
    ShowWindow(hBtnUpgShield, SW_SHOW);
    
    System *sys = &systems[currentSystemId];
    char infoText[512];
    sprintf(infoText, "%s (DOCKED)\n%s\nMkt:F:%d M:%d T:%d\nInv:F:%d M:%d T:%d\nE:%d/₭%d C:%d/₭%d\nW:%d/₭%d S:%d/₭%d", 
        sys->name, sys->desc,
        sys->food_price, sys->minerals_price, sys->tech_price,
        cargoFood, cargoMinerals, cargoTech,
        engineLevel, 1000*engineLevel, cargoLevel, 1500*cargoLevel,
        weaponLevel, 2000*weaponLevel, shieldLevel, 2000*shieldLevel);
    SetWindowText(hInfoArea, infoText);
}

void ShowMissionsView(HWND hwnd) {
    inMissionsView = 1;
    
    ShowWindow(hBtnMissions, SW_HIDE);
    ShowWindow(hBtnCourse, SW_HIDE);
    ShowWindow(hBtnBuyFood, SW_HIDE);
    ShowWindow(hBtnSellFood, SW_HIDE);
    ShowWindow(hBtnBuyMinerals, SW_HIDE);
    ShowWindow(hBtnSellMinerals, SW_HIDE);
    ShowWindow(hBtnBuyTech, SW_HIDE);
    ShowWindow(hBtnSellTech, SW_HIDE);
    ShowWindow(hBtnUpgEngine, SW_HIDE);
    ShowWindow(hBtnUpgCargo, SW_HIDE);
    ShowWindow(hBtnUpgWeapon, SW_HIDE);
    ShowWindow(hBtnUpgShield, SW_HIDE);
    
    ShowWindow(hBtnMissionBack, SW_SHOW);
    if (activeMission.type != 0) {
        ShowWindow(hBtnMissionAbandon, SW_SHOW);
        ShowWindow(hBtnMissionAccept1, SW_HIDE);
        ShowWindow(hBtnMissionAccept2, SW_HIDE);
    } else {
        ShowWindow(hBtnMissionAbandon, SW_HIDE);
        if (stationMissions[0].type != 0) ShowWindow(hBtnMissionAccept1, SW_SHOW);
        if (stationMissions[1].type != 0) ShowWindow(hBtnMissionAccept2, SW_SHOW);
    }
    
    char infoText[1024] = "--- MISSION BOARD ---\n\n";
    if (activeMission.type != 0) {
        char amsg[256];
        sprintf(amsg, "Active: %s %s (₭%d)\n\n", 
            activeMission.type == 1 ? "Deliver to" : "Bounty in", 
            systems[activeMission.targetId].name, activeMission.reward);
        strcat(infoText, amsg);
    } else {
        strcat(infoText, "No active missions.\n\n");
    }
    
    strcat(infoText, "Available Missions:\n");
    for (int i=0; i<2; i++) {
        if (stationMissions[i].type != 0) {
            char mmsg[128];
            sprintf(mmsg, "M%d: %s %s (₭%d)\n", i+1,
                stationMissions[i].type == 1 ? "Deliver to" : "Bounty in",
                systems[stationMissions[i].targetId].name, stationMissions[i].reward);
            strcat(infoText, mmsg);
        }
    }
    SetWindowText(hInfoArea, infoText);
}

void UpdateCombatUI(HWND hwnd) {
    char infoText[2048];
    sprintf(infoText, "--- COMBAT ---\nYOU Hull: %d\nPIRATE Hull: %d\n\n%s", hull, enemyHull, combatLog);
    SetWindowText(hInfoArea, infoText);
    UpdateDashboard();
}

void EndCombat(HWND hwnd) {
    inCombat = 0;
    ShowWindow(hBtnCombatAttack, SW_HIDE);
    ShowWindow(hBtnCombatEvade, SW_HIDE);
    ShowWindow(hBtnCombatUseTech, SW_HIDE);
    ShowWindow(hBtnCombatFlee, SW_HIDE);
    
    SetWindowText(hInfoArea, "Select a system on the map for details.");
    selectedSystem = -1;
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            hFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
            hBgBrush = CreateSolidBrush(RGB(5, 5, 15));
            
            HWND h0 = CreateWindow("STATIC", "HULL: 100/100", WS_VISIBLE | WS_CHILD, 20, 20, 110, 20, hwnd, NULL, NULL, NULL);
            hHullText = h0;
            SendMessage(h0, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND h1 = CreateWindow("STATIC", "FUEL: 100%", WS_VISIBLE | WS_CHILD, 140, 20, 100, 20, hwnd, NULL, NULL, NULL);
            hFuelText = h1;
            SendMessage(h1, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND h2 = CreateWindow("STATIC", "CREDITS: 1,000", WS_VISIBLE | WS_CHILD, 240, 20, 150, 20, hwnd, NULL, NULL, NULL);
            hCreditsText = h2;
            SendMessage(h2, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND h3 = CreateWindow("STATIC", "CARGO: 0/50 TONS", WS_VISIBLE | WS_CHILD, 380, 20, 180, 20, hwnd, NULL, NULL, NULL);
            hCargoText = h3;
            SendMessage(h3, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND h4 = CreateWindow("STATIC", "LOCAL SYSTEMS", WS_VISIBLE | WS_CHILD, 440, 60, 150, 20, hwnd, NULL, NULL, NULL);
            SendMessage(h4, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hMissionText = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD, 20, 45, 600, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hMissionText, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hInfoArea = CreateWindow("STATIC", "Select a system on the map for details.", WS_VISIBLE | WS_CHILD, 440, 90, 220, 180, hwnd, NULL, NULL, NULL);
            SendMessage(hInfoArea, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnBuyFood = CreateWindow("BUTTON", "Buy Food", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 280, 95, 25, hwnd, (HMENU)ID_BTN_BUY_FOOD, NULL, NULL);
            hBtnSellFood = CreateWindow("BUTTON", "Sell Food", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 280, 95, 25, hwnd, (HMENU)ID_BTN_SELL_FOOD, NULL, NULL);
            
            hBtnBuyMinerals = CreateWindow("BUTTON", "Buy Min", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 310, 95, 25, hwnd, (HMENU)ID_BTN_BUY_MINERALS, NULL, NULL);
            hBtnSellMinerals = CreateWindow("BUTTON", "Sell Min", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 310, 95, 25, hwnd, (HMENU)ID_BTN_SELL_MINERALS, NULL, NULL);
            
            hBtnBuyTech = CreateWindow("BUTTON", "Buy Tech", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 340, 95, 25, hwnd, (HMENU)ID_BTN_BUY_TECH, NULL, NULL);
            hBtnSellTech = CreateWindow("BUTTON", "Sell Tech", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 340, 95, 25, hwnd, (HMENU)ID_BTN_SELL_TECH, NULL, NULL);

            hBtnCourse = CreateWindow("BUTTON", "Set Course", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 380, 150, 30, hwnd, (HMENU)ID_BTN_SET_COURSE, NULL, NULL);
            hBtnMissions = CreateWindow("BUTTON", "MISSIONS", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 440, 150, 25, hwnd, (HMENU)ID_BTN_MISSIONS, NULL, NULL);
            
            hBtnUpgEngine = CreateWindow("BUTTON", "Upg Eng", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 380, 95, 25, hwnd, (HMENU)ID_BTN_UPG_ENGINE, NULL, NULL);
            hBtnUpgCargo = CreateWindow("BUTTON", "Upg Cargo", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 380, 95, 25, hwnd, (HMENU)ID_BTN_UPG_CARGO, NULL, NULL);
            hBtnUpgWeapon = CreateWindow("BUTTON", "Upg Wpn", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 410, 95, 25, hwnd, (HMENU)ID_BTN_UPG_WEAPON, NULL, NULL);
            hBtnUpgShield = CreateWindow("BUTTON", "Upg Shld", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 410, 95, 25, hwnd, (HMENU)ID_BTN_UPG_SHIELD, NULL, NULL);

            hBtnCombatAttack = CreateWindow("BUTTON", "Attack", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 280, 95, 25, hwnd, (HMENU)ID_BTN_COMBAT_ATTACK, NULL, NULL);
            hBtnCombatEvade = CreateWindow("BUTTON", "Evade", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 280, 95, 25, hwnd, (HMENU)ID_BTN_COMBAT_EVADE, NULL, NULL);
            hBtnCombatUseTech = CreateWindow("BUTTON", "Use Tech", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 310, 95, 25, hwnd, (HMENU)ID_BTN_COMBAT_USE_TECH, NULL, NULL);
            hBtnCombatFlee = CreateWindow("BUTTON", "Flee", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 310, 95, 25, hwnd, (HMENU)ID_BTN_COMBAT_FLEE, NULL, NULL);

            hBtnMissionAccept1 = CreateWindow("BUTTON", "Accept M1", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 280, 95, 25, hwnd, (HMENU)ID_BTN_MISSION_ACCEPT_1, NULL, NULL);
            hBtnMissionAccept2 = CreateWindow("BUTTON", "Accept M2", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 280, 95, 25, hwnd, (HMENU)ID_BTN_MISSION_ACCEPT_2, NULL, NULL);
            hBtnMissionAbandon = CreateWindow("BUTTON", "Abandon", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 310, 95, 25, hwnd, (HMENU)ID_BTN_MISSION_ABANDON, NULL, NULL);
            hBtnMissionBack = CreateWindow("BUTTON", "Back", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 350, 150, 25, hwnd, (HMENU)ID_BTN_MISSION_BACK, NULL, NULL);

            ShowWindow(hBtnCourse, SW_HIDE);
            ShowWindow(hBtnMissions, SW_HIDE);
            ShowWindow(hBtnBuyFood, SW_HIDE);
            ShowWindow(hBtnSellFood, SW_HIDE);
            ShowWindow(hBtnBuyMinerals, SW_HIDE);
            ShowWindow(hBtnSellMinerals, SW_HIDE);
            ShowWindow(hBtnBuyTech, SW_HIDE);
            ShowWindow(hBtnSellTech, SW_HIDE);
            ShowWindow(hBtnUpgEngine, SW_HIDE);
            ShowWindow(hBtnUpgCargo, SW_HIDE);
            ShowWindow(hBtnUpgWeapon, SW_HIDE);
            ShowWindow(hBtnUpgShield, SW_HIDE);
            
            ShowWindow(hBtnCombatAttack, SW_HIDE);
            ShowWindow(hBtnCombatEvade, SW_HIDE);
            ShowWindow(hBtnCombatUseTech, SW_HIDE);
            ShowWindow(hBtnCombatFlee, SW_HIDE);

            ShowWindow(hBtnMissionAccept1, SW_HIDE);
            ShowWindow(hBtnMissionAccept2, SW_HIDE);
            ShowWindow(hBtnMissionAbandon, SW_HIDE);
            ShowWindow(hBtnMissionBack, SW_HIDE);
            
            GenerateStationMissions();
            UpdateDashboard();
            return 0;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(0, 255, 204));
            SetBkColor(hdcStatic, RGB(5, 5, 15));
            return (INT_PTR)hBgBrush;
        }
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
            if (pdis->CtlType == ODT_BUTTON) {
                HDC hdc = pdis->hDC;
                RECT rect = pdis->rcItem;
                int state = pdis->itemState;
                
                if (state & ODS_SELECTED) {
                    FillRect(hdc, &rect, CreateSolidBrush(RGB(0, 255, 204)));
                    SetTextColor(hdc, RGB(0, 0, 0));
                    SetBkColor(hdc, RGB(0, 255, 204));
                } else {
                    FillRect(hdc, &rect, CreateSolidBrush(RGB(0, 0, 0)));
                    HBRUSH hBorder = CreateSolidBrush(RGB(0, 255, 204));
                    FrameRect(hdc, &rect, hBorder);
                    DeleteObject(hBorder);
                    SetTextColor(hdc, RGB(0, 255, 204));
                    SetBkColor(hdc, RGB(0, 0, 0));
                }
                char text[32];
                GetWindowText(pdis->hwndItem, text, sizeof(text));
                SelectObject(hdc, hFont);
                DrawText(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }
            return FALSE;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH hMapBg = CreateSolidBrush(RGB(2, 10, 16));
            SelectObject(hdc, hMapBg);
            HPEN hBorderPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 204));
            SelectObject(hdc, hBorderPen);
            Rectangle(hdc, 20, 70, 420, 470);
            
            for(int y = 72; y < 468; y += 4) {
                HPEN hScanLine = CreatePen(PS_SOLID, 1, RGB(0, 50, 50));
                SelectObject(hdc, hScanLine);
                MoveToEx(hdc, 22, y, NULL);
                LineTo(hdc, 418, y);
                DeleteObject(hScanLine);
            }
            
            DeleteObject(hMapBg);
            DeleteObject(hBorderPen);
            
            HBRUSH hGreenBrush = CreateSolidBrush(RGB(0, 255, 0));
            HBRUSH hCyanBrush = CreateSolidBrush(RGB(0, 255, 255));
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 255, 204));
            SelectObject(hdc, hFont);
            
            for (int i = 0; i < 5; i++) {
                int px = 20 + (systems[i].x * 400 / 100);
                int py = 70 + (systems[i].y * 400 / 100);
                
                if (i == selectedSystem) {
                    SelectObject(hdc, hCyanBrush);
                    SelectObject(hdc, GetStockObject(WHITE_PEN));
                } else if (i == currentSystemId) {
                    SelectObject(hdc, hCyanBrush);
                    SelectObject(hdc, GetStockObject(NULL_PEN));
                } else {
                    SelectObject(hdc, hGreenBrush);
                    SelectObject(hdc, GetStockObject(NULL_PEN));
                }
                
                int r = (i == selectedSystem || i == currentSystemId) ? 8 : 6;
                Ellipse(hdc, px - r, py - r, px + r, py + r);
                
                char nameBuf[64];
                if (i == currentSystemId) {
                    sprintf(nameBuf, "%s (HERE)", systems[i].name);
                } else {
                    strcpy(nameBuf, systems[i].name);
                }
                
                if (i == currentSystemId) {
                    SetTextColor(hdc, RGB(0, 255, 255));
                } else {
                    SetTextColor(hdc, RGB(0, 255, 204));
                }
                TextOut(hdc, px + 12, py - 8, nameBuf, strlen(nameBuf));
            }
            
            DeleteObject(hGreenBrush);
            DeleteObject(hCyanBrush);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            if (inCombat) return 0;
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            
            for (int i = 0; i < 5; i++) {
                int px = 20 + (systems[i].x * 400 / 100);
                int py = 70 + (systems[i].y * 400 / 100);
                
                if (abs(mx - px) < 15 && abs(my - py) < 15) {
                    selectedSystem = i;
                    inMissionsView = 0;
                    
                    char infoText[512];
                    if (selectedSystem == currentSystemId) {
                        sprintf(infoText, "%s (DOCKED)\n%s\nMkt:F:%d M:%d T:%d\nInv:F:%d M:%d T:%d\nE:%d/₭%d C:%d/₭%d\nW:%d/₭%d S:%d/₭%d", 
                            systems[i].name, systems[i].desc,
                            systems[i].food_price, systems[i].minerals_price, systems[i].tech_price,
                            cargoFood, cargoMinerals, cargoTech,
                            engineLevel, 1000*engineLevel, cargoLevel, 1500*cargoLevel,
                            weaponLevel, 2000*weaponLevel, shieldLevel, 2000*shieldLevel);
                    } else {
                        sprintf(infoText, "%s\n\n%s\n\nCoordinates: X:%d Y:%d", 
                            systems[i].name, systems[i].desc, systems[i].x, systems[i].y);
                    }
                        
                    SetWindowText(hInfoArea, infoText);
                    if (selectedSystem != currentSystemId) {
                        ShowWindow(hBtnCourse, SW_SHOW);
                        ShowWindow(hBtnMissions, SW_HIDE);
                        ShowWindow(hBtnBuyFood, SW_HIDE);
                        ShowWindow(hBtnSellFood, SW_HIDE);
                        ShowWindow(hBtnBuyMinerals, SW_HIDE);
                        ShowWindow(hBtnSellMinerals, SW_HIDE);
                        ShowWindow(hBtnBuyTech, SW_HIDE);
                        ShowWindow(hBtnSellTech, SW_HIDE);
                        ShowWindow(hBtnUpgEngine, SW_HIDE);
                        ShowWindow(hBtnUpgCargo, SW_HIDE);
                        ShowWindow(hBtnUpgWeapon, SW_HIDE);
                        ShowWindow(hBtnUpgShield, SW_HIDE);
                        
                        ShowWindow(hBtnMissionAccept1, SW_HIDE);
                        ShowWindow(hBtnMissionAccept2, SW_HIDE);
                        ShowWindow(hBtnMissionAbandon, SW_HIDE);
                        ShowWindow(hBtnMissionBack, SW_HIDE);
                    } else {
                        ShowStationView(hwnd);
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                }
            }
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_SET_COURSE && selectedSystem != -1) {
                int dx = systems[selectedSystem].x - systems[currentSystemId].x;
                int dy = systems[selectedSystem].y - systems[currentSystemId].y;
                int dist = (int)sqrt((double)(dx * dx + dy * dy));
                int fuelCost = (dist / 2) / engineLevel;
                if (fuelCost < 1) fuelCost = 1;
                
                if (fuel < fuelCost) {
                    char msg[128];
                    sprintf(msg, "Not enough fuel! You need %d%% fuel.", fuelCost);
                    MessageBox(hwnd, msg, "Warning", MB_OK | MB_ICONWARNING);
                } else {
                    fuel -= fuelCost;
                    currentSystemId = selectedSystem;
                    GenerateStationMissions();
                    
                    int forcedCombat = 0;
                    char missionMsg[256] = "";
                    if (activeMission.type != 0 && activeMission.targetId == currentSystemId) {
                        if (activeMission.type == 1) {
                            credits += activeMission.reward;
                            sprintf(missionMsg, "\n\nMission Complete: Delivery made. Earned ₭%d.", activeMission.reward);
                            activeMission.type = 0;
                        } else if (activeMission.type == 2) {
                            forcedCombat = 1;
                        }
                    }
                    
                    char resultMsg[1024];
                    sprintf(resultMsg, "Traveled to %s.\nFuel consumed: %d%%.%s", systems[currentSystemId].name, fuelCost, missionMsg);
                    
                    int randomEncounter = rand() % 100;
                    if (forcedCombat || randomEncounter < 30) {
                        if (forcedCombat) MessageBox(hwnd, "Alert: Bounty target located! Intercepting pirate vessel!", "Navigation Log", MB_OK);
                        else MessageBox(hwnd, "Alert: Hostile pirate vessel encountered!", "Navigation Log", MB_OK);
                        
                        inCombat = 1;
                        enemyMaxHull = 30 + currentSystemId * 20;
                        enemyHull = enemyMaxHull;
                        strcpy(combatLog, "Pirate intercepts your ship!\nTactical combat initiated.\n");
                        
                        ShowWindow(hBtnCourse, SW_HIDE);
                        ShowWindow(hBtnMissions, SW_HIDE);
                        ShowWindow(hBtnBuyFood, SW_HIDE);
                        ShowWindow(hBtnSellFood, SW_HIDE);
                        ShowWindow(hBtnBuyMinerals, SW_HIDE);
                        ShowWindow(hBtnSellMinerals, SW_HIDE);
                        ShowWindow(hBtnBuyTech, SW_HIDE);
                        ShowWindow(hBtnSellTech, SW_HIDE);
                        ShowWindow(hBtnUpgEngine, SW_HIDE);
                        ShowWindow(hBtnUpgCargo, SW_HIDE);
                        ShowWindow(hBtnUpgWeapon, SW_HIDE);
                        ShowWindow(hBtnUpgShield, SW_HIDE);
                        
                        ShowWindow(hBtnCombatAttack, SW_SHOW);
                        ShowWindow(hBtnCombatEvade, SW_SHOW);
                        ShowWindow(hBtnCombatUseTech, SW_SHOW);
                        ShowWindow(hBtnCombatFlee, SW_SHOW);
                        
                        UpdateCombatUI(hwnd);
                        return 0;
                    } else if (randomEncounter < 50) {
                        strcat(resultMsg, "\n\nNotice: You found drifting debris.");
                        int found = (rand() % 100) + 20;
                        credits += found;
                        char foundMsg[64];
                        sprintf(foundMsg, "\nSalvaged %d credits.", found);
                        strcat(resultMsg, foundMsg);
                    } else {
                        strcat(resultMsg, "\n\nThe journey was uneventful.");
                    }
                    
                    UpdateDashboard();
                    
                    MessageBox(hwnd, resultMsg, "Navigation Log", MB_OK);
                    
                    SetWindowText(hInfoArea, "Select a system on the map for details.");
                    ShowWindow(hBtnCourse, SW_HIDE);
                    selectedSystem = -1;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) >= ID_BTN_COMBAT_ATTACK && LOWORD(wParam) <= ID_BTN_COMBAT_FLEE && inCombat) {
                int cmd = LOWORD(wParam);
                int playerEvading = 0;
                char turnMsg[256] = "";
                
                if (cmd == ID_BTN_COMBAT_ATTACK) {
                    int dmg = (10 + weaponLevel * 10) * (80 + rand() % 40) / 100;
                    enemyHull -= dmg;
                    sprintf(turnMsg, "You attack and deal %d damage!\n", dmg);
                } else if (cmd == ID_BTN_COMBAT_EVADE) {
                    playerEvading = 1;
                    sprintf(turnMsg, "You take evasive maneuvers.\n");
                } else if (cmd == ID_BTN_COMBAT_USE_TECH) {
                    if (cargoTech > 0) {
                        cargoTech--;
                        hull += 40;
                        if (hull > maxHull) hull = maxHull;
                        sprintf(turnMsg, "Used 1 Tech cargo to repair hull.\n");
                    } else {
                        sprintf(turnMsg, "No Tech cargo available to use!\n");
                    }
                } else if (cmd == ID_BTN_COMBAT_FLEE) {
                    if (rand() % 100 < 50) {
                        MessageBox(hwnd, "Successfully escaped!", "Flee", MB_OK);
                        EndCombat(hwnd);
                        return 0;
                    } else {
                        sprintf(turnMsg, "Failed to escape!\n");
                    }
                }
                
                if (enemyHull <= 0) {
                    int bounty = 100 + currentSystemId * 50;
                    credits += bounty;
                    char winMsg[256];
                    sprintf(winMsg, "Pirate destroyed! Claimed ₭%d standard bounty.", bounty);
                    if (activeMission.type == 2 && activeMission.targetId == currentSystemId) {
                        credits += activeMission.reward;
                        char bMsg[128];
                        sprintf(bMsg, "\nMission Complete! Earned extra ₭%d.", activeMission.reward);
                        strcat(winMsg, bMsg);
                        activeMission.type = 0;
                    }
                    MessageBox(hwnd, winMsg, "Combat Won", MB_OK);
                    EndCombat(hwnd);
                    return 0;
                }
                
                int hitChance = playerEvading ? 50 : 90;
                if (rand() % 100 < hitChance) {
                    int eDmg = (5 + currentSystemId * 5) * (80 + rand() % 40) / 100;
                    hull -= eDmg;
                    char hitMsg[64];
                    sprintf(hitMsg, "Pirate hits you for %d damage!\n", eDmg);
                    strcat(turnMsg, hitMsg);
                } else {
                    strcat(turnMsg, "Pirate's attack misses!\n");
                }
                
                strcpy(combatLog, turnMsg);
                UpdateCombatUI(hwnd);
                
                if (hull <= 0) {
                    MessageBox(hwnd, "CRITICAL: Hull integrity lost! Emergency tow to Sol.", "Defeat", MB_OK);
                    hull = maxHull;
                    credits -= 500;
                    if (credits < 0) credits = 0;
                    currentSystemId = 0;
                    EndCombat(hwnd);
                }
                return 0;
            } else if (LOWORD(wParam) >= ID_BTN_BUY_FOOD && LOWORD(wParam) <= ID_BTN_SELL_TECH && selectedSystem == currentSystemId) {
                int used = cargoFood + cargoMinerals + cargoTech;
                int cmd = LOWORD(wParam);
                System *sys = &systems[currentSystemId];
                
                if (cmd == ID_BTN_BUY_FOOD) {
                    if (credits >= sys->food_price && used < cargoMax) { credits -= sys->food_price; cargoFood++; }
                } else if (cmd == ID_BTN_SELL_FOOD) {
                    if (cargoFood > 0) { credits += sys->food_price; cargoFood--; }
                } else if (cmd == ID_BTN_BUY_MINERALS) {
                    if (credits >= sys->minerals_price && used < cargoMax) { credits -= sys->minerals_price; cargoMinerals++; }
                } else if (cmd == ID_BTN_SELL_MINERALS) {
                    if (cargoMinerals > 0) { credits += sys->minerals_price; cargoMinerals--; }
                } else if (cmd == ID_BTN_BUY_TECH) {
                    if (credits >= sys->tech_price && used < cargoMax) { credits -= sys->tech_price; cargoTech++; }
                } else if (cmd == ID_BTN_SELL_TECH) {
                    if (cargoTech > 0) { credits += sys->tech_price; cargoTech--; }
                }
                UpdateDashboard();
                
                char infoText[512];
                sprintf(infoText, "%s (DOCKED)\n%s\nMkt:F:%d M:%d T:%d\nInv:F:%d M:%d T:%d\nE:%d/₭%d C:%d/₭%d\nW:%d/₭%d S:%d/₭%d", 
                    sys->name, sys->desc,
                    sys->food_price, sys->minerals_price, sys->tech_price,
                    cargoFood, cargoMinerals, cargoTech,
                    engineLevel, 1000*engineLevel, cargoLevel, 1500*cargoLevel,
                    weaponLevel, 2000*weaponLevel, shieldLevel, 2000*shieldLevel);
                SetWindowText(hInfoArea, infoText);
            } else if (LOWORD(wParam) >= ID_BTN_UPG_ENGINE && LOWORD(wParam) <= ID_BTN_UPG_SHIELD && selectedSystem == currentSystemId) {
                int cmd = LOWORD(wParam);
                if (cmd == ID_BTN_UPG_ENGINE) {
                    int cost = 1000 * engineLevel;
                    if (credits >= cost) { credits -= cost; engineLevel++; }
                } else if (cmd == ID_BTN_UPG_CARGO) {
                    int cost = 1500 * cargoLevel;
                    if (credits >= cost) { credits -= cost; cargoLevel++; cargoMax = 50 * cargoLevel; }
                } else if (cmd == ID_BTN_UPG_WEAPON) {
                    int cost = 2000 * weaponLevel;
                    if (credits >= cost) { credits -= cost; weaponLevel++; }
                } else if (cmd == ID_BTN_UPG_SHIELD) {
                    int cost = 2000 * shieldLevel;
                    if (credits >= cost) { credits -= cost; shieldLevel++; maxHull = 100 + (shieldLevel - 1) * 50; hull = maxHull; }
                }
                
                UpdateDashboard();
                
                char infoText[512];
                System *sys = &systems[currentSystemId];
                sprintf(infoText, "%s (DOCKED)\n%s\nMkt:F:%d M:%d T:%d\nInv:F:%d M:%d T:%d\nE:%d/₭%d C:%d/₭%d\nW:%d/₭%d S:%d/₭%d", 
                    sys->name, sys->desc,
                    sys->food_price, sys->minerals_price, sys->tech_price,
                    cargoFood, cargoMinerals, cargoTech,
                    engineLevel, 1000*engineLevel, cargoLevel, 1500*cargoLevel,
                    weaponLevel, 2000*weaponLevel, shieldLevel, 2000*shieldLevel);
                SetWindowText(hInfoArea, infoText);
            } else if (LOWORD(wParam) == ID_BTN_MISSIONS) {
                ShowMissionsView(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_MISSION_ACCEPT_1) {
                activeMission = stationMissions[0];
                stationMissions[0].type = 0;
                UpdateDashboard();
                ShowMissionsView(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_MISSION_ACCEPT_2) {
                activeMission = stationMissions[1];
                stationMissions[1].type = 0;
                UpdateDashboard();
                ShowMissionsView(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_MISSION_ABANDON) {
                activeMission.type = 0;
                UpdateDashboard();
                ShowMissionsView(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_MISSION_BACK) {
                ShowStationView(hwnd);
            }
            return 0;
        }
        case WM_DESTROY: {
            if (hFont) DeleteObject(hFont);
            if (hBgBrush) DeleteObject(hBgBrush);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KStellar Window Class";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(5, 5, 15));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "KStellar Phase 3",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 680, 520,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
