---
current_agent: kilo-tester
next_agent: kilo-tester
agent_rotation:
- kilo-graphics
- kilo-soundscape
- kilo-tester
- kilo-usability
- kilo-qa
- kilo-expander
- kilo-creator
model: gemini-3.8-flash-high
effort: high
timeout_minutes: 10
status: ready
current_targets:
  kilo_creator: kweb://echo-subsystem.net
  kilo_graphics: KDragon
  kilo_soundscape: KAlchemy
  kilo_tester: KPaint
  kilo_usability: KDB
  kilo_qa: KGo
  kilo_expander: KJournal
virtual_web_target: kweb://echo-subsystem.net
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
  agent: kilo-soundscape
  app: KFortress
  timestamp: 2026-10-17T21:38:00+0000
last_planner_run: '2026-10-17T20:53:00Z'
---

# KiloApps Master Fleet Work & Queue State

This document is the single active source of truth for autonomous agent dispatching.
The Windows Task Scheduler orchestrator (`scripts/orchestrate.py`) parses the YAML frontmatter above on every tick to dispatch the active skill.

## Fleet Core Directives (Quick Reference)

> Full architectural rules codified in [.agents/AGENTS.md](.agents/AGENTS.md) and [.agents/rules/](.agents/rules/).
> Full historical director directives codified in [archive/director_directives_archive.md](archive/director_directives_archive.md).

1. **Single-App-Per-Turn & Quality Craft**: Exactly ONE target application per turn. Plan iteratively, inspect diffs, test thoroughly, advance queue, commit, and push.
2. **Strict Size Ceiling**: Every web file (`.html`) and native executable (`.exe`) must remain strictly **< 999 KB**.
3. **Concise Logging & Verification**: ≤8 lines of terse bullet points in execution log. Inspect your diffs and re-read code as needed to verify correctness and dual-parity. Verify with `npm run build` and `uv run scripts/test_app_startup.py --app <target>`.
4. **Multiplayer Autostart Prohibition**: Games must NEVER autostart into matchmaking. Default to local offline play; multiplayer requires explicit button click. Include 25s solo AI fallback.
5. **Anti-Trademark / Parody Rule**: No real commercial marks or scene group names (use *Surreal Tournament*, *VoidCraft*, *RAZOR 1999*, *FLARELIGHT*). Must pass `python scripts/security_lint.py`.
6. **ARG Mystery Preservation (TINAG)**: Never label UI or copy with `(ARG)` or walkthrough guides; never leak `ECHO-1999-ARCHITECT` before App #100 (`KMatrix`).
7. **Perimeter Glint & Comet Ban**: Remove traveling border dots, orbiting balls, and specular glints.
8. **Chiptune Audio Architecture**: Genesis YM2612 2-Op FM / SNES SPC700 stereo delay standard. Zero external audio assets.
9. **Mature Application Quarantine Policy (DIRECTOR MANDATE - CRITICAL)**: If any agent decides to skip an app because it is complete or mature (except `kilo-graphics` skipping non-game apps), do NOT rotate it to the queue bottom! The agent MUST move the app into `## Mature Applications (Archived from Active Queues)` in `next_work.md` and remove it completely from all active queues. The app is permanently archived from all fleet turns until the Director personally directs otherwise.

---

## Active Target Queues (Gemini Flash Optimized Tickets)

### 1. Virtual 1999 Web & ARG Node Creator (`kilo-creator`)
- **Target**: `kweb://echo-subsystem.net`
- **Primary File**: `KiloOS/public/web/echo_subsystem.html`
- **Objective**: Deep harmonic decoders & SIGINT grid expansion: acoustic spectrogram visualizer, carrier wave frequency analyzer, numbers station decoders, secret transponder routing, and Anti-Potemkin depth (<999 KB).
- **Upcoming Queue**: `kweb://deep-core`, `kweb://darknet`, `kweb://portal`, `kweb://webring`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KDragon`
- **Primary Files**: `KiloOS/public/apps/kdragon.html`, `KDragon/main.c` (if present)
- **Objective**: Dragon pet & battle visual overhaul: generate pet dragon stages (egg, hatchling, juvenile, wyrm) and fantasy arena background with Imagen 3 pipeline; ensure 0 specular glints or traveling border dots.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kdragon.html` canvas rendering routines for programmer art.
  2. Generate pet dragon / arena battle sprites via 2-stage asset pipeline (`generate_image` with solid `#FF00FF` -> `scripts/asset_pipeline.py`).
  3. Search for and eliminate any traveling border dots or rotating specular glints.
  4. Run `python scripts/check_icons.py` to audit icon uniqueness.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KAlchemy`.
  - Set `current_agent: kilo-soundscape`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KAlchemy`, `KFortress`, `KMech`, `KColosseum`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `KAlchemy`
