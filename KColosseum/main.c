#include <windows.h>
#include <stdio.h>
#include <string.h>

static unsigned int g_seed = 0;
void my_srand(unsigned int seed) {
    g_seed = seed;
}
int my_rand() {
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7FFF;
}

static const short g_sinTable[16] = {
    0, 382, 707, 923, 1000, 923, 707, 382,
    0, -382, -707, -923, -1000, -923, -707, -382
};
int FastSin(int step) {
    int idx = (step & 15);
    return g_sinTable[idx];
}
int FastCos(int step) {
    int idx = ((step + 4) & 15);
    return g_sinTable[idx];
}

DWORD WINAPI SoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (type == 1) { Beep(800, 100); }
    else if (type == 2) { Beep(150, 200); }
    else if (type == 3) { Beep(500, 100); Beep(550, 100); Beep(600, 100); Beep(650, 200); }
    else if (type == 4) { Beep(440, 150); Sleep(50); Beep(440, 150); Sleep(50); Beep(440, 150); Sleep(50); Beep(587, 400); }
    else if (type == 5) { Beep(200, 500); Beep(150, 1000); }
    return 0;
}
void PlaySoundAsync(int type) {
    CreateThread(NULL, 0, SoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}

#define ID_BUY_BUTTON 101
#define ID_MARKET_LIST 102
#define ID_OWNED_LIST 103
#define ID_FUNDS_LABEL 104
#define ID_REFRESH_BUTTON 105
#define ID_TRAIN_STR 106
#define ID_TRAIN_AGI 107
#define ID_TRAIN_VIT 108
#define ID_FIGHT_BUTTON 109
#define ID_ATTACK_BUTTON 110
#define ID_DEFEND_BUTTON 111
#define ID_FLEE_BUTTON 112
#define ID_COMBAT_LOG 113
#define ID_EQ_GLADIUS 114
#define ID_EQ_TRIDENT 115
#define ID_EQ_ARMOR 116
#define ID_EQ_SHIELD 117
#define ID_HEAL_BUTTON 118
#define ID_SHOWBOAT_BUTTON 119
#define ID_FAVOR_LABEL 120
#define ID_HELP_BUTTON 121
#define ID_HELP_BACK_BUTTON 122

typedef struct {
    int id;
    char name[32];
    int cost;
    char desc[128];
    int str;
    int agi;
    int vit;
    int weapon;
    int armor;
    int shield;
    int damageTaken;
    int isTwins;
    int isBeast;
} Gladiator;

typedef struct {
    int x, y;
    int vx, vy;
    int size;
    COLORREF color;
    int life;
    int maxLife;
    int type; // 0: spark, 1: dust, 2: shard, 3: coin, 4: cross
    int rot;
} Particle;

typedef struct {
    char text[32];
    int x, y;
    COLORREF color;
    int life;
} Floater;

#define MAX_PARTICLES 128
#define MAX_FLOATERS 16
Particle g_particles[MAX_PARTICLES];
int g_particleCount = 0;
Floater g_floaters[MAX_FLOATERS];
int g_floaterCount = 0;

int g_shakeAmt = 0;
int g_playerLunge = 0;
int g_enemyLunge = 0;
int g_playerFlash = 0;
int g_enemyFlash = 0;
int g_playerStance = 0; // 0: idle, 1: attack, 2: defend, 3: showboat, 4: dead
int g_enemyStance = 0;
int g_animTick = 0;
int g_currentView = 0; // 0: Dash, 1: Combat, 2: Help

HWND g_hWndMain = NULL;

const char* firstNames[] = {"Titus", "Flamma", "Spiculus", "Marcus", "Lucius", "Gaius", "Quintus", "Aulus"};
const char* epithets[] = {"the Strong", "the Swift", "the Bear", "the Lion", "the Fierce", "the Giant"};
int nextId = 1;
int arenaLevel = 1;

const char* GetLeagueName(int level) {
    if (level < 3) return "Local Pits";
    if (level < 6) return "Provincial Arena";
    if (level < 9) return "Capital Amphitheatre";
    if (level < 10) return "The Grand Colosseum";
    return "Champion of Rome";
}

int GetEffStr(Gladiator* g);
int GetEffAgi(Gladiator* g);
int GetEffVit(Gladiator* g);

void UpdateGladiatorDesc(Gladiator* g) {
    char eqStr[64];
    eqStr[0] = '\0';
    if (g->weapon == 1) lstrcatA(eqStr, " [Glad]");
    else if (g->weapon == 2) lstrcatA(eqStr, " [Trid]");
    if (g->armor == 1) lstrcatA(eqStr, " [Armr]");
    if (g->shield == 1) lstrcatA(eqStr, " [Shld]");
    int maxHp = GetEffVit(g) * 10;
    int hp = maxHp - g->damageTaken;
    wsprintfA(g->desc, "HP:%d/%d STR:%d AGI:%d VIT:%d%s", hp, maxHp, g->str, g->agi, g->vit, eqStr);
}

Gladiator GenerateGladiator(int forArena) {
    Gladiator g;
    g.weapon = 0;
    g.armor = 0;
    g.shield = 0;
    g.damageTaken = 0;
    g.isTwins = 0;
    g.isBeast = 0;
    g.desc[0] = '\0';
    g.id = nextId++;
    
    int fNameIdx = my_rand() % 8;
    if (my_rand() % 2 == 0) {
        int epIdx = my_rand() % 6;
        wsprintfA(g.name, "%s %s", firstNames[fNameIdx], epithets[epIdx]);
    } else {
        wsprintfA(g.name, "%s", firstNames[fNameIdx]);
    }
    
    g.str = (my_rand() % 10) + 1;
    g.agi = (my_rand() % 10) + 1;
    g.vit = (my_rand() % 10) + 1;
    
    if (forArena || (my_rand() % 100) < 30) {
        if ((my_rand() % 100) < 30) g.weapon = (my_rand() % 2) + 1;
        if ((my_rand() % 100) < 30) g.armor = 1;
        if ((my_rand() % 100) < 30) g.shield = 1;
    }
    
    int statTotal = g.str + g.agi + g.vit;
    g.cost = statTotal * 20 + (my_rand() % 50);
    if (g.weapon > 0) g.cost += 50;
    if (g.armor > 0) g.cost += 50;
    if (g.shield > 0) g.cost += 30;
    
    UpdateGladiatorDesc(&g);
    return g;
}

int GetEffStr(Gladiator* g) { return g->str + (g->weapon == 1 ? 3 : 0); }
int GetEffAgi(Gladiator* g) { return g->agi + (g->weapon == 2 ? 3 : 0); }
int GetEffVit(Gladiator* g) { return g->vit + (g->armor == 1 ? 5 : 0); }

Gladiator market[10];
int market_count = 0;

Gladiator owned[10];
int owned_count = 0;

int funds = 1000;

HWND hTitle, hFundsLabel, hL1, hRefreshButton, hMarketList, hBuyButton, hL2, hOwnedList, hTrainStrBtn, hTrainAgiBtn, hTrainVitBtn, hFightBtn;
HWND hEqGladiusBtn, hEqTridentBtn, hEqArmorBtn, hEqShieldBtn, hHealBtn;
HWND hCombatTitle, hCombatPlayer, hCombatEnemy, hAttackBtn, hDefendBtn, hShowboatBtn, hFleeBtn, hCombatLog, hFavorLabel;
HWND hHelpBtn, hHelpTitle, hHelpText, hHelpBackBtn;

Gladiator* currentFighter = NULL;
Gladiator enemyFighter;
int playerHp, playerMaxHp, enemyHp, enemyMaxHp;
int playerDefending = 0;
int enemyDefending = 0;
int combatOver = 0;
int crowdFavor = 0;

HBRUSH hbrBkgnd, hbrCrimson, hbrList;
HFONT hFont, hTitleFont;

void AddScreenShake(int amt) {
    if (amt > g_shakeAmt) g_shakeAmt = amt;
}

void SpawnParticles(int x, int y, int count, int type, COLORREF color) {
    for (int i = 0; i < count && g_particleCount < MAX_PARTICLES; i++) {
        Particle* p = &g_particles[g_particleCount++];
        p->x = x;
        p->y = y;
        p->type = type;
        p->color = color;
        p->life = 20 + (my_rand() % 15);
        p->maxLife = p->life;
        p->rot = my_rand() % 360;
        p->size = (my_rand() % 4) + 2;

        if (type == 0) { // Spark
            p->vx = (my_rand() % 15) - 7;
            p->vy = (my_rand() % 15) - 7;
        } else if (type == 1) { // Dust
            p->vx = (my_rand() % 7) - 3;
            p->vy = -(my_rand() % 5) - 1;
            p->size = (my_rand() % 5) + 4;
        } else if (type == 2) { // Shard
            p->vx = (my_rand() % 11) - 5;
            p->vy = -(my_rand() % 8) - 2;
            p->size = (my_rand() % 4) + 3;
        } else if (type == 3) { // Coin
            p->x = x + (my_rand() % 100) - 50;
            p->y = 10;
            p->vx = (my_rand() % 5) - 2;
            p->vy = (my_rand() % 4) + 3;
            p->size = 5;
        } else if (type == 4) { // Heal
            p->x = x + (my_rand() % 40) - 20;
            p->y = y + (my_rand() % 30) - 15;
            p->vx = (my_rand() % 3) - 1;
            p->vy = -(my_rand() % 3) - 1;
            p->size = 6;
        }
    }
}

void AddFloatingText(const char* text, int x, int y, COLORREF color) {
    if (g_floaterCount < MAX_FLOATERS) {
        Floater* f = &g_floaters[g_floaterCount++];
        lstrcpyA(f->text, text);
        f->x = x;
        f->y = y;
        f->color = color;
        f->life = 25;
    }
}

void UpdateUI() {
    char buf[256];
    wsprintfA(buf, "Treasury: %d Denarii | League: %s (Level %d)", funds, GetLeagueName(arenaLevel), arenaLevel);
    SetWindowTextA(hFundsLabel, buf);

    SendMessageA(hMarketList, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < market_count; i++) {
        wsprintfA(buf, "%s - %s (%dD)", market[i].name, market[i].desc, market[i].cost);
        SendMessageA(hMarketList, LB_ADDSTRING, 0, (LPARAM)buf);
    }

    EnableWindow(hRefreshButton, funds >= 50);

    SendMessageA(hOwnedList, LB_RESETCONTENT, 0, 0);
    if (owned_count == 0) {
        SendMessageA(hOwnedList, LB_ADDSTRING, 0, (LPARAM)"No gladiators owned.");
    } else {
        for (int i = 0; i < owned_count; i++) {
            wsprintfA(buf, "%s - %s", owned[i].name, owned[i].desc);
            SendMessageA(hOwnedList, LB_ADDSTRING, 0, (LPARAM)buf);
        }
    }
}

void BuyGladiator(int index) {
    if (index >= 0 && index < market_count && funds >= market[index].cost) {
        funds -= market[index].cost;
        owned[owned_count++] = market[index];
        for (int i = index; i < market_count - 1; i++) {
            market[i] = market[i + 1];
        }
        market_count--;
        UpdateUI();
    } else if (index >= 0 && index < market_count) {
        MessageBoxA(NULL, "Not enough funds!", "Error", MB_OK | MB_ICONWARNING);
    }
}

void SwitchView(int view) {
    g_currentView = view;
    int cmdDash = (view == 0) ? SW_SHOW : SW_HIDE;
    int cmdComb = (view == 1) ? SW_SHOW : SW_HIDE;
    int cmdHelp = (view == 2) ? SW_SHOW : SW_HIDE;

    ShowWindow(hTitle, cmdDash);
    ShowWindow(hFundsLabel, cmdDash);
    ShowWindow(hL1, cmdDash);
    ShowWindow(hRefreshButton, cmdDash);
    ShowWindow(hMarketList, cmdDash);
    ShowWindow(hBuyButton, cmdDash);
    ShowWindow(hL2, cmdDash);
    ShowWindow(hOwnedList, cmdDash);
    ShowWindow(hEqGladiusBtn, cmdDash);
    ShowWindow(hEqTridentBtn, cmdDash);
    ShowWindow(hEqArmorBtn, cmdDash);
    ShowWindow(hEqShieldBtn, cmdDash);
    ShowWindow(hTrainStrBtn, cmdDash);
    ShowWindow(hTrainAgiBtn, cmdDash);
    ShowWindow(hTrainVitBtn, cmdDash);
    ShowWindow(hFightBtn, cmdDash);
    ShowWindow(hHealBtn, cmdDash);
    ShowWindow(hHelpBtn, cmdDash);

    ShowWindow(hCombatTitle, SW_HIDE);
    ShowWindow(hCombatPlayer, cmdComb);
    ShowWindow(hCombatEnemy, cmdComb);
    ShowWindow(hAttackBtn, cmdComb);
    ShowWindow(hDefendBtn, cmdComb);
    ShowWindow(hShowboatBtn, cmdComb);
    ShowWindow(hFleeBtn, cmdComb);
    ShowWindow(hCombatLog, cmdComb);
    ShowWindow(hFavorLabel, cmdComb);

    ShowWindow(hHelpTitle, cmdHelp);
    ShowWindow(hHelpText, cmdHelp);
    ShowWindow(hHelpBackBtn, cmdHelp);

    if (view == 1 && g_hWndMain) {
        SetTimer(g_hWndMain, 1, 33, NULL);
    } else if (g_hWndMain) {
        KillTimer(g_hWndMain, 1);
        InvalidateRect(g_hWndMain, NULL, TRUE);
    }
}

void LogCombat(const char* msg) {
    SendMessageA(hCombatLog, LB_ADDSTRING, 0, (LPARAM)msg);
    int count = SendMessageA(hCombatLog, LB_GETCOUNT, 0, 0);
    SendMessageA(hCombatLog, LB_SETTOPINDEX, count - 1, 0);
}

void UpdateCombatUI() {
    char buf[256];
    wsprintfA(buf, "%s\nHP: %d / %d\nSTR:%d AGI:%d VIT:%d",
        currentFighter->name, playerHp, playerMaxHp, GetEffStr(currentFighter), GetEffAgi(currentFighter), GetEffVit(currentFighter));
    SetWindowTextA(hCombatPlayer, buf);

    wsprintfA(buf, "%s\nHP: %d / %d\nSTR:%d AGI:%d VIT:%d",
        enemyFighter.name, enemyHp, enemyMaxHp, GetEffStr(&enemyFighter), GetEffAgi(&enemyFighter), GetEffVit(&enemyFighter));
    SetWindowTextA(hCombatEnemy, buf);

    wsprintfA(buf, "Crowd Favor: %d%%", crowdFavor);
    SetWindowTextA(hFavorLabel, buf);
}

void CheckCrowdFavor() {
    if (crowdFavor >= 100) {
        crowdFavor = 0;
        PlaySoundAsync(3);
        AddScreenShake(8);
        char buf[128];
        if ((my_rand() % 100) < 50) {
            int gold = 50 + (my_rand() % 51);
            funds += gold;
            wsprintfA(buf, "The crowd goes wild! They throw %d Denarii into the arena!", gold);
            LogCombat(buf);
            SpawnParticles(275, 40, 20, 3, RGB(255, 215, 0));
            AddFloatingText("+DENARII!", 135, 60, RGB(255, 215, 0));
        } else {
            int heal = 20 + (my_rand() % 21);
            playerHp += heal;
            if (playerHp > playerMaxHp) playerHp = playerMaxHp;
            wsprintfA(buf, "The crowd cheers! A medical sponge is thrown! Healed %d HP!", heal);
            LogCombat(buf);
            SpawnParticles(135, 95, 18, 4, RGB(50, 205, 50));
            AddFloatingText("+HEAL!", 135, 60, RGB(50, 205, 50));
        }
        UpdateCombatUI();
        UpdateUI();
    }
}

void EnterArena(int index) {
    currentFighter = &owned[index];
    playerMaxHp = GetEffVit(currentFighter) * 10;
    playerHp = playerMaxHp - currentFighter->damageTaken;
    
    enemyFighter = GenerateGladiator(1);
    
    int isSpecial = (arenaLevel > 2 && (my_rand() % 100) < 35);
    if (isSpecial) {
        int eventType = my_rand() % 3;
        if (eventType == 0) {
            lstrcpyA(enemyFighter.name, "Ferocious Lion");
            enemyFighter.str += arenaLevel * 2;
            enemyFighter.agi += arenaLevel * 2;
            enemyFighter.vit += arenaLevel * 2;
            enemyFighter.weapon = 0; enemyFighter.shield = 0; enemyFighter.armor = 0;
            enemyFighter.isBeast = 1;
        } else if (eventType == 1) {
            lstrcpyA(enemyFighter.name, "Armed Chariot");
            enemyFighter.vit += arenaLevel * 3;
            enemyFighter.str += (arenaLevel * 3) / 2;
            enemyFighter.armor = 1; enemyFighter.shield = 1;
        } else {
            lstrcpyA(enemyFighter.name, "Twin Gladiators");
            enemyFighter.vit += (arenaLevel * 3) / 2;
            enemyFighter.str += (arenaLevel * 3) / 2;
            enemyFighter.isTwins = 1;
        }
    } else {
        enemyFighter.str += (arenaLevel * 3) / 2;
        enemyFighter.agi += (arenaLevel * 3) / 2;
        enemyFighter.vit += (arenaLevel * 3) / 2;
        if (arenaLevel > 2 && (my_rand() % 100) < 50) enemyFighter.weapon = (my_rand() % 2) + 1;
        if (arenaLevel > 4 && (my_rand() % 100) < 50) enemyFighter.shield = 1;
        if (arenaLevel > 6 && (my_rand() % 100) < 50) enemyFighter.armor = 1;
        if (arenaLevel > 8) { enemyFighter.weapon = (my_rand() % 2) + 1; enemyFighter.shield = 1; enemyFighter.armor = 1; }
    }
    
    UpdateGladiatorDesc(&enemyFighter);
    
    enemyMaxHp = GetEffVit(&enemyFighter) * 10;
    enemyHp = enemyMaxHp;

    combatOver = 0;
    playerDefending = 0;
    enemyDefending = 0;
    crowdFavor = 0;
    g_particleCount = 0;
    g_floaterCount = 0;
    g_playerLunge = 0;
    g_enemyLunge = 0;
    g_playerFlash = 0;
    g_enemyFlash = 0;
    g_playerStance = 0;
    g_enemyStance = 0;

    SendMessageA(hCombatLog, LB_RESETCONTENT, 0, 0);
    char buf[128];
    wsprintfA(buf, "Match starts! %s vs %s", currentFighter->name, enemyFighter.name);
    LogCombat(buf);

    PlaySoundAsync(3);
    UpdateCombatUI();
    SwitchView(1);
}

void CombatAction(int action) {
    if (combatOver) {
        SwitchView(0);
        return;
    }
    
    char buf[128];
    if (action == 2) { // flee
        wsprintfA(buf, "%s flees the arena in disgrace!", currentFighter->name);
        LogCombat(buf);
        PlaySoundAsync(5);
        AddFloatingText("FLED!", 135, 70, RGB(255, 60, 60));
        if (arenaLevel > 1) {
            arenaLevel--;
            wsprintfA(buf, "Your ludus drops to level %d...", arenaLevel);
            LogCombat(buf);
        }
        combatOver = 1;
        SetWindowTextA(hAttackBtn, "Leave");
        EnableWindow(hDefendBtn, FALSE);
        EnableWindow(hShowboatBtn, FALSE);
        EnableWindow(hFleeBtn, FALSE);
        return;
    }

    if (action == 3) { // showboat
        int favorGain = 30 + (my_rand() % 21);
        crowdFavor += favorGain;
        wsprintfA(buf, "%s plays to the crowd!", currentFighter->name);
        LogCombat(buf);
        g_playerStance = 3;
        AddFloatingText("+FAVOR!", 135, 70, RGB(255, 215, 0));
        SpawnParticles(135, 80, 15, 0, RGB(255, 215, 0));
        CheckCrowdFavor();
    }

    playerDefending = (action == 1);
    if (playerDefending) {
        wsprintfA(buf, "%s takes a defensive stance.", currentFighter->name);
        LogCombat(buf);
        crowdFavor -= 5;
        if (crowdFavor < 0) crowdFavor = 0;
        g_playerStance = 2;
        AddFloatingText("DEFEND", 135, 70, RGB(255, 215, 0));
        SpawnParticles(135, 110, 8, 1, RGB(180, 150, 110));
    }

    if (action == 0) { // attack
        g_playerStance = 1;
        g_playerLunge = 35;
        int hitChance = 75 + (GetEffAgi(currentFighter) - GetEffAgi(&enemyFighter)) * 5;
        if (enemyDefending) hitChance -= (enemyFighter.shield == 1 ? 50 : 30);
        
        if ((my_rand() % 100) < hitChance) {
            int dmg = GetEffStr(currentFighter) + (my_rand() % 4);
            if (enemyDefending) dmg -= (enemyFighter.shield == 1 ? 5 : 3);
            if (dmg < 1) dmg = 1;
            enemyHp -= dmg;
            crowdFavor += 5 + (my_rand() % 11);
            wsprintfA(buf, "%s hits for %d damage!", currentFighter->name, dmg);
            LogCombat(buf);
            PlaySoundAsync(1);
            g_enemyFlash = 8;
            AddScreenShake(dmg > 10 ? 8 : 5);
            SpawnParticles(415, 95, 18, 0, RGB(255, 240, 150));
            SpawnParticles(415, 95, 8, 2, RGB(212, 175, 55));
            char dmgStr[16];
            wsprintfA(dmgStr, "-%d", dmg);
            AddFloatingText(dmgStr, 415, 70, RGB(255, 60, 60));
            CheckCrowdFavor();
        } else {
            crowdFavor -= 5;
            if (crowdFavor < 0) crowdFavor = 0;
            wsprintfA(buf, "%s misses!", currentFighter->name);
            LogCombat(buf);
            PlaySoundAsync(2);
            AddFloatingText("MISS!", 415, 70, RGB(180, 180, 180));
            SpawnParticles(415, 115, 6, 1, RGB(180, 150, 110));
        }
    }

    if (enemyHp <= 0) {
        enemyHp = 0;
        g_enemyStance = 4;
        UpdateCombatUI();
        int reward = 100 + (arenaLevel * 50);
        if (lstrcmpA(enemyFighter.name, "Ferocious Lion") == 0 || lstrcmpA(enemyFighter.name, "Armed Chariot") == 0 || enemyFighter.isTwins) {
            reward += 100 + (arenaLevel * 20);
        }
        wsprintfA(buf, "%s is defeated! You win %d Denarii!", enemyFighter.name, reward);
        LogCombat(buf);
        PlaySoundAsync(4);
        funds += reward;
        AddScreenShake(12);
        SpawnParticles(135, 70, 25, 0, RGB(255, 215, 0));
        SpawnParticles(275, 30, 20, 3, RGB(255, 215, 0));
        AddFloatingText("VICTORY!", 275, 50, RGB(255, 215, 0));

        if (arenaLevel < 10) {
            arenaLevel++;
            wsprintfA(buf, "Your ludus advances to level %d!", arenaLevel);
            LogCombat(buf);
        }
        UpdateUI();
        combatOver = 1;
        SetWindowTextA(hAttackBtn, "Leave");
        EnableWindow(hDefendBtn, FALSE);
        EnableWindow(hShowboatBtn, FALSE);
        EnableWindow(hFleeBtn, FALSE);
        return;
    }

    enemyDefending = (my_rand() % 100) < 25;
    if (enemyDefending) {
        wsprintfA(buf, "%s takes a defensive stance.", enemyFighter.name);
        LogCombat(buf);
        g_enemyStance = 2;
        AddFloatingText("DEFEND", 415, 70, RGB(255, 215, 0));
    } else {
        g_enemyStance = 1;
        g_enemyLunge = 35;
        int hitChance = 75 + (GetEffAgi(&enemyFighter) - GetEffAgi(currentFighter)) * 5;
        if (playerDefending) hitChance -= (currentFighter->shield == 1 ? 50 : 30);
        
        if ((my_rand() % 100) < hitChance) {
            int dmg = GetEffStr(&enemyFighter) + (my_rand() % 4);
            if (playerDefending) dmg -= (currentFighter->shield == 1 ? 5 : 3);
            if (dmg < 1) dmg = 1;
            playerHp -= dmg;
            currentFighter->damageTaken += dmg;
            wsprintfA(buf, "%s hits for %d damage!", enemyFighter.name, dmg);
            LogCombat(buf);
            PlaySoundAsync(1);
            g_playerFlash = 8;
            AddScreenShake(dmg > 10 ? 8 : 5);
            SpawnParticles(135, 95, 18, 0, RGB(255, 240, 150));
            SpawnParticles(135, 95, 8, 2, RGB(212, 175, 55));
            char dmgStr[16];
            wsprintfA(dmgStr, "-%d", dmg);
            AddFloatingText(dmgStr, 135, 70, RGB(255, 60, 60));
        } else {
            wsprintfA(buf, "%s misses!", enemyFighter.name);
            LogCombat(buf);
            PlaySoundAsync(2);
            AddFloatingText("MISS!", 135, 70, RGB(180, 180, 180));
            SpawnParticles(135, 115, 6, 1, RGB(180, 150, 110));
        }
        
        if (enemyFighter.isTwins && playerHp > 0) {
            g_enemyLunge = 35;
            if ((my_rand() % 100) < hitChance) {
                int dmg = GetEffStr(&enemyFighter) + (my_rand() % 4);
                if (playerDefending) dmg -= (currentFighter->shield == 1 ? 5 : 3);
                if (dmg < 1) dmg = 1;
                playerHp -= dmg;
                currentFighter->damageTaken += dmg;
                wsprintfA(buf, "%s (Twin 2) hits for %d damage!", enemyFighter.name, dmg);
                LogCombat(buf);
                PlaySoundAsync(1);
                g_playerFlash = 8;
                AddScreenShake(5);
                SpawnParticles(135, 95, 12, 0, RGB(255, 240, 150));
                char dmgStr[16];
                wsprintfA(dmgStr, "-%d", dmg);
                AddFloatingText(dmgStr, 135, 70, RGB(255, 60, 60));
            } else {
                wsprintfA(buf, "%s (Twin 2) misses!", enemyFighter.name);
                LogCombat(buf);
                PlaySoundAsync(2);
                AddFloatingText("MISS!", 135, 70, RGB(180, 180, 180));
            }
        }
    }

    if (playerHp <= 0) {
        playerHp = 0;
        g_playerStance = 4;
        UpdateCombatUI();
        wsprintfA(buf, "%s is DEAD.", currentFighter->name);
        LogCombat(buf);
        PlaySoundAsync(5);
        AddScreenShake(10);
        AddFloatingText("DEAD", 135, 70, RGB(255, 0, 0));
        
        for (int i = 0; i < owned_count; i++) {
            if (&owned[i] == currentFighter) {
                for (int j = i; j < owned_count - 1; j++) {
                    owned[j] = owned[j + 1];
                }
                owned_count--;
                break;
            }
        }
        
        if (arenaLevel > 1) {
            arenaLevel--;
            wsprintfA(buf, "Your ludus drops to level %d...", arenaLevel);
            LogCombat(buf);
        }
        UpdateUI();
        combatOver = 1;
        SetWindowTextA(hAttackBtn, "Leave");
        EnableWindow(hDefendBtn, FALSE);
        EnableWindow(hShowboatBtn, FALSE);
        EnableWindow(hFleeBtn, FALSE);
        return;
    }

    UpdateCombatUI();
}

// GDI Procedural Drawing Helper Routines
void DrawArenaGDI(HDC hdc, int width, int height) {
    // 1. Arena Upper Wall & Arches
    HBRUSH hbrSky = CreateSolidBrush(RGB(45, 25, 12));
    RECT rcSky = {0, 0, width, height / 2};
    FillRect(hdc, &rcSky, hbrSky);
    DeleteObject(hbrSky);

    // Stone Pillars / Archways
    HBRUSH hbrPillar = CreateSolidBrush(RGB(115, 75, 40));
    HBRUSH hbrArchDark = CreateSolidBrush(RGB(30, 15, 6));
    HPEN hpenPillar = CreatePen(PS_SOLID, 1, RGB(70, 40, 20));
    HGDIOBJ oldBrush = SelectObject(hdc, hbrPillar);
    HGDIOBJ oldPen = SelectObject(hdc, hpenPillar);

    for (int x = 20; x < width; x += 65) {
        RoundRect(hdc, x, 15, x + 48, 75, 12, 12);
        SelectObject(hdc, hbrArchDark);
        RoundRect(hdc, x + 6, 22, x + 42, 75, 10, 10);
        SelectObject(hdc, hbrPillar);
    }

    // Imperial Banners (SPQR)
    HBRUSH hbrCrim = CreateSolidBrush(RGB(139, 0, 0));
    HPEN hpenGold = CreatePen(PS_SOLID, 2, RGB(212, 175, 55));
    SelectObject(hdc, hbrCrim);
    SelectObject(hdc, hpenGold);

    int bannerX[2] = {65, width - 95};
    for (int b = 0; b < 2; b++) {
        POINT pts[5] = {
            {bannerX[b], 5}, {bannerX[b] + 28, 5},
            {bannerX[b] + 28, 55}, {bannerX[b] + 14, 65}, {bannerX[b], 55}
        };
        Polygon(hdc, pts, 5);
    }

    // Torch Flames
    HBRUSH hbrTorchWood = CreateSolidBrush(RGB(65, 40, 15));
    HBRUSH hbrFire = CreateSolidBrush(RGB(255, 69, 0));
    HBRUSH hbrFireCore = CreateSolidBrush(RGB(255, 215, 0));
    HPEN hpenNull = (HPEN)GetStockObject(NULL_PEN);
    SelectObject(hdc, hpenNull);

    int torchX[2] = {130, width - 150};
    for (int t = 0; t < 2; t++) {
        SelectObject(hdc, hbrTorchWood);
        Rectangle(hdc, torchX[t], 45, torchX[t] + 4, 68);
        
        int flicker = (FastSin(g_animTick * 3 + torchX[t]) * 3) / 1000;
        SelectObject(hdc, hbrFire);
        Ellipse(hdc, torchX[t] - 4, 38 + flicker, torchX[t] + 8, 52 + flicker);
        SelectObject(hdc, hbrFireCore);
        Ellipse(hdc, torchX[t] - 2, 42 + flicker, torchX[t] + 6, 50 + flicker);
    }

    // 2. Sandy Arena Ground
    HBRUSH hbrSand = CreateSolidBrush(RGB(198, 158, 102));
    RECT rcSand = {0, height / 2, width, height};
    FillRect(hdc, &rcSand, hbrSand);
    DeleteObject(hbrSand);

    // Stone Rim Line
    HPEN hpenRim = CreatePen(PS_SOLID, 3, RGB(90, 60, 30));
    SelectObject(hdc, hpenRim);
    MoveToEx(hdc, 0, height / 2, NULL);
    LineTo(hdc, width, height / 2);

    // 3. Ornate Golden L-Bracket Corner Filigree
    SelectObject(hdc, hpenGold);
    int m = 6, len = 20;
    MoveToEx(hdc, m, m + len, NULL); LineTo(hdc, m, m); LineTo(hdc, m + len, m);
    MoveToEx(hdc, width - m - len, m, NULL); LineTo(hdc, width - m, m); LineTo(hdc, width - m, m + len);
    MoveToEx(hdc, m, height - m - len, NULL); LineTo(hdc, m, height - m); LineTo(hdc, m + len, height - m);
    MoveToEx(hdc, width - m - len, height - m, NULL); LineTo(hdc, width - m, height - m); LineTo(hdc, width - m, height - m - len);

    // Cleanup
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(hbrPillar);
    DeleteObject(hbrArchDark);
    DeleteObject(hpenPillar);
    DeleteObject(hbrCrim);
    DeleteObject(hpenGold);
    DeleteObject(hbrTorchWood);
    DeleteObject(hbrFire);
    DeleteObject(hbrFireCore);
    DeleteObject(hpenRim);
}

void DrawGladiatorGDI(HDC hdc, int x, int y, Gladiator* g, int isEnemy, int stance, int lunge, int flash) {
    int bob = (FastSin(g_animTick * 2) * 2) / 1000;
    int drawX = x + (isEnemy ? -lunge : lunge);
    int drawY = y + bob;

    if (stance == 4) { // Dead
        HBRUSH hbrDead = CreateSolidBrush(RGB(80, 20, 20));
        HGDIOBJ oldB = SelectObject(hdc, hbrDead);
        Ellipse(hdc, drawX - 20, drawY + 20, drawX + 20, drawY + 34);
        SelectObject(hdc, oldB);
        DeleteObject(hbrDead);
        return;
    }

    // Shadow
    HBRUSH hbrShadow = CreateSolidBrush(RGB(140, 110, 70));
    HPEN hpenNull = (HPEN)GetStockObject(NULL_PEN);
    HGDIOBJ oldB = SelectObject(hdc, hbrShadow);
    HGDIOBJ oldP = SelectObject(hdc, hpenNull);
    Ellipse(hdc, x - 18, y + 30, x + 18, y + 40);

    // Legs / Greaves
    HBRUSH hbrGreaves = CreateSolidBrush(RGB(205, 127, 50));
    SelectObject(hdc, hbrGreaves);
    Rectangle(hdc, drawX - 8, drawY + 12, drawX - 3, drawY + 32);
    Rectangle(hdc, drawX + 3, drawY + 12, drawX + 8, drawY + 32);
    DeleteObject(hbrGreaves);

    // Tunic (Red for player, Blue for enemy)
    COLORREF tunicCol = isEnemy ? (flash > 0 ? RGB(255, 60, 60) : RGB(30, 65, 105)) : (flash > 0 ? RGB(255, 60, 60) : RGB(139, 0, 0));
    HBRUSH hbrTunic = CreateSolidBrush(tunicCol);
    SelectObject(hdc, hbrTunic);
    POINT tunicPts[4] = {
        {drawX - 11, drawY - 2}, {drawX + 11, drawY - 2},
        {drawX + 13, drawY + 18}, {drawX - 13, drawY + 18}
    };
    Polygon(hdc, tunicPts, 4);
    DeleteObject(hbrTunic);

    // Armor: Lorica Segmentata / Bronze Muscle Cuirass
    if (g->armor == 1) {
        HBRUSH hbrArmor = CreateSolidBrush(RGB(212, 175, 55));
        HPEN hpenArmor = CreatePen(PS_SOLID, 1, RGB(139, 101, 8));
        SelectObject(hdc, hbrArmor);
        SelectObject(hdc, hpenArmor);
        RoundRect(hdc, drawX - 10, drawY - 2, drawX + 10, drawY + 14, 4, 4);
        DeleteObject(hbrArmor);
        DeleteObject(hpenArmor);
    }

    // Head / Face
    HBRUSH hbrSkin = CreateSolidBrush(RGB(210, 154, 104));
    SelectObject(hdc, hbrSkin);
    Ellipse(hdc, drawX - 9, drawY - 20, drawX + 9, drawY - 2);

    // Helmet & Red Crest
    HBRUSH hbrHelm = CreateSolidBrush(RGB(184, 134, 11));
    HPEN hpenHelm = CreatePen(PS_SOLID, 1, RGB(100, 70, 10));
    SelectObject(hdc, hbrHelm);
    SelectObject(hdc, hpenHelm);
    Ellipse(hdc, drawX - 10, drawY - 22, drawX + 10, drawY - 8);

    // Crest
    HBRUSH hbrCrest = CreateSolidBrush(isEnemy ? RGB(75, 20, 75) : RGB(204, 0, 0));
    SelectObject(hdc, hbrCrest);
    Ellipse(hdc, drawX - 9, drawY - 30, drawX + 9, drawY - 18);
    DeleteObject(hbrCrest);

    // Defense Shield Aura
    if (stance == 2) {
        HPEN hpenAura = CreatePen(PS_SOLID, 2, RGB(255, 215, 0));
        SelectObject(hdc, (HBRUSH)GetStockObject(NULL_BRUSH));
        SelectObject(hdc, hpenAura);
        Ellipse(hdc, drawX - 24, drawY - 18, drawX + 24, drawY + 30);
        DeleteObject(hpenAura);
    }

    // Shield (Scutum)
    if (g->shield == 1) {
        int shieldX = isEnemy ? (stance == 2 ? drawX - 6 : drawX - 14) : (stance == 2 ? drawX + 6 : drawX + 14);
        HBRUSH hbrShield = CreateSolidBrush(isEnemy ? RGB(30, 65, 105) : RGB(160, 0, 0));
        HPEN hpenShield = CreatePen(PS_SOLID, 2, RGB(212, 175, 55));
        SelectObject(hdc, hbrShield);
        SelectObject(hdc, hpenShield);
        RoundRect(hdc, shieldX - 7, drawY - 6, shieldX + 7, drawY + 22, 4, 4);
        DeleteObject(hbrShield);
        DeleteObject(hpenShield);
    }

    // Weapon (Gladius or Trident)
    int weaponX = isEnemy ? drawX + 12 : drawX - 12;
    int weaponY = drawY + 2;
    if (stance == 1) { // Attack
        weaponX = isEnemy ? drawX - 16 : drawX + 16;
    }

    if (g->weapon == 1) { // Gladius
        HPEN hpenBlade = CreatePen(PS_SOLID, 3, RGB(240, 240, 250));
        SelectObject(hdc, hpenBlade);
        if (stance == 1) {
            MoveToEx(hdc, weaponX, weaponY, NULL);
            LineTo(hdc, isEnemy ? weaponX - 20 : weaponX + 20, weaponY - 12);
        } else {
            MoveToEx(hdc, weaponX, weaponY, NULL);
            LineTo(hdc, weaponX, weaponY - 22);
        }
        DeleteObject(hpenBlade);
    } else if (g->weapon == 2) { // Trident
        HPEN hpenShaft = CreatePen(PS_SOLID, 2, RGB(139, 90, 43));
        SelectObject(hdc, hpenShaft);
        MoveToEx(hdc, weaponX, weaponY + 8, NULL);
        LineTo(hdc, weaponX, weaponY - 26);
        DeleteObject(hpenShaft);

        HPEN hpenProng = CreatePen(PS_SOLID, 2, RGB(205, 127, 50));
        SelectObject(hdc, hpenProng);
        MoveToEx(hdc, weaponX - 5, weaponY - 24, NULL); LineTo(hdc, weaponX + 5, weaponY - 24);
        MoveToEx(hdc, weaponX - 5, weaponY - 24, NULL); LineTo(hdc, weaponX - 5, weaponY - 32);
        MoveToEx(hdc, weaponX, weaponY - 24, NULL);     LineTo(hdc, weaponX, weaponY - 36);
        MoveToEx(hdc, weaponX + 5, weaponY - 24, NULL); LineTo(hdc, weaponX + 5, weaponY - 32);
        DeleteObject(hpenProng);
    }

    // Restore
    SelectObject(hdc, oldB);
    SelectObject(hdc, oldP);
    DeleteObject(hbrShadow);
    DeleteObject(hbrSkin);
    DeleteObject(hbrHelm);
    DeleteObject(hpenHelm);
}

void DrawLionGDI(HDC hdc, int x, int y, int lunge, int flash) {
    int bob = (FastSin(g_animTick * 3) * 2) / 1000;
    int drawX = x - lunge;
    int drawY = y + bob;

    // Shadow
    HBRUSH hbrShadow = CreateSolidBrush(RGB(140, 110, 70));
    HPEN hpenNull = (HPEN)GetStockObject(NULL_PEN);
    HGDIOBJ oldB = SelectObject(hdc, hbrShadow);
    HGDIOBJ oldP = SelectObject(hdc, hpenNull);
    Ellipse(hdc, x - 25, y + 26, x + 25, y + 36);

    // Lion Body
    COLORREF bodyCol = flash > 0 ? RGB(255, 80, 80) : RGB(217, 155, 89);
    HBRUSH hbrBody = CreateSolidBrush(bodyCol);
    SelectObject(hdc, hbrBody);
    Ellipse(hdc, drawX - 16, drawY - 4, drawX + 32, drawY + 24);

    // Legs
    HBRUSH hbrPaws = CreateSolidBrush(RGB(198, 138, 76));
    SelectObject(hdc, hbrPaws);
    Rectangle(hdc, drawX - 12, drawY + 12, drawX - 5, drawY + 28);
    Rectangle(hdc, drawX + 2, drawY + 12, drawX + 9, drawY + 28);
    Rectangle(hdc, drawX + 16, drawY + 12, drawX + 23, drawY + 28);

    // Mane (Dark Brown fur)
    HBRUSH hbrMane = CreateSolidBrush(RGB(107, 62, 20));
    SelectObject(hdc, hbrMane);
    Ellipse(hdc, drawX - 32, drawY - 18, drawX + 4, drawY + 18);

    // Head & Snout
    SelectObject(hdc, hbrBody);
    Ellipse(hdc, drawX - 30, drawY - 12, drawX - 10, drawY + 8);
    HBRUSH hbrSnout = CreateSolidBrush(RGB(232, 184, 125));
    SelectObject(hdc, hbrSnout);
    Ellipse(hdc, drawX - 36, drawY - 4, drawX - 24, drawY + 6);

    // Glowing Eye
    HBRUSH hbrEye = CreateSolidBrush(RGB(255, 204, 0));
    SelectObject(hdc, hbrEye);
    Ellipse(hdc, drawX - 28, drawY - 8, drawX - 22, drawY - 2);

    SelectObject(hdc, oldB);
    SelectObject(hdc, oldP);
    DeleteObject(hbrShadow);
    DeleteObject(hbrBody);
    DeleteObject(hbrPaws);
    DeleteObject(hbrMane);
    DeleteObject(hbrSnout);
    DeleteObject(hbrEye);
}

void DrawChariotGDI(HDC hdc, int x, int y, int lunge, int flash) {
    int drawX = x - lunge;
    int drawY = y;

    // Shadow
    HBRUSH hbrShadow = CreateSolidBrush(RGB(140, 110, 70));
    HPEN hpenNull = (HPEN)GetStockObject(NULL_PEN);
    HGDIOBJ oldB = SelectObject(hdc, hbrShadow);
    HGDIOBJ oldP = SelectObject(hdc, hpenNull);
    Ellipse(hdc, x - 35, y + 26, x + 35, y + 36);

    // War Horse in front
    HBRUSH hbrHorse = CreateSolidBrush(RGB(92, 58, 33));
    SelectObject(hdc, hbrHorse);
    Ellipse(hdc, drawX - 44, drawY - 4, drawX - 12, drawY + 22);
    // Horse Head
    Ellipse(hdc, drawX - 52, drawY - 18, drawX - 36, drawY - 2);

    // Chariot Chassis
    HBRUSH hbrCart = CreateSolidBrush(RGB(139, 90, 43));
    HPEN hpenGold = CreatePen(PS_SOLID, 2, RGB(212, 175, 55));
    SelectObject(hdc, hbrCart);
    SelectObject(hdc, hpenGold);
    RoundRect(hdc, drawX - 12, drawY - 10, drawX + 24, drawY + 18, 4, 4);

    // Driver
    HBRUSH hbrSkin = CreateSolidBrush(RGB(210, 154, 104));
    SelectObject(hdc, hbrSkin);
    Ellipse(hdc, drawX + 2, drawY - 22, drawX + 16, drawY - 8);

    // Spear
    HPEN hpenSpear = CreatePen(PS_SOLID, 2, RGB(60, 40, 20));
    SelectObject(hdc, hpenSpear);
    MoveToEx(hdc, drawX + 8, drawY - 14, NULL);
    LineTo(hdc, drawX - 55, drawY - 24);

    // Wheel
    HPEN hpenWheel = CreatePen(PS_SOLID, 3, RGB(205, 127, 50));
    SelectObject(hdc, (HBRUSH)GetStockObject(NULL_BRUSH));
    SelectObject(hdc, hpenWheel);
    Ellipse(hdc, drawX - 2, drawY + 6, drawX + 22, drawY + 30);

    SelectObject(hdc, oldB);
    SelectObject(hdc, oldP);
    DeleteObject(hbrShadow);
    DeleteObject(hbrHorse);
    DeleteObject(hbrCart);
    DeleteObject(hpenGold);
    DeleteObject(hbrSkin);
    DeleteObject(hpenSpear);
    DeleteObject(hpenWheel);
}

void DrawHealthBarGDI(HDC hdc, int x, int y, int hp, int maxHp, const char* name, COLORREF fillCol) {
    SetTextColor(hdc, RGB(255, 215, 0));
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, x - 35, y - 16, name, lstrlenA(name));

    HBRUSH hbrDark = CreateSolidBrush(RGB(50, 10, 10));
    HPEN hpenGold = CreatePen(PS_SOLID, 1, RGB(212, 175, 55));
    HGDIOBJ oldB = SelectObject(hdc, hbrDark);
    HGDIOBJ oldP = SelectObject(hdc, hpenGold);

    Rectangle(hdc, x - 40, y, x + 40, y + 7);

    if (maxHp > 0 && hp > 0) {
        int fillW = (hp * 78) / maxHp;
        if (fillW > 78) fillW = 78;
        HBRUSH hbrFill = CreateSolidBrush(fillCol);
        SelectObject(hdc, hbrFill);
        Rectangle(hdc, x - 39, y + 1, x - 39 + fillW, y + 6);
        DeleteObject(hbrFill);
    }

    SelectObject(hdc, oldB);
    SelectObject(hdc, oldP);
    DeleteObject(hbrDark);
    DeleteObject(hpenGold);
}

