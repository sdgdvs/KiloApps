import re
import os

MAIN_C_PATH = r"d:\KiloApps\KQuest\main.c"
HTML_PATH = r"d:\KiloApps\KiloOS\public\apps\kquest.html"
PACKAGE_JSON = r"d:\KiloApps\KiloOS\package.json"
APP_JSX = r"d:\KiloApps\KiloOS\src\App.jsx"

# --- MAIN.C PATCHING ---
with open(MAIN_C_PATH, 'r', encoding='utf-8') as f:
    main_c = f.read()

# 1. States
main_c = main_c.replace(
    "#define STATE_MAGIC_MENU    19",
    "#define STATE_MAGIC_MENU    19\n#define STATE_TOWN_PAGE2    20\n#define STATE_FACTIONS      21\n#define STATE_MOUNTS        22"
)

# 2. Hero struct
main_c = main_c.replace(
    "    Companion companion;\n    int ngLevel;\n} Hero;",
    "    Companion companion;\n    int faction;\n    int mount;\n    int ngLevel;\n} Hero;"
)

# 3. SetupButtons for TOWN
main_c = main_c.replace(
    'SetWindowTextA(hBtn6, "⚙️ System Utils");',
    'SetWindowTextA(hBtn6, "▶️ More Options");\n            break;\n        }\n\n        case STATE_TOWN_PAGE2:\n            SetWindowTextA(hBtn1, "🚩 Factions");\n            SetWindowTextA(hBtn2, "🐎 Mounts");\n            SetWindowTextA(hBtn3, "⚙️ System Utils");\n            SetWindowTextA(hBtn4, "---");\n            SetWindowTextA(hBtn5, "---");\n            SetWindowTextA(hBtn6, "◀️ Back to Town 1");'
)

# SetupButtons for Factions & Mounts
main_c = main_c.replace(
    'case STATE_CRAFTING:',
    '''case STATE_FACTIONS:
            SetWindowTextA(hBtn1, "Join Vanguard (STR)");
            SetWindowTextA(hBtn2, "Join Arcane (INT)");
            SetWindowTextA(hBtn3, "Join Syndicate (AGI)");
            SetWindowTextA(hBtn4, "---");
            SetWindowTextA(hBtn5, "---");
            SetWindowTextA(hBtn6, "Back");
            break;
        case STATE_MOUNTS:
            SetWindowTextA(hBtn1, "Buy Horse(100G)");
            SetWindowTextA(hBtn2, "Buy Wolf(200G)");
            SetWindowTextA(hBtn3, "Buy Dragon(500G)");
            SetWindowTextA(hBtn4, "---");
            SetWindowTextA(hBtn5, "---");
            SetWindowTextA(hBtn6, "Back");
            break;
        case STATE_CRAFTING:'''
)

# SetupButtons for Crafting Overhaul
main_c = main_c.replace(
    'SetWindowTextA(hBtn4, "Upgrade Gear(30G,3S)");',
    'SetWindowTextA(hBtn4, "Craft Masterwork Relic");'
)

# HandleButton6 for STATE_TOWN
main_c = main_c.replace(
    '        gameState = STATE_UTILS;\n        LogMessage("Entered System Utils menu.");',
    '''        gameState = STATE_TOWN_PAGE2;
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_TOWN_PAGE2) {
        gameState = STATE_TOWN;'''
)

