# KiloApps Games Content Plan

This document tracks the **perpetual, never-ending** improvement loop for all KiloApps games.

## Agent Rules & Guidelines

**Perpetual Loop (NEVER STOP)**
This agent loops forever. After completing a full loop through all games, start the next loop immediately. Each subsequent loop should go DEEPER than the last:
- **Loop 1:** Basic content pass (high scores, difficulty, sound effects).
- **Loop 2:** Deeper mechanics (new game modes, power-ups, AI improvements).
- **Loop 3+:** Approach commercial quality. Add MORE levels, MORE content, MORE variety. Games should feel rich and long. You have up to **999KB per game** — that is a LOT of room for content like level data, enemy patterns, word lists, puzzle banks, and procedurally generated variety. Fill that budget.
- **When all games in a loop are done:** If there are new games (created by the Creator agent), add them to the inventory. Then start the next loop with ALL games reset to "Needs Improvement".
- **CREATE NEW GAME** mode: Only if the Director or user explicitly requests it, OR if you finish a loop and all existing games are already very deep. The Creator agent is the primary new-game creator.

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

**Content Additions (1-2 turns per game, pick 2-3 per turn)**
- Additional levels, stages, or procedurally generated content
- Difficulty settings (Easy / Medium / Hard)
- Progressive difficulty (games should get harder as you play longer)
- High score tracking (localStorage for web, file/registry for native)
- Visual feedback: screen shake, particle effects, flash on hit, smooth transitions
- Sound effects (Web Audio API synthesized tones for web; Beep()/PlaySound for native)
- Power-ups, special abilities, or bonus items
- Enemy/obstacle variety
- Win/lose conditions with satisfying feedback
- Tutorial or first-play hints
- Keyboard AND mouse/touch controls where appropriate (web), keyboard for native

**EXE-Specific Improvements**
- Add new enemy types, items, or level content to the native C code
- Expand procedural generation (more room types, varied layouts)
- Add new gameplay mechanics that fit the existing architecture
- Improve the UI within terminal/GDI constraints
- Fix bugs or balance issues

**Quality Bar**
- Must have: start screen, score display, game-over screen with restart
- Must have at least 2 difficulty levels or progressive difficulty
- Persist high scores
- Responsive controls
- Web games: use `requestAnimationFrame`
- Exe games: preserve Win32 message loop and rendering approach

**Creating a New Game**
- Create BOTH native (`K[Name]/main.c`, `build.bat`, `app.rc`, `icon.ico`) and web versions (`KiloOS/public/apps/k[name].html`).
- Register in `App.jsx` APPS array with folder: 'Games'.
- Use appropriate window dimensions.
- Good candidates: Breakout, Asteroids, Frogger, 2048, Sudoku, Freecell, Hangman, Connect 4, etc.

**Multi-Agent Coordination**