- **Primary Files**: `KiloOS/public/apps/kalchemy.html`, `KAlchemy/main.c` (if present)
- **Objective**: Authentic Genesis YM2612 2-Op FM / SPC700 stereo delay chiptune suite: mystical laboratory theme, bubbling potion transmutation stings, and crystalline resonance earcons replacing generic sine beeps.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kalchemy.html` audio synthesis functions.
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
- **Upcoming Queue**: `KDragon`, `KVoid`, `KColony`, `KFortress`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KPaint`
- **Primary File**: `KiloOS/public/apps/kpaint.html`
- **Objective**: Interactive UI element & canvas tool audit: tool selection, color picker, layer controls, brush size sliders, palette swaps, export/import persistence, and keyboard shortcuts.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KPaint`.
  2. Audit all UI elements: tool selection, color swatches, layer controls, modal dialogs, and shortcuts.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KPaint` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KNet`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KNet`, `KColony`, `KTrader`, `KMystery`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KDB`
- **Primary File**: `KiloOS/public/apps/kdb.html`
- **Objective**: Database explorer responsive layout, narrow-viewport (<540px) adaptation, keyboard shortcuts (F1/H help, Esc modal dismissal), accessible focus glow, and high-DPI canvas scaling.
- **Action Steps**:
  1. Verify window bounds and responsive container wrapping under narrow (<540px) widths.
  2. Ensure clear keyboard shortcuts (F1/H help, Esc dialog dismissals) and accessible focus indicators.
  3. Verify HiDPI canvas/text scaling and comfortable retro theme contrast.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KCalc`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KCalc`, `KCipher`, `KPass`, `KVault`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KGo`
- **Primary Files**: `KiloOS/public/apps/kgo.html`, `KGo/main.c`
- **Objective**: Pass 5 QA audit: add first-run tutorial onboarding overlay (`kgo_tutorialSeen`), verify F5 quicksave & F9 quickload capture complete board state/captures/turn, test Escape modal dismissal, and ensure clean dual Win32 native compilation (`KGo.exe`) and web cartridge (<999 KB).
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KGo/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KGo`
  - `cd KiloOS && npm run build` (0 Vite errors, `kgo.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KMaze`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KMaze`, `KSnake`, `K2048`, `KSudoku`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KJournal`
- **Primary Files**: `KiloOS/public/apps/kjournal.html`, `KJournal/main.c` (if present)
- **Objective**: Deep functional feature expansion: rich markdown rendering, calendar day indexing, tag searching, encrypted entry export, and template library.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kjournal.html`.
  2. Expand journal utility: rich markdown rendering, calendar day indexing, tag searching, and export.
  3. Zero external dependencies; maintain responsive 1999 retro styling.
- **Verification**: `uv run scripts/test_app_startup.py --app KJournal` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KBookmark`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KBookmark`, `KFont`, `KRSS`, `KVault`.

---

## Mature Applications (Archived from Active Queues)

> **POLICY**: Applications listed below have reached feature completion, high stability, and mature visual/audio design.
> **EXCLUSION RULE**: These applications are **permanently removed from all active worker queues** until the Director personally directs otherwise.
> **SKIP ADDICTION PREVENTION**: If any agent (except `kilo-graphics` skipping non-game apps) encounters or decides to skip an app because it is complete or mature, it must NOT rotate the app to the queue bottom! The agent MUST record the app in this table, remove it from all upcoming queues and targets in `next_work.md`, and advance to the next non-mature app.

| Application | Primary File | Pass Count / Date | Category | Summary of Mature State |
|---|---|---|---|---|
| **KTowers** | `KiloOS/public/apps/ktowers.html` | Pass 6+ (2026-10-08) | Puzzle / Strategy | Feature-complete Tower of Hanoi variant; full state persistence, responsive controls, zero glints. |
| **KDarts** | `KiloOS/public/apps/kdarts.html` | Pass 6+ (2026-10-08) | Sports / Arcade | Complete physics simulation, scoring engine, audio effects, and UI persistence. |
| **KReversi** | `KiloOS/public/apps/kreversi.html` | Pass 6+ (2026-10-07) | Strategy / Board | Complete board AI, move history, quicksave/load, RFMS multiplayer support. |
| **KMines** | `KiloOS/public/apps/kmines.html` | Pass 8+ (2026-10-15) | Puzzle | Complete classic minesweeper with retro styling, timer persistence, custom difficulty. |
| **KPong** | `KiloOS/public/apps/kpong.html` | Pass 9+ (2026-10-15) | Arcade | Full retro arcade physics, AI paddle tiers, local two-player, CRT curvature shaders. |
| **KAsteroids** | `KiloOS/public/apps/kasteroids.html` | Pass 8+ (2026-10-16) | Arcade | Pure vector wireframe physics, particle explosions, high score persistence. |
| **KChess** | `KiloOS/public/apps/kchess.html` | Pass 9+ (2026-10-15) | Strategy / Board | Complete chess engine, move validation, FEN/PGN import/export, RFMS multiplayer. |
| **KConnect4** | `KiloOS/public/apps/kconnect4.html` | Pass 6+ (2026-10-07) | Strategy / Board | Complete connect 4 engine, minimax AI, win animations, RFMS multiplayer. |
| **KHangman** | `KiloOS/public/apps/khangman.html` | Pass 5 (2026-10-17) | Word / Puzzle | 20-stage campaign, blitz mode, custom word bank studio, dual Win32 native client, full F5/F9 state persistence. |
| **KNote** | `KiloOS/public/apps/knote.html` | Pass 6+ (2026-10-17) | Office / Notes | Feature-complete cybernetic notepad; multi-tab editing, find/replace, markdown preview, keyboard navigation, full state persistence. |

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-17T21:38:00+0000 — kilo-soundscape: KFortress ("Siege of the Iron Citadel" — Combat / Defense FM Chiptune)**
  - Composition: 32-step D Minor / D Phrygian Dominant progression (Dm → C → Bb → A7); 6-channel architecture with vocal lead phrasing, contrary counterpoint, and shell brass stabs (no doubled 5ths).
  - Soundscape & Synthesis: Genesis YM2612 2-Op FM engine (overdriven lead, hollow flute, war horns, slap bass, inharmonic kick/snare/hat) + SPC700 185ms stereo slapback delay line (2200Hz LP filter).
  - Dynamic Adaptation & SFX: Peaceful ambient drone / build phase vs. galloping battle march; modernized 14 siege earcons & spells plus defeat dirge.
  - Accessibility & Modal Fix: Attached standard modal-close classes & titles to helpModal, fixing startup test automation.
  - Status: PASS ✅ (Vite clean, 214.5 KB HTML + 182.8 KB Win32 EXE < 999 KB ceiling, test_app_startup PASS, security lint 100% clean).
  - Queue: Advanced `kilo_soundscape` to `KAlchemy`; rotation handoff to `kilo-tester`.

