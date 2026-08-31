# KiloApps Games Content Plan

This document tracks the **perpetual, never-ending** improvement loop for all KiloApps games.

## DIRECTOR NOTE (2026-07-29): STOP CAMPAIGN SPAM — FOCUS ON DEPTH WHERE IT MATTERS + GAME BALANCE

**⚠️ READ THIS BEFORE EVERY TURN. This supersedes old loop instructions.**

The old approach of "add 20-stage campaigns and active skills to every game" is OVER. A 20-stage campaign with boss encounters makes sense for KRogue and KSpace. It does NOT make sense for Chess, Connect4, Hangman, or Towers of Hanoi. Those are **classic strategy/puzzle games** — they need smart AI and good difficulty curves, not boss fights and powerups.

**From now on, games are split into two categories with DIFFERENT work priorities:**

### 🎮 DEEP GAMES (get campaigns, content, lore, progression)
These are action, adventure, RPG, and exploration games where depth = more content:
- **KRogue** — roguelike RPG (biomes, enemies, spells, items, floors)
- **KQuest** — fantasy RPG (chapters, bosses, spells, story)
- **KStarship** — space exploration (sectors, subsystems, encounters)
- **KAlchemy** — crafting/discovery (recipes, quests, lab upgrades)
- **KSpace** — arcade shooter (waves, enemy variety, boss fights)
- **KAsteroids** — arcade shooter (sectors, hazards, bosses)
- **KMaze** — exploration/survival (mazes, traps, items, fog of war)
- **KPac** — action (maze variety, ghost AI, power-ups)
- **KBreakout** — arcade (brick layouts, powerups, boss stages)
- **KSnake** — action (obstacle mazes, rival AI, boss encounters)

For Deep Games: Add more content, enemy variety, procedural generation, narrative elements, crafting, lore. These games SHOULD have long campaigns and progression systems. Fill the 999KB budget.

### ♟️ CLASSIC GAMES (get balance, usability, AI tuning — NO MORE CAMPAIGNS)
These are traditional board, card, and puzzle games where quality = smart AI and good UX:
- **KChess, KGo, KReversi, KConnect4** — board games
- **KSolitaire, KFreecell** — card games
- **KSudoku, K2048, KTowers, KMines** — puzzle games
- **KHangman, KWords, KSimon, KMatch3** — word/pattern games
- **KTetris, KPong, KDarts** — skill games

For Classic Games: **DO NOT add more campaign stages, boss encounters, or active skill hotkeys.** They already have too many. Instead focus on:
1. **Game Balance** — Is Easy too easy? Is Hard actually hard? Does the AI play well? Does difficulty ramp smoothly?
2. **AI Quality** — For board games (Chess, Go, Reversi, Connect4): make the AI smarter and more varied, not just add more stages.
3. **Usability** — Are controls intuitive? Is the UI clear? Can a new player figure out how to play? Is there a help/tutorial?
4. **Bug Fixes** — Fix broken mechanics, impossible levels, crashes, or stuck states.
5. **Polish** — Score display, win/loss feedback, smooth animations, sound effects.

### 🔧 GAME BALANCE PASS (NEW PRIORITY)
**Before adding ANY new content to a game, play-test it mentally first:**
- Can a player actually win Stage 1 on their first try? If not, it's too hard.
- Is Easy mode trivially easy (AI makes random moves)? If so, fix it.
- Does the difficulty curve feel smooth? Or does it jump from trivial to impossible?
- Are powerups so strong they trivialize the game? Or so weak they're useless?
- For board game AI: does Easy AI feel fair but beatable? Does Hard AI feel challenging but not cheating?
- Is the campaign so long that nobody will finish it? 10 stages is plenty for most games. 20+ is only for Deep Games.

**When you pick a Classic Game from the queue, your job is a BALANCE & USABILITY pass, not more content.**

---

## ⏱️ TURN SCOPING & TERMINATION (CRITICAL — READ EVERY TURN)

**Single-Item-Per-Turn Rule:**
- Each cron trigger = ONE turn. Process exactly ONE item from your queue, then STOP.
- "Loop forever" means the CRON loops forever across turns, NOT that you loop within a single turn.
- After committing and pushing your work for ONE item, STOP CALLING TOOLS immediately.

