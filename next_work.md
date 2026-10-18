---
current_agent: kilo-expander
next_agent: kilo-expander
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
  kilo_creator: kweb://darknet
  kilo_graphics: KAlchemy
  kilo_soundscape: KVoid
  kilo_tester: KTrader
  kilo_usability: KPass
  kilo_qa: K2048
  kilo_expander: KFont
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
  agent: kilo-qa
  app: KSnake
  timestamp: 2026-10-18T19:20:00+0000
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
- **Target**: `kweb://darknet`
- **Primary File**: `KiloOS/public/web/darknet.html`
- **Objective**: Encrypted underground relay & cryptography lab: hex memory inspector, Bell 103/202 FSK modem, Shared Signal Interlock, Feistel & RC4 cipher suites (<999 KB).
- **Upcoming Queue**: `kweb://portal`, `kweb://webring`, `kweb://warez`, `kweb://geocities`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KAlchemy`
- **Primary Files**: `KiloOS/public/apps/kalchemy.html`, `KAlchemy/main.c` (if present)
- **Objective**: Alchemical lab & potion crafting visual overhaul: cauldron animations, reagent sprites, and parchment grimoire UI; ensure 0 specular glints or traveling border dots.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kalchemy.html` canvas rendering routines for programmer art.
  2. Generate reagent / cauldron sprites via 2-stage asset pipeline (`generate_image` with solid `#FF00FF` -> `scripts/asset_pipeline.py`).
  3. Search for and eliminate any traveling border dots or rotating specular glints.
  4. Run `python scripts/check_icons.py` to audit icon uniqueness.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KFortress`.
  - Set `current_agent: kilo-soundscape`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KFortress`, `KMech`, `KColosseum`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `KVoid`
- **Primary Files**: `KiloOS/public/apps/kvoid.html`, `KVoid/main.c` (if present)
- **Objective**: Authentic Genesis YM2612 2-Op FM / SPC700 stereo delay chiptune suite: cosmic void ambient drone, stellar jump resonance, and void combat earcons.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kvoid.html` audio synthesis functions.
  2. Implement hardware composer chiptune standard:
     - 3-Voice counterpoint: Lead melody, counter-melody/arpeggio, and punchy FM bassline.
     - Shell voicings: Root + 3rd + 7th (omit 5ths).
     - Swing-tick groove: 16-step rhythmic swing.
     - SPC700 warm stereo slapback delay line (180ms with lowpass filtering).
  3. Pure Web Audio synthesis with zero external audio assets.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `KFortress`.
  - Set `current_agent: kilo-tester`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KFortress`, `KMech`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KTrader`
- **Primary File**: `KiloOS/public/apps/ktrader.html`
- **Objective**: Space trading simulation UI audit: market commodity exchanges, ship cargo holds, navigation route controls, warp travel dialogs, and save/load state.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KTrader`.
  2. Audit all UI elements: commodity buy/sell buttons, cargo capacity sliders, star map navigation, and hotkeys.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KTrader` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KMystery`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KMystery`, `KNet`, `KCalc`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KPass`
- **Primary File**: `KiloOS/public/apps/kpass.html`
- **Objective**: Retro password generator & security vault ergonomics: password entropy gauge, character set toggles, responsive master vault layout, clear keyboard shortcuts (F1/H help, Esc clear), and accessible focus glow.
- **Action Steps**:
  1. Verify window bounds and responsive container wrapping under narrow (<540px) widths.
  2. Ensure clear keyboard shortcuts (F1/H help, Esc clear) and accessible focus indicators.
  3. Verify password generation ergonomics, HiDPI font scaling, and comfortable contrast.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KVault`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KVault`, `KCalc`, `KHex`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `K2048`
