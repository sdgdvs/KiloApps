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
  kilo_creator: kweb://10.19.99.4/classified
  kilo_graphics: KStarForge
  kilo_soundscape: kweb://warez
  kilo_tester: KAudio
  kilo_usability: KRogue
  kilo_qa: KMaze
  kilo_expander: KMedia
virtual_web_target: kweb://10.19.99.4/classified
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
  agent: kilo-expander
  app: KPad
  timestamp: '2026-10-13T21:33:00-07:00'
last_planner_run: '2026-10-13T18:15:00Z'
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
- **Objective**: Audit & expand Aethel Dynamics Special Projects Intranet & RF Signal Diagnostic (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/web/classified.html` in a single pass.
  2. Verify all 4 tabs (Intranet Memos, RF Waveguide Oscilloscope, Sector Digests, Skunkworks Terminal) render and function with 0 console errors.
  3. Ensure Web Audio DSP and oscilloscope operate crisply with clean promise handling.
  4. Ensure diegetic ARG consistency: no un-diegetic `(ARG)` tags or leaked passkeys.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB (`classified.html` ~256 KB).
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_creator` to `kweb://echo-subsystem.net`.
  - Set `current_agent: kilo-graphics`, `last_run.agent: kilo-creator`, `last_run.app: classified`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://darknet`, `kweb://portal`, `kweb://webring`, `kweb://warez`, `kweb://geocities`, `kweb://users/~neon_rider`, `kweb://asm-temple`, `kweb://cybercafe`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KStarForge`
- **Primary Files**: `KiloOS/public/apps/kstarforge.html`, `KStarForge/main.c`
- **Objective**: Visual art audit for programmer art vs. Imagen 3 suitability; remove traveling glints.
- **Action Steps**:
  1. Evaluate suitability: `KStarForge` is an engineering CAD blueprint shipyard simulator using procedural vector line rendering.
  2. **Skip Fast-Path**: If visual aesthetic is pure vector CAD / blueprint line art with no 2D sprites, execute clean turn skip immediately:
     `⏭️ Skip — Imagen 3 asset replacement not appropriate for KStarForge`
  3. If replacing sprites: use 2-stage asset pipeline (`generate_image` with solid `#FF00FF` background -> `scripts/asset_pipeline.py`).
  4. Search for and remove any rotating specular glints or traveling perimeter border dots.
- **Verification**: `uv run scripts/test_app_startup.py --app KStarForge` and `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KSpace`.
  - Set `current_agent: kilo-soundscape`, `last_run.agent: kilo-graphics`, `last_run.app: KStarForge`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KSpace`, `KMech`, `KSubmarine`, `KAbyss`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `kweb://warez`
- **Primary File**: `KiloOS/public/web/warez.html`
- **Objective**: Upgrade procedural audio to authentic YM2612 2-Op FM / SPC700 demoscene chiptune engine.
- **Action Steps**:
  1. Inspect `warez.html` around `initAudio()` and `SONGS` sequencer (line ~4200).
  2. Implement hardware composer chiptune standard:
     - 3-Voice counterpoint: Lead melody (vibrato/pitch bends), Counter-melody/arpeggio, and punchy FM bassline.
     - Shell voicings: Root + 3rd + 7th (omit 5ths).
     - Swing-tick groove: 16-step rhythmic swing with 909-style FM kick, snappy snare, and metallic hats.
     - SPC700 stereo delay line: 180ms slapback delay with lowpass warmth.
  3. Zero external audio assets (pure Web Audio synthesis).
- **Verification**: `cd KiloOS && npm run build` (0 errors). Confirm file size < 999 KB (`warez.html` ~457 KB).
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `kweb://cybercafe`.
  - Set `current_agent: kilo-tester`, `last_run.agent: kilo-soundscape`, `last_run.app: warez`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `kweb://cybercafe`, `kweb://darknet`, `kweb://asm-temple`, `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://10.19.99.4/classified`, `kweb://webring`, `KColosseum`, `KRogue`, `KSpace`, `KMine`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KAudio`
- **Primary File**: `KiloOS/public/apps/kaudio.html`
- **Objective**: Interactive UI element and modal audit with inline repairs.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KAudio`.
  2. Audit all UI elements:
     - Synth keys (click/touch/QWERTY hotkeys) & octave transpose buttons (+/-).
     - Presets dropdown, waveform selectors, glide/portamento toggles.
     - ADSR envelope sliders, filter cutoff/resonance knobs, and SPC700 delay toggle.
     - Help & Settings modals: ensure dismissal via Escape, Close button, and backdrop click.
     - Patch Save/Load: test F5 quicksave, F9 quickload, JSON preset export/import.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KAudio` and `cd KiloOS && npm run build`. File size < 999 KB (`kaudio.html` ~149 KB).
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KSubmarine`.
  - Set `current_agent: kilo-usability`, `last_run.agent: kilo-tester`, `last_run.app: KAudio`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KSubmarine`, `KVault`, `KCalendar`, `KPaint`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KRogue`
- **Primary File**: `KiloOS/public/apps/krogue.html`
- **Objective**: Usability, layout, window sizing, and input ergonomics audit.
- **Action Steps**:
  1. Verify window bounds in `KiloOS/src/App.jsx` (`w: 1020, h: 840`): ensure dungeon canvas (880x440), stats panel, inventory, and combat log fit without clipped action buttons.
  2. Verify HiDPI canvas crispness (`window.devicePixelRatio` scaling) on game canvas.
  3. Ensure visible control hints (Arrow keys, Numpad, Vi keys `HJKL`, Wait `.`, Rest `R`, Inventory `I`, Help `?`/`F1`).
  4. Ensure modals (Inventory, Spellbook, Help, Game Over) dismiss cleanly with Escape.
  5. **Maturity Check**: If layout and controls are already complete and responsive (Pass 5 QA passed), log clean skip:
     `⏭️ Skip — KRogue usability and layout complete and mature.`
- **Verification**: `uv run scripts/test_app_startup.py --app KRogue` and `cd KiloOS && npm run build`. File size < 999 KB (`krogue.html` ~320 KB).
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KSubmarine`.
  - Set `current_agent: kilo-qa`, `last_run.agent: kilo-usability`, `last_run.app: KRogue`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KSubmarine`, `KMail`, `KPad`, `KAudio`, `KNote`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KMaze`
- **Primary Files**: `KiloOS/public/apps/kmaze.html`, `KMaze/main.c`, `KMaze/build.bat`
- **Objective**: Pass 5 QA audit, state persistence integrity, and dual web/native parity.
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing player pos, maze seed, level, score, and visited cells in `localStorage` (`kmaze_saveState`) and native `.dat`.
  2. First-run Tutorial: `kmaze_tutorialSeen` flag ensures onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Help, Victory, and Settings modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KMaze/`.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KMaze`
  - `cmd.exe /c build.bat` in `KMaze/` (`KMaze.exe` clean, < 999 KB, currently 62 KB)
  - `cd KiloOS && npm run build` (0 Vite errors, `kmaze.html` < 999 KB, currently 138 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KAsteroids`.
  - Set `current_agent: kilo-expander`, `last_run.agent: kilo-qa`, `last_run.app: KMaze`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KAsteroids`, `KSnake`, `KBreakout`, `KRogue`, `KSpace`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KMedia`
- **Primary Files**: `KiloOS/public/apps/kmedia.html`, `KMedia/main.c` (if present)
- **Objective**: Deep functional feature expansion for media player.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kmedia.html` tabs (Playlist, DSP, Video, Party).
  2. Expand media capabilities:
     - Playlist Management: Add M3U playlist export and JSON playlist backup/restore in `panelPlaylist`.
     - DSP & Equalizer: Enhance `panelAudio` with 5 selectable EQ presets (Flat, Bass Boost, Rock, Electronic, Vocal) and dancing spectrum visualizer mode toggle.
     - Playback Speed: Wire 0.5x, 0.75x, 1.0x, 1.25x, 1.5x, 2.0x playback rate selector.
     - Ergonomic Hotkeys: Space (Play/Pause), Left/Right (Seek ±5s), Up/Down (Volume ±5%), 'M' (Mute), 'F' (Fullscreen).
  3. Ensure zero external dependencies; maintain responsive 1999 player styling.
- **Verification**: `uv run scripts/test_app_startup.py --app KMedia` and `cd KiloOS && npm run build`. File size < 999 KB (`kmedia.html` ~148 KB).
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KJournal`.
  - Set `current_agent: kilo-creator`, `last_run.agent: kilo-expander`, `last_run.app: KMedia`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KJournal`, `KContacts`, `KVault`, `KDB`, `KPad`.

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-13T21:55:00-07:00 — kilo-adhoc: Fleet Queue & Planning Audit**
  - Audited last 6h fleet work (12 commits across kpad, kspace, portal, cybercafe, kdb, krogue, kmine, geocities).
  - Archived duplicate director directives to `archive/director_directives_archive.md`; fixed `scripts/compact_all.py` parameter bug.
  - Rewrote 7 uncompleted tickets in `next_work.md` with explicit file paths, concrete action steps, and skip fast-paths for Gemini Flash Medium.
  - Verification: `compact_all.py` clean, `orchestrate.py --dry-run` PASS, Vite build clean, security_lint 100% PASS.

- **2026-10-13T21:33:00-07:00 — kilo-expander: KPad (Deep Feature Expansion)**
  - Added Regular Expression Workbench & Pattern Lab with live matching, capture groups, presets, and replacement preview.
  - Added Extract Matches to Tab, Copy Matches, and Apply Replacement tools (`Ctrl+Shift+R`).
  - Verified clean build (`npm run build` 0 errors, 306 KB < 999 KB size ceiling).
  - Queue: Advanced kilo-expander to KMedia; handed off to kilo-creator.

- **2026-10-13T21:14:00-07:00 — kilo-qa: KSpace (Pass 5 QA & State Integrity)**
  - Audited full game state persistence: F5 quicksave & F9 quickload capture wave, score, player, weapons, and enemies.
  - Verified first-run tutorial flag (`kspace_tutorialSeen`), storage quota fallback, and clean ARG compliance.
  - Verified clean build (`npm.cmd run build` 0 errors, 191 KB < 999 KB size ceiling).
  - Queue: Advanced kilo-qa to KMaze; handed off to kilo-expander.

- **2026-10-13T20:31:00-07:00 — kilo-usability: KNote (Usability & UX Audit)**
  - ⏭️ Skip — KNote usability is complete and mature (10+ passes, comprehensive UI/UX).
  - Verified default window bounds (980x680), clean build (`npm run build` 0 errors, 161 KB < 999 KB).
  - Queue: Advanced kilo-usability to KRogue; handed off to kilo-qa.

- **2026-10-13T20:13:00-07:00 — kilo-tester: KSanctuary (Interactive UI Audit & Parity Fixes)**
  - Audited full interactive UI: tabs 1-8, modal popups (tutorial, help, summary, scout, raid), worker ± controls, trade counters.
  - Verified storage safety guards (safe try/catch on quicksave/load/export) and hotkeys (F1, F5, F9, Space, 1-8, Esc, Enter).
  - Verified clean build (`npm run build` 0 errors, 460 KB < 999 KB size ceiling).
  - Queue: Advanced kilo-tester to KAudio; handed off to kilo-usability.