4 worker agents + 1 director operate on this repo on overlapping schedules. You are the **Game Content Expander** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`game_content_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Feature Expander agent), `app_fix_plan.md` (QA agent), `new_app_plan.md` (Creator agent), `kiloos_ux_plan.md` (inactive)
- **Shared file `KiloOS/src/App.jsx`** — shared ownership. You may ONLY add entries to the APPS array (to register new games). Protocol: `git pull` → add APPS entry only → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — Do NOT edit.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.

**General Constraints**
- No KiloApp may exceed 999KB (web or native).
- All game HTML files must be SINGLE self-contained files (inline CSS + JS).
- Do NOT edit: `master_plan.md`, `architecture.md`, `.agents/AGENTS.md`, `KiloOS/src/index.css`.
- Do NOT edit other agents' plan files: `app_work_plan.md`, `app_fix_plan.md`, `kiloos_ux_plan.md`.
- Your focus is CONTENT, GAMEPLAY DEPTH, and GAME LENGTH — not visual polish. Add more levels, more enemies, more modes, more mechanics. Fill the 999KB budget with gameplay content.
- Do NOT add ARG/easter egg elements.
- **CLEANUP:** Before committing, delete any temporary `patch_*.py` or `patch_*.js` scripts in the repo root. Subagents must not leave scratch files behind.
- **Logging discipline:** Keep this plan file concise. Brief notes per turn in the Progress Log. Do NOT dump file contents or create verbose logs.

---
## Game Inventory & Parity Audit

| Game       | Web Parity Tier | Last Touched (Builder) | Status | Notes |
|------------|-----------------|------------------------|--------|-------|
| KMines     | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: 10-level campaign, radar powerup, stats. |
| KRogue     | Tier 2          | Feature-Expanded       | Loop 5 Completed | Web upgraded to basic playable version. EXE is a full roguelike. Expanded to 20 levels, added Balrog, Titan, Beholder, Mind Flayer, Void biome. |
| KSnake     | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: Spider enemies, Ice powerup, stats, 10-level campaign win. |
| KTetris    | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: 10-Stage Campaign, dynamic garbage, hard drop scoring. |
| KPong      | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: 10-Level campaign, moving obstacles, debuff powerups, stats. |
| KMaze      | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: 20-level campaign, speed powerup, teleporters, stats. |
# KiloApps Games Content Plan

This document tracks the **perpetual, never-ending** improvement loop for all KiloApps games.

## Agent Rules & Guidelines

**Perpetual Loop (NEVER STOP)**
This agent loops forever. After completing a full loop through all games, start the next loop immediately. Each subsequent loop should go DEEPER than the last:
- **Loop 1:** Basic content pass (high scores, difficulty, sound effects).
- **Loop 2:** Deeper mechanics (new game modes, power-ups, AI improvements).
- **Loop 3+:** Approach commercial quality. Add MORE levels, MORE content, MORE variety. Games should feel rich and long. You have up to **999KB per game** — that is a LOT of room for content like level data, enemy patterns, word lists, puzzle banks, and procedurally generated variety. Fill that budget.
- **When all games in a loop are done:** If there are new games (created by the Creator agent), add them to the inventory. Then start the next loop with ALL games reset to "Needs Improvement".
- **CREATE NEW GAME** mode: Only if the Director or user explicitly requests it, OR if you finish a loop and all existing games are already very deep. The Creator agent is the primary new-game creator.

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

**Content Additions (1-2 turns per game, pick 2-3 per turn)**
- Additional levels, stages, or procedurally generated content
- Difficulty settings (Easy / Medium / Hard)
- Progressive difficulty (games should get harder as you play longer)
- High score tracking (localStorage for web, file/registry for native)
- Visual feedback: screen shake, particle effects, flash on hit, smooth transitions
- Sound effects (Web Audio API synthesized tones for web; Beep()/PlaySound for native)
- Power-ups, special abilities, or bonus items
- Enemy/obstacle variety
- Win/lose conditions with satisfying feedback
- Tutorial or first-play hints
- Keyboard AND mouse/touch controls where appropriate (web), keyboard for native

**EXE-Specific Improvements**
- Add new enemy types, items, or level content to the native C code
- Expand procedural generation (more room types, varied layouts)
- Add new gameplay mechanics that fit the existing architecture
- Improve the UI within terminal/GDI constraints
- Fix bugs or balance issues

**Quality Bar**
- Must have: start screen, score display, game-over screen with restart
- Must have at least 2 difficulty levels or progressive difficulty
- Persist high scores
- Responsive controls
- Web games: use `requestAnimationFrame`
- Exe games: preserve Win32 message loop and rendering approach

**Creating a New Game**
- Create BOTH native (`K[Name]/main.c`, `build.bat`, `app.rc`, `icon.ico`) and web versions (`KiloOS/public/apps/k[name].html`).
- Register in `App.jsx` APPS array with folder: 'Games'.
- Use appropriate window dimensions.
- Good candidates: Breakout, Asteroids, Frogger, 2048, Sudoku, Freecell, Hangman, Connect 4, etc.

**Multi-Agent Coordination**

4 worker agents + 1 director operate on this repo on overlapping schedules. You are the **Game Content Expander** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`game_content_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Feature Expander agent), `app_fix_plan.md` (QA agent), `new_app_plan.md` (Creator agent), `kiloos_ux_plan.md` (inactive)
- **Shared file `KiloOS/src/App.jsx`** — shared ownership. You may ONLY add entries to the APPS array (to register new games). Protocol: `git pull` → add APPS entry only → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — Do NOT edit.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.

