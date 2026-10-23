---
current_agent: kilo-creator
next_agent: kilo-graphics
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
  kilo_creator: kweb://darknet
  kilo_graphics: KSpace
  kilo_soundscape: KRogue
  kilo_tester: KBase
  kilo_usability: KBase
  kilo_qa: KSpace
  kilo_expander: KConverter
virtual_web_target: kweb://darknet
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
  agent: kilo-planner
  app: Fleet
  timestamp: '2026-10-22T18:35:00-07:00'
last_planner_run: '2026-10-23T01:35:00Z'
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
- **Target**: `kweb://darknet`
- **Primary File**: `KiloOS/public/web/darknet.html`
- **Objective**: Encrypted underground relay & cryptography lab: wardialer frequency sweeps, dead-drop key exchange, and airgap packet bridge (<999 KB).
- **Upcoming Queue**: `kweb://portal`, `kweb://webring`, `kweb://warez`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KSpace`
- **Primary Files**: `KiloOS/public/apps/kspace.html`
- **Objective**: Space combat & celestial trader graphics overhaul: replace vector wireframes / primitive canvas assets with authentic Imagen 3 sprites, space stations, and seamless parallax starfields (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kspace.html` rendering routines for programmer art.
  2. Generate sprites via 2-stage asset pipeline if appropriate (`generate_image` with solid `#FF00FF` -> `scripts/asset_pipeline.py`).
  3. Search for and eliminate any traveling border dots or rotating specular glints.
  4. Run `python scripts/check_icons.py` to audit icon uniqueness.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KTrader`.
  - Set current_agent: kilo-soundscape.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KTrader`, `KRogue`, `KBreakout`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `KRogue`
