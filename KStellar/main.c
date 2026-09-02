#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define ID_BTN_SET_COURSE 101

#define MAX_SYSTEMS 30
typedef struct {
    int id;
    char name[32];
    char sector[16];
    int x;
    int y;
    char desc[256];
    char economy[32];
    int food_price;
    int minerals_price;
    int tech_price;
    int phenomenon; // 0=None, 1=Black Hole, 2=Solar Flare, 3=Derelict Ship
} System;

System systems[MAX_SYSTEMS];

typedef struct {
    char name[32];
    float food;
    float min;
    float tech;
} EconomyType;

void GenerateGalaxy() {
    const char* sysNames[] = {"Aegis", "Borealis", "Cygnus", "Draco", "Eridanus", "Fenrir", "Goliath", "Helios", "Icarus", "Juno", "Krypton", "Lyra", "Orion", "Pegasus", "Rigel", "Sirius", "Taurus", "Ursa", "Vega", "Wolf"};
    const char* sectorNames[] = {"Alpha", "Beta", "Gamma", "Delta", "Omega"};
    const char* suffixes[] = {"Prime", "Secundus", "Tertius", "Minor", "Major", "Station", "Outpost", "Belt"};

    EconomyType economies[] = {
        {"Agricultural", 0.5f, 1.5f, 1.8f},
        {"Mining", 1.5f, 0.5f, 1.5f},
        {"Industrial", 1.3f, 1.2f, 0.8f},
        {"Tech Hub", 1.5f, 1.5f, 0.5f},
        {"Trading Hub", 1.0f, 1.0f, 1.0f}
    };

    systems[0].id = 1;
    strcpy(systems[0].name, "Sol");
    strcpy(systems[0].sector, "Alpha");
    systems[0].x = 50;
    systems[0].y = 50;
    strcpy(systems[0].desc, "Cradle of humanity. Highly developed planetary system.");
    strcpy(systems[0].economy, "Trading Hub");
    systems[0].food_price = 20;
    systems[0].minerals_price = 40;
    systems[0].tech_price = 150;
    systems[0].phenomenon = 0;

    for (int i = 1; i < MAX_SYSTEMS; i++) {
        systems[i].id = i + 1;
        const char* sec = sectorNames[rand() % 5];
        const char* prefix = sysNames[rand() % 20];
        const char* suffix = suffixes[rand() % 8];
        sprintf(systems[i].name, "%s %s", prefix, suffix);
        strcpy(systems[i].sector, sec);
        systems[i].x = 5 + (rand() % 90);
        systems[i].y = 5 + (rand() % 90);
        
        int ecoIdx = rand() % 5;
        strcpy(systems[i].economy, economies[ecoIdx].name);
        sprintf(systems[i].desc, "A %s system in the %s sector.", economies[ecoIdx].name, sec);
        
        int bFood = 20 + (rand() % 10);
        int bMin = 40 + (rand() % 20);
        int bTech = 100 + (rand() % 50);
        
        systems[i].food_price = (int)(bFood * economies[ecoIdx].food);
        systems[i].minerals_price = (int)(bMin * economies[ecoIdx].min);
        systems[i].tech_price = (int)(bTech * economies[ecoIdx].tech);
        systems[i].phenomenon = (i % 3 == 0) ? (((i / 3) % 3) + 1) : 0;
    }
}

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

#define ID_BTN_INVESTIGATE 140

#define ID_BTN_FACTIONS 150
#define ID_BTN_FACT_DONATE 151
#define ID_BTN_FACT_DUES 152
#define ID_BTN_FACT_BRIBE 153
#define ID_BTN_FACT_BACK 154

#define ID_BTN_SOUND_TOGGLE 160
#define ID_BTN_DRONE_TOGGLE 161

#define SFX_BLIP 1
#define SFX_WARP 2
#define SFX_LASER 3
#define SFX_ENEMY_LASER 4
#define SFX_UPGRADE 5
#define SFX_EXPLOSION 6
#define SFX_PHENOM 7
#define SFX_HEAL 8

HWND hBtnSoundToggle, hBtnDroneToggle;

int soundEnabled = 1;
int droneEnabled = 1;
HANDLE hDroneThread = NULL;
volatile int droneRunning = 1;

extern int inCombat;

DWORD WINAPI SoundEffectThread(LPVOID lpParam) {
    if (!soundEnabled) return 0;
    int type = (int)(intptr_t)lpParam;
    if (type == SFX_BLIP) {
        Beep(1200, 30);
    } else if (type == SFX_WARP) {
        Beep(160, 50);
        Beep(260, 60);
        Beep(420, 80);
        Beep(650, 100);
        Beep(320, 100);
    } else if (type == SFX_LASER) {
        Beep(1100, 30);
        Beep(700, 35);
        Beep(350, 45);
    } else if (type == SFX_ENEMY_LASER) {
        Beep(380, 40);
        Beep(180, 60);
    } else if (type == SFX_UPGRADE) {
        Beep(523, 70);
        Beep(659, 70);
        Beep(784, 70);
        Beep(1046, 120);
    } else if (type == SFX_EXPLOSION) {
        Beep(180, 70);
        Beep(120, 90);
        Beep(70, 140);
    } else if (type == SFX_PHENOM) {
        Beep(440, 80);
        Beep(660, 80);
        Beep(880, 100);
        Beep(1320, 120);
    } else if (type == SFX_HEAL) {
        Beep(500, 60);
        Beep(850, 90);
    }
    return 0;
}

void PlaySfx(int type) {
    if (!soundEnabled) return;
    CreateThread(NULL, 0, SoundEffectThread, (LPVOID)(intptr_t)type, 0, NULL);
}

