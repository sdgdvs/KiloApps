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
  kilo_tester: KHangman
  kilo_usability: "KHex (Weave Arc 1 Memory Offset IP Clue)"
  kilo_graphics: "KAbyss (Weave Arc 2 Precursor Relic Glyph)"
  kilo_qa: KZip
  kilo_expander: "KGo (Firebase RTDB Online Multiplayer)"
  kilo_creator: "kweb://users/~neon_rider (Virtual Web 1999 & ASM Devlog)"
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
  app: KCalc
  timestamp: "2026-09-23T17:50:00Z"
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
13. **🎨 Daily App Icon Uniqueness Audit (DIRECTOR MANDATE - CRITICAL)**:
    - Every application in `KiloOS/src/App.jsx` MUST possess a unique, visually distinctive 32x32 `.ico` file in `KiloOS/public/assets/icons/`. Reusing icons or copying existing `.ico` files (e.g. copying `kpass.ico` or pointing multiple apps to `knet.ico`) is strictly prohibited.
    - The autonomous fleet enforces this daily via `scripts/check_icons.py` during `kilo-planner` runs and on every `kilo-graphics` pass. If missing or duplicate icon hashes are detected, resolve immediately via `python scripts/check_icons.py --fix`.

---

## Active Target Queues

### 1. Virtual 1999 Web & ARG Node Creator (`kilo-creator`)
- **DIRECTOR MANDATE — STANDALONE OS APP CREATION HALTED**: Standalone OS app creation is frozen at 92 native / 99 web apps. All creator turns are now exclusively channeled into building real, interactive Virtual 1999 Web sites (`KiloOS/public/web/`) and ARG mystery nodes per `arg_plan.md`. Zero shallow stubs; every page must be a functioning Web 1.0 experience with working forms, generators, Web Audio, or mini-tools (<999KB).
- **Current Target**: `kweb://users/~neon_rider` (Personal Hacker / Demoscene & Win32 ASM Homepage)
- **Upcoming Queue**:
  `kweb://asm-temple` (x86 Assembly Programming Shrine & Opcode Converter),
  `kweb://cybercafe` (Underground BBS, Threaded Forums & ASCII Art Canvas),
  `kweb://10.19.99.4/classified` (Tier 3 Corporate Intranet Leak & Memory Dumps),
  `kweb://echo-subsystem.net` (Tier 3 Research Journal & Harmonic Decoders),
  `kweb://deep-core` (Tier 3 Ghost Node Terminal & KMatrix Passkey Fragment),
  `kweb://portal` (Yahoo/Excite 1999 Directory Upgrade: simulated search, live stocks, classifieds),
  `kweb://warez` (Cracktros, Chiptune Jukebox & Demoscene Vault expansion).

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Current Target**: `KAbyss`
- **Upcoming Queue**:
  `KColosseum`, `KCyber`, `KMech`, `KMystery`, `KVoid`, `KWizard`, `KStarship`, `KChrono`, `KStarForge`, `KFortress`, `KCosmic`, `KStellar`, `KSanctuary`, `KDragon`, `KSubmarine`, `KStarDredge`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KHangman` (keyboard cutoff)
- **Upcoming Queue**:
  `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAudio`, `KBBS`, `KBase`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColosseum`, `KContacts`, `KCosmic`, `KCyber`, `KDB`, `KDragon`, `KFlash`, `KFont`, `KFortress`, `KGraph`, `KHabit`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMech`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KSanctuary`, `KScript`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`, `KTask`, `KStarForge`, `KTerm`, `KHash`, `KRSS`, `KClip`, `KCipher`, `KPomodoro`, `KCalc`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KHex` (Tab 2 label, scrollbar, row cutoff)
- **Upcoming Queue**:
  `KContacts` (reposition bottom-right toast blocking submit button), `KFarm` (reposition bottom toast blocking seed radio buttons), `KPaint`, `KAudio`, `KFont`, `KGraph`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPass`, `KPing`, `KRadio`, `KRead`, `KScript`, `KSynth`, `KSys`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KChrono`, `KTask`, `KStarForge`, `KPad`, `KBookmark`, `KHash`, `KRSS`, `KClip`, `KCalc`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KZip`
- **Upcoming Queue**:
  `KChrono`, `KCyber`, `KDragon`, `KFortress`, `KMech`, `KMystery`, `KQuest`, `KSanctuary`, `KStarDredge`, `KSubmarine`, `KHash`, `KRSS`, `KClip`, `KCipher`, `KTodo`, `KTrader`, `KType`, `KVault` *(Completed in Pass 5: K2048, KAudio, KBBS, KMaze, KSnake, KSolitaire, KSpace, KStarship, KStellar, KSynth, KSys, KTask, KTerm, KTimer, KTodo, KTrader, KType, KVault, KVoid, KWizard)*.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KGo` (Implement Seamless Firebase RTDB Multiplayer per Mandate 12)
