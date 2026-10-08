---
current_agent: kilo-usability
next_agent: kilo-qa
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
  kilo_creator: "kweb://10.19.99.4/classified"
  kilo_graphics: KColony
  kilo_tester: KQuarantine
  kilo_usability: KSolitaire
  kilo_qa: KHangman
  kilo_expander: KDarts
virtual_web_target: "kweb://10.19.99.4/classified"
virtual_web_rotation:
  - "kweb://geocities"
  - "kweb://users/~neon_rider"
  - "kweb://asm-temple"
  - "kweb://cybercafe"
  - "kweb://10.19.99.4/classified"
  - "kweb://echo-subsystem.net"
  - "kweb://deep-core"
  - "kweb://darknet"
  - "kweb://portal"
  - "kweb://webring"
  - "kweb://warez"
last_run:
  agent: kilo-tester
  app: KConverter
  timestamp: "2026-10-08T01:45:00-07:00"
last_planner_run: "2026-10-07T14:38:00Z"
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
   - Entries in the `## Director Directives` section below are submitted by human directors via the KDirector console (`KiloOS/public/apps/kdirector.html`). Because web browsers operate in an isolated client-side sandbox, directives staged on `kiloapps.web.app` are routed via the Fork Dispatch Bridge: directors paste the formatted markdown block into `next_work.md` on their fork (or submit via GitHub issue), then run `Contributor Fleet Turn` in GitHub Actions to have autonomous AI worker agents execute them.
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
12. **Seamless Online Multiplayer via Firebase & Autostart Prohibition (DIRECTOR MANDATE - CRITICAL)**:
    - `kilo-creator` and `kilo-expander` must concentrate on adding online multiplayer features that work seamlessly through Firebase Realtime Database (`https://kiloappschat-default-rtdb.firebaseio.com`) using the standardized RFMS service [`KiloOS/public/assets/js/retro_multiplayer.js`](KiloOS/public/assets/js/retro_multiplayer.js) (spec: [`docs/RFMS_SPEC.md`](docs/RFMS_SPEC.md)).
    - **No Autostart in Multiplayer (CRITICAL - MANDATORY CONNECT GATE)**: Apps must NEVER autostart into multiplayer, initiate matchmaking, or connect to the online lobby on application boot/load. People do not like being thrown into multiplayer without hitting a "Connect" / "Play Online" button or getting a warning first. Every game must default to local offline play (e.g. against local Easy AI or solo mode) or boot to a start screen with explicit mode choices. Online connection must be strictly gated behind an explicit user action (menu item, connect screen, or "Connect" button).
    - Players from different computers anywhere on the internet visiting `kiloapps.web.app` who are not otherwise communicating must be able to play together in real-time without needing custom servers, shared LANs, or external communication tools—identical to how KChat connects global users in the `#general` room.
    - **Mandatory 25s Solo AI Fallback**: If waiting for a peer and none joins within 25 seconds, app automatically engages local AI cyber-bot.
    - Priority focus: turn-based board & strategy games (*KChess*, *KConnect4*, *KGo*, *KReversi*, *KDarts*, *KBattleship*), competitive arcade duel modes (*KTetris*, *K2048*, *KSnake*), and collaborative apps (*KDraw*, *KPaint*, *KSynth*, *KPad*). Always preserve offline/solo/vs-AI mode as a graceful fallback.
13. **🎨 Daily App Icon Uniqueness Audit (DIRECTOR MANDATE - CRITICAL)**:
    - Every application in `KiloOS/src/App.jsx` MUST possess a unique, visually distinctive 32x32 `.ico` file in `KiloOS/public/assets/icons/`. Reusing icons or copying existing `.ico` files (e.g. copying `kpass.ico` or pointing multiple apps to `knet.ico`) is strictly prohibited.
    - The autonomous fleet enforces this daily via `scripts/check_icons.py` during `kilo-planner` runs and on every `kilo-graphics` pass. If missing or duplicate icon hashes are detected, resolve immediately via `python scripts/check_icons.py --fix`.
