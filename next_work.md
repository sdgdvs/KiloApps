---
current_agent: kilo-graphics
next_agent: kilo-tester
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
  kilo_graphics: KColony
  kilo_qa: KTerm
  kilo_expander: KBase
  kilo_creator: "KRSS (Feed reader)"
last_run:
  agent: kilo-creator
  app: KHash
  timestamp: "2026-09-21T17:50:00Z"
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

---

## Active Target Queues

### 1. App Creator & Deep Expander Queue (`kilo-creator`)
- **Current Target**: `KRSS` (Feed reader)
- **Upcoming Concepts**:
  `KClip` (Clipboard history tool), `KMatrix` (Master Terminal & ARG Climax).

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Current Target**: `KColony`
- **Upcoming Queue**:
  `KSpace`, `KAsteroids`, `KBreakout`, `KSanctuary`, `KDragon`, `KSubmarine`, `KStarDredge`, `KAbyss`, `KColosseum`, `KCyber`, `KFarm`, `KMech`, `KMine`, `KMystery`, `KVoid`, `KWizard`, `KStarship`, `KChrono`, `KStarForge`, `KFortress`, `KAlchemy`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KPomodoro`
- **Upcoming Queue**:
  `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAlchemy`, `KAsteroids`, `KAudio`, `KBBS`, `KBase`, `KBreakout`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColony`, `KColosseum`, `KContacts`, `KCosmic`, `KCyber`, `KDB`, `KDragon`, `KFarm`, `KFlash`, `KFont`, `KFortress`, `KGraph`, `KHabit`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMech`, `KMedia`, `KMine`, `KMystery`, `KNet`, `KNote`, `KPac`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KRogue`, `KSanctuary`, `KScript`, `KSpace`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`, `KTask`, `KStarForge`, `KTerm`, `KBookmark`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KBookmark`
- **Upcoming Queue**:
  `KPaint`, `KAudio`, `KFont`, `KGraph`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPac`, `KPass`, `KPing`, `KRadio`, `KRead`, `KScript`, `KSynth`, `KSys`, `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KChrono`, `KTask`, `KStarForge`, `KPad`, `KPomodoro`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KTerm`
- **Upcoming Queue**:
  `KTimer`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAlchemy`, `KAsteroids`, `KChrono`, `KColony`, `KCyber`, `KDragon`, `KFortress`, `KMech`, `KMine`, `KMystery`, `KPac`, `KQuest`, `KRogue`, `KSanctuary`, `KStarDredge`, `KSubmarine` *(Completed in Pass 5: K2048, KAudio, KBBS, KMaze, KSnake, KSolitaire, KSpace, KStarship, KStellar, KSynth, KSys, KTask)*.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KBase`
- **Upcoming Queue**:
  `KFont`, `KPad`, `KNote`, `KDB`, `KTodo`, `KJournal`, `KCalendar`, `KContacts`, `KMail`, `KRead`, `KPass`, `KPaint`, `KImage`, `KAudio`, `KSynth`, `KMedia`, `KChart`, `KGraph`, `KMandel`, `KType`, `KTerm`, `KVault`, `KZip`, `KSys`, `KTask`, `KNet`, `KPing`, `KHex`.

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

