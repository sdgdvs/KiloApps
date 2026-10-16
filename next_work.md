---
current_agent: kilo-graphics
next_agent: kilo-soundscape
agent_rotation:
- kilo-graphics
- kilo-soundscape
- kilo-tester
- kilo-usability
- kilo-qa
- kilo-expander
- kilo-creator
model: gemini-3.8-flash-medium
timeout_minutes: 6
status: ready
current_targets:
  kilo_creator: kweb://webring
  kilo_graphics: KWizard
  kilo_soundscape: kweb://cybercafe
  kilo_tester: KSpace
  kilo_usability: KNote
  kilo_qa: KConnect4
  kilo_expander: KClip
virtual_web_target: kweb://webring
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
  agent: kilo-creator
  app: kweb://portal
  timestamp: 2026-10-16T20:15:00+0000
last_planner_run: '2026-10-16T20:35:00Z'
---

# KiloApps Master Fleet Work & Queue State

This document is the single active source of truth for autonomous agent dispatching.
The Windows Task Scheduler orchestrator (`scripts/orchestrate.py`) parses the YAML frontmatter above on every tick to dispatch the active skill.

## Fleet Core Directives (Quick Reference)

> Full architectural rules codified in [.agents/AGENTS.md](.agents/AGENTS.md) and [.agents/rules/](.agents/rules/).
> Full historical director directives codified in [archive/director_directives_archive.md](archive/director_directives_archive.md).

1. **Single-App-Per-Turn**: Exactly ONE target application per turn. Inspect once, batch edits, verify builds, advance queue, commit, and push.
2. **Strict Size Ceiling**: Every web file (`.html`) and native executable (`.exe`) must remain strictly **< 999 KB**.
3. **Token Conservation (CRITICAL)**: ≤6 lines of terse bullet points in execution log. Never re-read files after editing. Max 5 active log entries in this file (older entries rotated by `scripts/rotate_logs.py`).
4. **Multiplayer Autostart Prohibition**: Games must NEVER autostart into matchmaking. Default to local offline play; multiplayer requires explicit button click. Include 25s solo AI fallback.
5. **Anti-Trademark / Parody Rule**: No real commercial marks or scene group names (use *Surreal Tournament*, *VoidCraft*, *RAZOR 1999*, *FLARELIGHT*). Must pass `python scripts/security_lint.py`.
6. **ARG Mystery Preservation (TINAG)**: Never label UI or copy with `(ARG)` or walkthrough guides; never leak `ECHO-1999-ARCHITECT` before App #100 (`KMatrix`).
7. **Perimeter Glint & Comet Ban**: Remove traveling border dots, orbiting balls, and specular glints.
8. **Chiptune Audio Architecture**: Genesis YM2612 2-Op FM / SNES SPC700 stereo delay standard. Zero external audio assets.

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
  - Set `current_agent: kilo-soundscape`.
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
  - Set `current_agent: kilo-tester`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KDragon`, `KWizard`, `KFortress`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KSpace`
- **Primary File**: `KiloOS/public/apps/kspace.html`
- **Objective**: Space flight simulator & combat UI audit: radar display, thruster controls, docking nav, modal dismissal, keyboard shortcuts.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KSpace`.
  2. Audit all UI elements: cockpit buttons, hyperspace jump modal, throttle controls, weapon switching, shortcuts.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KSpace` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KPac`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KPac`, `KCalendar`, `KChess`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KNote`
- **Primary File**: `KiloOS/public/apps/knote.html`
- **Objective**: Note & markdown editor responsive layout, tab management ergonomics, clean font scaling, and keyboard shortcut overlays (F1/H).
- **Action Steps**:
  1. Verify window bounds and responsive container wrapping under narrow (<540px) widths.
  2. Ensure clear keyboard shortcuts (F1/H help, Esc dialog dismissals) and accessible focus indicators.
  3. Verify HiDPI canvas/text scaling and comfortable retro theme contrast.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KChart`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KChart`, `KDB`, `KCalc`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KConnect4`
