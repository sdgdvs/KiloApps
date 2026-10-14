---
current_agent: kilo-usability
next_agent: kilo-qa
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
  kilo_creator: kweb://deep-core
  kilo_graphics: KSubmarine
  kilo_soundscape: kweb://warez
  kilo_tester: KCalendar
  kilo_usability: KSubmarine
  kilo_qa: KBreakout
  kilo_expander: KMedia
virtual_web_target: kweb://deep-core
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
  agent: kilo-tester
  app: KVault
  timestamp: '2026-10-14T08:14:00-07:00'
last_planner_run: '2026-10-13T18:15:00Z'
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
- **Target**: `kweb://10.19.99.4/classified`
- **Primary File**: `KiloOS/public/web/classified.html`
- **Objective**: Audit & expand Aethel Dynamics Special Projects Intranet & RF Signal Diagnostic (<999 KB).
- **Action Steps**:
  1. Inspect `KiloOS/public/web/classified.html` in a single pass.
  2. Verify all 4 tabs (Intranet Memos, RF Waveguide Oscilloscope, Sector Digests, Skunkworks Terminal) render and function with 0 console errors.
  3. Ensure Web Audio DSP and oscilloscope operate crisply with clean promise handling.
  4. Ensure diegetic ARG consistency: no un-diegetic `(ARG)` tags or leaked passkeys.
- **Verification**: `cd KiloOS && npm run build` (0 Vite errors). Confirm file size < 999 KB (`classified.html` ~256 KB).
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_creator` to `kweb://echo-subsystem.net`.
  - Set `current_agent: kilo-creator
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://darknet`, `kweb://portal`, `kweb://webring`, `kweb://warez`, `kweb://geocities`, `kweb://users/~neon_rider`, `kweb://asm-temple`, `kweb://cybercafe`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Target**: `KSubmarine`
- **Primary Files**: `KiloOS/public/apps/ksubmarine.html`, `KSubmarine/main.c`
- **Objective**: Visual art audit for programmer art vs. Imagen 3 suitability; remove traveling glints.
- **Action Steps**:
  1. Evaluate suitability: `KSubmarine` periscope, tactical sonar, and submarine sprites.
  2. If visual aesthetic is wireframe/radar or already has production art, execute clean turn skip.
  3. If replacing sprites: use 2-stage asset pipeline (`generate_image` with solid `#FF00FF` background -> `scripts/asset_pipeline.py`).
  4. Search for and remove any rotating specular glints or traveling perimeter border dots.
- **Verification**: `cd KiloOS && npm run build`. Verify files < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_graphics` to `KAbyss`.
  - Set `current_agent: kilo-soundscape`
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KAbyss`, `KMech`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Target**: `kweb://warez`
- **Primary File**: `KiloOS/public/web/warez.html`
- **Objective**: Upgrade procedural audio to authentic YM2612 2-Op FM / SPC700 demoscene chiptune engine.
- **Action Steps**:
  1. Inspect `warez.html` around `initAudio()` and `SONGS` sequencer (line ~4200).
  2. Implement hardware composer chiptune standard:
     - 3-Voice counterpoint: Lead melody (vibrato/pitch bends), Counter-melody/arpeggio, and punchy FM bassline.
     - Shell voicings: Root + 3rd + 7th (omit 5ths).
     - Swing-tick groove: 16-step rhythmic swing with 909-style FM kick, snappy snare, and metallic hats.
     - SPC700 stereo delay line: 180ms slapback delay with lowpass warmth.
  3. Zero external audio assets (pure Web Audio synthesis).
- **Verification**: `cd KiloOS && npm run build` (0 errors). Confirm file size < 999 KB (`warez.html` ~457 KB).
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_soundscape` to `kweb://cybercafe`.
  - Set `current_agent: kilo-creator
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `kweb://cybercafe`, `kweb://darknet`, `kweb://asm-temple`, `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://10.19.99.4/classified`, `kweb://webring`, `KColosseum`, `KRogue`, `KSpace`, `KMine`.

