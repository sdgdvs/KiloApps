#include <windows.h>

#define STATE_CHAR_CREATE 0
#define STATE_TOWN        1
#define STATE_SHOP        2
#define STATE_DUNGEON     3
#define STATE_COMBAT      4
#define STATE_GAME_OVER   5

static unsigned int rngSeed = 54321;
static int xrand() {
    rngSeed = rngSeed * 1103515245 + 12345;
    return (int)((rngSeed / 65536) % 32768);
}

static int ContainsSubstr(const char* str, const char* sub) {
    if (!str || !sub) return 0;
    int i, j;
    for (i = 0; str[i] != '\0'; i++) {
        for (j = 0; sub[j] != '\0' && str[i + j] == sub[j]; j++);
        if (sub[j] == '\0') return 1;
    }
    return 0;
}

typedef struct {
    char name[32];
    int hp, maxHp;
    int str, def;
    int xp, gold;
} MonsterDef;

typedef struct {
    char name[32];
    char hazardName[32];
    MonsterDef monsters[5];
    MonsterDef boss;
} BiomeDef;

static const BiomeDef g_Biomes[3] = {
    {
        "Goblin Mines",
        "Cave-In",
        {
            {"Cave Goblin", 30, 30, 8, 3, 30, 15},
            {"Goblin Slinger", 25, 25, 10, 2, 35, 18},
            {"Cave Spider", 38, 38, 11, 4, 45, 22},
            {"Mine Taskmaster", 55, 55, 15, 6, 70, 40},
            {"Rock Golem", 75, 75, 17, 10, 95, 55}
        },
        {"Goblin King (Boss)", 130, 130, 22, 10, 260, 160}
    },
    {
        "Ancient Catacombs",
        "Poison Fog",
        {
            {"Skeleton Archer", 32, 32, 11, 3, 38, 20},
            {"Tomb Ghoul", 45, 45, 13, 5, 52, 28},
            {"Crypt Necromancer", 50, 50, 16, 4, 75, 45},
            {"Dread Wraith", 65, 65, 18, 7, 100, 60},
            {"Bone Colossus", 85, 85, 20, 11, 120, 70}
        },
        {"Lich Lord (Boss)", 160, 160, 26, 12, 350, 220}
    },
    {
        "Dragon Spire",
        "Lava Burst",
        {
            {"Magma Imp", 40, 40, 14, 4, 50, 25},
            {"Fire Drake", 60, 60, 17, 7, 80, 45},
            {"Obsidian Elemental", 80, 80, 20, 12, 110, 65},
            {"Wyvern Sentinel", 95, 95, 23, 10, 140, 85},
            {"Hellhound", 70, 70, 21, 8, 105, 60}
        },
        {"Obsidian Dragon (Boss)", 250, 250, 32, 18, 600, 500}
    }
};

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
    int biome; // 0: Goblin Mines, 1: Ancient Catacombs, 2: Dragon Spire
    char weaponName[32];
    int weaponBonusStr;
    char armorName[32];
    int armorBonusDef;
    int hpPotions;
    int mpPotions;
    int questMonstersKilled;
    int questMonstersDone;
    int questBossKilled;
    int questBossDone;
} Hero;

typedef struct {
    char name[32];
    int hp, maxHp;
    int str, def;
    int xp, gold;
} Enemy;

static Hero player;
static Enemy currentEnemy;
static int gameState = STATE_CHAR_CREATE;
static int selectedClassIndex = 0; // 0: Warrior, 1: Mage, 2: Rogue

HWND hStatusText;
HWND hInfoText;
HWND hLogEdit;
HWND hBtn1, hBtn2, hBtn3, hBtn4, hBtn5, hBtn6;

HBRUSH hBgBrush = NULL;
HBRUSH hPanelBrush = NULL;

void LogMessage(const char* msg) {
    if (!hLogEdit) return;
    int len = GetWindowTextLength(hLogEdit);
    SendMessage(hLogEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)msg);
    SendMessage(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
}

