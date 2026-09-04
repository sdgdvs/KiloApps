#include <windows.h>
#include <stdio.h>
#include <string.h>

#define ID_BTN_DEST1 101
#define ID_BTN_DEST2 102
#define ID_BTN_DEST3 104
#define ID_LIST_LOG  103
#define ID_BTN_BUY_START 200
#define ID_BTN_SELL_START 210
#define ID_BTN_HELP 600

typedef struct {
    int credits;
    int fuel;
    int maxFuel;
    int cargo;
    int maxCargo;
    int location; // Index in planets array
    int inventory[8]; // Food, Water, Ore, Tech, Meds, Luxury, Contraband, Military
    int engineLevel;
    int cargoLevel;
    int weaponLevel;
    int inCombat;
    int playerShields;
    int enemyShields;
    int enemyMaxShields;
    int repTraders;
    int repPirates;
    int repNavy;
    int activeMissionType; // 0=None, 1=Delivery, 2=Bounty, 3=Smuggling
    int activeMissionTarget;
    int activeMissionReward;
    int bountyTarget;
    int hasDreadnought;
} GameState;

GameState state = { 1000, 100, 100, 0, 20, 0, {0,0,0,0,0,0,0,0}, 0, 0, 0, 0, 50, 30, 30, 0, 0, 0, 0, 0, 0, 0, 0 };

int availMissionType[3];
int availMissionTarget[3];
int availMissionReward[3];


const char* goodNames[8] = { "Food", "Water", "Ore", "Tech", "Meds", "Luxury", "Contra", "Military" };
int currentPrices[8];

unsigned int rngState = 0x1234;
unsigned int SimpleRand() {
    rngState = (rngState >> 1) ^ (-(int)(rngState & 1u) & 0xB400u);
    return rngState;
}

// --- LOOP 2 GRAPHICS ENGINE HELPERS ---
int FastSin(int deg) {
    deg = ((deg % 360) + 360) % 360;
    if (deg > 180) return -FastSin(deg - 180);
    if (deg > 90) deg = 180 - deg;
    if (deg <= 30) return (deg * 50) / 30;
    if (deg <= 60) return 50 + ((deg - 30) * 36) / 30;
    return 86 + ((deg - 60) * 14) / 30;
}
int FastCos(int deg) {
    return FastSin(deg + 90);
}

typedef struct {
    int active;
    int x, y;       // 10x scaled coordinates
    int vx, vy;
    int life, maxLife;
    int layer;      // 0=spark, 1=smoke, 2=debris, 3=star
    COLORREF color;
    int size;
    int rot, vrot;
} GfxParticle;

#define MAX_PARTICLES 64
GfxParticle particles[MAX_PARTICLES];

typedef struct {
    int active;
    int x, y;
    int tx, ty;
    int progress;   // 0 to 100
    int speed;
    COLORREF color;
    int fromPlayer;
} GfxLaser;

#define MAX_LASERS 8
GfxLaser lasers[MAX_LASERS];

typedef struct {
    int x, y;
    int speed;
    int size;
    int twinkle;
} GfxStar;

#define MAX_STARS 50
GfxStar stars[MAX_STARS];
int starsInit = 0;

int shakeAmount = 0;
int shakeAngle = 0;
int jumpAnimTimer = 0;
int shieldHitPlayer = 0;
int shieldHitEnemy = 0;
DWORD animTick = 0;

void TriggerScreenShake(int amt) {
    if (amt > shakeAmount) shakeAmount = amt;
}

void SpawnParticleBurst(int x, int y, int type, int count) {
    for (int i = 0; i < count; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!particles[p].active) {
                particles[p].active = 1;
                particles[p].x = x * 10;
                particles[p].y = y * 10;
                int spd = 10 + (SimpleRand() % 25);
                int angle = SimpleRand() % 360;
                particles[p].vx = (FastCos(angle) * spd) / 100;
                particles[p].vy = (FastSin(angle) * spd) / 100;
                particles[p].rot = SimpleRand() % 360;
                particles[p].vrot = ((int)(SimpleRand() % 21) - 10);
                particles[p].life = 15 + (SimpleRand() % 15);
                particles[p].maxLife = particles[p].life;
                particles[p].size = 2 + (SimpleRand() % 3);

                if (type == 0) { // explosion
                    int r = SimpleRand() % 3;
                    if (r == 0) { particles[p].color = RGB(255, 255, 200); particles[p].layer = 0; }
                    else if (r == 1) { particles[p].color = RGB(255, 80, 0); particles[p].layer = 1; particles[p].size += 2; }
                    else { particles[p].color = RGB(160, 160, 160); particles[p].layer = 2; }
                } else if (type == 1) { // laserHitPlayer
                    particles[p].color = RGB(0, 255, 255);
                    particles[p].layer = 0;
                } else if (type == 2) { // laserHitEnemy
                    particles[p].color = RGB(255, 50, 0);
                    particles[p].layer = 0;
                } else if (type == 3) { // celebration
                    particles[p].color = (SimpleRand() % 2 == 0) ? RGB(255, 215, 0) : RGB(255, 0, 255);
                    particles[p].layer = 3;
                    particles[p].size = 4;
                    particles[p].life = 25 + (SimpleRand() % 20);
                    particles[p].maxLife = particles[p].life;
                } else { // asteroid
                    particles[p].color = RGB(160, 110, 60);
                    particles[p].layer = 2;
                }
                break;
            }
        }
    }
}

void SpawnLaser(int fromPlayer) {
    for (int i = 0; i < MAX_LASERS; i++) {
        if (!lasers[i].active) {
            lasers[i].active = 1;
            lasers[i].fromPlayer = fromPlayer;
            lasers[i].progress = 0;
            lasers[i].speed = 12;
            if (fromPlayer) {
                lasers[i].x = 135; lasers[i].y = 75;
                lasers[i].tx = 470; lasers[i].ty = 75;
                lasers[i].color = RGB(0, 255, 255);
            } else {
                lasers[i].x = 465; lasers[i].y = 75;
                lasers[i].tx = 100; lasers[i].ty = 75;
                lasers[i].color = RGB(255, 40, 0);
            }
            break;
        }
    }
}

