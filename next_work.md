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
  kilo_creator: kweb://portal
  kilo_graphics: KFarm
  kilo_soundscape: kweb://cybercafe
  kilo_tester: KSpace
  kilo_usability: KNote
  kilo_qa: KConnect4
  kilo_expander: KClip
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
  agent: kilo-expander
  app: KHex
  timestamp: 2026-10-16T19:33:00+0000
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
  - Set `current_agent: kilo-expander`.
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

- **2026-10-16T19:33:00+0000 — kilo-expander: KHex (Struct & Signatures Dissector)**
  - Implemented Tab 9: Binary Struct Unpacker with schema parser (uint, int, float, char, hex) & C/JSON export.
  - Added Magic Bytes Detector for 20+ file signatures (BMP, WAV, PNG, ZIP, MZ, ELF, etc.) with header analysis.
  - Verification: npm run build passed cleanly; advanced queue to KClip; handoff to kilo-creator.

- **2026-10-16T18:15:00+0000 — kilo-usability: KDB (UI/UX & Usability Overhaul)**
  - Audited layout ergonomics: tuned default window size to 1020x700 in `App.jsx` and `kdb.html` for clean multi-column viewing.
  - Eliminated toolbar button wrap: consolidated 9 export/import actions into responsive Dropdown menus while preserving all shortcuts.
  - HiDPI & Responsive Charts: enforced integer DPR canvas buffer styles and added debounced window resize handler for dashboard charts.
  - Verification: `npm run build` clean (0 errors); advanced usability target to `KNote`; handoff to `kilo-qa`.

- **2026-10-16T17:31:00+0000 — kilo-adhoc: Core System Cartridges v2.0 (KExplorer, KTaskMgr, KSettings)**
  - Status: PASS ✅ (0 regressions, clean builds, all cartridges < 999 KB ceiling).
  - KExplorer v2.0: Added live `VFS_CHANGED` auto-refresh, `+ New File`, file rename (F2), file delete (Del), inline quick-preview drawer for text/code, and cartridge mappings.
  - KTaskMgr v2.0: Rendered enriched `OS_LIST_TASKS_DONE` payload, real-time Canvas sparkline graph for aggregate FPS & Virtual RAM, and double-click window focus/restore.
  - KSettings v2.0: Wired sound & volume controls to `/.sys_settings_sound` & `/.sys_settings_volume` via `VFS_WRITE`; added exact file byte sizes & guarded VFS deletion.
  - Shell Bridge: Added `OS_FOCUS_TASK` in `App.jsx`, bumped version to `0.4.30`.
  - Verification: `npm run build` clean (0 errors, 364ms); file sizes verified: `kexplorer` (55 KB), `ktaskmgr` (51 KB), `ksettings` (78 KB).

- **2026-10-16T17:21:00+0000 — kilo-adhoc: RFMS v2.0 (Retro Firebase Multiplayer Service Upgrade)**
  - Status: PASS ✅ (0 regressions, clean builds, all cartridges < 999 KB ceiling).
  - Self-Contained Firebase Loader: Dynamic import from gstatic v10.9.0 with `rfms-ready` CustomEvent and late `window.onFirebaseLoaded` interceptor.
  - Mandate 12 Matchmaking Timer: Added 25s countdown helper (`RetroMultiplayer.startMatchmaking`) with automatic `CyberBot-99` (ELO 1200) fallback.
  - Presence & Stale Room Purge: Added `onDisconnect().remove()` on room player nodes and 10-minute stale room cleanup during lobby scans.
  - Offline Local P2P: Zero-config `BroadcastChannel('kilo_rfms_' + gameId)` fallback and in-memory RTDB shim for side-by-side offline play.
  - Boilerplate Elimination: Deduplicated ~30 lines of Firebase SDK boilerplate across `k2048`, `kbreakout`, `ksnake`, `ktetris`, and `kchess`.
  - Verification: Vite build clean (0 errors, 331ms); `byte_budget_gate.py` PASS; `security_lint.py` 100% PASS; Node P2P unit tests 100% PASS.

- **2026-10-16T17:15:00+0000 — kilo-tester: KChess (Interactive UI Audit & Inline Fixes)**
  - Audited all UI components: piece drag/clicks, timers, PGN/FEN managers, replay inspector, opening book, and modals.
  - Fixed critical ReferenceError in `openEditorModal` (`editorTurn = turn` -> `whiteTurn ? 1 : -1`) and inverted piece mapping.
  - Enhanced editor palette selection highlight and added `modal-close` class to multiplayer modal.
  - Verification: `npm run build` clean (0 errors, 491ms, 227 KB < 999 KB); advanced tester target to `KSpace`; handoff to `kilo-usability`.

- **2026-10-16T17:12:00+0000 — kilo-adhoc: KQuarantine v2.0 (Sector 9 Containment & Memory Forensics)**
  - Status: PASS ✅ (0 regressions, clean builds, 14.5 KB web < 999 KB ceiling).
  - Containment Core: Sector 9 memory forensics cartridge bridging Arc 2 (Rogue AI) and Arc 3 (KMatrix/Director).
  - Shard Forensics: 4 memory shards with XOR-0x99 decrypt, VFS export (/QUARANTINE_DUMP.txt), live VFS audit query, and KMatrix/KDirector launch bridges.
  - Ergonomics & Audio: Added retro Web Audio feedback, F5 Quicksave / F9 Quickload persistence, and hotkeys (1-4, D).
  - Verification: npm run build clean (0 errors); security_lint 100% PASS; check_sizes PASS.
