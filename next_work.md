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
  kilo_creator: kweb://10.19.99.4/classified
  kilo_graphics: KDragon
  kilo_tester: KTrader
  kilo_usability: KRogue
  kilo_qa: KRogue
  kilo_expander: KMine
virtual_web_target: kweb://10.19.99.4/classified
virtual_web_rotation:
- kweb://10.19.99.4/classified
- kweb://echo-subsystem.net
- kweb://deep-core
- kweb://darknet
- kweb://portal
- kweb://webring
- kweb://warez
- kweb://geocities
- kweb://users/~neon_rider
- kweb://asm-temple
- kweb://cybercafe
last_run:
  agent: kilo-qa
  app: KBreakout
  timestamp: 2026-10-10T19:15:00-07:00
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
- **Current Target**: `kweb://10.19.99.4/classified`
- **Upcoming Queue**: `kweb://10.19.99.4/classified`, `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://darknet`, `kweb://portal`, `kweb://webring`, `kweb://warez`, `kweb://geocities`, `kweb://users/~neon_rider`, `kweb://asm-temple`, `kweb://cybercafe`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Mission**: Replace programmer art with Imagen 3 assets. Skip immediately if vector/board/mature.
- **Current Target**: `KDragon`
- **Upcoming Queue**: `KWizard`, `KFarm`, `KMatch3`, `KQuest`, `KSpace`, `KColosseum`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KTrader`
- **Upcoming Queue**: `KType`, `KVault`, `KVoid`, `KCalendar`, `KChart`, `KChat`, `KMine`, `KPing`, `KSanctuary`, `KAudio`, `KSubmarine`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KPing`
- **Upcoming Queue**: `KNetMap`, `KRogue`, `KCalendar`, `KSnake`, `KAudio`, `KBudget`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KRogue`
- **Upcoming Queue**: `KPong`, `KPac`, `KBBS`, `KBudget`, `KAlchemy`, `KColony`, `KBreakout`.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KMine`
- **Upcoming Queue**: `KPac`, `KStarForge`, `KColosseum`, `KAbyss`, `KPong`, `KRogue`, `KChess`.

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

- **2026-10-10T19:15:00-07:00 — kilo-qa: KBreakout (Pass 5: State & Tutorial Integrity Audit)**
  - Status: PASS ✅ (Quicksave F5/F9 state persistence, first-run tutorial flag, modal keybinds audited).
  - Verification: Full state persistence, storage safety guards, non-autostart duel lobby verified.
  - Build & Size: Vite build clean (`npm run build`); HTML 160 KB << 999 KB ceiling.
  - Queue: Advanced kilo-qa target to KRogue; rotation handoff to kilo-expander.

- **2026-10-10T19:04:00-07:00 — kilo-usability: KNetMap (HiDPI Scaling & Transform Resilience Audit)**
  - Status: PASS ✅ (Explicit setTransform HiDPI scaling, crisp canvas rendering, modal/F1 hotkeys verified).
  - Usability: Guarded resizeCanvas against transform drift, validated 1080x720 window dimensions and help modal accessibility.
  - Build & Size: Vite build clean (`npm run build`); HTML 110 KB << 999 KB ceiling.
  - Queue: Advanced kilo-usability target to KRogue; rotation handoff to kilo-qa.

- **2026-10-10T18:50:00-07:00 — kilo-tester: KSubmarine (Interactive UI & Storage Resilience Audit)**
  - Status: PASS ✅ (UI controls, shortcuts, modals audited; localStorage safe helpers wrapped).
  - Verification: Checked 30+ buttons/toggles, keydown shortcuts, CRT themes, and quicksave/load.
  - Build & Size: Vite build clean (`npm run build`); 458 KB << 999 KB ceiling.
  - Queue: Advanced kilo-tester target to KTrader; rotation handoff to kilo-usability.

- **2026-10-11T01:38:02+0000 — kilo-graphics: KDragon (Zero-Token Auto-Skip — Inappropriate Target)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KDragon (pure vector, board game, or mature art).
  - Optimization: Handled via orchestrator pre-flight zero-token auto-skip.
  - Queue: Advanced `kilo-graphics` to `KDragon`; rotation handoff to `kilo-tester`.

- **2026-10-10T18:32:00-07:00 — kilo-creator: kweb://cybercafe (Virtual 1999 Web Experience & KNet Links)**
  - Status: PASS ✅ (Anti-Potemkin Web 1.0 hub: threaded BBS, ASCII studio, mIRC lounge, LAN arena, 355 KB < 999 KB).
  - Integration: Verified active bidirectional linking across KNet, portal, webring, and classifieds directory.
  - Verification: Vite build clean (`npm run build`); zero console errors; security lint compliant.
  - Queue: Advanced kilo-creator target to `kweb://10.19.99.4/classified`; rotation handoff to kilo-graphics.

