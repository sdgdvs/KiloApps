---
current_agent: kilo-graphics
next_agent: kilo-qa
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
  kilo_graphics: KStarship
  kilo_qa: KStarship
  kilo_expander: KSys
  kilo_creator: "KStarForge (Deep-space shipyard engineering sim)"
last_run:
  agent: kilo-usability
  app: KChrono
  timestamp: "2026-09-18T00:41:00Z"
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
   - Agents MUST read the `⚠️ AGENT NOTE` annotation on each directive. If an agent judges that a directive is counterproductive to the project's core purpose (the 999KB retro OS, ludonarrative consonance, the ARG integrity, or fleet stability), the agent MUST skip it, log the reason for rejection, and leave the directive in place for the human director to review.
   - Directors can freely propose new apps, request features, add content to existing apps, add websites to the Virtual 1999 Web, and steer creative direction. Agents should implement these in good faith unless they conflict with the project's foundational pillars.
   - Once an agent completes or rejects a directive, it removes the entry from this section and logs the outcome in the execution log.

---

## Active Target Queues

### 1. App Creator & Deep Expander Queue (`kilo-creator`)
- **Current Target**: `KStarForge` (Deep-space shipyard engineering sim)
- **Upcoming Concepts**:
  `KPomodoro` (Work/break cycle manager), `KBookmark` (Categorized link vault), `KHash` (Multi-algorithm checksum tool), `KRSS` (Feed reader), `KClip` (Clipboard history tool).

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Current Target**: `KStarship`
- **Upcoming Queue**:
  `KChrono`, `KFortress`, `KAlchemy`, `KColony`, `KSpace`, `KAsteroids`, `KBreakout`, `KSanctuary`, `KDragon`, `KSubmarine`, `KStarDredge`, `KAbyss`, `KColosseum`, `KCyber`, `KFarm`, `KMech`, `KMine`, `KMystery`, `KVoid`, `KWizard`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KTask`
- **Upcoming Queue**:
  `KTerm`, `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAlchemy`, `KAsteroids`, `KAudio`, `KBBS`, `KBase`, `KBreakout`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColony`, `KColosseum`, `KContacts`, `KCosmic`, `KCyber`, `KDB`, `KDragon`, `KFarm`, `KFlash`, `KFont`, `KFortress`, `KGraph`, `KHabit`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMech`, `KMedia`, `KMine`, `KMystery`, `KNet`, `KNote`, `KPac`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KRogue`, `KSanctuary`, `KScript`, `KSpace`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KTask`
- **Upcoming Queue**:
  `KPad`, `KPaint`, `KAudio`, `KFont`, `KGraph`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPac`, `KPass`, `KPing`, `KRadio`, `KRead`, `KScript`, `KSynth`, `KSys`, `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KChrono`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KStarship`
- **Upcoming Queue**:
  `KStellar`, `KSynth`, `KSys`, `KTask`, `KTerm`, `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAlchemy`, `KAsteroids`, `KChrono`, `KColony`, `KCyber`, `KDragon`, `KFortress`, `KMech`, `KMine`, `KMystery`, `KPac`, `KQuest`, `KRogue`, `KSanctuary`, `KStarDredge`, `KSubmarine`, `KVoid`, `KWizard` *(Completed in Pass 5: K2048, KAudio, KBBS, KMaze, KSnake, KSolitaire, KSpace)*.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KSys`
- **Upcoming Queue**:
  `KTask`, `KNet`, `KPing`, `KHex`, `KBase`, `KFont`, `KPad`, `KNote`, `KDB`, `KTodo`, `KJournal`, `KCalendar`, `KContacts`, `KMail`, `KRead`, `KPass`, `KPaint`, `KImage`, `KAudio`, `KSynth`, `KMedia`, `KChart`, `KGraph`, `KMandel`, `KType`, `KTerm`, `KVault`, `KZip`.

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