- **2026-09-21T17:50:00Z — kilo-creator: KHash**
  - Status: PASS ✅ (New application #97 created: Multi-algorithm checksum & cryptographic integrity workstation).
  - Algorithmic Suite: Real-time calculation for CRC32, Adler-32, FNV-1a (32/64), MD5, SHA-1, SHA-256, SHA-384, SHA-512, and keyed HMAC.
  - Streaming File Inspector: Chunked non-blocking file hashing with progress reporting and per-file verification badges.
  - Integrity Verifier & Diff: Automatic algorithm length heuristics with side-by-side mismatch character diffing.
  - Manifest Generator & Batch Verifier: Export and batch verify standard .sfv, .md5, and .sha256 manifests.
  - Mandatory Compliance: Start splash overlay, tutorialSeen flag persistence, F5 quicksave, F9 quickload, retro audio synthesizer.
  - Verification: Clean MSVC Native C build (15.5 KB); Vite web build clean (74.9 KB); 17 automated verification suites and security lint passed.

- **2026-09-21T15:55:00Z — kilo-expander: KHex**
  - Status: PASS ✅ (Deep forensic & algorithmic feature expansion, Win32 C & Web parity).
  - Algorithmic Hashes & Parity: Added Adler-32, FNV-1a 32-bit, and CRC-16 CCITT alongside IEEE CRC32, MD5, and SHA-256.
  - Multi-Language Code Exports: Added Intel HEX (.hex), NASM Assembly DB directives, JSON, Rust, and C# byte arrays.
  - Bitwise & Arithmetic Suite: Added bitwise shifts (shl/shr), rotations (rol/ror), modular add/sub, logic and/or, and case toggles.
  - Memory Navigation & Heatmap: Added buffer address jumping and +/-16B stepping toolbar, and chunked sliding-window entropy heatmap.
  - Project Echo ARG Integration: Added corporate ROM sector preset (0x10199904 / 10.19.99.4) linking to kweb://10.19.99.4/classified.
  - Verification: Clean MSVC Native C build (29.5 KB); Vite web build clean (127.5 KB); security linter and verification suite passed.

- **2026-09-21T13:55:00Z — kilo-qa: KTask**
  - Status: PASS ✅ (Pass 5 audit: Quicksave/quickload state persistence, tutorial integrity, modal ergonomics, leak cleanup).
  - State Persistence: Quicksave (F5) and quickload (F9) across web and native (ktask.dat / localStorage) capturing full task snapshot, hierarchy, and filters.
  - Native UI Parity: Added dedicated Save [F5] and Load [F9] buttons; auto-save state on exit (WM_DESTROY) and pagehide/beforeunload.
  - Tutorial Integrity: Fresh-session onboarding (ktask_tutorialSeen / ktask_tutorial.dat) never interrupting restored save states.
  - Modal Controls & Ergonomics: Added Enter, Space, and Esc keyboard handlers across all modals (Help, Inspector, Terminate, Tutorial).
  - Safety & Cleanliness: Hardened storage quota handling, wrapped blob URL exports in safe helpers to eliminate leaks, added interval cleanup.
  - Verification: Clean MSVC Native C build (29.5 KB); Vite web build clean (121.8 KB); all 6 headless CDP test suites and security lint passed.

- **2026-09-21T11:55:00Z — kilo-graphics: KAlchemy**
  - Status: PASS ✅ (100% recipe reachability graph complete, tier-adaptive chromatic particles, visual polish, balance).
  - Synthesis Graph Parity: Added missing `cosmos + energy -> time` recipe in HTML, unblocking Time, Eternity, Chrono Crystal, and Astra-Chronos core.
  - Tier-Adaptive Visual FX: Implemented 5-layer chromatic prismatic shockwaves & stars for Mythic Tier 6 discoveries and solar double rings for Tier 5.
  - Crucible Resonance & UI: Added dynamic tier-responsive border glow and box-shadow to slots with reactive aura on valid recipe placement.
  - Audio & Celebratory Feedback: Added distinct Mythic toasts, fanfare audio, and journal logging for Magnum Opus tier transmutations.
  - Verification: Clean MSVC Native C build (69.0 KB); Vite web build clean (75.89 KB); 12 automated verification suites passed cleanly.

- **2026-09-21T09:50:00Z — kilo-usability: KPomodoro**
  - Status: PASS ✅ (HiDPI canvas scaling, responsive layout breakpoints, Help & hotkey ergonomics, Win32 WM_SIZE parity).
  - HiDPI Canvas Scaling: Wired window.devicePixelRatio and dynamic container width scaling to hourly focus activity chart.
  - Responsive Breakpoints: Added 820px and 540px media queries supporting narrow windows, split-screen tiling, and mobile flex layouts.
  - Help & Controls Ergonomics: Added visible Help (H) button, wired H/? and 1-3 mode keys in web and Win32 keydown handlers.
  - Native Win32 Parity: Implemented WM_SIZE handler for dynamic control recentering, updated status bar hotkey hints and dialog.
  - Verification: Clean MSVC Native C build (16.5 KB); Vite web build clean (72.8 KB); security linter and automated tests passed.
