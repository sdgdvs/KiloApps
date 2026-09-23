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
  kilo_tester: KClip
  kilo_usability: KClip
  kilo_graphics: KSubmarine
  kilo_qa: KVault
  kilo_expander: KNote
  kilo_creator: "KCipher (Cryptographic Cipher Suite)"
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
  app: KDragon
  timestamp: "2026-09-23T01:17:00Z"
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
- **Current Target**: `KCipher` (Cryptographic Cipher Suite)
- **Upcoming Concepts**:
  `KNetMap` (Subnet Topology Visualizer), `KSteno` (Stenographic Carrier Suite).

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Current Target**: `KSubmarine`
- **Upcoming Queue**:
  `KStarDredge`, `KAbyss`, `KColosseum`, `KCyber`, `KMech`, `KMystery`, `KVoid`, `KWizard`, `KStarship`, `KChrono`, `KStarForge`, `KFortress`, `KCosmic`, `KStellar`, `KSanctuary`, `KDragon`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KClip`
- **Upcoming Queue**:
  `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAudio`, `KBBS`, `KBase`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColosseum`, `KContacts`, `KCosmic`, `KCyber`, `KDB`, `KDragon`, `KFlash`, `KFont`, `KFortress`, `KGraph`, `KHabit`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMech`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KSanctuary`, `KScript`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`, `KTask`, `KStarForge`, `KTerm`, `KPomodoro`, `KHash`, `KRSS`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KClip`
- **Upcoming Queue**:
  `KPaint`, `KAudio`, `KFont`, `KGraph`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPass`, `KPing`, `KRadio`, `KRead`, `KScript`, `KSynth`, `KSys`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KChrono`, `KTask`, `KStarForge`, `KPad`, `KBookmark`, `KHash`, `KRSS`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KVault`
- **Upcoming Queue**:
  `KVoid`, `KWizard`, `KZip`, `KChrono`, `KCyber`, `KDragon`, `KFortress`, `KMech`, `KMystery`, `KQuest`, `KSanctuary`, `KStarDredge`, `KSubmarine`, `KHash`, `KRSS`, `KClip`, `KTodo`, `KTrader`, `KType` *(Completed in Pass 5: K2048, KAudio, KBBS, KMaze, KSnake, KSolitaire, KSpace, KStarship, KStellar, KSynth, KSys, KTask, KTerm, KTimer, KTodo, KTrader, KType)*.

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

- **2026-09-23T01:17:00Z — kilo-graphics: KDragon**
  - Status: PASS ✅ (Graphics, enemy archetypes, Astral evolution, balance pass, glint audit).
  - Web & Native: Purged lingering glint properties; verified zero traveling comets or perimeter dots.
  - Game Content: Added 6 enemy archetypes, elemental counters, 4th evolution (Astral Dragon), and Hoard Relics tray.
  - Audio & VFX: Added Genesis YM2612 FM / SPC700 delay audio engine, floating damage numbers, and colored shockwaves.
  - Balance: Re-tuned arena progression, special ability cooldowns, and minigame rewards across web and native.
  - Verification: Native MSVC C (137.5 KB); Single-file HTML5 (120.9 KB); Vite build clean; security lint 0 violations.

- **2026-09-23T01:15:00Z — kilo-creator: KFleet**
  - Status: PASS ✅ (Created new app KFleet: Fleet Telemetry Console).
  - Scope: Distributed fleet monitor with 8 nodes, tactical vector radar, multi-channel oscilloscope, CLI uplink.
  - Mandatory Specs: Retro splash screen, first-run tutorial flag, F5/F9 quicksave/quickload, JSON backup/import.
  - Audio Engine: 2-operator FM synthesis (Genesis YM2612) with warm delay (SPC700 standard), zero external assets.
  - Compliance: No perimeter glints/comets (Rule 11), ARG consonance (Node 0x7F lore), strict size ceiling (<999KB).
  - Verification: Native MSVC C (18.0 KB); Single-file HTML5 (78.5 KB); Vite clean build; security lint 0 violations.

- **2026-09-23T01:00:00Z — kilo-vision-audit: Fleet Comprehensive**
  - Status: PASS ✅ (92 primary apps audited across 5 dimensions using native AI vision).
  - Scope & Scoring: 92/92 apps scored (Fleet avg: 8.84/10; 0 apps below 5.0 threshold).
  - Perfect 10s: KConverter, KFlash, KHabit, KMail, KPad, KRead, KStarDredge, KTimer, KVault, KZip.
  - Glint Audit: Flagged lingering specular border comets across games; stripped 10 apps in commit 67d1d81c.
  - Gallery Integration: Enriched docs/gallery/index.html and vision_scores.json with visual badges.

- **2026-09-23T01:00:00Z — kilo-graphics: Fleet-Wide Specular Glint Purge**
  - Status: PASS ✅ (Fleet Rule 11 & Director Directive 100% complete across all 31 native C and 33 web apps).
  - Scope: Purged rotating specular glint comets, traveling perimeter border dots, and moving border balls.
  - Native Win32 C: Purged glints in K2048, KChess, KConnect4, KSolitaire, KSudoku, KTowers, KGo, KReversi, KMines, KFreecell, KAsteroids, KPong, KSpace, KSnake, KPac, KTetris, KDarts, KSimon, KColony, KDragon, KFarm, KFortress, KMatch3, KQuest, KStarship, KMandel, KHex, KTrader, KWords, KHangman, KMine, KMystery.
  - HTML5 Web: Purged CSS/canvas perimeter glints across all 33 corresponding web apps; preserved authentic weapon/mob sprites.
  - Preserved Authenticity: Retained static period-accurate frames, corner filigrees, and genuine in-game highlights.
  - Verification: All 31 native binaries recompiled (<255 KB each); Vite web build clean (223ms); security lint 0 violations.

- **2026-09-23T00:45:00Z — kilo-qa: KType**
  - Status: PASS ✅ (Pass 5 audit: State persistence, tutorial onboarding integrity, modal controls, size limits).
  - State Persistence: Quicksave (F5) & quickload (F9) across web (localStorage) and native (ktype.dat) capturing full state.
  - Native UI Parity: Added Save [F5], Load [F9] headers, auto-recovery on launch, status toast banner, and C state file IO.
  - Tutorial Integrity: Fresh-session onboarding modal (ktype_tutorialSeen / ktype_tutorial.dat) never interrupting restored save states.
  - Modal Ergonomics: Added backdrop dismissal and Esc/Enter/Space handlers across Help and Tutorial modals.
  - Safety & Storage: Wrapped all storage access with quota-safe helpers; cleaned up object URLs and timer leaks.
  - Verification: MSVC C clean build (22.0 KB); Vite web build clean (221ms); security lint passed (0 violations).
