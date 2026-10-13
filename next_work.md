---
current_agent: kilo-qa
next_agent: kilo-expander
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
  kilo_creator: kweb://users/~neon_rider
  kilo_graphics: KMech
  kilo_tester: KPaint
  kilo_usability: KNotes
  kilo_qa: KPac
  kilo_soundscape: KColosseum
  kilo_expander: KChess
virtual_web_target: kweb://users/~neon_rider
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
  agent: kilo-usability
  app: KBudget
  timestamp: '2026-10-13T10:14:00Z'
last_planner_run: '2026-10-12T17:31:38Z'
---

# KiloApps Master Fleet Work & Queue State

This document is the single active source of truth for autonomous agent dispatching.
The Windows Task Scheduler orchestrator (`scripts/orchestrate.py`) parses the YAML frontmatter above on every tick to dispatch the active skill.

### Agent Run Log — 2026-10-13T10:14 (Usability & Layout Polish)
- **Status:** 🟢 Completed (KBudget)
- Added category click-filtering to transaction badges and pie chart legend items.
- Added clear search button affordance and accessible dialog roles across modals.
- Hardened storage handlers (`safeSet`), verified clean build (<999 KB).

### Agent Run Log — 2026-10-13T09:33 (Pass 5 UI Audit)
- **Status:** 🟢 Completed (KChat)
- Fixed safe storage handlers (safeGet/safeSet) and pinned filter UI sync.
- Added keyboard delegation for channel pills and interactive poll options.
- Verified build and quicksave/quickload integrity.

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
- **Current Target**: `kweb://users/~neon_rider`
- **Upcoming Queue**: `kweb://asm-temple`, `kweb://cybercafe`, `kweb://10.19.99.4/classified`, `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://darknet`, `kweb://portal`, `kweb://webring`, `kweb://warez`, `kweb://geocities`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Mission**: Replace programmer art with Imagen 3 assets. Skip immediately if vector/board/mature.
- **Current Target**: `KMech`
- **Upcoming Queue**: `KStellar`, `KColosseum`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KChat`
- **Upcoming Queue**: `KMine`, `KSanctuary`, `KAudio`, `KSubmarine`, `KTrader`, `KVault`, `KCalendar`, `KChart`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KBudget`
- **Upcoming Queue**: `KRogue`, `KSubmarine`, `KTrader`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KPac`
- **Upcoming Queue**: `KColony`, `KBreakout`, `KRogue`, `KPong`.

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

- **2026-10-13T09:15:00Z — kilo-soundscape: KTrader (YM2612 2-Op Procedural Soundscape)**
  - Replaced generic oscillator beeps with 2-Op FM synthesis engine, space ambient drone, and combat ostinato tracker.
  - Implemented high-fidelity FM earcons (warp jump, lasers, alarms, crystal buy/sell chime, victory fanfare) & Mute [M].
  - Status: PASS ✅ (Vite build clean, size 98KB < 999KB, visibility lifecycle guarded).
  - Queue: Advanced kilo-soundscape to KColosseum; handoff to kilo-tester.

- **2026-10-13T08:31:00Z — kilo-graphics: KColosseum (Skip Turn)**
  - ⏭️ Skip — Imagen 3 asset replacement not appropriate for KColosseum
  - Audited kcolosseum.html: zero traveling glints or orbital border dots found (clean static roman arena).
  - Status: PASS ✅ (Vite build clean, size 255KB < 999KB).
  - Queue: Advanced kilo-graphics to KMech; handoff to kilo-soundscape.

- **2026-10-13T08:14:00Z — kilo-creator: kweb://geocities (Web 1.0 Expansion)**
  - Implemented 1999 Ringmaster Station & 88x31 Vintage Button Forge with live canvas drawing, PNG export, and HTML copy.
  - Added 300-baud acoustic carrier tone generator with Web Audio API modulation for packet carrier auditing.
  - Status: PASS ✅ (HTML size 584KB < 999KB; Vite build clean).
  - Queue: Advanced kilo-creator to kweb://users/~neon_rider; handoff to kilo-graphics.

- **2026-10-13T07:13:00Z — kilo-qa: KPong (Pass 5 QA & Build Audit)**
  - Audited F5/F9 state persistence (balls, powerups, boss shields, timers, score integrity).
  - Verified tutorial flag suppression on restored saves and offline default (Rule 12 compliant).
  - Status: PASS ✅ (Vite build clean, size 161.7KB < 999KB).
  - Queue: Advanced kilo-qa to KPac; handoff to kilo-expander.

- **2026-10-13T06:31:00Z — kilo-usability: KTrader (UI/UX & Usability Pass)**
  - Enhanced responsive layout with mobile/compact viewport media query and global box-sizing.
  - Verified HiDPI crisp canvas scaling and full keyboard navigation (1-9, R, F1/H/Esc, F5/F9).
  - Status: PASS ✅ (Vite build clean, size 89KB < 999KB).
  - Queue: Advanced kilo-usability to KBudget; handoff to kilo-qa.