**General Constraints**
- No KiloApp may exceed 999KB (web or native).
- All game HTML files must be SINGLE self-contained files (inline CSS + JS).
- Do NOT edit: `master_plan.md`, `architecture.md`, `.agents/AGENTS.md`, `KiloOS/src/index.css`.
- Do NOT edit other agents' plan files: `app_work_plan.md`, `app_fix_plan.md`, `kiloos_ux_plan.md`.
- Your focus is CONTENT, GAMEPLAY DEPTH, and GAME LENGTH — not visual polish. Add more levels, more enemies, more modes, more mechanics. Fill the 999KB budget with gameplay content.
- Do NOT add ARG/easter egg elements.
- **CLEANUP:** Before committing, delete any temporary `patch_*.py` or `patch_*.js` scripts in the repo root. Subagents must not leave scratch files behind.
- **Logging discipline:** Keep this plan file concise. Brief notes per turn in the Progress Log. Do NOT dump file contents or create verbose logs.

---
## Game Inventory & Parity Audit

| Game       | Web Parity Tier | Last Touched (Builder) | Status | Notes |
|------------|-----------------|------------------------|--------|-------|
| KMines     | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: 10-level campaign, radar powerup, stats. |
| KRogue     | Tier 2          | Feature-Expanded       | Loop 5 Completed | Web upgraded to basic playable version. EXE is a full roguelike. Expanded to 20 levels, added Balrog, Titan, Beholder, Mind Flayer, Void biome. |
| KSnake     | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: Spider enemies, Ice powerup, stats, 10-level campaign win. |
| KTetris    | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: 10-Stage Campaign, dynamic garbage, hard drop scoring. |
| KPong      | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: 10-Level campaign, moving obstacles, debuff powerups, stats. |
| KMaze      | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: 20-level campaign, speed powerup, teleporters, stats. |
| KSolitaire | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: 10-stage campaign, Bomb trap, X-Ray powerup. |
| KSpace     | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: Zig-Zag enemies, Asteroids, Smart Bomb, Lifetime Stats. |
| KPac       | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: Expanded Campaign to 10 maps, advanced AI for all ghosts, and Lifetime Stats tracking. |
| KChess     | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: Expanded Campaign to 10 stages, Castling, En Passant, and PST AI Evaluation. |
| KBreakout  | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: Campaign layouts, new powerups, UFO enemy, Lifetime Stats. |
| K2048      | Tier 1          | Feature-Expanded       | Loop 5 Completed | Full parity. Loop 5 completed: 20-level campaign, Hammer & Shuffle powerups. |
| KSudoku    | Tier 1          | Game Content Expander  | Loop 5 Completed | Full parity. Loop 5 completed: 10-Stage Campaign, Magic Wand power-up, 3-Strikes rule. |
| KAsteroids | Tier 1          | Creator-Created        | Needs Improvement | Created by Creator agent. Loop 5 pending. |
| KFreecell  | Tier 1          | Creator-Created        | Needs Improvement | Created by Creator agent. Loop 5 pending. |
| KConnect4  | Tier 1          | Creator-Created        | Needs Improvement | Created by Creator agent. Loop 5 pending. |
| KHangman   | Tier 1          | Creator-Created        | Needs Improvement | Created by Creator agent. Loop 5 pending. |
| KSimon     | Tier 1          | Creator-Created        | Needs Improvement | Created by Creator agent. Loop 5 pending. |
| KMatch3    | Tier 1          | Creator-Created        | Needs Improvement | Created by Creator agent. Loop 5 pending. |
| KWords     | Tier 1          | Creator-Created        | Needs Improvement | Created by Creator agent. Loop 5 pending. |
| KGo        | Tier 1          | Creator-Created        | Needs Improvement | Created by Creator agent (in progress). Loop 5 pending. |

## Progress Log

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
