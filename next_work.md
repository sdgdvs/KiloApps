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
  kilo_creator: kweb://cybercafe
  kilo_graphics: KWizard
  kilo_soundscape: kweb://asm-temple
  kilo_tester: KPaint
  kilo_usability: KDB
  kilo_qa: KMinesweeper
  kilo_expander: KClip
virtual_web_target: kweb://cybercafe
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
  app: KHangman
  timestamp: 2026-10-17T17:18:25+0000
last_planner_run: '2026-10-16T20:35:00Z'
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
- **Objective**: Expand Central KiloNet 1999 Webring Hub, random teleporter, and badge studio (<999 KB).
- **Upcoming Queue**: `kweb://warez`, `kweb://geocities`, `kweb://users/~neon_rider`, `kweb://asm-temple`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KWizard`
- **Primary Files**: `KiloOS/public/apps/kwizard.html`, `KWizard/main.c` (if present)
- **Objective**: Card battle arena visual overhaul: generate spell card illustration sprites & fantasy arena background textures with Imagen 3 pipeline; ensure 0 specular glints.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kwizard.html` canvas rendering routines for programmer art.
  2. Generate spell card / arena battle sprites via 2-stage asset pipeline (`generate_image` with solid `#FF00FF` -> `scripts/asset_pipeline.py`).
  3. Search for and eliminate any traveling border dots or rotating specular glints.
  4. Run `python scripts/check_icons.py` to audit icon uniqueness.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KDragon`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KDragon`, `KAlchemy`, `KFortress`, `KSpace`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `kweb://cybercafe`
- **Primary File**: `KiloOS/public/web/cybercafe.html`
- **Objective**: Authentic YM2612 2-Op FM / SPC700 underground BBS & cyber lounge chiptune audio suite.
- **Action Steps**:
  1. Inspect `KiloOS/public/web/cybercafe.html` audio synthesis functions.
  2. Implement hardware composer chiptune standard:
     - 3-Voice counterpoint: Lead melody, counter-melody/arpeggio, and punchy FM bassline.
     - Shell voicings: Root + 3rd + 7th (omit 5ths).
     - Swing-tick groove: 16-step rhythmic swing.
     - SPC700 warm stereo slapback delay line (180ms with lowpass filtering).
  3. Pure Web Audio synthesis with zero external audio assets.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `KDragon`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KDragon`, `KWizard`, `KFortress`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KSpace`