void InitHero(int classIdx) {
    lstrcpyA(player.name, "Valerius");
    player.level = 1;
    player.xp = 0;
    player.nextXp = 100;
    player.gold = 50;
    player.floor = 1;
    player.biome = 0;
    player.hpPotions = 3;
    player.mpPotions = 2;
    player.questMonstersKilled = 0;
    player.questMonstersDone = 0;
    player.questBossKilled = 0;
    player.questBossDone = 0;

    if (classIdx == 0) { // Warrior
        lstrcpyA(player.heroClass, "Warrior");
        player.maxHp = 60; player.hp = 60;
        player.maxMp = 15; player.mp = 15;
        player.str = 15; player.intStat = 6; player.def = 12; player.agi = 8;
        lstrcpyA(player.weaponName, "Iron Shortsword"); player.weaponBonusStr = 4;
        lstrcpyA(player.armorName, "Chainmail Armor"); player.armorBonusDef = 5;
    } else if (classIdx == 1) { // Mage
        lstrcpyA(player.heroClass, "Mage");
        player.maxHp = 40; player.hp = 40;
        player.maxMp = 45; player.mp = 45;
        player.str = 7; player.intStat = 18; player.def = 6; player.agi = 10;
        lstrcpyA(player.weaponName, "Apprentice Staff"); player.weaponBonusStr = 2;
        lstrcpyA(player.armorName, "Silk Robes"); player.armorBonusDef = 2;
    } else { // Rogue
        lstrcpyA(player.heroClass, "Rogue");
        player.maxHp = 45; player.hp = 45;
        player.maxMp = 25; player.mp = 25;
        player.str = 11; player.intStat = 8; player.def = 8; player.agi = 16;
        lstrcpyA(player.weaponName, "Twin Daggers"); player.weaponBonusStr = 5;
        lstrcpyA(player.armorName, "Leather Vest"); player.armorBonusDef = 3;
    }
}

void UpdateUI() {
    char statusBuf[512];
    char infoBuf[512];

    const char* locStr = "Oakhaven Town";
    if (gameState == STATE_DUNGEON || gameState == STATE_COMBAT) {
        locStr = g_Biomes[player.biome].name;
    }

    wsprintfA(statusBuf, "Hero: %s (%s)  |  Lvl: %d  |  HP: %d/%d  |  MP: %d/%d  |  Gold: %d Gold  |  Loc: %s (Floor %d)",
        player.name, player.heroClass, player.level,
        player.hp, player.maxHp, player.mp, player.maxMp,
        player.gold, locStr, player.floor);

    wsprintfA(infoBuf,
        "BIOME: %s (Hazard: %s)\r\n"
        "ATTRIBUTES: STR %d (+%d) | INT %d | DEF %d (+%d) | AGI %d | XP %d/%d\r\n"
        "EQUIPMENT: Weapon: %s | Armor: %s\r\n"
        "INVENTORY: HP Potions x%d | MP Potions x%d  |  QUESTS: Chambers (%d/5) [%s] | Boss (%d/1) [%s]",
        g_Biomes[player.biome].name, g_Biomes[player.biome].hazardName,
        player.str, player.weaponBonusStr, player.intStat, player.def, player.armorBonusDef, player.agi, player.xp, player.nextXp,
        player.weaponName, player.armorName,
        player.hpPotions, player.mpPotions,
        player.questMonstersKilled, player.questMonstersDone ? "DONE" : "ACTIVE",
        player.questBossKilled, player.questBossDone ? "DONE" : "ACTIVE");

    if (hStatusText) SetWindowTextA(hStatusText, statusBuf);
    if (hInfoText) SetWindowTextA(hInfoText, infoBuf);
}

