---
current_agent: kilo-tester
next_agent: kilo-usability
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
  kilo_creator: kweb://echo-subsystem.net
  kilo_graphics: KAbyss
  kilo_soundscape: KAbyss
  kilo_tester: KCalc
  kilo_usability: KPad
  kilo_qa: KPuzzle
  kilo_expander: KBase
virtual_web_target: kweb://echo-subsystem.net
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
  agent: kilo-soundscape
  app: KSpace
  timestamp: '2026-10-22T03:33:00-07:00'
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
- **Target**: `kweb://echo-subsystem.net`
- **Primary File**: `KiloOS/public/web/echo_subsystem.html`
- **Objective**: Echo Subsystem acoustic lab & subterranean RF transceiver node: 1999Hz Morse subcarrier demodulation, soundscape spectrogram, and cross-node resonance adhering to Anti-Potemkin Quality Standard (<999 KB).
- **Upcoming Queue**: `kweb://deep-core`, `kweb://darknet`, `kweb://portal`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KAbyss`
- **Primary Files**: `KiloOS/public/apps/kabyss.html`
- **Objective**: Deep sea trench exploration & sonar survey graphics overhaul: replace vector / primitive canvas submarines and trench fauna with Imagen 3 assets (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kabyss.html` rendering routines for programmer art.
  2. Generate sprites via 2-stage asset pipeline if appropriate (`generate_image` with solid `#FF00FF` -> `scripts/asset_pipeline.py`).
  3. Search for and eliminate any traveling border dots or rotating specular glints.
  4. Run `python scripts/check_icons.py` to audit icon uniqueness.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KRacer`.
  - Set current_agent: kilo-soundscape.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KRacer`, `KSpace`, `KTrader`.

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
  - Set `current_agent: kilo-tester`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KAbyss`, `KRacer`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KCalc`
- **Primary File**: `KiloOS/public/apps/kcalc.html`
- **Objective**: Desktop calculator interactive UI audit: arithmetic operations, display memory, scientific function buttons, and keyboard hotkeys.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KCalc`.
  2. Audit all UI elements: numpad buttons, operator keys, memory triggers, mode toggles, and shortcuts.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KCalc` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KPing`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KPing`, `KBase`, `KPass`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KPad`
- **Primary File**: `KiloOS/public/apps/kpad.html`
- **Objective**: Rich text & markdown scratchpad ergonomics: responsive sidebar layout, document outline scaling, keyboard shortcuts guide, and HiDPI crisp canvas/display rendering.
- **Action Steps**:
  1. Inspect layout adaptability across standard and compact window dimensions.
  2. Verify keyboard navigation, shortcut discoverability, and focus trapping.
  3. Ensure high-contrast accessibility, crisp font rendering, and HiDPI canvas scaling.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KTerminal`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KTerminal`, `KBase`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KPuzzle`
