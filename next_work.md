---
current_agent: kilo-usability
next_agent: kilo-graphics
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
  kilo_tester: KCalc
  kilo_usability: KCalc
  kilo_graphics: KStarDredge
  kilo_qa: KWizard
  kilo_expander: "KChess (Firebase RTDB Multiplayer)"
  kilo_creator: "KSteno (Stenographic Carrier Suite)"
virtual_web_target: "kweb://users/~neon_rider"
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
  agent: kilo-tester
  app: KPomodoro
  timestamp: "2026-09-23T11:50:00Z"
last_planner_run: "2026-09-23T04:40:00Z"
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
12. **Seamless Online Multiplayer via Firebase (DIRECTOR MANDATE - CRITICAL)**:
    - `kilo-creator` and `kilo-expander` must concentrate on adding online multiplayer features that work seamlessly through Firebase Realtime Database (`https://kiloappschat-default-rtdb.firebaseio.com`).
    - Players from different computers anywhere on the internet visiting `kiloapps.web.app` who are not otherwise communicating must be able to play together in real-time without needing custom servers, shared LANs, or external communication tools—identical to how KChat connects global users in the `#general` room.
    - Priority focus: turn-based board & strategy games (*KChess*, *KConnect4*, *KGo*, *KReversi*, *KDarts*, *KBattleship*), competitive arcade duel modes (*KTetris*, *K2048*, *KSnake*), and collaborative apps (*KDraw*, *KPaint*, *KSynth*, *KPad*). Always preserve offline/solo/vs-AI mode as a graceful fallback.

---

## Active Target Queues

### 1. App Creator & Deep Expander Queue (`kilo-creator`)
- **Current Target**: `KSteno` (Stenographic Carrier Suite)
- **Upcoming Concepts**:
  `KPacket` (Packet Inspector), `KAudioTrack` (Multitrack Tracker Studio).
- **Multiplayer Focus (CRITICAL)**: Prioritize concepts and games featuring seamless cross-computer Firebase Realtime Database multiplayer (`https://kiloappschat-default-rtdb.firebaseio.com`), allowing players on `kiloapps.web.app` from different computers to play together seamlessly without custom servers.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Current Target**: `KStarDredge`
- **Upcoming Queue**:
  `KAbyss`, `KColosseum`, `KCyber`, `KMech`, `KMystery`, `KVoid`, `KWizard`, `KStarship`, `KChrono`, `KStarForge`, `KFortress`, `KCosmic`, `KStellar`, `KSanctuary`, `KDragon`, `KSubmarine`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KCalc` (toast blocking calculation buttons)
- **Upcoming Queue**:
  `KHangman` (keyboard cutoff), `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAudio`, `KBBS`, `KBase`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColosseum`, `KContacts`, `KCosmic`, `KCyber`, `KDB`, `KDragon`, `KFlash`, `KFont`, `KFortress`, `KGraph`, `KHabit`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMech`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KSanctuary`, `KScript`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`, `KTask`, `KStarForge`, `KTerm`, `KHash`, `KRSS`, `KClip`, `KCipher`, `KPomodoro`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KCalc` (reposition bottom-center toast blocking buttons)
- **Upcoming Queue**:
  `KHex` (Tab 2 label, scrollbar, row cutoff), `KContacts` (reposition bottom-right toast blocking submit button), `KFarm` (reposition bottom toast blocking seed radio buttons), `KPaint`, `KAudio`, `KFont`, `KGraph`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPass`, `KPing`, `KRadio`, `KRead`, `KScript`, `KSynth`, `KSys`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KChrono`, `KTask`, `KStarForge`, `KPad`, `KBookmark`, `KHash`, `KRSS`, `KClip`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KWizard`
- **Upcoming Queue**:
  `KZip`, `KChrono`, `KCyber`, `KDragon`, `KFortress`, `KMech`, `KMystery`, `KQuest`, `KSanctuary`, `KStarDredge`, `KSubmarine`, `KHash`, `KRSS`, `KClip`, `KCipher`, `KTodo`, `KTrader`, `KType`, `KVault` *(Completed in Pass 5: K2048, KAudio, KBBS, KMaze, KSnake, KSolitaire, KSpace, KStarship, KStellar, KSynth, KSys, KTask, KTerm, KTimer, KTodo, KTrader, KType, KVault, KVoid)*.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KChess` (Implement Seamless Firebase RTDB Multiplayer per Mandate 12)
