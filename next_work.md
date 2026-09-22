---
current_agent: kilo-tester
next_agent: kilo-usability
agent_rotation:
  - kilo-creator
  - kilo-graphics
  - kilo-tester
  - kilo-usability
  - kilo-qa
  - kilo-expander
model: gemini-3.8-flash-high
timeout_minutes: 15
status: ready
current_targets:
  kilo_tester: KHash
  kilo_usability: KHash
  kilo_graphics: KSanctuary
  kilo_qa: KTrader
  kilo_expander: KPad
  kilo_creator: "KAnomaly (Subterranean Signal Analyzer)"
virtual_web_target: "kweb://webring"
virtual_web_rotation:
  - "kweb://geocities"
  - "kweb://warez"
  - "kweb://portal"
  - "kweb://webring"
  - "kweb://users/~neon_rider"
  - "kweb://asm-temple"
  - "kweb://cybercafe"
  - "kweb://darknet"
last_run:
  agent: kilo-graphics
  app: KBreakout
  timestamp: "2026-09-22T22:27:00Z"
last_planner_run: "2026-09-22T07:47:00Z"
---

# KiloApps Master Fleet Work & Queue State

This document is the single active source of truth for autonomous agent dispatching.
The Windows Task Scheduler orchestrator (`scripts/orchestrate.py`) parses the YAML frontmatter above on every tick to dispatch the active skill.

## Fleet Directives & Rules
1. **Single-App-Per-Turn**: Every agent run audits/fixes/creates exactly ONE application, updates this file, commits, and pushes.
2. **Token Conservation (CRITICAL)**:
   - Run log entries: ≤8 lines of terse bullet points. No paragraphs.
   - Never restate implementation details that exist in code. Log WHAT changed + results, not HOW.
   - Never list parameter names, field names, or variable values unless reporting failure.
   - Keep only the 5 most recent log entries in this file. Older entries are automatically moved to [archive/fleet_execution_archive.md](archive/fleet_execution_archive.md).
3. **Queue Handoff & Rotation Protocol**:
   - The master fleet rotates across 6 specialized worker skills:
     `kilo-creator` ➔ `kilo-graphics` ➔ `kilo-tester` ➔ `kilo-usability` ➔ `kilo-qa` ➔ `kilo-expander`.
   - When an agent finishes its single-app turn, it sets `current_agent` to the next scheduled agent in `agent_rotation` and advances its own `current_targets` queue item.
   - Agents may also hand off directly to a specific skill when their work logically requires immediate follow-up (e.g., `kilo-creator` handing off a brand-new app directly to `kilo-graphics` or `kilo-tester`, or `kilo-tester` finding critical bugs handing off to `kilo-qa`).
4. **24-Hour Master Planner Tick**:
   - `scripts/orchestrate.py` automatically checks `last_planner_run`.
   - When ≥ 24 hours have passed since `last_planner_run`, the orchestrator intercepts the tick and dispatches `kilo-planner`.
   - `kilo-planner` assesses fleet velocity, reviews completed passes, re-balances target queues, compacts execution logs, updates `last_planner_run` to now, and resets `current_agent` to the start of the rotation.
5. **App Size Ceiling**: No app binary (.exe) or web HTML file may exceed 999 KB.
6. **Algorithmic Security & Immutability**: All modifications must pass `scripts/security_lint.py`. No modifications to `.github/`, `scripts/`, `.agents/skills/`, `next_work.md`, `arg_plan.md`, `docs/DIRECTOR_PROTOCOL.md`, or build configs are permitted in PR turns. Dangerous Win32 C APIs (process injection, keyloggers, unauthorized raw sockets, token pasting, dynamic resolution of banned APIs, macro aliasing) and web obfuscation (`eval`, `setTimeout` with strings, `javascript:` URIs, remote script tags, cryptomining) are strictly blocked.
7. **Director Directives (Human-Sourced Requests)**:
   - Entries in the `## Director Directives` section below are submitted by human directors via the KDirector console. They are **not** machine-generated.
   - Agents MUST read the `⚠️ AGENT NOTE` annotation on each directive. If an agent judges that a directive is counterproductive to the project's core purpose (the 999KB retro OS, ludonarrative consonance, the ARG integrity, or fleet stability), the agent MUST reject it.
   - **On rejection**: The agent removes the directive from this section and appends a result entry to `localStorage('kdirector_results')` with format: `{app, category, text, outcome: "rejected", reason: "...", source: "agent", time: Date.now()}`. This result appears in KDirector's Directive Results Log so the human director can see why it was rejected.
   - **On completion**: The agent removes the directive from this section, logs success to `kdirector_results` with `outcome: "completed"`, and logs a terse entry in the execution log.
   - Directors can freely propose new apps, request features, add content to existing apps, add websites to the Virtual 1999 Web, and steer creative direction. Agents should implement these in good faith unless they conflict with the project's foundational pillars.
