---
current_agent: kilo-soundscape
next_agent: kilo-tester
agent_rotation:
- kilo-creator
- kilo-graphics
- kilo-soundscape
- kilo-tester
- kilo-usability
- kilo-qa
- kilo-expander
model: gemini-3.8-flash-high
effort: selective
timeout_minutes: 8
tool_call_cap: 30
commit_checkpoint: 15
status: ready
current_targets:
  kilo_creator: kweb://deep-core
  kilo_graphics: KRacer
  kilo_soundscape: KAbyss
  kilo_tester: KPing
  kilo_usability: KTerminal
  kilo_qa: KCalc
  kilo_expander: KConverter
virtual_web_target: kweb://deep-core
virtual_web_rotation:
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
- kweb://darknet
last_run:
  agent: kilo-graphics
  app: KAbyss
  timestamp: '2026-10-22T12:34:00-07:00'
last_planner_run: '2026-10-21T23:32:00Z'
---

# KiloApps Master Fleet Work & Queue State

This document is the single active source of truth for autonomous agent dispatching.
The Windows Task Scheduler orchestrator (`scripts/orchestrate.py`) parses the YAML frontmatter above on every tick to dispatch the active skill.

## Fleet Core Directives (Quick Reference)

> Full architectural rules codified in [.agents/AGENTS.md](.agents/AGENTS.md) and [.agents/rules/](.agents/rules/).
> Full historical director directives codified in [archive/director_directives_archive.md](archive/director_directives_archive.md).

1. **Single-App-Per-Turn & Quality Craft**: Exactly ONE target application per turn. Plan iteratively, inspect diffs, test thoroughly, advance queue, commit, and push.
2. **Strict Size Ceiling**: Every web file (`.html`) and native executable (`.exe`) must remain strictly **< 999 KB**.
3. **Tool Call Budget (<=30), Commit Checkpoint (~15) & Concise Logging**: Strict budget of <= 30 tool calls per turn. **Commit Checkpoint**: Once you have substantive edits or reach ~15 tool calls, wrap up, verify, update `next_work.md`, and commit/push immediately. Inspect diffs to verify correctness and dual-parity. Verify with `npm run build` and `uv run scripts/test_app_startup.py --app <target>`, and write a ≤8 line terse log.
4. **Multiplayer Autostart Prohibition**: Games must NEVER autostart into matchmaking. Default to local offline play; multiplayer requires explicit button click. Include 25s solo AI fallback.
5. **Anti-Trademark / Parody Rule**: No real commercial marks or scene group names (use *Surreal Tournament*, *VoidCraft*, *RAZOR 1999*, *FLARELIGHT*). Must pass `python scripts/security_lint.py`.
6. **ARG Mystery Preservation (TINAG)**: Never label UI or copy with `(ARG)` or walkthrough guides; never leak `ECHO-1999-ARCHITECT` before App #100 (`KMatrix`).
7. **Perimeter Glint & Comet Ban**: Remove traveling border dots, orbiting balls, and specular glints.
8. **Chiptune Audio Architecture**: Genesis YM2612 2-Op FM / SNES SPC700 stereo delay standard. Zero external audio assets.
9. **Mature Application Quarantine Policy (DIRECTOR MANDATE - CRITICAL)**: If any agent decides to skip an app because it is complete or mature (except `kilo-graphics` skipping non-game apps), do NOT rotate it to the queue bottom! The agent MUST move the app into `## Mature Applications (Archived from Active Queues)` in `next_work.md` and remove it completely from all active queues. The app is permanently archived from all fleet turns until the Director personally directs otherwise.

---

## Active Target Queues (Gemini Flash Optimized Tickets)
### 1. Virtual 1999 Web & ARG Node Creator (`kilo-creator`)
- **Target**: `kweb://deep-core`
- **Primary File**: `KiloOS/public/web/deep_core.html`
- **Objective**: Deep Core subterranean ghost node & precursor terminal: memory hex fragment reconstruction, encrypted entropy sink, and cross-node parity adhering to Anti-Potemkin Quality Standard (<999 KB).
- **Upcoming Queue**: `kweb://darknet`, `kweb://portal`, `kweb://webring`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KRacer`
- **Primary Files**: `KiloOS/public/apps/kracer.html`
- **Objective**: Outrun-style pseudo-3D arcade racer graphics overhaul: replace vector / primitive canvas vehicles and roadside sprites with Imagen 3 assets (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kracer.html` rendering routines for programmer art.
  2. Generate sprites via 2-stage asset pipeline if appropriate (`generate_image` with solid `#FF00FF` -> `scripts/asset_pipeline.py`).
  3. Search for and eliminate any traveling border dots or rotating specular glints.
  4. Run `python scripts/check_icons.py` to audit icon uniqueness.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KSpace`.
  - Set current_agent: kilo-soundscape.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KSpace`, `KTrader`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `KSpace`