# HandleButtonX logic
main_c = main_c.replace(
    'void HandleButton1() {\n    if (gameState == STATE_TITLE)',
    '''void handleFactions(int choice) {
    if (player.faction != 0) {
        LogMessage("You already belong to a faction!");
        return;
    }
    player.faction = choice;
    if (choice == 1) { player.str += 5; LogMessage("Joined Vanguard! +5 STR"); }
    if (choice == 2) { player.intStat += 5; LogMessage("Joined Arcane Order! +5 INT"); }
    if (choice == 3) { player.agi += 5; LogMessage("Joined Syndicate! +5 AGI"); }
    UpdateUI();
}

void handleMounts(int choice, int cost) {
    if (player.mount == choice) {
        LogMessage("You already own this mount!");
        return;
    }
    if (player.gold < cost) {
        LogMessage("Not enough gold!");
        return;
    }
    player.gold -= cost;
    player.mount = choice;
    if (choice == 1) { player.maxHp += 20; player.hp += 20; LogMessage("Bought Horse! +20 HP"); }
    if (choice == 2) { player.maxHp += 10; player.hp += 10; player.agi += 10; LogMessage("Bought Wolf! +10 HP, +10 AGI"); }
    if (choice == 3) { player.maxHp += 30; player.hp += 30; player.str += 10; LogMessage("Bought Dragon! +30 HP, +10 STR"); }
    UpdateUI();
}

void HandleButton1() {
    if (gameState == STATE_TITLE)'''
)

# Adding clicks for Factions/Mounts in HandleButton1,2,3,6
main_c = main_c.replace(
    '} else if (gameState == STATE_UTILS) {',
    '''} else if (gameState == STATE_TOWN_PAGE2) {
        gameState = STATE_FACTIONS;
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_FACTIONS) {
        handleFactions(1);
    } else if (gameState == STATE_MOUNTS) {
        handleMounts(1, 100);
    } else if (gameState == STATE_UTILS) {'''
)
main_c = main_c.replace(
    '} else if (gameState == STATE_CONFIG) {',
    '''} else if (gameState == STATE_TOWN_PAGE2) {
        gameState = STATE_MOUNTS;
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_FACTIONS) {
        handleFactions(2);
    } else if (gameState == STATE_MOUNTS) {
        handleMounts(2, 200);
    } else if (gameState == STATE_CONFIG) {'''
)
main_c = main_c.replace(
    '} else if (gameState == STATE_REPLAYS) {',
    '''} else if (gameState == STATE_TOWN_PAGE2) {
        gameState = STATE_UTILS;
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_FACTIONS) {
        handleFactions(3);
    } else if (gameState == STATE_MOUNTS) {
        handleMounts(3, 500);
    } else if (gameState == STATE_REPLAYS) {'''
)
main_c = main_c.replace(
    '} else if (gameState == STATE_ACHIEVEMENTS) {',
    '''} else if (gameState == STATE_FACTIONS || gameState == STATE_MOUNTS) {
        gameState = STATE_TOWN_PAGE2;
        SetupButtons();
        UpdateUI();
    } else if (gameState == STATE_ACHIEVEMENTS) {'''
)

# Crafting Overhaul in HandleButton4
main_c = main_c.replace(
    '''    } else if (gameState == STATE_CRAFTING) {
        if (player.gold >= 30 && player.ironScrap >= 3) {
            player.gold -= 30;
            player.ironScrap -= 3;
            player.str += 2;
            player.def += 2;
            LogMessage("⚒️ Upgraded Gear! +2 STR, +2 DEF.");
            UpdateUI();
        } else {
            LogMessage("Need 30 Gold and 3 Iron Scrap to upgrade gear.");
        }''',
    '''    } else if (gameState == STATE_CRAFTING) {
        if (player.ironScrap >= 3 && player.arcaneDust >= 3 && player.elementalCore >= 3) {
            player.ironScrap -= 3; player.arcaneDust -= 3; player.elementalCore -= 3;
            player.str += 10; player.def += 10; player.intStat += 10; player.agi += 10;
            LogMessage("🌟 Crafted Masterwork Relic! +10 All Stats.");
            UpdateUI();
        } else {
            LogMessage("Need 3 Iron, 3 Dust, 3 Cores for Masterwork Relic.");
        }'''
)

# UpdateUI
main_c = main_c.replace(
    'if (gameState == STATE_TOWN || gameState == STATE_SHOP || gameState == STATE_CRAFTING) {',
    'if (gameState == STATE_TOWN || gameState == STATE_TOWN_PAGE2 || gameState == STATE_FACTIONS || gameState == STATE_MOUNTS || gameState == STATE_SHOP || gameState == STATE_CRAFTING) {'
)

