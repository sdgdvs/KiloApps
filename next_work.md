---
current_agent: kilo-qa
next_agent: kilo-expander
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
  kilo_tester: KClip
  kilo_usability: KClip
  kilo_graphics: KDragon
  kilo_qa: KType
  kilo_expander: KNote
  kilo_creator: "KFleet (Fleet Telemetry Console)"
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
  agent: kilo-usability
  app: KRSS
  timestamp: "2026-09-23T00:30:00Z"
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
11. **Perimeter Glint & Traveling Comet Ban (DIRECTOR MANDATE - CRITICAL)**:
    - All agents (especially `kilo-graphics` and `kilo-usability`) MUST systematically remove rotating/traveling specular glint comets and moving perimeter border dots from both web (HTML) and native (C) on all app passes.
    - These moving dots are annoying, look like distracting projectiles/balls, and clutter gameplay across apps. Replace with clean, static, or period-accurate borders without traveling dots or orbital glint particles. NEVER add new perimeter traveling glints.

---

## Active Target Queues

### 1. App Creator & Deep Expander Queue (`kilo-creator`)
- **Current Target**: `KFleet` (Fleet Telemetry Console)
- **Upcoming Concepts**:
  `KCipher` (Cryptographic Cipher Suite), `KNetMap` (Subnet Topology Visualizer), `KSteno` (Stenographic Carrier Suite).

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Current Target**: `KDragon`
- **Upcoming Queue**:
  `KSubmarine`, `KStarDredge`, `KAbyss`, `KColosseum`, `KCyber`, `KMech`, `KMystery`, `KVoid`, `KWizard`, `KStarship`, `KChrono`, `KStarForge`, `KFortress`, `KCosmic`, `KStellar`, `KSanctuary`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KClip`
- **Upcoming Queue**:
  `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAudio`, `KBBS`, `KBase`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColosseum`, `KContacts`, `KCosmic`, `KCyber`, `KDB`, `KDragon`, `KFlash`, `KFont`, `KFortress`, `KGraph`, `KHabit`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMech`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KSanctuary`, `KScript`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`, `KTask`, `KStarForge`, `KTerm`, `KPomodoro`, `KHash`, `KRSS`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KClip`
- **Upcoming Queue**:
  `KPaint`, `KAudio`, `KFont`, `KGraph`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPass`, `KPing`, `KRadio`, `KRead`, `KScript`, `KSynth`, `KSys`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KChrono`, `KTask`, `KStarForge`, `KPad`, `KBookmark`, `KHash`, `KRSS`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KType`
- **Upcoming Queue**:
  `KVault`, `KVoid`, `KWizard`, `KZip`, `KChrono`, `KCyber`, `KDragon`, `KFortress`, `KMech`, `KMystery`, `KQuest`, `KSanctuary`, `KStarDredge`, `KSubmarine`, `KHash`, `KRSS`, `KClip`, `KTodo`, `KTrader` *(Completed in Pass 5: K2048, KAudio, KBBS, KMaze, KSnake, KSolitaire, KSpace, KStarship, KStellar, KSynth, KSys, KTask, KTerm, KTimer, KTodo, KTrader)*.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KNote`
- **Upcoming Queue**:
  `KDB`, `KTodo`, `KJournal`, `KCalendar`, `KContacts`, `KMail`, `KRead`, `KPass`, `KPaint`, `KImage`, `KAudio`, `KSynth`, `KMedia`, `KChart`, `KGraph`, `KMandel`, `KType`, `KVault`, `KZip`, `KSys`, `KTask`, `KNet`, `KPing`, `KHash`, `KRSS`, `KFont`, `KPad`.

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

- **[ALL_APPS / FLEET] — Visual Quality & Graphics** | Director Directive
  - ⚠️ AGENT NOTE: Human director request. Priority fleet-wide directive.
  - Instructions: Systematically remove rotating/traveling specular glint comets, perimeter glint dots, and moving border balls across both web (HTML) and native (Win32 C) on every app pass. They are annoying across every app and look like distracting projectiles/balls. Replace with clean, static, or period-accurate borders without traveling dots or orbital glint particles. NEVER add new perimeter traveling glints.

---

## Recent Execution Logs (Max 5 Entries)