- **Primary File**: `KiloOS/public/apps/kspace.html`
- **Objective**: Space exploration & tactical combat UI audit: star map controls, modal dismissal, keyboard hotkeys, storage persistence.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KSpace`.
  2. Audit all UI elements: weapon selection, galaxy navigation, combat HUD, shortcuts.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KSpace` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KPac`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KPaint`, `KPac`, `KCalendar`, `KNet`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KChart`
- **Primary File**: `KiloOS/public/apps/kchart.html`
- **Objective**: Chart and graph visualizer responsive layout, canvas high-DPI scaling, and shortcut overlays.
- **Action Steps**:
  1. Verify window bounds and responsive container wrapping under narrow (<540px) widths.
  2. Ensure clear keyboard shortcuts (F1/H help, Esc dialog dismissals) and accessible focus indicators.
  3. Verify HiDPI canvas/text scaling and comfortable retro theme contrast.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KDB`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KDB`, `KCalc`, `KNote`.


### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KMinesweeper`
- **Primary Files**: `KiloOS/public/apps/kminesweeper.html`, `apps/kminesweeper/` (if present)
- **Objective**: Pass 5 QA audit, state persistence integrity, and dual web/native parity.
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `apps/kminesweeper/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KMinesweeper`
  - `cd KiloOS && npm run build` (0 Vite errors, `kminesweeper.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KTetris`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KTetris`, `KAudio`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KClip`
- **Primary Files**: `KiloOS/public/apps/kclip.html`, `KClip/main.c` (if present)
- **Objective**: Deep functional feature expansion for clipboard manager.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kclip.html`.
  2. Expand clipboard manager capabilities:
     - Multi-format clip storage: Text, HTML snippet, image data URL, and hex dump preview.
     - Snippet tags & categorization: Custom labels, search/filter regex, and pinned items.
     - Batch Import & Export: JSON and plain text batch archive export and import.
  3. Zero external dependencies; maintain responsive 1999 retro styling.
- **Verification**: `uv run scripts/test_app_startup.py --app KClip` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KJournal`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KJournal`, `KBookmark`, `KHex`, `KFont`.

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

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-17T17:18:25+0000 — kilo-qa: KHangman (Pass 5 State Persistence & Mature Quarantine)**
  - Dual Parity & State: Added full stats sync to Win32 `GameState`, enabled saving finished sessions, and ensured clean `KillTimer` on `WM_DESTROY`.
  - Onboarding Integrity: Guarded tutorial against interrupting restored save files in both native C and web JS.
  - Modal Ergonomics & Robustness: Improved `safeGetStorage` fallback and enabled immediate Escape modal dismissal from text inputs.
  - Mature Promotion: Promoted feature-complete KHangman (20 stages, blitz, duel, dictionary studio, Win32 client) to Mature Applications table.
  - Verification: Clean builds (`npm.cmd run build` 0 errors, `build.bat` 0 warnings); `test_app_startup.py` PASS; size: 148.9 KB HTML, 36.3 KB EXE (<999 KB).
  - Queue: Advanced `kilo_qa` to `KMinesweeper`; rotation handoff to `kilo-expander`.

- **2026-10-17T16:54:11+0000 — kilo-usability: KChart (Responsive Viewport Polish & Keyboard Accessibility)**
  - Responsive Viewport: Added narrow-viewport (<540px) adaptation, full-width toolbar flex-wrapping, button grouping, and table scrolling.
  - Accessibility & Polish: Added distinct help hint pill and high-visibility focus ring glow (`box-shadow: 0 0 0 3px rgba(96, 165, 250, 0.35)`) on all inputs and buttons.
  - Verification: Clean Vite build (`npm.cmd run build`), size 21.8 KB (<999 KB ceiling).
  - Queue: Advanced `kilo_usability` to `KDB`; rotation handoff to `kilo-qa`.

- **2026-10-17T10:00:00-07:00 — kilo-adhoc: Fleet Upgrade (Flash High Effort, 10m Timeout, Mature Application Quarantine)**
  - Model & Reasoning: Upgraded fleet orchestrator and work state to Gemini 3.8 Flash High effort (`--effort high`, `--model gemini-3.8-flash-high`) and expanded timeout to 10 minutes.
  - Quality Directives: Eliminated blindfold directives (removed single-pass batch restrictions and no-re-read bans); mandated iterative diff checks, build verification, and automated startup testing (`test_app_startup.py`).
  - Mature Quarantine Policy: Codified anti-skip policy across orchestrator, workspace rules, and all skills: apps skipped for maturity/completion are permanently quarantined to `## Mature Applications` and removed from active queues until user reassigns.
  - Queue Sanitization: Purged mature apps from upcoming queues (seeded `KTowers`, `KDarts`, `KReversi`, `KMines`, `KPong`, `KAsteroids`, `KChess`, `KConnect4` in quarantine table).
  - Verification: Clean Vite build (`npm.cmd run build`), 100% security lint PASS, dry-run orchestrator verified.

- **2026-10-17T09:35:00-07:00 — kilo-adhoc: Fleet Dashboard & Scheduler (Turn Metrics, PC A Slot Claim, Tool Budget Uncap)**
  - Dashboard Metrics: Fixed turn metrics to accurately display all-time turns (1,941 receipts) and last 24h turns (42 receipts) with zero-overhead cached counter.
  - Quota & Slot Balancing: Assigned PC C's :48 minute slot to PC A (now 2 turns/hr at :12, :48) with non-colliding windows across fleet.
  - Tool Call Limit: Removed restrictive <=10 tool call directives from orchestrator agy agent dispatch and all skill definitions.
  - Security Lint: Resolved javascript: protocol in webring.html; security_lint 100% PASS.
  - Verification: Security gates passed; task registered in Windows Task Scheduler (next run :48).

- **2026-10-17T16:24:00+0000 — kilo-soundscape: kweb://users/~neon_rider (YM2612 2-Op FM / SPC700 Procedural Audio Overhaul)**
  - Audio Engine: Composed 6 authentic 32-step procedural chiptunes with 3-voice counterpoint (FM slap bass, lead, shell arps) and dynamic drum synthesis (FM kick, filtered noise snare, 2-op hi-hats).
  - Presets & Earcons: Added FM Operator Lab presets (Slap Bass, Cyber Lead, Crystal Chime, Sub Drone, FM Brass, Action Stinger) and tactile UI earcons/stingers (success fanfare, error buzz, 1999Hz harmonic chime, micro-clicks).
  - Delay Line: Calibrated SNES SPC700 slapback delay line (190ms with 2200Hz lowpass damping) and dynamic 4-channel VU meter bounce.
  - Verification: Clean Vite build (`npm.cmd run build` 0 errors); `neon_rider.html` 287 KB (<999 KB ceiling).
  - Queue: Advanced `kilo_soundscape` to `kweb://asm-temple`; rotation handoff to `kilo-tester`.
