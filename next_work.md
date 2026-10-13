---
current_agent: kilo-graphics
next_agent: kilo-soundscape
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
  kilo_creator: kweb://webring
  kilo_graphics: KCyber
  kilo_tester: KVault
  kilo_usability: KAudio
  kilo_qa: KBreakout
  kilo_soundscape: KCyber
  kilo_expander: KChess
virtual_web_target: kweb://webring
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
  app: kweb://portal
  timestamp: '2026-10-12T21:31:00Z'
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
- **Current Target**: `KCyber`
- **Upcoming Queue**: `KSpace`, `KStellar`, `KColosseum`, `KMech`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KVault`
- **Upcoming Queue**: `KCalendar`, `KChart`, `KChat`, `KMine`, `KSanctuary`, `KAudio`, `KSubmarine`, `KTrader`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KAudio`
- **Upcoming Queue**: `KSubmarine`, `KTrader`, `KBudget`, `KRogue`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KBreakout`
- **Upcoming Queue**: `KRogue`, `KPong`, `KPac`, `KColony`.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KChess`
- **Upcoming Queue**: `KColosseum`, `KPong`, `KRogue`.

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

- **2026-10-12T21:15:00Z — kilo-expander: KRogue (Multi-Slot Save & Replay Expansion)**
  - Status: PASS ✅ (Vite build clean, size 319KB < 999KB).
  - Feature Expansion: Implemented multi-slot save architecture (Slots 1-3) with HUD slot switcher [Slot 1-3].
  - Cleaned up malformed script tag in header; verified F5 quicksave and F9 quickload parity.
  - Queue: Advanced kilo-expander to KChess; handed off to kilo-creator.

- **2026-10-12T20:32:00Z — kilo-qa: KColony (Pass 5: QA & Build Quality Audit)**
  - Status: PASS ✅ (Vite build clean, size 134KB < 999KB).
  - State & Tutorial: Quicksave/quickload [F5/F9] verified; first-run tutorial flag intact.
  - Controls & Modals: Verified hotkeys [0-9, H, F1, Esc, Space], responsive modal overlays.
  - Queue: Advanced kilo-qa to KBreakout; handoff to kilo-expander.

- **2026-10-12T20:14:00Z — kilo-usability: KRogue (UI/UX & Usability Maturity Audit)**
  - ⏭️ Skip — KRogue usability is mature (10+ passes, crisp HiDPI canvas, F1/H modal, responsive D-Pad).
  - Queue: Advanced kilo-usability to KAudio; handoff to kilo-qa.

- **2026-10-12T19:31:00Z — kilo-tester: KTrader (Interactive UI Element Audit & Inline Fixes)**
  - Status: PASS ✅ (Vite build clean, size 88KB < 999KB).
  - Audit: Verified modals (help, briefing, victory), export/import JSON, quicksave/quickload [F5/F9], hyperspace [1-9], combat, and refuel.
  - Inline Fixes: Added "Sell All" bulk transaction button, confirm guard to mission abandonment, and button tooltips.
  - Queue: Advanced kilo-tester to KVault; handoff to kilo-usability.

- **2026-10-12T19:15:00Z — kilo-soundscape: KSanctuary (Procedural FM Soundscape & Music Engine)**
  - Status: PASS ✅ (Vite build clean, size 462KB < 999KB).
  - Audio: Implemented authentic YM2612 2-Op FM synthesis engine with SPC700 lowpass warmth.
  - Music: Composed 32-step modal D-minor desolate shelter theme (FM lead, SPC pad, grit bass, percussion).
  - Earcons: Upgraded all UI earcons, action stingers (relic, klaxon, seismic drop), and visibility handlers.
  - Queue: Advanced kilo-soundscape to KCyber; handoff to kilo-tester.
