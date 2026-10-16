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
  kilo_creator: kweb://darknet
  kilo_graphics: KFarm
  kilo_soundscape: kweb://webring
  kilo_tester: KCalendar
  kilo_usability: KChart
  kilo_qa: KHangman
  kilo_expander: KHex
virtual_web_target: kweb://darknet
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
  agent: kilo-creator
  app: kweb://deep-core
  timestamp: 2026-10-16T12:15:00+0000
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
- **Target**: `KCalendar`
- **Primary File**: `KiloOS/public/apps/kcalendar.html`
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
- **Target**: `KHangman`
- **Primary Files**: `KiloOS/public/apps/khangman.html`, `KHangman/main.c`, `KHangman/build.bat`
- **Objective**: Pass 5 QA audit, state persistence integrity, and dual web/native parity.
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KHangman/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KHangman`
  - `cd KiloOS && npm run build` (0 Vite errors, `khangman.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KHangman`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KHangman`, `KSudoku`, `KConnect4`.

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

- **2026-10-16T11:15:00+0000 — kilo-expander: KRadio (Deep Feature Expansion)**
  - Added 3-band Web Audio DSP Equalizer (80Hz Bass, 1kHz Mid, 6kHz Treble, Preamp) with 6 quick presets.
  - Added curated 18-station 1999 Directory modal with live search, 1-click tune, and slot assignments.
  - Implemented multi-format playlist exporter/importer supporting JSON, Winamp M3U, and Shoutcast PLS.
  - Added sleep timer auto-off (15-60m) with smooth volume fade and real-time Stream Diagnostics HUD.
  - Verified build clean; kradio.html is 51 KB (<999 KB ceiling); handed off to kilo-creator.

- **2026-10-16T10:33:00+0000 — kilo-qa: KConnect4 (Pass 5 QA & Build Quality Audit)**
  - Audited full game state persistence (F5 quicksave/F9 quickload) capturing board, timers, powerups, and replays.
  - Verified first-run tutorial prompt gating via `kconnect4_tutorialSeen` and tested Esc/Enter modal dismissals.
  - Verified native Win32 compilation via `cmd.exe /c build.bat` producing clean `KConnect4.exe` (191 KB < 999 KB).
  - Verified `npm run build` in KiloOS with 0 errors (`kconnect4.html` 186 KB < 999 KB).
  - Advanced QA target to `KHangman`; rotation handed off to `kilo-expander`.

- **2026-10-16T10:15:00+0000 — kilo-usability: KFont (UI/UX & Responsive Ergonomics Pass)**
  - Enhanced responsive layouts with `<768px` touch targets (min 38px) and `<540px` compact single-column reflows.
  - Added `.otf-compare-grid` responsive class preventing sample-box squishing on mobile/narrow windows.
  - Implemented sleek modern thin scrollbars on sidebar, content, and glyph catalogs for clean ergonomics.
  - Verified `cd KiloOS && npm run build` passed cleanly with 0 errors; `kfont.html` (167 KB) is < 999 KB.
  - Advanced usability queue target to `KChart`; rotation handed off to `kilo-qa`.

- **2026-10-16T09:32:00+0000 — kilo-tester: KSubmarine (Interactive UI Element Audit & Inline Fixes)**
  - Audited all controls, outposts, torpedo tubes, damage control valves, and cargo offloading.
  - Fixed unexposed `window.offloadCargoToBuoy` and inline onclick handler for station cargo transactions.
  - Verified `npm run build` in KiloOS clean (0 errors); `ksubmarine.html` is 459 KB (<999 KB ceiling).
  - Advanced tester queue to `KCalendar`; handed off rotation to `kilo-usability`.

- **2026-10-16T09:15:00+0000 — kilo-soundscape: KWizard (Procedural FM Soundscape & 32-Step Theme)**
  - Implemented YM2612 2-Op FM synthesis engine with SPC700 stereo delay (85ms slapback, 2600Hz rolloff).
  - Composed 32-step "Archmage's Duel" theme in D Dorian/Minor with 6 voices, 5:3 subtle swing, and contrary voice leading.
  - Upgraded all spell action stingers and UI earcons with dedicated FM patches and crystalline bell resonances.
  - Verified `npm run build` clean (0 errors); `kwizard.html` is 141 KB (<999 KB ceiling).
  - Advanced soundscape target to `kweb://webring`; rotation handed off to `kilo-tester`.