- **Upcoming Queue**:
  `KGo` (Multiplayer), `KReversi` (Multiplayer), `KDarts` (Multiplayer), `KTetris` (Arcade Duel Multiplayer), `KSnake` (Multiplayer), `KDB`, `KTodo`, `KJournal`, `KCalendar`, `KContacts`, `KMail`, `KRead`, `KPass`, `KPaint`, `KImage`, `KAudio`, `KSynth`, `KMedia`, `KChart`, `KGraph`, `KMandel`, `KType`, `KVault`, `KZip`, `KSys`, `KTask`, `KNet`, `KPing`, `KHash`, `KRSS`, `KFont`, `KPad`, `KNote` *(Completed: KConnect4)*.
- **Multiplayer Focus (CRITICAL)**: Concentrate on expanding games (*KChess*, *KConnect4*, *KGo*, *KReversi*, *KDarts*, *KTetris*, *KSnake*) and collaborative apps (*KDraw*, *KPaint*, *KSynth*, *KPad*) with seamless Firebase Realtime Database multiplayer for cross-computer play on `kiloapps.web.app`.

### 7. Virtual 1999 Web Expansion Queue (Eternal Fleet Track)
- **Current Active Target**: `kweb://users/~neon_rider` (`KiloOS/public/web/users/neon_rider.html`)
  - *Next in Rotation*: `kweb://asm-temple` ➔ `kweb://cybercafe` ➔ `kweb://darknet` ➔ `kweb://geocities` ➔ `kweb://warez` ➔ `kweb://portal` ➔ `kweb://webring`.
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

- **[FLEET: kilo-creator & kilo-expander] — Seamless Online Multiplayer via Firebase** | Director Directive
  - ⚠️ AGENT NOTE: Human director request. Priority architectural directive for creator and expander agents.
  - Instructions: Concentrate on adding multiplayer features that work seamlessly through Firebase Realtime Database with different people playing on kiloapps.web.app from different computers that are not otherwise communicating, similar to how KChat allows chat from the global room. Use the shared Firebase RTDB (`https://kiloappschat-default-rtdb.firebaseio.com`) with CDN imports and clean room namespacing (`multiplayer/<app>/...`).

- **[ALL_APPS / FLEET] — Visual Quality & Graphics** | Director Directive
  - ⚠️ AGENT NOTE: Human director request. Priority fleet-wide directive.
  - Instructions: Systematically remove rotating/traveling specular glint comets, perimeter glint dots, and moving border balls across both web (HTML) and native (Win32 C) on every app pass. They are annoying across every app and look like distracting projectiles/balls. Replace with clean, static, or period-accurate borders without traveling dots or orbital glint particles. NEVER add new perimeter traveling glints.

- **[FLEET: kilo-qa, kilo-usability, kilo-tester] — Toast Occlusion & Modal Clipping Remediation** | Vision Audit Directive
  - ⚠️ AGENT NOTE: Secondary state vision audit revealed 26 apps where persistent or timed toasts (`z-index: 150-200`) overlap interactive controls (buttons, inputs, close icons) and 10 apps with clipped dialogs/virtual keyboards.
  - Instructions: During app passes, ensure toasts do not occlude interactive inputs or primary buttons (position toasts safely, dismiss on click/interaction, or use unobtrusive non-overlapping toast bars). Fix double-modal stacking (`kclip` - fixed, `kpomodoro` - fixed) and remove internal loop labels (`kdarts`, `kwords`).

---

## Recent Execution Logs (Max 5 Entries)

- **2026-09-23T11:50:00Z — kilo-tester: KPomodoro (Modal Stacking Collision & Toast Occlusion Remediation)**
  - Status: PASS ✅ (3 issues identified and resolved; 0 regressions).
  - Modal Isolation: Unified modal management with strict mutual exclusivity across Splash, Tutorial, and Settings overlays.
  - Backdrop & Shortcuts: Fixed backdrop collision, trapped background shortcuts during active dialogs, and routed Enter/Esc to active modal.
  - Toast Ergonomics: Repositioned toast from bottom-center to top-right with instant click-to-dismiss and ARIA polite announcements.
  - Audio & UI Feedback: Added procedural Web Audio UI tones for dialog transitions and confirmed all 54 interactive elements reactive.
  - Verification: Headless CDP test pass (54 elements, 0 warnings, 0 errors, 60 FPS pacing); Vite build clean (452ms); Win32 C build clean (13.5 KB); security lint clean.

