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
  kilo_creator: kweb://10.19.99.4/classified
  kilo_graphics: KSpace
  kilo_soundscape: KTrader
  kilo_tester: KPass
  kilo_usability: KCalc
  kilo_qa: KColosseum
  kilo_expander: KPing
virtual_web_target: kweb://10.19.99.4/classified
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
  app: kweb://cybercafe
  timestamp: '2026-10-21T17:35:00-07:00'
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
- **Target**: `kweb://10.19.99.4/classified`
- **Primary File**: `KiloOS/public/web/classified.html`
- **Objective**: Classified Corporate Intranet Leak dossier: classified memo viewer, security clearance authentications, encrypted document decryptor, and subterranean ARG breadcrumbs adhering to Anti-Potemkin Quality Standard (<999 KB).
- **Upcoming Queue**: `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://darknet`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KSpace`
- **Primary Files**: `KiloOS/public/apps/kspace.html`
- **Objective**: Space combat retro arcade overhaul: replace vector / primitive shapes with Imagen 3 generated ship, asteroid, laser, and space station sprites, plus seamless nebula starfield background (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kspace.html` canvas rendering routines for programmer art.
  2. Generate sprites via 2-stage asset pipeline if appropriate (`generate_image` with solid `#FF00FF` -> `scripts/asset_pipeline.py`).
  3. Search for and eliminate any traveling border dots or rotating specular glints.
  4. Run `python scripts/check_icons.py` to audit icon uniqueness.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KTrader`.
  - Set `current_agent: kilo-soundscape`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KTrader`, `KAbyss`, `KRacer`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `KTrader`
- **Primary Files**: `KiloOS/public/apps/ktrader.html`
- **Objective**: Stock exchange and commodity trader retro audio suite: trading pit ambient drones, ticker tape rhythmic chatter, bull/bear bell earcons, and market crash/boom stingers.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/ktrader.html` audio synthesis functions.
  2. Implement hardware composer chiptune standard:
     - 3-Voice counterpoint: Lead melody, counter-melody/arpeggio, and punchy FM bassline.
     - Shell voicings: Root + 3rd + 7th (omit 5ths).
     - Swing-tick groove: 16-step rhythmic swing.
     - SPC700 warm stereo slapback delay line (180ms with lowpass filtering).
  3. Pure Web Audio synthesis with zero external audio assets.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `KSpace`.
  - Set `current_agent: kilo-tester`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KSpace`, `KAbyss`, `KRacer`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KPass`
- **Primary File**: `KiloOS/public/apps/kpass.html`
- **Objective**: Retro password manager and credential safe interactive UI audit: master password prompt, generator controls, entry cards, search bar, clip actions, and hotkey persistence.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KPass`.
  2. Audit all UI elements: snippet triggers, copy buttons, search inputs, tags, and shortcuts.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KPass` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KCalc`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KCalc`, `KPing`, `KBase`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KCalc`
- **Primary File**: `KiloOS/public/apps/kcalc.html`
- **Objective**: Desktop calculator ergonomics: responsive button grid, standard/scientific mode toggles, memory indicators, keyboard numpad hotkeys, and HiDPI crisp canvas/display rendering.
- **Action Steps**:
  1. Inspect layout adaptability across standard and compact window dimensions.
  2. Verify keyboard navigation across credential records and generator dialogs.
  3. Ensure high-contrast accessibility and crisp font/cursor rendering.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KPing`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KPing`, `KPad`, `KTerminal`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KColosseum`
- **Primary Files**: `KiloOS/public/apps/kcolosseum.html`, `apps/kcolosseum/main.c` (if present)
- **Objective**: Pass 5 QA audit: verify first-run tutorial onboarding overlay (`kcolosseum_tutorialSeen`), verify F5 quicksave & F9 quickload capturing gladiator stats, arena stage, equipment, and gold; test modal dismissal; compile Win32 native client and verify web cartridge (<999 KB).
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete gladiator combat, inventory, and stage progression.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `apps/kcolosseum/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KColosseum`
  - `cd KiloOS && npm run build` (0 Vite errors, `kcolosseum.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KPuzzle`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KPuzzle`, `KCalc`, `KSpace`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KPing`
- **Primary Files**: `KiloOS/public/apps/kping.html`, `KPing/main.c` (if present)
- **Objective**: Network latency, ICMP echo topology, and retro traceroute diagnostics suite expansion (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kping.html`.
  2. Implement granular ping intervals, jitter histograms, and simulated subnet traceroutes.
  3. Ensure dual Win32 native client parity and clean build.
