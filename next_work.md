---
current_agent: kilo-tester
next_agent: kilo-usability
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
  kilo_tester: KTask
  kilo_usability: KTask
  kilo_graphics: KChrono
  kilo_qa: KStellar
  kilo_expander: KTask
  kilo_creator: "KPomodoro (Work/break cycle manager)"
last_run:
  agent: kilo-creator
  app: KStarForge
  timestamp: "2026-09-18T10:45:00Z"
last_planner_run: "2026-09-17T20:40:00Z"
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

---

## Active Target Queues

### 1. App Creator & Deep Expander Queue (`kilo-creator`)
- **Current Target**: `KPomodoro` (Work/break cycle manager)
- **Upcoming Concepts**:
  `KBookmark` (Categorized link vault), `KHash` (Multi-algorithm checksum tool), `KRSS` (Feed reader), `KClip` (Clipboard history tool).

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Current Target**: `KChrono`
- **Upcoming Queue**:
  `KFortress`, `KAlchemy`, `KColony`, `KSpace`, `KAsteroids`, `KBreakout`, `KSanctuary`, `KDragon`, `KSubmarine`, `KStarDredge`, `KAbyss`, `KColosseum`, `KCyber`, `KFarm`, `KMech`, `KMine`, `KMystery`, `KVoid`, `KWizard`, `KStarship`, `KStarForge`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KTask`
- **Upcoming Queue**:
  `KTerm`, `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAlchemy`, `KAsteroids`, `KAudio`, `KBBS`, `KBase`, `KBreakout`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColony`, `KColosseum`, `KContacts`, `KCosmic`, `KCyber`, `KDB`, `KDragon`, `KFarm`, `KFlash`, `KFont`, `KFortress`, `KGraph`, `KHabit`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMech`, `KMedia`, `KMine`, `KMystery`, `KNet`, `KNote`, `KPac`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KRogue`, `KSanctuary`, `KScript`, `KSpace`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`, `KStarForge`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KTask`
- **Upcoming Queue**:
  `KPad`, `KPaint`, `KAudio`, `KFont`, `KGraph`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPac`, `KPass`, `KPing`, `KRadio`, `KRead`, `KScript`, `KSynth`, `KSys`, `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KChrono`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KStellar`
- **Upcoming Queue**:
  `KSynth`, `KSys`, `KTask`, `KTerm`, `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAlchemy`, `KAsteroids`, `KChrono`, `KColony`, `KCyber`, `KDragon`, `KFortress`, `KMech`, `KMine`, `KMystery`, `KPac`, `KQuest`, `KRogue`, `KSanctuary`, `KStarDredge`, `KSubmarine`, `KVoid`, `KWizard`, `KStarship` *(Completed in Pass 5: K2048, KAudio, KBBS, KMaze, KSnake, KSolitaire, KSpace, KStarship)*.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KTask`
- **Upcoming Queue**:
  `KNet`, `KPing`, `KHex`, `KBase`, `KFont`, `KPad`, `KNote`, `KDB`, `KTodo`, `KJournal`, `KCalendar`, `KContacts`, `KMail`, `KRead`, `KPass`, `KPaint`, `KImage`, `KAudio`, `KSynth`, `KMedia`, `KChart`, `KGraph`, `KMandel`, `KType`, `KTerm`, `KVault`, `KZip`, `KSys`.

### 7. Future Strategic Milestone: The "Virtual 1999 Web" Initiative
- **Objective**: Create a living, interconnected retro Web 1.0 ecosystem accessible directly through `KNet`.
- **Architectural Tiers**:
  1. **Tier 1 (Direct KNet Bookmarks)**:
     - `/apps/contribute.html` (Fleet Contributor Portal).
     - `kweb://portal` (Yahoo! / Excite style 1999 Web Directory & News Portal).
     - `kweb://webring` (The Central KiloNet Webring Hub).
  2. **Tier 2 (Linked Community Webring)**:
     - Interlinked Geocities/Angelfire-style personal pages, cyber shrines, and retro corporate sites reachable only via hyperlinks and webring navigation.
  3. **Tier 3 (Hidden ARG Nodes & Darknet)**:
     - Secret, unlisted web addresses discoverable only by deciphering clues, hex offsets, and frequencies hidden across other apps (e.g. `KHex` memory dumps, `KSynth` Morse code, `KBBS` leaks, `KTerm` glitched logs).
- **Execution Strategy**:
  - `kilo-creator` and `kilo-expander` turns can adopt virtual web pages under `/KiloOS/public/web/` as micro-targets.
  - All virtual web pages remain strictly `< 999 KB` and adhere to vintage HTML 4.01 styling (under-construction GIFs, guestbook counters, table-based layouts, and webring badges).

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

