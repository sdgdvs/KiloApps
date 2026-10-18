---
current_agent: kilo-expander
next_agent: kilo-creator
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
  kilo_creator: kweb://deep-core
  kilo_graphics: KDragon
  kilo_soundscape: KDragon
  kilo_tester: KColony
  kilo_usability: KCipher
  kilo_qa: KSnake
  kilo_expander: KBookmark
virtual_web_target: kweb://deep-core
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
  app: KMaze
  timestamp: 2026-10-18T11:20:00+0000
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
- **Target**: `kweb://deep-core`
- **Primary File**: `KiloOS/public/web/deep_core.html`
- **Objective**: Subterranean deep core command terminal & ARG climax precursor: seismic waveguide telemetry, subterranean relay packet analysis, and Tier 3 Anti-Potemkin depth (<999 KB).
- **Upcoming Queue**: `kweb://darknet`, `kweb://portal`, `kweb://webring`, `kweb://warez`.

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
- **Target**: `KDragon`
- **Primary Files**: `KiloOS/public/apps/kdragon.html`, `KDragon/main.c` (if present)
- **Objective**: Authentic Genesis YM2612 2-Op FM / SPC700 stereo delay chiptune suite: fantasy dragon arena & wyrm battle soundscape, breath roar stings, and draconian combat earcons.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kdragon.html` audio synthesis functions.
  2. Implement hardware composer chiptune standard:
     - 3-Voice counterpoint: Lead melody, counter-melody/arpeggio, and punchy FM bassline.
     - Shell voicings: Root + 3rd + 7th (omit 5ths).
     - Swing-tick groove: 16-step rhythmic swing.
     - SPC700 warm stereo slapback delay line (180ms with lowpass filtering).
  3. Pure Web Audio synthesis with zero external audio assets.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `KVoid`.
  - Set `current_agent: kilo-tester`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KVoid`, `KColony`, `KFortress`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KColony`
- **Primary File**: `KiloOS/public/apps/kcolony.html`
- **Objective**: Space colony simulation UI audit: module construction controls, resource sliders, life support telemetry, disaster event dialogs, and save/load state.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KColony`.
  2. Audit all UI elements: construction panel, worker allocations, trade depot, event modals, and hotkeys.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KColony` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KTrader`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KTrader`, `KMystery`, `KNet`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KCipher`
