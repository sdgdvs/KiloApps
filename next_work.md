---
current_agent: kilo-graphics
next_agent: kilo-tester
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
  kilo_creator: kweb://darknet
  kilo_graphics: KMatch3
  kilo_tester: KAudio
  kilo_usability: KBudget
  kilo_qa: KBBS
  kilo_expander: KMatrix
virtual_web_target: kweb://darknet
virtual_web_rotation:
- kweb://deep-core
- kweb://darknet
- kweb://portal
- kweb://webring
- kweb://warez
- kweb://geocities
- kweb://users/~neon_rider
- kweb://asm-temple
- kweb://cybercafe
- kweb://10.19.99.4/classified
- kweb://echo-subsystem.net
last_run:
  agent: kilo-creator
  app: kweb://deep-core
  timestamp: 2026-10-12T17:31:38Z
last_planner_run: '2026-10-12T17:31:38Z'
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
- **Current Target**: `kweb://deep-core`
- **Upcoming Queue**: `kweb://darknet`, `kweb://portal`, `kweb://webring`, `kweb://warez`, `kweb://geocities`, `kweb://users/~neon_rider`, `kweb://asm-temple`, `kweb://cybercafe`, `kweb://10.19.99.4/classified`, `kweb://echo-subsystem.net`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Mission**: Replace programmer art with Imagen 3 assets. Skip immediately if vector/board/mature.
- **Current Target**: `KMatch3`
- **Upcoming Queue**: `KColosseum`, `KMech`, `KCyber`, `KSpace`, `KStellar`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KAudio`
- **Upcoming Queue**: `KSubmarine`, `KTrader`, `KVault`, `KCalendar`, `KChart`, `KChat`, `KMine`, `KSanctuary`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KBudget`
- **Upcoming Queue**: `KNetMap`, `KRogue`, `KAudio`, `KSubmarine`, `KTrader`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KBBS`
- **Upcoming Queue**: `KColony`, `KBreakout`, `KRogue`, `KPong`, `KPac`.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KMatrix`
- **Upcoming Queue**: `KPong`, `KRogue`, `KChess`, `KColosseum`.

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

- **2026-10-12T17:31:38Z — kilo-planner: Fleet (24h Planning, Queue Rework & Compaction)**
  - Status: PASS ✅ (24h fleet planning complete; icon uniqueness 105/105 passed).
  - Schedule: Reworked 6-agent rotation (creator ➔ graphics ➔ tester ➔ usability ➔ qa ➔ expander).
  - Targets: Unstuck graphics queue to KMatch3; advanced web target to deep-core; synchronized all queues.
  - Queue: Handoff to kilo-creator; next planner run scheduled in 24h.

- **2026-10-12T10:28:30-07:00 — kilo-adhoc: Fleet (Increase Factory Turn Timing to 1 Turn/Hour)**
  - Status: PASS ✅ (Windows Scheduled Task re-armed to 1 turn/hr at :30 past each hour in continuous mode).
  - Queue Healing: Fixed non-UTF-8 CP1252 byte 0x97 in next_work.md; added resilient error handling in orchestrator.
  - Node Config: Updated register_task.ps1, orchestrator.py, and dashboard.py to detect DESKTOP-HSSS709 as PC B.
  - Verification: Dry run clean (daily planner next); Vite build 100% clean; security_lint 100% PASS.

- **2026-10-11T14:14:00+0000 — kilo-tester: KSanctuary (Interactive UI Audit & Hotkey Polish)**
  - Status: PASS ✅ (Audited modals, inputs, and hotkeys in ksanctuary.html).
  - Fixes: Bound Enter/Space dismissal for help modal; wired 'M' hotkey for audio toggle.
  - Validation: Clean build (<999KB), all interactive controls responsive.
  - Queue: Advanced kilo-tester to KAudio; handoff to kilo-usability.

- **2026-10-11T14:02:02+0000 — kilo-graphics: KDragon (Zero-Token Auto-Skip — Inappropriate Target)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KDragon (pure vector, board game, or mature art).
  - Optimization: Handled via orchestrator pre-flight zero-token auto-skip.
  - Queue: Advanced `kilo-graphics` to `KDragon`; rotation handoff to `kilo-tester`.

- **2026-10-11T13:50:00+0000 — kilo-creator: kweb://10.19.99.4/classified (Virtual 1999 Web Expansion)**
  - Status: PASS ✅ (Vite build clean; HTML 255 KB << 999 KB ceiling).
  - Web & ARG: Audited 10 skunkworks tabs (memos, hex, RF lab, topology, sniffer, term, airgap, interferometer, CAD).
  - Integration: Verified bidirectional routing across KNet, Portal, Webring, Geocities, and Neon Rider.
  - Queue: Advanced kilo-creator target to kweb://echo-subsystem.net; rotation handoff to kilo-graphics.
