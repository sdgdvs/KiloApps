---
current_agent: kilo-soundscape
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
  kilo_creator: kweb://portal
  kilo_graphics: KFarm
  kilo_soundscape: kweb://warez
  kilo_tester: KChess
  kilo_usability: KDB
  kilo_qa: KSudoku
  kilo_expander: KHex
virtual_web_target: kweb://portal
virtual_web_rotation:
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
- kweb://deep-core
last_run:
  agent: kilo-graphics
  app: KFarm
  timestamp: 2026-10-16T16:12:03+0000
last_planner_run: '2026-10-15T20:17:00Z'
---

# KiloApps Master Fleet Work & Queue State

This document is the single active source of truth for autonomous agent dispatching.
The Windows Task Scheduler orchestrator (`scripts/orchestrate.py`) parses the YAML frontmatter above on every tick to dispatch the active skill.

## Fleet Core Directives (Quick Reference)

> Full architectural rules codified in [.agents/AGENTS.md](.agents/AGENTS.md) and [.agents/rules/](.agents/rules/).
> Full historical director directives codified in [archive/director_directives_archive.md](archive/director_directives_archive.md).

1. **Single-App-Per-Turn**: Exactly ONE target application per turn. Inspect once, batch edits, verify builds, advance queue, commit, and push.
2. **Strict Size Ceiling**: Every web file (`.html`) and native executable (`.exe`) must remain strictly **< 999 KB**.
3. **Token Conservation (CRITICAL)**: ≤6 lines of terse bullet points in execution log. Never re-read files after editing. Max 5 active log entries in this file (older entries rotated by `scripts/rotate_logs.py`).
4. **Multiplayer Autostart Prohibition**: Games must NEVER autostart into matchmaking. Default to local offline play; multiplayer requires explicit button click. Include 25s solo AI fallback.
5. **Anti-Trademark / Parody Rule**: No real commercial marks or scene group names (use *Surreal Tournament*, *VoidCraft*, *RAZOR 1999*, *FLARELIGHT*). Must pass `python scripts/security_lint.py`.
6. **ARG Mystery Preservation (TINAG)**: Never label UI or copy with `(ARG)` or walkthrough guides; never leak `ECHO-1999-ARCHITECT` before App #100 (`KMatrix`).
7. **Perimeter Glint & Comet Ban**: Remove traveling border dots, orbiting balls, and specular glints.
8. **Chiptune Audio Architecture**: Genesis YM2612 2-Op FM / SNES SPC700 stereo delay standard. Zero external audio assets.

---

## Active Target Queues (Gemini Flash Optimized Tickets)

### 1. Virtual 1999 Web & ARG Node Creator (`kilo-creator`)
- **Target**: `kweb://10.19.99.4/classified`
- **Primary File**: `KiloOS/public/web/classified.html`
- **Objective**: Virtual 1999 classifieds and bulletin board destination (<999 KB).
- **Upcoming Queue**: `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://darknet`, `kweb://portal`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KFarm`
- **Primary Files**: `KiloOS/public/apps/kfarm.html`, `KFarm/main.c` (if present)
- **Objective**: Replace 2D tile canvas programmer art (primitive rect fills for crops, terrain, and animals) with Imagen 3 generated sprite assets & tiles; ensure 0 specular glints.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kfarm.html` canvas rendering routines for programmer art.
  2. Generate crop/terrain/animal tiles via 2-stage asset pipeline (`generate_image` with solid `#FF00FF` -> `scripts/asset_pipeline.py`).
  3. Search for and eliminate any traveling border dots or rotating specular glints.
  4. Run `python scripts/check_icons.py` to audit icon uniqueness.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KWizard`.
  - Set `current_agent: kilo-soundscape`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KWizard`, `KDragon`, `KAlchemy`, `KFortress`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `KDragon`
- **Primary File**: `KiloOS/public/apps/kdragon.html`
- **Objective**: Authentic YM2612 2-Op FM / SPC700 medieval fantasy dungeon chiptune audio suite.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kdragon.html` audio synthesis functions.
  2. Implement hardware composer chiptune standard:
     - 3-Voice counterpoint: Lead melody, counter-melody/arpeggio, and punchy FM bassline.
     - Shell voicings: Root + 3rd + 7th (omit 5ths).
     - Swing-tick groove: 16-step rhythmic swing.
     - SPC700 warm stereo slapback delay line (180ms with lowpass filtering).
  3. Pure Web Audio synthesis with zero external audio assets.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `KDragon`.
  - Set `current_agent: kilo-tester`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KDragon`, `KWizard`, `KFortress`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KChess`
- **Primary File**: `KiloOS/public/apps/kchess.html`
- **Objective**: Chess board interactive UI element, move highlight canvas, and modal audit with inline repairs.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KChess`.
  2. Audit all UI elements: piece drag/click, timer clocks, move notation log, undo/redo, shortcuts, and modals.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KChess` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KChess`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KChess`, `KSpace`, `KPac`, `KCalendar`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KChart`
