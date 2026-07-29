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

4 worker agents + 1 director operate on this repo on overlapping schedules. You are the **Game Content Expander** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`game_content_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Feature Expander agent), `app_fix_plan.md` (QA agent), `new_app_plan.md` (Creator agent), `usability_plan.md` (inactive)
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
| KQuest     | Tier 1 | Loop 6 done (15 chapters, bosses, 4 spells) | Loop 7: Side quests, NPC dialogue, equipment upgrades, more chapter variety |
| KStarship  | Tier 1 | Loop 6 done (15 sectors, subsystems, 7 enemies) | Loop 7: Crew management, trade routes, narrative events, tech tree |
| KSpace     | Tier 1 | Loop 7 done (20 waves, bosses, 4 skills) | Loop 8: New enemy formations, weapon upgrades, scoring combos |
| KAsteroids | Tier 1 | Loop 7 done (20 sectors, Mothership boss) | Loop 8: Asteroid belt environments, weapon variety, shield mechanics |
| KMaze      | Tier 1 | Loop 7 done (35 stages, Minotaur boss, fog) | Loop 8: New maze themes, puzzle rooms, NPC encounters, deeper item system |
| KPac       | Tier 1 | Loop 7 done (20 mazes, 5 ghosts, Ghost King) | Loop 8: New maze hazards, ghost behaviors, scoring variety |
| KBreakout  | Tier 1 | Loop 7 done (20 stages, Boss Fortress) | Loop 8: New brick types, boss variety, level editor concepts |
| KSnake     | Tier 1 | Loop 7 done (20 stages, Hydra boss, rival AI) | Loop 8: New environments, food variety, rival AI behaviors |
| KRogue     | Tier 1 | Loop 8 done (40 levels, new enemies, crafting, NPCs) | Loop 9: Advanced trap mechanics, boss rematches, cursed items |
| KAlchemy   | Tier 1 | Creator building (Phase 12/14) | Wait for Creator to finish — then Loop 1 content pass |

### ♟️ Classic Games (Balance & Usability Queue — round-robin, pick top, do balance audit, move to bottom)
| Game       | Parity | Status | Next Work |
|------------|--------|--------|-----------|
| KChess     | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Audit AI difficulty curve, ensure Easy is beatable, Hard is challenging. Check opening/endgame AI quality. |
| KGo        | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Audit AI difficulty across 9x9/13x13/19x19. Ensure handicap system works. Test Tsumego puzzle quality. |
| KReversi   | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Audit AI difficulty curve, check Minimax depth, ensure board sizes feel right. |
| KConnect4  | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Audit AI difficulty levels. Ensure Grandmaster is genuinely hard. Check blocker cell mechanics. |
| KSolitaire | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Check deal fairness, ensure Vegas mode scoring is balanced, verify undo/powerup economy. |
| KFreecell  | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Check that all dealt hands are solvable. Verify difficulty constraints make sense. |
| KSudoku    | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Audit puzzle generation quality. Ensure Easy puzzles have unique solutions and Hard is solvable. |
| K2048      | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Check tile spawn rates, verify move-limit stages are achievable, audit powerup balance. |
| KMines     | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Check first-click safety, mine density per difficulty, ensure Sonar powerup isn't overpowered. |
| KTowers    | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Verify optimal move counts are achievable, check locked-disk puzzles are solvable. |
| KTetris    | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Check drop speed curve, verify Pentomino pieces don't make late stages impossible. |
| KPong      | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Audit AI paddle speed per difficulty, check powerup spawn rates, verify boss is beatable. |
| KHangman   | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Check word difficulty per category, ensure strike limits are fair, verify 14-letter words aren't absurd. |
| KWords     | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Check word placement in grids, ensure Fog of War mechanic is fun not frustrating. |
| KSimon     | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Check sequence speed curve, verify 8-button mode is playable, audit Chaos Reverse fairness. |
| KMatch3    | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Check board generation ensures moves exist, verify Boss HP is achievable, audit powerup economy. |
| KDarts     | Tier 1 | Loop 7 done — NEEDS BALANCE PASS | Check wind/wobble tuning per difficulty, verify AI opponent accuracy curve is fair. |


## Progress Log

**Loop 8**
- [x] KRogue (Expanded to 40 levels, added Abyss biome, new enemies (Void Walker, Abyss Fiend, Deep Worm, Abyssal Overlord), Crafting Anvil, and Quest NPCs to both EXE and HTML)

**Loop 7**
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
