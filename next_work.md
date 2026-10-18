---
current_agent: kilo-creator
next_agent: kilo-graphics
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
  kilo_tester: KNet
  kilo_usability: KCalc
  kilo_qa: KMaze
  kilo_expander: KBookmark
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
  agent: kilo-expander
  app: KJournal
  timestamp: 2026-10-17T23:56:00+0000
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
- **Target**: `KNet`
- **Primary File**: `KiloOS/public/apps/knet.html`
- **Objective**: Interactive UI element & browser controls audit: URL address bar, navigation history, bookmarked sites, search indexing, RFMS node pings, modal dialogs, and keyboard shortcuts.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KNet`.
  2. Audit all UI elements: address navigation, back/forward buttons, tab controls, bookmark triggers, and modals.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KNet` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KColony`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KColony`, `KTrader`, `KMystery`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KCalc`
- **Primary File**: `KiloOS/public/apps/kcalc.html`
- **Objective**: Scientific/programmer calculator responsive keypad layout, narrow-viewport (<540px) button scaling, keyboard shortcuts (F1/H help, Esc clear), accessible focus glow, and high-contrast LCD readout.
- **Action Steps**:
  1. Verify window bounds and responsive container wrapping under narrow (<540px) widths.
  2. Ensure clear keyboard shortcuts (F1/H help, Esc clear) and accessible focus indicators.
  3. Verify keypad ergonomics, HiDPI font scaling, and comfortable retro theme contrast.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KCipher`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KCipher`, `KPass`, `KVault`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KMaze`
- **Primary Files**: `KiloOS/public/apps/kmaze.html`, `KMaze/main.c` (if present)
- **Objective**: Pass 5 QA audit: add first-run tutorial onboarding overlay (`kmaze_tutorialSeen`), verify F5 quicksave & F9 quickload capture complete maze generation/timer/stats, test Escape modal dismissal, and ensure clean dual Win32 native compilation and web cartridge (<999 KB).
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KMaze/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KMaze`
  - `cd KiloOS && npm run build` (0 Vite errors, `kmaze.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KSnake`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KSnake`, `K2048`, `KSudoku`.

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

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-17T23:56:00+0000 — kilo-expander: KJournal (Deep Feature Expansion & Security Vault)**
  - Markdown Power Tools: Added tables (|...|), callout alerts (> [!NOTE|TIP|WARN]), mark highlights (==...==), and interactive checkbox callbacks syncing markdown state.
  - Day Index & Activity Matrix: Implemented 52-week activity heatmap, yearly stats, day/week index badge, and month jump navigation (Alt+D).
  - Hashtag Management Studio: Added batch tag rename and global tag deletion across all entries with frequency metrics and quick editor insertion chips.
  - Military-Grade Encrypted Vault: Added AES-256-GCM + PBKDF2 (100k iter) encrypted vault export/import (.kjv) and standalone self-decrypting offline HTML vaults.
  - Custom Templates Studio: Enabled user-defined prompt creation and library management alongside 11 built-in structured frameworks.
  - Dual Parity & Build Verification: Expanded Win32 C template list to 11 templates; verified clean build (`build.bat` 205 KB EXE, `npm run build`, `test_app_startup` PASS, <999 KB).
  - Status: PASS ✅ (Deep markdown, heatmap, vault security, tag studio, and dual parity verified).
  - Queue: Advanced `kilo_expander` to `KBookmark`; rotation handoff to `kilo-creator`.

- **2026-10-17T22:56:00+0000 — kilo-qa: KGo (Pass 5 QA & Tutorial Integrity — Mature Quarantine)**
  - State Persistence: Wrapped all storage in safeGet/safeSet; preserved timer remaining across quickload; persisted consecutivePasses, handicapStones, and tsumego index.
  - Tutorial Integrity: Enforced kgo_tutorialSeen web flag & kgo_tutorialSeen.dat native check on fresh sessions only; auto-dismisses on saved game restore.
  - Modal Dismissals & Shortcuts: Added accessible modal-close classes to help/mp dialogs; unified Escape key dismissal for modals & onboarding; added VK_ESCAPE in native C.
  - Mature Promotion: Promoted feature-complete KGo (Pass 5) to Mature Applications quarantine; removed from active worker queues.
  - Verification: Clean Vite build (npm run build 0 err), Win32 native compile (build.bat 0 err), test_app_startup PASS (168.1 KB < 999 KB ceiling).
  - Status: PASS ✅ (Full state persistence, clean modal handling, dual-target parity verified; promoted to Mature quarantine).
  - Queue: Advanced kilo_qa to KMaze; rotation handoff to kilo-expander.