- **Upcoming Queue**:
  `KReversi` (Multiplayer), `KDarts` (Multiplayer), `KTetris` (Arcade Duel Multiplayer), `KSnake` (Multiplayer), `KDB`, `KTodo`, `KJournal`, `KCalendar`, `KContacts`, `KMail`, `KRead`, `KPass`, `KPaint`, `KImage`, `KAudio`, `KSynth`, `KMedia`, `KChart`, `KGraph`, `KMandel`, `KType`, `KVault`, `KZip`, `KSys`, `KTask`, `KNet`, `KPing`, `KHash`, `KRSS`, `KFont`, `KPad`, `KNote` *(Completed: KConnect4, KChess)*.
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

- **[FLEET-WIDE] — Pivot to ARG, Multiplayer & Virtual Net Expansion (Freeze Standalone App Creation)** | Director Directive
  - ⚠️ AGENT NOTE: Human director priority directive.
  - Instructions: Halt creation of new standalone OS apps (frozen at 92 native / 99 web).
    1. **Virtual Net**: Pivot `kilo-creator` 100% to building real, rich Web 1.0 destinations in `KiloOS/public/web/` (`users/~neon_rider`, `asm-temple`, `cybercafe`, Tier 3 hidden nodes) and linking them to `KNet`.
    2. **Multiplayer**: Keep `kilo-expander` dedicated to seamless Firebase RTDB multiplayer retrofits (`KGo`, `KReversi`, `KDarts`, `KTetris`, etc.) so users across different computers can play together without servers.
    3. **ARG Clue-Weaving**: Weave subtle ARG clues (Arc 1 & 2 per `arg_plan.md`) into existing apps: `KHex` (internal IP `10.19.99.4/classified` offset), `KSynth` (1999Hz morse spelling `echo-subsystem.net`), `KTerm` (glitched sysadmin log pointing to `kweb://deep-core`), `KBBS` (sysop server notes), and `KNote` (`system_recovery_1999.log`).

- **[FLEET: kilo-creator & kilo-expander] — Seamless Online Multiplayer via Firebase** | Director Directive
  - ⚠️ AGENT NOTE: Human director request. Priority architectural directive for creator and expander agents.
  - Instructions: Concentrate on adding multiplayer features that work seamlessly through Firebase Realtime Database with different people playing on kiloapps.web.app from different computers that are not otherwise communicating, similar to how KChat allows chat from the global room. Use the shared Firebase RTDB (`https://kiloappschat-default-rtdb.firebaseio.com`) with CDN imports and clean room namespacing (`multiplayer/<app>/...`).

- **[ALL_APPS / FLEET] — Visual Quality & Graphics** | Director Directive
  - ⚠️ AGENT NOTE: Human director request. Priority fleet-wide directive.
  - Instructions: Systematically remove rotating/traveling specular glint comets, perimeter glint dots, and moving border balls across both web (HTML) and native (Win32 C) on every app pass. They are annoying across every app and look like distracting projectiles/balls. Replace with clean, static, or period-accurate borders without traveling dots or orbital glint particles. NEVER add new perimeter traveling glints.

- **[FLEET: kilo-graphics & kilo-planner] — Daily App Icon Uniqueness Audit** | Director Directive
  - ⚠️ AGENT NOTE: Human director request. Priority fleet-wide directive.
  - Instructions: Ensure that every application registered in `KiloOS/src/App.jsx` has a unique, visually distinctive 32x32 `.ico` file in `KiloOS/public/assets/icons/`. Reusing or copying existing icons is strictly prohibited. Run `python scripts/check_icons.py` to audit for missing or duplicate icon hashes across the fleet daily during planner ticks and on graphics passes. If duplicates are found, resolve them immediately using `python scripts/check_icons.py --fix`.

- **[FLEET: kilo-qa, kilo-usability, kilo-tester] — Toast Occlusion & Modal Clipping Remediation** | Vision Audit Directive
  - ⚠️ AGENT NOTE: Secondary state vision audit revealed 26 apps where persistent or timed toasts (`z-index: 150-200`) overlap interactive controls (buttons, inputs, close icons) and 10 apps with clipped dialogs/virtual keyboards.
  - Instructions: During app passes, ensure toasts do not occlude interactive inputs or primary buttons (position toasts safely, dismiss on click/interaction, or use unobtrusive non-overlapping toast bars). Fix double-modal stacking (`kclip` - fixed, `kpomodoro` - fixed) and remove internal loop labels (`kdarts`, `kwords`).

---

## Recent Execution Logs (Max 5 Entries)

