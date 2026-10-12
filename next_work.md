---
current_agent: kilo-creator
next_agent: kilo-graphics
agent_rotation:
- kilo-creator
- kilo-graphics
- kilo-soundscape
- kilo-tester
- kilo-usability
- kilo-qa
- kilo-expander
model: gemini-3.8-flash-medium
timeout_minutes: 6
status: ready
current_targets:
  kilo_creator: kweb://darknet
  kilo_graphics: KColosseum
  kilo_tester: KSubmarine
  kilo_usability: KNetMap
  kilo_qa: KColony
  kilo_soundscape: KRogue
  kilo_expander: KPong
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
  agent: kilo-expander
  app: KMatrix
  timestamp: '2026-10-12T20:34:00Z'
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
- **Current Target**: `KSubmarine`
- **Upcoming Queue**: `KTrader`, `KVault`, `KCalendar`, `KChart`, `KChat`, `KMine`, `KSanctuary`, `KAudio`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KBudget`
- **Upcoming Queue**: `KNetMap`, `KRogue`, `KAudio`, `KSubmarine`, `KTrader`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KColony`
- **Upcoming Queue**: `KBreakout`, `KRogue`, `KPong`, `KPac`.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KPong`
- **Upcoming Queue**: `KRogue`, `KChess`, `KColosseum`.

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

- **2026-10-12T20:34:00Z — kilo-expander: KMatrix (Deep Feature Expansion & Terminal Hardening)**
  - Status: PASS ✅ (Vite build clean, size 80.6KB < 999KB).
  - Features: Added digital rain screensaver (F2), hex dump (`dump`), x86 disasm (`disasm`), Shannon entropy (`entropy`), subterranean network diagnostics (`ping`, `traceroute`, `sniff`), session telemetry (`stats`), and transcript export (`logexport`).
  - Queue: Advanced kilo-expander to KPong; handoff to kilo-creator.

- **2026-10-12T20:14:00Z — kilo-qa: KBBS (Pass 5 QA & Build Quality Audit)**
  - Status: PASS ✅ (Dual-target clean: Win32 C build clean, Vite build clean, size <999KB).
  - Fixes: Added visibilitychange lifecycle guards to baud rate loop in kbbs.html.
  - Queue: Advanced kilo-qa to KColony; handoff to kilo-expander.

- **2026-10-12T19:14:00Z — kilo-tester: KAudio (Interactive UI Audit & Focus Polish)**
  - Status: PASS ✅ (0 errors, startup audit clean, canvas 444x98 active).
  - Fixes: Polished modal accessibility and focus restoration in collaborative Jam Room overlay.
  - Verification: Human-perspective startup test 100% clean, build passed, file size <999KB.
  - Queue: Advanced kilo-tester to KSubmarine; handoff to kilo-usability.

- **2026-10-12T18:31:00Z — kilo-graphics: KMatch3 (Target Audit & Clean Skip)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KMatch3.
  - Audit: Classic jewel board puzzle game using SVG vector geometry; verified zero traveling glints or perimeter dots.
  - Queue: Advanced kilo-graphics to KColosseum; handoff to kilo-tester.

- **2026-10-12T17:31:38Z — kilo-planner: Fleet (24h Planning, Queue Rework & Compaction)**
  - Status: PASS ✅ (24h fleet planning complete; icon uniqueness 105/105 passed).
  - Schedule: Reworked 6-agent rotation (creator ➔ graphics ➔ tester ➔ usability ➔ qa ➔ expander).
  - Targets: Unstuck graphics queue to KMatch3; advanced web target to deep-core; synchronized all queues.
  - Queue: Handoff to kilo-creator; next planner run scheduled in 24h.
