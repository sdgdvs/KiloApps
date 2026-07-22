#include <windows.h>

#define STATE_CHAR_CREATE 0
#define STATE_TOWN        1
#define STATE_SHOP        2
#define STATE_DUNGEON     3
#define STATE_COMBAT      4
#define STATE_GAME_OVER   5

static unsigned int rngSeed = 12345;
static int xrand() {
    rngSeed = rngSeed * 1103515245 + 12345;
    return (int)((rngSeed / 65536) % 32768);
}

typedef struct {
    char name[32];
    char heroClass[16];
    int level;
    int hp, maxHp;
    int mp, maxMp;
    int str, intStat, def, agi;
    int gold;
    int xp, nextXp;
    int floor;
    int hpPotions;
    int mpPotions;
    int weaponBonus;
    int armorBonus;
} Hero;

typedef struct {
    char name[32];
    int hp, maxHp;
    int str, def;
    int xp, gold;
} Enemy;

static Hero player = { "Valerius", "Warrior", 1, 60, 60, 15, 15, 6, 12, 8, 50, 0, 100, 1, 3, 2, 4, 3 };
static Enemy currentEnemy;
static int gameState = STATE_TOWN;

HWND hLogEdit;
HWND hStatusText;
HWND hBtn1, hBtn2, hBtn3, hBtn4, hBtn5;

void LogMessage(const char* msg) {
    if (!hLogEdit) return;
    int len = GetWindowTextLength(hLogEdit);
    SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)msg);
    SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
}

void UpdateUI() {
    char buf[512];
    wsprintfA(buf, "Hero: %s (%s) | Lvl: %d | HP: %d/%d | MP: %d/%d | STR: %d | DEF: %d | Gold: %d 🪙 | XP: %d/%d | Location: %s",
        player.name, player.heroClass, player.level,
        player.hp, player.maxHp, player.mp, player.maxMp,
        player.str + player.weaponBonus, player.def + player.armorBonus,
        player.gold, player.xp, player.nextXp,
        (gameState == STATE_DUNGEON || gameState == STATE_COMBAT) ? "Dungeon Spire" : "Oakhaven Town");
    
    if (hStatusText) {
        SetWindowText(hStatusText, buf);
    }
}

void SetupButtons() {
    switch (gameState) {
        case STATE_TOWN:
            SetWindowText(hBtn1, "⚔️ Explore Dungeon");
            SetWindowText(hBtn2, "🍺 Rest at Inn (10 Gold)");
            SetWindowText(hBtn3, "🛒 Visit Shop");
            SetWindowText(hBtn4, "📜 View Attributes");
            SetWindowText(hBtn5, "🔄 Reset Game");
            break;
        case STATE_SHOP:
            SetWindowText(hBtn1, "🧪 Buy HP Potion (15 Gold)");
            SetWindowText(hBtn2, "🧪 Buy MP Potion (15 Gold)");
            SetWindowText(hBtn3, "⚔️ Buy Sword (+6 STR, 50G)");
            SetWindowText(hBtn4, "🛡️ Buy Armor (+6 DEF, 50G)");
            SetWindowText(hBtn5, "🏰 Back to Town");
            break;
        case STATE_DUNGEON:
            SetWindowText(hBtn1, "🚶 Advance Chamber");
            SetWindowText(hBtn2, "🪜 Descend Floor");
            SetWindowText(hBtn3, "🧪 Use HP Potion");
            SetWindowText(hBtn4, "🏰 Return to Town");
            SetWindowText(hBtn5, "---");
            break;
        case STATE_COMBAT:
            SetWindowText(hBtn1, "⚔️ Attack");
            SetWindowText(hBtn2, "🔮 Special Ability");
            SetWindowText(hBtn3, "🧪 Use HP Potion");
            SetWindowText(hBtn4, "🏃 Flee");
            SetWindowText(hBtn5, "---");
            break;
        case STATE_GAME_OVER:
            SetWindowText(hBtn1, "🔄 Restart Journey");
            SetWindowText(hBtn2, "---");
            SetWindowText(hBtn3, "---");
            SetWindowText(hBtn4, "---");
            SetWindowText(hBtn5, "---");
            break;
    }
}