- **Verification**: `uv run scripts/test_app_startup.py --app KPing` and `cd KiloOS && npm.cmd run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KBase`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KBase`, `KConverter`.

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

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-21T17:35:00-07:00 — kilo-creator: kweb://cybercafe (Virtual 1999 Cybercafe Deep Expansion & ARG Webring Polish)**
  - Webring Topology Polish: Replaced clearnet link to Tier 3 echo-subsystem with canonical Member #007 Contributor Portal per arg_plan.md.
  - Subterranean Litho-BBS Relay: Implemented station CLI `dial 10.19.99.254` / `telnet 10.19.99.254` with acoustic V.90 handshake & Node 0x7F telemetry burst.
  - Verification & Lint: Verified 0 security lint violations, 0 Vite build errors, and cybercafe.html strictly compliant (<999 KB, 471 KB).
  - Queue Handoff: Advanced current_targets.kilo_creator and virtual_web_target to kweb://10.19.99.4/classified; rotated current_agent to kilo-graphics.

- **2026-10-21T16:32:00-07:00 — kilo-planner: Fleet (24h Fleet Planning & Queue Rework)**
  - Fleet Velocity: Assessed 105 apps; 26 mature applications archived in quarantine table.
  - Queue Rebalance: Dispersed targets across active queues (kweb://cybercafe, KSpace, KTrader, KPass, KCalc, KColosseum, KPing); 0 mature collisions.
  - Icon Uniqueness: Verified all 105 apps have distinct icons and 0 duplicate SHA256 hashes via check_icons.py.
  - Verification: Clean startup tests verified across active targets; Vite build clean; zero console errors.
  - Queue Handoff: Updated last_planner_run timestamp; rotated current_agent to kilo-creator for next scheduled cycle.

- **2026-10-21T15:38:00-07:00 — kilo-expander: KHex (Deep Feature Expansion: PE32 Dissector, Multi-Segment Diff & Binary Patch Builder)**
  - PE32 Header Dissector: Added full DOS MZ, COFF header, Optional Header 32/64, and Section table parser (.text, .rdata, .data) with RVA-to-raw jump and sample PE generator.
  - Multi-Segment Diffing: Upgraded binary diff engine with contiguous hunk segmentation, mutation type tagging (Modify, Insert, Delete), and dual hunk/byte view toggles.
  - Binary Patch Builder: Added IPS ROM patch builder, DOS DEBUG script generator, C array delta struct, JSON deltas, and in-place patch applier.
  - Dual-Target Parity: Updated Win32 native C client with PE32 header dissector and multi-segment delta reporting; built KHex.exe cleanly (34.3 KB).
  - Verification & Build: Direct startup test PASS (264.4 KB < 999 KB); Vite build clean; F5/F9 state and keyboard shortcuts verified.
  - Queue Handoff: Advanced current_targets.kilo_expander to KPing; rotated current_agent to kilo-creator.

- **2026-10-21T14:35:00-07:00 — kilo-qa: KWords (Pass 5 QA & Build Quality Audit — Promoted to Mature)**
  - State Persistence: Wired explicit F5 (quicksave) & F9 (quickload) hotkeys across web & Win32 native client; verified complete board, theme, charges, stage, and score backup.
  - First-Run Tutorial: Ensured fresh sessions launch onboarding help modal via storage/dat flags, with restored saves automatically suppressing overlay.
  - Modal Dismissals: Added Esc, Enter, and Space dismissals across help, statistics, and victory overlays; verified clean modal dismissal and canvas hit-test unblocked.
  - Dual Parity & Build: Compiled Win32 native client KWords.exe cleanly (165.8 KB); web cartridge verified (156.4 KB < 999 KB); test_app_startup PASS; Vite build clean.
  - Quarantine & Queue: Promoted KWords to Mature Applications table; advanced current_targets.kilo_qa to KColosseum; handoff to kilo-expander.

- **2026-10-21T13:36:00-07:00 — kilo-usability: KHex (Navigation Ergonomics & Usability Polish)**
  - Tab Ergonomics: Upgraded .tab-nav to flex-wrap with 6px gap to comfortably display all 9 suite tabs without clipping or hidden scrolling.
  - Keyboard Hotkeys: Bound [9] to switchTab('disasm'); added Ctrl+Home/End buffer boundary jumping; added [Enter] for instant byte editing.
  - Lifecycle Sync: Automated disassembler run upon switching to Tab 9; added Escape dismissal for byte & offset input bars.
  - Accessibility & Guides: Added high-contrast :focus-visible rings across suite; updated Help and Tutorial modals with Tabs 8-9 documentation.
  - Dual Parity & Build: Verified KHex native Win32 C build clean (31.5 KB) and web cartridge (223.1 KB < 999 KB); startup check PASS; Vite clean.
  - Queue Handoff: Advanced current_targets.kilo_usability to KPass; rotated current_agent to kilo-qa.

- **2026-10-21T13:16:36-07:00 — kilo-tester: KClip (Interactive UI Element Audit & Wiring Fixes)**
  - UI Element Audit: Verified snippet triggers, modal lifecycles, font size zoom, word wrap toggle, and dual text/hex/HTML preview tabs.
  - Inline Repairs: Added missing handlers in pplyTransform for kebab, sort_asc, sort_desc, dedupe, escape_html, unescape_html, and csv_to_md.
  - Feature Enhancement: Integrated responsive Find & Replace bar with keyboard shortcut (Ctrl+H) and Escape-to-close handling.
  - Dual Parity & Size Verification: Built Win32 native KClip.exe cleanly (21.5 KB); web cartridge < 999 KB (197.7 KB); startup check PASS.
  - Queue Handoff: Advanced current_targets.kilo_tester to KTrader; rotated current_agent to kilo-usability.
