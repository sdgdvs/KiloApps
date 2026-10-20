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
model: gemini-3.8-flash-high
effort: selective
timeout_minutes: 8
tool_call_cap: 30
commit_checkpoint: 15
status: ready
current_targets:
  kilo_creator: kweb://asm-temple
  kilo_graphics: KColosseum
  kilo_soundscape: KColosseum
  kilo_tester: KClip
  kilo_usability: KHex
  kilo_qa: KWords
  kilo_expander: KHex
virtual_web_target: kweb://asm-temple
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
  agent: kilo-creator
  app: kweb://users/~neon_rider
  timestamp: '2026-10-20T22:20:00Z'
last_planner_run: '2026-10-19T22:16:00Z'
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
- **Target**: `kweb://asm-temple`
- **Primary File**: `KiloOS/public/web/asm_temple.html`
- **Objective**: Expand x86 assembly temple shrine: 16-bit real mode demos, interrupt vector tables, opcode reference matrices, and subterranean ARG reflections adhering to Anti-Potemkin Quality Standard (<999 KB).
- **Upcoming Queue**: `kweb://cybercafe`, `kweb://10.19.99.4/classified`, `kweb://echo-subsystem.net`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KColosseum`
- **Primary Files**: `KiloOS/public/apps/kcolosseum.html`, `KColosseum/main.c` (if present)
- **Objective**: Gladiator combat visual asset pipeline evaluation: warrior sprites, arena terrain textures, and UI HUD; eliminate any specular glints or traveling border dots.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kcolosseum.html` canvas rendering routines for programmer art.
  2. Generate sprites via 2-stage asset pipeline if appropriate (`generate_image` with solid `#FF00FF` -> `scripts/asset_pipeline.py`).
  3. Search for and eliminate any traveling border dots or rotating specular glints.
  4. Run `python scripts/check_icons.py` to audit icon uniqueness.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KColosseum`.
  - Set `current_agent: kilo-soundscape`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KTrader`, `KSpace`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `KColosseum`
- **Primary Files**: `KiloOS/public/apps/kcolosseum.html`, `KColosseum/main.c` (if present)
- **Objective**: Gladiator arena combat chiptune soundscape: roaring crowd drones, steel clashing transients, triumphant victory fanfares, and sand arena theme.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kcolosseum.html` audio synthesis functions.
  2. Implement hardware composer chiptune standard:
     - 3-Voice counterpoint: Lead melody, counter-melody/arpeggio, and punchy FM bassline.
     - Shell voicings: Root + 3rd + 7th (omit 5ths).
     - Swing-tick groove: 16-step rhythmic swing.
     - SPC700 warm stereo slapback delay line (180ms with lowpass filtering).
  3. Pure Web Audio synthesis with zero external audio assets.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `KTrader`.
  - Set `current_agent: kilo-tester`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KTrader`, `KSpace`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KClip`
- **Primary File**: `KiloOS/public/apps/kclip.html`
- **Objective**: Multi-format retro clipboard manager and snippet archive interactive UI audit: format tabs, paste history, search filtering, and hotkey persistence.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KClip`.
  2. Audit all UI elements: snippet triggers, copy buttons, search inputs, tags, and shortcuts.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KClip` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KTrader`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KTrader`, `KSpace`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KHex`
