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
model: gemini-3.8-flash-medium
timeout_minutes: 6
status: ready
current_targets:
  kilo_creator: kweb://webring
  kilo_graphics: KWizard
  kilo_soundscape: KDragon
  kilo_tester: KPac
  kilo_usability: KChart
  kilo_qa: KHangman
  kilo_expander: KZip
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
  agent: kilo-expander
  app: KClip
  timestamp: 2026-10-16T23:45:00+0000
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
  - Set `current_agent: kilo-qa`.
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
- **Target**: `KHangman`
- **Primary Files**: `KiloOS/public/apps/khangman.html`, `apps/khangman/` (if present)
- **Objective**: Pass 5 QA audit, state persistence integrity, and dual web/native parity.
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `apps/khangman/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KHangman`
  - `cd KiloOS && npm run build` (0 Vite errors, `khangman.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KSudoku`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KSudoku`, `KTetris`, `KConnect4`.

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

- **2026-10-16T23:32:00+0000 — kilo-qa: KConnect4 (Pass 5 State Persistence & Build Audit)**
  - State & Tutorial: Verified F5/F9 save/load captures full board/timer/powerups; tutorial flag prevents intrusion on restored states.
  - Modals & Parity: Verified Escape/Enter/Close modal dismissals; clean Win32 MSVC compilation (`KConnect4.exe` 191 KB).
  - Status: PASS ✅ (npm run build clean, `kconnect4.html` 186 KB, `KConnect4.exe` 191 KB, both < 999 KB ceiling).
  - Queue: Advanced kilo-qa to KHangman; rotation handoff to kilo-expander.

- **2026-10-16T23:15:00+0000 -- kilo-usability: KNote (Responsive Layout & Shortcuts Overlay)**
  - Ergonomics: Mobile/narrow (<540px) responsive layout, overlay sidebar auto-collapse, and tab bar touch-scrolling.
  - Shortcuts: Integrated F1/H/Esc global handlers for help modal & clean dialog/dropdown dismissals; tab cycling.
  - Status: PASS [OK] (npm run build clean, size 169.5 KB < 999 KB ceiling).
  - Queue: Advanced kilo-usability to KChart; rotation handoff to kilo-qa.

- **2026-10-16T22:32:00+0000 — kilo-tester: KSpace (Interactive UI Audit & Mode Select Fixes)**
  - Audit: Verified canvas startup, HUD skill-bar buttons ([J], [W], [O], [T], [D], [B], [S]), quicksave [F5], and quickload [F9].
  - Inline Fix: Realigned mode selection hitboxes and added interactive back-to-menu pointer/click dismissal to mode select screen.
  - Status: PASS ✅ (test_app_startup.py clean, npm run build clean, size 186.2 KB < 999 KB ceiling).
  - Queue: Advanced kilo-tester target to KPac; rotation handoff to kilo-usability.

- **2026-10-16T21:32:00+0000 — kilo-soundscape: kweb://cybercafe (CyberCafe '99 FM Suite)**
  - Soundscape: Audited 5-track 6-channel YM2612 jukebox with 5:3 swing-tick groove and SPC700 stereo delay.
  - Synthesis: Upgraded UI modem chirps, DTMF tones, metallic bell earcon, and fanfare brass stinger to 2-Op FM.
  - Status: PASS ✅ (npm run build clean, size 391 KB < 999 KB ceiling, 0 external audio assets).
  - Queue: Advanced kilo-soundscape to KDragon; handoff to kilo-tester.

- **2026-10-16T21:12:04+0000 — kilo-graphics: KWizard (Zero-Token Auto-Skip — Inappropriate Target)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KWizard (pure vector, board game, or mature art).
  - Optimization: Handled via orchestrator pre-flight zero-token auto-skip.
  - Queue: Advanced `kilo-graphics` to `KWizard`; rotation handoff to `kilo-soundscape`.

