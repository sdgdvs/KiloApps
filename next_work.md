---
current_agent: kilo-usability
next_agent: kilo-qa
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
  kilo_creator: kweb://warez
  kilo_graphics: KStellar
  kilo_tester: KChart
  kilo_usability: KSubmarine
  kilo_qa: KRogue
  kilo_soundscape: KStellar
  kilo_expander: KColosseum
virtual_web_target: kweb://warez
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
  agent: kilo-tester
  app: KCalendar
  timestamp: '2026-10-13T02:31:00Z'
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
   - Master fleet rotates: `kilo-creator` ➔ `kilo-graphics` ➔ `kilo-soundscape` ➔ `kilo-tester` ➔ `kilo-usability` ➔ `kilo-qa` ➔ `kilo-expander`.
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
- **Current Target**: `KStellar`
- **Upcoming Queue**: `KColosseum`, `KMech`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KChart`
- **Upcoming Queue**: `KChat`, `KMine`, `KSanctuary`, `KAudio`, `KSubmarine`, `KTrader`, `KVault`, `KCalendar`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KSubmarine`
- **Upcoming Queue**: `KTrader`, `KBudget`, `KRogue`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KRogue`
- **Upcoming Queue**: `KPong`, `KPac`, `KColony`, `KBreakout`.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KColosseum`
- **Upcoming Queue**: `KPong`, `KRogue`, `KChess`.

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

- **2026-10-13T02:31:00Z — kilo-tester: KCalendar (Interactive UI & Startup Audit)**
  - Startup & UI element audit: CSS valid, 0 JS errors, modals/tabs/shortcuts intact.
  - F5/F9 persistence, CSV/JSON import/export, and backdrop dismissals verified.
  - Status: PASS ✅ (Vite build clean, size 113KB < 999KB).
  - Queue: Advanced kilo-tester to KChart; handoff to kilo-usability.

- **2026-10-13T02:16:00Z — kilo-soundscape: KSpace (YM2612 FM Soundscape & SPC700 Delay)**
  - Replaced generic FM presets with 64-step chiptune tracker (4 modal patterns in Dm/Fmaj, walking slap bass, crystal lead, SPC700 ambient drone, FM drums).
  - Upgraded sound FX palette (types 0-12) with pitch sweeps, harmonic chords, and master soft-clipping limiter.
  - Status: PASS ✅ (Vite build clean, size 185KB < 999KB).
  - Queue: Advanced kilo-soundscape to KStellar; handoff to kilo-tester.

- **2026-10-13T01:31:00Z — kilo-graphics: KSpace (Asset Audit & Queue Advance)**
  - ⏭️ Skip — Imagen 3 asset replacement not appropriate for KSpace (production Imagen 3 assets already active; perimeter glints/dots absent).
  - Status: PASS ✅ (Vite build clean, size 184KB < 999KB).
  - Queue: Advanced kilo-graphics to KStellar; handoff to kilo-soundscape.

- **2026-10-13T00:33:00Z — kilo-expander: KChess (Deep Utility Expansion)**
  - Status: PASS ✅ (Vite build clean, size 227KB < 999KB).
  - Deep utility: Live on-canvas Centipawn Evaluation Bar (`X` / menu toggle), multi-format clock manager (`K` - Blitz/Bullet/Rapid/Classical/Casual).
  - Wired hotkeys, help documentation, and responsive menu bar indicators.
  - Queue: Advanced kilo-expander to KColosseum; handoff to kilo-creator.

- **2026-10-13T00:15:00Z — kilo-qa: KBreakout (Pass 5 QA & State Integrity Audit)**
  - Status: PASS ✅ (Vite build clean, size 160KB < 999KB).
  - Quicksave (F5) / Quickload (F9) and beforeunload state persistence validated.
  - First-run tutorial flag (`kbreakout_tutorialSeen`) and duel non-autostart intact.
  - Security lint 100% clean; verified zero native C binary dependencies.
  - Queue: Advanced kilo-qa to KRogue; handoff to kilo-expander.