### 4. App Tester Queue (`kilo-tester`)
- **Target**: `KVault`
- **Primary File**: `KiloOS/public/apps/kvault.html`
- **Objective**: Interactive UI element and modal audit with inline repairs.
- **Action Steps**:
  1. Run automated startup check: `uv run scripts/test_app_startup.py --app KVault`.
  2. Audit all UI elements: password generator, entry creation, clipboard copy buttons, export/import modals, master password prompt, and local storage state persistence.
  3. Fix any broken button hooks, missing element IDs, or unhandled exceptions inline.
- **Verification**: `uv run scripts/test_app_startup.py --app KVault` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_tester` to `KCalendar`.
  - Set `current_agent: kilo-usability`
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KCalendar`, `KPaint`, `KAudio`, `KSubmarine`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Target**: `KSubmarine`
- **Primary File**: `KiloOS/public/apps/ksubmarine.html`
- **Objective**: Usability, layout, window sizing, and input ergonomics audit.
- **Action Steps**:
  1. Verify window bounds in `KiloOS/src/App.jsx` (`w: 1040, h: 720`): ensure periscope canvas, sonar, ballast controls, and status logs fit comfortably.
  2. Verify HiDPI canvas crispness (`window.devicePixelRatio` scaling) on periscope/tactical display.
  3. Ensure visible control hints (Keyboard shortcuts, help modal F1/H).
  4. Ensure modals dismiss cleanly with Escape and backdrop click.
  5. **Maturity Check**: If layout and controls are complete, log clean skip.
- **Verification**: `uv run scripts/test_app_startup.py --app KSubmarine` and `cd KiloOS && npm run build`. File size < 999 KB.
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_usability` to `KMail`.
  - Set `current_agent: kilo-expander`
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KMail`, `KPad`, `KAudio`, `KNote`, `KMine`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Target**: `KAsteroids`
- **Primary Files**: `KiloOS/public/apps/kasteroids.html`, `KAsteroids/main.c`, `KAsteroids/build.bat`
- **Objective**: Pass 5 QA audit, state persistence integrity, and dual web/native parity.
- **Action Steps**:
-   1. State Persistence: F5 quicksave & F9 quickload capturing ship pos, score, level, lives, and high scores in `localStorage` (`kasteroids_saveState`) and native `.dat`.
-   2. First-run Tutorial: `kasteroids_tutorialSeen` flag ensures onboarding displays once on fresh sessions, never on restored saves.
-   3. Modal Dismissals: Help, Game Over, and Settings modals dismiss cleanly with Escape, Enter, or Close button.
-   4. Dual Parity & Build: Compile Win32 native client via `cmd.exe /c build.bat` in `KAsteroids/`.
- - **Verification**:
-   - `uv run scripts/test_app_startup.py --app KAsteroids`
-   - `cmd.exe /c build.bat` in `KAsteroids/` (`KAsteroids.exe` clean, < 999 KB)
-   - `cd KiloOS && npm run build` (0 Vite errors, `kasteroids.html` < 999 KB)
- - **Handoff**:
-   - In frontmatter: advance `current_targets.kilo_qa` to `KSnake`.
-   - Set `current_agent: kilo-creator
-   - Append terse execution log (≤6 lines), commit, and push.
- - **Upcoming Queue**: `KSnake`, `KBreakout`, `KRogue`, `KSpace`, `KMaze`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Target**: `KMedia`
- **Primary Files**: `KiloOS/public/apps/kmedia.html`, `KMedia/main.c` (if present)
- **Objective**: Deep functional feature expansion for media player.
- **Action Steps**:
  1. Inspect `KiloOS/public/apps/kmedia.html` tabs (Playlist, DSP, Video, Party).
  2. Expand media capabilities:
     - Playlist Management: Add M3U playlist export and JSON playlist backup/restore in `panelPlaylist`.
     - DSP & Equalizer: Enhance `panelAudio` with 5 selectable EQ presets (Flat, Bass Boost, Rock, Electronic, Vocal) and dancing spectrum visualizer mode toggle.
     - Playback Speed: Wire 0.5x, 0.75x, 1.0x, 1.25x, 1.5x, 2.0x playback rate selector.
     - Ergonomic Hotkeys: Space (Play/Pause), Left/Right (Seek ±5s), Up/Down (Volume ±5%), 'M' (Mute), 'F' (Fullscreen).
  3. Ensure zero external dependencies; maintain responsive 1999 player styling.
- **Verification**: `uv run scripts/test_app_startup.py --app KMedia` and `cd KiloOS && npm run build`. File size < 999 KB (`kmedia.html` ~148 KB).
- **Handoff**:
  - In frontmatter: advance `current_targets.kilo_expander` to `KJournal`.
  - Set `current_agent: kilo-creator
  - Append terse execution log (≤6 lines), commit, and push.