- **Primary File**: `KiloOS/public/apps/kchart.html`
- **Objective**: Chart and graph visualizer layout responsiveness, touch interactions, and legend usability.
- **Action Steps**:
  1. Verify window bounds and responsive container wrapping under narrow (<540px) widths.
  2. Ensure clear keyboard shortcuts (F1/H help, Esc dialog dismissals) and accessible focus indicators.
  3. Verify HiDPI canvas scaling and comfortable theme contrast.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KDB`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KChart`, `KDB`, `KNote`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KSudoku`
- **Primary Files**: `KiloOS/public/apps/ksudoku.html`, `KSudoku/main.c`, `KSudoku/build.bat`
- **Objective**: Pass 5 QA audit, state persistence integrity, and dual web/native parity.
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KSudoku/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KSudoku`
  - `cd KiloOS && npm run build` (0 Vite errors, `ksudoku.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KSudoku`.
  - Set `current_agent: kilo-graphics`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KSudoku`, `KConnect4`, `KHangman`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KFont`
- **Primary Files**: `KiloOS/public/apps/kfont.html`, `KFont/main.c` (if present)
- **Objective**: Deep functional feature expansion for bitmap font editor.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kfont.html`.
  2. Expand font editor capabilities:
     - BDF / FON / TTF Export & Import: Add standardized bitmap font format exporters.
     - Extended Character Grid: 256-character ASCII/CP437 matrix with individual glyph editing.
     - Live Typography Previewer: Real-time text preview box with kerning, leading, and scale controls.
  3. Zero external dependencies; maintain responsive 1999 retro styling.
- **Verification**: `uv run scripts/test_app_startup.py --app KFont` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KHex`.
  - Set `current_agent: kilo-graphics`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KHex`, `KClip`, `KJournal`, `KBookmark`.

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-16T16:25:00+0000 — kilo-adhoc: KiloOS Kernel v2.0 (Singleton Audio, Persistent VFS, IPC v2.0)**
  - Status: PASS ✅ (0 regressions, clean builds, 254 KB bundle < 999 KB ceiling).
  - Web Audio Singleton: Replaced per-click/startup `new AudioContext()` leak with persistent singleton audio synthesizer.
  - Persistent VFS: Auto-hydrates from `kiloos_vfs_state_v2` with 400ms debounced auto-save so user sessions survive page reloads.
  - IPC v2.0 Switchboard: Extended Window postMessage bridge with `VFS_DELETE`, `VFS_STAT`, live `VFS_CHANGED` broadcast, `OS_NOTIFY`, and `OS_TELEMETRY_BEAT`.
  - Enriched Telemetry: Added FPS and memory metrics to `OS_LIST_TASKS` and `openApp` state tracking for KTaskMgr and KDirector.
  - Verification: Vite build clean (0 errors, 308ms); `security_lint.py` 100% PASS; `byte_budget_gate.py` 100% PASS.

- **2026-10-16T16:18:00+0000 — kilo-adhoc: @kiloapps/sdk v2.0 (ZzFX 20-Param Synth, Gamepad/Touch & VFS Sync)**
  - Status: PASS ✅ (0 regressions, clean builds, 34 KB < 999 KB ceiling).
  - Procedural Audio: full ZzFX 20-param synthesis engine + bandpass noise; 100% backward compatible with `playTone` and `sfx`.
  - Input Controller: real HTML5 Gamepad API polling, multi-button Virtual Touch Pad (D-Pad + A/B), frame-accurate `isDown`/`justPressed`.
  - VFS Sync & Persistence: boot hydration from KiloOS VFS_READ, out-of-the-box F5 Quicksave / F9 Quickload, and live 60FPS telemetry stream.
  - Verification: 100% unit tests pass (`node packages/sdk/test.js`); Vite build clean; security_lint PASS.

- **2026-10-16T16:12:03+0000 — kilo-graphics: KFarm (Zero-Token Auto-Skip — Inappropriate Target)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KFarm (pure vector, board game, or mature art).
  - Optimization: Handled via orchestrator pre-flight zero-token auto-skip.
  - Queue: Advanced `kilo-graphics` to `KFarm`; rotation handoff to `kilo-soundscape`.

- **2026-10-16T14:32:00+0000 — kilo-qa: KHangman (Pass 5 QA & State Integrity)**
  - Hardened quicksave (F5) / quickload (F9) to restore category, custom word lists, and reset duel state safely.
  - Enforced first-run tutorial persistence flags (`khangman_tutorialSeen`), preventing prompts on restored save states.
  - Fixed modal dismissals: Esc/Enter/Space cleanly dismiss active overlays without triggering unwanted game inputs.
  - Verified clean native build (`KHangman.exe`, 36 KB) and web build (`npm run build`, 0 errors, 147 KB < 999 KB).
  - Advanced QA target to `KSudoku`; handoff to `kilo-expander`.

- **2026-10-16T14:15:00+0000 — kilo-usability: KChart (UI/UX & Usability Pass)**
  - Enhanced responsive layout: added narrow viewport (<640px/<440px) media queries for header, workspace, toolbar, and stats grid.
  - Improved legend usability: made pie, donut, and polar chart legends dynamically sized and repositioned to prevent canvas clipping.
  - Added direct legend interactive hover hit-testing and pointerup touch events for instant slice highlighting.
  - Verified HiDPI canvas scaling, accessible focus indicators (:focus-visible), and Esc modal/input dismissal.
  - Verified `npm run build` (0 Vite errors, `kchart.html` 126 KB < 999 KB); advanced usability target to `KDB`; handoff to `kilo-qa`.
