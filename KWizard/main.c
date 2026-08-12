#include <windows.h>

#define BTN_DRAW 101
#define BTN_RESET 102
#define BTN_END_TURN 103
#define CMB_DIFFICULTY 104
#define BTN_CAMPAIGN 105
#define BTN_DECK 106
#define LST_AVAIL 107
#define LST_DECK 108
#define BTN_DECK_CLOSE 109
#define BTN_HELP 110
#define LST_HELP 111
#define BTN_HELP_CLOSE 112
#define IDT_CAMPAIGN_NEXT 1001

typedef struct {
    char name[32];
    int cost;
    char effect[64];
    int damage;
    int heal;
    int burn;
    int freeze;
    int shield;
    int regen;
    int poison;
} CardDef;

CardDef sampleCards[] = {
    {"Fireball", 3, "Deals 4 Fire damage", 4, 0, 0, 0, 0, 0, 0},
    {"Scorch", 1, "Deals 1 Fire damage", 1, 0, 0, 0, 0, 0, 0},
    {"Flame Strike", 5, "Deals 5 AoE Fire damage", 5, 0, 0, 0, 0, 0, 0},
    {"Ember", 1, "Burns for 1 damage", 0, 0, 2, 0, 0, 0, 0},
    {"Pyroblast", 6, "Deals 8 Fire damage", 8, 0, 0, 0, 0, 0, 0},
    {"Wall of Fire", 4, "Creates a fiery barrier", 0, 0, 0, 0, 5, 0, 0},
    {"Meteor", 8, "Deals 10 Fire damage", 10, 0, 0, 0, 0, 0, 0},
    {"Ignite", 2, "Deals 2 Fire dmg over time", 0, 0, 3, 0, 0, 0, 0},
    {"Ice Shard", 2, "Deals 2 Ice damage", 2, 0, 0, 0, 0, 0, 0},
    {"Frostbolt", 3, "Deals 3 Ice dmg, slows", 3, 0, 0, 1, 0, 0, 0},
    {"Blizzard", 6, "Deals 4 AoE Ice damage", 4, 0, 0, 1, 0, 0, 0},
    {"Frost Nova", 4, "Freezes enemies in place", 0, 0, 0, 1, 0, 0, 0},
    {"Ice Lance", 1, "Deals 1 Ice dmg (3 if frozen)", 1, 0, 0, 0, 0, 0, 0},
    {"Glacial Spike", 7, "Deals 9 Ice damage", 9, 0, 0, 0, 0, 0, 0},
    {"Cold Snap", 5, "Resets cooldowns (Ice)", 0, 0, 0, 0, 0, 0, 0},
    {"Arcane Missiles", 1, "Fires 3 arcane bolts", 3, 0, 0, 0, 0, 0, 0},
    {"Arcane Intellect", 3, "Draw 2 cards", 0, 0, 0, 0, 0, 0, 0},
    {"Counterspell", 3, "Interrupts a spell", 0, 0, 0, 0, 0, 0, 0},
    {"Magic Missile", 2, "Deals 2 Arcane damage", 2, 0, 0, 0, 0, 0, 0},
    {"Arcane Blast", 4, "Deals 5 Arcane damage", 5, 0, 0, 0, 0, 0, 0},
    {"Time Warp", 8, "Take an extra turn", 0, 0, 0, 0, 0, 0, 0},
    {"Polymorph", 4, "Turns target into a sheep", 0, 0, 0, 1, 0, 0, 0},
    {"Mana Shield", 2, "Absorbs damage using mana", 0, 0, 0, 0, 4, 0, 0},
    {"Healing Touch", 2, "Heals 3 Life points", 0, 3, 0, 0, 0, 0, 0},
    {"Rejuvenation", 3, "Heals 4 over time", 0, 0, 0, 0, 0, 4, 0},
    {"Regrowth", 4, "Heals 2 + 2 over time", 0, 2, 0, 0, 0, 2, 0},
    {"Swiftmend", 1, "Instantly heals 2", 0, 2, 0, 0, 0, 0, 0},
    {"Tranquility", 8, "Heals 10 to all allies", 0, 10, 0, 0, 0, 0, 0},
    {"Nourish", 3, "Heals 4", 0, 4, 0, 0, 0, 0, 0},
    {"Nature's Grasp", 2, "Roots attackers", 0, 0, 0, 1, 0, 0, 0},
    {"Lifebloom", 2, "Heals 1, blooms for 3", 0, 1, 0, 0, 0, 3, 0},
    {"Flash Heal", 2, "Fast heal for 3", 0, 3, 0, 0, 0, 0, 0},
    {"Greater Heal", 5, "Heals 7", 0, 7, 0, 0, 0, 0, 0},
    {"Renew", 1, "Heals 2 over time", 0, 0, 0, 0, 0, 3, 0},
    {"Poison Bolt", 4, "Deals 2 dmg, poisons for 3", 2, 0, 0, 0, 0, 0, 3},
    {"Venom Strike", 2, "Poisons for 2", 0, 0, 0, 0, 0, 0, 2}
};
#define NUM_SAMPLE_CARDS (sizeof(sampleCards)/sizeof(CardDef))

