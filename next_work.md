---
current_agent: kilo-expander
next_agent: kilo-creator
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
  kilo_creator: kweb://webring
  kilo_graphics: KFortress
  kilo_soundscape: KFortress
  kilo_tester: KMystery
  kilo_usability: KVault
  kilo_qa: KMatch3
  kilo_expander: KRSS
virtual_web_target: kweb://webring
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
  agent: kilo-qa
  app: KFreecell
  timestamp: '2026-10-19T07:18:00+0000'
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
- **Target**: `kweb://webring`
- **Primary File**: `KiloOS/public/web/webring.html`
- **Objective**: Central Webring Hub & RingMaster v2.4 deep expansion: interactive member hub traversal, ring topology visualizer, custom ring joining wizard, and retro webring badge generator (<999 KB).
- **Upcoming Queue**: `kweb://warez`, `kweb://geocities`, `kweb://users/~neon_rider`, `kweb://asm-temple`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KFortress`
- **Primary Files**: `KiloOS/public/apps/kfortress.html`, `KFortress/main.c` (if present)
- **Objective**: Stronghold and fortress visual asset pipeline evaluation: terrain/tile textures, entity sprites, and UI HUD; eliminate any specular glints or traveling border dots.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kfortress.html` canvas rendering routines for programmer art.
  2. Generate sprites via 2-stage asset pipeline if appropriate (`generate_image` with solid `#FF00FF` -> `scripts/asset_pipeline.py`).
  3. Search for and eliminate any traveling border dots or rotating specular glints.
  4. Run `python scripts/check_icons.py` to audit icon uniqueness.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KMech`.
  - Set `current_agent: kilo-soundscape`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KMech`, `KColosseum`, `KTrader`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `KFortress`