void SpawnMonster() {
    gameState = STATE_COMBAT;
    char nameBuf[32];
    if (player.floor == 5) {
        wsprintfA(currentEnemy.name, "Goblin King (Boss)");
        currentEnemy.maxHp = 120;
        currentEnemy.str = 20;
        currentEnemy.def = 10;
        currentEnemy.xp = 200;
        currentEnemy.gold = 120;
    } else {
        int r = xrand() % 4;
        if (r == 0) wsprintfA(nameBuf, "Cave Rat");
        else if (r == 1) wsprintfA(nameBuf, "Goblin Scout");
        else if (r == 2) wsprintfA(nameBuf, "Skeleton Archer");
        else wsprintfA(nameBuf, "Dark Cultist");

        wsprintfA(currentEnemy.name, "%s", nameBuf);
        currentEnemy.maxHp = 30 + player.floor * 10;
        currentEnemy.str = 8 + player.floor * 3;
        currentEnemy.def = 3 + player.floor * 2;
        currentEnemy.xp = 25 + player.floor * 15;
        currentEnemy.gold = 15 + player.floor * 10;
    }
    currentEnemy.hp = currentEnemy.maxHp;

    char msg[128];
    wsprintfA(msg, "⚠️ A wild %s (HP: %d) attacks you on Floor %d!", currentEnemy.name, currentEnemy.hp, player.floor);
    LogMessage(msg);
    SetupButtons();
    UpdateUI();
}

void EnemyTurn() {
    if (currentEnemy.hp <= 0) return;

    int totalDef = player.def + player.armorBonus;
    int dmg = currentEnemy.str - (totalDef / 2);
    if (dmg < 2) dmg = 2;

    player.hp -= dmg;
    char msg[128];
    wsprintfA(msg, "💥 %s hits you for %d damage!", currentEnemy.name, dmg);
    LogMessage(msg);

    if (player.hp <= 0) {
        player.hp = 0;
        LogMessage("💀 YOU HAVE FALLEN IN COMBAT! Game Over.");
        gameState = STATE_GAME_OVER;
        SetupButtons();
    }
    UpdateUI();
}

void CheckLevelUp() {
    if (player.xp >= player.nextXp) {
        player.level++;
        player.xp -= player.nextXp;
        player.nextXp = (int)(player.nextXp * 1.5);
        player.maxHp += 15; player.hp = player.maxHp;
        player.maxMp += 10; player.mp = player.maxMp;
        player.str += 3; player.def += 2;
        char msg[128];
        wsprintfA(msg, "🌟 LEVEL UP! You are now Level %d! HP and MP restored!", player.level);
        LogMessage(msg);
    }
}