int playerHand[7];
int opponentHand[7];
int playerCount = 0;
int opponentCount = 0;

int playerDeck[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
int playerDeckCount = 20;

int playerHp = 30;
int opponentHp = 30;
int gameState = 0; // 0 = playing, 1 = player win, 2 = opponent win

int campaignLevel = 0;
typedef struct {
    char name[32];
    int diff;
    int hp;
    int deckSize;
    int deck[36];
} MageDef;

MageDef mages[] = {
    {"Novice Pyromancer", 0, 20, 4, {0, 1, 3, 7}},
    {"Apprentice Cryomancer", 0, 25, 4, {8, 9, 12, 14}},
    {"Arcane Scholar", 1, 30, 5, {15, 16, 18, 19, 22}},
    {"Forest Druid", 1, 35, 6, {23, 24, 25, 28, 29, 30}},
    {"Venomancer", 1, 40, 4, {34, 35, 23, 30}},
    {"Master Pyromancer", 2, 45, 8, {0, 1, 2, 3, 4, 5, 6, 7}},
    {"Master Cryomancer", 2, 50, 7, {8, 9, 10, 11, 12, 13, 14}},
    {"Arcane Archon", 2, 55, 8, {15, 16, 17, 18, 19, 20, 21, 22}},
    {"High Priest", 2, 60, 8, {16, 22, 23, 26, 27, 28, 31, 32}},
    {"Grand Magus", 2, 70, 36, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35}}
};

int playerMana = 1;
int playerMaxMana = 1;
int opponentMana = 1;
int opponentMaxMana = 1;

int playerBurn = 0, playerFreeze = 0, playerShield = 0, playerRegen = 0, playerPoison = 0;
int opponentBurn = 0, opponentFreeze = 0, opponentShield = 0, opponentRegen = 0, opponentPoison = 0;

char arenaMsg[128] = "Spells and effects go here";

void DealDamageToPlayer(int dmg) {
    if (dmg <= 0) return;
    if (playerShield > 0) {
        if (playerShield >= dmg) {
            playerShield -= dmg;
            dmg = 0;
        } else {
            dmg -= playerShield;
            playerShield = 0;
        }
    }
    playerHp -= dmg;
    if (playerHp < 0) playerHp = 0;
}

void DealDamageToOpponent(int dmg) {
    if (dmg <= 0) return;
    if (opponentShield > 0) {
        if (opponentShield >= dmg) {
            opponentShield -= dmg;
            dmg = 0;
        } else {
            dmg -= opponentShield;
            opponentShield = 0;
        }
    }
    opponentHp -= dmg;
    if (opponentHp < 0) opponentHp = 0;
}

const char* GetSoundType(CardDef* cd) {
    if (cd->damage > 0 && strstr(cd->effect, "Fire") != NULL) return "fire";
    if (cd->damage > 0 && strstr(cd->effect, "Ice") != NULL) return "ice";
    if (cd->damage > 0 && strstr(cd->effect, "Arcane") != NULL) return "arcane";
    if (cd->heal > 0 || cd->regen > 0) return "heal";
    if (strcmp(cd->name, "Ice Lance") == 0 || strstr(cd->name, "Frost") != NULL || strstr(cd->name, "Cold") != NULL || strstr(cd->name, "Blizzard") != NULL) return "ice";
    if (strstr(cd->name, "Fire") != NULL || strstr(cd->name, "Flame") != NULL || strstr(cd->name, "Pyro") != NULL || strstr(cd->name, "Ignite") != NULL || strstr(cd->name, "Ember") != NULL || strstr(cd->name, "Scorch") != NULL || strstr(cd->name, "Meteor") != NULL) return "fire";
    return "arcane";
}