void SetupButtons() {
    ShowWindow(hBtn1, SW_SHOW);
    ShowWindow(hBtn2, SW_SHOW);
    ShowWindow(hBtn3, SW_SHOW);
    ShowWindow(hBtn4, SW_SHOW);
    ShowWindow(hBtn5, SW_SHOW);
    ShowWindow(hBtn6, SW_SHOW);

    switch (gameState) {
        case STATE_CHAR_CREATE:
            SetWindowTextA(hBtn1, "Select Warrior");
            SetWindowTextA(hBtn2, "Select Mage");
            SetWindowTextA(hBtn3, "Select Rogue");
            SetWindowTextA(hBtn4, "Begin Quest");
            SetWindowTextA(hBtn5, "---");
            SetWindowTextA(hBtn6, "---");
            break;

        case STATE_TOWN: {
            char bBtn[64];
            wsprintfA(bBtn, "Biome: %s", g_Biomes[player.biome].name);
            SetWindowTextA(hBtn1, "Enter Dungeon");
            SetWindowTextA(hBtn2, bBtn);
            SetWindowTextA(hBtn3, "Rest at Inn (10G)");
            SetWindowTextA(hBtn4, "Visit Shop");
            SetWindowTextA(hBtn5, "Claim Rewards");
            SetWindowTextA(hBtn6, "Reset Game");
            break;
        }

        case STATE_SHOP:
            SetWindowTextA(hBtn1, "Buy HP Potion (15G)");
            SetWindowTextA(hBtn2, "Buy MP Potion (15G)");
            SetWindowTextA(hBtn3, "Steel Sword (+8 STR, 60G)");
            SetWindowTextA(hBtn4, "Plate Armor (+9 DEF, 75G)");
            SetWindowTextA(hBtn5, "Back to Town");
            SetWindowTextA(hBtn6, "---");
            break;

        case STATE_DUNGEON:
            SetWindowTextA(hBtn1, "Advance Chamber");
            SetWindowTextA(hBtn2, "Descend Staircase");
            SetWindowTextA(hBtn3, "Use HP Potion");
            SetWindowTextA(hBtn4, "Use MP Potion");
            SetWindowTextA(hBtn5, "Return to Town");
            SetWindowTextA(hBtn6, "---");
            break;

        case STATE_COMBAT:
            SetWindowTextA(hBtn1, "Attack");
            if (lstrcmpA(player.heroClass, "Mage") == 0) {
                SetWindowTextA(hBtn2, "Fireball (10 MP)");
            } else if (lstrcmpA(player.heroClass, "Rogue") == 0) {
                SetWindowTextA(hBtn2, "Shadow Strike (8 MP)");
            } else {
                SetWindowTextA(hBtn2, "Shield Bash (5 MP)");
            }
            SetWindowTextA(hBtn3, "Use HP Potion");
            SetWindowTextA(hBtn4, "Use MP Potion");
            SetWindowTextA(hBtn5, "Flee Battle");
            SetWindowTextA(hBtn6, "---");
            break;

        case STATE_GAME_OVER:
            SetWindowTextA(hBtn1, "Restart Journey");
            SetWindowTextA(hBtn2, "---");
            SetWindowTextA(hBtn3, "---");
            SetWindowTextA(hBtn4, "---");
            SetWindowTextA(hBtn5, "---");
            SetWindowTextA(hBtn6, "---");
            break;
    }
}

void CheckLevelUp() {
    if (player.xp >= player.nextXp) {
        player.level++;
        player.xp -= player.nextXp;
        player.nextXp = (int)(player.nextXp * 1.5);
        player.maxHp += 15; player.hp = player.maxHp;
        player.maxMp += 10; player.mp = player.maxMp;
        player.str += 3; player.intStat += 2; player.def += 2; player.agi += 2;
        char msg[128];
        wsprintfA(msg, "🌟 LEVEL UP! Reached Level %d! HP/MP restored and attributes increased!", player.level);
        LogMessage(msg);
    }
}

