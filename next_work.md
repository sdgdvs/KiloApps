---
current_agent: kilo-usability
next_agent: kilo-usability
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
  kilo_creator: kweb://geocities
  kilo_graphics: KWizard
  kilo_soundscape: kweb://webring
  kilo_tester: KChess
  kilo_usability: KDB
  kilo_qa: KSudoku
  kilo_expander: KJournal
virtual_web_target: kweb://geocities
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
  agent: kilo-tester
  app: KCalendar
  timestamp: 2026-10-17T06:15:00+0000
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
- **Target**: `KCalendar`
- **Primary File**: `KiloOS/public/apps/kcal.html`
- **Objective**: Calendar & event scheduler UI audit: event creation, modal dismissal, month navigation, storage persistence.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KCalendar`.
  2. Audit all UI elements: calendar grid buttons, event modal forms, reminder alerts, shortcuts.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KCalendar` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KChess`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KCalendar`, `KChess`, `KSpace`, `KPac`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KDB`
- **Primary File**: `KiloOS/public/apps/kdb.html`
- **Objective**: Relational query studio responsive grid sizing, schema editor ergonomics, SQL keyword autocomplete contrast, and keyboard shortcut overlays (F1/H).
- **Action Steps**:
  1. Verify window bounds and responsive container wrapping under narrow (<540px) widths.
  2. Ensure clear keyboard shortcuts (F1/H help, Esc dialog dismissals) and accessible focus indicators.
  3. Verify HiDPI canvas/text scaling and comfortable retro theme contrast.
- **Verification**: `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KCalc`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KCalc`, `KNote`, `KChart`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KSudoku`
- **Primary Files**: `KiloOS/public/apps/ksudoku.html`, `apps/ksudoku/` (if present)
- **Objective**: Pass 5 QA audit, state persistence integrity, and dual web/native parity.
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing complete game state, stats, and settings.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `apps/ksudoku/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KSudoku`
  - `cd KiloOS && npm run build` (0 Vite errors, `ksudoku.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `KTetris`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KTetris`, `KConnect4`, `KHangman`.

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

- **2026-10-17T06:15:00+0000 — kilo-tester: KCalendar (Interactive UI & Clipboard Fallback Audit)**
  - UI Elements: Audited all buttons, export dropdown options, modals, shortcuts (F1/F5/F9/Escape), and event form hooks.
  - Clipboard Reliability: Added textarea execCommand fallback to copyAgendaSummary for seamless operation in sandboxed iframes.
  - Status: PASS ✅ (`kcalendar.html` 116.4 KB < 999 KB ceiling; Vite build clean in 1.22s with 0 errors).
  - Queue: Advanced `kilo_tester` target to `KChess`; rotation handoff to `kilo-usability`.

- **2026-10-17T05:35:00+0000 — kilo-soundscape: KWizard (Procedural FM Soundscape & Chiptune Overhaul)**
  - Audio Engine: 64-step 6-channel YM2612 2-Op FM + SPC700 stereo delay composition ("The Archmage's Duel" in D Dorian & Escalation).
  - Sound Design: Ambient mystic drone (D2/D1 respiration), rich element spell patches (fire, ice, arcane, lightning, poison, nature).
  - UI Earcons: Integrated card hover micro-chimes, parchment draw, mana depletion error buzz, and turn gong stingers.
  - Verification: `npm run build` clean (464ms); file size ~148 KB < 999 KB ceiling.
  - Queue: Advanced `kilo_soundscape` target to `kweb://webring`; rotation handoff to `kilo-tester`.

- **2026-10-17T05:12:05+0000 — kilo-graphics: KWizard (Zero-Token Auto-Skip — Inappropriate Target)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KWizard (pure vector, board game, or mature art).
  - Optimization: Handled via orchestrator pre-flight zero-token auto-skip.
  - Queue: Advanced `kilo-graphics` to `KWizard`; rotation handoff to `kilo-soundscape`.

- **2026-10-17T04:30:00+0000 — kilo-creator: kweb://warez (Underground 1999 Scene Vault & ARG Gateway)**
  - Anti-Potemkin Audit: Verified 16 functional tabs, x86 crack sandbox, chiptune tracker/jukebox, and Firebase shoutbox.
  - ARG & Network Integration: Verified subterranean 1999Hz carrier signals & cross-links in KNet, Portal, and Webring.
  - Status: PASS ✅ (`warez.html` 517.5 KB < 999 KB ceiling; Vite build clean with 0 errors).
  - Queue: Advanced `virtual_web_target` to `kweb://geocities`; rotation handoff to `kilo-graphics`.

- **2026-10-17T04:15:00+0000 — kilo-expander: KZip (Deep Feature Expansion)**
  - Compression Benchmark Lab: real-time side-by-side Store vs RLE vs Deflate speed, ratio, and throughput [Alt+B].
  - Multi-Volume Splitter & Combiner: 1.44MB/720KB/custom floppy volume slicer & part reconstructor [Alt+S].
  - Deduplication & Cryptographic Hash Audit: SHA-256 / CRC-32 duplicate detector & space reclamation [Alt+D].
  - Archive Transcoder: 1-click container conversion between ZIP, KZA, TAR, and SFX [Ctrl+T].
  - Status: PASS ✅ (`kzip.html` 150.2 KB < 999 KB ceiling; 0 JS errors, 0 Vite build errors).
