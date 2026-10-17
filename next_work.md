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
  kilo_creator: kweb://asm-temple
  kilo_graphics: KWizard
  kilo_soundscape: kweb://users/~neon_rider
  kilo_tester: KPac
  kilo_usability: KChart
  kilo_qa: KHangman
  kilo_expander: KFont
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
  app: KHex
  timestamp: '2026-10-17T14:35:00+0000'
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
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KDB`, `KCalc`, `KNote`.


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
  - In frontmatter: advance `current_targets.kilo_qa` to `KMinesweeper`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KMinesweeper`, `KTetris`, `KConnect4`.

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

- **2026-10-17T14:35:00+0000 — kilo-expander: KHex (Opcode Disassembler & CPU Instruction Decoder)**
  - Feature Expansion: Added multi-architecture opcode disassembler (x86-32, x86-16, MOS 6502, Z80) with instruction decode tables.
  - Interactive Inspector: Linked disassembly addresses directly to hex viewer byte offsets with mnemonic filtering and category tagging.
  - Export & Tools: Added .ASM download and clipboard export; integrated sample retro routines (x86 cracktro, 6502 screen clear, Z80 vblank).
  - Verification: `npm run build` clean (0 errors); `khex.html` ~208 KB (<999 KB ceiling).
  - Queue: Advanced `kilo_expander` to `KFont`; rotation handoff to `kilo-creator`.

- **2026-10-17T14:14:00+0000 — kilo-qa: KConnect4 (Pass 5 QA & Build Audit)**
  - State persistence verified: F5 quicksave & F9 quickload functional with quota protection.
  - First-run tutorial verified: onboarding triggers on fresh session only (`kconnect4_tutorialSeen`).
  - Native & web build parity verified: `KConnect4.exe` (190 KB) & `kconnect4.html` (186 KB) < 999 KB.
  - Status: PASS ✅ (clean native C build & 0 Vite errors).

- **2026-10-17T13:32:00+0000 — kilo-usability: KNote (Usability & Layout Audit)**
  - ⏭️ Skip — app usability is complete and mature.
  - Status: PASS ✅ (`knote.html` ~172 KB < 999 KB ceiling; Vite build clean with 0 errors).
  - Queue: Advanced `kilo_usability` target to `KChart`; rotation handoff to `kilo-qa`.

- **2026-10-17T13:15:00+0000 — kilo-tester: KSpace (Interactive UI Audit & Inline Repairs)**
  - Skill UI: Added explicit button types, titles, and ARIA labels across skill bar controls.
  - Controls: Added keybind fallbacks for skill hotkeys; added touch-action: none to canvas to prevent mobile gesture conflicts.
  - Feedback: Enlarged top HUD pause/help/audio touch hitboxes; added explicit toast confirmation on quicksave resume.
  - Verification: Clean Vite build; file size 194 KB (<999 KB ceiling); advanced queue to KPac.

- **2026-10-17T12:35:00+0000 — kilo-soundscape: kweb://geocities (Procedural FM Audio & Soundscape Engine)**
  - Soundscape: Expanded YM2612 2-Op FM engine with ambient drone voice (1:1 warm analog drift) and contrary motion counter-melody voice.
  - UI Earcons: Integrated full FM earcon suite (confirm chime, cancel stinger, warning clank, ultra-short pip).
  - Verification: Clean Vite build; file size ~608 KB (<999 KB ceiling).
  - Queue: Advanced `kilo_soundscape` target to `kweb://users/~neon_rider`; rotation handoff to `kilo-tester`.