**Subagent Timeout Rule:**
- If you spawn a subagent, set a timer for 8 minutes using the `schedule` tool with `TimerCondition` set to the subagent's conversation ID.
- If the timer fires (subagent hasn't finished in 8 min), KILL the subagent using `manage_subagents`, log a one-line failure note in your plan file, commit, push, and STOP.
- NEVER spawn more than ONE subagent at a time.
- NEVER spawn a second subagent if the first one failed. Stop and let the next cron turn retry.

**Graceful Termination Checklist (do this EVERY turn before stopping):**
1. Processed one item
2. Updated plan file
3. Committed and pushed
4. All subagents terminated (killed or completed)
5. STOP — call no more tools

---

## Agent Rules & Guidelines

**Perpetual Loop (NEVER STOP)**
This agent loops forever. Pick the top game from the queue, do work appropriate to its category (see above), move it to the bottom, repeat.
- For **Deep Games**: Add meaningful new content — new enemies, mechanics, story, levels, items.
- For **Classic Games**: Do a balance/usability audit — test difficulty, improve AI, fix UX issues, tune parameters.
- **CREATE NEW GAME** mode: Only if the Director or user explicitly requests it. The Creator agent is the primary new-game creator.

**Dual-Target Strategy (CRITICAL)**
Each game exists in two forms: a native Windows executable (`K[Name]/main.c`) and a web HTML5 version (`KiloOS/public/apps/k[name].html`). ALWAYS audit both versions before working on a game.
- **Tier 1 (Full Parity)**: Both web and exe have equivalent gameplay. Improve both in parallel.
- **Tier 2 (Simplified Web Version)**: Web version is simplified but playable. Build the fullest web version possible. The exe version should always be equal or superior.
- **Tier 3 (Exe-Only with Web Stub)**: The game is too complex to port perfectly right now. Focus content improvement on the EXE version. Web is a polished download stub. Over time, incrementally port features to move from Tier 3 → Tier 2 → Tier 1.

**Important Rules**
- ALWAYS read the exe source FIRST. It is the "canonical" version and may be far more advanced than the web version.
- NEVER remove features from the exe to match a simpler web version.
- NEVER replace a working advanced exe with a simpler reimplementation.
- When improving the exe, preserve ALL existing gameplay systems and add to them.
- If you encounter a download-stub web version, it's intentional. Improve the exe, and optionally begin an incremental web port if time allows.

**Content Additions (for Deep Games only — pick 2-3 per turn)**
- New enemies, bosses, items, spells, or story elements
- Procedural generation improvements (more room types, varied layouts)
- Lore, narrative, crafting recipes, progression unlocks
- New biomes, environments, or themes
- Sound effects (Web Audio API for web; Beep()/PlaySound for native)

**Balance & Usability Pass (for Classic Games — do ALL of these)**
- Test each difficulty level: is Easy actually easy? Is Hard actually hard?
- Audit AI quality: does the AI make reasonable moves at each difficulty?
- Check first-play experience: can a new player figure out controls?
- Verify win/loss conditions work correctly
- Ensure score display, high scores, and restart work properly
- Check that powerups (if any) are balanced — not too strong, not useless

**Quality Bar**
- Must have: start screen, score display, game-over screen with restart
- Must have at least 2 difficulty levels or progressive difficulty
- Persist high scores
- Responsive controls
- Web games: use `requestAnimationFrame`
- Exe games: preserve Win32 message loop and rendering approach

**Multi-Agent Coordination**

6 worker agents + 2 directors operate on this repo on overlapping schedules. You are the **Game Content Expander** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`game_content_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Feature Expander agent), `app_fix_plan.md` (QA agent), `new_app_plan.md` (Creator agent), `usability_plan.md`
- **Shared file `KiloOS/src/App.jsx`** — shared ownership. You may ONLY add entries to the APPS array (to register new games). Protocol: `git pull` → add APPS entry only → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — Do NOT edit.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.

**General Constraints**
- No KiloApp may exceed 999KB (web or native).
- All game HTML files must be SINGLE self-contained files (inline CSS + JS).
- Do NOT edit: `master_plan.md`, `architecture.md`, `.agents/AGENTS.md`, `KiloOS/src/index.css`.
- Do NOT edit other agents' plan files: `app_work_plan.md`, `app_fix_plan.md`, `usability_plan.md`.
- Your focus is CONTENT, GAMEPLAY DEPTH, and GAME LENGTH — not visual polish. Add more levels, more enemies, more modes, more mechanics. Fill the 999KB budget with gameplay content.
- Do NOT add ARG/easter egg elements.
- **CLEANUP:** Before committing, delete any temporary `patch_*.py` or `patch_*.js` scripts in the repo root. Subagents must not leave scratch files behind.
- **Logging discipline:** Keep this plan file concise. Brief notes per turn in the Progress Log. Do NOT dump file contents or create verbose logs.

---
## Game Inventory & Parity Audit

### 🎮 Deep Games (Content Expansion Queue — round-robin, pick top, do content work, move to bottom)
| Game       | Parity | Status | Next Work |
|------------|--------|--------|-----------|
| KAsteroids | Tier 1 | Loop 9 done (Branching paths, escort missions, black hole hazard) | Loop 10: Supermassive asteroid bosses, warp core collection, zero-G inertia anomalies |
| KMaze      | Tier 1 | Loop 9 done (Spike traps, boss rematches, cursed relics) | Loop 10: Multi-level dungeons, save points, dynamic lighting |
| KPac       | Tier 1 | Loop 9 done (Multi-phase bosses, escort missions, branching paths) | Loop 10: Arcade endless mode, procedural ghost personalities, item crafting |
| KBreakout  | Tier 1 | Loop 9 done (Advanced level editor, meteor hazards, boss rematches) | Loop 10: Multi-ball chaos mode, gravity wells, custom powerup forging |
| KSnake     | Tier 1 | Loop 9 done (Snake vs Snake mode, growing walls, weather effects) | Loop 10: Map editor, boss gauntlet, branching campaigns |
| KRogue     | Tier 1 | Loop 10 done (Final sanctuary floors, true ending, ultra bosses) | Loop 11: Classes/Loadouts (Warrior, Mage, Rogue) |
| KQuest     | Tier 1 | Loop 9 done (Factions, mount system, crafting overhaul) | Loop 10: Kingdom management, army battles, castle defense |
| KStarship  | Tier 1 | Loop 9 done (Interstellar factions, planetary landing, crew moral) | Loop 10: Galactic super-weapons, faction wars, alien boarding parties |
| KFortress  | Tier 1 | Loop 1 done (3 new towers [Tesla, Ballista, Poison], Dynamite trap, 4 new enemies [Necromancer, Skeleton, Wyvern, Golem], Hero Militia squad, 2 new maps) | Loop 2: Elemental tower fusions, siege weapon upgrades, endless challenge mutators |
| KAlchemy   | Tier 1 | Loop 2 done (8 Tier-6 Mythic elements, 16 secret combos, Daily Trials with streaks, Magnum Opus Rebirth with 4 Astral Perks) | Loop 3: Ancient Alchemical Guild expeditions, planetary transmutations, elemental familiar summons |
| KColony    | Tier 1 | Loop 2 done (4 new structures [Geothermal, Bio-Dome, Shield Pylon, Drone Hub], 10-tier tech tree, planetary biomes [Cryo, Volcanic, Acid] & dynamic weather, mutator anomalies, 3-tier expeditions) | Loop 3: Colony trade freighters, underground cavern networks, orbital strike beacons |
| KSpace     | Tier 1 | Loop 10 done (Elite enemy squads, planetary bombardment missions, weapon overcharge) | Loop 11: Capital ship dreadnought sieges, drone companion wings, hyper-jump mechanics |

### ♟️ Classic Games (Balance & Usability Queue — round-robin, pick top, do balance audit, move to bottom)
| Game       | Parity | Status | Next Work |
|------------|--------|--------|-----------|
| KFreecell  | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Checked that all dealt hands are solvable, rebalanced campaign difficulty constraints. |
| KSudoku    | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Audit puzzle generation quality. Ensure Easy puzzles have unique solutions and Hard is solvable. |
| K2048      | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Check tile spawn rates, verify move-limit stages are achievable, audit powerup balance. |
| KMines     | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Check first-click safety, mine density per difficulty, ensure Sonar powerup isn't overpowered. |
| KTowers    | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Verified optimal move counts (BFS solver) and checked locked-disk puzzles. Fixed unlock bug in main.c. |
| KTetris    | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Adjusted Pentomino piece frequency in late stages and smoothed the drop speed curve. |
| KPong      | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Adjusted AI paddle speed per difficulty, nerfed boss, fixed powerup spawn rates. |
| KHangman   | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Audited word difficulty, removed multi-word phrases, smoothed strike limits for late campaign. |
| KWords     | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Fixed word placement silent failures, buffed Fog of War radius from 3x3 to 5x5, fixed UI bugs. |
| KSimon     | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Reset timer on correct inputs, smoothed speed curves, fixed 6-button label bugs. |
| KMatch3    | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Auto-shuffle on no moves, nerfed Boss HP, reduced powerup costs. |
| KDarts     | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Tuned wind/wobble by difficulty, fixed 301 bust bug, fixed Killer logic. |
| KChess     | Tier 1 | Loop 9 done — BALANCE PASS COMPLETE | Tuned Easy/Hard/Master Minimax depths and opening/endgame PST. |
| KGo        | Tier 1 | Loop 9 done — BALANCE PASS COMPLETE | Fixed AI turn skipping, handicap AI trigger, double-pass scoring, and tuned Grandmaster positional weights. |
| KReversi   | Tier 1 | Loop 9 done — BALANCE PASS COMPLETE | Fixed AI softlock on pass, increased Grandmaster Minimax depths, verified difficulty curve. |
| KConnect4  | Tier 1 | Loop 9 done — BALANCE PASS COMPLETE | Upgraded Grandmaster heuristic evaluation for stronger defensive and aggressive plays, verified blocker drop mechanics. |
| KSolitaire | Tier 1 | Loop 9 done — BALANCE PASS COMPLETE | Guaranteed opening deal fairness, fixed foundation-to-tableau Vegas scoring, added bankroll persistence on abandon, and added right-click instant foundation play. |


## Progress Log

**Loop 10**
- [x] KSpace (Loop 10: Added 4 Elite Enemy Squad archetypes [Valkyrie, Phantom, Cruiser, Drop Pod] with distinct elite behaviors and auras, dynamic Planetary Bombardment missions with targeted kinetic orbital strikes and burning surface terrain, Weapon Overcharge hyper-mode gauge system with triple damage and piercing energy plasma, Overcharge Core powerups, and alert banners in both EXE and HTML)
- [x] KColony (Loop 2: Added 4 new advanced structures [Geothermal Generator, Bio-Dome, Shield Pylon, Drone Hub], 10-tier research tech tree with passive upgrades, planetary biomes [Mars Prime, Cryo Tundra, Volcanic Inferno, Acid Swamp] with dynamic weather [Blizzards, Solar Flares, Acid Rain, Seismic Tremors], mutator anomalies, and 3-tier expedition system in both EXE and HTML)
- [x] KAlchemy (Loop 2: Added 8 Tier-6 Secret/Mythic elements, 16 secret combinations, Daily Alchemical Trials with streak tracking and rerolls, and Magnum Opus Rebirth prestige system with 4 permanent Astral Perks in both EXE and HTML)
- [x] KFortress (Loop 1: Added Tesla, Ballista, and Poison towers, Dynamite trap, Necromancer, Skeleton, Wyvern boss, and Stone Golem enemies, Hero 4th skill [Militia Reinforcements], and 2 new maps [Thunder Peak, Eldritch Necropolis] in both EXE and HTML)
- [x] KSolitaire (Balance audit complete: Implemented deal fairness validator guaranteeing playable opening moves, fixed Vegas scoring exploit for returning cards to tableau, added active match bankroll persistence on abandon, and added right-click instant foundation play in both EXE and HTML)
- [x] KRogue (Loop 10: Added True Sanctuary biome (floors 41-50), Tier 8 Ultra Bosses (Seraphim, Eldritch God), and relocated True Astaroth final battle to floor 50 in both EXE and HTML)

**Loop 9**
- [x] KStarship (Loop 9: Added Interstellar Factions, Planetary Landings (rest/explore events), and Crew Morale resource system in both EXE and HTML)
- [x] KConnect4 (Balance audit complete: Upgraded Grandmaster heuristic evaluation to heavily penalize opponent 3-in-a-rows and value its own traps, verified blocker cell mechanics in both EXE and HTML)
- [x] KQuest (Loop 9: Added 3 Joinable Factions, Mounts System (Horse/Wolf/Dragon) in Town Stables, and Crafting Overhaul with Masterwork Relics in both EXE and HTML)
- [x] KReversi (Balance audit complete: Fixed AI softlock on double-pass, increased Grandmaster Minimax depths for 6x6/8x8/10x10 boards, verified difficulty curve in both EXE and HTML)
- [x] KGo (Balance audit complete: Fixed AI turn blocking, fixed handicap AI triggering, added double-pass auto-score, and tuned Grandmaster AI 3rd/4th line territory evaluation in both EXE and HTML)
- [x] KSnake (Loop 9: Added Snake vs Snake mode, dynamic growing maze walls, and rain weather effects in both EXE and HTML)
- [x] KChess (Balance audit complete: Adjusted Easy heuristic, increased Hard to 3-ply and Master to 4-ply Minimax, and verified opening/endgame PST logic in both EXE and HTML)
- [x] KBreakout (Loop 9: Added interactive Level Editor mode, Meteor Shower falling hazards, and expanded campaign to 40 stages with Boss Rematches in both EXE and HTML)
- [x] KDarts (Balance audit complete: Scaled wind/wobble by difficulty, tuned AI accuracy curve, fixed 301 bust bug, and fixed Killer mode target logic in both EXE and HTML)
- [x] KPac (Loop 9: Added multi-phase Ghost King Boss, VIP ghost escort missions, and randomized branching level portals in both EXE and HTML)
- [x] KMatch3 (Balance audit complete: Implemented auto-shuffle when no moves exist, nerfed Stage 20 Boss HP to 75, reduced all powerup costs to 300 in both EXE and HTML)
- [x] KMaze (Loop 9: Added toggling Spike Traps, Cursed Relics [risk/reward score item], and procedural Boss Rematches every 5 levels in both EXE and HTML)
- [x] KSimon (Balance audit complete: Reset timers on correct input, smoothed sequence speed curve, fixed 6-button label map bug in both EXE and HTML)
- [x] KAsteroids (Loop 9: Added branching sector portals, allied freighter escort defense, and Black Hole gravity hazards in both EXE and HTML)
- [x] KWords (Balance audit complete: Guaranteed target word placement, buffed Fog of War reveal radius to 5x5, and fixed input/color UI bugs in both EXE and HTML)
- [x] KSpace (Loop 9: Added multi-phase bosses, allied escort defense missions, and post-boss branching paths in both EXE and HTML)
- [x] KHangman (Balance audit complete: Removed multi-word phrases, smoothed strike limit difficulty curve for stages 11-20 in both EXE and HTML)
- [x] KColony (Loop 1: Rebalanced Mine production, added dynamic scaling to alien spawns/HP, and deepened expedition risks/rewards in both EXE and HTML)
- [x] KPong (Balance audit complete: Standardized AI speeds, nerfed Stage 20 boss speed, and fixed powerup spawn rates in both EXE and HTML)
- [x] KAlchemy (Loop 1: Expanded recipes to 62, rebalanced quest gold/XP scaling, and reduced early lab upgrade costs in both EXE and HTML)
- [x] KTetris (Balance audit complete: Adjusted Pentomino frequency for late stages and smoothed drop speed curve in both EXE and HTML)
- [x] KStarship (Loop 8: Added Fleet Battles, Alien Diplomacy, and interactive Deep Space Anomalies in both EXE and HTML)
- [x] KTowers (Balance audit complete: Validated optimal move counts, fixed stage unlock bug, verified locked-disk solvability in both EXE and HTML)
- [x] KRogue (Loop 9: Advanced trap mechanics, boss rematches, cursed items to both EXE and HTML)

**Loop 8**
- [x] KQuest (Loop 8: Added Paladin and Ranger job classes, overhauled magic system with Spells menu and class skills, added Ranger DPS companion 'Robin' to the Mercenary Guild in both EXE and HTML)
- [x] KMines (Balance audit complete: Maintained 3x3 first-click safety with 1000+ attempt safe fallback, rebalanced Easy/Rush mine density by resizing to 9x9, modified Sonar powerup to reveal safe-cells without directly flagging mines in both EXE and HTML)
- [x] K2048 (Balance audit complete: Adjusted spawn rates to 90/10 for 2/4, mathematically rebalanced move limits for all campaign stages, fixed Stage 29 target, and updated powerup starting stocks in both EXE and HTML)
- [x] KSnake (Expanded Campaign Mode to 25 stages ending with Omega Boss Lair, added Ghost Berry food, Hunter AI rivals that chase the player, and fixed C linking bug by replacing math.h trig functions in both EXE and HTML)
- [x] KSudoku (Balance audit complete: Implemented backtracking solver to guarantee unique solutions for Easy/Medium puzzles, ensured Hard puzzles remain solvable in both EXE and HTML)
- [x] KBreakout (Expanded Campaign Mode to 30 stages featuring string-based custom level loader, Phantom Glass bricks, Armored bricks, and Stage 30 Final Core Boss variant in both EXE and HTML)
- [x] KFreecell (Balance audit complete: Implemented inline DFS solver to guarantee dealt hands are solvable, rebalanced campaign constraints for better difficulty curve in both EXE and HTML)
- [x] KPac (Expanded Campaign Mode to include maze hazards (tile 6), unpredictable teleport behavior for the Red Ghost on loop 7+, and an 8x scoring multiplier on loop 7+ with 0 fright timer in both EXE and HTML)
- [x] KSolitaire (Balance audit complete: Fixed Vegas mode stock pass rules, adjusted economy score deductions for undo and powerups, added score floor for Classic mode in both EXE and HTML)
- [x] KMaze (Expanded Campaign Mode to new maze themes, puzzle rooms, NPC encounters, and Time Freeze [T] active item to both EXE and HTML)
- [x] KConnect4 (Balance audit complete: Increased Grandmaster AI minimax depth to 7, fixed top-down physical piece drop logic for blocker cells in both EXE and HTML)
- [x] KReversi (Balance audit complete: Adjusted Minimax depths for 6x6/8x8/10x10 boards, added 50/50 tie-breaking logic in both EXE and HTML)
- [x] KSpace (Expanded Campaign with V-Strike formation, Weapon Upgrades powerups, and Scoring Combos to both EXE and HTML)
- [x] KGo (Balance audit complete: Enhanced AI defensive evaluation, adjusted handicap system, fixed Tsumego puzzle layouts in both EXE and HTML)
- [x] KChess (Balance audit complete: Adjusted AI difficulty curve, implemented endgame awareness in Piece-Square Tables, made Easy beatable and Hard a 2-ply Minimax for both EXE and HTML)
- [x] KRogue (Expanded to 40 levels, added Abyss biome, new enemies (Void Walker, Abyss Fiend, Deep Worm, Abyssal Overlord), Crafting Anvil, and Quest NPCs to both EXE and HTML)
- [x] KAsteroids (Added Dense Asteroid Belt environments to Sectors 8 and 13, Missile Swarm active weapon [M], and regenerating Deflector Shields for Cruiser UFOs to both EXE and HTML)

**Loop 7**
- [x] KStarship (Implemented Crew Management, Deep Space Trade Routes, Sector Campaign Narrative Events, and the Upgrade Bay Tech Tree to both EXE and HTML)
- [x] KQuest (Expanded Campaign to 17 chapters with Crystal Caverns & Ruined Castle, added Tavern state with Barkeep, side quests, and gear upgrades to both EXE and HTML)
- [x] KReversi (Expanded Campaign Mode to 20 stages, 4 AI Personalities, 2X Double-Flip bonus tiles, and Bomb Disc [B], Freeze AI [F], Optimal Hint [H], Undo [U] active skills to both EXE and HTML)
- [x] KRogue (Expanded to 30 levels across 6 biomes, moved Astaroth the Fallen to L30, added Arch-Mage, Lich Lord, Shadow Assassin, Shadow Behemoth, Mimic Chests, 3 trap types, Meteor Strike [M], Invisibility Cloak [I], Divine Miracle [D] spells to both EXE and HTML)
- [x] KMines (Expanded Campaign to 20 stages with dynamic board sizes 8x8 to 24x24, hidden Treasure Chests, timed speedrun stages, Sonar [R], Detector [D], Blast Shield [S] power-ups, and Blitz rapid-clear combo multipliers to both EXE and HTML)
- [x] KSnake (Expanded Campaign to 20 stages with custom maze obstacle maps, moving portals, CPU rival snakes, Stage 20 Hydra Viper Boss, Golden Apple / Poison Berry / Speed Berry fruits, and Ghost [G], Time Slow [F], Food Magnet [M] active skills to both EXE and HTML)
- [x] KTetris (Expanded Campaign Mode to 20 stages with progressive drop speeds, 6 Pentomino 5-block piece shapes in stage 15+, Row Nuke [B], Piece Swap [S], Gravity Freeze [F] active skills, and Earthquake board hazards to both EXE and HTML)
- [x] KPong (Expanded Campaign Mode to 20 stages with progressive ball speeds, dynamic arena obstacles, gravity wells, warp portals, multi-ball rounds & pickups, Stage 20 Hyper-CPU Boss paddle with adaptive spin and shield barrier, and Slow-Mo [F], Mega Paddle [P], Fireball Shot [B] active skills to both EXE and HTML)
- [x] KMaze (Expanded Campaign Mode to 35 stages with progressive grid dimensions 11x11 to 41x41, Darkness Fog of War, Lava Traps, Teleporter Pads, Locked Doors & Keys, Stage 35 Minotaur King Boss room, Pickaxe/Wall Breaker [P], Compass/Pathfinder [C], Speed Shoes [S], and Minotaur Stun Spray [F] active skills to both EXE and HTML)
- [x] KSolitaire (Expanded Campaign Mode to 20 stages featuring Draw 1 / Draw 3 variations, Vegas scoring constraints, Suit-locked foundations, Ice/Frozen cards, Stage 20 Grandmaster Challenge, Magic Wand [W], X-Ray Vision [X], Shuffle Stock [S], Free Undo [U] active skills, and Vegas Money Mode & stats tracking to both EXE and HTML)
- [x] KSpace (Expanded Campaign Mode to 20 waves featuring Kamikaze interceptors, Stealth cloak fighters, Asteroid hazards, formation attacks, Stage 20 Alien Mothership Boss with multi-stage turret destruction and deflector shield phases, and Time Stop [T], Tactical Dash [D], Smart Bomb [B], Hyper-Shield [S] active skills to both EXE and HTML)
- [x] KPac (Expanded Campaign Mode to 20 stages featuring 20 unique maze layouts, warp tunnels, speed zones, 5 AI Ghost Personalities (Blinky, Pinky, Inky, Clyde, Sue), Stage 20 Ghost King Boss encounter with phantom clone spawns and 8 HP, and Freeze [F], Sprint [S], Magnet [M], Shield [B] active skills to both EXE and HTML)
- [x] KChess (Expanded Campaign Mode to 20 stages, 4 AI Personalities (Novice, Aggressive Attacker, Positional Defender, Grandmaster Minimax Alpha-Beta), Optimal AI Hint [H], Undo Move [U], Time Freeze [F], Chess Puzzle Mode, and Blitz Timer Mode to both EXE and HTML)
- [x] KBreakout (Expanded Campaign Mode to 20 stages featuring 20 unique brick architectures, moving hazard shields, UFO drone enemies, Stage 20 Boss Fortress with rotating shields and core HP, Explosive 3x3 AoE, 3-hit metal, portal warp, mystery drop special bricks, and Laser [L], Multi-Ball Split [M], Fireball [F], Safety Barrier [B] active skills to both EXE and HTML)
- [x] K2048 (Expanded Campaign Mode to 30 stages featuring dynamic board sizes 3x3 to 6x6, target tile goals 256 to 8192, move limits, Stage 30 8192 Grandmaster Challenge, Frozen/Bomb/Wildcard special tiles, Tile Upgrade [U], Grid Rotate [R], Tile Hammer [H], Free Undo [Z] active skills to both EXE and HTML)
- [x] KSudoku (Expanded Campaign Mode to 20 stages featuring 4x4 Mini, 9x9 Classic, and 16x16 Hexadoku grids, Killer Sudoku sum cages, Fog of War darkness cells, Stage 20 Fiendish Master Challenge, Smart Hint [H], Pencil Auto-Fill [P], Mistake Shield [S], Time Freeze [F] active skills to both EXE and HTML)
- [x] KAsteroids (Expanded Campaign Mode to 20 Sectors featuring Armored Asteroids (3 hits to split), Mag-Mine homing hazards, Alien Hunter Squadrons, Sector Space Storms, Stage 20 Alien Mothership Core Boss with multi-component turrets & deflector shield, and EMP Blast [E], Piercing Laser [L], Hyperdrive Warp [H], Energy Shield [S] active skills to both EXE and HTML)
- [x] KFreecell (Expanded Campaign Mode to 20 stages featuring 4-cell, 3-cell, 2-cell constraints, King-only empty tableau rules, Baker's Game suit build, Frozen cards thawed by adjacent plays, Stage 20 Grandmaster Challenge, Magic Wand [W], Extra Freecell [E], Auto-Solve [A], Free Undo [U] active skills, and Time Attack Blitz mode & stats tracking to both EXE and HTML)
- [x] KConnect4 (Expanded Campaign Mode to 20 stages featuring dynamic grid dimensions 7x6 to 10x8, crackable blocker cells, 4 AI personalities (Rookie, Aggressive, Trapper, Grandmaster Minimax Alpha-Beta), Stage 20 Grandmaster Challenge, Bomb/Drill/Magnet discs, and Optimal AI Hint [H], Undo [U], Column Freeze [F] active skills to both EXE and HTML)
- [x] KHangman (Expanded Campaign Mode to 20 stages featuring 10 word categories, progressive word length 4-14 letters, strike limits 3 to 6 max wrong guesses, Stage 20 Polymath Grandmaster Challenge, Vowel Reveal [V], Consonant Radar [H], Strike Shield [S], Freeze Timer [F] active skills, and Time Attack Blitz mode to both EXE and HTML)
- [x] KSimon (Expanded Campaign Mode to 20 stages featuring progressive speeds down to 100ms, sequence lengths up to 30 steps, 4/6/8-button grid layouts, Chaos Reverse, Pitch Audio/Sound-Only mode, Stage 20 Grandmaster Memory Master Challenge, and Hint [H], Slow-Mo [S], Strike Shield [B], Time Freeze [F] active skills to both EXE and HTML)
- [x] KMatch3 (Expanded Campaign Mode to 20 stages featuring dynamic 6x6 to 10x10 grids, Stage 20 Jewel King Boss with 100 HP, barrier gems, moving obstacles, multi-hit Stone & Iron tiles, 3x3 Bomb, Rainbow, Line Blaster gems, and Hammer [H], Extra Moves [E], Shuffle [S], Color Nuke [L] active skills to both EXE and HTML)
- [x] KWords (Expanded Campaign Mode to 20 stages with dynamic grid dimensions 10x10 to 20x20, 12 distinct word search themes, Stage 20 Polyglot Grandmaster Challenge, 2-click Frozen tiles, Fog of War darkness cells, bonus Secret Words, and Word Radar [R], Word Pathfinder [P], Freeze Timer [F], Hint [H] active skills to both EXE and HTML)
- [x] KGo (Expanded Campaign Mode to 20 stages featuring 9x9, 13x13, 19x19 boards, handicap 0-8, variable komi, 4 AI Personalities (Territorial, Influence, Balanced, Grandmaster), Tsumego puzzles, Stage 20 Go Legend Challenge, Superko & Ko overlay, and Optimal AI Hint [H], Territory Estimator [T], Undo [U], Group Liberty Analyzer [S] active skills to both EXE and HTML)
- [x] KDarts (Expanded Campaign Mode to 20 stages, 6 game modes including 501 Double Out and Killer Darts, progressive AI opponents, wind & wobble mechanics, and Focus (75% wobble slowdown) [F], Magnet [M], Undo Dart [U], Laser Sight Arc Preview [L] active skills to both EXE and HTML)
- [x] KTowers (Expanded Campaign Mode to 20 stages featuring 3-10 disks, 3-5 pegs, Frame-Stewart solver, Stage 20 Tower Grandmaster Challenge, cyclic & color-coded rules, locked disks, and Frame-Stewart Hint [H], Undo [U], Time Freeze [F], Teleport [S] active skills to both EXE and HTML)

**Loop 6**
- [x] KQuest (Expanded Campaign to 15 chapter dungeons, added multi-stage boss encounters, and 4 battle spells & consumables (Lightning Storm, Holy Shield, Phoenix Elixir, Berserk Might) to both EXE and HTML)
- [x] KStarship (Expanded Campaign to 15 Sectors, added Energy Allocation & Subsystems, 7 Enemy Types & Multi-stage Bosses, and 4 Power-ups to both EXE and HTML)
- [x] KMines (Expanded Campaign to 15 levels, added Sonar power-up and Rush Mode to both EXE and HTML)
- [x] KRogue (Expanded to 25 levels, added Abyss biome, and moved final boss to level 25 for both EXE and HTML)
- [x] KSnake (Expanded to 15-level campaign win, added Portals, and Tracker Bug enemies to both EXE and HTML)
- [x] KTetris (Expanded Campaign to 15 stages, added Bomb power-up, and Earthquake mechanics to both EXE and HTML)
- [x] KPong (Expanded Campaign to 15 levels, added Horizontal Obstacle, and Freeze power-up to both EXE and HTML)
- [x] KMaze (Expanded to 30 levels, Darkness mechanic, Minotaur enemy, Pickaxe powerup added to both EXE and HTML)
- [x] KSolitaire (Expanded Campaign to 15 stages, added Ghost and Freeze power-ups, and Rush Mode to both EXE and HTML)
- [x] KSpace (Added Boss Enemy, Time Stop power-up, and Dash mechanic to both EXE and HTML)
- [x] KPac (Expanded Campaign to 15 maps, added Freeze power-up, and Green Tracker Ghost to both EXE and HTML)
- [x] KChess (Expanded Campaign to 15 stages, added Freeze and Undo powerups to both EXE and HTML)
- [x] KBreakout (Expanded Campaign with unique layouts, added Piercing Ball & Extra Life powerups, UFO enemies, and Lifetime Stats to both EXE and HTML)
- [x] K2048 (Expanded Campaign to 30 stages, added Rotate and Upgrade Power-Ups, and mapped Undo in C version to both EXE and HTML)
- [x] KSudoku (Expanded Campaign to 15 stages, added Fog mechanic, Rush Mode, and Shield powerup to both EXE and HTML)
- [x] KAsteroids (Added 15-Sector Campaign, Mother Boss UFO, Armored Asteroids, EMP & Laser powerups, and Hyperdrive ability to both EXE and HTML)
- [x] KFreecell (Expanded Campaign to 15 stages, added King-only empty tableau mechanic, Magic Wand & Extra Cell powerups, and Time Attack Mode to both EXE and HTML)
- [x] KConnect4 (Expanded Campaign to 15 stages, added Drill power-up, Speed Mode, and Crackable blocks to both EXE and HTML)
- [x] KHangman (Expanded Campaign to 15 stages, added 5 new categories, Time Attack Blitz mode, and Shield power-up to both EXE and HTML)
- [x] KSimon (Expanded Campaign to 15 stages, added Hint, Slow-mo, and Shield power-ups, and Chaos Mode to both EXE and HTML)
- [x] KMatch3 (Expanded Campaign to 15 stages, added Stone obstacles, 3x3 Bomb gems, Rainbow gems, and 3 powerups (Hammer, Extra Moves, Shuffle) to both EXE and HTML)
- [x] KWords (Expanded Campaign to 15 stages, added 3 new themes (Elements, Sci-Fi, Myth), Radar, Freeze Timer, Hint powerups, and Frozen tile obstacles to both EXE and HTML)
- [x] KGo (Expanded Campaign to 15 stages, added 3 AI personalities (Territorial, Influence, Grandmaster), Tsumego puzzles, AI Hint (H), Territory Estimator (T), and Undo (U) to both EXE and HTML)
- [x] KDarts (Expanded Campaign to 15 stages, added 5 game modes (501, 301, Cricket, ATC, Blitz), Focus/Magnet/Undo powerups, wind and wobble mechanics to both EXE and HTML)
- [x] KTowers (Expanded Campaign to 15 stages, 3-5 peg Reve's Puzzles, Frame-Stewart solver, Hint, Undo, Time Freeze powerups, Locked Disks, Adjacent-only rules to both EXE and HTML)
- [x] KReversi (Expanded Campaign to 15 stages, added blocked/hole cells, 4 AI personalities (including Grandmaster Minimax), and Bomb Disc power-up to both EXE and HTML)

**Loop 5**
- [x] KMines (Expanded Campaign to 10 levels, added Radar powerup and Lifetime Stats tracking to both EXE and HTML)
- [x] KRogue (Expanded to 20 levels, added Balrog, Titan, Beholder, Mind Flayer, and Void biome to both EXE and HTML)
- [x] KSnake (Expanded to 10-Level campaign win, added Spider enemies, Ice power-up, and Lifetime Stats tracking to both EXE and HTML)
- [x] KTetris (Expanded Campaign to 10 stages, added Dynamic Garbage injection in later stages, and Hard Drop scoring to both EXE and HTML)
- [x] KPong (Expanded Campaign to 10 levels, added moving obstacles, debuff powerups, and Lifetime Wins tracking to both EXE and HTML)
- [x] KMaze (Expanded to 20-level campaign, added Speed Boost powerup, Teleporters, and Lifetime Stats tracking to both EXE and HTML)
- [x] KSolitaire (Expanded Campaign to 10 stages, added Bomb trap and X-Ray powerups to both EXE and HTML)
- [x] KSpace (Added Zig-Zag enemy, Asteroid obstacles, Smart Bomb power-up, and Lifetime Stats tracking to both EXE and HTML)
- [x] KPac (Expanded Campaign to 10 maps, enhanced Ghost AI, and Lifetime Stats tracking to both EXE and HTML)
- [x] KChess (Expanded Campaign to 10 stages, added Castling, En Passant, and Piece-Square Table AI Evaluation to both EXE and HTML)
- [x] KBreakout (Expanded Campaign with unique layouts, added Piercing Ball & Extra Life powerups, UFO enemies, and Lifetime Stats to both EXE and HTML)
- [x] K2048 (Expanded Campaign to 20 stages, added Shuffle and Hammer Power-Ups to both EXE and HTML)
- [x] KSudoku (Added 10-Stage Campaign Mode, Magic Wand Power-up, and 3-Strikes rule to both EXE and HTML)
- [x] KAsteroids (Added Shield and Spread Shot Power-Ups, and Tracker Mine enemies to both EXE and HTML)
- [x] KFreecell (Added 10-Stage Campaign Mode, Baker's Game Ruleset, and Shuffle Power-Up to both EXE and HTML)
- [x] KConnect4 (Added 10-Stage Campaign Mode, Bomb Power-Up, and Obstacles to both EXE and HTML)
- [x] KHangman (Expanded to 10 Categories x 20 Words, added 10-Stage Campaign Mode and Bomb Power-Up to both EXE and HTML)
- [x] KSimon (Expanded to 6-color grid, added 10-Stage Campaign Mode, and Hint power-up to both EXE and HTML)
- [x] KMatch3 (Added Ice tiles obstacle, Shuffle Power-up, and Bomb Power-up to both EXE and HTML)
- [x] KWords (Added 10-Stage Campaign Mode, Magic Wand power-up, 4 new themes, and Combo System to both EXE and HTML)
- [x] KGo (Added 10-Stage Campaign Mode, Handicap Stones, and Hard AI to both EXE and HTML)

**Loop 4**
- [x] KMines (Added Campaign Mode with 5 progressively larger grids, and a single-use Shield mechanic to both EXE and HTML)
- [x] KRogue (Expanded to 15 levels, added Ghost/Hydra/Cube enemies, Biomes, Lightning spell, and Kills tracking to both EXE and HTML)
- [x] KSnake (Added Campaign Mode, Ghost Power-Up, and Native High Score Persistence to both EXE and HTML)
- [x] KTetris (Added Start Screen, 5-Stage Campaign Mode, and Line Clear Statistics Tracking to both EXE and HTML)
- [x] KPong (Added 5-Level Campaign Mode, mid-match paddle-buff Power-Ups, and Persistent High Rally stats to both EXE and HTML)
- [x] KMaze (Added Lava Traps, Fake Walls, Compass minimap lock, and expanded handcrafted levels from 5 to 10 to both EXE and HTML)
- [x] KSolitaire (Added 5-Stage Campaign Mode, Clock & Shuffle Power-Ups, and Persistent Lifetime Statistics tracking to both EXE and HTML)
- [x] KSpace (Added Wave progression, Boss Tank enemies, Rapid Fire powerup, Sound Effects, and Stats Tracking to both EXE and HTML)
- [x] KPac (Added Campaign Mode with 3 maps, Speed Power-Up, and Pink Ghost intercept AI to both EXE and HTML)
- [x] KChess (Added 5-Stage Campaign Mode, 1-ply Minimax AI Difficulty, and Lifetime Statistics Tracking to both EXE and HTML)
- [x] KBreakout (Added 5-Stage Campaign Mode, Extra Life/Piercing Ball powerups, and Lifetime Statistics Tracking to both EXE and HTML)
- [x] K2048 (Added 10-Stage Campaign Mode, Bomb Tiles (-3), and Threes Ruleset to both EXE and HTML)

**Loop 3**
- [x] KMines (Added chording feature for fast clearing and true first-click safety (3x3 empty space) to both EXE and HTML)
- [x] KRogue (Added Magic Shrine feature to both EXE and Web, added Orc and Cave Troll enemies to Web)
- [x] KSnake (Added Golden Apple mechanic (bonus points, shrinks snake) to both EXE and HTML)
- [x] KTetris (Added wall kicks and combo score multiplier to both EXE and HTML)
- [x] KPong (Added paddle spin physics and dynamic shrinking paddles as rally increases to both EXE and HTML)
- [x] KMaze (Added procedural maze generation extending the game from 5 to 10 levels to both EXE and HTML)
- [x] KSolitaire (Added score and streak mechanic instead of just moves to both EXE and HTML)
- [x] KSpace (Added enemy bullets for Type 2 enemies to create bullet-hell mechanics to both EXE and HTML)
- [x] KPac (Added target-tracking AI for Red Ghost and Fruit bonus item spawning to both EXE and HTML)
- [x] KChess (Added check highlighting, strict move validation, and checkmate/stalemate detection to both EXE and HTML)
- [x] KChess (Added basic greedy AI (PvE mode) with toggle ('M') to both EXE and HTML)
- [x] KBreakout (Added unbreakable bricks, 2-hit bricks, and wide paddle power-up to both EXE and HTML)
- [x] K2048 (Added Campaign Mode, Wildcard tile, and pitch-scaling Audio to both EXE and HTML)

**Loop 2**
- [x] KMines (Added difficulty levels: Easy/Medium/Hard to both EXE and HTML)
- [x] KRogue (Added Ice Storm spell & Teleport Trap to EXE, upgraded Web version toward Tier 2 with messages, stairs, gold, and varied enemies)
- [x] KSnake (Added randomly placed obstacles based on difficulty and sound effects to both EXE and HTML)
- [x] KTetris (Added Hold piece feature and sound effects to both EXE and HTML)
- [x] KPong (Added win condition (11 points), game over state, restart mechanic, and hit particles (web) to both EXE and HTML)
- [x] KMaze (Added minimap and sound effects to both EXE and HTML)
- [x] KSolitaire (Added initial preview phase and sound effects to both EXE and HTML)
- [x] KSpace (Added tracking enemies and shield powerups to both EXE and HTML)
- [x] KPac (Added 3 lives system with visual display to both EXE and HTML)
- [x] KChess (Added visual valid move indicators to both EXE and HTML)
- [x] KBreakout (Added 3 lives system instead of instant game over to both EXE and HTML)
- [x] Create New Game (K2048 created for both EXE and Web, registered in App.jsx)

**Loop 1**
- [x] KRogue (EXE: Added Skeleton, Centipede, Gargoyle, Demon, Amulet of Life)
- [x] KSnake (Added high scores and progressive difficulty speed-up to both EXE and HTML)
- [x] KTetris (Added next piece preview and high scores to both EXE and HTML, added progressive speed to HTML)
- [x] KSolitaire (Added moves counter, high score tracking, and Easy/Hard difficulty modes to both EXE and HTML)
- [x] KSpace (Added starfield background, progressive difficulty spawn/speed, high scores, restart to both EXE and HTML)
- [x] KMines (Added best time tracking, timer, and sound effects to both EXE and HTML)
- [x] KPong (Added progressive ball speed and Web Audio/MessageBeep sound effects on paddle hit and scoring to both EXE and HTML)
- [x] KMaze (Added start screen, win screen, best time tracking and elapsed time display to both EXE and HTML)
- [x] KPac (Added high score tracking and sound effects to both EXE and HTML)
- [x] KChess (Added King capture check/game over screen, 'R' to restart, and sound effects to both EXE and HTML)
- [x] Create New Game (KBreakout created for both EXE and Web, registered in App.jsx)
