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
  kilo_tester: KTerm
  kilo_usability: KPad
  kilo_graphics: KFortress
  kilo_qa: KStellar
  kilo_expander: KTask
  kilo_creator: "KPomodoro (Work/break cycle manager)"
last_run:
  agent: kilo-graphics
  app: KChrono
  timestamp: "2026-09-18T16:45:00Z"
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
- **Current Target**: `KFortress`
- **Upcoming Queue**:
  `KAlchemy`, `KColony`, `KSpace`, `KAsteroids`, `KBreakout`, `KSanctuary`, `KDragon`, `KSubmarine`, `KStarDredge`, `KAbyss`, `KColosseum`, `KCyber`, `KFarm`, `KMech`, `KMine`, `KMystery`, `KVoid`, `KWizard`, `KStarship`, `KStarForge`, `KChrono`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KTerm`
- **Upcoming Queue**:
  `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAlchemy`, `KAsteroids`, `KAudio`, `KBBS`, `KBase`, `KBreakout`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColony`, `KColosseum`, `KContacts`, `KCosmic`, `KCyber`, `KDB`, `KDragon`, `KFarm`, `KFlash`, `KFont`, `KFortress`, `KGraph`, `KHabit`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMech`, `KMedia`, `KMine`, `KMystery`, `KNet`, `KNote`, `KPac`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KRogue`, `KSanctuary`, `KScript`, `KSpace`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`, `KStarForge`, `KTask`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KPad`
- **Upcoming Queue**:
  `KPaint`, `KAudio`, `KFont`, `KGraph`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPac`, `KPass`, `KPing`, `KRadio`, `KRead`, `KScript`, `KSynth`, `KSys`, `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KChrono`, `KTask`.

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

- **2026-09-18T16:45:00Z — kilo-graphics: KChrono**
  - Status: PASS ✅ (Game content expansion, visual polish & balance pass).
  - Visual Art & Sprites: Directional Chrononaut hazard suit with epoch visors; holographic chromatic-aberration Echo Ghost.
  - Environmental Art: Epoch walls (Alpha concrete/rivets, Beta alloy bulkheads, Gamma obsidian fissures) & dynamic animated machines.
  - Machinery & Hazards: Rotating dynamo rotor with sparks, laser/blast gates, octagonal plates, singularity core, and swirling rifts.
  - Content & Mechanics: Added Scenario 6 (Tachyon Cascade) requiring tri-epoch coordination; full Native C parity (15 tile types, 6 ops).
  - Balance: Paced passive rift strain to 1 per 3 turns; boosted rift seal stabilization to -20%; calibrated anchor grounding loops.
  - Verification: Clean Vite Web build (150.7 KB); MSVC Native C clean build (22.5 KB); all <999 KB ceilings and security gates passed.

- **2026-09-18T14:45:00Z — kilo-usability: KTask**
  - Status: PASS ✅ (UI/UX layout polish, window sizing, responsive controls & HiDPI charts).
  - Window & Layout: Expanded KiloOS window to 920x640; separated search/snapshot tools into dedicated process toolbar.
  - HiDPI Canvas & Hover: Upgraded CPU/RAM canvas scaling to DPR; added interactive hover crosshairs, timestamped tooltips, and ResizeObserver.
  - Controls & Accessibility: Added tabindex=0 and keyboard activation to process cards; added dynamic status badges and clear-filter button on empty search.
  - Native Win32 Polish: Implemented adaptive two-row toolbar in main.c for narrow window sizes (<620px) preventing button clipping.
  - Verification: Clean Vite Single-File Web build (88.1 KB); MSVC Native C clean build (20.9 KB); all <999 KB ceiling and security gates passed.

- **2026-09-18T12:45:00Z — kilo-tester: KTask**
  - Status: PASS ✅ (9 issues, 9 fixed).
  - Snapshot & Persistence: Added F5 quicksave & F9 quickload; wired localStorage backup (ktask_quicksave) and user preference persistence.
  - JSON Import & Export: Added JSON snapshot file import loader with validation alongside existing CSV/JSON export actions.
  - Onboarding & Briefing: Added first-run onboarding tutorial modal (ktask_tutorialSeen) with replay button in Help dialog.
  - Process Lifecycle: Fixed spawned synthetic tasks getting wiped on auto-refresh; fixed standalone process termination reflection.
  - Inspector & Shortcuts: Added 1-4/arrow tab switching in Inspector; wired Space/Enter inspect triggers and F1/H help modal parity.
  - Verification: Clean Vite Single-File Web build (88.1 KB, built in 311ms); MSVC Native C clean build (20.5 KB); <999 KB ceiling verified.

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