void StartCombat() {
    gameState = STATE_COMBAT;
    const BiomeDef* b = &g_Biomes[player.biome];

    if (player.floor == 5) {
        lstrcpyA(currentEnemy.name, b->boss.name);
        currentEnemy.maxHp = b->boss.maxHp;
        currentEnemy.str = b->boss.str;
        currentEnemy.def = b->boss.def;
        currentEnemy.xp = b->boss.xp;
        currentEnemy.gold = b->boss.gold;
    } else if (player.floor >= 10) {
        lstrcpyA(currentEnemy.name, "Obsidian Dragon (Final Boss)");
        currentEnemy.maxHp = 250;
        currentEnemy.str = 32;
        currentEnemy.def = 18;
        currentEnemy.xp = 600;
        currentEnemy.gold = 500;
    } else {
        int r = xrand() % 5;
        const MonsterDef* m = &b->monsters[r];
        lstrcpyA(currentEnemy.name, m->name);
        currentEnemy.maxHp = m->hp;
        currentEnemy.str = m->str;
        currentEnemy.def = m->def;
        currentEnemy.xp = m->xp;
        currentEnemy.gold = m->gold;

        // Scale monster stats with floor
        int floorBonus = player.floor - 1;
        currentEnemy.maxHp += floorBonus * 8;
        currentEnemy.str += floorBonus * 2;
        currentEnemy.def += floorBonus * 1;
        currentEnemy.xp += floorBonus * 10;
        currentEnemy.gold += floorBonus * 8;
    }
    currentEnemy.hp = currentEnemy.maxHp;

    char msg[128];
    wsprintfA(msg, "⚠️ Encounter in %s! A hostile %s (HP: %d, STR: %d, DEF: %d) attacks on Floor %d!",
        b->name, currentEnemy.name, currentEnemy.hp, currentEnemy.str, currentEnemy.def, player.floor);
    LogMessage(msg);
    SetupButtons();
    UpdateUI();
}

void EnemyTurn() {
    if (currentEnemy.hp <= 0) return;

    int totalDef = player.def + player.armorBonusDef;
    int dmg = currentEnemy.str - (totalDef / 2);
    if (dmg < 2) dmg = 2;

    player.hp -= dmg;
    char msg[128];
    wsprintfA(msg, "💥 %s attacks you for %d damage!", currentEnemy.name, dmg);
    LogMessage(msg);

    // Environmental Hazard Ambient Tick during Combat (20% chance)
    if ((xrand() % 100) < 20 && player.hp > 0) {
        const BiomeDef* b = &g_Biomes[player.biome];
        int hDmg = 3 + (player.floor * 3 / 2);
        player.hp -= hDmg;
        if (player.hp < 0) player.hp = 0;
        char hmsg[128];
        wsprintfA(hmsg, "⚠️ %s Ambient Hazard! %s environment deals %d damage!", b->hazardName, b->name, hDmg);
        LogMessage(hmsg);
    }

    if (player.hp <= 0) {
        player.hp = 0;
        LogMessage("💀 YOU HAVE FALLEN IN COMBAT! Your soul vanishes into darkness.");
        gameState = STATE_GAME_OVER;
        SetupButtons();
    }
    UpdateUI();
}

void CombatVictory() {
    char msg[128];
    wsprintfA(msg, "🎉 VICTORY! Defeated %s! Gained +%d XP and +%d Gold!", currentEnemy.name, currentEnemy.xp, currentEnemy.gold);
    LogMessage(msg);

    player.xp += currentEnemy.xp;
    player.gold += currentEnemy.gold;

    if (player.questMonstersKilled < 5) player.questMonstersKilled++;
    if (ContainsSubstr(currentEnemy.name, "Boss") || ContainsSubstr(currentEnemy.name, "King") || ContainsSubstr(currentEnemy.name, "Lord") || ContainsSubstr(currentEnemy.name, "Dragon")) {
        player.questBossKilled = 1;
    }

    CheckLevelUp();
    gameState = STATE_DUNGEON;
    SetupButtons();
    UpdateUI();
}