- **Primary Files**: `KiloOS/public/apps/kpuzzle.html`
- **Objective**: Pass 5 QA audit: verify first-run tutorial onboarding overlay (`kpuzzle_tutorialSeen`), verify F5 quicksave & F9 quickload capturing grid state, move count, timer, and difficulty; test modal dismissal; compile Win32 native client (if present) and verify web cartridge (<999 KB).
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete puzzle layout, timer, moves, and completion records.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `apps/kpuzzle/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KPuzzle`
  - `cd KiloOS && npm run build` (0 Vite errors, `kpuzzle.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KCalc`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KCalc`, `KSpace`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KBase`
- **Primary Files**: `KiloOS/public/apps/kbase.html`, `KBase/main.c` (if present)
- **Objective**: Base encoding, radices conversion, and binary bitfield inspection suite expansion (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kbase.html`.
  2. Implement granular bitwise masks, radices conversion (bin/oct/dec/hex/base64), and endianness toggles.
  3. Ensure dual Win32 native client parity and clean build.
- **Verification**: `uv run scripts/test_app_startup.py --app KBase` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KConverter`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KConverter`, `KCalc`.

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

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-22T03:33:00-07:00 — kilo-soundscape: KSpace (Starlight Frontier: Deep Void Protocol)**
  - Composition: 64-step Dm9 -> Bbmaj7#11 -> Gm9 -> A7b9 progression, 6 channels, 5:3 swing groove (~138 BPM).
  - Voices & Patches: Crystalline Lead w/ delayed vibrato, Hollow Flute counter, Shell Arp, YM2612 Slap Bass, SPC700 Pad.
  - Dual Parity: Web [M]/[N] audio controls & Asteroid shatter resonance (SFX 13) mirrored in Win32 C (KSpace.exe 91.6 KB).
  - Verification: test_app_startup PASS (200.4 KB < 999 KB), Vite build clean (0 errors), security_lint PASS.
  - Queue Handoff: Advanced current_targets.kilo_soundscape to KAbyss; rotated current_agent to kilo-tester.

- **2026-10-22T02:18:00-07:00 — kilo-graphics: KTrader (Perimeter Glint Audit & Inappropriate Target Skip)**
  - ⏭️ Skip — Imagen 3 asset replacement not appropriate for KTrader
  - Target Assessment: KTrader is a cyan vector HUD/CRT commodity and stock exchange sim; raster assets inappropriate.
  - Glint Audit: Eliminated traveling orbital satellite dot in `ktrader.html` & `KTrader/main.c`; anchored to static mooring dock.
  - Dual Parity: Clean parity verified across Web HTML & Win32 C `KTrader.exe` (26.6 KB).
  - Verification: `npm run build` PASS (0 errors), `test_app_startup.py` PASS (107.2 KB < 999 KB), `security_lint.py` PASS, icons UNIQUE.
  - Queue Handoff: Advanced `current_targets.kilo_graphics` to `KAbyss`; rotated `current_agent` to `kilo-soundscape`.

- **2026-10-21T23:36:00-07:00 — kilo-expander: KPing (Diagnostic Suite Expansion & Dual Parity)**
  - PASS ✅ (Subnet topology, granular pacing & fleet mesh parity implemented across web and Win32 C).
  - Web Improvements: Repaired btnTopo disable logic so subnet topology trace can be cleanly cancelled/stopped; wired subnet sweep & pacing controls.
  - Native Win32 Parity: Created Topo [X] and Mesh [G] buttons; instantiated Int(ms) granular pacing input & hotkeys ('[' / ']'); added Mode 8 Fleet Mesh telemetry.
  - Interactive Hotkeys: Added 'G' for fleet mesh echo, '[' / ']' for pacing interval adjustments, and 'J' for RFC 3550 jitter telemetry in native client.
  - Parity & Verification: test_app_startup PASS (175.5 KB < 999 KB); native KPing.exe clean build (47.6 KB < 999 KB); Vite build PASS (0 errors).
  - Queue Handoff: Advanced current_targets.kilo_expander to KBase; rotated current_agent to kilo-creator.

- **2026-10-21T21:35:00-07:00 — kilo-qa: KColosseum (Pass 5 QA & Build Audit)**
  - PASS ✅ (Modal dismissal & facilities persistence repaired; promoted to Mature Quarantine).
  - Modal Dismissal: Included facilitiesModal in closeAllModals & anyModalOpen; added Enter dismissal.
  - State Persistence: Extended applySaveData to initialize all 6 ludus facilities; verified F5/F9 save/load parity.
  - First-Run Tutorial: Verified kcolosseum_tutorialSeen flag suppresses briefing on restored saves.
  - Parity & Verification: test_app_startup PASS (270.8 KB < 999 KB); native KColosseum.exe built cleanly (39.4 KB); npm run build PASS (0 errors).
  - Queue Handoff: Advanced current_targets.kilo_qa to KPuzzle; rotated current_agent to kilo-expander.

- **2026-10-21T20:35:00-07:00 — kilo-usability: KPing (UI/UX & Layout Polish)**
  - PASS ✅ (Layout & window ergonomics optimized; dual parity verified).
  - Window & Layout: Expanded default window in App.jsx to 1040x720; streamlined toolbar padding/margins to prevent cramped wrapping.
  - Responsive Ergonomics: Added 1040px responsive breakpoint and tightened controls grid so all diagnostic fields fit cleanly.
  - Telemetry Tooltips & ARIA: Added descriptive RFC/ITU-T standard tooltips and ARIA labels across telemetry analytics cards.
  - Accessibility: Added aria-label & aria-pressed toggles on audio and host inputs; verified HiDPI canvas scaling.
  - Verification: test_app_startup PASS (159.6 KB < 999 KB); native KPing.exe built cleanly (39.4 KB); npm run build PASS (0 errors).
  - Queue Handoff: Advanced current_targets.kilo_usability to KPad; rotated current_agent to kilo-qa.