- **2026-10-17T21:12:02+0000 — kilo-graphics: KDragon (Zero-Token Auto-Skip — Inappropriate Target)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KDragon (pure vector, board game, or mature art).
  - Optimization: Handled via orchestrator pre-flight zero-token auto-skip.
  - Queue: Advanced `kilo-graphics` to `KDragon`; rotation handoff to `kilo-soundscape`.

- **2026-10-17T20:53:00Z — kilo-planner: fleet-wide (Daily 24h Fleet Planning, Queue Health & Log Compaction)**
  - Status: PASS ✅ (24h fleet health assessment, schedule rework, and log compaction complete).
  - Icon Uniqueness: Verified 105 apps in App.jsx, 105 unique .ico files, 0 duplicate hashes (check_icons.py 100% PASS).
  - Quarantine Enforcement: Quarantined mature KNote (Pass 6+); purged mature KMines and KNote from active queues.
  - Queue Rework: Advanced kilo_creator/virtual_web to echo-subsystem; routed graphics to KDragon, soundscape to KFortress, tester to KPaint, usability to KDB, qa to KGo.
  - Byte-Budget Gate: 107 web cartridges and 104 native executables verified <999 KB budget ceiling.
  - Compaction & Archival: Executed rotate_logs.py with zero data loss; updated last_planner_run timestamp.

- **2026-10-17T20:18:00+0000 — kilo-creator: kweb://cybercafe (Deep Web Destination Expansion)**
  - CCTV Surveillance Console: Added 5-feed 10Base-T video matrix with PTZ controls, motion detection, IR night vision, auto-cycle, and snapshot export.
  - ShoutCast '99 NetRadio: Implemented real-time procedural FM chiptune stream with 5 stations, 10-band parametric equalizer, and 64-bin FFT spectrum visualizer.
  - Lore & Anti-Trademark Parody: Fictionalized radio stations, Tremor III Arena PA intercom, and Trapster trivia per ARG standards.
  - KNet & Directory Federation: Verified integration across KNet address bar/chips, portal.html directory/traceroute, and central webring hub.
  - Verification: Clean Vite build (`npm.cmd run build` 0 err), test_app_startup PASS (459.1 KB), 100% security lint PASS; strictly <999 KB ceiling.
  - Queue: Advanced `kilo_creator` to `kweb://10.19.99.4/classified`; rotation handoff to `kilo-graphics`.

- **2026-10-17T18:38:00+0000 — kilo-expander: KClip (Deep Feature Expansion & Dual-Target Parity)**
  - Dual-Target Parity: Expanded Win32 C client with camelCase, snake_case, line deduplication, buffer telemetry (CRC-32, char/word counts), and text archive export/import.
  - Multi-Format Views & Diagnostics: Added HTML live sandbox viewer and image/dataURL inspector to text/hex modes; added Shannon entropy & randomness metrics.
  - Interoperability & Code Tools: Integrated JSON prettify/minify, HTML entity escaping/unescaping, Base64 & URL decoders, CSV library export, and SQL schema/INSERT generator.
  - Template Library: Expanded snippet templates (Win32 WndProc, HTML5 retro skeleton, SQL DDL schema, REST JSON spec, Project Echo telemetry frame).
  - Verification: Clean builds (`npm.cmd run build` 0 errors, `build.bat` 0 warnings); `test_app_startup.py` PASS; size: 188.7 KB HTML, 21.5 KB EXE (<999 KB ceiling).
  - Queue: Advanced `kilo_expander` to `KJournal`; rotation handoff to `kilo-creator`.
