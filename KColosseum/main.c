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
} Gladiator;

const char* firstNames[] = {"Titus", "Flamma", "Spiculus", "Marcus", "Lucius", "Gaius", "Quintus", "Aulus"};
const char* epithets[] = {"the Strong", "the Swift", "the Bear", "the Lion", "the Fierce", "the Giant"};
int nextId = 1;

void UpdateGladiatorDesc(Gladiator* g) {
    char eqStr[64];
    eqStr[0] = '\0';
    if (g->weapon == 1) lstrcatA(eqStr, " [Glad]");
    else if (g->weapon == 2) lstrcatA(eqStr, " [Trid]");
    if (g->armor == 1) lstrcatA(eqStr, " [Armr]");
    if (g->shield == 1) lstrcatA(eqStr, " [Shld]");
    wsprintfA(g->desc, "STR:%d AGI:%d VIT:%d%s", g->str, g->agi, g->vit, eqStr);
}

Gladiator GenerateGladiator(int forArena) {
    Gladiator g;
    g.weapon = 0;
    g.armor = 0;
    g.shield = 0;
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
HWND hEqGladiusBtn, hEqTridentBtn, hEqArmorBtn, hEqShieldBtn;
HWND hCombatTitle, hCombatPlayer, hCombatEnemy, hAttackBtn, hDefendBtn, hFleeBtn, hCombatLog;

Gladiator* currentFighter = NULL;
Gladiator enemyFighter;
int playerHp, playerMaxHp, enemyHp, enemyMaxHp;
int playerDefending = 0;
int enemyDefending = 0;
int combatOver = 0;

HBRUSH hbrBkgnd, hbrCrimson, hbrList;
HFONT hFont, hTitleFont;

void UpdateUI() {
    char buf[256];
    wsprintfA(buf, "Treasury: %d Denarii", funds);
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
    int cmdDash = (view == 0) ? SW_SHOW : SW_HIDE;
    int cmdComb = (view == 1) ? SW_SHOW : SW_HIDE;

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

    ShowWindow(hCombatTitle, cmdComb);
    ShowWindow(hCombatPlayer, cmdComb);
    ShowWindow(hCombatEnemy, cmdComb);
    ShowWindow(hAttackBtn, cmdComb);
    ShowWindow(hDefendBtn, cmdComb);
    ShowWindow(hFleeBtn, cmdComb);
    ShowWindow(hCombatLog, cmdComb);
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
}

void EnterArena(int index) {
    currentFighter = &owned[index];
    playerMaxHp = GetEffVit(currentFighter) * 10;
    playerHp = playerMaxHp;

    enemyFighter = GenerateGladiator(1);
    enemyMaxHp = GetEffVit(&enemyFighter) * 10;
    enemyHp = enemyMaxHp;

    combatOver = 0;
    playerDefending = 0;
    enemyDefending = 0;

    SendMessageA(hCombatLog, LB_RESETCONTENT, 0, 0);
    char buf[128];
    wsprintfA(buf, "Match starts! %s vs %s", currentFighter->name, enemyFighter.name);
    LogCombat(buf);

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
        combatOver = 1;
        SetWindowTextA(hAttackBtn, "Leave");
        EnableWindow(hDefendBtn, FALSE);
        EnableWindow(hFleeBtn, FALSE);
        return;
    }

    playerDefending = (action == 1);
    if (playerDefending) {
        wsprintfA(buf, "%s takes a defensive stance.", currentFighter->name);
        LogCombat(buf);
    }

    if (action == 0) { // attack
        int hitChance = 75 + (GetEffAgi(currentFighter) - GetEffAgi(&enemyFighter)) * 5;
        if (enemyDefending) hitChance -= (enemyFighter.shield == 1 ? 50 : 30);
        
        if ((my_rand() % 100) < hitChance) {
            int dmg = GetEffStr(currentFighter) + (my_rand() % 4);
            if (enemyDefending) dmg -= (enemyFighter.shield == 1 ? 5 : 3);
            if (dmg < 1) dmg = 1;
            enemyHp -= dmg;
            wsprintfA(buf, "%s hits for %d damage!", currentFighter->name, dmg);
            LogCombat(buf);
        } else {
            wsprintfA(buf, "%s misses!", currentFighter->name);
            LogCombat(buf);
        }
    }

    if (enemyHp <= 0) {
        enemyHp = 0;
        UpdateCombatUI();
        wsprintfA(buf, "%s is defeated! You win 100 Denarii!", enemyFighter.name);
        LogCombat(buf);
        funds += 100;
        UpdateUI();
        combatOver = 1;
        SetWindowTextA(hAttackBtn, "Leave");
        EnableWindow(hDefendBtn, FALSE);
        EnableWindow(hFleeBtn, FALSE);
        return;
    }

    enemyDefending = (my_rand() % 100) < 25;
    if (enemyDefending) {
        wsprintfA(buf, "%s takes a defensive stance.", enemyFighter.name);
        LogCombat(buf);
    } else {
        int hitChance = 75 + (GetEffAgi(&enemyFighter) - GetEffAgi(currentFighter)) * 5;
        if (playerDefending) hitChance -= (currentFighter->shield == 1 ? 50 : 30);
        
        if ((my_rand() % 100) < hitChance) {
            int dmg = GetEffStr(&enemyFighter) + (my_rand() % 4);
            if (playerDefending) dmg -= (currentFighter->shield == 1 ? 5 : 3);
            if (dmg < 1) dmg = 1;
            playerHp -= dmg;
            wsprintfA(buf, "%s hits for %d damage!", enemyFighter.name, dmg);
            LogCombat(buf);
        } else {
            wsprintfA(buf, "%s misses!", enemyFighter.name);
            LogCombat(buf);
        }
    }

    if (playerHp <= 0) {
        playerHp = 0;
        UpdateCombatUI();
        wsprintfA(buf, "%s is defeated...", currentFighter->name);
        LogCombat(buf);
        combatOver = 1;
        SetWindowTextA(hAttackBtn, "Leave");
        EnableWindow(hDefendBtn, FALSE);
        EnableWindow(hFleeBtn, FALSE);
        return;
    }

    UpdateCombatUI();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            my_srand(GetTickCount());
            for (int i = 0; i < 3; i++) {
                market[market_count++] = GenerateGladiator(0);
            }

            hTitle = CreateWindowA("STATIC", "KColosseum - Ludus Management", WS_VISIBLE | WS_CHILD | SS_CENTER,
                          10, 10, 560, 30, hwnd, NULL, NULL, NULL);
            SendMessageA(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

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
                          290, 310, 270, 25, hwnd, (HMENU)ID_FIGHT_BUTTON, NULL, NULL);
            SendMessageA(hFightBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Arena Combat Controls (Hidden by default)
            hCombatTitle = CreateWindowA("STATIC", "Arena Combat", WS_CHILD | SS_CENTER,
                          10, 10, 560, 30, hwnd, NULL, NULL, NULL);
            SendMessageA(hCombatTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

            hCombatPlayer = CreateWindowA("STATIC", "", WS_CHILD,
                          30, 50, 240, 80, hwnd, NULL, NULL, NULL);
            SendMessageA(hCombatPlayer, WM_SETFONT, (WPARAM)hFont, TRUE);

            hCombatEnemy = CreateWindowA("STATIC", "", WS_CHILD,
                          310, 50, 240, 80, hwnd, NULL, NULL, NULL);
            SendMessageA(hCombatEnemy, WM_SETFONT, (WPARAM)hFont, TRUE);

            hAttackBtn = CreateWindowA("BUTTON", "Attack", WS_CHILD,
                          130, 140, 90, 30, hwnd, (HMENU)ID_ATTACK_BUTTON, NULL, NULL);
            SendMessageA(hAttackBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hDefendBtn = CreateWindowA("BUTTON", "Defend", WS_CHILD,
                          240, 140, 90, 30, hwnd, (HMENU)ID_DEFEND_BUTTON, NULL, NULL);
            SendMessageA(hDefendBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hFleeBtn = CreateWindowA("BUTTON", "Flee", WS_CHILD,
                          350, 140, 90, 30, hwnd, (HMENU)ID_FLEE_BUTTON, NULL, NULL);
            SendMessageA(hFleeBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            hCombatLog = CreateWindowA("LISTBOX", NULL, WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
                          30, 180, 520, 170, hwnd, (HMENU)ID_COMBAT_LOG, NULL, NULL);
            SendMessageA(hCombatLog, WM_SETFONT, (WPARAM)hFont, TRUE);

            UpdateUI();
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
            } else if (LOWORD(wParam) == ID_ATTACK_BUTTON) {
                if (combatOver) {
                    SwitchView(0);
                } else {
                    CombatAction(0);
                }
            } else if (LOWORD(wParam) == ID_DEFEND_BUTTON) {
                CombatAction(1);
            } else if (LOWORD(wParam) == ID_FLEE_BUTTON) {
                CombatAction(2);
            } else if (LOWORD(wParam) == ID_FIGHT_BUTTON) {
                int sel = SendMessageA(hOwnedList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    SetWindowTextA(hAttackBtn, "Attack");
                    EnableWindow(hDefendBtn, TRUE);
                    EnableWindow(hFleeBtn, TRUE);
                    EnterArena(sel);
                } else {
                    MessageBoxA(hwnd, "Select a gladiator to fight.", "Info", MB_OK | MB_ICONINFORMATION);
                }
            }
            return 0;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            HWND hCtrl = (HWND)lParam;
            if (hCtrl == hFundsLabel || hCtrl == hCombatTitle) {
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