14. **ARG Guidelines & Mystery Preservation Protocol (DIRECTOR MANDATE - CRITICAL)**:
    - Clues to the ARG must be subtle, atmospheric, and diegetic (in-universe). Never use cudgel-like explanations or walkthroughs.
    - NEVER label content with `(ARG)`, `ARG Secrets`, `ARG Lore`, `ARG Guidance`, or `ARG Clue`.
    - Never explain the autonomous fleet meta-twist before the endgame, and never leak the master passkey `ECHO-1999-ARCHITECT` in plain text. The climax is reserved solely for App #100 (`KMatrix`) and `KDirector`.
15. **🖼️ Exclusive Imagen 3 Asset Replacement Mandate for `kilo-graphics` (DIRECTOR MANDATE - CRITICAL)**:
    - For upcoming turns, `kilo-graphics` does NOTHING BUT replace programmer art (geometric cutouts, primitive vector fills, procedural line art) with Imagen 3 generated game assets (sprites, sprite sheets, seamless textures, and backgrounds) using the standardized 2-stage asset pipeline (`generate_image` + `scripts/asset_pipeline.py`).
    - **Turn Skipping for Inappropriate Targets**: If `kilo-graphics` reaches an application where replacing vector art with Imagen 3 generated assets is NOT appropriate (e.g. pure vector/wireframe arcade classics like *KAsteroids*, classic board games like *KChess*/*KGo*, text/utility games, or apps that already have complete production art), the agent MUST skip its turn (`⏭️ Skip — Imagen 3 asset replacement not appropriate for [app]`), rotate the target to the queue bottom, advance the rotation, and terminate cleanly without touching code.

---

## Active Target Queues

### 1. Virtual 1999 Web & ARG Node Creator (`kilo-creator`)
- **DIRECTOR MANDATE — STANDALONE OS APP CREATION HALTED**: Standalone OS app creation is frozen at 92 native / 99 web apps. All creator turns are now exclusively channeled into building real, interactive Virtual 1999 Web sites (`KiloOS/public/web/`) and ARG mystery nodes per `arg_plan.md`. Zero shallow stubs; every page must be a functioning Web 1.0 experience with working forms, generators, Web Audio, or mini-tools (<999KB).
- **Current Target**: `kweb://10.19.99.4/classified`
- **Upcoming Queue**:
  `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://darknet`, `kweb://portal`, `kweb://webring`, `kweb://warez`, `kweb://geocities`, `kweb://users/~neon_rider`, `kweb://asm-temple`, `kweb://cybercafe`
  *(Completed Phase 1 & 2 Builds: kweb://geocities, kweb://cybercafe deep expansion complete. Active: Surface-site ARG breadcrumbs & middle-game puzzle gating)*.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **EXCLUSIVE MISSION**: Replace programmer vector art with Imagen 3 generated sprites and backgrounds via the 2-stage asset pipeline. If the target app is not appropriate for raster/sprite replacement (e.g. wireframe classics or abstract board games), skip the turn immediately.
- **Current Target**: `KColony`
- **Upcoming Queue**:
  `KDragon`, `KMech`, `KColosseum`, `KAbyss`, `KBreakout`, `KAsteroids`, `KSpace`, `KFarm`, `KWizard` *(Note: Pure vector/wireframe or abstract board targets skip automatically per Mandate 15. Completed: KQuest Phases 1-5, KSpace)*.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KQuarantine`
- **Upcoming Queue**:
  `KSettings`, `KTaskMgr`, `K2048`, `KSudoku`, `KConnect4`, `KHangman`, `KSimon`, `KFreecell`, `KMatch3`, `KWords`, `KGo`, `KDarts`, `KRSS`, `KClip`, `KCipher`, `KCalc`, `KMine`, `KSnake`, `KTetris`, `KPong`, `KMaze`, `KSolitaire`, `KChess`, `KTimer`, `KConverter` *(Completed: KChess, KMaze, KPong, KStarForge, KTask, KChrono, KSys, KSynth, KCyber, KCosmic, KContacts, KDB, KDragon, KFlash, KFont, KFortress, KGraph, KHabit, KHex, KImage, KJournal, KMail, KMandel, KMech, KMedia, KMystery, KNet, KNote, KPad, KPaint, KPass, KPing, KQuest, KRadio, KRead, KSanctuary, KScript, KStarDredge, KStarship, KStellar, KSubmarine, KTerm, KHash, KRSS, KClip, KCipher, KCalc, KMine, KSnake, KTetris, KSolitaire, KColor, KTimer, KConverter)*.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KSolitaire`
- **Upcoming Queue**:
  `KSudoku`, `KTetris`, `KWords`, `KBBS`, `KCalendar`, `KChess`, `KFreecell`, `KMatch3`, `KPong`, `KSimon`, `KSnake` *(Completed: KCalc, KPomodoro, KTimer, KClock, KHash, KMystery, KMandel, KFont, KPing, KAudio, KSynth, KScript, KRead, KRadio, KSys, KTodo, KTrader, KType, KVault, KVoid, KWizard, KZip, KChrono, KTask, KStarForge, KPad, KBookmark, KRSS, KClip, KHex, KHabit, KFarm, KPaint, KGraph, KImage, KJournal, KMail, KMedia, KNet, KNote, KPass, KMine, KBBS, KCalendar, KChart, KColor, KChess, KConnect4, KConverter, KFreecell, KHangman, KMatch3, KPong, KSettings, KSimon, KSnake)*.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KHangman`
- **Upcoming Queue**:
  `KSimon`, `KAsteroids`, `KFreecell`, `KMatch3`, `KWords`, `KGo`, `KDarts`, `KTowers`, `KReversi`, `KAlchemy`, `KColony`, `KStarForge`, `KBreakout`, `KRogue`, `KTetris`, `KPong`, `KPac`, `KChess`, `KColor`, `KConverter`, `KHabit` *(Completed in Pass 5: KBBS, KChrono, KCipher, KClip, KCyber, KDragon, KFortress, KHash, KMaze, KMech, KMystery, KQuest, KSanctuary, KSnake, KSolitaire, KSpace, KStarDredge, KStarship, KStellar, KSubmarine, KSynth, KSys, KTask, KTerm, KTimer, KTodo, KTrader, KType, KVault, KVoid, KWizard, KZip, KRSS, K2048, KChart, KGraph, KContacts, KScript, KRead, KColosseum, KAbyss, KMedia, KAudio, KRadio, KPad, KPaint, KCalc, KMine, KCosmic, KBase, KBudget, KCalendar, KFarm, KFlash, KFont, KImage, KJournal, KMail, KMandel, KNet, KNote, KPass, KDB, KHex, KBookmark, KPing, KChat, KClock, KPomodoro, KBreakout, KRogue, KTetris, KPong, KPac, KChess, KColor, KConverter, KQuarantine, KSettings, KTaskMgr, KHabit, KSudoku, KConnect4)*.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KDarts`
- **Upcoming Queue**:
  `KTowers`, `KSimon`, `KMatch3`, `KPong`, `KSnake`, `KRogue`, `KBreakout`, `KMine`, `KSolitaire`, `KSudoku`, `KConnect4`, `KGo`, `KReversi` *(Completed: KHangman, KWords, KSolitaire, KColosseum, KSpace, KReversi, KSimon, KMatch3, KTowers, KSnake, KPong, KGo, KSynth, KNote, KFont, KRSS, KSys, KZip, KVault, KType, KMandel, KGraph, KChart, KPaint, KConnect4, KChess, KTetris, KDB, KTodo, KJournal, KCalendar, KContacts, KMail, KRead, KPass, KImage, KAudio, KMedia, KTask, KNet, KPing, KHash, KPad, K2048, KDarts, KRogue, KBreakout, KCalc, KMine, KSudoku)*.
- **Multiplayer Focus (CRITICAL)**: Concentrate on expanding games (*KChess*, *KConnect4*, *KGo*, *KReversi*, *KDarts*, *KTetris*, *KSnake*, *K2048*, *KMatch3*, *KSimon*) and collaborative apps (*KDraw*, *KPaint*, *KSynth*, *KPad*) with seamless Firebase Realtime Database multiplayer for cross-computer play on `kiloapps.web.app`.

### 7. Virtual 1999 Web Expansion Queue (Eternal Fleet Track)
- **Current Active Target**: `kweb://users/~neon_rider` (`KiloOS/public/web/users/neon_rider.html`)
  - *Next in Rotation*: `kweb://asm-temple` ➔ `kweb://cybercafe` ➔ `kweb://10.19.99.4/classified` ➔ `kweb://echo-subsystem.net` ➔ `kweb://deep-core` ➔ `kweb://darknet` ➔ `kweb://portal` ➔ `kweb://webring` ➔ `kweb://warez` ➔ `kweb://geocities`.
- **Anti-Potemkin Quality Standard**:
  - All 11 Virtual 1999 Web destinations are live, interactive, and feature-complete.
  - See [archive/virtual_web_completed.md](archive/virtual_web_completed.md) for full historical breakdown of completed content mandates across all sites.
  - Active focus: Surface-site ARG breadcrumb density pass and cross-site puzzle validation.
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

- **[FLEET: kilo-graphics] — Exclusive Focus on Imagen 3 Asset Overhauls & Inappropriate App Turn Skipping** | Director Directive
  - ⚠️ AGENT NOTE: Human director priority mandate for `kilo-graphics`.
  - Instructions: The graphics agent must do nothing but replace programmer art with Imagen 3 generated assets for a while on its turns. If assigned an app where Imagen 3 asset replacement is not appropriate (pure vector/wireframe arcade games, abstract board games, text utilities, or apps with already mature art), skip the turn cleanly (`⏭️ Skip — Imagen 3 asset replacement not appropriate for [app]`), rotate the target to the queue bottom, and terminate without code changes.

- **[FLEET-WIDE] — Pivot to ARG, Multiplayer & Virtual Net Expansion (Freeze Standalone App Creation)** | Director Directive
  - ⚠️ AGENT NOTE: Human director priority directive.
  - Instructions: Halt creation of new standalone OS apps (frozen at 92 native / 99 web).
    1. **Virtual Net**: Pivot `kilo-creator` 100% to building real, rich Web 1.0 destinations in `KiloOS/public/web/` (`users/~neon_rider`, `asm-temple`, `cybercafe`, Tier 3 hidden nodes) and linking them to `KNet`.
    2. **Multiplayer**: Keep `kilo-expander` dedicated to seamless Firebase RTDB multiplayer retrofits (`KGo`, `KReversi`, `KDarts`, `KTetris`, etc.) so users across different computers can play together without servers.
    3. **ARG Clue-Weaving**: Weave subtle ARG clues (Arc 1 & 2 per `arg_plan.md`) into existing apps: `KHex` (internal IP `10.19.99.4/classified` offset), `KSynth` (1999Hz morse spelling `echo-subsystem.net`), `KTerm` (glitched sysadmin log pointing to `kweb://deep-core`), `KBBS` (sysop server notes), and `KNote` (`system_recovery_1999.log`).

- **[FLEET: kilo-creator & kilo-expander] — Seamless Online Multiplayer via Firebase** | Director Directive
  - ⚠️ AGENT NOTE: Human director request. Priority architectural directive for creator and expander agents.
  - Instructions: Concentrate on adding multiplayer features that work seamlessly through Firebase Realtime Database with different people playing on kiloapps.web.app from different computers that are not otherwise communicating, similar to how KChat allows chat from the global room. Use the shared Firebase RTDB (`https://kiloappschat-default-rtdb.firebaseio.com`) with CDN imports and clean room namespacing (`multiplayer/<app>/...`).
  - **No Autostart Mandate (CRITICAL)**: Never auto-connect, auto-matchmake, or drop players into online matches on application boot. Apps must boot to offline play (e.g. against local Easy AI or solo mode) or present a start screen with an explicit "Connect" / "Play Online" button. Online multiplayer must always be an intentional, user-initiated action.

- **[ALL_APPS / FLEET] — Visual Quality & Graphics** | Director Directive
  - ⚠️ AGENT NOTE: Human director request. Priority fleet-wide directive.
  - Instructions: Systematically remove rotating/traveling specular glint comets, perimeter glint dots, and moving border balls across both web (HTML) and native (Win32 C) on every app pass. They are annoying across every app and look like distracting projectiles/balls. Replace with clean, static, or period-accurate borders without traveling dots or orbital glint particles. NEVER add new perimeter traveling glints.

- **[FLEET: kilo-graphics & kilo-planner] — Daily App Icon Uniqueness Audit** | Director Directive
  - ⚠️ AGENT NOTE: Human director request. Priority fleet-wide directive.
  - Instructions: Ensure that every application registered in `KiloOS/src/App.jsx` has a unique, visually distinctive 32x32 `.ico` file in `KiloOS/public/assets/icons/`. Reusing or copying existing icons is strictly prohibited. Run `python scripts/check_icons.py` to audit for missing or duplicate icon hashes across the fleet daily during planner ticks and on graphics passes. If duplicates are found, resolve them immediately using `python scripts/check_icons.py --fix`.

- **[FLEET: kilo-qa, kilo-usability, kilo-tester] — Toast Occlusion & Modal Clipping Remediation** | Vision Audit Directive
  - ⚠️ AGENT NOTE: Secondary state vision audit revealed 26 apps where persistent or timed toasts (`z-index: 150-200`) overlap interactive controls (buttons, inputs, close icons) and 10 apps with clipped dialogs/virtual keyboards.
  - Instructions: During app passes, ensure toasts do not occlude interactive inputs or primary buttons (position toasts safely, dismiss on click/interaction, or use unobtrusive non-overlapping toast bars). Fix double-modal stacking (`kclip` - fixed, `kpomodoro` - fixed) and remove internal loop labels (`kdarts` - fixed, `kwords`).

- **[ARG / FLEET-WIDE] — Middle-Game Puzzle Chain & Tier 3 Node Gating** | Director Directive
  - ⚠️ AGENT NOTE: Human director priority directive. ARG structural improvement.
  - Instructions: The progression from "player discovers the virtual web" to "player is ready for KMatrix endgame" is too loose. Agents (`kilo-creator`, `kilo-expander`) must build a **gated middle-game puzzle chain** across the Tier 3 nodes (`darknet`, `classified`, `echo_subsystem`, `deep_core`). Specifically:
    1. **Sequential key-artifact system**: Each Tier 3 node should yield a specific artifact (a decoded phrase, a hex offset, a frequency value, a file fragment) that is **required as input** to unlock a deeper layer on a *different* Tier 3 node. Example: decoding a Morse transmission on `echo_subsystem` reveals a memory offset that, when entered in `darknet`'s hex inspector, unlocks a classified memo fragment; that memo contains coordinates that unlock a hidden panel on `classified`.
    2. **Puzzle chain must converge on `deep_core`**: The final Tier 3 node before the endgame. All threads from the other 3 nodes should feed information needed to reach `deep_core`'s inner sanctum, which in turn points players toward KMatrix (App #100).
    3. **No walkthrough scaffolding**: Implement via diegetic mechanisms only (input fields that validate specific answers, hex addresses that highlight when correct, frequency lock indicators). Never explain the sequence or label puzzle steps.
    4. **Breadcrumb ordering hint**: Subtly suggest the investigation order via in-universe timestamps, log sequence numbers, or packet IDs — e.g., `echo_subsystem` logs are dated earliest, `deep_core` latest.

- **[ARG / VIRTUAL WEB] — Surface-Site Breadcrumb Density Pass** | Director Directive
  - ⚠️ AGENT NOTE: Human director priority directive. ARG discoverability improvement.
  - Instructions: The "surface" virtual web sites (`geocities`, `warez`, `webring`, `portal`, `asm_temple`, `cybercafe`) currently have very few ARG breadcrumbs (geocities: 1 reference, warez: 3). Players who only explore surface sites may never feel the pull toward the mystery. Agents (`kilo-creator`, `kilo-expander`, `kilo-graphics`) must seed **subtle, atmospheric anomalies** into every surface site during their next pass:
    1. **Geocities**: A corrupted guestbook entry from a user whose timestamp reads `1999-12-31 23:59:58` containing garbled text that, when decoded (ROT13 or hex), spells a Tier 3 URL fragment. A "neighborhood watch" bulletin mentioning unusual network traffic from subnet `10.19.99.x`.
    2. **Warez**: A scene NFO file from a fictional group whose release notes contain a suspiciously specific frequency (`1999Hz`) and mention intercepting "echo transmissions." A cracktro that briefly flashes hex addresses matching `deep_core` offsets.
    3. **Webring**: One or two "dead" webring nodes in the topology map that resolve to `???` or show anomalous ping times, hinting at hidden nodes not in the public directory. A traceroute that passes through a gateway named `echo-gw-07.kilonet.internal`.
    4. **Portal**: A classified ad posted by "SysAdmin_NULL" seeking help with "anomalous signal patterns at 1999Hz" with a reply-to address pointing to the echo subsystem. A news ticker item about unexplained network anomalies on the corporate intranet.
    5. **ASM Temple**: An opcode reference entry with a "NOTE" annotation referencing an undocumented instruction behavior observed "only on KiloNet internal nodes." A PE32 dissector sample binary whose embedded strings contain Tier 3 URL fragments.
    6. **Cybercafe**: An IRC channel message from a user warning about "ghost packets from 10.19.99.4" or a BBS thread titled "has anyone else heard the signal?" with timestamps that form a pattern.
    7. **Tone**: All breadcrumbs must be *atmospheric and ambient* — things a casual user might dismiss as flavor text but an attentive investigator would collect. Never label, highlight, or explain them.

- **[ARG / FLEET: kilo-creator & kilo-expander] — Solo-Completable Collaborative ARG Mechanics** | Director Directive
  - ⚠️ AGENT NOTE: Human director priority directive. Multiplayer ARG design constraint.
  - Instructions: Build ARG puzzle mechanics that *leverage* the Firebase RTDB multiplayer infrastructure as a clue-delivery or puzzle-solving medium, but with the hard constraint that **every puzzle must be completable by a single person** using either a VM, a phone alongside their computer, or any two browser tabs. Do NOT require strangers or coordinated groups. Specifically:
    1. **Shared signal board**: A Firebase RTDB node (`arg/signals/...`) where certain in-app actions (e.g., tuning KRadio to 1999Hz, entering a specific hex value in KHex, completing a specific KCipher decode) silently write a "signal fragment" to the shared board. When enough fragments accumulate (from the same user across different apps, or across sessions), a hidden panel on one of the Tier 3 web pages reveals new content. The threshold should be reachable by one person using 2-3 apps.
    2. **Dual-presence puzzle**: One Tier 3 node requires two simultaneous "connections" — but this should work with two browser tabs on the same computer, or a phone and desktop both visiting `kiloapps.web.app`. Example: `classified` shows a locked signal diagnostic that requires a second user (or the same user in another tab) to be actively running `echo_subsystem`'s Morse transmitter at the correct frequency. The RTDB presence check should use session IDs, not unique users, so one person with two tabs qualifies.
    3. **Dead-drop guestbooks**: Certain Firebase guestbooks (`cybercafe` IRC, `geocities` shoutbox, `neon_rider` guestbook) should have a hidden mechanic where posting a specific passphrase (discoverable from puzzle chain artifacts) triggers a server-side Firebase rule or client-side listener that reveals a hidden response message containing the next clue. Since these are persistent RTDB writes, the player's own post triggers their own reveal — no second person needed.
    4. **Explicit design rule**: If a puzzle involves Firebase presence, matchmaking, or multi-session state, it MUST include a "solo path" — either via multi-tab, or via a time-delayed fallback (e.g., if no second connection appears within 30 seconds, the puzzle auto-advances with a "signal lock acquired from cached relay" diegetic message).

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-08T01:45:00-07:00 — kilo-tester: KConverter (Interactive UI Audit: Smart Parser Delimiter Fix, JSON Portability & State Ergonomics)**
  - Status: PASS ✅ (6 issues identified and fixed; 0 regressions).
  - Parser Fix: Implemented `splitExpressUnits` resolving collision where unit `in` (inches) broke expression splitting into empty tokens.
  - Express Ergonomics: Added `12 in ➔ cm` preset; wired `X`/`S` swap shortcut in Express mode; clear button resets empty card state.
  - Workspace Portability: Added full workspace JSON export and file import with schema validation inside Help modal.
  - State & Safety: Added dropdown option fallbacks in `populateDropdowns`; fixed `dismissTutorial` respecting checkbox state.
  - Accessibility & Focus: Added focus trapping and restoration for Help modal with `btnCloseHelp` element ID.
  - Verification: `test_app_startup.py` PASS; `test_web_apps.js` PASS (75 controls); MSVC clean; Vite build clean (510ms); web 102.2 KB < 999 KB ceiling.
  - Queue: Advanced `kilo_tester` to `KQuarantine`; rotation handoff to `kilo-usability`.

- **2026-10-08T01:20:00-07:00 — kilo-graphics: KWizard (Skip Turn — Inappropriate Target & Glint/Border Audit)**
  - Status: ⏭️ Skip — Imagen 3 asset replacement not appropriate for KWizard
  - Rationale: Fantasy dueling card game locked in human review queue with 1:1 dual-target Win32 GDI C parity and procedural mage archetypes.
  - Glint & Border Ban: Verified static arena frame; zero rotating specular glints or traveling perimeter border dots in web or native C (Rule 11 compliant).
  - Verification: `check_icons.py` 100% PASS; `security_lint.py` 100% PASS; MSVC clean (12.9 KB); `test_app_startup.py` PASS; web (129.3 KB) < 999 KB ceiling.
  - Queue: Advanced `kilo_graphics` to `KColony`; rotation handoff to `kilo-tester`.

- **2026-10-07T23:36:00-07:00 — kilo-expander: KReversi (Skip Turn — Mature 5+ Passes & Locked in Human Review Queue)**
  - Status: ⏭️ Skip — app is feature-complete and mature.
  - Review Queue: Verified locked in `docs/human_review_queue.md` (60 FPS, 17ms pacing, 222 KB < 999 KB).
  - Parity & Standards: Full RFMS RetroMultiplayer, Rule 11 clean borders, F5/F9 state parity, opening book & FEN engine active.
  - Verification: `security_lint.py` 100% PASS; `test_app_startup.py` PASS (0 JS err); MSVC clean; Vite build clean (642ms).
  - Queue: Advanced `kilo_expander` to `KDarts`; rotation handoff to `kilo-creator`.

- **2026-10-07T23:20:00-07:00 — kilo-qa: KConnect4 (Pass 5: Tutorial & State Integrity, Quicksave/Load Parity, Rule 11 Clean Borders)**
  - Status: PASS ✅ (0 regressions, 179.2 KB web / 191.0 KB native < 999 KB ceiling).
  - Full State Persistence: F5 quicksave and F9 quickload parity across web and Win32 C with state feedback.
  - Tutorial & Overlay Integrity: First-run tutorial flag `kconnect4_tutorialSeen` / `.dat` guards fresh sessions; Esc/Enter/Space modal dismiss.
  - Visual Quality & Polish: Clean static perimeter border in web and native (Rule 11 compliant).
  - Storage & Resource Safety: Wrapped local storage with safe getters/setters; visibilitychange loop pausing.
  - Verification: `security_lint.py` 100% PASS; `test_web_apps.js` PASS (107/107); MSVC native clean; Vite clean (646ms).
  - Queue: Advanced `kilo_qa` to `KHangman`; rotation handoff to `kilo-expander`.

- **2026-10-07T22:40:00-07:00 — kilo-usability: KSnake (UI/UX Pass: Window Dimensions, HiDPI Retina Crispness, Toast Occlusion & Rule 11 Clean Borders)**
  - Status: PASS ✅ (0 regressions, 289.9 KB web / 54.8 KB native < 999 KB ceiling).
  - Window & Layout: Tuned dimensions to 860x720 in App.jsx; container overflow-y prevents clipping in Duel/Editor modes.
  - HiDPI Retina Crispness: Implemented applyCanvasDpi with dynamic DPR, disabled image smoothing, and resize handler.
  - Toast Occlusion & Safety: Moved toast container to bottom-viewport center to prevent occluding header and modal close buttons.
  - Controls & Help Ergonomics: Added F1 preventDefault; expanded in-game Help guide with Quicksave/Load and feature shortcuts.
  - Rule 11 Compliance: Removed pulsating perimeter border in web and native Win32 C, replaced with clean static arcade border.
  - Verification: Security linter 100% PASS; test_app_startup PASS; MSVC native clean; Vite clean (5.99s).
  - Queue: Advanced kilo_usability to KSolitaire; rotation handoff to kilo-qa.






