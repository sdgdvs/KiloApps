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
  kilo_tester: KPomodoro
  kilo_usability: KCalc
  kilo_graphics: KSubmarine
  kilo_qa: KVoid
  kilo_expander: "KConnect4 (Firebase RTDB Multiplayer)"
  kilo_creator: "KNetMap (Subnet Topology Visualizer)"
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
  agent: kilo-usability
  app: KClip
  timestamp: "2026-09-23T06:50:00Z"
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
- **Current Target**: `KNetMap` (Subnet Topology Visualizer)
- **Upcoming Concepts**:
  `KSteno` (Stenographic Carrier Suite), `KPacket` (Packet Inspector).
- **Multiplayer Focus (CRITICAL)**: Prioritize concepts and games featuring seamless cross-computer Firebase Realtime Database multiplayer (`https://kiloappschat-default-rtdb.firebaseio.com`), allowing players on `kiloapps.web.app` from different computers to play together seamlessly without custom servers.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Current Target**: `KSubmarine`
- **Upcoming Queue**:
  `KStarDredge`, `KAbyss`, `KColosseum`, `KCyber`, `KMech`, `KMystery`, `KVoid`, `KWizard`, `KStarship`, `KChrono`, `KStarForge`, `KFortress`, `KCosmic`, `KStellar`, `KSanctuary`, `KDragon`.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KPomodoro` (double-modal collision fix)
- **Upcoming Queue**:
  `KCalc` (toast blocking calculation buttons), `KHangman` (keyboard cutoff), `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAudio`, `KBBS`, `KBase`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColosseum`, `KContacts`, `KCosmic`, `KCyber`, `KDB`, `KDragon`, `KFlash`, `KFont`, `KFortress`, `KGraph`, `KHabit`, `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMech`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KSanctuary`, `KScript`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`, `KTask`, `KStarForge`, `KTerm`, `KHash`, `KRSS`, `KClip`, `KCipher`.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KCalc` (reposition bottom-center toast blocking buttons)
- **Upcoming Queue**:
  `KHex` (Tab 2 label, scrollbar, row cutoff), `KContacts` (reposition bottom-right toast blocking submit button), `KFarm` (reposition bottom toast blocking seed radio buttons), `KPaint`, `KAudio`, `KFont`, `KGraph`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPass`, `KPing`, `KRadio`, `KRead`, `KScript`, `KSynth`, `KSys`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KChrono`, `KTask`, `KStarForge`, `KPad`, `KBookmark`, `KHash`, `KRSS`, `KClip`.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KVoid`