- **Primary Files**: `KiloOS/public/apps/kspace.html`
- **Objective**: Deep space exploration and asteroid mining retro audio suite: celestial ambient drones, mining laser resonance, mineral collection chimes, and warp drive sweeps.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kspace.html` audio synthesis functions.
  2. Implement hardware composer chiptune standard:
     - 3-Voice counterpoint: Lead melody, counter-melody/arpeggio, and punchy FM bassline.
     - Shell voicings: Root + 3rd + 7th (omit 5ths).
     - Swing-tick groove: 16-step rhythmic swing.
     - SPC700 warm stereo slapback delay line (180ms with lowpass filtering).
  3. Pure Web Audio synthesis with zero external audio assets.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `KAbyss`.
  - Set current_agent: kilo-tester.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KAbyss`, `KRacer`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KPing`
- **Primary File**: `KiloOS/public/apps/kping.html`
- **Objective**: Network latency & packet telemetry interactive UI audit: target selection, ping interval sliders, packet size controls, sweep controls, and shortcuts.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KPing`.
  2. Audit all UI elements: ping triggers, interval inputs, sweep toggles, topology canvas, and keyboard hotkeys.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KPing` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KBase`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KBase`, `KPass`, `KCalc`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KTerminal`
- **Primary File**: `KiloOS/public/apps/kterminal.html`
- **Objective**: Terminal emulator ergonomics: responsive scrollback buffer, command autocomplete, font scaling, shortcut guide, and HiDPI canvas/display rendering.
- **Action Steps**:
  1. Inspect layout adaptability across standard and compact window dimensions.
  2. Verify keyboard navigation, shortcut discoverability, and focus trapping.
  3. Ensure high-contrast accessibility, crisp font rendering, and HiDPI canvas scaling.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KBase`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KBase`, `KPass`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KCalc`
- **Primary Files**: `KiloOS/public/apps/kcalc.html`, `KCalc/main.c` (if present)
- **Objective**: Pass 5 QA audit: verify calculator precision, memory registers, tape history, export formats, F5 quicksave & F9 quickload persistence, modal dismissals, and dual Win32 native client parity (<999 KB).
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete calculator state, history tape, memory registers, and unit conversions.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `apps/kcalc/` or `KCalc/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KCalc`
  - `cd KiloOS && npm run build` (0 Vite errors, `kcalc.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KSpace`.
  - Set current_agent: kilo-expander.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KSpace`, `KPing`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KConverter`
- **Primary Files**: `KiloOS/public/apps/kconverter.html`, `KConverter/main.c` (if present)
- **Objective**: Unit, time, coordinates, audio pitch, and data rate converter suite expansion (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kconverter.html`.
  2. Implement dimensional conversion categories, reciprocal ratios, and precision controls.
  3. Ensure dual Win32 native client parity and clean build.