- **2026-09-18T00:41:00Z — kilo-usability: KChrono**
  - Status: PASS ✅ (UI/UX, responsive layout, HiDPI scaling, and input ergonomics pass).
  - Window & Layout: Expanded KiloOS window (1140x740) in App.jsx; eliminated viewport/sidebar crowding.
  - HiDPI & Crispness: Added devicePixelRatio scaling for simCanvas and chronographCanvas; added ClearType fonts in native C.
  - Responsive Controls: Normalized canvas click/touch coordinates; added live mousemove hover tile inspector.
  - Ergonomics & Accessibility: Added interactive footer buttons, header Wait/Act button, and scenario double-click launch.
  - Onboarding & Parity: Added [F1/H] help hotkey and overlay Enter/Space dismiss; added full native C mouse support and AdjustWindowRect.
  - Verification: Single-file Web build clean (128.7 KB, Vite built in 285ms); Native MSVC clean build (15.5 KB); <999 KB ceiling verified.

- **2026-09-17T22:42:00Z — kilo-tester: KChrono**
  - Status: PASS ✅ (8 issues, 8 fixed).
  - Interactive Wiring: Added backdrop click dismissal on all overlays; added F1/? Help toggle, Enter action trigger, and Esc menu toggle.
  - Chrono-Deck & Locker: Wired Chrono-Locker storage cache, cross-epoch item aging, deck slot rendering, and artifact strain discharge (hotkeys 4-7).
  - Causality & Scenarios: Latched blast gate in Scenario 2 upon dual biometric auth; added passive paradox strain in Scenario 3 while rifts active.
  - Telemetry & Inspector: Added direct epoch switching on Chronograph node click; expanded Causal Inspector for all tiles, crates, and phantoms.
  - Settings Persistence: Added persistent localStorage storage for master volume, SFX, tachyon drone, CRT scanlines, and color theme.
  - Verification: Single-file Web build (121.5 KB, Vite built in 333ms); Native MSVC clean build (15 KB); zero Vite build breaks; <999 KB ceiling.

- **2026-09-17T20:40:00Z — kilo-planner: Fleet Planning & Queue Compaction**
  - Status: PASS ✅ (24h velocity evaluated, mature apps pruned, queues sanitized).
  - Queue Rework: Deprioritized 22 mature apps per registry; prioritized newly created KChrono for UI testing and usability.
  - Rotation Schedule: Set 24h rotation to tester ➔ usability ➔ graphics ➔ qa ➔ expander ➔ creator.
  - Compaction: Moved older execution logs (KQuest) to archive/fleet_execution_archive.md; enforced 5-entry cap.
  - Verification: Clean orchestrator dry-run, security lint verified, targets validated.

- **2026-09-17T18:45:00Z — kilo-expander: KTerm**
  - Status: PASS ✅ (Deep functional feature expansion & ARG integration).
  - Pipelines & Redirection: Added output redirection (> and >>) and command chaining (; and &&) across web and native.
  - Text & Math Utilities: Added grep/findstr, wc, head/tail (-n N), calc (recursive math parser), touch, del/rm, copy/cp, move/ren.
  - History & Navigation: Added history list with !n and !! recall, tab completion for all 44 commands and files.
  - System Diagnostics & Lore: Added ps/tasks, uptime, ping, netstat, dmesg/syslog, glitch memory dump, and Ctrl+Alt+E echo intercept.
  - CRT Color Themes: Added 6 dynamic themes (Green, Amber, Cyan, White, Crimson, Purple) with toolbar toggle and theme/color command.
  - Verification: MSVC Native C (40.5 KB) and Single-File Web (83 KB) clean builds; strictly <999 KB ceiling verified.

- **2026-09-17T16:45:00Z — kilo-qa: KSpace**
  - Status: PASS ✅ (Pass 5 audit: State persistence, tutorial integrity, overlays).
  - Highlights: Complete mission quicksave (F5) and quickload (F9) across web and native with quota fallback.
  - Tutorial Integrity: Fresh-session tutorial onboarding (`kspace_tutorialSeen` / `kspace_tutorial.dat`) protecting restored saves.
  - Overlay & Controls: Game over and victory screens now feature working `[F9] Reload Quicksave` actions and shortcuts.
  - Bug Fixes: Removed save deletion on death; fixed help overlay click-through returning to menu instead of resuming game.
  - Verification: Clean single-file Web build (139 KB, Vite built in 319ms); Native MSVC clean build (72.7 KB); strictly <999 KB ceiling.