void HandleButton1() {
    if (gameState == STATE_TOWN) {
        gameState = STATE_DUNGEON;
        LogMessage("You enter the Obsidian Spire. Shadows loom all around...");
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_SHOP) {
        if (player.gold >= 15) {
            player.gold -= 15;
            player.hpPotions++;
            LogMessage("Bought 1x Health Potion for 15 Gold.");
            UpdateUI();
        } else {
            LogMessage("Not enough gold!");
        }
    } else if (gameState == STATE_DUNGEON) {
        int r = xrand() % 100;
        if (r < 65) {
            SpawnMonster();
        } else if (r < 85) {
            int g = 20 + xrand() % 30;
            player.gold += g;
            char msg[128];
            wsprintfA(msg, "✨ Found a chest containing %d Gold!", g);
            LogMessage(msg);
            UpdateUI();
        } else {
            player.hp = player.maxHp;
            LogMessage("⛩️ Prayed at an ancient shrine. Full HP restored!");
            UpdateUI();
        }
    } else if (gameState == STATE_COMBAT) {
        int totalStr = player.str + player.weaponBonus;
        int dmg = totalStr * 2 - currentEnemy.def;
        if (dmg < 3) dmg = 3;

        currentEnemy.hp -= dmg;
        char msg[128];
        wsprintfA(msg, "⚔️ You attack %s dealing %d damage!", currentEnemy.name, dmg);
        LogMessage(msg);

        if (currentEnemy.hp <= 0) {
            currentEnemy.hp = 0;
            wsprintfA(msg, "🎉 Defeated %s! Gained +%d XP and +%d Gold!", currentEnemy.name, currentEnemy.xp, currentEnemy.gold);
            LogMessage(msg);
            player.xp += currentEnemy.xp;
            player.gold += currentEnemy.gold;
            CheckLevelUp();
            gameState = STATE_DUNGEON;
            SetupButtons();
            UpdateUI();
        } else {
            EnemyTurn();
        }
    } else if (gameState == STATE_GAME_OVER) {
        player.level = 1;
        player.maxHp = 60; player.hp = 60;
        player.maxMp = 15; player.mp = 15;
        player.gold = 50; player.xp = 0; player.floor = 1;
        player.hpPotions = 3; player.mpPotions = 2;
        gameState = STATE_TOWN;
        LogMessage("--- New Journey Begun in Oakhaven ---");
        SetupButtons();
        UpdateUI();
    }
}

void HandleButton2() {
    if (gameState == STATE_TOWN) {
        if (player.gold >= 10) {
            player.gold -= 10;
            player.hp = player.maxHp;
            player.mp = player.maxMp;
            LogMessage("🍺 Rested at the Inn. HP and MP fully restored!");
            UpdateUI();
        } else {
            LogMessage("Not enough gold for the Inn!");
        }
    } else if (gameState == STATE_SHOP) {
        if (player.gold >= 15) {
            player.gold -= 15;
            player.mpPotions++;
            LogMessage("Bought 1x Mana Potion for 15 Gold.");
            UpdateUI();
        } else {
            LogMessage("Not enough gold!");
        }
    } else if (gameState == STATE_DUNGEON) {
        player.floor++;
        char msg[128];
        wsprintfA(msg, "🪜 Descended to Floor %d of the Spire.", player.floor);
        LogMessage(msg);
        UpdateUI();
    } else if (gameState == STATE_COMBAT) {
        if (player.mp >= 5) {
            player.mp -= 5;
            int dmg = (player.str + player.intStat) * 2;
            currentEnemy.hp -= dmg;
            char msg[128];
            wsprintfA(msg, "🔮 Cast Special Power! Dealt %d damage to %s!", dmg, currentEnemy.name);
            LogMessage(msg);
            if (currentEnemy.hp <= 0) {
                currentEnemy.hp = 0;
                wsprintfA(msg, "🎉 Defeated %s!", currentEnemy.name);
                LogMessage(msg);
                player.xp += currentEnemy.xp;
                player.gold += currentEnemy.gold;
                CheckLevelUp();
                gameState = STATE_DUNGEON;
                SetupButtons();
                UpdateUI();
            } else {
                EnemyTurn();
            }
        } else {
            LogMessage("Not enough MP!");
        }
    }
}

void HandleButton3() {
    if (gameState == STATE_TOWN) {
        gameState = STATE_SHOP;
        LogMessage("Entered Oakhaven Shop.");
        SetupButtons();
    } else if (gameState == STATE_SHOP) {
        if (player.gold >= 50) {
            player.gold -= 50;
            player.weaponBonus += 6;
            LogMessage("Equipped Steel Sword (+6 STR)!");
            UpdateUI();
        } else {
            LogMessage("Not enough gold!");
        }
    } else if (gameState == STATE_DUNGEON || gameState == STATE_COMBAT) {
        if (player.hpPotions > 0) {
            player.hpPotions--;
            player.hp = (player.hp + 35 > player.maxHp) ? player.maxHp : player.hp + 35;
            LogMessage("🧪 Drank Health Potion (+35 HP)!");
            UpdateUI();
            if (gameState == STATE_COMBAT) EnemyTurn();
        } else {
            LogMessage("No Health Potions remaining!");
        }
    }
}

