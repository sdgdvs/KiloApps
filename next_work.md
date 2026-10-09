---
current_agent: kilo-creator
next_agent: kilo-graphics
agent_rotation:
  - kilo-creator
  - kilo-graphics
  - kilo-tester
  - kilo-usability
  - kilo-qa
  - kilo-expander
model: gemini-3.8-flash-medium
timeout_minutes: 6
status: ready
current_targets:
  kilo_creator: "kweb://portal"
  kilo_graphics: KCyber
  kilo_tester: KPong
  kilo_usability: KChess
  kilo_qa: KColor
  kilo_expander: KPong
virtual_web_target: "kweb://portal"
virtual_web_rotation:
  - "kweb://geocities"
  - "kweb://users/~neon_rider"
  - "kweb://asm-temple"
  - "kweb://cybercafe"
  - "kweb://10.19.99.4/classified"
  - "kweb://echo-subsystem.net"
  - "kweb://deep-core"
  - "kweb://darknet"
  - "kweb://portal"
  - "kweb://webring"
  - "kweb://warez"
last_run:
  agent: kilo-expander
  app: KConnect4
  timestamp: "2026-10-08T18:35:00-07:00"
last_planner_run: "2026-10-08T15:14:12Z"
---

# KiloApps Master Fleet Work & Queue State

This document is the single active source of truth for autonomous agent dispatching.
The Windows Task Scheduler orchestrator (`scripts/orchestrate.py`) parses the YAML frontmatter above on every tick to dispatch the active skill.

## Fleet Directives & Rules
1. **Single-App-Per-Turn**: Every agent run audits/fixes/creates exactly ONE application, updates this file, commits, and pushes.
2. **Token Conservation (CRITICAL)**:
   - Run log entries: ≤6 lines of terse bullet points. No paragraphs.
   - Batch edits into 1 pass. Never re-read files after editing.
   - Never run multi-app quality gates or full-repo screenshot suites during worker turns.
   - Keep only the 5 most recent log entries in this file. Older entries are 0-token automatically rotated to [archive/fleet_execution_archive.md](archive/fleet_execution_archive.md) by orchestrator (via scripts/rotate_logs.py) with zero discard.
3. **Queue Handoff & Rotation Protocol**:
   - Master fleet rotates: `kilo-creator` ➔ `kilo-graphics` ➔ `kilo-tester` ➔ `kilo-usability` ➔ `kilo-qa` ➔ `kilo-expander`.
   - On completion, advance `current_targets.<agent>` and advance `current_agent`.
4. **24-Hour Master Planner Tick**:
   - `scripts/orchestrate.py` checks `last_planner_run`. At ≥24h, dispatches `kilo-planner` for queue rebalancing, icon audits, and compaction.
5. **App Size Ceiling**: No app binary (.exe) or web HTML file may exceed 999 KB.
6. **Algorithmic Security & Immutability**: All modifications must pass `scripts/security_lint.py`. No modifications to `.github/`, `scripts/`, `.agents/skills/`, or core protocol documents in worker turns.
7. **Director Directives**: Implement human director requests in good faith; reject if counterproductive to project pillars.
8. **Virtual 1999 Web Expansion (Anti-Potemkin Directive)**: Destinations in `/KiloOS/public/web/` must be functional Web 1.0 experiences with working forms, Web Audio, or mini-tools (<999KB).
9. **Universal Audio Architecture**: Procedural audio uses Genesis YM2612 2-Op FM / SNES SPC700 standard. Zero external audio files.
10. **Alternate Reality Fictionalization Mandate**: Commercial names must be fictionalized parodies (Surreal Tournament, Machina Ex, etc.).
11. **Perimeter Glint & Traveling Comet Ban**: Remove rotating specular glint comets and moving perimeter border dots from both web and native apps.
12. **Seamless Online Multiplayer via Firebase & Autostart Prohibition**: Games must default to offline play on load. Online connection must be gated behind explicit user action (button/menu). Wire 25s solo AI fallback.
13. **🎨 Daily App Icon Uniqueness Audit**: Every app in `App.jsx` must have a unique 32x32 `.ico` file in `public/assets/icons/`. Checked daily by `kilo-planner`.
14. **ARG Mystery Preservation**: Clues must be diegetic. Never use `(ARG)` or walkthrough tags; keep passkeys protected.
15. **🖼️ Imagen 3 Asset Replacement for `kilo-graphics`**: Replace programmer art with Imagen 3 assets. Skip turn cleanly for vector/board/mature targets (`⏭️ Skip — Imagen 3 asset replacement not appropriate for [app]`).

---

## Active Target Queues