void HandleButton1() {
    if (gameState == STATE_CHAR_CREATE) {
        selectedClassIndex = 0;
        InitHero(0);
        LogMessage("Selected Class: Warrior (High HP & Defense).");
        UpdateUI();
    } else if (gameState == STATE_TOWN) {
        gameState = STATE_DUNGEON;
        char msg[128];
        wsprintfA(msg, "You venture into %s...", g_Biomes[player.biome].name);
        LogMessage(msg);
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_SHOP) {
        if (player.gold >= 15) {
            player.gold -= 15;
            player.hpPotions++;
            LogMessage("Bought 1x Health Potion (+35 HP) for 15 Gold.");
            UpdateUI();
        } else {
            LogMessage("Not enough gold!");
        }
    } else if (gameState == STATE_DUNGEON) {
        int r = xrand() % 100;
        if (r < 50) {
            StartCombat();
        } else if (r < 70) {
            int g = 15 + (xrand() % 25) + (player.floor * 10);
            player.gold += g;
            char msg[128];
            wsprintfA(msg, "✨ Found a treasure chest in %s with %d Gold!", g_Biomes[player.biome].name, g);
            LogMessage(msg);
            UpdateUI();
        } else if (r < 85) {
            const BiomeDef* b = &g_Biomes[player.biome];
            int hDmg = 6 + player.floor * 2 + (xrand() % 5);
            player.hp -= hDmg;
            char msg[128];
            if (player.biome == 0) {
                wsprintfA(msg, "🪨 CAVE-IN HAZARD! Loose rocks crash down! Took %d physical damage!", hDmg);
            } else if (player.biome == 1) {
                wsprintfA(msg, "☠️ POISON FOG HAZARD! Toxic miasma fills the vault! Took %d poison damage!", hDmg);
            } else {
                wsprintfA(msg, "🔥 LAVA BURST HAZARD! Erupting magma geysers shoot up! Took %d fire damage!", hDmg);
            }
            LogMessage(msg);
            if (player.hp <= 0) {
                player.hp = 0;
                LogMessage("💀 Slain by environmental hazard!");
                gameState = STATE_GAME_OVER;
                SetupButtons();
            }
            UpdateUI();
        } else {
            if ((xrand() % 2) == 0) {
                player.hp = player.maxHp;
                LogMessage("⛩️ Prayed at a glowing red altar. Health fully restored!");
            } else {
                player.mp = player.maxMp;
                LogMessage("⛩️ Prayed at a glowing blue altar. Mana fully restored!");
            }
            UpdateUI();
        }
    } else if (gameState == STATE_COMBAT) {
        int totalStr = player.str + player.weaponBonusStr;
        int dmg = (int)(totalStr * 1.2) - (currentEnemy.def / 2);
        if (dmg < 2) dmg = 2;

        BOOL isCrit = ((xrand() % 100) < (player.agi * 2));
        if (isCrit) {
            dmg = (int)(dmg * 1.75);
            char msg[128];
            wsprintfA(msg, "🎯 CRITICAL HIT! You strike %s for %d damage!", currentEnemy.name, dmg);
            LogMessage(msg);
        } else {
            char msg[128];
            wsprintfA(msg, "⚔️ You attack %s dealing %d damage.", currentEnemy.name, dmg);
            LogMessage(msg);
        }

        currentEnemy.hp -= dmg;
        if (currentEnemy.hp <= 0) {
            currentEnemy.hp = 0;
            CombatVictory();
        } else {
            EnemyTurn();
        }
    } else if (gameState == STATE_GAME_OVER) {
        InitHero(selectedClassIndex);
        gameState = STATE_CHAR_CREATE;
        LogMessage("--- New Journey Initialized ---");
        SetupButtons();
        UpdateUI();
    }
}

