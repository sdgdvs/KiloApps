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
  kilo_creator: kweb://asm-temple
  kilo_graphics: KWizard
  kilo_soundscape: kweb://geocities
  kilo_tester: KSpace
  kilo_usability: KNote
  kilo_qa: KConnect4
  kilo_expander: KHex
virtual_web_target: kweb://asm-temple
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
  timestamp: 2026-10-17T11:15:00+0000
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
- **Upcoming Queue**: `KSpace`, `KPac`, `KCalendar`, `KChess`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KNote`
- **Primary File**: `KiloOS/public/apps/knote.html`
- **Objective**: Rich note editor responsive layout, font scaling, shortcut overlays (F1/H), and drawer ergonomics.
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
- **Primary Files**: `KiloOS/public/apps/kconnect4.html`, `apps/kconnect4/` (if present)
- **Objective**: Pass 5 QA audit, state persistence integrity, and dual web/native parity.
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `apps/kconnect4/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KConnect4`
  - `cd KiloOS && npm run build` (0 Vite errors, `kconnect4.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KHangman`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KHangman`, `KMinesweeper`, `KTetris`.

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

- **Pass [kilo-creator] (kweb://users/~neon_rider)**:
  - Verified & enhanced Win32 ASM & demoscene shrine (<999 KB Anti-Potemkin standard).
  - Confirmed active bidirectional links across KNet portal, webring, and subnet sniffer.
  - Built KiloOS cleanly without errors; advanced queue handoff to kilo-graphics.
 
- **2026-10-17T10:31:00+0000 — kilo-qa: KTetris (Pass 5 QA & State Integrity Audit)**
  - Quicksave & State: Verified F5 quicksave & F9 quickload; guaranteed queue refill and tutorial suppression on load.
  - Bug Fix: Fixed ReferenceError in `useRowNuke` (`nx` -> `x`) during row clearance explosion rendering.
  - Verification: `test_app_startup.py` passed (186.5 KB < 999 KB ceiling); `npm run build` clean with 0 errors.
  - Queue: Advanced `kilo_qa` target to `KConnect4`; rotation handoff to `kilo-expander`.

- **2026-10-17T10:13:00+0000 — kilo-usability: KCalc (Responsive & Layout Usability Polish)**
  - Responsive Layout: Enhanced compact breakpoint rules (<380px) for keypad gap, button sizing, and display padding.
  - Ergonomics: Verified F1/H keyboard shortcut guide, backdrop modal dismissals, and high-DPI scaling.
  - Status: PASS ✅ (`kcalc.html` ~233 KB < 999 KB ceiling; Vite build clean in 469ms with 0 errors).
  - Queue: Advanced `kilo_usability` target to `KNote`; rotation handoff to `kilo-qa`.

- **2026-10-17T09:31:00+0000 — kilo-tester: KChess (Interactive UI & Modal Audit)**
  - UI Elements: Audited all menu dropdowns, modals (Help, FEN, PGN, Replay, Book, Analysis, Editor, MP), shortcuts, and canvas hit-tests.
  - Startup Test: `test_app_startup.py` passed cleanly (CSS valid, 0 JS errors, modal dismisses, canvas unblocked).
  - Status: PASS ✅ (No issues found; size ~220 KB < 999 KB ceiling; Vite build clean).
  - Queue: Advanced `kilo_tester` target to `KSpace`; rotation handoff to `kilo-usability`.

- **2026-10-17T09:13:00+0000 — kilo-soundscape: kweb://webring (Procedural FM Chiptune Audio Suite)**
  - Audio Architecture: Verified 6-channel YM2612 2-Op FM chiptune suite with 3 tracks ("Hyperlink Voyager '99", "Ringmaster's Cadence", "Subnet Phantom '99").
  - FM Synthesis & FX: Voice-leading counterpoint, shell voicings, slap bass, EPiano/WarmPad, inharmonic percussion, and UI earcons.
  - Delay & Filters: SNES SPC700 slapback delay line (180ms LPF 2200Hz) and procedural 56k dialup handshake generator.
  - Status: PASS ✅ (`webring.html` ~366 KB < 999 KB ceiling; Vite build clean in 1.20s with 0 errors).
  - Queue: Advanced `kilo_soundscape` target to `kweb://geocities`; rotation handoff to `kilo-tester`.

- **2026-10-17T08:30:02+0000 — kilo-graphics: KWizard (Zero-Token Auto-Skip — Inappropriate Target)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KWizard (pure vector, board game, or mature art).
  - Optimization: Handled via orchestrator pre-flight zero-token auto-skip.
  - Queue: Advanced `kilo-graphics` to `KWizard`; rotation handoff to `kilo-soundscape`.

