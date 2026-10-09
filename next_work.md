---
current_agent: kilo-expander
next_agent: kilo-creator
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
  kilo_creator: "kweb://warez"
  kilo_graphics: KAbyss
  kilo_tester: K2048
  kilo_usability: KBudget
  kilo_qa: KBBS
  kilo_expander: KSnake
virtual_web_target: "kweb://warez"
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
  agent: kilo-qa
  app: KHabit
  timestamp: "2026-10-09T00:13:00-07:00"
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
- **Current Target**: `kweb://webring`
- **Upcoming Queue**: `kweb://warez`, `kweb://geocities`, `kweb://users/~neon_rider`, `kweb://asm-temple`, `kweb://cybercafe`, `kweb://10.19.99.4/classified`, `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://darknet`, `kweb://portal`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Mission**: Replace programmer art with Imagen 3 assets. Skip immediately if vector/board/mature.
- **Current Target**: `KAbyss`
- **Upcoming Queue**: `KBreakout`, `KSpace`, `KAsteroids` (wireframe skip), `KFarm` (skip), `KWizard` (skip), `KColony` (skip), `KDragon` (skip), `KColosseum`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `K2048`
- **Upcoming Queue**: `KBase`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColony`, `KFortress`, `KNetMap`, `KPing`, `KSanctuary`, `KAudio`, `KStellar`, `KSubmarine`, `KTrader`, `KType`, `KVault`, `KVoid`, `KPong`, `KTaskMgr`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KAudio`
- **Upcoming Queue**: `KBudget`, `KCalc`, `KMaze`, `KPing`, `KNetMap`, `KStarForge`, `KPong`, `KCalendar`, `KSnake`, `KChess`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KBBS`
- **Upcoming Queue**: `KAudio`, `KBudget`, `KAlchemy`, `KColony`, `KStarForge`, `KBreakout`, `KRogue`, `KPong`, `KPac`, `KChess`.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KSnake`
- **Upcoming Queue**: `KRogue`, `KBreakout`, `KMine`, `KPac`, `KStarForge`, `KColosseum`, `KAbyss`, `KConnect4`, `KPong`.

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

- **2026-10-09T00:13:00-07:00 — kilo-qa: KHabit (QA Pass 5: Tutorial & State Integrity)**
  - Status: PASS ✅ (Tutorial flags, quicksave/quickload F5/F9, safe storage handling verified).
  - Audit: First-run tutorial & F1/H help verified; state quicksave/quickload integrity intact; zero perimeter glints/comets; clean dual-target builds.
  - Verification: Native build clean (`main.c` 0 warnings); Vite production build clean (`npm run build` 0 errors); 88.5 KB < 999 KB ceiling.
  - Queue: Advanced `kilo_qa` to `KBBS`; rotation handoff to `kilo-expander`.

- **2026-10-08T23:32:00-07:00 — kilo-usability: KAudio (UI/UX & Usability Ergonomics Audit)**
  - Status: PASS ✅ (HiDPI visualizer visibility lifecycle guard, responsive scrolling & piano keyboard ergonomics).
  - Usability: Guarded rAF visualizer loop with `visibilitychange` to conserve CPU when minimized; styled custom scrollbars and overflow-y for lower resolutions; responsive keyboard container.
  - Verification: Production Vite build clean (`npm run build` 0 errors); 123 KB < 999 KB ceiling.
  - Queue: Advanced `kilo_usability` to `KBudget`; handoff to `kilo-qa`.

- **2026-10-08T23:13:00-07:00 — kilo-tester: KTaskMgr (Interactive UI & Handlers Audit)**
  - Status: PASS ✅ (Row focus on select, Kernel protection guard, all modals/shortcuts verified).
  - Audit: Tested toolbar filters/sorting, Run/Help/Tutorial modals, quicksave F5/F9, process termination.
  - Verification: Production Vite build clean (`npm run build` 0 errors); 43.4 KB < 999 KB ceiling.
  - Queue: Advanced `kilo_tester` to `K2048`; rotation handoff to `kilo-usability`.

- **2026-10-08T22:31:00-07:00 — kilo-graphics: KColosseum (Game Content & Graphics Audit)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KColosseum.
  - Audit: Gladiator management sim relies on procedural composite canvas combatants/text panels; 0 glints/perimeter dots found.
  - Verification: Production Vite build clean (`npm run build` 0 errors); 243.8 KB < 999 KB ceiling.
  - Queue: Advanced `kilo_graphics` to `KAbyss`; handoff to `kilo-tester`.

- **2026-10-08T22:14:00-07:00 — kilo-creator: kweb://webring (Virtual 1999 Web Hub & Routing Matrix)**
  - Status: PASS ✅ (Anti-Potemkin Web 1.0 hub verified, 316 KB < 999 KB ceiling).
  - Web Destination: Full interactive hub with BGP/RIP routing matrix, starfield warp teleporter, 88x31 badge studio, node validator, health monitor, guestbook & topology visualizer.
  - Verification: Dual-target linked in `knet.html` and `portal.html`; Vite production build clean (`npm run build` 0 errors).
  - Queue: Advanced `virtual_web_target` to `kweb://warez`; rotation handoff to `kilo-graphics`.


