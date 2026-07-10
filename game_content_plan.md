# KiloApps Games Content Plan

This document tracks the perpetual improvement loop for all KiloApps games, ensuring they become fun, replayable, and content-rich.

## Agent Rules & Guidelines

**Mode Determination**
- **IMPROVE EXISTING**: If any game in the list has NOT completed a full content pass.
- **CREATE NEW GAME**: If all existing games have completed at least one full content pass. After creating a new game, reset all games to "needs improvement".

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

**General Constraints**
- No KiloApp may exceed 999KB.
- All game HTML files must be SINGLE self-contained files (inline CSS + JS).
- Do NOT edit: `master_plan.md`, `architecture.md`, `.agents/AGENTS.md`, `KiloOS/src/index.css`, `app_work_plan.md`, `app_fix_plan.md`, `kiloos_ux_plan.md`.
- Do NOT redo polish work the Builder agent has already completed. Your focus is CONTENT and GAMEPLAY, not visual polish.
- Do NOT add ARG/easter egg elements.

---
## Game Inventory & Parity Audit

| Game       | Web Parity Tier | Last Touched (Builder) | Status | Notes |
|------------|-----------------|------------------------|--------|-------|
| KMines     | Tier 1          | Feature-Expanded       | Needs Improvement | Full parity. |
| KRogue     | Tier 3          | Never Touched          | Needs Improvement | Web is a download stub (0.7KB). EXE is a 64KB full roguelike. |
| KSnake     | Tier 1          | Never Touched          | Needs Improvement | Full parity. |
| KTetris    | Tier 1          | Never Touched          | Needs Improvement | Full parity. |
| KPong      | Tier 1          | Polished               | Needs Improvement | Full parity. |
| KMaze      | Tier 1          | Feature-Expanded       | Needs Improvement | Full parity. |
| KSolitaire | Tier 1          | Never Touched          | Needs Improvement | Memory Match game in both Web and EXE. |
| KSpace     | Tier 1          | Never Touched          | Needs Improvement | Full parity. |
| KPac       | Tier 1          | Feature-Expanded       | Needs Improvement | Full parity. |
| KChess     | Tier 1          | Polished / Expanded    | Needs Improvement | Full parity. |

## Progress Log

**Loop 1**
- [x] KRogue (EXE: Added Skeleton, Centipede, Gargoyle, Demon, Amulet of Life)
- [x] KSnake (Added high scores and progressive difficulty speed-up to both EXE and HTML)
- [x] KTetris (Added next piece preview and high scores to both EXE and HTML, added progressive speed to HTML)
- [x] KSolitaire (Added moves counter, high score tracking, and Easy/Hard difficulty modes to both EXE and HTML)
- [ ] KSpace
- [ ] KMines
- [ ] KPong
- [ ] KMaze
- [ ] KPac
- [ ] KChess
- [ ] Create New Game