- **Primary Files**: `KiloOS/public/apps/k2048.html`, `K2048/main.c` (if present)
- **Objective**: Pass 5 QA audit: verify first-run tutorial onboarding overlay (`k2048_tutorialSeen`), verify F5 quicksave & F9 quickload capture complete board state/score/undo history, test Escape modal dismissal, and ensure clean dual Win32 native compilation and web cartridge (<999 KB).
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `K2048/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app K2048`
  - `cd KiloOS && npm run build` (0 Vite errors, `k2048.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KSudoku`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KSudoku`, `KTetris`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KFont`
- **Primary Files**: `KiloOS/public/apps/kfont.html`, `KFont/main.c` (if present)
- **Objective**: Bitmap font studio feature expansion: multi-glyph canvas grid, BDF/FON/hex export & import, TTF rasterization preview, and character map inspector.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kfont.html`.
  2. Implement multi-glyph canvas grid, BDF/FON/hex export & import, TTF rasterization preview.
  3. Zero external dependencies; maintain responsive 1999 retro styling.
- **Verification**: `uv run scripts/test_app_startup.py --app KFont` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KRSS`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KRSS`, `KVault`, `KDB`.

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

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-18T19:20:00+0000 — kilo-qa: KSnake (Pass 5 QA Audit & Mature Quarantine)**
  - State Persistence: Verified F5 quicksave & F9 quickload capturing complete game state; enabled quickload on victory screen; ensured tutorial flag is safely recorded upon game restoration.
  - Tutorial & Onboarding: Confirmed first-run tutorial overlay (`ksnake_tutorialSeen` / `ksnake_tutorial.dat`) displays on fresh sessions and never intrudes on restored saves; added [T] tutorial shortcut to Win32 C menu.
  - Modal Dismissals & Ergonomics: Added close buttons and Escape/Enter/Space handlers to Game Over and Campaign Route Selection overlays; wired modal backdrop click dismissals.
  - Dual Parity & Build: Verified clean Win32 C build (`KSnake.exe`, 55.3 KB), startup test PASS (283.4 KB < 999 KB, 0 JS errors), Vite build clean (456ms), security lint PASS.
  - Quarantine & Queue: Promoted KSnake to Mature Applications quarantine table; advanced `kilo_qa` to K2048; rotation handoff to `kilo-expander`.

- **2026-10-18T18:36:00+0000 — kilo-usability: KCipher (UX Polish, HiDPI Canvas & Accessible Ergonomics)**
  - HiDPI Canvas Crispness: Integrated `window.devicePixelRatio` scaling into frequency chart canvas for razor-sharp rendering on high-DPI screens.
  - Ergonomics & Shortcuts: Added Esc key clear for active textareas/panes, added tutorial step 5 hotkeys manual, and updated footer hotkeys reference.
  - Accessible Focus & Navigation: Implemented theme-reactive `:focus-visible` glow rings on all interactive elements; added keyboard navigation to lore items.
  - Responsive Layout: Added responsive wrapping under <=768px for frequency, stego, lore, header, footer, and transform controls.
  - Dual Parity & Build: Added Esc clear handling and updated manual in Win32 C (`KCipher.exe`), clean build (11.5 KB), startup test PASS (111.5 KB < 999 KB, 0 JS errors), Vite build clean (487ms).
  - Queue Handoff: Advanced `kilo_usability` to `KPass`; rotated active agent to `kilo-qa`.

- **2026-10-18T18:20:00+0000 — kilo-tester: KColony (UI Audit, Hotkey Hardening & Mature Quarantine)**
  - PASS ✅ (5 issues, 5 fixed): Fixed modal display check, repaired game loop intervals, added numpad/A hotkeys, wired active export autosave, and aligned structure panel order.
  - Interactive Audit: Verified all 20 structure buttons, 13 research paths, 3 expedition tiers, trade depot, orbital strike, and Escape/backdrop modal dismissals.
  - State Persistence: Confirmed F5 quicksave, F9 quickload, JSON import/export, and first-run tutorial flag (`kcolony_tutorialSeen`) never intrudes on restored saves.
  - Verification: Clean Win32 C compilation (`KColony.exe`), startup test PASS (138.3 KB < 999 KB, 0 JS errors), Vite build clean (461ms), byte gate PASS.
  - Quarantine & Queue: Promoted KColony to Mature Applications quarantine; advanced `kilo_tester` to KTrader; rotation handoff to `kilo-usability`.

- **2026-10-18T16:15:00+0000 — kilo-soundscape: KDragon (Audio Audit & Mature Quarantine)**
  - Procedural Audio: Verified YM2612 2-Op FM / SPC700 chiptune suite (3 multi-voice tracks, 18 FM presets, swing-tick groove, delayed vibrato, stereo delay).
  - Verification: Clean Win32 C build (`KDragon.exe`), startup test PASS (196.1 KB < 999 KB, 0 JS errors), Vite build clean (475ms), security lint PASS.
  - Quarantine: Promoted KDragon to Mature Applications quarantine table; removed from active queues.
  - Queue: Advanced `kilo_soundscape` to `KVoid`, `kilo_graphics` to `KAlchemy`; rotation handoff to `kilo-tester`.

- **2026-10-18T15:12:02+0000 — kilo-graphics: KDragon (Zero-Token Auto-Skip — Inappropriate Target)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KDragon (pure vector, board game, or mature art).
  - Optimization: Handled via orchestrator pre-flight zero-token auto-skip.
  - Queue: Advanced `kilo-graphics` to `KDragon`; rotation handoff to `kilo-soundscape`.
