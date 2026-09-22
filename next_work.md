---
current_agent: kilo-qa
next_agent: kilo-expander
agent_rotation:
  - kilo-tester
  - kilo-usability
  - kilo-graphics
  - kilo-qa
  - kilo-expander
  - kilo-creator
model: gemini-3.8-flash-high
timeout_minutes: 15
status: ready
current_targets:
  kilo_tester: KPomodoro
  kilo_usability: KBookmark
  kilo_graphics: KAsteroids
  kilo_qa: KTimer
  kilo_expander: KFont
  kilo_creator: "KClip (Clipboard history tool)"
virtual_web_target: "kweb://portal"
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
  app: KSpace
  timestamp: "2026-09-22T04:05:00Z"
last_planner_run: "2026-09-21T05:46:00Z"
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
- **Current Target**: `KClip` (Clipboard history tool)
- **Upcoming Concepts**:
  `KMatrix` (Master Terminal & ARG Climax).

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Current Target**: `KAsteroids`
- **Upcoming Queue**:
  `KBreakout`, `KSanctuary`, `KDragon`, `KSubmarine`, `KStarDredge`, `KAbyss`, `KColosseum`, `KCyber`, `KFarm`, `KMech`, `KMine`, `KMystery`, `KVoid`, `KWizard`, `KStarship`, `KChrono`, `KStarForge`, `KFortress`, `KAlchemy`, `KColony`, `KSpace`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KPomodoro`
- **Upcoming Queue**:
  `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAlchemy`, `KAsteroids`, `KAudio`, `KBBS`, `KBase`, `KBreakout`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColony`, `KColosseum`, `KContacts`, `KCosmic`, `KCyber`, `KDB`, `KDragon`, `KFarm`, `KFlash`, `KFont`, `KFortress`, `KGraph`, `KHabit`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMech`, `KMedia`, `KMine`, `KMystery`, `KNet`, `KNote`, `KPac`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KRogue`, `KSanctuary`, `KScript`, `KSpace`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`, `KTask`, `KStarForge`, `KTerm`, `KBookmark`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KBookmark`
- **Upcoming Queue**:
  `KPaint`, `KAudio`, `KFont`, `KGraph`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPac`, `KPass`, `KPing`, `KRadio`, `KRead`, `KScript`, `KSynth`, `KSys`, `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KChrono`, `KTask`, `KStarForge`, `KPad`, `KPomodoro`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KTimer`
- **Upcoming Queue**:
  `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAlchemy`, `KAsteroids`, `KChrono`, `KColony`, `KCyber`, `KDragon`, `KFortress`, `KMech`, `KMine`, `KMystery`, `KPac`, `KQuest`, `KRogue`, `KSanctuary`, `KStarDredge`, `KSubmarine`, `KTerm` *(Completed in Pass 5: K2048, KAudio, KBBS, KMaze, KSnake, KSolitaire, KSpace, KStarship, KStellar, KSynth, KSys, KTask, KTerm)*.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KFont`
- **Upcoming Queue**:
  `KPad`, `KNote`, `KDB`, `KTodo`, `KJournal`, `KCalendar`, `KContacts`, `KMail`, `KRead`, `KPass`, `KPaint`, `KImage`, `KAudio`, `KSynth`, `KMedia`, `KChart`, `KGraph`, `KMandel`, `KType`, `KTerm`, `KVault`, `KZip`, `KSys`, `KTask`, `KNet`, `KPing`, `KHex`, `KBase`.

### 7. Virtual 1999 Web Expansion Queue (Eternal Fleet Track)
- **Current Active Target**: `kweb://portal` (`KiloOS/public/web/portal.html`)
  - *Next in Rotation*: `kweb://webring` ➔ `kweb://users/~neon_rider` ➔ `kweb://asm-temple` ➔ `kweb://cybercafe` ➔ `kweb://darknet` ➔ `kweb://geocities`.
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

- **2026-09-22T04:35:00Z — director-task: Alternate Reality Fictionalization Mandate & Automated Checks**
  - Status: PASS ✅ (Fictionalized remaining commercial brands; codified parody mandate; automated security lint).
  - KRSS Fictionalization: Replaced Quake III/Unreal with Tremor III/Surreal Tournament; Slashdot/Wired/Onion with SlashNet/Cabled/The Scallion across C and HTML.
  - Foundational Docs: Codified parody standards in .agents/AGENTS.md, arg_plan.md Section 7, next_work.md Rule 10, and worker SKILL.md files.
  - Automated Gatekeeper: Added BANNED_TRADEMARK_PATTERNS to scripts/security_lint.py scanning C, HTML, JS, and JSX.
  - Verification: Clean Vite build; full repo security lint passed (0 violations); all sizes strictly < 999 KB.

