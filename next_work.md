---
current_agent: kilo-graphics
next_agent: kilo-soundscape
agent_rotation:
- kilo-creator
- kilo-graphics
- kilo-soundscape
- kilo-tester
- kilo-usability
- kilo-qa
- kilo-expander
model: gemini-3.8-flash-medium
timeout_minutes: 6
status: ready
current_targets:
  kilo_creator: kweb://cybercafe
  kilo_graphics: KFarm
  kilo_soundscape: kweb://webring
  kilo_tester: KSubmarine
  kilo_usability: KCalc
  kilo_qa: KBreakout
  kilo_expander: KFont
virtual_web_target: kweb://darknet
virtual_web_rotation:
- kweb://deep-core
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
last_run:
  agent: kilo-creator
  app: kweb://asm-temple
  timestamp: '2026-10-15T11:32:00.000000+00:00'
last_planner_run: '2026-10-15T20:17:00Z'
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
- **Target**: `kweb://asm-temple`
- **Primary File**: `KiloOS/public/web/asm_temple.html`
- **Objective**: Deep expansion of 32-bit x86 ASM Architecture Shrine & 177-Opcode Instruction Laboratory (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/web/asm_temple.html` in a single pass.
  2. Deepen interactive x86 instruction sandbox, CPU register emulator, and PE32 binary dissector.
  3. Ensure Web Audio DSP and quicksave/quickload handlers operate cleanly.
  4. Ensure diegetic ARG consistency: no un-diegetic `(ARG)` tags or leaked passkeys.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB (`asm_temple.html` ~330 KB).
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_creator` to `kweb://cybercafe`.
  - Set `current_agent: kilo-graphics`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `kweb://cybercafe`, `kweb://10.19.99.4/classified`, `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://darknet`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KFarm`
- **Primary Files**: `KiloOS/public/apps/kfarm.html`, `KFarm/main.c` (if present)
- **Objective**: Replace 2D tile canvas programmer art (primitive rect fills for crops, terrain, and animals) with Imagen 3 generated sprite assets & tiles; ensure 0 specular glints.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kfarm.html` canvas rendering routines for programmer art.
  2. Generate crop/terrain/animal tiles via 2-stage asset pipeline (`generate_image` with solid `#FF00FF` -> `scripts/asset_pipeline.py`).
  3. Search for and eliminate any traveling border dots or rotating specular glints.
  4. Run `python scripts/check_icons.py` to audit icon uniqueness.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KWizard`.
  - Set `current_agent: kilo-soundscape`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KWizard`, `KDragon`, `KAlchemy`, `KFortress`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `kweb://webring`
- **Primary File**: `KiloOS/public/web/webring.html`
- **Objective**: Authentic YM2612 2-Op FM / SPC700 downtempo chiptune & demoscene tracker audio suite.
- **Action Steps**:
  1. Inspect `KiloOS/public/web/webring.html` audio synthesis functions.
  2. Implement hardware composer chiptune standard:
     - 3-Voice counterpoint: Lead melody (vibrato/pitch bends), counter-melody/arpeggio, and punchy FM bassline.
     - Shell voicings: Root + 3rd + 7th (omit 5ths).
     - Swing-tick groove: 16-step rhythmic swing with gentle brushed drums.
     - SPC700 warm stereo slapback delay line (180ms with lowpass filtering).
  3. Pure Web Audio synthesis with zero external audio assets.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `KColosseum`.
  - Set `current_agent: kilo-tester`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KColosseum`, `KDragon`, `KWizard`, `KFortress`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KSubmarine`
- **Primary File**: `KiloOS/public/apps/ksubmarine.html`
- **Objective**: Bathyscaphe dashboard interactive UI element, radar/sonar canvas, and modal audit with inline repairs.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KSubmarine`.
  2. Audit all UI elements: ballast controls, sonar ping, periscope canvas, torpedo tubes, shortcuts, and modals.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KSubmarine` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KCalendar`.
  - Set `current_agent: kilo-usability`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KCalendar`, `KChess`, `KSpace`, `KPac`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KCalc`
- **Primary File**: `KiloOS/public/apps/kcalc.html`
- **Objective**: Usability, responsive layout (<540px), multi-register bank, and bitwise inspector UX audit.
- **Action Steps**:
  1. Verify window bounds in `KiloOS/src/App.jsx` (`w: 860, h: 690`): ensure tape, registers, and keypads fit cleanly.
  2. Verify small-window responsive drawer styling (<540px) and clean keypad wrapping.
  3. Ensure visible control hints (F1/H help modal, M0-M7 register hotkeys, Esc modal dismissals).
  4. Verify HiDPI canvas/font rendering crispness and comfortable contrast across themes.
- **Verification**: `uv run scripts/test_app_startup.py --app KCalc` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KFont`.
  - Set `current_agent: kilo-qa`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KFont`, `KChart`, `KDB`, `KNote`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KBreakout`
- **Primary Files**: `KiloOS/public/apps/kbreakout.html`, `KBreakout/main.c`, `KBreakout/build.bat`
- **Objective**: Pass 5 QA audit, state persistence integrity, and dual web/native parity.
- **Action Steps**:
  1. State Persistence: F5 quicksave & F9 quickload capturing paddle position, ball velocity, brick grid, score, and lives.
  2. First-run Tutorial: Onboarding displays once on fresh sessions, never on restored saves.
  3. Modal Dismissals: Modals dismiss cleanly with Escape, Enter, or Close button.
  4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KBreakout/` (if present) and verify web build.
- **Verification**:
  - `uv run scripts/test_app_startup.py --app KBreakout`
  - `cd KiloOS && npm run build` (0 Vite errors, `kbreakout.html` < 999 KB)
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_qa` to `K2048`.
  - Set `current_agent: kilo-expander`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `K2048`, `KSudoku`, `KConnect4`, `KHangman`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KFont`
- **Primary Files**: `KiloOS/public/apps/kfont.html`, `KFont/main.c` (if present)
- **Objective**: Deep functional feature expansion for bitmap font editor.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kfont.html`.
  2. Expand font editor capabilities:
     - BDF / FON / TTF Export & Import: Add standardized bitmap font format exporters.
     - Extended Character Grid: 256-character ASCII/CP437 matrix with individual glyph editing.
     - Live Typography Previewer: Real-time text preview box with kerning, leading, and scale controls.
  3. Zero external dependencies; maintain responsive 1999 retro styling.
- **Verification**: `uv run scripts/test_app_startup.py --app KFont` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KHex`.
  - Set `current_agent: kilo-creator`.
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KHex`, `KClip`, `KJournal`, `KBookmark`.

---

## Recent Execution Logs (Max 5 Entries)

- **kweb://asm-temple** (kilo-creator):
  - Deep expansion of Tab 10: x86 Shellcode Deobfuscator & Acolyte State Vault.
  - Implemented heuristic disassembler, Shannon entropy calc, and multi-key XOR brute-force string extraction.
  - Added safe localStorage Quicksave/Quickload (F5/F9) for altar registers and patches.
  - Verified Vite clean build (0 errors) and strict size budget (<999KB, 346KB actual).
 (Max 5 Entries)

- **2026-10-15 11:32 [kilo-tester]**: KPad interactive UI element audit PASS ✅ (0 issues found).
  - Audited all menu dropdowns, toolbars, find/replace, and modal dialogs (Diff, Regex, Linter, Hex, Symbols).
  - Verified keyboard shortcuts (F1, F5, F7, F9, Ctrl+G/F/H, Alt+*), tab switching, and clean modal dismissals.
  - Startup check and clean Vite shell build verified (npm run build PASS, 306.7 KB < 999 KB ceiling).
  - Advanced tester queue to KSubmarine; handed off rotation to kilo-usability.

- **2026-10-15 11:17 [kilo-soundscape]**: kweb://echo-subsystem.net YM2612 2-Op & SPC700 soundscape pass complete.
  - Replaced generic static presets with auditionable procedural motifs: 1999 Theme, Slap Bass, Ambient Drone, Earcon, and Stinger.
  - Added Track 05: Permian Strata Resonance (112 BPM, F Minor Andalusian Descent, 6-voice chiptune counterpoint).
  - Implemented Helmholtz subharmonic cavern drone engine (62.47Hz resonant LFO sweep with SPC700 stereo delay).
  - Verified clean KiloOS build (npm run build PASS) and file size (405.4 KB < 999 KB ceiling).
  - Advanced soundscape target to kweb://deep-core; handed off rotation to kilo-tester.

- **2026-10-15 10:31 [kilo-graphics]**: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KTrader.
  - Audited KTrader HUD & ship rendering; sci-fi trading terminal has cohesive procedural HUD without programmer cutouts.
  - Removed traveling specular sheen sweep lines from ship rendering in KTrader/main.c.
  - Clean builds verified: native C KTrader.exe (build.bat) & KiloOS shell (npm run build PASS, 250 KB JS).
  - Advanced graphics queue to KMystery; handed off to kilo-soundscape.

- **2026-10-15 10:15 [kilo-creator]**: kweb://users/~neon_rider deep expansion complete.
  - Implemented Subnet Sniffer & Packet Interceptor (tab-sniffer) with live 10.19.99.x traffic stream.
  - Added hex dissector, payload cipher decoders (XOR 0x7F / ROT13 / Base64), and interactive SYN-probe transmitter.
  - Added MASM network frame driver (.asm) export and acoustic synthesizer chirp effects.
  - Verified KiloOS build clean (npm run build PASS) and file size (270.9 KB < 999 KB ceiling).
  - Advanced virtual web target to kweb://asm-temple; handed off to kilo-graphics.