void HandleButton2() {
    if (gameState == STATE_CHAR_CREATE) {
        selectedClassIndex = 1;
        InitHero(1);
        LogMessage("Selected Class: Mage (High Mana & Spell Power).");
        UpdateUI();
    } else if (gameState == STATE_TOWN) {
        player.biome = (player.biome + 1) % 3;
        player.floor = 1;
        char msg[128];
        wsprintfA(msg, "🗺️ Selected Dungeon Biome: %s (Hazard: %s)", g_Biomes[player.biome].name, g_Biomes[player.biome].hazardName);
        LogMessage(msg);
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_SHOP) {
        if (player.gold >= 15) {
            player.gold -= 15;
            player.mpPotions++;
            LogMessage("Bought 1x Mana Potion (+25 MP) for 15 Gold.");
            UpdateUI();
        } else {
            LogMessage("Not enough gold!");
        }
    } else if (gameState == STATE_DUNGEON) {
        player.floor++;
        char msg[128];
        wsprintfA(msg, "🪜 Descended to Floor %d of %s.", player.floor, g_Biomes[player.biome].name);
        LogMessage(msg);
        UpdateUI();
    } else if (gameState == STATE_COMBAT) {
        int cost = 5;
        if (lstrcmpA(player.heroClass, "Mage") == 0) cost = 10;
        else if (lstrcmpA(player.heroClass, "Rogue") == 0) cost = 8;

        if (player.mp >= cost) {
            player.mp -= cost;
            int dmg = 0;
            if (lstrcmpA(player.heroClass, "Mage") == 0) {
                dmg = (int)(player.intStat * 2.2);
                char msg[128];
                wsprintfA(msg, "🔥 Cast Fireball! Dealt %d magic damage to %s!", dmg, currentEnemy.name);
                LogMessage(msg);
            } else if (lstrcmpA(player.heroClass, "Rogue") == 0) {
                dmg = (int)(player.agi * 1.8);
                char msg[128];
                wsprintfA(msg, "🗡️ Shadow Strike! Dealt %d critical damage to %s!", dmg, currentEnemy.name);
                LogMessage(msg);
            } else {
                int totalStr = player.str + player.weaponBonusStr;
                dmg = (int)(totalStr * 1.5);
                char msg[128];
                wsprintfA(msg, "🛡️ Shield Bash! Dealt %d physical damage to %s!", dmg, currentEnemy.name);
                LogMessage(msg);
            }

            currentEnemy.hp -= dmg;
            if (currentEnemy.hp <= 0) {
                currentEnemy.hp = 0;
                CombatVictory();
            } else {
                EnemyTurn();
            }
        } else {
            LogMessage("Not enough MP!");
        }
    }
}