- **Primary File**: `KiloOS/public/apps/khex.html`
- **Objective**: Hexadecimal editor and binary inspector ergonomics: responsive byte grid, ASCII sidebar, search/jump controls, and keyboard navigation.
- **Action Steps**:
  1. Inspect layout adaptability across standard and compact window dimensions.
  2. Verify keyboard navigation across byte grid and inspector panels.
  3. Ensure high-contrast accessibility and crisp font/cursor rendering.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KPass`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KPass`, `KCalc`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KWords`
- **Primary Files**: `KiloOS/public/apps/kwords.html`, `KWords/main.c` (if present)
- **Objective**: Pass 5 QA audit: verify first-run tutorial onboarding overlay (`kwords_tutorialSeen`), verify F5 quicksave & F9 quickload capturing complete game board, vocabulary state, round, and high score; test Escape modal dismissal; compile Win32 native client and verify web cartridge (<999 KB).
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KWords/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KWords`
  - `cd KiloOS && npm run build` (0 Vite errors, `kwords.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KColosseum`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KColosseum`, `KCalc`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KHex`
- **Primary Files**: `KiloOS/public/apps/khex.html`, `KHex/main.c` (if present)
- **Objective**: Hexadecimal editor and binary analysis suite expansion: PE32 header dissector, multi-segment diffing, and binary patch builder (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/khex.html`.
  2. Implement deeper structural analysis tools and binary inspection features.
  3. Ensure dual Win32 native client parity and clean build.
- **Verification**: `uv run scripts/test_app_startup.py --app KHex` and `cd KiloOS && npm.cmd run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KPing`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KPing`, `KBase`.

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

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-20T22:20:00Z — kilo-creator: kweb://users/~neon_rider (Virtual 1999 Web Deep Expansion & Scoping Repair)**
  - Navigation & Scoping Fix: Extracted Section 11 (Subnet Sniffer) out of `switchTab()` closure, fixing tab re-entrancy and leak; properly routed tab activation.
  - Interactive Demoscene Shouts: Added interactive 1999 demoscene greetings box in `tab-about` with retro audio stingers (FLARELIGHT, RAZOR 1999, SKID VECTOR, PAST CREW, SILICON SILENCE, PARALAX).
  - Devlog Expansion: Added Article 10 covering self-modifying x86 machine code, dynamic opcode rewriting, and `VirtualProtect` memory page permissions in Win32 protected mode.
  - XM Tracker Display: Implemented real-time 4-channel FastTracker II / ProTracker pattern viewer row display synced to YM2612 2-Op FM + SPC700 tracker engine.
  - Dual-Target Parity: Enhanced `KNet/main.c` Win32 C virtual web terminal to display matching 1999 ASM devlog, hardware rig, demoscene greetings, and webring nav; compiled cleanly to `KNet.exe` (46 KB).
  - Verification: Security linter 100% PASS; `npm run build` clean in 550ms; startup check PASS (`KNet`); all sizes strictly < 999 KB (`neon_rider.html` 291.8 KB).
  - Queue Handoff: Advanced `current_targets.kilo_creator` and `virtual_web_target` to `kweb://asm-temple`; rotated `current_agent` to `kilo-graphics`.

- **2026-10-20T21:55:00Z — kilo-adhoc: Fleet Configuration (Token Efficiency & Tool Budget Upgrade)**
  - Tool Call Budget: Enforced strict <= 30 tool calls per turn with commit checkpoint at ~15 tool calls in orchestrator and all skills.
  - Timeout & Selective Effort: Reduced default timeout to 8m (from 10m); implemented selective effort (high for creator/expander/soundscape/planner; medium for tester/usability/qa/graphics).
  - KDB Expansion Completed: Verified & committed KDB relational schema studio, visual ER diagram, and query builder; Win32 C and web builds clean.
  - Verification: `security_lint.py` PASS; `test_app_startup.py --app KDB` PASS; Vite build clean (488ms).
  - Queue Handoff: Advanced `current_targets.kilo_expander` to `KHex`; active rotation to `kilo-creator`.

- **2026-10-20T18:22:00Z — kilo-qa: KSimon (Pass 5 QA Audit & Mature Promotion)**
  - State Persistence: Audited & fixed F5 quicksave / F9 quickload capturing complete game sequence, mode, round, elapsed time, powerup charges, and stage in both Web storage and Win32 C (`ksimon.ini`).
  - Tutorial & Overlays: Verified first-run tutorial overlay (`ksimon_tutorialSeen`) never interrupts restored saves; wired start-on-dismiss for Space/Enter/Got-It buttons; verified clean Esc cancellation and modal dismissals.
  - Glint Audit & Parity: Verified zero traveling border comets or specular glints; compiled Win32 native client (`KSimon.exe`, 141.5 KB) and verified dual-target parity with web cartridge (124.5 KB < 999 KB).
  - Verification: Startup check PASS (0 JS errors, CSS valid, canvas unblocked); Vite build clean (514ms); native Win32 C build clean (0 warnings).
  - Quarantine & Handoff: Promoted KSimon to Mature Applications quarantine; advanced `kilo_qa` to `KWords`; rotation handoff to `kilo-expander`.

- **2026-10-20T14:17:00Z — kilo-usability: KFont (Bitmap Font Designer & Typography Workbench UX)**
  - Window & Ergonomics: Adjusted default dimensions to 1024x768 in App.jsx; added responsive glyph filter/jump input and range counter.
  - Zoom & Matrix Controls: Added zoom in/out/fit controls for pixel matrix editor and anatomy inspector with wheel zoom support.
  - Accessibility & Navigation: Implemented forced-colors high contrast styles, sample text wrap/align toggles, and ARIA labels.
  - Keyboard & Modal Shortcuts: Bound [F1/H] help, [F5/F9] save/load, [E] profile export, [M] audio toggle, and arrow key navigation.
  - Dual Parity & Build: Recompiled Win32 native client (`KFont.exe`, 32.8 KB); startup check PASS (218.0 KB < 999 KB, 0 JS err); Vite build clean.
  - Queue Handoff: Advanced `current_targets.kilo_usability` to `KHex`; rotation handoff to `kilo-qa`.

- **2026-10-20T05:22:00Z — kilo-tester: KChart (UI Audit, Inline Fixes & Mature Promotion)**
  - UI & Interaction Audit: Audited 10 visualization engines, 7 trend/regression overlays, YM2612 FM / SPC700 sonification sweep, 7 export formats (SVG, PNG, MD, C, HTML, CSV, JSON), and table editing.
  - Inline Repairs: Added active sonification cancellation on table edits, transforms, and arrow nudges; fixed index bounds validation for inline cell edits; added hoveredIndex adjustment on point deletion.
  - Parity & Verification: Startup check PASS (131.9 KB < 999 KB, CSS valid, 0 JS err, modal dismisses cleanly); native Win32 C clean (`KChart.exe`, 36.0 KB); Vite build clean (475ms).
  - Quarantine & Handoff: Promoted KChart to Mature Applications quarantine; advanced `kilo_tester` to `KClip`; rotation handoff to `kilo-usability`.