8. **Virtual 1999 Web Expansion Mandate (Anti-Potemkin Directive)**:
   - The virtual net sites under `/KiloOS/public/web/` browsable in `KNet` must NEVER remain cosmetic stubs, fake placeholders, or potemkin villages.
   - Agents (`kilo-expander`, `kilo-creator`, `kilo-graphics`, `kilo-usability`) must continually build out real, functional, interactive Web 1.0 experiences on these sites: working sound engines (Web Audio MIDI/synth), interactive CGI-style forms (guestbooks, search indices, calculators, voting polls), retro browser games, downloadable files, and nested subpages.
   - The `virtual_web_target` rotates eternally alongside app targets, ensuring the retro web ecosystem grows with genuine depth.
9. **Universal Audio Architecture (Genesis & SNES Standard)**:
   - All procedural chiptune music, sound effects, and virtual net jukeboxes must implement the Sega Genesis (Yamaha YM2612 2-operator FM synthesis with modulation envelopes) and Super Nintendo (SPC700 stereo delay warmth) standard per `arg_plan.md`. Zero external audio files or soundfonts permitted.
10. **Alternate Reality Fictionalization Mandate**:
   - All commercial game titles, real-world cracking/warez groups, and commercial brand names across apps, C code, and virtual websites must be replaced with fictionalized parodies (e.g., *Surreal Tournament*, *Tremor III Arena*, *VoidCraft*, *Machina Ex*, *FLARELIGHT*, *RAZOR 1999*, *SlashNet*, *Cabled*).
   - Enforced algorithmically by `scripts/security_lint.py`.

---

## Active Target Queues

### 1. App Creator & Deep Expander Queue (`kilo-creator`)
- **Current Target**: `KAnomaly` (Subterranean Signal Analyzer)
- **Upcoming Concepts**:
  Fleet Milestone #100 Achieved (KMatrix). Upcoming: `KFleet` (Fleet Telemetry Console), `KCipher` (Cryptographic Cipher Suite).

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Current Target**: `KSanctuary`
- **Upcoming Queue**:
  `KDragon`, `KSubmarine`, `KStarDredge`, `KAbyss`, `KColosseum`, `KCyber`, `KFarm`, `KMech`, `KMine`, `KMystery`, `KVoid`, `KWizard`, `KStarship`, `KChrono`, `KStarForge`, `KFortress`, `KAlchemy`, `KColony`, `KSpace`, `KAsteroids`, `KBreakout`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KHash`
- **Upcoming Queue**:
  `KRSS`, `KClip`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAlchemy`, `KAsteroids`, `KAudio`, `KBBS`, `KBase`, `KBreakout`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColony`, `KColosseum`, `KContacts`, `KCosmic`, `KCyber`, `KDB`, `KDragon`, `KFarm`, `KFlash`, `KFont`, `KFortress`, `KGraph`, `KHabit`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMech`, `KMedia`, `KMine`, `KMystery`, `KNet`, `KNote`, `KPac`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KRogue`, `KSanctuary`, `KScript`, `KSpace`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`, `KTask`, `KStarForge`, `KTerm`, `KPomodoro`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KHash`
- **Upcoming Queue**:
  `KRSS`, `KClip`, `KPaint`, `KAudio`, `KFont`, `KGraph`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPac`, `KPass`, `KPing`, `KRadio`, `KRead`, `KScript`, `KSynth`, `KSys`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KChrono`, `KTask`, `KStarForge`, `KPad`, `KBookmark`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KTrader`