void TriggerJumpFX() {
    jumpAnimTimer = 25;
    TriggerScreenShake(8);
    SpawnParticleBurst(100, 75, 3, 15);
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

void GenerateMissions() {
    for (int i=0; i<3; i++) {
        int type = 1 + (SimpleRand() % 3);
        int target = SimpleRand() % NUM_PLANETS;
        while(target == state.location) target = SimpleRand() % NUM_PLANETS;
        
        availMissionType[i] = type;
        availMissionTarget[i] = target;
        if (type == 1) availMissionReward[i] = 100 + (SimpleRand() % 200);
        else if (type == 2) availMissionReward[i] = 300 + (SimpleRand() % 400);
        else availMissionReward[i] = 400 + (SimpleRand() % 500);
    }
}

void GetMarketBase(int pIdx, int basePrices[8]) {
    Planet p = planets[pIdx];
    int base[8] = {50, 50, 50, 50, 50, 100, 150, 200};
    if (p.ecoType == 0) { base[0] = 20; base[1] = 25; base[3] = 90; }
    if (p.ecoType == 1) { base[2] = 20; base[0] = 80; }
    if (p.ecoType == 2) { base[3] = 30; base[4] = 40; base[2] = 80; }
    if (p.ecoType == 3) { base[2] = 30; base[3] = 40; base[0] = 80; }
    if (p.ecoType == 4) { base[0]=40; base[1]=40; base[2]=40; base[3]=40; base[4]=40; }
    base[3] += (5 - p.techLevel) * 10;
    base[4] += (5 - p.techLevel) * 10;
    for(int i=0; i<8; i++) basePrices[i] = base[i];
}

HWND hStatCredits, hStatFuel, hStatCargo, hStatWeapons;
HWND hStatLoc;
HWND hBtnDest1, hBtnDest2, hBtnDest3;
HWND hListLog;
#define ID_BTN_UPG_CARGO 301
#define ID_BTN_UPG_ENGINE 302
#define ID_BTN_UPG_WEAPON 303
#define ID_BTN_UPG_DREAD 304
HWND hBtnUpgCargo, hBtnUpgEngine, hBtnUpgWeapon, hBtnUpgDread;

#define ID_BTN_FIRE 401
#define ID_BTN_FLEE 402

HWND hStatCombatTitle;
HWND hStatCombatPlayer;
HWND hStatCombatEnemy;
HWND hBtnFire;
HWND hBtnFlee;

#define ID_BTN_BRIBE 403
HWND hBtnBribe;
HWND hStatReps;

HWND hStatGoodName[8];
HWND hStatGoodPrice[8];
HWND hStatGoodOwned[8];
HWND hBtnGoodBuy[8];
HWND hBtnGoodSell[8];

HWND hStatMissions;
HWND hStatActiveMission;
HWND hBtnAbandonMission;
HWND hBtnMission[3];
HWND hBtnHelp;
#define ID_BTN_MISSION1 501
#define ID_BTN_MISSION2 502
#define ID_BTN_MISSION3 503
#define ID_BTN_ABANDON 504

int destTarget[3];
int destCost[3];

void GeneratePrices() {
    int bases[8];
    GetMarketBase(state.location, bases);
    for (int i = 0; i < 8; i++) {
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

    wsprintf(buf, "Traders Rep: %d | Pirates Rep: %d | Navy Rep: %d", state.repTraders, state.repPirates, state.repNavy);
    SetWindowText(hStatReps, buf);

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
        ShowWindow(hBtnBribe, SW_SHOW);
        EnableWindow(hBtnBribe, state.credits >= 100);
        
        wsprintf(buf, "Shields: %d", state.playerShields);
        SetWindowText(hStatCombatPlayer, buf);
        wsprintf(buf, "Enemy: %d / %d", state.enemyShields, state.enemyMaxShields);
        SetWindowText(hStatCombatEnemy, buf);
        
        ShowWindow(hBtnAbandonMission, SW_HIDE);
        for(int i=0; i<3; i++) ShowWindow(hBtnMission[i], SW_HIDE);
        ShowWindow(hStatMissions, SW_HIDE);
        ShowWindow(hStatActiveMission, SW_HIDE);
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
        ShowWindow(hBtnBribe, SW_HIDE);
        
        ShowWindow(hStatMissions, SW_SHOW);
        ShowWindow(hStatActiveMission, SW_SHOW);
    }
    
    if (state.activeMissionType != 0) {
        const char* mType = (state.activeMissionType == 1) ? "Deliver to" : ((state.activeMissionType == 2) ? "Bounty at" : "Smuggle to");
        wsprintf(buf, "Active: %s %s (%d cr)", mType, planets[state.activeMissionTarget].name, state.activeMissionReward);
        SetWindowText(hStatActiveMission, buf);
        if (!state.inCombat) ShowWindow(hBtnAbandonMission, SW_SHOW);
        for (int i=0; i<3; i++) ShowWindow(hBtnMission[i], SW_HIDE);
    } else {
        SetWindowText(hStatActiveMission, "No active mission.");
        ShowWindow(hBtnAbandonMission, SW_HIDE);

        for (int i=0; i<3; i++) {
            const char* mType = (availMissionType[i] == 1) ? "Deliver to" : ((availMissionType[i] == 2) ? "Bounty at" : "Smuggle to");
            wsprintf(buf, "%s %s (%d cr)", mType, planets[availMissionTarget[i]].name, availMissionReward[i]);
            SetWindowText(hBtnMission[i], buf);
            if (!state.inCombat) ShowWindow(hBtnMission[i], SW_SHOW);
        }
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

    if (!state.hasDreadnought) {
        SetWindowText(hBtnUpgDread, "Dreadnought (100k)");
        EnableWindow(hBtnUpgDread, !state.inCombat && state.credits >= 100000);
    } else {
        SetWindowText(hBtnUpgDread, "Dreadnought Acquired");
        EnableWindow(hBtnUpgDread, FALSE);
    }

    // Update Market
    for (int i = 0; i < 8; i++) {
        int visible = 1;
        if (i == 5 && state.repTraders < 50 && state.inventory[i] == 0) visible = 0;
        if (i == 6 && state.repPirates < 50 && state.inventory[i] == 0) visible = 0;
        if (i == 7 && state.repNavy < 50 && state.inventory[i] == 0) visible = 0;
        
        if (visible) {
            ShowWindow(hStatGoodName[i], SW_SHOW);
            ShowWindow(hStatGoodPrice[i], SW_SHOW);
            ShowWindow(hStatGoodOwned[i], SW_SHOW);
            ShowWindow(hBtnGoodBuy[i], SW_SHOW);
            ShowWindow(hBtnGoodSell[i], SW_SHOW);
            
            wsprintf(buf, "%d cr", currentPrices[i]);
            SetWindowText(hStatGoodPrice[i], buf);
            wsprintf(buf, "Own: %d", state.inventory[i]);
            SetWindowText(hStatGoodOwned[i], buf);
            
            EnableWindow(hBtnGoodBuy[i], !state.inCombat && state.credits >= currentPrices[i] && state.cargo < state.maxCargo);
            EnableWindow(hBtnGoodSell[i], !state.inCombat && state.inventory[i] > 0);
        } else {
            ShowWindow(hStatGoodName[i], SW_HIDE);
            ShowWindow(hStatGoodPrice[i], SW_HIDE);
            ShowWindow(hStatGoodOwned[i], SW_HIDE);
            ShowWindow(hBtnGoodBuy[i], SW_HIDE);
            ShowWindow(hBtnGoodSell[i], SW_HIDE);
        }
    }
}

void PlaySoundEffect(int type) {
    if (type == 1) { // jump
        for(int i=100; i<800; i+=100) Beep(i, 50);
    } else if (type == 2) { // laser
        for(int i=800; i>200; i-=100) Beep(i, 20);
    } else if (type == 3) { // chime
        Beep(600, 100);
        Beep(800, 150);
    } else if (type == 4) { // alert
        Beep(400, 200);
        Beep(600, 200);
    } else if (type == 5) { // fail
        Beep(200, 300);
    }
}

void Travel(int btnIdx, HWND hwnd) {
    int cost = destCost[btnIdx];
    int target = destTarget[btnIdx];
    
    if (state.fuel >= cost) {
        PlaySoundEffect(1); // jump
        state.fuel -= cost;
        state.location = target;
        TriggerJumpFX();
        char buf[256];
        wsprintf(buf, "> Hyperspace jump complete. Arrived at %s. Used %d fuel.", planets[target].name, cost);
        LogMessage(buf);

        int navyIntercept = 0;
        int forcedPirate = 0;

        if (state.activeMissionType == 3) {
            if ((SimpleRand() % 100) < 25) navyIntercept = 1;
        }
        if (state.activeMissionType == 2 && target == state.activeMissionTarget) {
            forcedPirate = 1;
        }

        if (navyIntercept) {
            PlaySoundEffect(4); // alert
            LogMessage("> 🚨 INTERCEPTED BY GALACTIC NAVY! Contraband found! 🚨");
            int fine = (int)(state.credits * 0.3) + 200;
            state.credits -= fine;
            if (state.credits < 0) state.credits = 0;
            state.repNavy -= 10;
            wsprintf(buf, "> Navy fined you %d cr and confiscated the goods. Mission Failed!", fine);
            LogMessage(buf);
            state.activeMissionType = 0;
        } else if (forcedPirate) {
            PlaySoundEffect(4); // alert
            state.inCombat = 1;
            state.bountyTarget = 1;
            state.playerShields = 50 + state.cargoLevel * 10;
            state.enemyMaxShields = 20 + (SimpleRand() % 40);
            state.enemyShields = state.enemyMaxShields;
            TriggerScreenShake(5);
            LogMessage("> 🚨 BOUNTY TARGET INTERCEPTED! RED ALERT! 🚨");
        } else if (SimpleRand() % 100 < 30) {
            int enc = SimpleRand() % 3;
            if (enc == 0) {
                PlaySoundEffect(4); // alert
                int fuelLoss = 5 + (SimpleRand() % 11);
                state.fuel -= fuelLoss;
                if (state.fuel < 0) state.fuel = 0;
                SpawnParticleBurst(300, 75, 4, 20);
                TriggerScreenShake(7);
                char encBuf[256];
                wsprintf(encBuf, "> WARNING: Asteroid field! Evasive maneuvers cost %d fuel.", fuelLoss);
                LogMessage(encBuf);
            } else if (enc == 1) {
                PlaySoundEffect(3); // chime
                int creditsGained = 50 + (SimpleRand() % 101);
                state.credits += creditsGained;
                state.repTraders += 5;
                SpawnParticleBurst(300, 75, 3, 15);
                char encBuf[256];
                wsprintf(encBuf, "> Distress signal! Helped stranded ship. +%d cr, Traders Rep +5.", creditsGained);
                LogMessage(encBuf);
            } else {
                PlaySoundEffect(4); // alert
                state.inCombat = 1;
                state.bountyTarget = 0;
                state.playerShields = 50 + state.cargoLevel * 10;
                state.enemyMaxShields = 20 + (SimpleRand() % 40);
                state.enemyShields = state.enemyMaxShields;
                TriggerScreenShake(5);
                LogMessage("> 🚨 RED ALERT: PIRATE VESSEL INTERCEPTED YOU! 🚨");
            }
        }

        GeneratePrices();
        GenerateMissions();
        
        if (state.activeMissionType == 1 || state.activeMissionType == 3) {
            if (target == state.activeMissionTarget) {
                PlaySoundEffect(3); // chime
                state.credits += state.activeMissionReward;
                SpawnParticleBurst(500, 75, 3, 25);
                if (state.activeMissionType == 3) state.repPirates += 5;
                else state.repTraders += 5;
                wsprintf(buf, "> Mission Complete! Earned %d cr.", state.activeMissionReward);
                LogMessage(buf);
                state.activeMissionType = 0;
            }
        }

        UpdateUI(hwnd);
        InvalidateRect(hwnd, NULL, FALSE);
    } else {
        PlaySoundEffect(5); // fail
        LogMessage("> Insufficient fuel!");
    }
}

void EnemyTurn(HWND hwnd) {
    PlaySoundEffect(2); // laser
    SpawnLaser(0);
    int dmg = 5 + (SimpleRand() % 15);
    state.playerShields -= dmg;
    char buf[128];
    wsprintf(buf, "> Pirates fired! Your shields took %d damage.", dmg);
    LogMessage(buf);

    if (state.playerShields <= 0) {
        LogMessage("> Your shields failed!");
        SpawnParticleBurst(100, 75, 0, 30);
        TriggerScreenShake(9);
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
            GenerateMissions();

            for (int i = 0; i < MAX_STARS; i++) {
                stars[i].x = SimpleRand() % 600;
                stars[i].y = SimpleRand() % 150;
                stars[i].speed = 1 + (SimpleRand() % 3);
                stars[i].size = (SimpleRand() % 10 == 0) ? 2 : 1;
                stars[i].twinkle = SimpleRand() % 360;
            }
            starsInit = 1;
            SetTimer(hwnd, 1, 33, NULL);

            HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
            
            CreateWindow("STATIC", "KTrader Space Trading Sim", WS_CHILD | WS_VISIBLE,
                20, 15, 300, 20, hwnd, NULL, NULL, NULL);

            hBtnHelp = CreateWindow("BUTTON", "Help", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 530, 12, 80, 25, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            SendMessage(hBtnHelp, WM_SETFONT, (WPARAM)hFont, TRUE);

            hStatCredits = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 10, 210, 140, 20, hwnd, NULL, NULL, NULL);
            hStatFuel = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 160, 210, 140, 20, hwnd, NULL, NULL, NULL);
            hStatCargo = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 310, 210, 140, 20, hwnd, NULL, NULL, NULL);
            hStatWeapons = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 460, 210, 140, 20, hwnd, NULL, NULL, NULL);

            hStatReps = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 10, 230, 600, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hStatReps, WM_SETFONT, (WPARAM)hFont, TRUE);

            CreateWindow("STATIC", "Navigation", WS_CHILD | WS_VISIBLE, 20, 250, 100, 20, hwnd, NULL, NULL, NULL);
            hStatLoc = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 20, 280, 260, 20, hwnd, NULL, NULL, NULL);

            hBtnDest1 = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 310, 250, 30, hwnd, (HMENU)ID_BTN_DEST1, NULL, NULL);
            hBtnDest2 = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 350, 250, 30, hwnd, (HMENU)ID_BTN_DEST2, NULL, NULL);
            hBtnDest3 = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 390, 250, 30, hwnd, (HMENU)ID_BTN_DEST3, NULL, NULL);

            hStatCombatTitle = CreateWindow("STATIC", "🚨 COMBAT ENGAGED 🚨", WS_CHILD, 20, 250, 200, 20, hwnd, NULL, NULL, NULL);
            hStatCombatPlayer = CreateWindow("STATIC", "Shields: 50", WS_CHILD, 20, 280, 200, 20, hwnd, NULL, NULL, NULL);
            hStatCombatEnemy = CreateWindow("STATIC", "Enemy: 30 / 30", WS_CHILD, 20, 310, 200, 20, hwnd, NULL, NULL, NULL);
            hBtnFire = CreateWindow("BUTTON", "Fire Weapons", WS_CHILD | BS_PUSHBUTTON, 20, 340, 120, 30, hwnd, (HMENU)ID_BTN_FIRE, NULL, NULL);
            hBtnFlee = CreateWindow("BUTTON", "Attempt Flee", WS_CHILD | BS_PUSHBUTTON, 150, 340, 120, 30, hwnd, (HMENU)ID_BTN_FLEE, NULL, NULL);
            hBtnBribe = CreateWindow("BUTTON", "Pay Toll(100cr)", WS_CHILD | BS_PUSHBUTTON, 280, 340, 150, 30, hwnd, (HMENU)ID_BTN_BRIBE, NULL, NULL);
            
            SendMessage(hStatCombatTitle, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hStatCombatPlayer, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hStatCombatEnemy, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnFire, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnFlee, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnBribe, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hStatShipyard = CreateWindow("STATIC", "Shipyard", WS_CHILD | WS_VISIBLE, 20, 430, 100, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hStatShipyard, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnUpgCargo = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 460, 140, 30, hwnd, (HMENU)ID_BTN_UPG_CARGO, NULL, NULL);
            hBtnUpgEngine = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 170, 460, 140, 30, hwnd, (HMENU)ID_BTN_UPG_ENGINE, NULL, NULL);
            hBtnUpgWeapon = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 320, 460, 140, 30, hwnd, (HMENU)ID_BTN_UPG_WEAPON, NULL, NULL);
            hBtnUpgDread = CreateWindow("BUTTON", "Dreadnought", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 470, 460, 140, 30, hwnd, (HMENU)ID_BTN_UPG_DREAD, NULL, NULL);
            SendMessage(hBtnUpgCargo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnUpgEngine, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnUpgWeapon, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnUpgDread, WM_SETFONT, (WPARAM)hFont, TRUE);

            hStatMissions = CreateWindow("STATIC", "Mission Board", WS_CHILD | WS_VISIBLE, 20, 500, 150, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hStatMissions, WM_SETFONT, (WPARAM)hFont, TRUE);

            hStatActiveMission = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 180, 500, 350, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hStatActiveMission, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBtnAbandonMission = CreateWindow("BUTTON", "Abandon", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 540, 500, 80, 20, hwnd, (HMENU)ID_BTN_ABANDON, NULL, NULL);
            SendMessage(hBtnAbandonMission, WM_SETFONT, (WPARAM)hFont, TRUE);

            for(int i=0; i<3; i++) {
                hBtnMission[i] = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 370 + i*22, 560, 20, hwnd, (HMENU)(ID_BTN_MISSION1 + i), NULL, NULL);
                SendMessage(hBtnMission[i], WM_SETFONT, (WPARAM)hFont, TRUE);
            }

            hListLog = CreateWindow("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
                20, 590, 600, 70, hwnd, (HMENU)ID_LIST_LOG, NULL, NULL);

            HWND hStatMarket = CreateWindow("STATIC", "Market", WS_CHILD | WS_VISIBLE, 300, 250, 100, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hStatMarket, WM_SETFONT, (WPARAM)hFont, TRUE);

            for (int i = 0; i < 8; i++) {
                int y = 280 + i * 22;
                hStatGoodName[i] = CreateWindow("STATIC", goodNames[i], WS_CHILD | WS_VISIBLE, 300, y, 60, 20, hwnd, NULL, NULL, NULL);
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

            SendDlgItemMessage(hwnd, ID_BTN_DEST1, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendDlgItemMessage(hwnd, ID_BTN_DEST2, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendDlgItemMessage(hwnd, ID_BTN_DEST3, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hListLog, WM_SETFONT, (WPARAM)hFont, TRUE);

            LogMessage("> Welcome to KTrader, Captain.");
            UpdateUI(hwnd);
            return 0;
        }
        case WM_TIMER: {
            animTick++;
            if (jumpAnimTimer > 0) jumpAnimTimer--;
            if (shieldHitPlayer > 0) shieldHitPlayer--;
            if (shieldHitEnemy > 0) shieldHitEnemy--;
            if (shakeAmount > 0) {
                shakeAngle = (shakeAngle + 45) % 360;
                shakeAmount = (shakeAmount * 85) / 100;
            }

            for (int i = 0; i < MAX_STARS; i++) {
                stars[i].twinkle = (stars[i].twinkle + 5) % 360;
                if (!jumpAnimTimer) {
                    stars[i].x -= stars[i].speed;
                    if (stars[i].x < 0) stars[i].x = 600;
                }
            }

            for (int i = 0; i < MAX_LASERS; i++) {
                if (lasers[i].active) {
                    lasers[i].progress += lasers[i].speed;
                    if (lasers[i].progress >= 100) {
                        lasers[i].active = 0;
                        if (lasers[i].fromPlayer) {
                            shieldHitEnemy = 10;
                            SpawnParticleBurst(lasers[i].tx, lasers[i].ty, 2, 8);
                            TriggerScreenShake(3);
                        } else {
                            shieldHitPlayer = 10;
                            SpawnParticleBurst(lasers[i].tx, lasers[i].ty, 1, 8);
                            TriggerScreenShake(4);
                        }
                    }
                }
            }

            for (int p = 0; p < MAX_PARTICLES; p++) {
                if (particles[p].active) {
                    particles[p].x += particles[p].vx;
                    particles[p].y += particles[p].vy;
                    particles[p].rot = (particles[p].rot + particles[p].vrot) % 360;
                    particles[p].life--;
                    if (particles[p].layer == 0) {
                        particles[p].vx = (particles[p].vx * 94) / 100;
                        particles[p].vy = (particles[p].vy * 94) / 100;
                    } else if (particles[p].layer == 1) {
                        particles[p].vy -= 1;
                    } else if (particles[p].layer == 2) {
                        particles[p].vy += 2;
                    }
                    if (particles[p].life <= 0) particles[p].active = 0;
                }
            }

            RECT rcGfx = { 20, 45, 620, 200 };
            InvalidateRect(hwnd, &rcGfx, FALSE);
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_HELP) {
                MessageBox(hwnd, "Captain's Log - Help Guide\n\n"
                    "How to Play:\nTravel between planets to buy low and sell high. Watch your fuel and be prepared for space pirates, asteroid fields, and Navy patrols.\n\n"
                    "Trade Goods:\n"
                    "- Food & Water: Cheap at Agri planets.\n"
                    "- Ore: Cheap at Mining planets.\n"
                    "- Tech & Meds: Produced at Tech planets.\n"
                    "- Luxury/Contraband/Military: Requires high reputation with Traders/Pirates/Navy. Navy may confiscate contraband.\n\n"
                    "Ship Upgrades:\n"
                    "- Cargo: Increases max cargo and shields.\n"
                    "- Engine: Reduces fuel cost.\n"
                    "- Weapons: Increases combat damage.\n"
                    "- Dreadnought: Ultimate end-game goal!", 
                    "Help", MB_OK | MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == ID_BTN_DEST1) {
                Travel(0, hwnd);
            } else if (LOWORD(wParam) == ID_BTN_DEST2) {
                Travel(1, hwnd);
            } else if (LOWORD(wParam) == ID_BTN_DEST3) {
                Travel(2, hwnd);
            } else if (LOWORD(wParam) >= ID_BTN_BUY_START && LOWORD(wParam) < ID_BTN_BUY_START + 8) {
                int good = LOWORD(wParam) - ID_BTN_BUY_START;
                int price = currentPrices[good];
                if (state.credits >= price && state.cargo < state.maxCargo) {
                    PlaySoundEffect(3); // chime
                    state.credits -= price;
                    state.inventory[good]++;
                    state.cargo++;
                    state.repTraders++;
                    char buf[128];
                    wsprintf(buf, "> Bought 1 %s for %d cr.", goodNames[good], price);
                    LogMessage(buf);
                    UpdateUI(hwnd);
                }
            } else if (LOWORD(wParam) >= ID_BTN_SELL_START && LOWORD(wParam) < ID_BTN_SELL_START + 8) {
                int good = LOWORD(wParam) - ID_BTN_SELL_START;
                int price = currentPrices[good];
                if (state.inventory[good] > 0) {
                    PlaySoundEffect(3); // chime
                    state.credits += price;
                    state.inventory[good]--;
                    state.cargo--;
                    state.repTraders++;
                    char buf[128];
                    wsprintf(buf, "> Sold 1 %s for %d cr.", goodNames[good], price);
                    LogMessage(buf);
                    UpdateUI(hwnd);
                }
            } else if (LOWORD(wParam) == ID_BTN_UPG_CARGO) {
                int cost = 500 * (state.cargoLevel + 1);
                if (state.credits >= cost) {
                    PlaySoundEffect(3); // chime
                    state.credits -= cost;
                    state.cargoLevel++;
                    state.maxCargo += 20;
                    SpawnParticleBurst(100, 75, 3, 16);
                    TriggerScreenShake(3);
                    char buf[128];
                    wsprintf(buf, "> Cargo Bay upgraded! Max cargo now %d.", state.maxCargo);
                    LogMessage(buf);
                    UpdateUI(hwnd);
                }
            } else if (LOWORD(wParam) == ID_BTN_UPG_ENGINE) {
                int cost = 1000 * (state.engineLevel + 1);
                if (state.credits >= cost) {
                    PlaySoundEffect(3); // chime
                    state.credits -= cost;
                    state.engineLevel++;
                    state.maxFuel += 50;
                    SpawnParticleBurst(100, 75, 3, 16);
                    TriggerScreenShake(3);
                    LogMessage("> Engine upgraded! Less fuel used for travel.");
                    UpdateUI(hwnd);
                }
            } else if (LOWORD(wParam) == ID_BTN_UPG_WEAPON) {
                int cost = 1500 * (state.weaponLevel + 1);
                if (state.credits >= cost) {
                    PlaySoundEffect(3); // chime
                    state.credits -= cost;
                    state.weaponLevel++;
                    SpawnParticleBurst(100, 75, 3, 16);
                    TriggerScreenShake(3);
                    char buf[128];
                    wsprintf(buf, "> Weapons upgraded to level %d!", state.weaponLevel);
                    LogMessage(buf);
                    UpdateUI(hwnd);
                }
            } else if (LOWORD(wParam) == ID_BTN_UPG_DREAD) {
                if (state.credits >= 100000 && !state.hasDreadnought) {
                    PlaySoundEffect(3); // chime
                    state.credits -= 100000;
                    state.hasDreadnought = 1;
                    SpawnParticleBurst(100, 75, 3, 40);
                    TriggerScreenShake(12);
                    LogMessage("> YOU WIN! You purchased the legendary Dreadnought! The galaxy is yours!");
                    UpdateUI(hwnd);
                }
            } else if (LOWORD(wParam) >= ID_BTN_MISSION1 && LOWORD(wParam) <= ID_BTN_MISSION3) {
                int idx = LOWORD(wParam) - ID_BTN_MISSION1;
                if (state.activeMissionType == 0) {
                    state.activeMissionType = availMissionType[idx];
                    state.activeMissionTarget = availMissionTarget[idx];
                    state.activeMissionReward = availMissionReward[idx];
                    LogMessage("> Mission accepted.");
                    UpdateUI(hwnd);
                }
            } else if (LOWORD(wParam) == ID_BTN_ABANDON) {
                if (state.activeMissionType != 0) {
                    PlaySoundEffect(5); // fail
                    state.activeMissionType = 0;
                    LogMessage("> Mission abandoned.");
                    UpdateUI(hwnd);
                }
            } else if (LOWORD(wParam) == ID_BTN_FIRE) {
                PlaySoundEffect(2); // laser
                SpawnLaser(1);
                int dmg = 10 + state.weaponLevel * 15 + (SimpleRand() % 10);
                state.enemyShields -= dmg;
                char buf[128];
                wsprintf(buf, "> You fired! Pirate shields took %d damage.", dmg);
                LogMessage(buf);

                if (state.enemyShields <= 0) {
                    LogMessage("> Pirate ship destroyed!");
                    SpawnParticleBurst(500, 75, 0, 35);
                    TriggerScreenShake(10);
                    int bounty = 50 + (SimpleRand() % 100);
                    state.credits += bounty;
                    state.repNavy += 5;
                    state.repPirates -= 5;
                    wsprintf(buf, "> Recovered %d cr. Navy Rep +5, Pirates Rep -5.", bounty);
                    LogMessage(buf);
                    
                    if (state.bountyTarget) {
                        state.credits += state.activeMissionReward;
                        state.repNavy += 5;
                        SpawnParticleBurst(500, 75, 3, 25);
                        wsprintf(buf, "> Bounty Complete! Earned %d cr bonus.", state.activeMissionReward);
                        LogMessage(buf);
                        state.activeMissionType = 0;
                        state.bountyTarget = 0;
                    }
                    state.inCombat = 0;
                } else {
                    EnemyTurn(hwnd);
                }
                UpdateUI(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_FLEE) {
                if ((SimpleRand() % 100) < 50) {
                    LogMessage("> Successfully fled from the pirates!");
                    TriggerJumpFX();
                    state.inCombat = 0;
                } else {
                    LogMessage("> Failed to escape!");
                    TriggerScreenShake(4);
                    EnemyTurn(hwnd);
                }
                UpdateUI(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_BRIBE) {
                if (state.credits >= 100) {
                    state.credits -= 100;
                    state.repPirates += 5;
                    state.inCombat = 0;
                    SpawnParticleBurst(300, 75, 3, 10);
                    LogMessage("> Paid 100 cr toll to pirates. Pirates Rep +5.");
                    UpdateUI(hwnd);
                }
            }
            return 0;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Double buffered rendering for 600x150 graphics viewport
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, 600, 150);
            HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

            // Background fill
            RECT rcBox = { 0, 0, 600, 150 };
            HBRUSH hBg = CreateSolidBrush(RGB(2, 7, 18));
            FillRect(memDC, &rcBox, hBg);
            DeleteObject(hBg);

            // Render Stars
            HPEN hWarpPen = CreatePen(PS_SOLID, 1, RGB(180, 230, 255));
            HPEN oldPen = (HPEN)SelectObject(memDC, hWarpPen);
            for (int i = 0; i < MAX_STARS; i++) {
                if (jumpAnimTimer > 0) {
                    int streak = jumpAnimTimer * stars[i].speed * 2;
                    MoveToEx(memDC, stars[i].x, stars[i].y, NULL);
                    LineTo(memDC, stars[i].x - streak, stars[i].y);
                } else {
                    int bright = 120 + FastSin(stars[i].twinkle) * 120 / 100;
                    if (bright < 50) bright = 50; if (bright > 255) bright = 255;
                    SetPixel(memDC, stars[i].x, stars[i].y, RGB(bright, bright, bright));
                    if (stars[i].size > 1 && stars[i].x + 1 < 600) {
                        SetPixel(memDC, stars[i].x + 1, stars[i].y, RGB(bright, bright, bright));
                    }
                }
            }
            SelectObject(memDC, oldPen);
            DeleteObject(hWarpPen);

            // Floating dust motes
            for (int i = 0; i < 6; i++) {
                int dx = ((animTick * (i + 1) * 2) + i * 100) % 600;
                int dy = 20 + i * 20 + FastSin(animTick * 4 + i * 60) * 8 / 100;
                SetPixel(memDC, dx, dy, RGB(0, 255, 255));
            }

            // Draw Player Ship (Left side: x~100, y~75)
            int py = 75 + (FastSin(animTick * 6) * 3) / 100;
            if (state.hasDreadnought) {
                // Dreadnought Flagship Sprite
                // Dual Violet Thrusters
                int dflLen = 22 + (FastSin(animTick * 20) * 6) / 100;
                POINT th1[3] = { {65, py - 12}, {65 - dflLen, py - 12}, {65, py - 6} };
                POINT th2[3] = { {65, py + 6}, {65 - dflLen, py + 6}, {65, py + 12} };
                HBRUSH hVBrush = CreateSolidBrush(RGB(208, 0, 255));
                HPEN hVPen = CreatePen(PS_SOLID, 1, RGB(255, 100, 255));
                SelectObject(memDC, hVBrush);
                SelectObject(memDC, hVPen);
                Polygon(memDC, th1, 3);
                Polygon(memDC, th2, 3);

                // Titan Hull (Dark Obsidian with Magenta Edge)
                HBRUSH hTitanBrush = CreateSolidBrush(RGB(28, 21, 42));
                HPEN hTitanPen = CreatePen(PS_SOLID, 2, RGB(192, 68, 254));
                SelectObject(memDC, hTitanBrush);
                SelectObject(memDC, hTitanPen);
                POINT titanPts[8] = {
                    {145, py}, {125, py - 18}, {90, py - 25}, {65, py - 18},
                    {70, py}, {65, py + 18}, {90, py + 25}, {125, py + 18}
                };
                Polygon(memDC, titanPts, 8);

                // Dual Heavy Railguns
                HBRUSH hGunBrush = CreateSolidBrush(RGB(16, 11, 26));
                HPEN hGunPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
                SelectObject(memDC, hGunBrush);
                SelectObject(memDC, hGunPen);
                Rectangle(memDC, 115, py - 16, 140, py - 12);
                Rectangle(memDC, 115, py + 12, 140, py + 16);

                // Pulsating Antimatter Core
                int coreRadius = 5 + (FastSin(animTick * 12) * 2) / 100;
                HBRUSH hCoreBrush = CreateSolidBrush(RGB(255, 0, 255));
                SelectObject(memDC, hCoreBrush);
                Ellipse(memDC, 100 - coreRadius, py - coreRadius, 100 + coreRadius, py + coreRadius);

                // Specular sheen sweep
                int sheenX = 65 + ((animTick * 3) % 80);
                HPEN hSheenPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                SelectObject(memDC, hSheenPen);
                MoveToEx(memDC, sheenX - 8, py - 14, NULL);
                LineTo(memDC, sheenX + 8, py + 14);

                DeleteObject(hVBrush); DeleteObject(hVPen);
                DeleteObject(hTitanBrush); DeleteObject(hTitanPen);
                DeleteObject(hGunBrush); DeleteObject(hGunPen);
                DeleteObject(hCoreBrush); DeleteObject(hSheenPen);
            } else {
                // Standard Player Scout / Cruiser Sprite
                // Animated Engine Flame
                int flLen = 16 + (FastSin(animTick * 25) * 5) / 100;
                POINT flPts[3] = { {78, py - 6}, {78 - flLen, py}, {78, py + 6} };
                HBRUSH hEngBrush = CreateSolidBrush(RGB(255, 136, 0));
                HPEN hEngPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
                SelectObject(memDC, hEngBrush);
                SelectObject(memDC, hEngPen);
                Polygon(memDC, flPts, 3);

                // Sleek Hull
                HBRUSH hShipBrush = CreateSolidBrush(RGB(0, 150, 230));
                HPEN hShipPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                SelectObject(memDC, hShipBrush);
                SelectObject(memDC, hShipPen);
                POINT shipPts[6] = {
                    {135, py}, {82, py + 18}, {90, py + 6},
                    {76, py}, {90, py - 6}, {82, py - 18}
                };
                Polygon(memDC, shipPts, 6);

                // Cockpit Canopy
                HBRUSH hCanopyBrush = CreateSolidBrush(RGB(0, 255, 255));
                SelectObject(memDC, hCanopyBrush);
                Ellipse(memDC, 95, py - 4, 115, py + 4);

                // Weapon Blasters based on level
                HPEN hCyanPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
                SelectObject(memDC, hCyanPen);
                if (state.weaponLevel >= 1) {
                    Rectangle(memDC, 92, py - 20, 106, py - 18);
                    Rectangle(memDC, 92, py + 18, 106, py + 20);
                }
                if (state.weaponLevel >= 2) {
                    Rectangle(memDC, 125, py - 2, 138, py + 2);
                }

                // Specular Sheen line
                int sheenX = 76 + ((animTick * 3) % 60);
                HPEN hSheenPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                SelectObject(memDC, hSheenPen);
                MoveToEx(memDC, sheenX - 6, py - 10, NULL);
                LineTo(memDC, sheenX + 6, py + 10);

                DeleteObject(hEngBrush); DeleteObject(hEngPen);
                DeleteObject(hShipBrush); DeleteObject(hShipPen);
                DeleteObject(hCanopyBrush); DeleteObject(hCyanPen);
                DeleteObject(hSheenPen);
            }

            // Player Shield Bubble
            if (state.inCombat && state.playerShields > 0) {
                HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                HPEN hShieldPen = CreatePen(PS_SOLID, shieldHitPlayer > 0 ? 2 : 1, shieldHitPlayer > 0 ? RGB(255, 255, 255) : RGB(0, 220, 255));
                SelectObject(memDC, hNullBrush);
                SelectObject(memDC, hShieldPen);
                Ellipse(memDC, 54, py - 32, 146, py + 32);
                DeleteObject(hShieldPen);
            }

            // Draw Right Side (Combat Pirate OR Peaceful Planet)
            if (state.inCombat) {
                // Pirate Raider
                int ey = 75 + (FastCos(animTick * 7) * 4) / 100;
                
                // Pirate Flame
                int pflLen = 16 + (FastSin(animTick * 25) * 5) / 100;
                POINT pflPts[3] = { {515, ey - 6}, {515 + pflLen, ey}, {515, ey + 6} };
                HBRUSH hPEng = CreateSolidBrush(RGB(255, 255, 0));
                HPEN hPEngPen = CreatePen(PS_SOLID, 1, RGB(255, 80, 0));
                SelectObject(memDC, hPEng);
                SelectObject(memDC, hPEngPen);
                Polygon(memDC, pflPts, 3);

                // Pirate Hull
                HBRUSH hPirateBrush = CreateSolidBrush(RGB(204, 17, 0));
                HPEN hPiratePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                SelectObject(memDC, hPirateBrush);
                SelectObject(memDC, hPiratePen);
                POINT pPts[6] = {
                    {465, ey}, {518, ey - 24}, {510, ey - 8},
                    {524, ey}, {510, ey + 8}, {518, ey + 24}
                };
                Polygon(memDC, pPts, 6);

                // Optic Visor
                HBRUSH hVisor = CreateSolidBrush(RGB(255, 255, 0));
                SelectObject(memDC, hVisor);
                Ellipse(memDC, 492, ey - 3, 508, ey + 3);

                // Disruptors
                HBRUSH hDisr = CreateSolidBrush(RGB(50, 0, 0));
                SelectObject(memDC, hDisr);
                Rectangle(memDC, 478, ey - 16, 490, ey - 13);
                Rectangle(memDC, 478, ey + 13, 490, ey + 16);

                DeleteObject(hPEng); DeleteObject(hPEngPen);
                DeleteObject(hPirateBrush); DeleteObject(hPiratePen);
                DeleteObject(hVisor); DeleteObject(hDisr);

                // Pirate Shield
                if (state.enemyShields > 0) {
                    HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                    HPEN hEShieldPen = CreatePen(PS_SOLID, shieldHitEnemy > 0 ? 2 : 1, shieldHitEnemy > 0 ? RGB(255, 200, 200) : RGB(255, 50, 50));
                    SelectObject(memDC, hNullBrush);
                    SelectObject(memDC, hEShieldPen);
                    Ellipse(memDC, 454, ey - 32, 546, ey + 32);
                    DeleteObject(hEShieldPen);
                }
            } else {
                // Celestial Planet
                int eco = planets[state.location].ecoType;
                COLORREF pAtm = RGB(79, 195, 247), pCore = RGB(13, 71, 161), pDetail = RGB(46, 125, 50);
                if (eco == 0) { pAtm = RGB(102, 187, 106); pCore = RGB(27, 94, 32); pDetail = RGB(255, 213, 79); }
                else if (eco == 1) { pAtm = RGB(255, 112, 67); pCore = RGB(191, 54, 12); pDetail = RGB(62, 39, 35); }
                else if (eco == 2) { pAtm = RGB(0, 229, 255); pCore = RGB(0, 96, 100); pDetail = RGB(24, 255, 255); }
                else if (eco == 3) { pAtm = RGB(255, 179, 0); pCore = RGB(55, 71, 79); pDetail = RGB(255, 87, 34); }

                // Atmosphere glow
                HBRUSH hNull = (HBRUSH)GetStockObject(NULL_BRUSH);
                HPEN hAtmPen = CreatePen(PS_SOLID, 2, pAtm);
                SelectObject(memDC, hNull);
                SelectObject(memDC, hAtmPen);
                Ellipse(memDC, 450, 25, 550, 125);
                DeleteObject(hAtmPen);

                // Planet Body
                HBRUSH hPlanet = CreateSolidBrush(pCore);
                HPEN hPlanetPen = CreatePen(PS_SOLID, 1, pAtm);
                SelectObject(memDC, hPlanet);
                SelectObject(memDC, hPlanetPen);
                Ellipse(memDC, 460, 35, 540, 115);
                DeleteObject(hPlanet); DeleteObject(hPlanetPen);

                // Shadow crescent
                HBRUSH hShadow = CreateSolidBrush(RGB(10, 15, 30));
                SelectObject(memDC, hShadow);
                Ellipse(memDC, 475, 45, 542, 117);
                DeleteObject(hShadow);

                // Planetary Craters / Surface features
                HBRUSH hDet = CreateSolidBrush(pDetail);
                SelectObject(memDC, hDet);
                Ellipse(memDC, 480, 55, 496, 71);
                Ellipse(memDC, 505, 80, 525, 100);
                Ellipse(memDC, 485, 92, 497, 104);
                DeleteObject(hDet);

                // Planetary Rings for Mining & Tech
                if (eco == 1 || eco == 2) {
                    HPEN hRingPen = CreatePen(PS_SOLID, 2, eco == 1 ? RGB(255, 170, 100) : RGB(0, 255, 255));
                    SelectObject(memDC, hNull);
                    SelectObject(memDC, hRingPen);
                    Arc(memDC, 440, 65, 560, 85, 440, 75, 560, 75);
                    DeleteObject(hRingPen);
                }

                // Orbiting Satellite Beacon
                int satX = 500 + (FastCos(animTick * 3) * 52) / 100;
                int satY = 75 + (FastSin(animTick * 3) * 22) / 100;
                HBRUSH hSatBrush = CreateSolidBrush(RGB(0, 255, 255));
                HPEN hSatPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                SelectObject(memDC, hSatBrush);
                SelectObject(memDC, hSatPen);
                Rectangle(memDC, satX - 2, satY - 2, satX + 3, satY + 3);
                DeleteObject(hSatBrush); DeleteObject(hSatPen);
            }

            // Laser Projectiles
            for (int i = 0; i < MAX_LASERS; i++) {
                if (lasers[i].active) {
                    int curX = lasers[i].x + ((lasers[i].tx - lasers[i].x) * lasers[i].progress) / 100;
                    int curY = lasers[i].y + ((lasers[i].ty - lasers[i].y) * lasers[i].progress) / 100;
                    int dir = lasers[i].fromPlayer ? -1 : 1;
                    HPEN hLaserPen = CreatePen(PS_SOLID, 3, lasers[i].color);
                    SelectObject(memDC, hLaserPen);
                    MoveToEx(memDC, curX, curY, NULL);
                    LineTo(memDC, curX + dir * 20, curY);
                    DeleteObject(hLaserPen);
                }
            }

            // 4-Layer Kinematic Particles
            for (int p = 0; p < MAX_PARTICLES; p++) {
                if (particles[p].active) {
                    int px = particles[p].x / 10;
                    int py = particles[p].y / 10;
                    if (px >= 0 && px < 600 && py >= 0 && py < 150) {
                        if (particles[p].layer == 0) { // spark
                            HPEN hSpkPen = CreatePen(PS_SOLID, 1, particles[p].color);
                            SelectObject(memDC, hSpkPen);
                            MoveToEx(memDC, px, py, NULL);
                            LineTo(memDC, px - particles[p].vx * 2, py - particles[p].vy * 2);
                            DeleteObject(hSpkPen);
                        } else if (particles[p].layer == 1) { // smoke
                            HBRUSH hSmk = CreateSolidBrush(particles[p].color);
                            SelectObject(memDC, hSmk);
                            Ellipse(memDC, px - particles[p].size, py - particles[p].size, px + particles[p].size, py + particles[p].size);
                            DeleteObject(hSmk);
                        } else if (particles[p].layer == 2) { // debris
                            HBRUSH hDeb = CreateSolidBrush(particles[p].color);
                            SelectObject(memDC, hDeb);
                            Rectangle(memDC, px - particles[p].size / 2, py - particles[p].size / 2, px + particles[p].size / 2, py + particles[p].size / 2);
                            DeleteObject(hDeb);
                        } else if (particles[p].layer == 3) { // celebration star
                            HPEN hStarPen = CreatePen(PS_SOLID, 2, particles[p].color);
                            SelectObject(memDC, hStarPen);
                            MoveToEx(memDC, px - particles[p].size, py, NULL);
                            LineTo(memDC, px + particles[p].size, py);
                            MoveToEx(memDC, px, py - particles[p].size, NULL);
                            LineTo(memDC, px, py + particles[p].size);
                            DeleteObject(hStarPen);
                        }
                    }
                }
            }

            // Cybernetic HUD Corner Filigree Brackets
            HPEN hHudPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
            SelectObject(memDC, hHudPen);
            // Top-Left
            MoveToEx(memDC, 6, 18, NULL); LineTo(memDC, 6, 6); LineTo(memDC, 18, 6);
            // Top-Right
            MoveToEx(memDC, 582, 6, NULL); LineTo(memDC, 594, 6); LineTo(memDC, 594, 18);
            // Bottom-Left
            MoveToEx(memDC, 6, 132, NULL); LineTo(memDC, 6, 144); LineTo(memDC, 18, 144);
            // Bottom-Right
            MoveToEx(memDC, 582, 144, NULL); LineTo(memDC, 594, 144); LineTo(memDC, 594, 132);

            // Traveling frame glint
            int glintPos = (animTick * 6) % 1500;
            int gx = 0, gy = 0;
            if (glintPos < 600) { gx = glintPos; gy = 0; }
            else if (glintPos < 750) { gx = 600; gy = glintPos - 600; }
            else if (glintPos < 1350) { gx = 600 - (glintPos - 750); gy = 150; }
            else { gx = 0; gy = 150 - (glintPos - 1350); }
            HBRUSH hGlint = CreateSolidBrush(RGB(255, 255, 255));
            SelectObject(memDC, hGlint);
            Rectangle(memDC, gx - 2, gy - 2, gx + 3, gy + 3);
            DeleteObject(hGlint);
            DeleteObject(hHudPen);

            // Blit double-buffered frame to screen with screen shake offset
            int offX = 0, offY = 0;
            if (shakeAmount > 0) {
                offX = (FastCos(shakeAngle) * shakeAmount) / 100;
                offY = (FastSin(shakeAngle) * shakeAmount) / 100;
            }
            BitBlt(hdc, 20 + offX, 45 + offY, 600, 150, memDC, 0, 0, SRCCOPY);

            // Clean up
            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
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
            KillTimer(hwnd, 1);
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
        CW_USEDEFAULT, CW_USEDEFAULT, 650, 700,
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