- **Primary Files**: `KiloOS/public/apps/kconnect4.html`, `KConnect4/main.c`, `KConnect4/build.bat`
- **Objective**: Pass 5 QA audit, state persistence integrity, and dual web/native parity.
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KConnect4/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KConnect4`
  - `cd KiloOS && npm run build` (0 Vite errors, `kconnect4.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KHangman`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KHangman`, `KSudoku`, `KTetris`.

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
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KJournal`, `KBookmark`, `KHex`, `KFont`.

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-16T20:35:00+0000 — kilo-planner: 24-Hour Fleet Planning & Queue Harmonization**
  - Evaluated velocity & queue health: 105 apps strictly <= 999 KB; resolved KFarm skip loop by re-targeting kilo-graphics to KWizard.
  - Aligned active target queues: kilo-soundscape -> kweb://cybercafe, tester -> KSpace, usability -> KNote, qa -> KConnect4, expander -> KClip.
  - Fixed syntax error in scripts/byte_budget_gate.py; verified 100% clean gate pass, icon uniqueness, and security lint.
  - Verification: npm run build clean (0 errors, 566ms); zero-discard log rotation executed cleanly.
  - Queue handoff: current_agent set to kilo-graphics; status set to ready.

- **2026-10-16T20:15:00+0000 — kilo-creator: kweb://portal (Netscape Bookmarks & 1999 Directory Engine)**
  - Implemented Netscape Bookmarks.html export engine in My KiloNet dashboard with full RFC standard DL/DT structure.
  - Verified directory indexing across all 1999 virtual web destinations and bidirectional webring navigation.
  - Verification: npm run build clean (0 errors, 500ms); file size 560 KB (< 999 KB ceiling).
  - Queue handoff: advanced virtual_web_target to kweb://webring; dispatched next_agent to kilo-graphics.

- **2026-10-16T12:55:00-07:00 — kilo-adhoc: Byte-Budget Enforcement Gate v2.0 & Drive Sync Automation**
  - Status: PASS ✅ (107 web cartridges + 104 native EXEs strictly <= 999,000 bytes; 100% manifest & icon parity).
  - scripts/byte_budget_gate.py: Upgraded to v2.0 auditing web HTML, native EXEs, App.jsx schema/icons, and reverse coverage.
  - Auto-generated catalog.json: Emits { id, title, folder, webBytes, exeBytes, budgetPercent } for live fleet telemetry.
  - KDirector & KSys: Wired to fetch live catalog.json for real-time byte telemetry, accurate app counts, and budget tracking.
  - Google Drive & CI/CD Automation: pack_tiered_context.py automated in orchestrator post-turn and .github byte-budget-gate workflow.
  - Verification: byte_budget_gate.py 100% PASS; pack_tiered_context.py 100% PASS; npm run build 0 errors; security_lint.py PASS.

- **2026-10-16T19:33:00+0000 — kilo-expander: KHex (Struct & Signatures Dissector)**
  - Implemented Tab 9: Binary Struct Unpacker with schema parser (uint, int, float, char, hex) & C/JSON export.
  - Added Magic Bytes Detector for 20+ file signatures (BMP, WAV, PNG, ZIP, MZ, ELF, etc.) with header analysis.
  - Verification: npm run build passed cleanly; advanced queue to KClip; handoff to kilo-creator.

- **2026-10-16T18:15:00+0000 — kilo-usability: KDB (UI/UX & Usability Overhaul)**
  - Audited layout ergonomics: tuned default window size to 1020x700 in `App.jsx` and `kdb.html` for clean multi-column viewing.
  - Eliminated toolbar button wrap: consolidated 9 export/import actions into responsive Dropdown menus while preserving all shortcuts.
  - HiDPI & Responsive Charts: enforced integer DPR canvas buffer styles and added debounced window resize handler for dashboard charts.
  - Verification: `npm run build` clean (0 errors); advanced usability target to `KNote`; handoff to `kilo-qa`.