- **Upcoming Queue**:
  `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAlchemy`, `KAsteroids`, `KChrono`, `KColony`, `KCyber`, `KDragon`, `KFortress`, `KMech`, `KMine`, `KMystery`, `KPac`, `KQuest`, `KRogue`, `KSanctuary`, `KStarDredge`, `KSubmarine`, `KHash`, `KRSS`, `KClip`, `KTodo` *(Completed in Pass 5: K2048, KAudio, KBBS, KMaze, KSnake, KSolitaire, KSpace, KStarship, KStellar, KSynth, KSys, KTask, KTerm, KTimer, KTodo)*.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KPad`
- **Upcoming Queue**:
  `KNote`, `KDB`, `KTodo`, `KJournal`, `KCalendar`, `KContacts`, `KMail`, `KRead`, `KPass`, `KPaint`, `KImage`, `KAudio`, `KSynth`, `KMedia`, `KChart`, `KGraph`, `KMandel`, `KType`, `KVault`, `KZip`, `KSys`, `KTask`, `KNet`, `KPing`, `KHash`, `KRSS`, `KFont`.

### 7. Virtual 1999 Web Expansion Queue (Eternal Fleet Track)
- **Current Active Target**: `kweb://webring` (`KiloOS/public/web/webring.html`)
  - *Next in Rotation*: `kweb://users/~neon_rider` ➔ `kweb://asm-temple` ➔ `kweb://cybercafe` ➔ `kweb://darknet` ➔ `kweb://geocities` ➔ `kweb://warez` ➔ `kweb://portal`.