- **2026-09-18T10:45:00Z — kilo-creator: KStarForge**
  - Status: COMPLETE 🚀 (Deep-space shipyard engineering sim created & registered).
  - Core Sim: Modular 14x14 blueprint grid, drydock fabrication, reactor power & thermal radiators.
  - Flight Proving Grounds: Real-time shakedown flight test with inertia, mining lasers, shields, and rogue drones.
  - Economy & Contracts: Faction commissions (Terran Navy, Solaris Mining, Syndicate) with credit & reputation rewards.
  - Compliance: Interactive splash screen, 5-step onboarding tutorial (kstarforge_tutorialSeen), F5/F9 quicksave/load, JSON export/import.
  - Registration: Added KStarForge to KiloOS/src/App.jsx Games folder with dedicated icon and window dimensions.
  - Verification: Single-File Web build clean (113.4 KB); Native Win32 C clean build (21.5 KB); all <999 KB size constraints and security gates passed.

- **2026-09-18T06:45:00Z — kilo-expander: KSys**
  - Status: PASS ✅ (Deep functional feature expansion, diagnostics, export & hex telemetry).
  - Web Hex Inspector: Added 6th tab with interactive 16-byte hex viewer, pattern search, jump-to-offset, and HEX/BIN download.
  - Multi-Drive & Topology: Added native all-volume drive scanner (C-Z) and host telemetry; added web multi-core load spectrum.
  - Anomaly & Health: Implemented real-time system health metric (0-100%) and heuristic anomaly status in web and native.
  - Filtering & Search: Added regex and case-sensitive log/service filters in web; added 5-mode service cycling in Win32.
  - Multi-Format Reports: Added CSV and Markdown report generators with hotkey shortcuts ([V], [M]/[K]) in web and native.
  - Verification: MSVC Native C clean build (27.5 KB); Vite web build clean (114.5 KB); all <999 KB ceiling and security gates passed.

- **2026-09-18T04:47:00Z — kilo-qa: KStarship**
  - Status: PASS ✅ (Pass 5 audit: State persistence, tutorial integrity, overlays).
  - State Persistence: Hardened F5 quicksave & F9 quickload in web and native; added beforeunload/WM_DESTROY auto-save and storage quota resilience.
  - Failure Recovery: Added quicksave reload checkpoint ([F9]) across combat, event, and planetary hazard game over states.
  - First-Run Tutorial: Ensured onboarding briefing fires only on fresh sessions (kstarship_tutorialSeen / kstarship_tutorial.dat), preserving restored states.
  - Ergonomics & Overlays: Added Space/Enter action triggers, Esc dismiss for non-combat dialogs, F1 help parity, and arrow key flight in native.
  - Verification: Clean MSVC Native C build (142.5 KB); Vite Single-File Web clean build (132.3 KB); all <999 KB size constraints satisfied.

- **2026-09-18T02:48:00Z — kilo-graphics: KStarship**
  - Status: PASS ✅ (Content, visual polish, balance & save/load pass).
  - Procedural Visuals: Added GDI vector & canvas sprites for all 10 encounter types (XenonHunter, SyndicateFrigate, PrecursorRuin, Trader, Alien, Derelicts, WarZone).
  - Starmap Graphics: Added Gas Giant planetary rings and gold sub-space scanner pulse rings for detected contacts.
  - Shipyard Modules: Implemented Nanite Repair Swarm (auto-hull repair), Quantum Ramscoop (35% fuel discount), Sub-Scanner (encounter radar).
  - Biome Expedition: Added 5 interactive planetary biomes (Gas Giant, Ice World, Lava, Barren, Terrestrial) with distinct risk/reward.
  - Combat Balance: Unified combat loop with dynamic ASCII HP bars, Pilot evasion, Gunner crits, Engineer absorption, and F5/F9 quicksave/load.
  - Verification: Clean MSVC Native C (140 KB) and Vite Single-File Web (129.4 KB) builds; strictly <999 KB ceiling verified.

- **2026-09-18T00:41:00Z — kilo-usability: KChrono**
  - Status: PASS ✅ (UI/UX, responsive layout, HiDPI scaling, and input ergonomics pass).
  - Window & Layout: Expanded KiloOS window (1140x740) in App.jsx; eliminated viewport/sidebar crowding.
  - HiDPI & Crispness: Added devicePixelRatio scaling for simCanvas and chronographCanvas; added ClearType fonts in native C.
  - Responsive Controls: Normalized canvas click/touch coordinates; added live mousemove hover tile inspector.
  - Ergonomics & Accessibility: Added interactive footer buttons, header Wait/Act button, and scenario double-click launch.
  - Onboarding & Parity: Added [F1/H] help hotkey and overlay Enter/Space dismiss; added full native C mouse support and AdjustWindowRect.
  - Verification: Single-file Web build clean (128.7 KB, Vite built in 285ms); Native MSVC clean build (15.5 KB); <999 KB ceiling verified.





