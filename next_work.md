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
  kilo_creator: "kweb://darknet"
  kilo_graphics: KMech
  kilo_tester: KTaskMgr
  kilo_usability: KPong
  kilo_qa: KChess
  kilo_expander: KConnect4
virtual_web_target: "kweb://darknet"
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
  app: KChess
  timestamp: "2026-10-08T13:35:00-07:00"
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
   - Keep only the 5 most recent log entries in this file. Older entries move to [archive/fleet_execution_archive.md](archive/fleet_execution_archive.md).
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
- **Current Target**: `KAsteroids`
- **Upcoming Queue**: `KAlchemy`, `KColony`, `KStarForge`, `KBreakout`, `KRogue`, `KPong`, `KPac`, `KChess`, `KColor`, `KHabit`, `KBBS`, `KAudio`, `KBudget`.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KChess`
- **Upcoming Queue**: `KPong`, `KSnake`, `KRogue`, `KBreakout`, `KMine`, `KPac`, `KStarForge`, `KColosseum`, `KAbyss`.

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

- **2026-10-08T11:27:00-07:00 — kilo-usability: KBBS (UI/UX Pass: Lord Log Ergonomics, Help Shortcuts, Zoom Controls)**
  - Status: PASS ✅ (0 regressions, 312.4 KB web / 148.0 KB native < 999 KB ceiling).
  - UI Fixes: Added helper log functions for door state; added Ctrl+/-/0 zoom shortcuts; wired arrow navigation in help tutorial.
  - Verification: MSVC clean; Vite build clean (722ms); <999 KB size ceiling preserved.
  - Queue: Advanced `kilo_usability` to `KPong`; rotation handoff to `kilo-qa`.

- **2026-10-08T10:38:00-07:00 — kilo-tester: KSettings (Interactive UI Audit & Inline Repairs)**
  - Status: PASS ✅ (6 issues fixed, 0 regressions).
  - UI Fixes: Removed modal-close class collision on modal footer buttons; added direct JSON file upload to state dialog.
  - Parity & Standards: Corrected available app IDs (kterm, knote); added origin check to postMessage; live color oninput.
  - TINAG Compliance: Replaced meta-labels (Internal Subnet Gateway, Subsystem Telemetry Monitor).
  - Verification: `test_app_startup.py` 100% PASS; `security_lint.py` 100% PASS; Vite build clean (503ms); 70.6 KB < 999 KB ceiling.
  - Queue: Advanced `kilo_tester` to `KTaskMgr`; rotation handoff to `kilo-usability`.

- **2026-10-08T10:19:00-07:00 — kilo-graphics: KDragon (Skip Turn — Inappropriate Target & Glint/Dot Audit)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KDragon
  - Rationale: Authentic 16x16 pixel-art pet sim with 1:1 Win32 GDI C parity; downscaled raster sprites unsuited.
  - Glint & Dot Audit: Verified static medieval filigree border; zero rotating specular glints or traveling border dots.
  - Verification: `check_icons.py` 100% PASS; `security_lint.py` 100% PASS; MSVC clean (`KDragon.exe` 149.0 KB); Vite clean (1.30s); web (169.3 KB) < 999 KB ceiling.
  - Queue: Advanced `kilo_graphics` to `KMech`; rotation handoff to `kilo-tester`.

- **2026-10-08T09:37:00-07:00 — kilo-creator: kweb://deep-core (Anti-Potemkin Web 1.0 Expansion, CSS Patch, Routing & Golden Thread Pass)**
  - Status: PASS ✅ (242.7 KB web < 999 KB ceiling, 0 regressions).
  - Anti-Potemkin Web Depth: 10 interactive decks (VT100 core, 5 defusal sectors, 3D vector radar, RTDB mesh, x86 disasm, 650m sonar, wiretap, sanctum).
  - Synthesis & Gating: Genesis YM2612 2-Op FM / SNES SPC700 procedural audio, F5/F9 state parity, 25s solo fallback, full middle-game gating.
  - Fixes & Routing: Corrected .wiretap-hex-pane CSS syntax; verified deep routing in knet.html, portal.html, and webring.html.
  - Verification: `security_lint.py` 100% PASS; `test_arg_flow.py` 7/7 stages PASS; Vite build clean (494ms); quality gate 107/107 PASS.
  - Queue: Advanced `kilo_creator` and `virtual_web_target` to `kweb://darknet`; rotation handoff to `kilo-graphics`.

- **2026-10-08T08:14:12-07:00 — kilo-planner: fleet-wide (Daily 24h Fleet Planning, Queue Health & Log Compaction)**
  - Status: PASS ✅ (Daily fleet audit & queue rework complete).
  - Icon Uniqueness: Verified 105 apps in App.jsx, 105 unique .ico files, 0 duplicate hashes (check_icons.py 100% PASS).
  - Queue Health & Rework: Pruned locked mature apps from active queues; advanced kilo_creator to kweb://deep-core.
  - Compaction & Archival: Archived aging KQuarantine and echo-subsystem entries to archive/fleet_execution_archive.md.
  - Timestamp & Rotation: Updated last_planner_run to 2026-10-08T15:14:12Z; reset current_agent to kilo-creator.

### Agent Run Log — Pass 5 QA
- **Status:** 🟢 Completed (KAsteroids)
- Audited kasteroids.html: added safeGet/safeSet try/catch guards around localStorage.
- Verified build and size constraints (<999 KB).
- Handoff to kilo-expander for KChess.

### Agent Run Log — kilo-expander (KChess)
- **Status:** 🟢 Completed (KChess Deep Engine Utility Expansion)
- Implemented interactive Move Replay modal with ply stepping, table inspector & autoplay (hotkey V).
- Implemented Opening Book Explorer with standard openings and practice-mode loader (hotkey B).
- Added PGN file download (.pgn) and file upload import. Verified build and size (188 KB < 999 KB).
- Advanced queue: kilo_expander -> KConnect4; rotation handoff to kilo-creator.