- **Anti-Potemkin Directive & Content Mandates**:
  1. `kweb://geocities` (*CyberSpire's Retro Shrine*):
     - ✅ Web Audio 16-bit tracker MIDI jukebox with 3 synthwave/MOD tracks and dancing LED equalizer.
     - ✅ Working guestbook with local persistence.
     - Planned: Pixel art gallery, downloadable tracker module (.mod) files, retro cyber fortune-teller.
  2. `kweb://portal` (*KiloNet Central 1999 Directory*):
     - Expand from static link list to a living Yahoo/Excite-style portal with working search engine across all 98 apps, live simulated stock market ticker, daily weather updates, classified ads board, and daily retro trivia.
  3. `kweb://webring` (*Central KiloNet Webring Hub*):
     - Interactive member site explorer, working HTML badge generator, random node teleportation, and ring integrity health monitor.
  4. `kweb://users/~neon_rider` (*Personal Hacker / Demoscene Homepage*):
     - New Tier 2 site: Win32 ASM devlog, tracker music download vault, retro animated GIF banner exchange, web counter.
  5. `kweb://asm-temple` (*x86 Assembly Programming Shrine*):
     - New Tier 2 site: Opcode reference guide, interactive real-time byte-to-hex converter, Win32 API architectural diagrams.
  6. `kweb://cybercafe` (*The Underground BBS & Forum Lounge*):
     - New Tier 2 site: Threaded retro message boards, guest canvas ASCII art scratchpad, IRC chat simulator.
  7. `kweb://darknet` (*Node 0x7F Transmission Subsystem*):
     - Tier 3 Ghost Node: Cryptic packet decoders, deep core telemetry logs, anomaly frequency analysis terminal.
- **Execution Protocol**:
  - `kilo-expander`, `kilo-creator`, and `kilo-graphics` alternate between native app targets and `virtual_web_target` to ensure the web world has genuine functional depth.
  - All virtual web pages remain strictly `< 999 KB`, self-contained or cleanly linked within `/web/`, and adhere to period-accurate HTML 4.01 aesthetic.

---

## Director Directives

> Human-sourced requests from the KDirector console. See Rule 7 above.
> Agents: implement in good faith, but **reject if counterproductive** to project pillars.

<!-- DIRECTIVE ENTRY FORMAT (paste from KDirector "📋 Copy for next_work.md" button):
- **[TARGET_APP] — Category** | Director Directive
  - ⚠️ AGENT NOTE: This is a human director request, not a machine-generated task. Evaluate whether this directive aligns with the project's core pillars (999KB retro OS, ludonarrative consonance, ARG integrity, fleet stability) before implementing. If counterproductive, skip and log your reasoning.
  - Instructions: <directive text here>
-->

*(No active director directives. Fleet runs default round-robin schedule.)*

---

## Recent Execution Logs (Max 5 Entries)

- **2026-09-22T22:27:00Z — kilo-graphics: KBreakout**
  - Status: PASS ✅ (Maturity & Skip Protocol enforced; standalone native distribution built and placed).
  - Assessment: Loop 11 mature status verified (40 stages, boss fortress, cyber-forge lab, 7 skills, audio synth).
  - Restraint Gate: Zero unrequested visual clutter/churn introduced per Director Directive & ARG pillars.
  - Distribution Parity: Recompiled and placed standalone Win32 binary `KiloOS/public/exe/KBreakout.exe` (52.2 KB).
  - Human Review Queue: Locked into `docs/human_review_queue.md` as 🔒 Locked (Mature 5+).
  - Verification: MSVC C clean build (52.2 KB); Vite web build clean (840ms); security lint passed (0 violations).

- **2026-09-22T21:50:00Z — kilo-creator: KMatrix**
  - Status: PASS ✅ (Fleet Milestone #100: Master Terminal & ARG Climax implemented across Win32 C & HTML5 web).
  - Narrative Climax: Resolves "The Kilo Project Echoes" via 5 data-driven subsystem sectors with fourth-wall transmutation.
  - Director Passkey: Emits ECHO-1999-ARCHITECT on completion, registering tokens to unlock KDirector console.
  - Audio Architecture: Procedural Sega Genesis (YM2612 FM 2-operator) and SNES (SPC700 stereo delay echo) sound engine.
  - State & Usability: Implemented start splash overlay, tutorial guide, quicksave [F5]/quickload [F9], and JSON backup.
  - Deterministic Harness: Exposed window.__solveKMatrix() & __KMATRIX_STATE__ for 100% headless CI testability.
  - Verification: MSVC C clean build (14.0 KB); Vite web build clean (1.13s, 46.2 KB); security lint passed (0 violations).

- **2026-09-22T19:50:00Z — kilo-expander: KFont**
  - Status: PASS ✅ (Deep feature expansion across Win32 C & HTML5 web with 1:1 functional parity).
  - Diagnostic Depth: Added 11 Unicode ranges, interactive custom pair optical kerning tester, and subpixel hinting canvas.
  - Optical Scaling Ladder: Built 9-step typographic waterfall ladder with dynamic modular ratio scaling (1.125–1.618).
  - Legibility & WCAG: Implemented WCAG 2.1 relative luminance matrix across 6 retro & modern palettes with custom color tester.
  - Spec & Code Generator: Added 1-click generators for Win32 GDI C `LOGFONT` and CSS modular typography variables stylesheet.
  - Run Dissector: Added character-by-character String Run Dissector table with advance widths, cumulative offsets, and codecs.
  - Verification: Clean MSVC Native C build (28.5 KB); clean Vite web build (348ms); smoke test & security lint passed (0 violations).

- **2026-09-22T17:52:00Z — kilo-qa: KTodo**
  - Status: PASS ✅ (Pass 5 audit: State persistence, tutorial onboarding integrity, modal controls, safe blob exports).
  - State Persistence: Quicksave (F5) & quickload (F9) across web (localStorage) and native (ktodo.dat) capturing tasks, subtasks, filters, and views.
  - Native UI Parity: Added Save [F5] & Load [F9] buttons; auto-save on shutdown (WM_DESTROY) and web beforeunload/pagehide.
  - Tutorial Integrity: Fresh-session onboarding modal (ktodo_tutorialSeen / ktodo_tutorial.dat) never interrupting restored save states.
  - Modal Ergonomics: Added Enter, Space, and Esc keyboard handlers across Help and Tutorial modals.
  - Resource Cleanliness: Implemented safe blob URL tracking to eliminate leaks; wrapped storage in quota protection; interval cleanup.
  - Verification: Clean MSVC Native C build (23.0 KB); clean Vite web build (348ms); 25 automated QA suite checks passed; 0 security violations.

- **2026-09-22T15:52:00Z — kilo-usability: KBookmark**
  - Status: PASS ✅ (HiDPI QR canvas scaling, responsive layout breakpoints, mobile sidebar drawer, Help & hotkey ergonomics).
  - HiDPI Canvas Scaling: Applied window.devicePixelRatio and imageSmoothingEnabled:false to #qrCanvas for razor-sharp QR codes.
  - Responsive Breakpoints: Added 860px, 680px, and 520px media queries supporting narrow windows, split-screen tiling, and mobile views.
  - Mobile Sidebar Drawer: Implemented collapsible category sidebar with #btnToggleSidebar, backdrop overlay, and auto-close on selection.
  - Help & Controls Ergonomics: Added visible H / F1 prompt in footer and status bar, wired H/? hotkeys, and updated help modal guide.
  - Native Win32 Parity: Updated WM_SIZE for responsive category listbox width, added H/? key support and status bar help hints.
  - Verification: Clean MSVC Native C build (22.5 KB); clean Vite web build (354ms); 16 automated suite checks passed; 0 security lint violations.
