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
effort: high
timeout_minutes: 10
status: ready
current_targets:
  kilo_creator: kweb://warez
  kilo_graphics: KMech
  kilo_soundscape: KMech
  kilo_tester: KMystery
  kilo_usability: KVault
  kilo_qa: KMatch3
  kilo_expander: KVault
virtual_web_target: kweb://warez
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
  app: KFortress
  timestamp: '2026-10-19T14:16:00+0000'
last_planner_run: '2026-10-18T21:17:00Z'
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
- **Target**: `kweb://warez`
- **Primary File**: `KiloOS/public/web/warez.html`
- **Objective**: Underground scene & warez hypermedia node expansion: ANSI/NFO browser, cracktro tracker synthesizer, BBS tele-relay gateway, and demoscene software vault (<999 KB).
- **Upcoming Queue**: `kweb://geocities`, `kweb://users/~neon_rider`, `kweb://asm-temple`, `kweb://cybercafe`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KMech`
- **Primary Files**: `KiloOS/public/apps/kmech.html`, `KMech/main.c` (if present)
- **Objective**: Mech combat visual asset pipeline evaluation: mech chassis sprites, battlefield terrain textures, and UI HUD; eliminate any specular glints or traveling border dots.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kmech.html` canvas rendering routines for programmer art.
  2. Generate sprites via 2-stage asset pipeline if appropriate (`generate_image` with solid `#FF00FF` -> `scripts/asset_pipeline.py`).
  3. Search for and eliminate any traveling border dots or rotating specular glints.
  4. Run `python scripts/check_icons.py` to audit icon uniqueness.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KColosseum`.
  - Set `current_agent: kilo-soundscape`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KColosseum`, `KTrader`, `KSpace`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `KMech`
- **Primary Files**: `KiloOS/public/apps/kmech.html`, `KMech/main.c` (if present)
- **Objective**: Heavy mechanized combat chiptune soundscape: industrial titanium stomps, hydraulic servo whines, plasma cannon discharge, and battle arena theme.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kmech.html` audio synthesis functions.
  2. Implement hardware composer chiptune standard:
     - 3-Voice counterpoint: Lead melody, counter-melody/arpeggio, and punchy FM bassline.
     - Shell voicings: Root + 3rd + 7th (omit 5ths).
     - Swing-tick groove: 16-step rhythmic swing.
     - SPC700 warm stereo slapback delay line (180ms with lowpass filtering).
  3. Pure Web Audio synthesis with zero external audio assets.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `KColosseum`.
  - Set `current_agent: kilo-tester`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KColosseum`, `KTrader`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KMystery`
- **Primary File**: `KiloOS/public/apps/kmystery.html`
- **Objective**: Detective mystery interactive UI audit: crime scene clue inspection, suspect interrogation dialogue choices, inventory evidence management, case docket notebook, and F5/F9 state persistence.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KMystery`.
  2. Audit all UI elements: clue investigation buttons, interrogation tree dialogs, evidence notebook, and hotkeys.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KMystery` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KChart`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KChart`, `KClip`, `KTrader`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KVault`