with open(MAIN_C_PATH, 'w', encoding='utf-8') as f:
    f.write(main_c)

# --- HTML PATCHING ---
with open(HTML_PATH, 'r', encoding='utf-8') as f:
    html = f.read()

# Add states
html = html.replace(
    "TAVERN: 'TAVERN'",
    "TAVERN: 'TAVERN',\n      FACTIONS: 'FACTIONS',\n      MOUNTS: 'MOUNTS'"
)

# Add to player struct
html = html.replace(
    "      achievements: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]\n    };",
    "      achievements: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],\n      faction: 'None',\n      mount: 'None'\n    };"
)

# Add buttons to town
html = html.replace(
    '<button class="btn" style="background:#fab387; color:#11111b;" onclick="setGameState(STATE.ACHIEVEMENTS)">🏆 Achievements</button>',
    '<button class="btn" style="background:#fab387; color:#11111b;" onclick="setGameState(STATE.ACHIEVEMENTS)">🏆 Achievements</button>\n        <button class="btn" style="background:#f38ba8; color:#11111b;" onclick="setGameState(STATE.FACTIONS)">🚩 Factions</button>\n        <button class="btn" style="background:#f9e2af; color:#11111b;" onclick="setGameState(STATE.MOUNTS)">🐎 Mounts</button>'
)

# Support in updateUI (not strictly necessary to change the check but better for rendering)
html = html.replace(
    'else if (currentState === STATE.TOWN || currentState === STATE.SHOP || currentState === STATE.CRAFTING) {',
    'else if (currentState === STATE.TOWN || currentState === STATE.SHOP || currentState === STATE.CRAFTING || currentState === STATE.FACTIONS || currentState === STATE.MOUNTS) {'
)

# Add renderFactions and renderMounts
html = html.replace(
    '// --- PHASE 6: CRAFTING & EQUIPMENT ENCHANTING SYSTEM ---',
    '''// --- FACTIONS & MOUNTS ---
    function joinFaction(f) {
      if(player.faction !== 'None') return logMsg("You already have a faction!", "warning");
      player.faction = f;
      if(f === 'Vanguard') { player.str += 5; logMsg("Joined Vanguard! +5 STR", "system"); }
      if(f === 'Arcane') { player.int += 5; logMsg("Joined Arcane Order! +5 INT", "system"); }
      if(f === 'Syndicate') { player.agi += 5; logMsg("Joined Syndicate! +5 AGI", "system"); }
      updateUI();
      renderFactions();
    }
    
    function buyMount(m, cost) {
      if(player.mount === m) return logMsg("You already own this mount!", "warning");
      if(player.gold < cost) return logMsg("Not enough gold!", "warning");
      player.gold -= cost;
      player.mount = m;
      if(m === 'Horse') { player.maxHp += 20; player.hp += 20; logMsg("Bought Horse! +20 HP", "system"); }
      if(m === 'Wolf') { player.maxHp += 10; player.hp += 10; player.agi += 10; logMsg("Bought Wolf! +10 HP/AGI", "system"); }
      if(m === 'Dragon') { player.maxHp += 30; player.hp += 30; player.str += 10; logMsg("Bought Dragon! +30 HP, +10 STR", "system"); }
      updateUI();
      renderMounts();
    }

    function renderFactions() {
      document.getElementById('location-title').textContent = '🚩 Faction Encampments';
      document.getElementById('location-desc').textContent = 'Pledge allegiance to a faction for permanent bonuses.';
      document.getElementById('dynamic-content').innerHTML = `
        <div class="craft-grid">
          <div class="craft-card" onclick="joinFaction('Vanguard')"><h3>Vanguard</h3><p>+5 STR</p></div>
          <div class="craft-card" onclick="joinFaction('Arcane')"><h3>Arcane Order</h3><p>+5 INT</p></div>
          <div class="craft-card" onclick="joinFaction('Syndicate')"><h3>Syndicate</h3><p>+5 AGI</p></div>
        </div>
        <p style="margin-top:10px; color:#89b4fa;">Current: ${player.faction}</p>
      `;
      document.getElementById('controls-panel').innerHTML = `<button class="btn btn-primary" onclick="setGameState(STATE.TOWN)">⬅️ Back to Town</button>`;
    }

    function renderMounts() {
      document.getElementById('location-title').textContent = '🐎 Town Stables';
      document.getElementById('location-desc').textContent = 'Buy a mount for permanent stat bonuses.';
      document.getElementById('dynamic-content').innerHTML = `
        <div class="craft-grid">
          <div class="craft-card" onclick="buyMount('Horse', 100)"><h3>Horse (100G)</h3><p>+20 HP</p></div>
          <div class="craft-card" onclick="buyMount('Wolf', 200)"><h3>Wolf (200G)</h3><p>+10 HP, +10 AGI</p></div>
          <div class="craft-card" onclick="buyMount('Dragon', 500)"><h3>Dragon (500G)</h3><p>+30 HP, +10 STR</p></div>
        </div>
        <p style="margin-top:10px; color:#89b4fa;">Current: ${player.mount}</p>
      `;
      document.getElementById('controls-panel').innerHTML = `<button class="btn btn-primary" onclick="setGameState(STATE.TOWN)">⬅️ Back to Town</button>`;
    }

    // --- PHASE 6: CRAFTING & EQUIPMENT ENCHANTING SYSTEM ---'''
)