void HandleButton4() {
    if (gameState == STATE_TOWN) {
        char msg[256];
        wsprintfA(msg, "📊 Attributes: STR %d, INT %d, DEF %d, AGI %d. Potions: HP x%d, MP x%d",
            player.str, player.intStat, player.def, player.agi, player.hpPotions, player.mpPotions);
        LogMessage(msg);
    } else if (gameState == STATE_SHOP) {
        if (player.gold >= 50) {
            player.gold -= 50;
            player.armorBonus += 6;
            LogMessage("Equipped Reinforced Armor (+6 DEF)!");
            UpdateUI();
        } else {
            LogMessage("Not enough gold!");
        }
    } else if (gameState == STATE_DUNGEON) {
        gameState = STATE_TOWN;
        LogMessage("Returned to Oakhaven Town.");
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_COMBAT) {
        if (xrand() % 100 < 60) {
            LogMessage("🏃 Fled safely from combat!");
            gameState = STATE_DUNGEON;
            SetupButtons();
            UpdateUI();
        } else {
            LogMessage("Failed to flee!");
            EnemyTurn();
        }
    }
}

void HandleButton5() {
    if (gameState == STATE_SHOP) {
        gameState = STATE_TOWN;
        LogMessage("Returned to Town.");
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_TOWN) {
        player.level = 1;
        player.maxHp = 60; player.hp = 60;
        player.gold = 50; player.xp = 0; player.floor = 1;
        gameState = STATE_TOWN;
        LogMessage("--- Game Reset ---");
        UpdateUI();
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hStatusText = CreateWindow("STATIC", "",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                20, 15, 740, 30, hwnd, (HMENU)102, GetModuleHandle(NULL), NULL);

            hLogEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                20, 50, 740, 360, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);

            hBtn1 = CreateWindow("BUTTON", "",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 430, 140, 40, hwnd, (HMENU)201, GetModuleHandle(NULL), NULL);

            hBtn2 = CreateWindow("BUTTON", "",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                170, 430, 140, 40, hwnd, (HMENU)202, GetModuleHandle(NULL), NULL);

            hBtn3 = CreateWindow("BUTTON", "",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                320, 430, 140, 40, hwnd, (HMENU)203, GetModuleHandle(NULL), NULL);

            hBtn4 = CreateWindow("BUTTON", "",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                470, 430, 140, 40, hwnd, (HMENU)204, GetModuleHandle(NULL), NULL);

            hBtn5 = CreateWindow("BUTTON", "",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                620, 430, 140, 40, hwnd, (HMENU)205, GetModuleHandle(NULL), NULL);

            SetupButtons();
            UpdateUI();
            LogMessage("=== Welcome to KQuest: Fantasy Dungeon RPG ===");
            LogMessage("Choose an action below to begin your adventure in Oakhaven Town!");
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case 201: HandleButton1(); break;
                case 202: HandleButton2(); break;
                case 203: HandleButton3(); break;
                case 204: HandleButton4(); break;
                case 205: HandleButton5(); break;
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(220, 220, 240));
            SetBkColor(hdcStatic, RGB(30, 30, 45));
            static HBRUSH hbrBkgnd = NULL;
            if (!hbrBkgnd) hbrBkgnd = CreateSolidBrush(RGB(30, 30, 45));
            return (INT_PTR)hbrBkgnd;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = CreateSolidBrush(RGB(20, 20, 30));
    wc.lpszClassName = "KQuestClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindow("KQuestClass", "KQuest - Fantasy RPG", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 800, 530,
                             NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void MainEntry() {
    int ret = WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT);
    ExitProcess(ret);
}