DWORD WINAPI AmbientDroneThread(LPVOID lpParam) {
    while (droneRunning) {
        if (soundEnabled && droneEnabled && !inCombat) {
            Beep(55, 120);
            Sleep(2500);
            if (soundEnabled && droneEnabled && !inCombat) {
                Beep(65, 90);
                Sleep(2500);
            }
        } else {
            Sleep(300);
        }
    }
    return 0;
}

typedef struct {
    int type; // 0=None, 1=Delivery, 2=Bounty
    int targetId;
    int reward;
} Mission;

HWND hMapArea, hInfoArea, hBtnCourse, hFuelText, hCreditsText, hCargoText, hHullText, hMissionText, hFactionText;
HWND hBtnBuyFood, hBtnSellFood, hBtnBuyMinerals, hBtnSellMinerals, hBtnBuyTech, hBtnSellTech;
HWND hBtnUpgEngine, hBtnUpgCargo, hBtnUpgWeapon, hBtnUpgShield;
HWND hBtnCombatAttack, hBtnCombatEvade, hBtnCombatUseTech, hBtnCombatFlee;
HWND hBtnMissions, hBtnMissionAccept1, hBtnMissionAccept2, hBtnMissionAbandon, hBtnMissionBack;
HWND hBtnFactions, hBtnFactDonate, hBtnFactDues, hBtnFactBribe, hBtnFactBack;
HWND hBtnInvestigate;

int selectedSystem = -1;
int currentSystemId = 0;
int inMissionsView = 0;
int inFactionsView = 0;

int repFed = 10;
int repTraders = 15;
int repPirates = -15;

const char* GetFactionTier(int rep) {
    if (rep >= 50) return "Allied";
    if (rep >= 15) return "Friendly";
    if (rep > -15) return "Neutral";
    if (rep > -50) return "Unfriendly";
    return "Hostile";
}

void AdjustRep(int fed, int trd, int pir) {
    repFed += fed;
    if (repFed > 100) repFed = 100;
    if (repFed < -100) repFed = -100;
    repTraders += trd;
    if (repTraders > 100) repTraders = 100;
    if (repTraders < -100) repTraders = -100;
    repPirates += pir;
    if (repPirates > 100) repPirates = 100;
    if (repPirates < -100) repPirates = -100;
}

int GetBuyPrice(int basePrice) {
    int p = basePrice;
    if (repTraders >= 15) p = p * 9 / 10;
    else if (repTraders <= -15) p = p * 11 / 10;
    return (p < 1) ? 1 : p;
}

int GetSellPrice(int basePrice) {
    int p = basePrice;
    if (repTraders >= 15) p = p * 11 / 10;
    else if (repTraders <= -15) p = p * 9 / 10;
    return (p < 1) ? 1 : p;
}

Mission activeMission = {0, -1, 0};
Mission stationMissions[2] = {{0,-1,0}, {0,-1,0}};

