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

4 agents operate on this repo on overlapping schedules. You are the **Games Content** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`game_content_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Builder agent), `app_fix_plan.md` (QA agent), `kiloos_ux_plan.md` (UX agent)
- **Shared file `KiloOS/src/App.jsx`** — owned by the UX agent. You may ONLY add entries to the APPS array (to register new games). Protocol: `git pull` → add APPS entry only → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — owned by the UX agent. Do NOT edit.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.

**General Constraints**
- No KiloApp may exceed 999KB (web or native).
- All game HTML files must be SINGLE self-contained files (inline CSS + JS).
- Do NOT edit: `master_plan.md`, `architecture.md`, `.agents/AGENTS.md`, `KiloOS/src/index.css`.
- Do NOT edit other agents' plan files: `app_work_plan.md`, `app_fix_plan.md`, `kiloos_ux_plan.md`.
- Do NOT redo polish work the Builder agent has already completed (check `app_work_plan.md`). Your focus is CONTENT and GAMEPLAY, not visual polish.
- Do NOT add ARG/easter egg elements.
- **Logging discipline:** Keep this plan file concise. Brief notes per turn in the Progress Log. Do NOT dump file contents or create verbose logs.

---
## Game Inventory & Parity Audit

| Game       | Web Parity Tier | Last Touched (Builder) | Status | Notes |
|------------|-----------------|------------------------|--------|-------|
| KMines     | Tier 1          | Feature-Expanded       | Needs Improvement | Full parity. |
| KRogue     | Tier 2          | Never Touched          | Needs Improvement | Web upgraded to basic playable version. EXE is a full roguelike. |
| KSnake     | Tier 1          | Never Touched          | Needs Improvement | Full parity. |
| KTetris    | Tier 1          | Never Touched          | Needs Improvement | Full parity. |
| KPong      | Tier 1          | Polished               | Needs Improvement | Full parity. |
| KMaze      | Tier 1          | Feature-Expanded       | Needs Improvement | Full parity. |
| KSolitaire | Tier 1          | Never Touched          | Needs Improvement | Memory Match game in both Web and EXE. |
| KSpace     | Tier 1          | Never Touched          | Needs Improvement | Full parity. |
| KPac       | Tier 1          | Feature-Expanded       | Needs Improvement | Full parity. |
| KChess     | Tier 1          | Polished / Expanded    | Needs Improvement | Full parity. |
| KBreakout  | Tier 1          | Never Touched          | Needs Improvement | Full parity, native and web. |
| K2048      | Tier 1          | Never Touched          | Needs Improvement | Full parity, native and web. |
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