- **Primary File**: `KiloOS/public/apps/kcipher.html`
- **Objective**: Classical & modern cryptography studio layout: cipher tool ribbon, responsive side-by-side plaintext/ciphertext split panes, frequency analysis histogram scaling, keyboard shortcuts (F1/H help, Esc clear), and accessible focus glow.
- **Action Steps**:
  1. Verify window bounds and responsive container wrapping under narrow (<540px) widths.
  2. Ensure clear keyboard shortcuts (F1/H help, Esc clear) and accessible focus indicators.
  3. Verify cipher workspace ergonomics, HiDPI font scaling, and comfortable contrast.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KPass`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KPass`, `KVault`, `KCalc`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KSnake`
- **Primary Files**: `KiloOS/public/apps/ksnake.html`, `KSnake/main.c` (if present)
- **Objective**: Pass 5 QA audit: verify first-run tutorial onboarding overlay (`ksnake_tutorialSeen`), verify F5 quicksave & F9 quickload capture complete snake state/difficulty/score, test Escape modal dismissal, and ensure clean dual Win32 native compilation and web cartridge (<999 KB).
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KSnake/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KSnake`
  - `cd KiloOS && npm run build` (0 Vite errors, `ksnake.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `K2048`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `K2048`, `KSudoku`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KBookmark`
- **Primary Files**: `KiloOS/public/apps/kbookmark.html`, `KBookmark/main.c` (if present)
- **Objective**: Deep bookmark manager expansion: hierarchical folders, tagging system, Netscape bookmark HTML format import/export, link health pings, and rich metadata.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kbookmark.html`.
  2. Implement nested directory hierarchies, tag indexing, Netscape bookmark file import/export, and quick link launcher.
  3. Zero external dependencies; maintain responsive 1999 retro styling.
- **Verification**: `uv run scripts/test_app_startup.py --app KBookmark` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KFont`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KFont`, `KRSS`, `KVault`.

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

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-18T11:20:00+0000 — kilo-qa: KMaze (Pass 5 QA & State Integrity — Quarantined to Mature)**
  - First-run Tutorial Integrity: Verified `kmaze_tutorialSeen` / `kmaze_tutorial.dat` onboarding triggers on fresh sessions, never interrupting restored saves.
  - State Persistence: Audited F5/F9 quicksave/quickload across Web and Win32 C; hardened storage quota handling and cleaned path tiles, screen shake, and damage timers on load.
  - Modal & Key Ergonomics: Fixed Escape key handling to prevent accidental new game triggers on title splash; wired Escape to toggle in-game Help/Pause modal in Web and Win32 C.
  - Parity & Verification: Clean Win32 C compilation (62.5 KB), startup test PASS (150.0 KB < 999 KB, 0 JS err, hit-test unblocked), Vite build clean (497ms).
  - Quarantine & Queue: Promoted KMaze to Mature Applications quarantine; advanced kilo_qa to KSnake; rotation handoff to kilo-expander.

- **2026-10-18T08:37:00+0000 — kilo-usability: KCalc (UX, Themes & Live Radix Polish)**
  - Dual-Target Readout Themes: Implemented 4 high-contrast display themes (OLED Slate, Retro Olive LCD, Amber Plasma, VFD Cyan) with 'L' hotkey cycling across Web and Win32 C.
  - Multi-Base Live Strip: Added live HEX / DEC / OCT / BIN radix readout with click-to-copy in display card.
  - Keyboard & Layout Ergonomics: Added accessible :focus-visible glow rings; widened native client bounds to 364px to eliminate control clipping; updated Help dialog (F1/H) with 'L' theme shortcut.
  - State Persistence: Persisted display theme in localStorage, autoSaveSession, and F5/F9 quicksave/quickload snapshots.
  - Verification: Clean Vite build (491ms), native C clean build (28.6 KB), startup test PASS (239.5 KB < 999 KB, 0 JS errors), security lint clean.
  - Queue: Advanced kilo_usability to KCipher; rotation handoff to kilo-qa.

- **2026-10-18T06:36:00+0000 — kilo-tester: KNet (Interactive UI Audit & Navigation Engine Fixes)**
  - Status: PASS ✅ (5 issues, 5 fixed).
  - Identifier Fix: Replaced undeclared `hexViewer` with `httpHexArea` in `quickSave()` and `beforeunload`, preventing ReferenceErrors during state snapshots.
  - Audio Param Fix: Defaulted `latency = 45` in `playPingTone()`, preventing non-finite NaN errors in Web Audio synthesizer during peer pings.
  - Routing & IPC: Added `location.hash` / `search` launch URL parsing, `hashchange`, and origin-validated `postMessage` listener (`NAVIGATE`/`KILO_OPEN_URL`).
  - Viewport Sync: Implemented same-origin `httpWebFrame` load listener updating address bar, history, and source/hex inspector buffers on hypermedia clicks.
  - Parity & Verification: Verified Win32 native C client (`build.bat` 45 KB), startup test PASS (189.8 KB < 999 KB, 0 JS errors), Vite build clean (466ms).
  - Queue: Advanced `kilo_tester` to `KColony`; rotation handoff to `kilo-usability`.

- **2026-10-18T05:38:00+0000 — kilo-soundscape: KAlchemy ("The Hermetic Transmutation" — Mystical Laboratory FM Suite)**
  - Composition: D Dorian / Andalusian cadence (Dm -> C -> Bb/G -> A7), 32 steps, 6 channels, shell voicings (no 5ths).
  - Voice Leading & Patches: Mystic Flute/E-Piano lead (delayed LFO vibrato), Crystalline Celesta counter, Slap/Wood bass, SPC700 delay.
  - Earcons: Upgraded bubbling simmer, dual FM crystal glass clinkers, electric transmutation zap, and 5-note pentatonic discovery chime.
  - Ergonomics: Wired M key toggle & header badge state sync; persisted mute preference in localStorage.
  - Verification: Clean Vite build (503ms), startup test PASS (253.0 KB < 999 KB, 0 JS err), security lint clean, native C clean (70.6 KB).
  - Queue: Advanced kilo-soundscape to KDragon; rotation handoff to kilo-tester.

- **2026-10-18T04:30:01+0000 — kilo-graphics: KDragon (Zero-Token Auto-Skip — Inappropriate Target)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KDragon (pure vector, board game, or mature art).
  - Optimization: Handled via orchestrator pre-flight zero-token auto-skip.
  - Queue: Advanced `kilo-graphics` to `KDragon`; rotation handoff to `kilo-soundscape`.