void PlaySoundEffect(const char* type) {
    if (strcmp(type, "fire") == 0) {
        Beep(150, 100);
        Beep(100, 100);
        Beep(50, 100);
    } else if (strcmp(type, "ice") == 0) {
        Beep(800, 50);
        Beep(1200, 50);
    } else if (strcmp(type, "arcane") == 0 || strcmp(type, "heal") == 0) {
        Beep(400, 100);
        Beep(600, 100);
        Beep(800, 150);
    } else if (strcmp(type, "damage") == 0) {
        Beep(100, 150);
    } else if (strcmp(type, "win") == 0) {
        Beep(440, 200);
        Beep(554, 200);
        Beep(659, 200);
        Beep(880, 400);
    } else if (strcmp(type, "lose") == 0) {
        Beep(300, 300);
        Beep(200, 300);
        Beep(100, 400);
    }
}

HWND hwndDraw, hwndReset, hwndCombo, hwndDeckBtn, hwndHelpBtn, hwndAvail, hwndDeck, hwndDeckClose, hwndHelp, hwndHelpClose;

unsigned int seed = 0;
int my_rand() {
    seed = seed * 1664525 + 1013904223;
    return (seed >> 16) & 0x7FFF;
}

void DrawCard(int isOpponent) {
    if (isOpponent) {
        if (opponentCount < 7) {
            if (campaignLevel > 0) {
                MageDef m = mages[campaignLevel - 1];
                opponentHand[opponentCount++] = m.deck[my_rand() % m.deckSize];
            } else {
                opponentHand[opponentCount++] = my_rand() % NUM_SAMPLE_CARDS;
            }
        }
    } else {
        if (playerCount < 7) {
            playerHand[playerCount++] = playerDeck[my_rand() % playerDeckCount];
        }
    }
}

void InitGame(int oppHp) {
    playerCount = 0;
    opponentCount = 0;
    playerHp = 30;
    opponentHp = oppHp;
    gameState = 0;
    playerMaxMana = 1;
    playerMana = 1;
    opponentMaxMana = 1;
    opponentMana = 1;
    playerBurn = 0; playerFreeze = 0; playerShield = 0; playerRegen = 0; playerPoison = 0;
    opponentBurn = 0; opponentFreeze = 0; opponentShield = 0; opponentRegen = 0; opponentPoison = 0;
    if (campaignLevel > 0) {
        wsprintf(arenaMsg, "Battle %d: vs %s!", campaignLevel, mages[campaignLevel-1].name);
    } else {
        strcpy(arenaMsg, "Spells and effects go here");
    }
    for (int i = 0; i < 3; i++) {
        DrawCard(0);
        DrawCard(1);
    }
}

void ResetGame() {
    campaignLevel = 0;
    InitGame(30);
}

int EvaluateCard(CardDef* cd) {
    int score = cd->cost * 2;
    if (opponentHp < 15 && (cd->heal > 0 || cd->shield > 0 || cd->regen > 0)) {
        score += 20 + cd->heal * 3 + cd->shield * 2 + cd->regen * 2;
    }
    if (strcmp(cd->name, "Ice Lance") == 0 && playerFreeze > 0) {
        score += 30;
    }
    if (playerHp <= 10 && cd->damage > 0) {
        score += cd->damage * 4;
    }
    if (strcmp(cd->name, "Time Warp") == 0) score += 25;
    if (strcmp(cd->name, "Arcane Intellect") == 0 && opponentCount < 3) score += 15;
    return score;
}