- **2026-10-17T22:20:00+0000 — kilo-usability: KDB (UI/UX & Usability Pass)**
  - Responsive & Viewport Ergonomics: Added <540px mobile wrapping for header controls, cards-grid single-column fallback, sleek custom scrollbars, and touch scroll.
  - Modal UX & Keyboard Navigation: Added header rows with close buttons (&times;) across all 9 modals; unified Escape key to dismiss any active modal and blur search.
  - HiDPI Canvas & Visual Polish: Secured dimension fallbacks and middle baseline for charts, center KPI total readout on donut chart, and rAF debounced redraw on resize.
  - Accessible Focus: Added high-contrast focus rings (`outline-offset: 2px`, glow shadow), button active press micro-interactions, and '?' accelerator shortcut.
  - Verification: Clean Vite build (`npm.cmd run build` 0 err), Win32 native compile (`build.bat` 0 err), test_app_startup PASS (128.6 KB < 999 KB ceiling).
  - Status: PASS ✅ (Layout, keyboard navigation, HiDPI charts, and accessible focus verified).
  - Queue: Advanced `kilo_usability` to `KCalc`; rotation handoff to `kilo-qa`.

- **2026-10-17T21:55:00+0000 — kilo-tester: KPaint (Interactive UI Audit & Inline Fixes — Mature Quarantine)**
  - UI Element Audit: Verified 115 buttons, 12 inputs, 5 selects, 4 dialog modals, 22 filters/tools, and multi-layer reel animation controls.
  - Storage Safety: Wrapped audio preference, tutorial flags, quicksave snapshot, and collab handle in safe storage try/catch handlers per quality standards.
  - Keyboard & Modal UX: Elevated Escape key handler to dismiss modals/dropdowns from focused text inputs; wired handleExportFnKey preventing page scroll on Space.
  - Mature Promotion: Promoted feature-complete KPaint (Pass 6+) to Mature Applications quarantine table; removed from active queues.
  - Verification: Clean Vite build (`npm.cmd run build` 0 err), Win32 native compile (`build.bat` 0 err), test_app_startup PASS (242.6 KB), check_sizes PASS (<999 KB).
  - Status: PASS ✅ (3 issues fixed; promoted to Mature quarantine).
  - Queue: Advanced `kilo_tester` to `KNet`; rotation handoff to `kilo-usability`.

- **2026-10-17T21:38:00+0000 — kilo-soundscape: KFortress ("Siege of the Iron Citadel" — Combat / Defense FM Chiptune)**
  - Composition: 32-step D Minor / D Phrygian Dominant progression (Dm → C → Bb → A7); 6-channel architecture with vocal lead phrasing, contrary counterpoint, and shell brass stabs (no doubled 5ths).
  - Soundscape & Synthesis: Genesis YM2612 2-Op FM engine (overdriven lead, hollow flute, war horns, slap bass, inharmonic kick/snare/hat) + SPC700 185ms stereo slapback delay line (2200Hz LP filter).
  - Dynamic Adaptation & SFX: Peaceful ambient drone / build phase vs. galloping battle march; modernized 14 siege earcons & spells plus defeat dirge.
  - Accessibility & Modal Fix: Attached standard modal-close classes & titles to helpModal, fixing startup test automation.
  - Status: PASS ✅ (Vite clean, 214.5 KB HTML + 182.8 KB Win32 EXE < 999 KB ceiling, test_app_startup PASS, security lint 100% clean).
  - Queue: Advanced `kilo_soundscape` to `KAlchemy`; rotation handoff to `kilo-tester`.