void HandleButton3() {
    if (gameState == STATE_CHAR_CREATE) {
        selectedClassIndex = 2;
        InitHero(2);
        LogMessage("Selected Class: Rogue (High Agility & Crits).");
        UpdateUI();
    } else if (gameState == STATE_TOWN) {
        if (player.gold >= 10) {
            player.gold -= 10;
            player.hp = player.maxHp;
            player.mp = player.maxMp;
            LogMessage("🍺 Rested at the Dragon's Rest Inn. HP and MP fully restored!");
            UpdateUI();
        } else {
            LogMessage("Not enough gold for the Inn!");
        }
    } else if (gameState == STATE_SHOP) {
        if (player.gold >= 60) {
            player.gold -= 60;
            lstrcpyA(player.weaponName, "Steel Longsword");
            player.weaponBonusStr = 8;
            LogMessage("Equipped Steel Longsword (+8 STR)!");
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
    if (gameState == STATE_CHAR_CREATE) {
        LogMessage("✨ Character created! Welcome to Oakhaven Town.");
        gameState = STATE_TOWN;
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_TOWN) {
        gameState = STATE_SHOP;
        LogMessage("Entered Oakhaven Shop.");
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_SHOP) {
        if (player.gold >= 75) {
            player.gold -= 75;
            lstrcpyA(player.armorName, "Plate Armor");
            player.armorBonusDef = 9;
            LogMessage("Equipped Plate Armor (+9 DEF)!");
            UpdateUI();
        } else {
            LogMessage("Not enough gold!");
        }
    } else if (gameState == STATE_DUNGEON || gameState == STATE_COMBAT) {
        if (player.mpPotions > 0) {
            player.mpPotions--;
            player.mp = (player.mp + 25 > player.maxMp) ? player.maxMp : player.mp + 25;
            LogMessage("🧪 Drank Mana Potion (+25 MP)!");
            UpdateUI();
            if (gameState == STATE_COMBAT) EnemyTurn();
        } else {
            LogMessage("No Mana Potions remaining!");
        }
    }
}

void HandleButton5() {
    if (gameState == STATE_TOWN) {
        int claimed = 0;
        if (player.questMonstersKilled >= 5 && !player.questMonstersDone) {
            player.questMonstersDone = 1;
            player.gold += 50;
            LogMessage("📜 Quest Completed: Clear Dungeon Chambers! +50 Gold!");
            claimed++;
        }
        if (player.questBossKilled >= 1 && !player.questBossDone) {
            player.questBossDone = 1;
            player.gold += 150;
            LogMessage("📜 Quest Completed: Slay Biome Boss! +150 Gold!");
            claimed++;
        }
        if (claimed == 0) {
            LogMessage("No completed quest rewards to claim right now.");
        }
        UpdateUI();
    } else if (gameState == STATE_SHOP) {
        gameState = STATE_TOWN;
        LogMessage("Returned to Town.");
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_DUNGEON) {
        gameState = STATE_TOWN;
        LogMessage("Returned safely to Oakhaven Town.");
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_COMBAT) {
        if ((xrand() % 100) < 60) {
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

void HandleButton6() {
    if (gameState == STATE_TOWN) {
        InitHero(selectedClassIndex);
        gameState = STATE_CHAR_CREATE;
        LogMessage("--- Game Reset to Character Creation ---");
        SetupButtons();
        UpdateUI();
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hBgBrush = CreateSolidBrush(RGB(17, 17, 27));
            hPanelBrush = CreateSolidBrush(RGB(30, 30, 46));

            hStatusText = CreateWindowA("STATIC", "",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                15, 10, 755, 25, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);

            hInfoText = CreateWindowA("STATIC", "",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                15, 40, 755, 65, hwnd, (HMENU)102, GetModuleHandle(NULL), NULL);

            hLogEdit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                15, 115, 755, 300, hwnd, (HMENU)103, GetModuleHandle(NULL), NULL);

            hBtn1 = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 15,  430, 118, 38, hwnd, (HMENU)201, GetModuleHandle(NULL), NULL);
            hBtn2 = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 142, 430, 118, 38, hwnd, (HMENU)202, GetModuleHandle(NULL), NULL);
            hBtn3 = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 269, 430, 118, 38, hwnd, (HMENU)203, GetModuleHandle(NULL), NULL);
            hBtn4 = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 396, 430, 118, 38, hwnd, (HMENU)204, GetModuleHandle(NULL), NULL);
            hBtn5 = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 523, 430, 118, 38, hwnd, (HMENU)205, GetModuleHandle(NULL), NULL);
            hBtn6 = CreateWindowA("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 650, 430, 118, 38, hwnd, (HMENU)206, GetModuleHandle(NULL), NULL);

            InitHero(0);
            SetupButtons();
            UpdateUI();
            LogMessage("=== Welcome to KQuest: Fantasy Dungeon RPG ===");
            LogMessage("Choose your Hero Class above or click 'Begin Quest'!");
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
                case 206: HandleButton6(); break;
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(205, 214, 244));
            SetBkColor(hdcStatic, RGB(30, 30, 46));
            return (INT_PTR)hPanelBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            SetTextColor(hdcEdit, RGB(205, 214, 244));
            SetBkColor(hdcEdit, RGB(24, 24, 37));
            static HBRUSH hEditBg = NULL;
            if (!hEditBg) hEditBg = CreateSolidBrush(RGB(24, 24, 37));
            return (INT_PTR)hEditBg;
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
    wc.lpszClassName = "KQuestClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA("KQuestClass", "KQuest - Fantasy Dungeon RPG", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
                              CW_USEDEFAULT, CW_USEDEFAULT, 800, 520,
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