- **Primary Files**: `KiloOS/public/apps/kfortress.html`, `KFortress/main.c` (if present)
- **Objective**: Stronghold siege chiptune soundscape: martial battle fanfare, stone siege drum cadence, and fortress victory stingers.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kfortress.html` audio synthesis functions.
  2. Implement hardware composer chiptune standard:
     - 3-Voice counterpoint: Lead melody, counter-melody/arpeggio, and punchy FM bassline.
     - Shell voicings: Root + 3rd + 7th (omit 5ths).
     - Swing-tick groove: 16-step rhythmic swing.
     - SPC700 warm stereo slapback delay line (180ms with lowpass filtering).
  3. Pure Web Audio synthesis with zero external audio assets.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `KMech`.
  - Set `current_agent: kilo-tester`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KMech`, `KColosseum`.

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
- **Target**: `KRSS`
- **Primary Files**: `KiloOS/public/apps/krss.html`, `KRSS/main.c` (if present)
- **Objective**: Retro RSS reader feature expansion: OPML feed import/export, simulated 1999 offline caching, headline category filtering, keyword notification rules, and article reading ergonomics (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/krss.html`.
  2. Implement OPML import/export, offline news store, keyword alerts, and headline filtering.
  3. Zero external dependencies; maintain responsive 1999 retro styling.
- **Verification**: `uv run scripts/test_app_startup.py --app KRSS` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KVault`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KVault`, `KDB`, `KHex`.

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

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-19T07:18:00+0000 — kilo-qa: KFreecell (Pass 5 QA Audit & Mature Promotion)**
  - State Persistence: Audited F5 quicksave & F9 quickload capturing complete tableau, freecells, foundations, undo stack, stats, and timer across Web and Win32 C.
  - Tutorial & Modals: First-run onboarding banner / Help dialog fires once on fresh session (`kfreecell_tutorialSeen`), never interrupting restored saves; clean Esc/Enter/Space dismissals.
  - Scoping & Storage: Refactored `ensureGameTimer` to top-level scope; audited storage quota safety (`safeGet`/`safeSet`) and `beforeunload` interval teardown.
  - Glint & Mystery: Confirmed static perimeter inlay, zero specular glints/dots, zero un-diegetic tags, full anti-trademark compliance.
  - Verification & Parity: Win32 C compile clean (`KFreecell.exe`, 129.5 KB), startup test PASS (105.6 KB, 0 JS errors, hit-test unblocked), Vite build clean (491ms), security lint PASS.
  - Quarantine & Handoff: Promoted KFreecell to Mature Applications quarantine; advanced `kilo_qa` to `KMatch3`; rotation handoff to `kilo-expander`.

- **2026-10-19T05:35:00+0000 — kilo-usability: KPass (Usability & Layout Polish)**
  - Responsive Layout: Added responsive wrapping for presets, control rows, and save form down to <540px and <440px without clipped buttons.
  - Window Sizing: Adjusted default dimensions to 640x720 in `App.jsx` and `kpass.html` for complete visibility of 4-mode controls and vault.
  - Keyboard & Ergonomics: Enhanced Esc handling in Web and Win32 C to dismiss modals, clear search, and reset filter dropdowns with toast feedback.
  - Visual Polish: Upgraded contrast for muted hints/labels (#888->#b0b0b0, #666->#949494) and unified focus-visible outlines.
  - Verification & Parity: Verified clean Win32 C compile (`KPass.exe`, 25.6 KB), startup test PASS (129.8 KB < 999 KB, 0 JS errors), Vite build clean (736ms), security lint PASS.
  - Queue Handoff: Advanced kilo_usability to KVault; rotation handoff to kilo-qa.

- **2026-10-19T03:38:00+0000 — kilo-tester: KTrader (Interactive UI Audit & Inline Fixes)**
  - UI Controls: Added Buy Max button for rapid cargo loading; wired emergency solar collector fallback to [R] hotkey when fuel is zero.
  - Ghost Bounty Bug: Fixed state retention bug in bountyTarget across combat flee, bribe, defeat, and mission abandonment in Web and Win32 C.
  - State & Parity: Added closeAllModals to quickload/import routines; guarded activeMission reward lookup against null; dual parity verified.
  - Verification & Sizes: Startup test PASS (99.0 KB < 999 KB, 0 JS errors, CSS valid, hit-test unblocked); Win32 C clean compile (26.0 KB); Vite build clean (459ms); security lint PASS.
  - Queue Handoff: Advanced kilo_tester to KMystery; rotation handoff to kilo-usability.

- **2026-10-19T02:40:00+0000 — kilo-soundscape: KVoid ("Derelict Bulkhead Protocol" — Cosmic Void Chiptune)**
  - Composition: D Minor / D Phrygian progression with Lydian Horizon shift (#11 on Bb Maj7), 6 logical voices, 64-step sequence.
  - Patches: YM2612 2-Op FM Slap Bass, Hollow Flute with delayed vibrato, DX7 Cosmic Bell, Void Pad shell voicings, inharmonic hats.
  - Architecture: SNES SPC700 stereo delay DSP (185ms, 2.2kHz LP filter, 30% feedback), 5:3 swing groove, urgent tempo acceleration.
  - Earcons & Stingers: Tri-color keycard pickup jingles, 1999Hz ARG terminal chirp, victory escape fanfare, quicksave/load chimes.
  - Status: PASS ✅ (Vite clean 470ms, 122.6 KB < 999 KB, startup check PASS, Win32 C build clean, security lint PASS).
  - Queue: Advanced kilo_soundscape to KFortress; rotation handoff to kilo-tester.

- **2026-10-19T01:36:00+0000 — kilo-graphics: KAlchemy (Asset Suitability Audit & Glint Removal)**
  - ⏭️ Skip — Imagen 3 asset replacement not appropriate for KAlchemy
  - Suitability Assessment: Confirmed alchemical codex/recipe puzzle card interface with emoji elements & particle canvas (<999 KB: 258.3 KB).
  - Glint Removal: Eliminated diagonal specular sheen sweep from crucible vessel in both Win32 C (`main.c`) and Web HTML (`kalchemy.html`).
  - Perimeter Dots: Removed `rotateRune` continuous perimeter rotation to ensure static, period-accurate mystical border.
  - Verification & Parity: Verified clean Win32 C build (`KAlchemy.exe`, 70.1 KB), startup test PASS (252.3 KB, 0 JS errors), Vite build clean, security lint PASS.
  - Queue Handoff: Advanced `kilo_graphics` to `KFortress`; rotation handoff to `kilo-soundscape`.