- **2026-09-22T04:05:00Z — kilo-graphics: KSpace**
  - Status: PASS ✅ (Multi-chassis fighter customization, procedural hulls & plumes, runaway speed fix, boss rush balance).
  - Multi-Chassis System: 3 distinct hulls (Alpha Interceptor, Crimson Vanguard, Void Phantom) with unique geometries, colorways, thruster plumes, hotkeys ([C]/[F4]), and persistence.
  - Runaway Speed Fix: Replaced unbounded speed formula with smoothly clamped curve, resolving telefragging at score > 10,000.
  - Balance & Boss Rush: Guaranteed emergency shield/repair drop on boss defeat in Boss Rush; tuned companion drone vulcan velocity.
  - Visuals & Ergonomics: Animated live ship preview in menu; clickable chassis HUD and menu badges; updated help screen.
  - Parity & Sizes: Native Win32 (76.0 KB) and Web (144.2 KB) maintain 1:1 parity and stay strictly < 999 KB ceiling.
  - Verification: Clean MSVC build; clean Vite build (338ms); security linter and automated native smoke suite passed cleanly.

- **2026-09-22T04:22:00Z — director-task: 0xRELEASE Warez Portal, Cracktros & Audio Standard**
  - Status: PASS ✅ (warez.html launched with 4 interactive cracktros & Genesis/SNES FM audio).
  - Warez Archive: Fictionalized 1999 scene parodies (Flarelight, Razor 1999, Paralax, Skid Vector) with filterable catalog.
  - Interactive Cracktros: 3D vector rotating polyhedra (cube, octahedron, star, torus), copper raster bars, sine scroller, 3D starfield.
  - Genesis/SNES Synthesis: 2-op FM (YM2612) slap-bass/brass and SPC700 stereo delay echoes with zero external audio assets.
  - Subterranean Darknet Model: Embedded cryptic gateway in NFO CRC32 checksum and keygen seeds routing to kweb://darknet.
  - Fleet Audio Standard: Enforced Genesis/SNES audio architecture across arg_plan.md, skills, and next_work.md Rule 9.
  - Verification: warez.html is 52.8 KB (<999 KB); clean Vite build; security lint passed.

- **2026-09-22T03:42:00Z — director-task: CyberSpire 2-Minute Demoscene & Keygen Soundtrack Upgrade**
  - Status: PASS ✅ (3 full 2-minute demoscene/keygen compositions with drops, risers, and tracker synthesis).
  - Compositions: Track 1 (135 BPM Synthwave, 02:08), Track 2 (128 BPM Amiga MOD, 02:15), Track 3 (144 BPM Keygen, 02:13).
  - Demoscene Engine: 50Hz SID keygen fast arps, Roland TB-303 resonant acid bass, portamento lead slides, 1.4s crash cymbals.
  - Drops & SFX: Added pre-drop white noise filter risers, laser zaps, dynamic pattern arrangement, and live LCD pattern tracking.
  - Stereo Space: Integrated 3/16th tempo-synced feedback tape delay with lowpass filtering.
  - Verification: geocities.html is 51.8 KB (<999 KB ceiling); clean Vite build; security lint passed.

- **2026-09-22T03:18:00Z — director-task: CyberSpire Retro Shrine & Virtual Web Mandate**
  - Status: PASS ✅ (Web Audio MIDI jukebox implemented; eternal Anti-Potemkin web task established).
  - Jukebox Audio: Built 4-voice polyphonic Web Audio tracker engine (lead, arp, bass, noise drums) with 3 tracks (135/126/140 BPM).
  - Visualizer & Controls: Animated 16-band LED peak meter, green LCD marquee, track select, volume/mute, and user unlock gesture.
  - Eternal Fleet Track: Added Rule 8, virtual_web_target rotation in next_work.md, SKILL.md updates, and arg_plan.md quality standards.
  - Verification: geocities.html is 29.3 KB (<999 KB ceiling); clean Vite build; security lint passed.