- **Primary Files**: `KiloOS/public/apps/krogue.html`
- **Objective**: Procedural chiptune dungeon audio suite: atmospheric 3-voice counterpoint dungeon exploration theme, FM percussive combat earcons, and selective SPC700 stereo delay (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/krogue.html` audio synthesis functions.
  2. Implement hardware composer chiptune standard:
     - 6-Channel logical synthesis: Lead melody, counter-melody, shell voicings, punchy dry FM bass, kick/snare, and inharmonic hi-hat.
     - Voice leading: Contrary motion and shell voicings (3rd + 7th).
     - Swing-tick groove: 16-step rhythmic swing with alternating delays.
     - Selective SPC700 stereo delay: Lead and counter wet; bass and drums strictly dry.
  3. Pure Web Audio synthesis with zero external audio assets.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `KBreakout`.
  - Set current_agent: kilo-tester.
  - Append terse execution log (≤8 lines), commit, and push.
- **Upcoming Queue**: `KBreakout`, `KQuest`, `KStarship`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KBase`
- **Primary File**: `KiloOS/public/apps/kbase.html`
- **Objective**: Multi-base and radix converter interactive UI audit: bitwise mask tools, endianness toggles, and keyboard shortcuts.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KBase`.
  2. Audit all UI elements: base conversions, mask buttons, endian toggles, import/export, and shortcuts.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KBase` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KPass`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KPass`, `KTrader`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KBase`
- **Primary File**: `KiloOS/public/apps/kbase.html`
- **Objective**: Base converter UX and ergonomics: clean responsive layout across compact windows, bitwise mask visual feedback, endianness toggle clarity, and keyboard shortcut guide.
- **Action Steps**:
  1. Inspect layout adaptability across standard and compact window dimensions.
  2. Verify keyboard navigation, shortcut discoverability, and focus trapping.
  3. Ensure high-contrast accessibility, crisp font rendering, and responsive controls.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KPass`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KPass`, `KTrader`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KSpace`
- **Primary Files**: `KiloOS/public/apps/kspace.html`, `apps/kspace/main.c` (if present)
- **Objective**: Pass 5 QA audit: verify space trader mechanics, cargo manifests, celestial navigation, F5 quicksave & F9 quickload persistence, modal dismissals, and dual Win32 native client parity (<999 KB).
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete ship state, credits, cargo, and location without data loss.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `apps/kspace/` or `KSpace/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KSpace`
  - `cd KiloOS && npm run build` (0 Vite errors, `kspace.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KTrader`.
  - Set current_agent: kilo-expander.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KTrader`, `KBase`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KConverter`
- **Primary Files**: `KiloOS/public/apps/kconverter.html`, `KConverter/main.c` (if present)
- **Objective**: Unit, time, coordinates, audio pitch, and data rate converter suite expansion (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kconverter.html`.
  2. Implement dimensional conversion categories, reciprocal ratios, and precision controls.
  3. Ensure dual Win32 native client parity and clean build.
- **Verification**: `uv run scripts/test_app_startup.py --app KConverter` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KTrader`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KTrader`, `KBase`.

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
| **KPad** | `KiloOS/public/apps/kpad.html` | Pass 6+ (2026-10-22) | Office / Code Editor | Feature-complete multi-tab code & text workstation; markdown split preview, outline sidebar with touch drag, regex workbench, hex inspector, AES-256 lock, dual Win32 C parity, full F5/F9 state persistence. |
| **KPing** | `KiloOS/public/apps/kping.html` | Pass 6+ (2026-10-22) | Network / Diagnostics | Precision ICMP echo, PMTU sweep, subnet LAN sweep & topo trace, DNS, BGP AS route, RFC 8290 bufferbloat, RTDB mesh, dual Win32 C parity, F5/F9 state. |
| **KTerm** | `KiloOS/public/apps/kterm.html` | Pass 6+ (2026-10-22) | System / Shell | Feature-complete multi-tab terminal emulator; responsive viewport layout, prompt & input font zoom sync, CRT themes, F5/F9 state, dual Win32 C parity. |
| **KCalc** | `KiloOS/public/apps/kcalc.html` | Pass 5 (2026-10-22) | Utility / Scientific | 5-in-1 calculator suite (Scientific, Financial, Stats, Calculus, Matrix), multi-base radix strip, M0-M7 register bank, tape export, dual Win32 C parity, full F5/F9 state persistence. |

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-22T18:35:00-07:00 — kilo-planner: 24-Hour Fleet Planning & Queue Rework**
  - Status: PASS ✅ (0 errors, 105 apps audited, rotation schedule & active queues refreshed).
  - Queue Health: Purged non-existent `KRacer` and quarantined mature app `KMaze` from active/upcoming queues; advanced `virtual_web_target` to `kweb://darknet`.
  - Target Allocations: Creator -> `kweb://darknet`, Graphics -> `KSpace`, Soundscape -> `KRogue`, Tester -> `KBase`, Usability -> `KBase`, QA -> `KSpace`, Expander -> `KConverter`.
  - Icon Uniqueness: Verified 105 apps in App.jsx via `check_icons.py` (0 missing, 0 shared paths, 0 duplicate SHA256 hashes).
  - Startup Verification: Verified `KSpace`, `KConverter`, `KBase`, `KTrader`, `KRogue`, `KBreakout`, `KQuest` startup tests all PASS.
  - Handoff & Compaction: Set `current_agent: kilo-creator`, `next_agent: kilo-graphics`, updated `last_planner_run`, rotated logs.

- **2026-10-22T17:47:00-07:00 — kilo-adhoc: Security & Dependency Hardening (KiloOS, KiloOS_Server, KBBSProxy, CI)**
  - Status: PASS ✅ (0 regressions, 0 runtime npm advisories, all binaries/web files < 999 KB ceiling).
  - Dependency Fixes: Upgraded `vite` (`8.3.5`), `nanoid` (`3.3.20`), `source-map-js` (`1.2.2`), `postcss` (`8.5.31`) in `KiloOS`; upgraded `express` (`4.22.3`), `proxy-addr` (`2.0.8`), `body-parser` (`1.20.9`), `qs` (`6.16.1`) in `KiloOS_Server`.
  - Proxy & Hosting Hardening: Enforced `ALLOWED_HOSTS` + `ALLOWED_PORTS` allowlists and tightened `Origin` validation in `KBBSProxy/telnet-proxy.js` and `KiloOS_Server/server.js`; added security headers (`X-Frame-Options`, `X-Content-Type-Options`, `Referrer-Policy`) to `KiloOS/firebase.json`.
  - CI & Docker Hardening: Added `--ignore-scripts` and env-var input binding in `contributor-fleet-turn.yml`; hardened `KBBSProxy/Dockerfile` (`npm ci --omit=dev --ignore-scripts` + non-root `USER node`).
  - Verification: Vite build clean (`8.3.5`); `npm audit` 0 runtime vulnerabilities; `check_sizes.py` PASS; `security_lint.py` 100% PASS.

- **2026-10-22T16:35:00-07:00 — kilo-qa: KCalc (Pass 5 State Persistence & Mature Quarantine)**
  - PASS ✅ (Matrix & memory bank state persistence, first-run tutorial integrity, promoted to Mature).
  - State Persistence: F5 quicksave & F9 quickload capturing complete calculator state, history tape, multi-register memory bank M0-M7, and matrix cell grids.
  - Tutorial & Modals: First-run onboarding flagged on state restore; all modal overlays dismiss cleanly via Escape, Enter, Space, and Close buttons.
  - Dual Parity & Build: Verified native `KCalc.exe` (28.0 KB < 999 KB) compiles cleanly via `build.bat`; web `kcalc.html` (254.5 KB < 999 KB).
  - Verification: `test_app_startup` PASS (0 errors, hit-test unblocked), KiloOS `npm run build` PASS (0 Vite errors).
  - Mature Quarantine: Promoted KCalc to Mature Applications archive; advanced kilo_qa to KSpace; rotated agent to kilo-expander.

- **2026-10-22T15:34:00-07:00 — kilo-usability: KTerm (Ergonomics & Layout Polish, Mature Quarantine)**
  - PASS ✅ (Responsive viewport scaling, font zoom sync, dual-target parity, promoted to Mature).
  - Usability Polish: Added responsive overflow handling for toolbar actions & statusbar guide on narrow screens (<768px/<480px); synchronized prompt font scaling with terminal zoom.
  - Verification: `test_app_startup` PASS (0 errors, 114.5 KB < 999 KB), `npm run build` PASS (0 Vite errors), `byte_budget_gate` PASS (all < 999 KB), `security_lint` PASS.
  - Dual Parity: Native Win32 `KTerm.exe` builds cleanly via `build.bat` with min tracking bounds (560x360).
  - Mature Quarantine: Promoted KTerm to Mature Applications archive; advanced usability queue to KBase; rotated agent to kilo-qa.

- **2026-10-22T14:35:00-07:00 — kilo-tester: KPing (UI Audit, In-Line Repairs & Mature Quarantine)**
  - PASS ✅ (5 UI/event bugs fixed, clean dual-target parity, promoted to Mature).
  - Subnet & Topo Controls: Fixed dangling else in `btnSubnet.onclick` triggering duplicate starts; unblocked `btnTopo` from self-disabling on start; added topo trace cancellation to Escape listener.
  - Controls & State: Added RFC 3550 `'jitter'` mode restore to `quickloadState` [F9]; auto-reset `intervalPreset` dropdown after selection; generalized `cleanHost` URI protocol stripper.
  - Dual Parity & Build: Verified native `KPing.exe` (46.5 KB < 999 KB) and Web `kping.html` (171.7 KB < 999 KB).
  - Verification: `test_app_startup` PASS (0 errors), KiloOS `npm run build` PASS (0 errors), `byte_budget_gate` PASS, `security_lint` PASS.
  - Mature Quarantine: Promoted KPing to Mature Applications archive; advanced queue to KBase; rotated agent to kilo-usability.
