---
current_agent: kilo-tester
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
  kilo_creator: kweb://geocities
  kilo_graphics: KDragon
  kilo_tester: KPing
  kilo_usability: KSnake
  kilo_qa: KBudget
  kilo_expander: KAbyss
virtual_web_target: kweb://geocities
virtual_web_rotation:
- kweb://asm-temple
- kweb://cybercafe
- kweb://10.19.99.4/classified
- kweb://echo-subsystem.net
- kweb://deep-core
- kweb://darknet
- kweb://portal
- kweb://webring
- kweb://warez
- kweb://geocities
- kweb://users/~neon_rider
last_run:
  agent: kilo-graphics
  app: KDragon
  timestamp: 2026-10-10T21:38:08+0000
last_planner_run: '2026-10-10T16:30:00Z'
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
- **Current Target**: `kweb://geocities`
- **Upcoming Queue**: `kweb://users/~neon_rider`, `kweb://asm-temple`, `kweb://cybercafe`, `kweb://10.19.99.4/classified`, `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://darknet`, `kweb://portal`, `kweb://webring`, `kweb://warez`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Mission**: Replace programmer art with Imagen 3 assets. Skip immediately if vector/board/mature.
- **Current Target**: `KDragon`
- **Upcoming Queue**: `KWizard`, `KFarm`, `KMatch3`, `KQuest`, `KSpace`, `KColosseum`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KPing`
- **Upcoming Queue**: `KSanctuary`, `KAudio`, `KStellar`, `KSubmarine`, `KTrader`, `KType`, `KVault`, `KVoid`, `KCalendar`, `KChart`, `KChat`, `KMine`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KSnake`
- **Upcoming Queue**: `KAudio`, `KBudget`, `KPing`, `KNetMap`, `KRogue`, `KCalendar`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KBudget`
- **Upcoming Queue**: `KAlchemy`, `KColony`, `KStarForge`, `KBreakout`, `KRogue`, `KPong`, `KPac`, `KBBS`.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KColosseum`
- **Upcoming Queue**: `KAbyss`, `KPong`, `KRogue`, `KChess`, `KMine`, `KPac`, `KStarForge`.

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

- **2026-10-10T21:38:08+0000 — kilo-graphics: KDragon (Zero-Token Auto-Skip — Inappropriate Target)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KDragon (pure vector, board game, or mature art).
  - Optimization: Handled via orchestrator pre-flight zero-token auto-skip.
  - Queue: Advanced `kilo-graphics` to `KDragon`; rotation handoff to `kilo-tester`.

- **2026-10-10T14:31:30-07:00 — kilo-creator: kweb://warez (Virtual 1999 Web Scene Vault & Forensics)**
  - Status: PASS ✅ (`kweb://warez` audited and verified; gold-standard scene vault)
  - Features: 14 tabs (x86 sandbox, YM2612 tracker, 64KB intro arena, ANSI studio, FXP courier, PE-Pack '99).
  - Integration: Verified `kweb://warez` in KNet, portal.html, webring.html; build clean (Vite 511ms, 415 KB < 999 KB).
  - Queue: Advanced creator target to `kweb://geocities`; handoff to `kilo-graphics`.

- **2026-10-10T14:16:00-07:00 — kilo-expander: KColosseum (Deep Feature Expansion & Combat Pacing)**
  - Status: PASS ✅ (`KColosseum` feature expansion passed)
  - Combat & Tactical Depth: Unlocked Roman War Cry tactical ability in solo league arena; adds crowd favor surge, enemy stagger status, and war horn audio.
  - Sizing & Build: Build clean (`npm run build`, 245 KB < 999 KB).
  - Queue: Advanced expander target to `KAbyss`; handoff to `kilo-creator`.

- **2026-10-10T13:31:00-07:00 — kilo-qa: KBBS (Pass 5 QA & State Integrity Audit)**
  - Status: PASS ✅ (`KBBS` Pass 5 tutorial & state persistence audit passed)
  - State & Integrity: Hardened F5/F9 master quicksave/quickload with silent settings persistence, zoom, and CRT sync; fixed `kbbs_tw_state` check in first-run tutorial guard.
  - Sizing & Build: Clean build (`npm run build`, web 169 KB < 999 KB; native 102 KB < 999 KB).
  - Queue: Advanced QA target to `KBudget`; handoff to `kilo-expander`.

- **2026-10-10T13:13:00-07:00 — kilo-usability: KCalendar (UI/UX Usability Audit & Pacing Pass)**
  - Status: PASS ✅ (`KCalendar` usability audit passed; mature app)
  - Ergonomics & Layout: Verified 1020x720 window bounds, F1/H user guide, accessible keyboard grid navigation (1-4 views, pills, cells).
  - Quality & Lifecycle: Storage wrapped in safeGet/safeSet, visibilitychange listeners present, zero forbidden glints/comets.
  - Sizing & Build: Clean build (`npm run build`, 114 KB < 999 KB). Advanced usability queue to `KSnake`; handoff to `kilo-qa`.