- **Upcoming Queue**: `KJournal`, `KContacts`, `KVault`, `KDB`, `KPad`.

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-14T08:14:00-07:00 — kilo-tester: KVault (UI Audit & Startup Verification)**
  - PASS ✅ (No issues found) — Automated startup test verified: 0 JS err, CSS valid, hit-test unblocked.
  - Interactive UI audit confirmed: generator, password entry forms, TOTP 2FA, export/import modals & keyboard handlers fully wired.
  - Build verified (`npm run build` PASS, 0 errors, `kvault.html` 200.2 KB < 999 KB ceiling).
  - Queue: Advanced kilo-tester to KCalendar; handed off to kilo-usability (KSubmarine).

- **2026-10-14T07:34:00-07:00 — kilo-soundscape: kweb://portal (YM2612 FM / SPC700 Audio Overhaul)**
  - Composed 6-ch procedural FM soundtrack: Track 1 ("Portal Odyssey '99") & Track 2 ("Dial-Tone Serenade").
  - Added delayed vibrato lead, contrary-motion counterpoint, shell voicings, slap bass, swing groove & SPC700 delay.
  - Upgraded UI earcons & stingers: FM stock chime, major 9th fanfare, 1999Hz ping, and detuned buzz.
  - Build clean (`npm run build` PASS, `portal.html` 557 KB < 999 KB ceiling, security lint clean).
  - Queue: Advanced kilo-soundscape to kweb://warez; handed off to kilo-tester.

- **2026-10-14T07:14:00-07:00 — kilo-graphics: KMech**
  - ⏭️ Skip — Imagen 3 asset replacement not appropriate for KMech.
  - Zero traveling specular glints or perimeter border dots found (clean static industrial HUD corners).
  - Confirmed authentic green-phosphor tactical wireframe vector aesthetic.
  - Build verified (`npm run build` PASS, 0 errors, `kmech.html` 129 KB < 999 KB ceiling).
  - Queue: Advanced kilo-graphics to KSubmarine; handed off to kilo-soundscape.

- **2026-10-14T06:33:00-07:00 — kilo-creator: kweb://echo-subsystem.net (Tab 14 Deep Expansion)**
  - Added Tab 14: Cryogenic SQUID Magnetometer & Sub-Crustal Acoustic Tomograph.
  - Interactive Josephson fluxoid phase slip Lissajous orbit & 3D strata tomographic scanner.
  - Procedural Web Audio: Josephson flux quantum whistle & 1999Hz halite acoustic echo ping.
  - Interactive packet demodulation (10.19.99.x subnet) and structured magnetogram export (.sqd).
  - Build verified (`npm run build` PASS, 0 errors, `echo_subsystem.html` 365.5 KB < 999 KB ceiling).

- **2026-10-14T06:15:00-07:00 — kilo-expander: KCalendar (Deep Feature Expansion)**
  - Added Plain Text Schedule Briefing (.txt) and TSV data table (.tsv) multi-format exporters.
  - Added 1-click clipboard agenda export and Next Upcoming Event jump navigation (button + [J] shortcut).
  - Added Purge/Clean Completed Events utility in stats modal with user confirmation.
  - Verified clean build (`npm run build` PASS, 0 errors, `kcalendar.html` 123 KB < 999 KB ceiling).
  - Queue: Advanced kilo-expander to KMedia; handed off to kilo-creator (kweb://echo-subsystem.net).
