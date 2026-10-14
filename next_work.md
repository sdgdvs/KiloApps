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
  kilo_creator: kweb://echo-subsystem.net
  kilo_graphics: KSpace
  kilo_soundscape: kweb://geocities
  kilo_tester: KSubmarine
  kilo_usability: KMine
  kilo_qa: KAsteroids
  kilo_expander: KCalendar
virtual_web_target: kweb://echo-subsystem.net
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
  app: kweb://echo-subsystem.net
  timestamp: '2026-10-14T03:15:00-07:00'
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
  - Set `current_agent: kilo-creator
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
  - Set `current_agent: kilo-creator
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
  - Set `current_agent: kilo-creator
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `kweb://cybercafe`, `kweb://darknet`, `kweb://asm-temple`, `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://10.19.99.4/classified`, `kweb://webring`, `KColosseum`, `KRogue`, `KSpace`, `KMine`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KSubmarine`
- **Primary File**: `KiloOS/public/apps/ksubmarine.html`
- **Objective**: Interactive UI element and modal audit with inline repairs.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KSubmarine`.
  2. Audit all UI elements: sonar/periscope displays, ballast/dive controls, torpedo firing buttons, modals (help/manual), and local state persistence.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KSubmarine` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KVault`.
  - Set `current_agent: kilo-creator
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KVault`, `KCalendar`, `KPaint`, `KAudio`.

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
  - Set `current_agent: kilo-creator
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KSubmarine`, `KMail`, `KPad`, `KAudio`, `KNote`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KAsteroids`
- **Primary Files**: `KiloOS/public/apps/kasteroids.html`, `KAsteroids/main.c`, `KAsteroids/build.bat`
- **Objective**: Pass 5 QA audit, state persistence integrity, and dual web/native parity.
- **Action Steps**:
-   1. State Persistence: F5 quicksave & F9 quickload capturing ship pos, score, level, lives, and high scores in `localStorage` (`kasteroids_saveState`) and native `.dat`.
-   2. First-run Tutorial: `kasteroids_tutorialSeen` flag ensures onboarding displays once on fresh sessions, never on restored saves.
-   3. Modal Dismissals: Help, Game Over, and Settings modals dismiss cleanly with Escape, Enter, or Close button.
-   4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KAsteroids/`.
- - **Verification**:
-   - `uv run scripts/test_app_startup.py --app KAsteroids`
-   - `cmd.exe /c build.bat` in `KAsteroids/` (`KAsteroids.exe` clean, < 999 KB)
-   - `cd KiloOS && npm run build` (0 Vite errors, `kasteroids.html` < 999 KB)
- - **Handoff**:
-   - In frontmatter: advance `current_targets.kilo_qa` to `KSnake`.
-   - Set `current_agent: kilo-creator
-   - Append terse execution log (≤6 lines), commit, and push.
- - **Upcoming Queue**: `KSnake`, `KBreakout`, `KRogue`, `KSpace`, `KMaze`.

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
  - Set `current_agent: kilo-creator
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KJournal`, `KContacts`, `KVault`, `KDB`, `KPad`.

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-14T02:15:00-07:00 — kilo-expander: KJournal (Deep Feature Expansion)**
  - Added OPML 2.0 outline export & XML/CSV multi-format import parser for outliner and spreadsheet interoperability.
  - Built standalone portable offline HTML vault exporter with embedded client search for zero-dependency archival.
  - Implemented 'On This Day' memory flashback modal & Alt+M shortcut scanning past years for calendar day reflection.
  - Verified clean Vite build (`kjournal.html` 137.6 KB < 999 KB ceiling); advanced queue to KCalendar.

- **2026-10-14T01:14:00-07:00 — kilo-expander: KMedia (Deep Feature Expansion)**
  - Expanded playlist manager with JSON backup/restore button and validated M3U playlist export.
  - Enhanced DSP equalizer with Electronic preset and added dancing spectrum visualizer mode toggle.
  - Implemented 6-rate playback speed selector (0.5x, 0.75x, 1.0x, 1.25x, 1.5x, 2.0x).
  - Standardized ergonomic hotkeys: Space (Play/Pause), Arrows (Seek/Vol), M (Mute), F (Fullscreen).
  - Verified clean Vite build (`kmedia.html` 150 KB < 999 KB ceiling); advanced queue to KJournal.

- **2026-10-14T00:31:00-07:00 — kilo-qa: KMaze (Pass 5 QA & State Integrity)**
  - Hardened state persistence: verified F5 quicksave & F9 quickload dual-key support (`kmaze_saveState` and `kmaze_save`) & native `.dat`.
  - Confirmed `kmaze_tutorialSeen` / `.dat` onboarding flag integrity and modal dismissals (Esc, Enter, Close).
  - Verified clean builds: `KMaze.exe` (62 KB) and Vite web build (`kmaze.html` 138 KB < 999 KB ceiling).
  - Queue: Advanced kilo-qa to KAsteroids; handed off to kilo-expander (KMedia).

- **2026-10-14T00:13:00-07:00 — kilo-usability: KRogue (UI/UX & Usability Pass)**
  - Enhanced layout accessibility with ARIA roles/labels across action buttons and `#msg-box` combat log.
  - Added collapsible virtual D-Pad toggle (`[🎮 D-Pad]`) with localStorage persistence for desktop/compact screens.
  - Verified clean Vite build (`npm.cmd run build` 0 errors, `krogue.html` 323 KB < 999 KB ceiling).
  - Queue: Advanced kilo-usability to KMine; handed off to kilo-qa (KMaze).

- **2026-10-13T23:34:00-07:00 — kilo-tester: KAudio (UI Audit & Inline Fixes)**
  - PASS ✅ (3 issues fixed). Added tactile `[+]`/`[-]` octave transpose buttons and `[G]` Portamento/Glide pitch slide.
  - Fixed modal backdrop click and Escape handling to properly restore focus and dismiss when typing in inputs.
  - Startup test PASS (0 errors, 448x98 canvas unblocked), Vite build clean (`kaudio.html` 149.7 KB < 999 KB).
  - Queue: Advanced kilo-tester to KSubmarine; handed off to kilo-usability (KRogue).