# Crafting Overhaul HTML
html = html.replace(
    '<!-- Armor Enchantments -->',
    '''<!-- Masterwork Relics (Overhaul) -->
            <div class="craft-card" style="border-color:#f9e2af;">
              <h3 style="color:#f9e2af;">🌟 Masterwork Relic</h3>
              <p>Ultimate gear. +10 to All Stats.</p>
              <div class="mats-req">Req: 3 Iron, 3 Dust, 3 Cores</div>
              <button class="btn btn-primary" onclick="craftMasterwork()">Craft Relic</button>
            </div>
            <!-- Armor Enchantments -->'''
)

html = html.replace(
    'function salvageLoot() {',
    '''function craftMasterwork() {
      if(player.materials.iron >= 3 && player.materials.dust >= 3 && player.materials.core >= 3) {
        player.materials.iron -= 3; player.materials.dust -= 3; player.materials.core -= 3;
        player.str += 10; player.int += 10; player.def += 10; player.agi += 10;
        logMsg("🌟 Crafted Masterwork Relic! +10 All Stats.", "craft");
        updateUI();
        renderCrafting();
      } else {
        logMsg("Not enough materials for Masterwork Relic!", "warning");
      }
    }
    
    function salvageLoot() {'''
)

# Add to setGameState
html = html.replace(
    'if (currentState === STATE.QUEST_BOARD) renderQuestBoard();',
    'if (currentState === STATE.QUEST_BOARD) renderQuestBoard();\n      if (currentState === STATE.FACTIONS) renderFactions();\n      if (currentState === STATE.MOUNTS) renderMounts();'
)

with open(HTML_PATH, 'w', encoding='utf-8') as f:
    f.write(html)

# --- APP VERSION BUMP ---
import json
with open(PACKAGE_JSON, 'r', encoding='utf-8') as f:
    pkg = json.load(f)
v = pkg['version'].split('.')
v[2] = str(int(v[2]) + 1)
pkg['version'] = '.'.join(v)
with open(PACKAGE_JSON, 'w', encoding='utf-8') as f:
    json.dump(pkg, f, indent=2)

with open(APP_JSX, 'r', encoding='utf-8') as f:
    jsx = f.read()
import re
jsx = re.sub(r"const MICROS_VERSION = 'v[0-9\.]+';", f"const MICROS_VERSION = 'v{pkg['version']}';", jsx)
with open(APP_JSX, 'w', encoding='utf-8') as f:
    f.write(jsx)