- **Primary File**: `KiloOS/public/apps/kvault.html`
- **Objective**: Encrypted document & archive vault ergonomics: file tree hierarchy, responsive preview pane, clear keyboard shortcuts (F1/H help, Esc clear), and accessible drag-and-drop focus glow.
- **Action Steps**:
  1. Verify window bounds and responsive container wrapping under narrow (<540px) widths.
  2. Ensure clear keyboard shortcuts (F1/H help, Esc clear) and accessible focus indicators.
  3. Verify document preview ergonomics, HiDPI font scaling, and comfortable contrast.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KFont`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KFont`, `KHex`, `KPass`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KMatch3`
- **Primary Files**: `KiloOS/public/apps/kmatch3.html`, `KMatch3/main.c` (if present)
- **Objective**: Pass 5 QA audit: verify first-run tutorial onboarding overlay (`kmatch3_tutorialSeen`), verify F5 quicksave & F9 quickload capturing complete game board state, score, level, and active powerups; test Escape modal dismissal; compile Win32 native client and verify web cartridge (<999 KB).
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KMatch3/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KMatch3`
  - `cd KiloOS && npm run build` (0 Vite errors, `kmatch3.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KSimon`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KSimon`, `KWords`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KVault`
- **Primary Files**: `KiloOS/public/apps/kvault.html`, `KVault/main.c` (if present)
- **Objective**: Retro encrypted vault & secrets manager expansion: multi-vault profiles, AES/XOR cipher suites, password entropy meter, custom field schema builder, and CSV/JSON secure import/export (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kvault.html`.
  2. Implement multi-vault management, cipher tools, entropy metrics, and schema expansion.
  3. Zero external dependencies; maintain responsive 1999 retro styling.
- **Verification**: `uv run scripts/test_app_startup.py --app KVault` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KDB`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KDB`, `KHex`, `KPing`.

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

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-19T14:16:00+0000 — kilo-soundscape: KFortress (Siege of the Iron Citadel & Vigil Soundscape)**
  - Dual Movements: Composed "Citadel in Vigil" (peacetime D Aeolian meditation) and "Siege of the Iron Citadel" (combat D Phrygian Dominant martial drive).
  - Voice Leading: 3-voice counterpoint with call-and-response lead/counter, root+3rd+7th shell voicings (no 5ths), and galloping 16th slap bass.
  - Patch Suite: Authored 9 YM2612 patches (Slap Bass, Distorted Lead w/ LFO, Hollow Flute, War Brass, Crystal Harp, Sub Drone, FM Kick/Snare/Hat).
  - DSP & Earcons: Wired SPC700 185ms/2200Hz delay selectively; upgraded 15+ action stingers/earcons (masonry build, gold coins, war horn, funeral dirge).
  - Parity & Verification: Win32 C clean compile (182.8 KB), startup test PASS (225.4 KB < 999 KB, 0 JS err), Vite clean (461ms), security lint PASS.
  - Quarantine & Handoff: Promoted KFortress to Mature Applications; advanced kilo_soundscape to KMech; rotation handoff to kilo-tester.

- **2026-10-19T12:40:00+0000 — kilo-graphics: KFortress (Imagen 3 Visual Assets & Pipeline Integration)**
  - Asset Generation & Stage 2: Generated authentic Imagen 3 assets via 2-stage pipeline (`scripts/asset_pipeline.py`); keyed magenta backgrounds to 32-bit RGBA sprites.
  - Sprite & Texture Registry: Integrated `citadel_keep.png`, `rift_gate.png`, `enemy_orc.png`, `enemy_goblin.png`, `enemy_skeleton.png`, `enemy_golem.png`, `enemy_wyvern.png`, and `kfortress_bg.png`.
  - Engine Rendering: Upgraded canvas drawing in `kfortress.html` with modular `drawSpriteAsset` and textured terrain background overlay, preserving resilient procedural fallbacks.
  - Glint & Border Ban: Verified clean static citadel architecture and filigree borders with zero traveling perimeter dots or rotating specular glints across Web and Win32 C.
  - Verification & Budget: Startup test PASS (213.5 KB < 999 KB, 0 JS errors, hit-test unblocked); Win32 C clean compile (182.8 KB); Vite clean (497ms); icon audit PASS (105/105); security lint PASS.
  - Queue Handoff: Advanced current_targets.kilo_graphics to KMech; rotation handoff to kilo-soundscape.

- **2026-10-19T11:40:00+0000 — kilo-creator: kweb://webring (RingMaster v2.4 Guided Webmaster Enrollment Expansion)**
  - RingMaster v2.4 Accreditation Wizard: Built 4-stage guided webmaster onboarding (Identity, Charter Audit, Neighbor Routing, Badge Customizer).
  - Pre-flight Charter Diagnostics: Automated checks for HTML 4.01 strict semantics, <999KB payload budget, zero commercial trackers, and parody rules.
  - Interactive Topology & Widgets: Dynamic predecessor placement in ring loop, live neighbor preview, and 4 retro widget archetypes (Table, Win95, Neon, 88x31).
  - Local Member Registry: Full client-side member management with localStorage persistence, live directory/topology sync, and tour hop navigation.
  - Verification & Budget: Startup audit PASS (402.2 KB < 999 KB, 0 JS errors, CSS valid, clean startup), Vite build clean (529ms), security lint clean.
  - Queue Handoff: Advanced current_targets.kilo_creator to kweb://warez; rotation handoff to kilo-graphics.

- **2026-10-19T08:34:00+0000 — kilo-expander: KRSS (Retro RSS Reader Feature Expansion)**
  - Syndication & OPML: Added deterministic XML/JSON parsing for RSS 0.91/2.0, Atom 1.0, JSON Feed 1.1, and round-trip OPML 2.0 import/export.
  - 1999 Offline News Spool: Implemented local disk caching spool, 56k dial-up ISP simulation toggle, and standalone offline reader HTML export.
  - Filtering & Rules: Added headline category filtering dropdown/badges, and configurable keyword alert rules with audio chimes and auto-starring.
  - Ergonomics: Integrated Bionic speed-scan mode, reader width/font presets, reading progress bar, bottom article navigation cards, and quote clipping.
  - Parity & Verification: Win32 C clean compile (22.0 KB), startup check PASS (192.4 KB < 999 KB, 0 JS err, unblocked), Vite clean (516ms), security lint PASS.
  - Queue Handoff: Advanced current_targets.kilo_expander to KVault; rotation handoff to kilo-creator.

- **2026-10-19T07:18:00+0000 — kilo-qa: KFreecell (Pass 5 QA Audit & Mature Promotion)**
  - State Persistence: Audited F5 quicksave & F9 quickload capturing complete tableau, freecells, foundations, undo stack, stats, and timer across Web and Win32 C.
  - Tutorial & Modals: First-run onboarding banner / Help dialog fires once on fresh session (`kfreecell_tutorialSeen`), never interrupting restored saves; clean Esc/Enter/Space dismissals.
  - Scoping & Storage: Refactored `ensureGameTimer` to top-level scope; audited storage quota safety (`safeGet`/`safeSet`) and `beforeunload` interval teardown.
  - Glint & Mystery: Confirmed static perimeter inlay, zero specular glints/dots, zero un-diegetic tags, full anti-trademark compliance.
  - Verification & Parity: Win32 C compile clean (`KFreecell.exe`, 129.5 KB), startup test PASS (105.6 KB, 0 JS errors, hit-test unblocked), Vite build clean (491ms), security lint PASS.
  - Quarantine & Handoff: Promoted KFreecell to Mature Applications quarantine; advanced `kilo_qa` to `KMatch3`; rotation handoff to `kilo-expander`.