- **2026-09-23T00:30:00Z — kilo-usability: KRSS**
  - Status: PASS ✅ (Layout dimensions, responsive breakpoints, collapsible sidebar, navigation ergonomics).
  - Window & Layout: Tuned App.jsx window to 1080x720; added responsive media queries (900px/720px) and retro scrollbars.
  - Collapsible Sidebar: Added 1-click & hotkey [B] sidebar collapse to maximize reader pane width across small displays.
  - Navigation & Reader: Added Prev/Next article buttons [◀]/[▶]; wired J/K/1-9 active card auto-scroll-into-view.
  - Font Zoom & State: Persisted zoomLevel and sidebar state in localStorage; added live zoom px display and +/-/0 keys.
  - Discoverability & Native: Added visible F1/H prompts and H/M/S/R key accelerators across web and native Win32 C.
  - Verification: MSVC C clean build (17.5 KB); Vite web build clean (224ms); security lint passed (0 violations).

- **2026-09-23T00:15:00Z — kilo-tester: KRSS**
  - Status: PASS ✅ (5 UI/interactive issues, 5 fixed).
  - Modal Dismissals: Added dimmed backdrop click handling and Space/Esc dismissal across Splash, Add Feed, OPML, and Help.
  - Keyboard Controls: Fixed Shift+M view-read action; wired 1-9 direct headline jumps, Enter/Space splash exit, and F1 help toggle.
  - OPML & State JSON: Added JSON state file import & auto-detection of pasted JSON into OPML manager; wired custom feed removal [✕].
  - Reading & Ergonomics: Decoupled font zoom from reader scroll position; hardened markdown & clipboard exports against null content.
  - Verification: MSVC C clean build (17.9 KB); Vite web build clean (209ms); security lint passed (0 violations).

- **2026-09-23T00:05:00Z — kilo-graphics: KSanctuary**
  - Status: PASS ✅ (Graphics polish, GDI sprite parity, raider emblems, balance pass).
  - Pixel Art & Sprites: GDI sprites in Win32 C & SVGs in web for 5 raider warbands, defense turrets, barricades, and weather hazards.
  - Visual Parity: Added Vault 811 tech archive landmark sprite, surface sentry pillbox, and workshop cutaway dweller.
  - Specular & Glint Audit: Verified zero traveling border comets or orbiting glints in native C and web HTML.
  - Gameplay Balance: Tuned barricade defense scaling, sentry overclock bonuses, and mitigated drought water penalty.
  - Verification: MSVC C clean build (237 KB); Vite web build clean (222ms); security lint passed (0 violations).

- **2026-09-22T23:45:00Z — kilo-creator: KAnomaly**
  - Status: PASS ✅ (Created Subterranean Signal Analyzer across Win32 C & HTML5 web with 100% feature parity).
  - Scientific Core: Real-time waterfall spectrogram, 64-band FFT, acoustic oscilloscope, and geophone strata profiling.
  - Multi-Sensor Array: 6 global borehole observatories (Kola, Carlsbad, Mariana, Yamantau, Hadron, Atacama).
  - Cryptographic Intercept: 12 subterranean anomalies with SSTV raster decoding and TDOA hyperbolic epicenter triangulation.
  - Audio Architecture: Procedural Sega Genesis (YM2612 FM dual-op) and SNES warm geophone rumble audio engine.
  - State & Usability: Splash screen, first-run tutorial modal, quicksave [F5]/quickload [F9], and JSON state export/import.
  - Verification: MSVC C clean build (19.0 KB); Vite web build clean (212ms); security lint passed (0 violations).

- **2026-09-22T23:35:00Z — kilo-expander: KPad**
  - Status: PASS ✅ (Deep feature expansion across Win32 C & HTML5 web with 1:1 functional parity).
  - Productivity & Session: Multi-tab tagging, tag filtering, pinned tabs, and global search index modal across all open tabs.
  - Formats & Previews: Live Markdown/HTML split preview, 2-way CSV ⇄ MD table converter, workspace JSON snapshot export/import.
  - Native Parity: Markdown export (.md) with frontmatter header, reverse line order tool, line endings and reading time stats.
  - Verification: MSVC C clean build (29.0 KB); Vite web build clean (213ms); security lint passed (0 violations).