- **2026-09-23T17:50:00Z — kilo-tester: KCalc (Interactive UI Audit & Inline Fixes)**
  - Status: PASS ✅ (6 UI/functional issues audited and fixed; 0 regressions).
  - Quicksave & Load: Implemented full workspace snapshots across [F5] Save, [F9] Load, and header buttons (`kcalc_quicksave`).
  - First-Run Tutorial: Added onboarding tutorial modal (`kcalc_tutorialSeen`) with Esc/Enter/Space/backdrop dismissal and Help access.
  - Math & Keypad Fixes: Resolved modulo/percentage regex collision (`10 mod 3`), wired reciprocal `1/x` handler, and fixed multi-memory indicators.
  - Toast & Validation: Eliminated 4-toast startup blitz (`silent=true`), added financial input bounds checking, and guarded `sendToCalc`.
  - Data Portability: Added JSON workspace state export/import (`kcalc_workspace.json`) alongside CSV/TXT history tools.
  - Verification: 15/15 node math/UI unit tests pass; MSVC C clean build (26.1 KB); Vite build clean (379ms); security lint clean; <999KB ceiling.

- **2026-09-23T16:45:00Z — kilo-creator: KSteno (Stenographic Carrier Suite & Dead-Drop Network)**
  - Status: PASS ✅ (New app created: native Win32 C + HTML5 web app registered in KiloOS).
  - Web App (ksteno.html, 125.7 KB): Multi-carrier workbench: 1/2/4-bit image LSB (seeded PRNG), 16-bit PCM audio modulation, SNOW whitespace chaff, Chi-Square (χ²) PoVs steganalysis, Firebase RTDB global dead-drop network.
  - Native App (KSteno.exe, 9.5 KB): Win32 GDI desktop carrier suite, whitespace encoder/decoder, RC4 payload armor, χ² frequency analyzer.
  - Mandates: Firebase cross-computer dead-drop channels (#global-dead-drop, #flarlight-covert, #sub-rosa-99), 0 glint particles, top-right safe toasts.
  - Standards: Title splash screen, first-run tutorial (`ksteno_tutorialSeen`), F5 quicksave/F9 quickload, 1999 ARG lore (FLARELIGHT, Node 0x7F).
  - Universal Audio: Procedural Genesis YM2612 FM synthesis operator pairs & SPC700 stereo delay warmth.
  - Verification: MSVC C clean build (9.5 KB); Vite clean build (356ms); security lint 100% PASS; strict <999KB ceiling.

- **2026-09-23T15:55:00Z — kilo-expander: KChess (Firebase RTDB Seamless Online Multiplayer)**
  - Status: PASS ✅ (Cross-computer multiplayer verified; 0 regressions).
  - Online Multiplayer: Added Firebase RTDB real-time move sync (`multiplayer/kchess/rooms/<roomId>`), turn alternation, SAN/FEN sync, and presence with `onDisconnect()`.
  - Matchmaking & Lobby: Built public lobby index (`multiplayer/kchess/lobby`), instant 1-click Quick Match, custom room creation (public/private), and code join.
  - Social & Controls: Added interactive match HUD bar with quick chat phrases, rematch handshakes, invite link copy, and spectator mode.
  - UI Ergonomics: Added `[O]` hotkey & toolbar button, online turn indicator, and disabled disruptive offline controls during active matches.
  - Verification: Single-file web (`kchess.html` 154.0 KB); Vite build clean (375ms); security lint 100% clean; strict <999 KB ceiling.

- **2026-09-23T14:45:00Z — kilo-qa: KWizard (Pass 5: Tutorial & State Integrity)**
  - Status: PASS ✅ (Full state persistence & tutorial isolation verified; 0 regressions).
  - Quicksave & Load: Implemented full state persistence across F5/F9 hotkeys and toolbar buttons in web (`kwizard_save`) and native C (`kwizard.dat`).
  - First-Run Tutorial: Added session-isolated onboarding (`kwizard_tutorialSeen` / `kwizard_tutorial.dat`) with Esc/Enter/Space dismissal.
  - UI Ergonomics: Added top-right toast notification system and comprehensive controls guide ([F1] Grimoire, [F5] Save, [F9] Load, [D] Deck, [E] End Turn).
  - Verification: MSVC C clean build (`KWizard.exe` 31.7 KB); single-file web (`kwizard.html` 82.1 KB); Vite build clean (346ms); security lint 100% clean.

- **2026-09-23T13:50:00Z — kilo-graphics: KStarDredge (Specular Glint Removal, Visual Polish & Raider Balance Pass)**
  - Status: PASS ✅ (Eliminated specular glints across web & C; verified clean HUD and synced raider balance).
  - Glint Purge: Removed pulsing specular glint square on ore chunks, mineral core glint dot, and canopy glint slash in `kstardredge.html` & `main.c`.
  - Balance Pass: Synchronized raider combat stats and bounty payouts in native C with web design specs (Skiff 90HP/450CR, Gunship 220HP/1100CR, Dread 450HP/2800CR).
  - Visual Polish: Cleaned asteroid ore crystal and ship cockpit glass geometry without rogue projectile-like artifacts.
  - Verification: MSVC C clean build (`KStarDredge.exe` 260.6 KB); single-file web (`kstardredge.html` 449.3 KB); Vite build clean (375ms); security lint 100% clean.