void PlayOpponentTurn() {
    if (gameState != 0) return;
    
    int diff = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
    if (campaignLevel > 0) {
        diff = mages[campaignLevel-1].diff;
    }
    if (diff == 2) {
        for (int x = 0; x < opponentCount - 1; x++) {
            for (int y = x + 1; y < opponentCount; y++) {
                if (EvaluateCard(&sampleCards[opponentHand[x]]) < EvaluateCard(&sampleCards[opponentHand[y]])) {
                    int temp = opponentHand[x];
                    opponentHand[x] = opponentHand[y];
                    opponentHand[y] = temp;
                }
            }
        }
    } else if (diff == 1) {
        for (int x = 0; x < opponentCount - 1; x++) {
            for (int y = x + 1; y < opponentCount; y++) {
                if (sampleCards[opponentHand[x]].cost < sampleCards[opponentHand[y]].cost) {
                    int temp = opponentHand[x];
                    opponentHand[x] = opponentHand[y];
                    opponentHand[y] = temp;
                }
            }
        }
    } else {
        for (int x = 0; x < opponentCount; x++) {
            int y = my_rand() % opponentCount;
            int temp = opponentHand[x];
            opponentHand[x] = opponentHand[y];
            opponentHand[y] = temp;
        }
    }

    int i = 0;
    char playedStr[256] = "Opponent played: ";
    int playedAny = 0;
    while (i < opponentCount) {
        CardDef cd = sampleCards[opponentHand[i]];
        if (opponentMana >= cd.cost) {
            opponentMana -= cd.cost;
            
            int dmg = cd.damage;
            if (strcmp(cd.name, "Ice Lance") == 0 && playerFreeze > 0) dmg = 3;
            
            DealDamageToPlayer(dmg);
            opponentHp += cd.heal;
            
            playerBurn += cd.burn;
            playerFreeze += cd.freeze;
            opponentShield += cd.shield;
            opponentRegen += cd.regen;
            playerPoison += cd.poison;
            
            PlaySoundEffect(GetSoundType(&cd));
            
            if (strcmp(cd.name, "Arcane Intellect") == 0) {
                DrawCard(1); DrawCard(1);
            }

            if (playerHp <= 0) {
                playerHp = 0;
                if (gameState != 2) PlaySoundEffect("lose");
                gameState = 2; // opponent win
            }
            if (opponentHp > 30) opponentHp = 30;

            if (playedAny) {
                strcat(playedStr, ", ");
            }
            strcat(playedStr, cd.name);
            playedAny = 1;
            
            for (int j = i; j < opponentCount - 1; j++) {
                opponentHand[j] = opponentHand[j + 1];
            }
            opponentCount--;
            
            if (gameState != 0) break;
        } else {
            i++;
        }
    }
    if (playedAny) {
        strcpy(arenaMsg, playedStr);
    } else {
        strcpy(arenaMsg, "Opponent ends turn without casting.");
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hwndCombo = CreateWindow("COMBOBOX", "", 
                                     CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE,
                                     130, 10, 100, 100,
                                     hwnd, (HMENU)CMB_DIFFICULTY, NULL, NULL);
            SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)"Easy");
            SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)"Medium");
            SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)"Hard");
            SendMessage(hwndCombo, CB_SETCURSEL, 1, 0); // Default to Medium
            hwndDraw = CreateWindow("BUTTON", "Draw Card",
                                    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                    240, 10, 100, 30,
                                    hwnd, (HMENU)BTN_DRAW, NULL, NULL);
            CreateWindow("BUTTON", "End Turn",
                         WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                         350, 10, 100, 30,
                         hwnd, (HMENU)BTN_END_TURN, NULL, NULL);
            hwndReset = CreateWindow("BUTTON", "Reset Game",
                                     WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                     460, 10, 100, 30,
                                     hwnd, (HMENU)BTN_RESET, NULL, NULL);
            CreateWindow("BUTTON", "Campaign",
                         WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                         570, 10, 100, 30,
                         hwnd, (HMENU)BTN_CAMPAIGN, NULL, NULL);
            hwndDeckBtn = CreateWindow("BUTTON", "Deck",
                                       WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                       680, 10, 50, 30,
                                       hwnd, (HMENU)BTN_DECK, NULL, NULL);
            hwndHelpBtn = CreateWindow("BUTTON", "Help",
                                       WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                       735, 10, 50, 30,
                                       hwnd, (HMENU)BTN_HELP, NULL, NULL);

            hwndAvail = CreateWindow("LISTBOX", "",
                                     WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                                     50, 50, 300, 400,
                                     hwnd, (HMENU)LST_AVAIL, NULL, NULL);
            hwndDeck = CreateWindow("LISTBOX", "",
                                    WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                                    450, 50, 300, 400,
                                    hwnd, (HMENU)LST_DECK, NULL, NULL);
            hwndDeckClose = CreateWindow("BUTTON", "Save & Close",
                                         WS_CHILD | BS_PUSHBUTTON,
                                         350, 470, 100, 30,
                                         hwnd, (HMENU)BTN_DECK_CLOSE, NULL, NULL);
                                         
            hwndHelp = CreateWindow("LISTBOX", "",
                                    WS_CHILD | WS_BORDER | WS_VSCROLL,
                                    50, 50, 700, 400,
                                    hwnd, (HMENU)LST_HELP, NULL, NULL);
            hwndHelpClose = CreateWindow("BUTTON", "Close Grimoire",
                                         WS_CHILD | BS_PUSHBUTTON,
                                         350, 470, 120, 30,
                                         hwnd, (HMENU)BTN_HELP_CLOSE, NULL, NULL);

            for (int i = 0; i < NUM_SAMPLE_CARDS; i++) {
                char buf[64];
                wsprintf(buf, "%s (Mana: %d)", sampleCards[i].name, sampleCards[i].cost);
                SendMessage(hwndAvail, LB_ADDSTRING, 0, (LPARAM)buf);
            }
            seed = GetTickCount();
            ResetGame();
            return 0;

        case WM_COMMAND:
            if (LOWORD(wParam) == BTN_DRAW) {
                if (gameState == 0) {
                    DrawCard(0);
                    DrawCard(1);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == BTN_END_TURN) {
                if (gameState == 0) {
                    if (playerFreeze > 0) playerFreeze--;

                    if (opponentMaxMana < 10) opponentMaxMana++;
                    opponentMana = opponentMaxMana;
                    DrawCard(1);

                    if (opponentBurn > 0) { DealDamageToOpponent(opponentBurn); opponentBurn--; }
                    if (opponentPoison > 0) { DealDamageToOpponent(opponentPoison); opponentPoison--; }
                    if (opponentRegen > 0) { opponentHp += opponentRegen; opponentRegen--; }
                    if (opponentHp > 30) opponentHp = 30;
                    
                    if (opponentHp <= 0) {
                        if (gameState != 1) PlaySoundEffect("win");
                        gameState = 1; // player win by dots
                        if (campaignLevel > 0) {
                            if (campaignLevel < 10) {
                                wsprintf(arenaMsg, "VICTORY! %s defeated! Next battle in 3s...", mages[campaignLevel-1].name);
                                SetTimer(hwnd, IDT_CAMPAIGN_NEXT, 3000, NULL);
                            } else {
                                strcpy(arenaMsg, "CAMPAIGN COMPLETE! You are the Grand Magus!");
                                campaignLevel = 0;
                            }
                        }
                    } else {
                        if (opponentFreeze > 0) {
                            strcpy(arenaMsg, "Opponent is frozen and skips turn!");
                            opponentFreeze--;
                        } else {
                            PlayOpponentTurn();
                        }
                    }

                    if (gameState == 0) {
                        if (playerMaxMana < 10) playerMaxMana++;
                        playerMana = playerMaxMana;
                        DrawCard(0);
                        
                        if (playerBurn > 0) { DealDamageToPlayer(playerBurn); playerBurn--; }
                        if (playerPoison > 0) { DealDamageToPlayer(playerPoison); playerPoison--; }
                        if (playerRegen > 0) { playerHp += playerRegen; playerRegen--; }
                        if (playerHp > 30) playerHp = 30;
                        if (playerHp <= 0) { playerHp = 0; if (gameState != 2) PlaySoundEffect("lose"); gameState = 2; }
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                }

            } else if (LOWORD(wParam) == BTN_RESET) {
                ResetGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == BTN_CAMPAIGN) {
                campaignLevel = 1;
                InitGame(mages[campaignLevel-1].hp);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == BTN_DECK) {
                gameState = 3;
                ShowWindow(hwndHelp, SW_HIDE);
                ShowWindow(hwndHelpClose, SW_HIDE);
                SendMessage(hwndDeck, LB_RESETCONTENT, 0, 0);
                for (int i = 0; i < playerDeckCount; i++) {
                    char buf[64];
                    wsprintf(buf, "%s (Mana: %d)", sampleCards[playerDeck[i]].name, sampleCards[playerDeck[i]].cost);
                    SendMessage(hwndDeck, LB_ADDSTRING, 0, (LPARAM)buf);
                }
                ShowWindow(hwndAvail, SW_SHOW);
                ShowWindow(hwndDeck, SW_SHOW);
                ShowWindow(hwndDeckClose, SW_SHOW);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == BTN_DECK_CLOSE) {
                if (playerDeckCount != 20) {
                    MessageBox(hwnd, "You must have exactly 20 cards in your deck.", "Deck Builder", MB_OK | MB_ICONWARNING);
                } else {
                    gameState = 0;
                    ShowWindow(hwndAvail, SW_HIDE);
                    ShowWindow(hwndDeck, SW_HIDE);
                    ShowWindow(hwndDeckClose, SW_HIDE);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (LOWORD(wParam) == BTN_HELP) {
                gameState = 4;
                ShowWindow(hwndAvail, SW_HIDE);
                ShowWindow(hwndDeck, SW_HIDE);
                ShowWindow(hwndDeckClose, SW_HIDE);
                SendMessage(hwndHelp, LB_RESETCONTENT, 0, 0);
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"=== HOW TO PLAY ===");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"You and your opponent take turns casting spells.");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"You start with 1 Max Mana, gaining 1 per turn (up to 10).");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"Defeat the opponent by reducing their HP to 0.");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"=== STATUS EFFECTS ===");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"Shield: Absorbs incoming damage.");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"Burn/Poison: Take damage at the start of your turn.");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"Frozen: Skip your next turn or cannot cast spells.");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"Regen: Heal HP at the start of your turn.");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"");
                SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)"=== SPELL INDEX ===");
                for (int i = 0; i < NUM_SAMPLE_CARDS; i++) {
                    char buf[128];
                    wsprintf(buf, "%s (Cost: %d) - %s", sampleCards[i].name, sampleCards[i].cost, sampleCards[i].effect);
                    SendMessage(hwndHelp, LB_ADDSTRING, 0, (LPARAM)buf);
                }
                ShowWindow(hwndHelp, SW_SHOW);
                ShowWindow(hwndHelpClose, SW_SHOW);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == BTN_HELP_CLOSE) {
                gameState = 0;
                ShowWindow(hwndHelp, SW_HIDE);
                ShowWindow(hwndHelpClose, SW_HIDE);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == LST_AVAIL && HIWORD(wParam) == LBN_DBLCLK) {
                if (gameState == 3 && playerDeckCount < 20) {
                    int sel = SendMessage(hwndAvail, LB_GETCURSEL, 0, 0);
                    if (sel != LB_ERR) {
                        playerDeck[playerDeckCount++] = sel;
                        char buf[64];
                        wsprintf(buf, "%s (Mana: %d)", sampleCards[sel].name, sampleCards[sel].cost);
                        SendMessage(hwndDeck, LB_ADDSTRING, 0, (LPARAM)buf);
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            } else if (LOWORD(wParam) == LST_DECK && HIWORD(wParam) == LBN_DBLCLK) {
                if (gameState == 3) {
                    int sel = SendMessage(hwndDeck, LB_GETCURSEL, 0, 0);
                    if (sel != LB_ERR) {
                        SendMessage(hwndDeck, LB_DELETESTRING, sel, 0);
                        for (int i = sel; i < playerDeckCount - 1; i++) {
                            playerDeck[i] = playerDeck[i + 1];
                        }
                        playerDeckCount--;
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            }
            return 0;

        case WM_TIMER:
            if (wParam == IDT_CAMPAIGN_NEXT) {
                KillTimer(hwnd, IDT_CAMPAIGN_NEXT);
                if (gameState == 1 && campaignLevel > 0 && campaignLevel < 10) {
                    campaignLevel++;
                    InitGame(mages[campaignLevel-1].hp);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return 0;

        case WM_LBUTTONDOWN: {
            int xPos = (short)LOWORD(lParam);
            int yPos = (short)HIWORD(lParam);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int cw = clientRect.right - clientRect.left;
            int ch = clientRect.bottom - clientRect.top;
            
            int cardW = 100;
            int cardH = 140;
            int gap = 10;
            
            int playerW = playerCount * cardW + (playerCount > 0 ? playerCount - 1 : 0) * gap;
            int playerX = (cw - playerW) / 2;
            int playerY = ch - cardH - 20;

            if (gameState != 0) return 0;
            if (playerFreeze > 0) {
                strcpy(arenaMsg, "You are frozen and cannot cast spells!");
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            }
            
            for (int i = 0; i < playerCount; i++) {
                int cx = playerX + i * (cardW + gap);
                if (xPos >= cx && xPos <= cx + cardW && yPos >= playerY && yPos <= playerY + cardH) {
                    CardDef cd = sampleCards[playerHand[i]];
                    if (playerMana >= cd.cost) {
                        playerMana -= cd.cost;
                        
                        int dmg = cd.damage;
                        if (strcmp(cd.name, "Ice Lance") == 0 && opponentFreeze > 0) dmg = 3;
                        DealDamageToOpponent(dmg);
                        
                        playerHp += cd.heal;
                        opponentBurn += cd.burn;
                        opponentFreeze += cd.freeze;
                        playerShield += cd.shield;
                        playerRegen += cd.regen;
                        opponentPoison += cd.poison;
                        
                        PlaySoundEffect(GetSoundType(&cd));
                        
                        if (strcmp(cd.name, "Arcane Intellect") == 0) {
                            DrawCard(0); DrawCard(0);
                        }

                        if (opponentHp <= 0) {
                            opponentHp = 0;
                            if (gameState != 1) PlaySoundEffect("win");
                            gameState = 1; // player win
                        }
                        if (playerHp > 30) playerHp = 30;

                        if (gameState == 1 && campaignLevel > 0) {
                            if (campaignLevel < 10) {
                                wsprintf(arenaMsg, "VICTORY! %s defeated! Next battle in 3s...", mages[campaignLevel-1].name);
                                SetTimer(hwnd, IDT_CAMPAIGN_NEXT, 3000, NULL);
                            } else {
                                strcpy(arenaMsg, "CAMPAIGN COMPLETE! You are the Grand Magus!");
                                campaignLevel = 0;
                            }
                        } else {
                            wsprintf(arenaMsg, "Cast %s: %s", cd.name, cd.effect);
                        }
                        
                        for (int j = i; j < playerCount - 1; j++) {
                            playerHand[j] = playerHand[j + 1];
                        }
                        playerCount--;
                    } else {
                        wsprintf(arenaMsg, "Not enough mana for %s!", cd.name);
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                }
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HFONT hFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                     DEFAULT_PITCH | FF_ROMAN, "Georgia");
            HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int cw = clientRect.right - clientRect.left;
            int ch = clientRect.bottom - clientRect.top;

            if (gameState == 3 || gameState == 4) {
                HBRUSH bgBrush = CreateSolidBrush(RGB(10, 5, 20));
                FillRect(hdc, &clientRect, bgBrush);
                DeleteObject(bgBrush);
                
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(232, 216, 183));
                
                if (gameState == 3) {
                    RECT lblA = {50, 20, 350, 50};
                    DrawText(hdc, "Available Spells (Double-click to add)", -1, &lblA, DT_CENTER | DT_SINGLELINE);
                    
                    char lblDStr[64];
                    wsprintf(lblDStr, "Your Deck (%d/20) (Double-click to remove)", playerDeckCount);
                    RECT lblD = {450, 20, 750, 50};
                    DrawText(hdc, lblDStr, -1, &lblD, DT_CENTER | DT_SINGLELINE);
                } else if (gameState == 4) {
                    RECT lblH = {0, 20, cw, 50};
                    DrawText(hdc, "Grimoire & How to Play", -1, &lblH, DT_CENTER | DT_SINGLELINE);
                }

                SelectObject(hdc, oldFont);
                DeleteObject(hFont);
                EndPaint(hwnd, &ps);
                return 0;
            }

            HBRUSH bgBrush = CreateSolidBrush(RGB(26, 11, 46));
            FillRect(hdc, &clientRect, bgBrush);
            DeleteObject(bgBrush);

            RECT arenaRect = {20, 220, cw - 20, ch - 220};
            HBRUSH arenaBrush = CreateSolidBrush(RGB(244, 235, 208));
            FillRect(hdc, &arenaRect, arenaBrush);
            DeleteObject(arenaBrush);
            
            HPEN borderPen = CreatePen(PS_SOLID, 3, RGB(184, 153, 71));
            HGDIOBJ oldPen = SelectObject(hdc, borderPen);
            HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HGDIOBJ oldBrush = SelectObject(hdc, nullBrush);
            Rectangle(hdc, arenaRect.left, arenaRect.top, arenaRect.right, arenaRect.bottom);
            SelectObject(hdc, oldBrush);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(92, 64, 51));
            HFONT hArenaFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                     DEFAULT_PITCH | FF_ROMAN, "Georgia");
            SelectObject(hdc, hArenaFont);
            if (gameState == 1) {
                SetTextColor(hdc, RGB(0, 128, 0));
                DrawText(hdc, "VICTORY! You defeated the opponent!", -1, &arenaRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else if (gameState == 2) {
                SetTextColor(hdc, RGB(128, 0, 0));
                DrawText(hdc, "DEFEAT! You have been slain...", -1, &arenaRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else {
                DrawText(hdc, arenaMsg, -1, &arenaRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            SelectObject(hdc, hFont);
            DeleteObject(hArenaFont);

            int cardW = 100;
            int cardH = 140;
            int gap = 10;
            
            HPEN cardBorderPen = CreatePen(PS_SOLID, 2, RGB(139, 115, 85));
            SelectObject(hdc, cardBorderPen);

            int oppW = opponentCount * cardW + (opponentCount > 0 ? opponentCount - 1 : 0) * gap;
            int oppX = (cw - oppW) / 2;
            int oppY = 60;
            HBRUSH oppBrush = CreateSolidBrush(RGB(61, 43, 31));

            for (int i = 0; i < opponentCount; i++) {
                int cx = oppX + i * (cardW + gap);
                SelectObject(hdc, oppBrush);
                Rectangle(hdc, cx, oppY, cx + cardW, oppY + cardH);
                
                SetTextColor(hdc, RGB(139, 115, 85));
                RECT textRect = {cx, oppY, cx + cardW, oppY + cardH};
                DrawText(hdc, "Card", -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            DeleteObject(oppBrush);

            int playerW = playerCount * cardW + (playerCount > 0 ? playerCount - 1 : 0) * gap;
            int playerX = (cw - playerW) / 2;
            int playerY = ch - cardH - 20;
            HBRUSH playerBrush = CreateSolidBrush(RGB(248, 241, 228));

            for (int i = 0; i < playerCount; i++) {
                int cx = playerX + i * (cardW + gap);
                SelectObject(hdc, playerBrush);
                Rectangle(hdc, cx, playerY, cx + cardW, playerY + cardH);
                
                CardDef cd = sampleCards[playerHand[i]];
                
                SetTextColor(hdc, RGB(44, 30, 22));
                RECT nameRect = {cx, playerY + 50, cx + cardW, playerY + 70};
                DrawText(hdc, cd.name, -1, &nameRect, DT_CENTER | DT_SINGLELINE);
                
                SetTextColor(hdc, RGB(0, 85, 128));
                char costStr[32];
                wsprintf(costStr, "Mana: %d", cd.cost);
                RECT costRect = {cx, playerY + 80, cx + cardW, playerY + 100};
                DrawText(hdc, costStr, -1, &costRect, DT_CENTER | DT_SINGLELINE);
            }
            DeleteObject(playerBrush);
            
            SelectObject(hdc, oldPen);
            DeleteObject(borderPen);
            DeleteObject(cardBorderPen);

            SetTextColor(hdc, RGB(232, 216, 183));
            char oppLabel[256];
            char oppName[64];
            if (campaignLevel > 0) strcpy(oppName, mages[campaignLevel-1].name);
            else strcpy(oppName, "Opponent");
            int pos = wsprintf(oppLabel, "%s Hand (HP: %d | Mana: %d/%d)", oppName, opponentHp, opponentMana, opponentMaxMana);
            if (opponentShield > 0) pos += wsprintf(oppLabel + pos, " [Shield %d]", opponentShield);
            if (opponentBurn > 0) pos += wsprintf(oppLabel + pos, " [Burn %d]", opponentBurn);
            if (opponentPoison > 0) pos += wsprintf(oppLabel + pos, " [Poison %d]", opponentPoison);
            if (opponentFreeze > 0) pos += wsprintf(oppLabel + pos, " [Frozen %d]", opponentFreeze);
            if (opponentRegen > 0) pos += wsprintf(oppLabel + pos, " [Regen %d]", opponentRegen);
            RECT lblOpp = {0, oppY - 20, cw, oppY};
            DrawText(hdc, oppLabel, -1, &lblOpp, DT_CENTER | DT_SINGLELINE);
            
            char playerLabel[256];
            pos = wsprintf(playerLabel, "Player Hand (HP: %d | Mana: %d/%d)", playerHp, playerMana, playerMaxMana);
            if (playerShield > 0) pos += wsprintf(playerLabel + pos, " [Shield %d]", playerShield);
            if (playerBurn > 0) pos += wsprintf(playerLabel + pos, " [Burn %d]", playerBurn);
            if (playerPoison > 0) pos += wsprintf(playerLabel + pos, " [Poison %d]", playerPoison);
            if (playerFreeze > 0) pos += wsprintf(playerLabel + pos, " [Frozen %d]", playerFreeze);
            if (playerRegen > 0) pos += wsprintf(playerLabel + pos, " [Regen %d]", playerRegen);
            RECT lblPlayer = {0, playerY - 20, cw, playerY};
            DrawText(hdc, playerLabel, -1, &lblPlayer, DT_CENTER | DT_SINGLELINE);

            SelectObject(hdc, oldFont);
            DeleteObject(hFont);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KWizardClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "KWizard",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
