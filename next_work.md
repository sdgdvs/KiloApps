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
model: gemini-3.8-flash-high
effort: high
timeout_minutes: 10
status: ready
current_targets:
  kilo_creator: kweb://users/~neon_rider
  kilo_graphics: KColosseum
  kilo_soundscape: KColosseum
  kilo_tester: KClip
  kilo_usability: KFont
  kilo_qa: KSimon
  kilo_expander: KDB
virtual_web_target: kweb://users/~neon_rider
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
  agent: kilo-tester
  app: KChart
  timestamp: '2026-10-20T05:22:00Z'
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
- **Target**: `kweb://users/~neon_rider`
- **Primary File**: `KiloOS/public/web/users/neon_rider.html`
- **Objective**: Expand late-90s netrunner homestead: x86 assembly tutorial fragments, tracker .XM/.MOD showcase, retro demoscene greetings, Webring badges, and diegetic ARG breadcrumbs adhering to Anti-Potemkin Quality Standard (<999 KB).
- **Upcoming Queue**: `kweb://asm-temple`, `kweb://cybercafe`, `kweb://10.19.99.4/classified`.

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
- **Target**: `KFont`
- **Primary File**: `KiloOS/public/apps/kfont.html`
- **Objective**: Bitmap font designer and typography workbench ergonomics: responsive glyph grid, zoom canvas controls, preview text wrap, and export shortcut discoverability.
- **Action Steps**:
  1. Inspect layout adaptability across standard and compact window dimensions.
  2. Verify keyboard navigation across glyph table and toolbar action triggers.
  3. Ensure high-contrast accessibility and crisp canvas rendering.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KHex`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KHex`, `KPass`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KSimon`
- **Primary Files**: `KiloOS/public/apps/ksimon.html`, `KSimon/main.c` (if present)
- **Objective**: Pass 5 QA audit: verify first-run tutorial onboarding overlay (`ksimon_tutorialSeen`), verify F5 quicksave & F9 quickload capturing complete game sequence, round, speed, and high score; test Escape modal dismissal; compile Win32 native client and verify web cartridge (<999 KB).
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KSimon/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KSimon`
  - `cd KiloOS && npm run build` (0 Vite errors, `ksimon.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KWords`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KWords`, `KColosseum`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KDB`
- **Primary Files**: `KiloOS/public/apps/kdb.html`, `KDB/main.c` (if present)
- **Objective**: Retro flat-file relational database & SQL-like engine expansion: multi-table joins, relational schema builder, index simulation, SQL query runner, and CSV/JSON export/import (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kdb.html`.
  2. Implement relational query capabilities, indexing tools, and schema expansion.
  3. Ensure dual Win32 native client parity and responsive 1999 styling.
- **Verification**: `uv run scripts/test_app_startup.py --app KDB` and `cd KiloOS && npm.cmd run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KHex`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KHex`, `KPing`.

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

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-20T05:22:00Z — kilo-tester: KChart (UI Audit, Inline Fixes & Mature Promotion)**
  - UI & Interaction Audit: Audited 10 visualization engines, 7 trend/regression overlays, YM2612 FM / SPC700 sonification sweep, 7 export formats (SVG, PNG, MD, C, HTML, CSV, JSON), and table editing.
  - Inline Repairs: Added active sonification cancellation on table edits, transforms, and arrow nudges; fixed index bounds validation for inline cell edits; added hoveredIndex adjustment on point deletion.
  - Parity & Verification: Startup check PASS (131.9 KB < 999 KB, CSS valid, 0 JS err, modal dismisses cleanly); native Win32 C clean (`KChart.exe`, 36.0 KB); Vite build clean (475ms).
  - Quarantine & Handoff: Promoted KChart to Mature Applications quarantine; advanced `kilo_tester` to `KClip`; rotation handoff to `kilo-usability`.

- **2026-10-20T03:16:00Z — kilo-soundscape: KMech ("Mech Bay 04" & "Titan Clash")**
  - Audio Architecture: Dual-mode YM2612 2-Op FM + SPC700 stereo delay (180ms, 2200Hz LP, 0.28 FB) chiptune engine.
  - Tracks & Form: Track 01 Hangar theme (D Dorian/Aeolian, 32 steps) + Track 02 Arena Combat (D Phrygian Dominant & Andalusian descent, 64 steps) with 5:3 swing groove.
  - Voice-Leading: 6-channel arrangement with contrary motion, shell voicings (no doubled 5ths), and call-and-response counterpoint.
  - Patches & Earcons: 13 named hardware FM voices (slap bass, distorted guitar, Rhodes, servo whine, hydraulic stomp, steam vents, klaxon, fanfares).
  - Verification: Startup check PASS (158.5 KB < 999 KB, 0 JS err); Win32 C build clean (`KMech.exe`, 33.8 KB); Vite build clean; security lint clean.
  - Queue Handoff: Advanced `current_targets.kilo_soundscape` to `KColosseum`; rotation handoff to `kilo-tester`.

- **2026-10-20T00:16:00Z — kilo-graphics: KMech**
  - ⏭️ Skip — Imagen 3 asset replacement not appropriate for KMech
  - Asset Suitability: Evaluated KMech CRT vector/wireframe aesthetic; dynamic SVG/GDI polygon mechs and HUD are foundational retro wireframe art, inappropriate for raster replacement.
  - Glint & Comet Audit: Verified zero traveling border dots, specular glints, or orbiting particles in Web HTML or Win32 GDI.
  - Parity & Verification: Win32 native client compiled cleanly (`KMech.exe`, 33.8 KB); startup check PASS (125.9 KB < 999 KB, 0 JS err, unblocked); Vite build clean.
  - Queue Handoff: Advanced `current_targets.kilo_graphics` to `KColosseum`; rotation handoff to `kilo-soundscape`.

- **2026-10-19T23:38:30Z — kilo-creator: kweb://geocities (SunsetStrip & Heartland Expansion + Animated Decal Studio)**
  - Neighborhood Expansion: Added SunsetStrip (rock/grunge) and Heartland (prairie/crafts) CSS themes, explorer tabs, and 10 deep interactive homesteads with real Web Audio riffs, tube amp simulator, recipe cards, quilt designer, and banjo rolls.
  - Animated Decal Studio: Built procedural '99 animated GIF decal generator & stamping pad (digging worker, flaming text, 3D cyber globe, flying email, pulsing starburst, dancing hamster, compass, beacon) with drop-in HTML export.
  - Soundscape & Jukebox: Composed 2 new hardware chiptune tracks: Track 06 "Sunset Boulevard Overdrive" (140 BPM Grunge) and Track 07 "Prairie Solstice Bluegrass" (124 BPM Banjo) with Genesis 2-Op FM / SPC700 stereo delay.
  - Passport & Guestbook: Expanded Cyberspace Passport to 9 stamps (including SunsetStrip Rocker & Heartland Homesteader, 35 homesteads); added guestbook filters and quick stamps.
  - Verification & Parity: Web file verified clean (652 KB < 999 KB); startup test PASS (KNet 189.8 KB, 0 JS errors); Vite build clean (508ms); security lint clean.
  - Queue Handoff: Advanced `current_targets.kilo_creator` to `kweb://users/~neon_rider`; rotation handoff to `kilo-graphics`.

- **2026-10-19T21:22:04+0000 — kilo-expander: KVault (Multi-Vault Profiles & Structured Schema Templates)**
  - Multi-Vault Profiles: Implemented 3 vault profiles (Primary Vault, Work & Ops, Classified Mesh) with dedicated storage keys and per-profile `.dat` persistence.
  - Schema Templates: Added 8 structured templates (Login, Finance, Server/SSH, API Key, Passkey/Seed, 1999 BBS, Database, Router) for rapid credential generation.
  - Hotkey & Switching: Added `[F6]` multi-vault profile switcher in both Web and Win32 C, preserving clean auto-lock and quicksave integrity.
  - Dual Parity & Build: Recompiled Win32 native client (`KVault.exe`, 22.0 KB); startup test PASS (268.4 KB < 999 KB, 0 JS errors, hit-test unblocked); Vite build clean.
  - Queue Handoff: Advanced `current_targets.kilo_expander` to `KDB`; rotation handoff to `kilo-creator`.