void DrawParticlesGDI(HDC hdc) {
    for (int i = 0; i < g_particleCount; i++) {
        Particle* p = &g_particles[i];
        HBRUSH hbr = CreateSolidBrush(p->color);
        HPEN hpenNull = (HPEN)GetStockObject(NULL_PEN);
        HGDIOBJ oldB = SelectObject(hdc, hbr);
        HGDIOBJ oldP = SelectObject(hdc, hpenNull);

        if (p->type == 0 || p->type == 1) { // Spark or Dust
            Ellipse(hdc, p->x - p->size / 2, p->y - p->size / 2, p->x + p->size / 2, p->y + p->size / 2);
        } else if (p->type == 2) { // Shard
            Rectangle(hdc, p->x - p->size / 2, p->y - p->size / 2, p->x + p->size / 2, p->y + p->size / 2);
        } else if (p->type == 3) { // Coin
            Ellipse(hdc, p->x - p->size, p->y - p->size / 2, p->x + p->size, p->y + p->size / 2);
        } else if (p->type == 4) { // Cross
            Rectangle(hdc, p->x - 1, p->y - 4, p->x + 2, p->y + 5);
            Rectangle(hdc, p->x - 4, p->y - 1, p->x + 5, p->y + 2);
        }

        SelectObject(hdc, oldB);
        SelectObject(hdc, oldP);
        DeleteObject(hbr);
    }

    // Float text
    for (int i = 0; i < g_floaterCount; i++) {
        Floater* f = &g_floaters[i];
        SetTextColor(hdc, f->color);
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, f->x - 15, f->y, f->text, lstrlenA(f->text));
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            g_hWndMain = hwnd;
            my_srand(GetTickCount());
            for (int i = 0; i < 3; i++) {
                market[market_count++] = GenerateGladiator(0);
            }

            hTitle = CreateWindowA("STATIC", "KColosseum - Ludus Management", WS_VISIBLE | WS_CHILD | SS_CENTER,
                          10, 10, 560, 30, hwnd, NULL, NULL, NULL);
            SendMessageA(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

            hHelpBtn = CreateWindowA("BUTTON", "Guide", WS_VISIBLE | WS_CHILD,
                          500, 10, 70, 25, hwnd, (HMENU)ID_HELP_BUTTON, NULL, NULL);
            SendMessageA(hHelpBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hFundsLabel = CreateWindowA("STATIC", "Treasury: 1000 Denarii", WS_VISIBLE | WS_CHILD | SS_CENTER,
                          10, 45, 560, 25, hwnd, (HMENU)ID_FUNDS_LABEL, NULL, NULL);
            SendMessageA(hFundsLabel, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

            hL1 = CreateWindowA("STATIC", "Available Recruits", WS_VISIBLE | WS_CHILD,
                          10, 80, 140, 20, hwnd, NULL, NULL, NULL);
            SendMessageA(hL1, WM_SETFONT, (WPARAM)hFont, TRUE);

            hRefreshButton = CreateWindowA("BUTTON", "Refresh (50D)", WS_VISIBLE | WS_CHILD,
                          160, 78, 120, 22, hwnd, (HMENU)ID_REFRESH_BUTTON, NULL, NULL);
            SendMessageA(hRefreshButton, WM_SETFONT, (WPARAM)hFont, TRUE);

            hMarketList = CreateWindowA("LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY,
                          10, 105, 270, 130, hwnd, (HMENU)ID_MARKET_LIST, NULL, NULL);
            SendMessageA(hMarketList, WM_SETFONT, (WPARAM)hFont, TRUE);

            hBuyButton = CreateWindowA("BUTTON", "Buy Selected", WS_VISIBLE | WS_CHILD,
                          10, 240, 270, 30, hwnd, (HMENU)ID_BUY_BUTTON, NULL, NULL);
            SendMessageA(hBuyButton, WM_SETFONT, (WPARAM)hFont, TRUE);

            hL2 = CreateWindowA("STATIC", "Your Gladiators", WS_VISIBLE | WS_CHILD,
                          290, 80, 270, 20, hwnd, NULL, NULL, NULL);
            SendMessageA(hL2, WM_SETFONT, (WPARAM)hFont, TRUE);

            hOwnedList = CreateWindowA("LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY,
                          290, 105, 270, 130, hwnd, (HMENU)ID_OWNED_LIST, NULL, NULL);
            SendMessageA(hOwnedList, WM_SETFONT, (WPARAM)hFont, TRUE);

            hEqGladiusBtn = CreateWindowA("BUTTON", "Glad(50)", WS_VISIBLE | WS_CHILD,
                          290, 240, 65, 30, hwnd, (HMENU)ID_EQ_GLADIUS, NULL, NULL);
            SendMessageA(hEqGladiusBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            hEqTridentBtn = CreateWindowA("BUTTON", "Trid(50)", WS_VISIBLE | WS_CHILD,
                          358, 240, 65, 30, hwnd, (HMENU)ID_EQ_TRIDENT, NULL, NULL);
            SendMessageA(hEqTridentBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            hEqArmorBtn = CreateWindowA("BUTTON", "Armr(50)", WS_VISIBLE | WS_CHILD,
                          426, 240, 65, 30, hwnd, (HMENU)ID_EQ_ARMOR, NULL, NULL);
            SendMessageA(hEqArmorBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            hEqShieldBtn = CreateWindowA("BUTTON", "Shld(30)", WS_VISIBLE | WS_CHILD,
                          494, 240, 65, 30, hwnd, (HMENU)ID_EQ_SHIELD, NULL, NULL);
            SendMessageA(hEqShieldBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hTrainStrBtn = CreateWindowA("BUTTON", "+STR (20D)", WS_VISIBLE | WS_CHILD,
                          290, 275, 85, 30, hwnd, (HMENU)ID_TRAIN_STR, NULL, NULL);
            SendMessageA(hTrainStrBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hTrainAgiBtn = CreateWindowA("BUTTON", "+AGI (20D)", WS_VISIBLE | WS_CHILD,
                          382, 275, 85, 30, hwnd, (HMENU)ID_TRAIN_AGI, NULL, NULL);
            SendMessageA(hTrainAgiBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hTrainVitBtn = CreateWindowA("BUTTON", "+VIT (20D)", WS_VISIBLE | WS_CHILD,
                          475, 275, 85, 30, hwnd, (HMENU)ID_TRAIN_VIT, NULL, NULL);
            SendMessageA(hTrainVitBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hFightBtn = CreateWindowA("BUTTON", "Fight in Arena", WS_VISIBLE | WS_CHILD,
                          290, 310, 180, 25, hwnd, (HMENU)ID_FIGHT_BUTTON, NULL, NULL);
            SendMessageA(hFightBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hHealBtn = CreateWindowA("BUTTON", "Heal", WS_VISIBLE | WS_CHILD,
                          480, 310, 80, 25, hwnd, (HMENU)ID_HEAL_BUTTON, NULL, NULL);
            SendMessageA(hHealBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Arena Combat Controls
            hCombatTitle = CreateWindowA("STATIC", "Arena Combat", WS_CHILD | SS_CENTER,
                          10, 10, 560, 30, hwnd, NULL, NULL, NULL);
            SendMessageA(hCombatTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

            hCombatPlayer = CreateWindowA("STATIC", "", WS_CHILD,
                          15, 172, 265, 40, hwnd, NULL, NULL, NULL);
            SendMessageA(hCombatPlayer, WM_SETFONT, (WPARAM)hFont, TRUE);

            hCombatEnemy = CreateWindowA("STATIC", "", WS_CHILD,
                          300, 172, 265, 40, hwnd, NULL, NULL, NULL);
            SendMessageA(hCombatEnemy, WM_SETFONT, (WPARAM)hFont, TRUE);

            hFavorLabel = CreateWindowA("STATIC", "Crowd Favor: 0%", WS_CHILD | SS_CENTER | WS_BORDER,
                          210, 214, 160, 22, hwnd, (HMENU)ID_FAVOR_LABEL, NULL, NULL);
            SendMessageA(hFavorLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

            hAttackBtn = CreateWindowA("BUTTON", "Attack", WS_CHILD,
                          65, 240, 100, 28, hwnd, (HMENU)ID_ATTACK_BUTTON, NULL, NULL);
            SendMessageA(hAttackBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hDefendBtn = CreateWindowA("BUTTON", "Defend", WS_CHILD,
                          175, 240, 100, 28, hwnd, (HMENU)ID_DEFEND_BUTTON, NULL, NULL);
            SendMessageA(hDefendBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hShowboatBtn = CreateWindowA("BUTTON", "Showboat", WS_CHILD,
                          285, 240, 100, 28, hwnd, (HMENU)ID_SHOWBOAT_BUTTON, NULL, NULL);
            SendMessageA(hShowboatBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hFleeBtn = CreateWindowA("BUTTON", "Flee", WS_CHILD,
                          395, 240, 100, 28, hwnd, (HMENU)ID_FLEE_BUTTON, NULL, NULL);
            SendMessageA(hFleeBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hCombatLog = CreateWindowA("LISTBOX", NULL, WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
                          15, 272, 550, 85, hwnd, (HMENU)ID_COMBAT_LOG, NULL, NULL);
            SendMessageA(hCombatLog, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Help Controls
            hHelpTitle = CreateWindowA("STATIC", "Lanista's Guide", WS_CHILD | SS_CENTER,
                          10, 10, 560, 30, hwnd, NULL, NULL, NULL);
            SendMessageA(hHelpTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

            const char* helpStr = "HOW TO PLAY:\n"
                                  "Buy gladiators from the market, equip them, and train their stats.\n"
                                  "Send them to the Arena to fight and earn Denarii. Dead gladiators are lost forever!\n\n"
                                  "COMBAT TACTICS:\n"
                                  "Attack: Uses STR for damage, AGI for hit chance vs enemy AGI.\n"
                                  "Defend: Skips turn but drastically reduces enemy hit chance & damage.\n"
                                  "Showboat: Skips turn to build Crowd Favor. At 100%, the crowd rewards you!\n"
                                  "Flee: Saves your gladiator, but you drop an Arena Level.\n\n"
                                  "EQUIPMENT:\n"
                                  "Glad: +3 STR (More dmg) | Trid: +3 AGI (Higher hit/dodge)\n"
                                  "Armr: +5 VIT (+50 HP)    | Shld: Increases Defend effectiveness\n";

            hHelpText = CreateWindowA("STATIC", helpStr, WS_CHILD | SS_LEFT,
                          20, 50, 540, 250, hwnd, NULL, NULL, NULL);
            SendMessageA(hHelpText, WM_SETFONT, (WPARAM)hFont, TRUE);

            hHelpBackBtn = CreateWindowA("BUTTON", "Back to Ludus", WS_CHILD,
                          230, 310, 140, 30, hwnd, (HMENU)ID_HELP_BACK_BUTTON, NULL, NULL);
            SendMessageA(hHelpBackBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            UpdateUI();
            return 0;
        }
        case WM_TIMER: {
            if (g_currentView == 1) {
                g_animTick++;
                // Update particles
                for (int i = 0; i < g_particleCount; i++) {
                    Particle* p = &g_particles[i];
                    p->x += p->vx;
                    p->y += p->vy;
                    if (p->type == 2 || p->type == 3) p->vy += 1; // Gravity
                    p->life--;
                    if (p->life <= 0) {
                        g_particles[i] = g_particles[--g_particleCount];
                        i--;
                    }
                }
                // Update floaters
                for (int i = 0; i < g_floaterCount; i++) {
                    Floater* f = &g_floaters[i];
                    f->y -= 1;
                    f->life--;
                    if (f->life <= 0) {
                        g_floaters[i] = g_floaters[--g_floaterCount];
                        i--;
                    }
                }
                // Decays
                if (g_shakeAmt > 0) g_shakeAmt--;
                if (g_playerFlash > 0) g_playerFlash--;
                if (g_enemyFlash > 0) g_enemyFlash--;
                if (g_playerLunge > 0) g_playerLunge = (g_playerLunge * 3) / 4;
                if (g_enemyLunge > 0) g_enemyLunge = (g_enemyLunge * 3) / 4;

                RECT rcArena = {15, 10, 565, 168};
                InvalidateRect(hwnd, &rcArena, FALSE);
            }
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (g_currentView == 1) {
                // Double-buffered rendering of Arena Canvas
                int arenaW = 550;
                int arenaH = 155;
                int arenaX = 15;
                int arenaY = 10;

                HDC memDC = CreateCompatibleDC(hdc);
                HBITMAP memBmp = CreateCompatibleBitmap(hdc, arenaW, arenaH);
                HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

                DrawArenaGDI(memDC, arenaW, arenaH);

                // Combatants
                if (currentFighter) {
                    DrawGladiatorGDI(memDC, 135, 95, currentFighter, 0, g_playerStance, g_playerLunge, g_playerFlash);
                    DrawHealthBarGDI(memDC, 135, 30, playerHp, playerMaxHp, currentFighter->name, RGB(34, 139, 34));
                }
                if (enemyHp >= 0) {
                    if (enemyFighter.isBeast) {
                        DrawLionGDI(memDC, 415, 95, g_enemyLunge, g_enemyFlash);
                    } else if (lstrcmpA(enemyFighter.name, "Armed Chariot") == 0) {
                        DrawChariotGDI(memDC, 415, 95, g_enemyLunge, g_enemyFlash);
                    } else if (enemyFighter.isTwins) {
                        DrawGladiatorGDI(memDC, 435, 88, &enemyFighter, 1, g_enemyStance, g_enemyLunge, g_enemyFlash);
                        DrawGladiatorGDI(memDC, 395, 98, &enemyFighter, 1, g_enemyStance, g_enemyLunge, g_enemyFlash);
                    } else {
                        DrawGladiatorGDI(memDC, 415, 95, &enemyFighter, 1, g_enemyStance, g_enemyLunge, g_enemyFlash);
                    }
                    DrawHealthBarGDI(memDC, 415, 30, enemyHp, enemyMaxHp, enemyFighter.name, RGB(204, 0, 0));
                }

                DrawParticlesGDI(memDC);

                int shakeOffX = 0, shakeOffY = 0;
                if (g_shakeAmt > 0) {
                    shakeOffX = (my_rand() % (g_shakeAmt * 2 + 1)) - g_shakeAmt;
                    shakeOffY = (my_rand() % (g_shakeAmt * 2 + 1)) - g_shakeAmt;
                }

                BitBlt(hdc, arenaX + shakeOffX, arenaY + shakeOffY, arenaW, arenaH, memDC, 0, 0, SRCCOPY);

                SelectObject(memDC, oldBmp);
                DeleteObject(memBmp);
                DeleteDC(memDC);
            }
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BUY_BUTTON) {
                int sel = SendMessageA(hMarketList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    BuyGladiator(sel);
                } else {
                    MessageBoxA(hwnd, "Select a gladiator to buy.", "Info", MB_OK | MB_ICONINFORMATION);
                }
            } else if (LOWORD(wParam) == ID_HELP_BUTTON) {
                SwitchView(2);
            } else if (LOWORD(wParam) == ID_HELP_BACK_BUTTON) {
                SwitchView(0);
            } else if (LOWORD(wParam) == ID_TRAIN_STR || LOWORD(wParam) == ID_TRAIN_AGI || LOWORD(wParam) == ID_TRAIN_VIT) {
                int sel = SendMessageA(hOwnedList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    if (funds >= 20) {
                        funds -= 20;
                        if (LOWORD(wParam) == ID_TRAIN_STR) owned[sel].str++;
                        if (LOWORD(wParam) == ID_TRAIN_AGI) owned[sel].agi++;
                        if (LOWORD(wParam) == ID_TRAIN_VIT) owned[sel].vit++;
                        UpdateGladiatorDesc(&owned[sel]);
                        UpdateUI();
                        SendMessageA(hOwnedList, LB_SETCURSEL, sel, 0);
                    } else {
                        MessageBoxA(hwnd, "Not enough funds to train!", "Error", MB_OK | MB_ICONWARNING);
                    }
                } else {
                    MessageBoxA(hwnd, "Select a gladiator to train.", "Info", MB_OK | MB_ICONINFORMATION);
                }
            } else if (LOWORD(wParam) >= ID_EQ_GLADIUS && LOWORD(wParam) <= ID_EQ_SHIELD) {
                int sel = SendMessageA(hOwnedList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    int cost = (LOWORD(wParam) == ID_EQ_SHIELD) ? 30 : 50;
                    if (funds >= cost) {
                        funds -= cost;
                        if (LOWORD(wParam) == ID_EQ_GLADIUS) owned[sel].weapon = 1;
                        if (LOWORD(wParam) == ID_EQ_TRIDENT) owned[sel].weapon = 2;
                        if (LOWORD(wParam) == ID_EQ_ARMOR) owned[sel].armor = 1;
                        if (LOWORD(wParam) == ID_EQ_SHIELD) owned[sel].shield = 1;
                        UpdateGladiatorDesc(&owned[sel]);
                        UpdateUI();
                        SendMessageA(hOwnedList, LB_SETCURSEL, sel, 0);
                    } else {
                        MessageBoxA(hwnd, "Not enough funds to equip!", "Error", MB_OK | MB_ICONWARNING);
                    }
                } else {
                    MessageBoxA(hwnd, "Select a gladiator to equip.", "Info", MB_OK | MB_ICONINFORMATION);
                }
            } else if (LOWORD(wParam) == ID_REFRESH_BUTTON) {
                if (funds >= 50) {
                    funds -= 50;
                    market_count = 0;
                    for (int i = 0; i < 3; i++) {
                        market[market_count++] = GenerateGladiator(0);
                    }
                    UpdateUI();
                }
            } else if (LOWORD(wParam) == ID_HEAL_BUTTON) {
                int sel = SendMessageA(hOwnedList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    if (owned[sel].damageTaken > 0) {
                        if (funds >= owned[sel].damageTaken) {
                            funds -= owned[sel].damageTaken;
                            owned[sel].damageTaken = 0;
                            UpdateGladiatorDesc(&owned[sel]);
                            UpdateUI();
                            SendMessageA(hOwnedList, LB_SETCURSEL, sel, 0);
                        } else {
                            MessageBoxA(hwnd, "Not enough funds to heal fully!", "Error", MB_OK | MB_ICONWARNING);
                        }
                    } else {
                        MessageBoxA(hwnd, "Gladiator is already at full health.", "Info", MB_OK | MB_ICONINFORMATION);
                    }
                } else {
                    MessageBoxA(hwnd, "Select a gladiator to heal.", "Info", MB_OK | MB_ICONINFORMATION);
                }
            } else if (LOWORD(wParam) == ID_ATTACK_BUTTON) {
                if (combatOver) {
                    for (int i = 0; i < owned_count; i++) {
                        if (currentFighter == NULL || &owned[i] != currentFighter) {
                            if (owned[i].damageTaken > 0) {
                                owned[i].damageTaken -= 2;
                                if (owned[i].damageTaken < 0) owned[i].damageTaken = 0;
                                UpdateGladiatorDesc(&owned[i]);
                            }
                        }
                    }
                    UpdateUI();
                    SwitchView(0);
                } else {
                    CombatAction(0);
                }
            } else if (LOWORD(wParam) == ID_DEFEND_BUTTON) {
                CombatAction(1);
            } else if (LOWORD(wParam) == ID_SHOWBOAT_BUTTON) {
                CombatAction(3);
            } else if (LOWORD(wParam) == ID_FLEE_BUTTON) {
                CombatAction(2);
            } else if (LOWORD(wParam) == ID_FIGHT_BUTTON) {
                int sel = SendMessageA(hOwnedList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    int maxHp = GetEffVit(&owned[sel]) * 10;
                    if (owned[sel].damageTaken >= maxHp) {
                        MessageBoxA(hwnd, "Gladiator is too injured to fight!", "Error", MB_OK | MB_ICONWARNING);
                    } else {
                        SetWindowTextA(hAttackBtn, "Attack");
                        EnableWindow(hDefendBtn, TRUE);
                        EnableWindow(hShowboatBtn, TRUE);
                        EnableWindow(hFleeBtn, TRUE);
                        EnterArena(sel);
                    }
                } else {
                    MessageBoxA(hwnd, "Select a gladiator to fight.", "Info", MB_OK | MB_ICONINFORMATION);
                }
            }
            return 0;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            HWND hCtrl = (HWND)lParam;
            if (hCtrl == hFundsLabel || hCtrl == hCombatTitle || hCtrl == hFavorLabel || hCtrl == hHelpTitle) {
                SetTextColor(hdcStatic, RGB(212, 175, 55));
                SetBkColor(hdcStatic, RGB(139, 0, 0));
                return (LRESULT)hbrCrimson;
            } else {
                SetTextColor(hdcStatic, RGB(139, 0, 0));
                SetBkColor(hdcStatic, RGB(245, 245, 220));
                return (LRESULT)hbrBkgnd;
            }
        }
        case WM_CTLCOLORLISTBOX: {
            HDC hdcList = (HDC)wParam;
            SetTextColor(hdcList, RGB(51, 51, 51));
            SetBkColor(hdcList, RGB(253, 245, 230));
            return (LRESULT)hbrList;
        }
        case WM_DESTROY: {
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandleA(NULL);
    const char CLASS_NAME[] = "KColosseumClass";

    hbrBkgnd = CreateSolidBrush(RGB(245, 245, 220));
    hbrCrimson = CreateSolidBrush(RGB(139, 0, 0));
    hbrList = CreateSolidBrush(RGB(253, 245, 230));
    hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, "Times New Roman");
    hTitleFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, "Times New Roman");

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    wc.hbrBackground = hbrBkgnd;

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "KColosseum", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd) {
        ShowWindow(hwnd, SW_SHOW);
        MSG msg = {0};
        while (GetMessageA(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    ExitProcess(0);
}