- **Verification**: `uv run scripts/test_app_startup.py --app KConverter` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KCalc`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KCalc`, `KPing`.

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
| **KPaint** | `KiloOS/public/apps/kpaint.html` | Pass 6+ (2026-10-17) | Media / Graphics | Feature-complete 1999 digital art studio; 15 export formats, sprite reel animation, multi-layer engine, dual Win32 client parity, zero glints. |
| **KGo** | `KiloOS/public/apps/kgo.html` | Pass 5 (2026-10-17) | Strategy / Board | Feature-complete Baduk/Go with 20-stage campaign, tsumego puzzle solver, AI personalities, SGF export/import, dual Win32 native client parity, full F5/F9 state persistence. |
| **KMaze** | `KiloOS/public/apps/kmaze.html` | Pass 5 (2026-10-18) | Action / 3D Raycaster | 45-chamber dungeon descent raycaster, Minotaur Overlord boss, dual Win32 native client, full F5/F9 state persistence, first-run tutorial integrity. |
| **KDragon** | `KiloOS/public/apps/kdragon.html` | Pass 6+ (2026-10-18) | RPG / Virtual Pet | Complete dragon pet & arena combat simulation, dual Win32 native client parity, YM2612 2-Op FM / SPC700 3-track audio suite, full F5/F9 state persistence. |
| **KColony** | `KiloOS/public/apps/kcolony.html` | Pass 6+ (2026-10-18) | Strategy / Simulation | 20-structure planetary colony simulation; 4 biomes, 7 modes, tech tree, expeditions, dual Win32 native client parity, YM2612 2-Op FM audio, full F5/F9 state persistence. |
| **KSnake** | `KiloOS/public/apps/ksnake.html` | Pass 5 (2026-10-18) | Arcade / Classic | 30-stage campaign with branch choices, Gauntlet mode, Map Editor Lab, Replay Viewer (.ksr), online/AI duel, dual Win32 native client parity, full F5/F9 state persistence. |
| **K2048** | `KiloOS/public/apps/k2048.html` | Pass 10+ (2026-10-18) | Puzzle / Arcade | 30-stage campaign, powerups, Threes ruleset, arcade duel multiplayer, FEN lab, replay viewer, full F5/F9 state persistence, dual Win32 native client parity. |
| **KSudoku** | `KiloOS/public/apps/ksudoku.html` | Pass 8+ (2026-10-18) | Puzzle / Logic | 30-stage campaign, RFMS online duel, logic explainer, replay scrubber, puzzle manager, full F5/F9 state persistence, dual Win32 native client parity. |
| **KTetris** | `KiloOS/public/apps/ktetris.html` | Pass 10+ (2026-10-18) | Arcade / Classic | Full retro arcade physics, SRS rotation, hold piece, ghost piece, marathon/sprint/ultra modes, RFMS multiplayer, full F5/F9 state persistence, dual Win32 native client parity. |
| **KFreecell** | `KiloOS/public/apps/kfreecell.html` | Pass 5 (2026-10-19) | Card / Solitaire | 4 game modes (incl. 20-stage campaign & time attack), 5 active skills, dual Win32 native client parity, full F5/F9 state persistence, first-run tutorial integrity, zero glints. |
| **KFortress** | `KiloOS/public/apps/kfortress.html` | Pass 6+ (2026-10-19) | Strategy / Tower Defense | Complete 12-map stronghold defense, Imagen 3 sprites & textured terrain, dual Win32 client parity, YM2612 2-Op FM / SPC700 2-movement soundscape, full F5/F9 state persistence. |
| **KMystery** | `KiloOS/public/apps/kmystery.html` | Pass 6+ (2026-10-19) | Puzzle / Adventure | Detective noir murder mystery; interactive clue dossier sketch viewer, forensic lab scanner, suspect interrogation tree, YM2612 2-Op FM jazz, dual Win32 native client parity, full F5/F9 persistence. |
| **KMatch3** | `KiloOS/public/apps/kmatch3.html` | Pass 5 (2026-10-19) | Puzzle / Match-3 | 20-stage campaign, Zen, Timed Rush & Duel Arena modes, Jewel King boss encounter, active skills, dual Win32 native client parity, full F5/F9 state persistence, first-run tutorial integrity. |
| **KChart** | `KiloOS/public/apps/kchart.html` | Pass 6+ (2026-10-20) | Office / Visualization | Feature-complete 1999 spreadsheet & business chart studio; 10 engines, regressions & OLS fit, YM2612 FM / SPC700 sonification, 7 export formats, dual Win32 C parity, full state persistence. |
| **KSimon** | `KiloOS/public/apps/ksimon.html` | Pass 5 (2026-10-20) | Arcade / Memory | Feature-complete 3D audio-visual memory console; 9 modes (incl. 20-stage campaign & online duel), dual Win32 native client parity, full F5/F9 state persistence, first-run tutorial integrity, zero glints. |
| **KWords** | `KiloOS/public/apps/kwords.html` | Pass 5 (2026-10-21) | Word / Puzzle | Feature-complete retro word search with 20-stage campaign, 12 themes, custom puzzle creator/import/export, replay scrubber, dual Win32 native client parity, full F5/F9 state persistence, first-run tutorial integrity. |
| **KColosseum** | KiloOS/public/apps/kcolosseum.html | Pass 5 (2026-10-21) | Strategy / RPG | Gladiatorial ludus management & 10-stage arena combat; 6 facility upgrades, F5/F9 state persistence, first-run tutorial integrity, replay telemetry, dual Win32 native client parity. |
| **KPad** | `KiloOS/public/apps/kpad.html` | Pass 6+ (2026-10-22) | Office / Code Editor | Feature-complete multi-tab code & text workstation; markdown split preview, outline sidebar with touch drag, regex workbench, hex inspector, AES-256 lock, dual Win32 C parity, full F5/F9 state persistence. |

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-22T12:34:00-07:00 — kilo-graphics: KAbyss (Asset Suitability Audit & Glint Sweep)**
  - ⏭️ Skip — Imagen 3 asset replacement not appropriate for KAbyss
  - Engine Audit: Procedural Canvas 2D dungeon crawler with dynamic FOV raycasting, torch radial lighting, and custom animated entity rendering.
  - Glint Sweep: Verified 0 traveling border dots, orbiting specular glints, or wandering perimeter balls.
  - Verification: Icon uniqueness PASS (105/105 unique hashes); test_app_startup PASS on KAbyss (479.4 KB < 999 KB); KiloOS npm build PASS (0 errors).
  - Queue Handoff: Advanced current_targets.kilo_graphics to KRacer; rotated current_agent to kilo-soundscape.

