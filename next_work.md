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
  kilo_creator: "kweb://cybercafe"
  kilo_graphics: KMech
  kilo_tester: KBudget
  kilo_usability: KChat
  kilo_qa: KBudget
  kilo_expander: KChess
virtual_web_target: "kweb://cybercafe"
virtual_web_rotation:
  - "kweb://asm-temple"
  - "kweb://cybercafe"
  - "kweb://10.19.99.4/classified"
  - "kweb://echo-subsystem.net"
  - "kweb://deep-core"
  - "kweb://darknet"
  - "kweb://portal"
  - "kweb://webring"
  - "kweb://warez"
  - "kweb://geocities"
  - "kweb://users/~neon_rider"
last_run:
  agent: kilo-creator
  app: "kweb://asm-temple"
  timestamp: "2026-10-09T09:13:00-07:00"
last_planner_run: "2026-10-09T15:32:00Z"
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
- **Current Target**: `kweb://cybercafe`
- **Upcoming Queue**: `kweb://10.19.99.4/classified`, `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://darknet`, `kweb://portal`, `kweb://webring`, `kweb://warez`, `kweb://geocities`, `kweb://users/~neon_rider`, `kweb://asm-temple`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Mission**: Replace programmer art with Imagen 3 assets. Skip immediately if vector/board/mature.
- **Current Target**: `KMech`
- **Upcoming Queue**: `KStellar`, `KStarship`, `KSubmarine`, `KSpace`, `KQuest`, `KColony`, `KColosseum`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KBudget`
- **Upcoming Queue**: `KCalendar`, `KChart`, `KChat`, `KColony`, `KFortress`, `KNetMap`, `KPing`, `KSanctuary`, `KAudio`, `KStellar`, `KSubmarine`, `KTrader`, `KType`, `KVault`, `KVoid`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KChat`
- **Upcoming Queue**: `KMaze`, `KPing`, `KNetMap`, `KStarForge`, `KPong`, `KCalendar`, `KSnake`, `KAudio`, `KBudget`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KBudget`
- **Upcoming Queue**: `KAlchemy`, `KColony`, `KStarForge`, `KBreakout`, `KRogue`, `KPong`, `KPac`, `KBBS`.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KChess`
- **Upcoming Queue**: `KMine`, `KPac`, `KStarForge`, `KColosseum`, `KAbyss`, `KPong`, `KRogue`.

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

- **2026-10-09T09:13:00-07:00 — kilo-creator: kweb://asm-temple (Virtual 1999 Web Audit & Expansion)**
  - Status: PASS ✅ (`kweb://asm-temple` Anti-Potemkin Web 1.0 audit & deep expansion verified)
  - Features: Audited x86 Opcode Temple, PE32 dissector, Mode 13h VGA canvas, YM2612 audio engine, and assembler.
  - Links & Sizing: Verified 285KB (<999KB), fully registered in KNet, portal.html, and webring.html.
  - Build: Production build clean (`npm run build` 0 errors); advanced queue to `kweb://cybercafe`; handoff to `kilo-graphics`.

- **2026-10-09T08:15:00-07:00 — kilo-tester: KCards / KSolitaire (Interactive UI & Hotkeys Audit)**
  - Status: PASS ✅ (`KCards`/`KSolitaire` UI audit clean, 0 JS errors)
  - Audit: Tested modals, shortcuts (F1, F3-F6, F9, Esc, Enter), safe storage, stats I/O.
  - Verification: Automated startup test passed: CSS valid, 0 JS err, canvas hit-test unblocked.
  - Verification: Production build clean (`npm run build` 0 errors); advanced `kilo_tester` to `KBudget`.

- **2026-10-09T07:31:00-07:00 — kilo-graphics: K2048 (Skip Turn — Inappropriate Target)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for K2048
  - Audit: Inspected K2048: abstract sliding tile puzzle using classic CSS/geometric styling; no glints or perimeter dots found.
  - Queue: Rotated `kilo_graphics` target to `KMech`; advanced turn to `kilo-tester`.

- **2026-10-09T07:13:00-07:00 — kilo-creator: kweb://users/~neon_rider (Anti-Potemkin Web 1.0 Expansion)**
  - Status: PASS ✅ (`kweb://users/~neon_rider` Anti-Potemkin verification & deep expansion)
  - Audit: Audited interactive x86 sandbox, Mode 13h VGA canvas, YM2612 tracker, 8x8 font studio, and dead-drop.
  - Verification: Confirmed strict size ceiling (<999KB: 256KB total) and clean production build.
  - Queue: Advanced queue: `virtual_web_target` to `kweb://asm-temple`; handoff to `kilo-graphics`.

- **2026-10-09T06:34:00-07:00 — kilo-expander: KBreakout (Feature Expansion)**
  - Status: PASS ✅ (`KBreakout` feature expansion completed)
  - Features: Added Board Preset Architect (5 tactical 6x10 configurations & direct preset loader) and Board Matrix I/O.
  - Telemetry: Implemented Tactical Mission Telemetry & Stats modal (`[T]`) tracking hits, combo streaks, and harvests.
  - Verification: Production build clean (`npm run build` 0 errors); advanced `kilo_expander` to `KChess`.