- **Upcoming Queue**:
  `KWizard`, `KZip`, `KChrono`, `KCyber`, `KDragon`, `KFortress`, `KMech`, `KMystery`, `KQuest`, `KSanctuary`, `KStarDredge`, `KSubmarine`, `KHash`, `KRSS`, `KClip`, `KCipher`, `KTodo`, `KTrader`, `KType`, `KVault` *(Completed in Pass 5: K2048, KAudio, KBBS, KMaze, KSnake, KSolitaire, KSpace, KStarship, KStellar, KSynth, KSys, KTask, KTerm, KTimer, KTodo, KTrader, KType, KVault)*.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KConnect4` (Implement Seamless Firebase RTDB Multiplayer per Mandate 12)
- **Upcoming Queue**:
  `KChess` (Multiplayer), `KGo` (Multiplayer), `KReversi` (Multiplayer), `KDarts` (Multiplayer), `KTetris` (Arcade Duel Multiplayer), `KSnake` (Multiplayer), `KDB`, `KTodo`, `KJournal`, `KCalendar`, `KContacts`, `KMail`, `KRead`, `KPass`, `KPaint`, `KImage`, `KAudio`, `KSynth`, `KMedia`, `KChart`, `KGraph`, `KMandel`, `KType`, `KVault`, `KZip`, `KSys`, `KTask`, `KNet`, `KPing`, `KHash`, `KRSS`, `KFont`, `KPad`, `KNote`.
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
  - Instructions: During app passes, ensure toasts do not occlude interactive inputs or primary buttons (position toasts safely, dismiss on click/interaction, or use unobtrusive non-overlapping toast bars). Fix double-modal stacking (`kclip`, `kpomodoro`) and remove internal loop labels (`kdarts`, `kwords`).

---

## Recent Execution Logs (Max 5 Entries)

- **2026-09-23T06:50:00Z — kilo-usability: KClip (Modal Isolation, Backdrop Collision & Toast Occlusion Remediation)**
  - Status: PASS ✅ (Eliminated modal collisions and toast occlusion; 0 regressions).
  - Modal Isolation: Enforced strict mutual exclusivity across all modals; auto-cleared background backdrops and ARIA states.
  - Tutorial Ergonomics: Added direct Snippets Library navigation button to tutorial footer for seamless onboarding transition.
  - Toast Positioning: Repositioned toast from bottom-right (blocking F1 status button) to top-right with instant click-to-dismiss.
  - Shortcut Guard: Blocked background key listeners (F1, F5, F9, B, T, C) during active modal states; enabled Enter/Space for templates.
  - Verification: Headless CDP test pass (74 interactive elements, 0 errors, 60 FPS frame pacing); Vite clean build; C build clean (15.8 KB).

- **2026-09-23T05:52:00Z — kilo-tester: KCipher (Interactive UI Audit & Inline Repairs)**
  - Status: PASS ✅ (6 UI/state issues resolved; 0 regressions).
  - State Sync: Synchronized `state` across QuickLoad, JSON Import/Export, and Lore Transmissions.
  - Modal & Backdrop: Added backdrop click dismissal to Tutorial and Splash overlays; wired Escape key across all modals.
  - Toast Ergonomics: Repositioned toast from bottom-right (occluding buttons) to top-right with click-to-dismiss.
  - Steganography & Bitplane: Added live capacity tracker, corrected RGB LSB capacity display, fixed bitplane view state leak.
  - Controls & Shortcuts: Added `Ctrl+Shift+Enter` decrypt hotkey, `Ctrl+1..5` tab shortcuts, and bidirectional decrypt logic.
  - Verification: Headless CDP test pass (58 elements, 0 errors, 60 FPS); Vite build clean; MSVC C build clean (9.2 KB).

- **2026-09-23T03:55:00Z — kilo-creator: KCipher (Cryptographic Cipher Suite & Steganography Workbench)**
  - Status: PASS ✅ (New application created; native Win32 C + HTML5 web app registered in KiloOS).
  - Web App (kcipher.html, 94.5 KB): 6 ciphers (Caesar/ROT13, Vigenère, Rail Fence, Substitution, RC4, XOR), live frequency analyzer, IoC/Entropy meters, Caesar brute-force cracker, 1-bit LSB steganography engine, procedural Web Audio.
  - Native App (KCipher.exe, 9.0 KB): Win32 GUI with Caesar, Vigenère, Rail Fence, Atbash, and RC4 stream ciphers.
  - Standards: Start splash screen, first-run tutorial (`kcipher_tutorialSeen`), F5 quicksave / F9 quickload, JSON export/import.
  - Lore Consonance: 1999 warez/ARG intercepts (SlashNet, FLARELIGHT, RAZOR 1999, KMatrix precursors).
  - Registration & Build: Added to System in `App.jsx`; Vite build clean (1.16s); `check_sizes.py` PASS; size <999KB ceiling.

- **2026-09-23T03:35:00Z — kilo-vision-audit: Fleet Secondary / Interacted State Vision Audit**
  - Status: PASS ✅ (103 apps audited across interacted UI states; 40 visual issues cataloged).
  - Headless Interaction: Enhanced `scripts/test_web_apps.js` to open menus, tabs, drawers, and modal states without hangs.
  - Secondary Capture: Generated 103 `_interact.png` screenshots and built interactive view toggles in gallery.
  - Defect Catalog: Cataloged 40 UI issues (26 toast occlusions over inputs/buttons, 2 modal stacking collisions, 10 cutoffs/overflows).
  - Gallery Enrichment: Updated `docs/gallery/index.html` and generated `docs/gallery/vision_scores_interact.json` (Fleet Interacted Avg: ★ 8.68/10).
  - Bug Fixes: Repaired `kreversi.html` syntax error and `ktrader.html` interaction reload crash.
  - Verification: Security lint 100% clean; KiloOS Vite build clean (233ms).

- **2026-09-23T03:10:00Z — kilo-qa: KChat AI Persona Removal**
  - Status: PASS ✅ (Completely removed AI persona section and feature from web and native KChat).
  - Web (kchat.html): Removed top AI persona selector, Ask AI button, /ai slash command, and activePersona state.
  - Native (main.c): Removed hPersonaCombo, hAskAI button, /ai command, GenerateAIResponse, and activePersona stats.
  - Channels & Help: Updated #ai-lounge channel to #lounge; updated Help tutorial, shortcuts, and documentation.
  - Ergonomics: Restored standard Ctrl+A select-all behavior in native edit control.
  - Verification: Native MSVC C clean build (26.5 KB); HTML5 (85.1 KB); Headless CDP 60 FPS pass (0 errors); Lint clean.