- **2026-10-22T11:38:00-07:00 — kilo-creator: echo-subsystem.net (Tab 17 PLL Demodulator & Airgap Bridge)**
  - PASS ✅ (Implemented Tab 17 Phase-Locked Loop demodulator, Costas I/Q phase plane scope, and cross-strata airgap bridge).
  - PLL & Demodulation: Coherent carrier tracking (1999Hz, 1200/2200Hz AFSK, 999.5Hz, 432Hz), loop filter bandwidths, and auto-acquire lock.
  - Costas I/Q Phase Scope: Real-time vector constellation canvas with phosphor persistence decay and phase slip generator.
  - Airgap Packet Bridge: Cross-node packet dissection (10.19.99.1/4/19/127), CRC-16 check, .CAP log export, and clipboard copy.
  - Audio Architecture: Procedural YM2612 2-Op FM + SPC700 stereo delay warmth monitor & AFSK chirp synthesis (zero external assets).
  - Dual Parity & Build: KNet Win32 C parity updated & compiled (46.0 KB < 999 KB); echo_subsystem.html verified (586.8 KB < 999 KB).
  - Verification: npm run build in KiloOS PASS (0 errors); test_app_startup PASS on KNet (189.8 KB); byte_budget_gate PASS.
  - Queue Handoff: Advanced kilo_creator and virtual_web_target to kweb://deep-core; rotated current_agent to kilo-graphics.

- **2026-10-22T10:35:00-07:00 — kilo-expander: KBase (Deep Feature Expansion & Dual Parity)**
  - PASS ✅ (Multi-base conversion, granular bitwise mask studio, endianness toggles, and dual parity).
  - Radices & Bases: Unsigned/signed DEC, HEX, OCT, BIN (64-bit), and Base64 live conversion.
  - Granular Mask Studio: Implemented 6-way simultaneous mask ops (Apply, Set, Clear, Toggle, Extract, Deposit).
  - Endianness Suite: 64-bit byte swap, 32-bit swap, LE/BE toggles, and popcount diagnostics.
  - Dual Parity & Build: Verified native KBase.exe (26.6 KB < 999 KB) and Web kbase.html (157.4 KB).
  - Verification: test_app_startup PASS, KiloOS npm build PASS (0 errors), security_lint PASS.
  - Queue Handoff: Advanced current_targets.kilo_expander to KConverter; rotated current_agent to kilo-creator.

- **2026-10-22T07:18:00-07:00 — kilo-qa: KPuzzle (Pass 5 QA & Build Audit)**
  - ⏭️ Skip — KPuzzle does not exist in fleet catalog; advanced target to KCalc.

- **2026-10-22T06:36:00-07:00 — kilo-usability: KPad (UI/UX Ergonomics & Mature Quarantine)**
  - PASS ✅ (Layout responsiveness, touch outline splitter, modal enter guards, and mature quarantine).
  - Modal Ergonomics: Fixed global Enter listener to guard interactive inputs (search, go-to-line, encryption passphrase) against premature modal dismissal.
  - Touch & Small Viewports: Added touch drag gestures (`touchstart`/`touchmove`/`touchend`) to outline splitter; constrained user-opened outline width to 75vw on mobile.
  - HiDPI & Splitter: Verified canvas minimap devicePixelRatio rendering and preview ratio drag mechanics.
  - Parity & Verification: Native KPad.exe clean build (38.9 KB < 999 KB); test_app_startup PASS (331.0 KB); KiloOS build PASS (0 errors); byte_budget_gate PASS; security_lint PASS.
  - Mature Quarantine: Promoted KPad to Mature Applications archive; advanced queue to KTerminal.