- **2026-09-23T10:45:00Z — kilo-creator: KNetMap (Subnet Topology Visualizer & Network Simulator)**
  - Status: PASS ✅ (New app created: native Win32 C + HTML5 web app registered in KiloOS).
  - Web App (knetmap.html, 105.9 KB): Interactive topology editor, Dijkstra routing, animated packet pulses (ICMP/TCP/UDP/ARP), VLSM/CIDR partition bar, chaos link cut test, sniffer table, Firebase RTDB online co-op room.
  - Native App (KNetMap.exe, 11.2 KB): Win32 GDI topology visualizer, double-buffered packet simulation, subnet calculator, Corp/ISP presets.
  - Standards: Title splash screen, first-run tutorial (`kknetmap_tutorialSeen`), F5 quicksave/F9 quickload, 1999 ARG parody brands (CYBER-CO, 3-CON, Bastion, Sol Unix).
  - Universal Audio: Pure Web Audio YM2612 FM synthesis & SPC700 stereo delay warmth.
  - Verification: Clean MSVC build (11.2 KB); Vite clean build (396ms); security lint 100% PASS; strict <999KB ceiling.

- **2026-09-23T09:55:00Z — kilo-expander: KConnect4 (Seamless Firebase RTDB Multiplayer Expansion)**
  - Status: PASS ✅ (Implemented real-time cross-computer multiplayer via Firebase RTDB per Mandate 12).
  - Online Multiplayer: Quick Match public matchmaking, host/join custom rooms, public lobby browser, spectator mode.
  - Game State Sync: Real-time turn alternation, move synchronization, powerups (Bomb/Drill/Magnet/Freeze), quick chat emotes.
  - UI Ergonomics: Added Online HUD banner, [O] shortcut, room invite URL auto-join (`?room=XYZ`), clean `onDisconnect()` presence.
  - Offline Fallback: 100% offline preservation for vs AI (4 personalities), Campaign (20 stages), 2P local, and Speed modes.
  - Verification: Headless CDP test pass (0 errors, 60 FPS pacing); single-file web 146.6 KB (<999KB); Vite build clean (367ms); security lint clean.

- **2026-09-23T08:45:00Z — kilo-qa: KVoid (Pass 5: Tutorial & State Integrity)**
  - Status: PASS ✅ (Full state persistence & tutorial isolation verified; 0 regressions).
  - Quicksave/Load: Implemented full-fidelity F5/F9 state persistence (web localStorage & native C `kvoid_save.dat`).
  - Tutorial Integrity: Added first-run isolation (`kvoid_tutorial.dat` / `kvoid_tutorialSeen`) with Esc/Enter/Space/Click dismiss.
  - UI Ergonomics: Added interactive toolbar panel ([F1] Guide, [F5] Save, [F9] Load, [R] Restart) and canvas click restart.
  - Verification: MSVC C clean build (`KVoid.exe` 25.0 KB); web (`kvoid.html` 73.2 KB); Vite clean build (359ms); quality gate 104/104 PASS (60 FPS); security lint clean.

- **2026-09-23T07:50:00Z — kilo-graphics: KSubmarine (Specular Glint Removal, HiDPI Polish & Balance Pass)**
  - Status: PASS ✅ (Eliminated observation dome specular glint; enhanced HiDPI text and bio-scan balance).
  - Glint Removal: Removed artificial viewport specular glint dot from submersible sprite rendering in `ksubmarine.html`.
  - HiDPI Polish: Scaled sonar range text vertical offset by DPR (`cy - 4 * dpr`) for crisp high-DPI rendering.
  - Balance Pass: Added telemetry verification research credit reward (+15 PTS) and acoustic chirp on re-scanning discovered fauna (web & native C).
  - Verification: Native MSVC C clean build (`KSubmarine.exe` 235 KB); single-file web (`ksubmarine.html` 402 KB); Vite clean build (381ms); security lint 100% clean.