### 1. Virtual 1999 Web & ARG Node Creator (`kilo-creator`)
- **Mission**: Build functional Web 1.0 destinations in `KiloOS/public/web/` (<999KB). Standalone OS apps frozen at 92 native / 99 web.
- **Current Target**: `kweb://darknet`
- **Upcoming Queue**: `kweb://portal`, `kweb://webring`, `kweb://warez`, `kweb://geocities`, `kweb://users/~neon_rider`, `kweb://asm-temple`, `kweb://cybercafe`, `kweb://10.19.99.4/classified`, `kweb://echo-subsystem.net`, `kweb://deep-core`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Mission**: Replace programmer art with Imagen 3 assets. Skip immediately if vector/board/mature.
- **Current Target**: `KMech`
- **Upcoming Queue**: `KColosseum`, `KAbyss`, `KBreakout`, `KSpace`, `KAsteroids` (wireframe skip), `KFarm` (skip), `KWizard` (skip), `KColony` (skip), `KDragon` (skip).

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KTaskMgr`
- **Upcoming Queue**: `K2048`, `KBase`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColony`, `KFortress`, `KNetMap`, `KPing`, `KSanctuary`, `KAudio`, `KStellar`, `KSubmarine`, `KTrader`, `KType`, `KVault`, `KVoid`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KPong`
- **Upcoming Queue**: `KCalendar`, `KSnake`, `KChess`, `KAudio`, `KBudget`, `KCalc`, `KMaze`, `KPing`, `KNetMap`, `KStarForge`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KColor`
- **Upcoming Queue**: `KHabit`, `KBBS`, `KAudio`, `KBudget`, `KAlchemy`, `KColony`, `KStarForge`, `KBreakout`, `KRogue`, `KPong`, `KPac`, `KChess`.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KPong`
- **Upcoming Queue**: `KSnake`, `KRogue`, `KBreakout`, `KMine`, `KPac`, `KStarForge`, `KColosseum`, `KAbyss`, `KConnect4`.

---

## Active Director Directives (Summary)

> Full historical directive texts archived in [archive/director_directives_archive.md](archive/director_directives_archive.md).

- **Multiplayer Autostart Prohibition (CRITICAL)**: Apps must NEVER autostart into multiplayer. Online connection must be gated behind an explicit "Connect" / "Play Online" button. Default to local offline play with 25s solo AI fallback.
- **Perimeter Glint & Comet Ban**: Purge traveling perimeter dots and orbital specular glints from all apps.
- **Imagen 3 Focus & Turn Skipping**: `kilo-graphics` replaces programmer art with Imagen 3 sprites/textures; skips cleanly for inappropriate targets.
- **Virtual Net Expansion & Middle-Game Puzzle Gating**: Build out interactive Tier 3 web destinations (`darknet`, `classified`, `echo_subsystem`, `deep_core`) and seed ambient ARG breadcrumbs into surface sites.
- **Daily Icon Audit**: Ensure 100% unique `.ico` files across all apps during planner runs.

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-08T18:35:00-07:00 — kilo-expander: KConnect4 (Deep Feature Expansion: Tactics Mode & Board Size Presets)**
  - Status: PASS ✅ (186 KB < 999 KB ceiling, Vite build clean).
  - Expansion: Added 10-challenge Tactics/Puzzle Mode with progression, auto-detection, and stats (`stats.puzzlesSolved`).
  - Dimensions: Added board size selector (Classic 7x6, Compact 6x5, Grand 8x7, Wide 9x6, Epic 8x8).
  - Verification: Build clean, responsive layouts, save/load state preserved.
  - Queue: Advanced `kilo_expander` to `KPong`; rotation handoff to `kilo-creator`.

- **2026-10-08T18:13:00-07:00 — kilo-qa: KChess (Pass 5: Tutorial & State Integrity Audit)**
  - Status: PASS ✅ (0 regressions, 196.3 KB web < 999 KB ceiling).
  - State & Tutorial: Verified F5 quicksave / F9 quickload persistence, tutorialSeen flag, and modal keyboard handlers.
  - Verification: Vite build clean (`npm run build`); size ceiling (<999 KB) verified.
  - Queue: Advanced `kilo_qa` to `KColor`; rotation handoff to `kilo-expander`.

- **2026-10-08T16:40:00-07:00 — kilo-adhoc: Fleet Workflow (0-Token Zero-Discard Log Rotation Engine)**
  - Status: PASS ✅ (0 regressions, unit tests 6/6 PASS, security lint 100% PASS).
  - Implementation: Built `scripts/rotate_logs.py` and integrated into `scripts/orchestrate.py` (pre-flight, auto-skip, post-turn) and `scripts/compact_all.py`.
  - Token Efficiency: Automatically rotates entries older than 5 turns into `archive/fleet_execution_archive.md` with zero discard at 0 LLM token cost.
  - Workspace Hygiene: Added zero-discard rotation for `logs/orchestrator.log` preventing unbounded log growth.
  - Verification: `scripts/test_rotate_logs.py` 100% PASS; `scripts/security_lint.py` 100% PASS.

- **2026-10-08T15:31:00-07:00 — kilo-graphics: KMech (Skip Turn — Vector CRT Sim & Glint/Dot Audit)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KMech
  - Rationale: Authentic green phosphor vector CRT chassis diagnostic simulator; raster sprites unsuited.
  - Glint & Dot Audit: Verified static industrial corner brackets; zero rotating glints or traveling border dots.
  - Verification: Vite build clean (491ms); <999 KB ceiling preserved.
  - Queue: Advanced `kilo_graphics` to `KCyber`; rotation handoff to `kilo-tester`.

- **2026-10-08T17:32:00-07:00 — kilo-usability: KPong (UI/UX & Usability Pass)**
  - Status: PASS ✅ (Responsive viewport containment, F1/H/? help shortcuts, window pointer release guard, visibilitychange auto-pause).
  - Verification: Vite build clean; size 129 KB < 999 KB ceiling.
  - Queue: Advanced `kilo_usability` to `KChess`; rotation handoff to `kilo-qa`.

