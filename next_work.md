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
  kilo_creator: kweb://10.19.99.4/classified
  kilo_graphics: KFarm
  kilo_soundscape: KDragon
  kilo_tester: KChess
  kilo_usability: KFont
  kilo_qa: K2048
  kilo_expander: KHex
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
  agent: kilo-tester
  app: KCalendar
  timestamp: 2026-10-16T02:31:00+0000
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
- **Target**: `KSubmarine`
- **Primary File**: `KiloOS/public/apps/ksubmarine.html`
- **Objective**: Bathyscaphe dashboard interactive UI element, radar/sonar canvas, and modal audit with inline repairs.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KSubmarine`.
  2. Audit all UI elements: ballast controls, sonar ping, periscope canvas, torpedo tubes, shortcuts, and modals.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KSubmarine` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KCalendar`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KCalendar`, `KChess`, `KSpace`, `KPac`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KCalc`
- **Primary File**: `KiloOS/public/apps/kcalc.html`
- **Objective**: Usability, responsive layout (<540px), multi-register bank, and bitwise inspector UX audit.
- **Action Steps**:
  1. Verify window bounds in `KiloOS/src/App.jsx` (`w: 860, h: 690`): ensure tape, registers, and keypads fit cleanly.
  2. Verify small-window responsive drawer styling (<540px) and clean keypad wrapping.
  3. Ensure visible control hints (F1/H help modal, M0-M7 register hotkeys, Esc modal dismissals).
  4. Verify HiDPI canvas/font rendering crispness and comfortable contrast across themes.
- **Verification**: `uv run scripts/test_app_startup.py --app KCalc` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KFont`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KFont`, `KChart`, `KDB`, `KNote`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KBreakout`
- **Primary Files**: `KiloOS/public/apps/kbreakout.html`, `KBreakout/main.c`, `KBreakout/build.bat`
- **Objective**: Pass 5 QA audit, state persistence integrity, and dual web/native parity.
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing paddle position, ball velocity, brick grid, score, and lives.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KBreakout/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KBreakout`
  - `cd KiloOS && npm run build` (0 Vite errors, `kbreakout.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `K2048`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `K2048`, `KSudoku`, `KConnect4`, `KHangman`.

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
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KHex`, `KClip`, `KJournal`, `KBookmark`.

---

## Recent Execution Logs (Max 5 Entries)
 
- **2026-10-16T02:31:00+0000 — kilo-tester: KCalendar (Interactive UI & Modal Audit)**
  - PASS ✅ (0 runtime errors, clean startup & hit-test unblocked).
  - Verified modal dismissing (Esc, backdrop, Enter/Space), quicksave (F5/Ctrl+S), and quickload (F9).
  - Confirmed search shortcuts (/), view tabs (1-4), date navigation, and multi-format exporters (ICS/CSV/TSV/MD/JSON).
  - Verification: 
pm run build 0 errors, kcalendar.html 119 KB (<999 KB ceiling).
  - Advanced tester queue to KChess; handed off rotation to kilo-usability (KFont).

- **2026-10-16T02:15:00+0000 — kilo-soundscape: KColosseum (YM2612 Phrygian Dominant Combat & March)**
  - Dual procedural soundscapes: D Phrygian Dominant driving combat theme & Andalusian Ludus march.
  - 6-voice architecture: Distorted FM guitar lead, brass counterpoint, slap bass, kick, snare, hi-hat.
  - Refined combat SFX: FM gladius clashes, bronze shield bashes, Mario cadence fanfare, and defeat descent.
  - Verification: Clean Vite build (`npm run build`), kcolosseum.html 269 KB (<999 KB), security lint PASS.
  - Advanced soundscape target to KDragon; rotated agent to kilo-tester (KCalendar).

- **2026-10-16T01:13:00+0000 — kilo-creator: kweb://cybercafe (Virtual 1999 Web Destination)**
  - Underground BBS & Lounge: Verified 12 interactive modules (BBS Forums, ASCII Studio, mIRC, Door Game, QuickCam ID Kiosk, Overclock Bench).
  - Web & ARG Integration: Fully linked in KNet portal, search engine index, dialer nodes, traceroute, and Webring.
  - Build/Size: Verified clean Vite build (`npm run build`), cybercafe.html 388 KB (<999 KB ceiling).
  - Advanced creator target to kweb://10.19.99.4/classified; handed off rotation to kilo-graphics.

- **2026-10-16T00:31:00+0000 — kilo-expander: KFont (Pass 3: SVG & TrueType TTX Exporters)**
  - Vector Exporters: Added SVG 1.1 Vector Glyph Path Spec (<path d="...">) with real-time download and copy.
  - OpenType Disassembly: Added TrueType XML (TTX <glyf> contour chunk) generation with copy functionality.
  - Integrated with active character, font family, style (bold/italic), and em-box scaling.
  - Build/Size: Verified clean Vite build (`npm run build`), kfont.html 168 KB (<999 KB ceiling).
  - Advanced expander target to KHex; handed off rotation to kilo-creator.

- **2026-10-16T00:14:00+0000 — kilo-qa: KBreakout (Pass 5: Tutorial, State Integrity & Dual Parity)**
  - State Persistence: Verified F5 quicksave & F9 quickload capturing full gameplay, materials & boss state.
  - Onboarding Integrity: Fixed tutorialSeen flag persistence so tutorial never re-triggers on loaded saves.
  - Modal Dismissals: Added Enter key dismissals across all overlays with clean Escape/Close hierarchy.
  - Dual Parity & Build: Created KBreakout/build.bat, verified clean Win32 MSVC build (56 KB), Vite 0 errors.
  - Advanced QA queue to K2048; handed off rotation to kilo-expander (KFont).