void GenerateStationMissions() {
    for (int i=0; i<2; i++) {
        stationMissions[i].type = (rand() % 2) + 1;
        int target = rand() % MAX_SYSTEMS;
        if (target == currentSystemId) {
            target = (target + 1) % MAX_SYSTEMS;
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

    char fBuf[64];
    sprintf(fBuf, "F:%+d T:%+d P:%+d", repFed, repTraders, repPirates);
    SetWindowText(hFactionText, fBuf);
    
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
    inFactionsView = 0;
    
    ShowWindow(hBtnMissionAccept1, SW_HIDE);
    ShowWindow(hBtnMissionAccept2, SW_HIDE);
    ShowWindow(hBtnMissionAbandon, SW_HIDE);
    ShowWindow(hBtnMissionBack, SW_HIDE);

    ShowWindow(hBtnFactDonate, SW_HIDE);
    ShowWindow(hBtnFactDues, SW_HIDE);
    ShowWindow(hBtnFactBribe, SW_HIDE);
    ShowWindow(hBtnFactBack, SW_HIDE);
    
    ShowWindow(hBtnMissions, SW_SHOW);
    ShowWindow(hBtnFactions, SW_SHOW);
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
    char phenomStr[64] = "";
    if (sys->phenomenon == 1) {
        strcpy(phenomStr, "\n[★ PHENOMENON: Black Hole]");
        SetWindowText(hBtnInvestigate, "Probe BH");
        ShowWindow(hBtnInvestigate, SW_SHOW);
    } else if (sys->phenomenon == 2) {
        strcpy(phenomStr, "\n[★ PHENOMENON: Solar Flare]");
        SetWindowText(hBtnInvestigate, "Harvest");
        ShowWindow(hBtnInvestigate, SW_SHOW);
    } else if (sys->phenomenon == 3) {
        strcpy(phenomStr, "\n[★ PHENOMENON: Derelict Ship]");
        SetWindowText(hBtnInvestigate, "Salvage");
        ShowWindow(hBtnInvestigate, SW_SHOW);
    } else {
        ShowWindow(hBtnInvestigate, SW_HIDE);
    }

    char infoText[512];
    sprintf(infoText, "%s (DOCKED)%s\nSec: %s | Eco: %s\nBuy: F:%d M:%d T:%d\nSel: F:%d M:%d T:%d\nInv: F:%d M:%d T:%d\nE:%d/₭%d C:%d/₭%d\nW:%d/₭%d S:%d/₭%d", 
        sys->name, phenomStr, sys->sector, sys->economy,
        GetBuyPrice(sys->food_price), GetBuyPrice(sys->minerals_price), GetBuyPrice(sys->tech_price),
        GetSellPrice(sys->food_price), GetSellPrice(sys->minerals_price), GetSellPrice(sys->tech_price),
        cargoFood, cargoMinerals, cargoTech,
        engineLevel, 1000*engineLevel, cargoLevel, 1500*cargoLevel,
        weaponLevel, 2000*weaponLevel, shieldLevel, 2000*shieldLevel);
    SetWindowText(hInfoArea, infoText);
}

void ShowMissionsView(HWND hwnd) {
    inMissionsView = 1;
    inFactionsView = 0;
    
    ShowWindow(hBtnMissions, SW_HIDE);
    ShowWindow(hBtnFactions, SW_HIDE);
    ShowWindow(hBtnInvestigate, SW_HIDE);
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

    ShowWindow(hBtnFactDonate, SW_HIDE);
    ShowWindow(hBtnFactDues, SW_HIDE);
    ShowWindow(hBtnFactBribe, SW_HIDE);
    ShowWindow(hBtnFactBack, SW_HIDE);
    
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

void ShowFactionsView(HWND hwnd) {
    inFactionsView = 1;
    inMissionsView = 0;

    ShowWindow(hBtnMissions, SW_HIDE);
    ShowWindow(hBtnFactions, SW_HIDE);
    ShowWindow(hBtnInvestigate, SW_HIDE);
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

    ShowWindow(hBtnMissionAccept1, SW_HIDE);
    ShowWindow(hBtnMissionAccept2, SW_HIDE);
    ShowWindow(hBtnMissionAbandon, SW_HIDE);
    ShowWindow(hBtnMissionBack, SW_HIDE);

    ShowWindow(hBtnFactDonate, SW_SHOW);
    ShowWindow(hBtnFactDues, SW_SHOW);
    ShowWindow(hBtnFactBribe, SW_SHOW);
    ShowWindow(hBtnFactBack, SW_SHOW);

    char infoText[1024];
    sprintf(infoText, "--- FACTIONS & DIPLOMACY ---\n\n"
                      "FEDERATION: %+d (%s)\n"
                      "Perk: +20%% Attack & +15%% Bounty (≥+15)\n\n"
                      "TRADERS: %+d (%s)\n"
                      "Perk: 10%% Buy disc / Sell markup (≥+15)\n\n"
                      "PIRATES: %+d (%s)\n"
                      "Perk: Safe passage & Tribute (≥+15)\n",
                      repFed, GetFactionTier(repFed),
                      repTraders, GetFactionTier(repTraders),
                      repPirates, GetFactionTier(repPirates));
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
    ShowWindow(hBtnInvestigate, SW_HIDE);
    
    SetWindowText(hInfoArea, "Select a system on the map for details.");
    selectedSystem = -1;
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            srand(GetTickCount());
            GenerateGalaxy();
            hFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
            hBgBrush = CreateSolidBrush(RGB(5, 5, 15));
            
            HWND h0 = CreateWindow("STATIC", "HULL: 100/100", WS_VISIBLE | WS_CHILD, 20, 20, 110, 20, hwnd, NULL, NULL, NULL);
            hHullText = h0;
            SendMessage(h0, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND h1 = CreateWindow("STATIC", "FUEL: 100%", WS_VISIBLE | WS_CHILD, 135, 20, 90, 20, hwnd, NULL, NULL, NULL);
            hFuelText = h1;
            SendMessage(h1, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND h2 = CreateWindow("STATIC", "CREDITS: 1,000", WS_VISIBLE | WS_CHILD, 230, 20, 120, 20, hwnd, NULL, NULL, NULL);
            hCreditsText = h2;
            SendMessage(h2, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND h3 = CreateWindow("STATIC", "CARGO: 0/50 TONS", WS_VISIBLE | WS_CHILD, 355, 20, 115, 20, hwnd, NULL, NULL, NULL);
            hCargoText = h3;
            SendMessage(h3, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hFactionText = CreateWindow("STATIC", "F:+10 T:+15 P:-15", WS_VISIBLE | WS_CHILD, 475, 20, 195, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hFactionText, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND h4 = CreateWindow("STATIC", "LOCAL SYSTEMS", WS_VISIBLE | WS_CHILD, 440, 60, 150, 20, hwnd, NULL, NULL, NULL);
            SendMessage(h4, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hMissionText = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD, 20, 45, 435, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hMissionText, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnSoundToggle = CreateWindow("BUTTON", "SND: ON", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 465, 42, 95, 22, hwnd, (HMENU)ID_BTN_SOUND_TOGGLE, NULL, NULL);
            hBtnDroneToggle = CreateWindow("BUTTON", "DRN: ON", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 570, 42, 95, 22, hwnd, (HMENU)ID_BTN_DRONE_TOGGLE, NULL, NULL);

            hDroneThread = CreateThread(NULL, 0, AmbientDroneThread, NULL, 0, NULL);
            
            hInfoArea = CreateWindow("STATIC", "Select a system on the map for details.", WS_VISIBLE | WS_CHILD, 440, 85, 230, 185, hwnd, NULL, NULL, NULL);
            SendMessage(hInfoArea, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnBuyFood = CreateWindow("BUTTON", "Buy Food", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 280, 95, 25, hwnd, (HMENU)ID_BTN_BUY_FOOD, NULL, NULL);
            hBtnSellFood = CreateWindow("BUTTON", "Sell Food", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 280, 95, 25, hwnd, (HMENU)ID_BTN_SELL_FOOD, NULL, NULL);
            
            hBtnBuyMinerals = CreateWindow("BUTTON", "Buy Min", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 310, 95, 25, hwnd, (HMENU)ID_BTN_BUY_MINERALS, NULL, NULL);
            hBtnSellMinerals = CreateWindow("BUTTON", "Sell Min", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 310, 95, 25, hwnd, (HMENU)ID_BTN_SELL_MINERALS, NULL, NULL);
            
            hBtnBuyTech = CreateWindow("BUTTON", "Buy Tech", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 340, 95, 25, hwnd, (HMENU)ID_BTN_BUY_TECH, NULL, NULL);
            hBtnSellTech = CreateWindow("BUTTON", "Sell Tech", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 340, 95, 25, hwnd, (HMENU)ID_BTN_SELL_TECH, NULL, NULL);

            hBtnCourse = CreateWindow("BUTTON", "Set Course", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 380, 195, 30, hwnd, (HMENU)ID_BTN_SET_COURSE, NULL, NULL);
            hBtnMissions = CreateWindow("BUTTON", "MISSIONS", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 440, 95, 25, hwnd, (HMENU)ID_BTN_MISSIONS, NULL, NULL);
            hBtnFactions = CreateWindow("BUTTON", "FACTIONS", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 440, 95, 25, hwnd, (HMENU)ID_BTN_FACTIONS, NULL, NULL);
            hBtnInvestigate = CreateWindow("BUTTON", "Anomaly", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 470, 195, 25, hwnd, (HMENU)ID_BTN_INVESTIGATE, NULL, NULL);
            
            hBtnUpgEngine = CreateWindow("BUTTON", "Upg Eng", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 375, 95, 25, hwnd, (HMENU)ID_BTN_UPG_ENGINE, NULL, NULL);
            hBtnUpgCargo = CreateWindow("BUTTON", "Upg Cargo", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 375, 95, 25, hwnd, (HMENU)ID_BTN_UPG_CARGO, NULL, NULL);
            hBtnUpgWeapon = CreateWindow("BUTTON", "Upg Wpn", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 405, 95, 25, hwnd, (HMENU)ID_BTN_UPG_WEAPON, NULL, NULL);
            hBtnUpgShield = CreateWindow("BUTTON", "Upg Shld", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 405, 95, 25, hwnd, (HMENU)ID_BTN_UPG_SHIELD, NULL, NULL);

            hBtnCombatAttack = CreateWindow("BUTTON", "Attack", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 280, 95, 25, hwnd, (HMENU)ID_BTN_COMBAT_ATTACK, NULL, NULL);
            hBtnCombatEvade = CreateWindow("BUTTON", "Evade", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 280, 95, 25, hwnd, (HMENU)ID_BTN_COMBAT_EVADE, NULL, NULL);
            hBtnCombatUseTech = CreateWindow("BUTTON", "Use Tech", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 310, 95, 25, hwnd, (HMENU)ID_BTN_COMBAT_USE_TECH, NULL, NULL);
            hBtnCombatFlee = CreateWindow("BUTTON", "Flee", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 310, 95, 25, hwnd, (HMENU)ID_BTN_COMBAT_FLEE, NULL, NULL);

            hBtnMissionAccept1 = CreateWindow("BUTTON", "Accept M1", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 280, 95, 25, hwnd, (HMENU)ID_BTN_MISSION_ACCEPT_1, NULL, NULL);
            hBtnMissionAccept2 = CreateWindow("BUTTON", "Accept M2", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 540, 280, 95, 25, hwnd, (HMENU)ID_BTN_MISSION_ACCEPT_2, NULL, NULL);
            hBtnMissionAbandon = CreateWindow("BUTTON", "Abandon", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 310, 95, 25, hwnd, (HMENU)ID_BTN_MISSION_ABANDON, NULL, NULL);
            hBtnMissionBack = CreateWindow("BUTTON", "Back", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 350, 195, 25, hwnd, (HMENU)ID_BTN_MISSION_BACK, NULL, NULL);

            hBtnFactDonate = CreateWindow("BUTTON", "Donate Fed (350)", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 280, 195, 25, hwnd, (HMENU)ID_BTN_FACT_DONATE, NULL, NULL);
            hBtnFactDues = CreateWindow("BUTTON", "Guild Dues (300)", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 310, 195, 25, hwnd, (HMENU)ID_BTN_FACT_DUES, NULL, NULL);
            hBtnFactBribe = CreateWindow("BUTTON", "Bribe Pirate (400)", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 340, 195, 25, hwnd, (HMENU)ID_BTN_FACT_BRIBE, NULL, NULL);
            hBtnFactBack = CreateWindow("BUTTON", "Back", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 375, 195, 25, hwnd, (HMENU)ID_BTN_FACT_BACK, NULL, NULL);

            ShowWindow(hBtnCourse, SW_HIDE);
            ShowWindow(hBtnMissions, SW_HIDE);
            ShowWindow(hBtnFactions, SW_HIDE);
            ShowWindow(hBtnInvestigate, SW_HIDE);
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

            ShowWindow(hBtnFactDonate, SW_HIDE);
            ShowWindow(hBtnFactDues, SW_HIDE);
            ShowWindow(hBtnFactBribe, SW_HIDE);
            ShowWindow(hBtnFactBack, SW_HIDE);
            
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
                char text[64];
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
            HPEN hBlackHolePen = CreatePen(PS_DOT, 1, RGB(255, 0, 255));
            HPEN hSolarPen = CreatePen(PS_DOT, 1, RGB(255, 180, 0));
            HPEN hDerelictPen = CreatePen(PS_DOT, 1, RGB(160, 210, 255));
            HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 255, 204));
            SelectObject(hdc, hFont);
            
            for (int i = 0; i < MAX_SYSTEMS; i++) {
                int px = 20 + (systems[i].x * 400 / 100);
                int py = 70 + (systems[i].y * 400 / 100);

                if (systems[i].phenomenon != 0) {
                    SelectObject(hdc, hNullBrush);
                    if (systems[i].phenomenon == 1) SelectObject(hdc, hBlackHolePen);
                    else if (systems[i].phenomenon == 2) SelectObject(hdc, hSolarPen);
                    else if (systems[i].phenomenon == 3) SelectObject(hdc, hDerelictPen);
                    int pr = (i == selectedSystem || i == currentSystemId) ? 12 : 8;
                    Ellipse(hdc, px - pr, py - pr, px + pr, py + pr);
                }
                
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
                
                int r = (i == selectedSystem || i == currentSystemId) ? 8 : 4;
                Ellipse(hdc, px - r, py - r, px + r, py + r);
                
                if (i == selectedSystem || i == currentSystemId) {
                    char nameBuf[64];
                    if (i == currentSystemId) {
                        sprintf(nameBuf, "%s (HERE)", systems[i].name);
                        SetTextColor(hdc, RGB(0, 255, 255));
                    } else {
                        strcpy(nameBuf, systems[i].name);
                        SetTextColor(hdc, RGB(0, 255, 204));
                    }
                    if (systems[i].phenomenon == 1) strcat(nameBuf, " [BH]");
                    else if (systems[i].phenomenon == 2) strcat(nameBuf, " [FLARE]");
                    else if (systems[i].phenomenon == 3) strcat(nameBuf, " [HULK]");
                    TextOut(hdc, px + 12, py - 8, nameBuf, strlen(nameBuf));
                }
            }
            
            DeleteObject(hGreenBrush);
            DeleteObject(hCyanBrush);
            DeleteObject(hBlackHolePen);
            DeleteObject(hSolarPen);
            DeleteObject(hDerelictPen);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            if (inCombat) return 0;
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            
            for (int i = 0; i < MAX_SYSTEMS; i++) {
                int px = 20 + (systems[i].x * 400 / 100);
                int py = 70 + (systems[i].y * 400 / 100);
                
                if (abs(mx - px) < 15 && abs(my - py) < 15) {
                    PlaySfx(SFX_BLIP);
                    selectedSystem = i;
                    inMissionsView = 0;
                    inFactionsView = 0;
                    
                    char phenomStr[64] = "";
                    if (systems[i].phenomenon == 1) strcpy(phenomStr, "\n[★ PHENOMENON: Black Hole]");
                    else if (systems[i].phenomenon == 2) strcpy(phenomStr, "\n[★ PHENOMENON: Solar Flare]");
                    else if (systems[i].phenomenon == 3) strcpy(phenomStr, "\n[★ PHENOMENON: Derelict Ship]");

                    char infoText[512];
                    if (selectedSystem == currentSystemId) {
                        sprintf(infoText, "%s (DOCKED)%s\nSec: %s | Eco: %s\nBuy: F:%d M:%d T:%d\nSel: F:%d M:%d T:%d\nInv: F:%d M:%d T:%d\nE:%d/₭%d C:%d/₭%d\nW:%d/₭%d S:%d/₭%d", 
                            systems[i].name, phenomStr, systems[i].sector, systems[i].economy,
                            GetBuyPrice(systems[i].food_price), GetBuyPrice(systems[i].minerals_price), GetBuyPrice(systems[i].tech_price),
                            GetSellPrice(systems[i].food_price), GetSellPrice(systems[i].minerals_price), GetSellPrice(systems[i].tech_price),
                            cargoFood, cargoMinerals, cargoTech,
                            engineLevel, 1000*engineLevel, cargoLevel, 1500*cargoLevel,
                            weaponLevel, 2000*weaponLevel, shieldLevel, 2000*shieldLevel);
                    } else {
                        sprintf(infoText, "%s%s\nSector: %s\nEconomy: %s\n%s\n\nCoordinates: X:%d Y:%d", 
                            systems[i].name, phenomStr, systems[i].sector, systems[i].economy, systems[i].desc, systems[i].x, systems[i].y);
                    }
                        
                    SetWindowText(hInfoArea, infoText);
                    if (selectedSystem != currentSystemId) {
                        ShowWindow(hBtnCourse, SW_SHOW);
                        ShowWindow(hBtnMissions, SW_HIDE);
                        ShowWindow(hBtnFactions, SW_HIDE);
                        ShowWindow(hBtnInvestigate, SW_HIDE);
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

                        ShowWindow(hBtnFactDonate, SW_HIDE);
                        ShowWindow(hBtnFactDues, SW_HIDE);
                        ShowWindow(hBtnFactBribe, SW_HIDE);
                        ShowWindow(hBtnFactBack, SW_HIDE);
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
                    PlaySfx(SFX_BLIP);
                    char msg[128];
                    sprintf(msg, "Not enough fuel! You need %d%% fuel.", fuelCost);
                    MessageBox(hwnd, msg, "Warning", MB_OK | MB_ICONWARNING);
                } else {
                    PlaySfx(SFX_WARP);
                    fuel -= fuelCost;
                    currentSystemId = selectedSystem;
                    GenerateStationMissions();
                    
                    int forcedCombat = 0;
                    char missionMsg[256] = "";
                    if (activeMission.type != 0 && activeMission.targetId == currentSystemId) {
                        if (activeMission.type == 1) {
                            credits += activeMission.reward;
                            AdjustRep(2, 8, 0);
                            sprintf(missionMsg, "\n\nMission Complete: Delivery made. Earned ₭%d.\nReputation: +8 Traders Guild, +2 Federation.", activeMission.reward);
                            activeMission.type = 0;
                        } else if (activeMission.type == 2) {
                            forcedCombat = 1;
                        }
                    }
                    
                    char resultMsg[1024];
                    sprintf(resultMsg, "Traveled to %s.\nFuel consumed: %d%%.%s", systems[currentSystemId].name, fuelCost, missionMsg);
                    
                    int randomEncounter = rand() % 100;
                    if (forcedCombat || randomEncounter < 25) {
                        if (forcedCombat) {
                            MessageBox(hwnd, "Alert: Bounty target located! Intercepting pirate vessel!", "Navigation Log", MB_OK);
                        } else if (repPirates >= 15) {
                            credits += 150;
                            AdjustRep(0, 0, 1);
                            char pMsg[256];
                            sprintf(pMsg, "\n\n[PIRATE SYNDICATE HAIL]\nA corsair recognizes your Friendly standing!\n'Hail, brother corsair!' Safe passage granted with ₭150 tribute!");
                            strcat(resultMsg, pMsg);
                            goto encounter_processed;
                        } else {
                            MessageBox(hwnd, "Alert: Hostile pirate vessel encountered!", "Navigation Log", MB_OK);
                        }
                        
                        PlaySfx(SFX_ENEMY_LASER);
                        inCombat = 1;
                        enemyMaxHull = 30 + currentSystemId * 20;
                        enemyHull = enemyMaxHull;
                        strcpy(combatLog, "Pirate intercepts your ship!\nTactical combat initiated.\n");
                        
                        ShowWindow(hBtnCourse, SW_HIDE);
                        ShowWindow(hBtnMissions, SW_HIDE);
                        ShowWindow(hBtnFactions, SW_HIDE);
                        ShowWindow(hBtnInvestigate, SW_HIDE);
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

                        ShowWindow(hBtnFactDonate, SW_HIDE);
                        ShowWindow(hBtnFactDues, SW_HIDE);
                        ShowWindow(hBtnFactBribe, SW_HIDE);
                        ShowWindow(hBtnFactBack, SW_HIDE);
                        
                        ShowWindow(hBtnCombatAttack, SW_SHOW);
                        ShowWindow(hBtnCombatEvade, SW_SHOW);
                        ShowWindow(hBtnCombatUseTech, SW_SHOW);
                        ShowWindow(hBtnCombatFlee, SW_SHOW);
                        
                        UpdateCombatUI(hwnd);
                        return 0;
                    } else if (randomEncounter < 40) {
                        int salvageCreds = (rand() % 150) + 200;
                        credits += salvageCreds;
                        hull += 20;
                        if (hull > maxHull) hull = maxHull;
                        int used = cargoFood + cargoMinerals + cargoTech;
                        char extra[64] = "";
                        if (used < cargoMax) {
                            cargoTech++;
                            strcpy(extra, " and salvaged 1 ton of Tech cargo");
                        }
                        AdjustRep(-2, 1, 5);
                        char dMsg[256];
                        sprintf(dMsg, "\n\n[STELLAR PHENOMENON: DERELICT SHIP]\nDrifting derelict starship salvaged! Gained %d credits%s, and repaired +20 Hull!\nReputation: +5 Pirates, +1 Traders, -2 Federation.", salvageCreds, extra);
                        strcat(resultMsg, dMsg);
                    } else if (randomEncounter < 55) {
                        int dmg = 22 - shieldLevel * 3;
                        if (dmg < 5) dmg = 5;
                        hull -= dmg;
                        fuel += 25;
                        if (fuel > 100) fuel = 100;
                        AdjustRep(0, 4, 0);
                        char sMsg[256];
                        sprintf(sMsg, "\n\n[STELLAR PHENOMENON: SOLAR FLARE]\nCoronal mass ejection storm struck your ship! Lost %d Hull, but magnetic collectors recharged +25%% Fuel!\nReputation: +4 Traders Guild.", dmg);
                        strcat(resultMsg, sMsg);
                    } else if (randomEncounter < 70) {
                        int dmg = 25 - shieldLevel * 4;
                        if (dmg < 5) dmg = 5;
                        hull -= dmg;
                        int sData = (rand() % 150) + 300;
                        credits += sData;
                        AdjustRep(5, 0, 0);
                        char bMsg[256];
                        sprintf(bMsg, "\n\n[STELLAR PHENOMENON: BLACK HOLE]\nMicro Black Hole gravitational shear! Tidal forces inflicted %d Hull damage, but gravity slingshot recorded Hawking telemetry worth %d credits!\nReputation: +5 Federation.", dmg, sData);
                        strcat(resultMsg, bMsg);
                    } else if (randomEncounter < 85) {
                        strcat(resultMsg, "\n\nNotice: You found drifting debris.");
                        int found = (rand() % 100) + 20;
                        credits += found;
                        char foundMsg[64];
                        sprintf(foundMsg, "\nSalvaged %d credits.", found);
                        strcat(resultMsg, foundMsg);
                    } else {
                        strcat(resultMsg, "\n\nThe journey was uneventful.");
                    }
                    
encounter_processed:
                    if (hull <= 0) {
                        PlaySfx(SFX_EXPLOSION);
                        strcat(resultMsg, "\n\nCRITICAL: Hull integrity lost during transit! Emergency tow to Sol.");
                        hull = maxHull;
                        credits -= 500;
                        if (credits < 0) credits = 0;
                        currentSystemId = 0;
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
                    PlaySfx(SFX_LASER);
                    int baseDmg = (10 + weaponLevel * 10) * (80 + rand() % 40) / 100;
                    if (repFed >= 15) baseDmg = baseDmg * 12 / 10;
                    enemyHull -= baseDmg;
                    if (repFed >= 15) sprintf(turnMsg, "You attack for %d damage! (Fed escort assists)\n", baseDmg);
                    else sprintf(turnMsg, "You attack and deal %d damage!\n", baseDmg);
                } else if (cmd == ID_BTN_COMBAT_EVADE) {
                    PlaySfx(SFX_BLIP);
                    playerEvading = 1;
                    sprintf(turnMsg, "You take evasive maneuvers.\n");
                } else if (cmd == ID_BTN_COMBAT_USE_TECH) {
                    if (cargoTech > 0) {
                        PlaySfx(SFX_HEAL);
                        cargoTech--;
                        hull += 40;
                        if (hull > maxHull) hull = maxHull;
                        sprintf(turnMsg, "Used 1 Tech cargo to repair hull.\n");
                    } else {
                        PlaySfx(SFX_BLIP);
                        sprintf(turnMsg, "No Tech cargo available to use!\n");
                    }
                } else if (cmd == ID_BTN_COMBAT_FLEE) {
                    if (rand() % 100 < 50) {
                        PlaySfx(SFX_WARP);
                        MessageBox(hwnd, "Successfully escaped!", "Flee", MB_OK);
                        EndCombat(hwnd);
                        return 0;
                    } else {
                        PlaySfx(SFX_BLIP);
                        sprintf(turnMsg, "Failed to escape!\n");
                    }
                }
                
                if (enemyHull <= 0) {
                    PlaySfx(SFX_EXPLOSION);
                    int bounty = 100 + currentSystemId * 50;
                    credits += bounty;
                    AdjustRep(6, 3, -8);
                    char winMsg[512];
                    sprintf(winMsg, "Pirate destroyed! Claimed ₭%d standard bounty.\nReputation: +6 Fed, +3 Traders, -8 Pirates.", bounty);
                    if (activeMission.type == 2 && activeMission.targetId == currentSystemId) {
                        int fedBonus = (repFed >= 15) ? (activeMission.reward * 15 / 100) : 0;
                        int totalReward = activeMission.reward + fedBonus;
                        credits += totalReward;
                        AdjustRep(10, 4, -12);
                        char bMsg[128];
                        sprintf(bMsg, "\nMission Complete! Earned extra ₭%d (Fed bonus: ₭%d).\nBounty Rep: +10 Fed, +4 Traders, -12 Pirates.", totalReward, fedBonus);
                        strcat(winMsg, bMsg);
                        activeMission.type = 0;
                    }
                    MessageBox(hwnd, winMsg, "Combat Won", MB_OK);
                    EndCombat(hwnd);
                    return 0;
                }
                
                int hitChance = playerEvading ? 50 : 90;
                if (rand() % 100 < hitChance) {
                    PlaySfx(SFX_ENEMY_LASER);
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
                    PlaySfx(SFX_EXPLOSION);
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
                int bPrice, sPrice;
                
                if (cmd == ID_BTN_BUY_FOOD) {
                    bPrice = GetBuyPrice(sys->food_price);
                    if (credits >= bPrice && used < cargoMax) { credits -= bPrice; cargoFood++; AdjustRep(0, 1, 0); }
                } else if (cmd == ID_BTN_SELL_FOOD) {
                    sPrice = GetSellPrice(sys->food_price);
                    if (cargoFood > 0) { credits += sPrice; cargoFood--; AdjustRep(0, 1, 0); }
                } else if (cmd == ID_BTN_BUY_MINERALS) {
                    bPrice = GetBuyPrice(sys->minerals_price);
                    if (credits >= bPrice && used < cargoMax) { credits -= bPrice; cargoMinerals++; AdjustRep(0, 1, 0); }
                } else if (cmd == ID_BTN_SELL_MINERALS) {
                    sPrice = GetSellPrice(sys->minerals_price);
                    if (cargoMinerals > 0) { credits += sPrice; cargoMinerals--; AdjustRep(0, 1, 0); }
                } else if (cmd == ID_BTN_BUY_TECH) {
                    bPrice = GetBuyPrice(sys->tech_price);
                    if (credits >= bPrice && used < cargoMax) { credits -= bPrice; cargoTech++; AdjustRep(0, 1, 0); }
                } else if (cmd == ID_BTN_SELL_TECH) {
                    sPrice = GetSellPrice(sys->tech_price);
                    if (cargoTech > 0) { credits += sPrice; cargoTech--; AdjustRep(0, 1, 0); }
                }
                PlaySfx(SFX_BLIP);
                UpdateDashboard();
                ShowStationView(hwnd);
            } else if (LOWORD(wParam) >= ID_BTN_UPG_ENGINE && LOWORD(wParam) <= ID_BTN_UPG_SHIELD && selectedSystem == currentSystemId) {
                int cmd = LOWORD(wParam);
                int cost = 0;
                int upgraded = 0;
                if (cmd == ID_BTN_UPG_ENGINE) {
                    cost = 1000 * engineLevel;
                    if (credits >= cost) { credits -= cost; engineLevel++; upgraded = 1; }
                } else if (cmd == ID_BTN_UPG_CARGO) {
                    cost = 1500 * cargoLevel;
                    if (credits >= cost) { credits -= cost; cargoLevel++; cargoMax = 50 * cargoLevel; upgraded = 1; }
                } else if (cmd == ID_BTN_UPG_WEAPON) {
                    cost = 2000 * weaponLevel;
                    if (credits >= cost) { credits -= cost; weaponLevel++; upgraded = 1; }
                } else if (cmd == ID_BTN_UPG_SHIELD) {
                    cost = 2000 * shieldLevel;
                    if (credits >= cost) { credits -= cost; shieldLevel++; maxHull = 100 + (shieldLevel - 1) * 50; hull = maxHull; upgraded = 1; }
                }
                if (upgraded) PlaySfx(SFX_UPGRADE);
                else PlaySfx(SFX_BLIP);
                UpdateDashboard();
                ShowStationView(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_SOUND_TOGGLE) {
                soundEnabled = !soundEnabled;
                SetWindowText(hBtnSoundToggle, soundEnabled ? "SND: ON" : "SND: OFF");
                if (soundEnabled) PlaySfx(SFX_BLIP);
            } else if (LOWORD(wParam) == ID_BTN_DRONE_TOGGLE) {
                droneEnabled = !droneEnabled;
                SetWindowText(hBtnDroneToggle, droneEnabled ? "DRN: ON" : "DRN: OFF");
                if (soundEnabled) PlaySfx(SFX_BLIP);
            } else if (LOWORD(wParam) == ID_BTN_MISSIONS) {
                PlaySfx(SFX_BLIP);
                ShowMissionsView(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_FACTIONS) {
                PlaySfx(SFX_BLIP);
                ShowFactionsView(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_FACT_DONATE) {
                PlaySfx(SFX_BLIP);
                if (credits >= 350) {
                    credits -= 350;
                    AdjustRep(15, 0, -8);
                    UpdateDashboard();
                    ShowFactionsView(hwnd);
                    MessageBox(hwnd, "Donated ₭350 to Galactic Federation Naval Fleet!\nReputation: +15 Fed, -8 Pirates.", "Diplomacy", MB_OK);
                } else {
                    MessageBox(hwnd, "Not enough credits to donate!", "Diplomacy", MB_OK | MB_ICONWARNING);
                }
            } else if (LOWORD(wParam) == ID_BTN_FACT_DUES) {
                PlaySfx(SFX_BLIP);
                if (credits >= 300) {
                    credits -= 300;
                    AdjustRep(0, 15, 0);
                    UpdateDashboard();
                    ShowFactionsView(hwnd);
                    MessageBox(hwnd, "Paid ₭300 in Traders Guild annual dues!\nReputation: +15 Traders Guild.", "Diplomacy", MB_OK);
                } else {
                    MessageBox(hwnd, "Not enough credits for guild dues!", "Diplomacy", MB_OK | MB_ICONWARNING);
                }
            } else if (LOWORD(wParam) == ID_BTN_FACT_BRIBE) {
                PlaySfx(SFX_BLIP);
                if (credits >= 400) {
                    credits -= 400;
                    AdjustRep(-8, 0, 15);
                    UpdateDashboard();
                    ShowFactionsView(hwnd);
                    MessageBox(hwnd, "Slipped ₭400 to Pirate Syndicate contact!\nReputation: +15 Pirates, -8 Federation.", "Diplomacy", MB_OK);
                } else {
                    MessageBox(hwnd, "Not enough credits for bribe!", "Diplomacy", MB_OK | MB_ICONWARNING);
                }
            } else if (LOWORD(wParam) == ID_BTN_FACT_BACK) {
                PlaySfx(SFX_BLIP);
                ShowStationView(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_MISSION_ACCEPT_1) {
                PlaySfx(SFX_BLIP);
                activeMission = stationMissions[0];
                stationMissions[0].type = 0;
                UpdateDashboard();
                ShowMissionsView(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_MISSION_ACCEPT_2) {
                PlaySfx(SFX_BLIP);
                activeMission = stationMissions[1];
                stationMissions[1].type = 0;
                UpdateDashboard();
                ShowMissionsView(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_MISSION_ABANDON) {
                PlaySfx(SFX_BLIP);
                activeMission.type = 0;
                UpdateDashboard();
                ShowMissionsView(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_MISSION_BACK) {
                PlaySfx(SFX_BLIP);
                ShowStationView(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_INVESTIGATE && selectedSystem == currentSystemId) {
                System *sys = &systems[currentSystemId];
                if (sys->phenomenon == 0) return 0;
                PlaySfx(SFX_PHENOM);
                
                char phenomMsg[512] = "";
                if (sys->phenomenon == 1) { // Black Hole
                    int dmg = 25 - shieldLevel * 3;
                    if (dmg < 5) dmg = 5;
                    hull -= dmg;
                    int reward = (rand() % 200) + 600;
                    credits += reward;
                    AdjustRep(5, 0, 0);
                    sprintf(phenomMsg, "BLACK HOLE PROBE OPERATION\n\nYour scientific probe pierced the event horizon accretion disk.\nSevere gravitational tidal shear inflicted %d Hull damage.\nTelemetry yielded rare Dark Matter & Chroniton data: Earned ₭%d credits!\nReputation: +5 Federation.", dmg, reward);
                } else if (sys->phenomenon == 2) { // Solar Flare
                    int dmg = 18 - shieldLevel * 2;
                    if (dmg < 5) dmg = 5;
                    hull -= dmg;
                    fuel += 35;
                    if (fuel > 100) fuel = 100;
                    int reward = (rand() % 100) + 350;
                    credits += reward;
                    AdjustRep(0, 4, 0);
                    sprintf(phenomMsg, "SOLAR FLARE HARVESTING\n\nMagnetic scoops deployed into coronal mass ejection.\nSolar radiation inflicted %d Hull damage.\nSuperheated plasma harvested: +35%% Fuel and ₭%d in synthesized plasma cells!\nReputation: +4 Traders Guild.", dmg, reward);
                } else if (sys->phenomenon == 3) { // Derelict Ship
                    int reward = (rand() % 150) + 400;
                    credits += reward;
                    hull += 30;
                    if (hull > maxHull) hull = maxHull;
                    int used = cargoFood + cargoMinerals + cargoTech;
                    char cargoInfo[128] = "Cargo hold full - components stripped to scrap.";
                    if (used + 2 <= cargoMax) {
                        cargoTech++;
                        cargoMinerals++;
                        strcpy(cargoInfo, "Recovered 1 ton of Tech and 1 ton of Minerals cargo!");
                    }
                    AdjustRep(-2, 1, 5);
                    sprintf(phenomMsg, "DERELICT SALVAGE OPERATION\n\nBoarding party scavenged the abandoned vessel.\nRecovered ₭%d in scrap credits.\nRepaired +30 Hull using intact hull plating.\n%s\nReputation: +5 Pirates, +1 Traders, -2 Federation.", reward, cargoInfo);
                }
                
                sys->phenomenon = 0; // Cleared
                UpdateDashboard();
                MessageBox(hwnd, phenomMsg, "Phenomenon Report", MB_OK);
                
                if (hull <= 0) {
                    PlaySfx(SFX_EXPLOSION);
                    MessageBox(hwnd, "CRITICAL: Fatal damage sustained during operation! Emergency tow to Sol.", "Disaster", MB_OK);
                    hull = maxHull;
                    credits -= 500;
                    if (credits < 0) credits = 0;
                    currentSystemId = 0;
                }
                
                ShowStationView(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
        }
        case WM_DESTROY: {
            droneRunning = 0;
            if (hDroneThread) {
                WaitForSingleObject(hDroneThread, 500);
                CloseHandle(hDroneThread);
            }
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
        "KStellar Phase 13",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 550,
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
