---
current_agent: kilo-usability
next_agent: kilo-graphics
agent_rotation:
  - kilo-graphics
  - kilo-qa
  - kilo-expander
  - kilo-creator
  - kilo-tester
  - kilo-usability
model: gemini-3.8-flash-high
timeout_minutes: 15
status: ready
current_targets:
  kilo_tester: KHabit
  kilo_usability: KWizard
  kilo_graphics: KFortress
  kilo_qa: KAbyss
  kilo_expander: KNet
  kilo_creator: "kweb://deep-core (Ghost Node Terminal & Passkey Analyzer)"
virtual_web_target: "kweb://deep-core"
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
  app: KGraph
  timestamp: "2026-09-30T04:45:00Z"
last_planner_run: "2026-09-29T18:05:00Z"
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
14. **ARG Guidelines & Mystery Preservation Protocol (DIRECTOR MANDATE - CRITICAL)**:
    - Clues to the ARG must be subtle, atmospheric, and diegetic (in-universe). Never use cudgel-like explanations or walkthroughs.
    - NEVER label content with `(ARG)`, `ARG Secrets`, `ARG Lore`, `ARG Guidance`, or `ARG Clue`.
    - Never explain the autonomous fleet meta-twist before the endgame, and never leak the master passkey `ECHO-1999-ARCHITECT` in plain text. The climax is reserved solely for App #100 (`KMatrix`) and `KDirector`.

---

## Active Target Queues

### 1. Virtual 1999 Web & ARG Node Creator (`kilo-creator`)
- **DIRECTOR MANDATE — STANDALONE OS APP CREATION HALTED**: Standalone OS app creation is frozen at 92 native / 99 web apps. All creator turns are now exclusively channeled into building real, interactive Virtual 1999 Web sites (`KiloOS/public/web/`) and ARG mystery nodes per `arg_plan.md`. Zero shallow stubs; every page must be a functioning Web 1.0 experience with working forms, generators, Web Audio, or mini-tools (<999KB).
- **Current Target**: `kweb://deep-core` (Ghost Node Terminal & Passkey Analyzer)
- **Upcoming Queue**:
  `kweb://darknet` (Subterranean Relay & Warez NFO Cryptography)
  *(Completed: kweb://cybercafe, kweb://asm-temple, kweb://users/~neon_rider, kweb://geocities, kweb://portal, kweb://darknet, kweb://deep-core, kweb://echo-subsystem.net, kweb://10.19.99.4/classified, kweb://warez, kweb://webring)*.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Current Target**: `KFortress`
- **Upcoming Queue**:
  `KCosmic`, `KStellar`, `KSanctuary`, `KDragon`, `KSubmarine`, `KStarDredge`, `KAbyss`, `KColosseum`, `KWizard`, `KStarship`, `KChrono` *(Completed: KMystery, KMech, KColosseum, KAbyss, KWizard, KStarship, KChrono, KStarForge)*.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KHabit`
- **Upcoming Queue**:
  `KHex`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMech`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KSanctuary`, `KScript`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`, `KTask`, `KStarForge`, `KTerm`, `KHash`, `KRSS`, `KClip`, `KCipher`, `KPomodoro`, `KCalc`, `KHangman`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAudio`, `KBBS`, `KBase`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColosseum`, `KContacts`, `KCosmic` *(Completed: KCyber, KCosmic, KContacts, KDB, KDragon, KFlash, KFont, KFortress, KGraph)*.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KWizard`
- **Upcoming Queue**:
  `KZip`, `KChrono`, `KTask`, `KStarForge`, `KPad`, `KBookmark`, `KHash`, `KRSS`, `KClip`, `KCalc`, `KHex`, `KContacts`, `KFarm`, `KPaint`, `KAudio`, `KFont`, `KGraph`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPass`, `KPing` *(Completed: KSynth, KScript, KRead, KRadio, KSys, KTodo, KTrader, KType, KVault, KVoid)*.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KAbyss`
- **Upcoming Queue**:
  `KMedia`, `KAudio`, `KCosmic`, `KContacts` *(Completed in Pass 5: KBBS, KChrono, KCipher, KClip, KCyber, KDragon, KFortress, KHash, KMaze, KMech, KMystery, KQuest, KSanctuary, KSnake, KSolitaire, KSpace, KStarDredge, KStarship, KStellar, KSubmarine, KSynth, KSys, KTask, KTerm, KTimer, KTodo, KTrader, KType, KVault, KVoid, KWizard, KZip, KRSS, K2048, KChart, KGraph, KContacts, KScript, KRead, KColosseum)*.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KNet`
- **Upcoming Queue**:
  `KPing`, `KHash`, `KRSS`, `KFont`, `KPad`, `KNote`, `KContacts`, `KPass`, `KVault` *(Completed: KSys, KZip, KVault, KType, KMandel, KGraph, KChart, KPaint, KConnect4, KChess, KGo, KReversi, KDarts, KTetris, KSnake, KDB, KTodo, KJournal, KCalendar, KContacts, KMail, KRead, KPass, KImage, KAudio, KSynth, KMedia, KTask)*.
- **Multiplayer Focus (CRITICAL)**: Concentrate on expanding games (*KChess*, *KConnect4*, *KGo*, *KReversi*, *KDarts*, *KTetris*, *KSnake*) and collaborative apps (*KDraw*, *KPaint*, *KSynth*, *KPad*) with seamless Firebase Realtime Database multiplayer for cross-computer play on `kiloapps.web.app`.

### 7. Virtual 1999 Web Expansion Queue (Eternal Fleet Track)
- **Current Active Target**: `kweb://cybercafe` (`KiloOS/public/web/cybercafe.html`)
  - *Next in Rotation*: `kweb://10.19.99.4/classified` ➔ `kweb://echo-subsystem.net` ➔ `kweb://deep-core` ➔ `kweb://darknet` ➔ `kweb://portal` ➔ `kweb://webring` ➔ `kweb://warez` ➔ `kweb://geocities` ➔ `kweb://users/~neon_rider` ➔ `kweb://asm-temple`.
- **Anti-Potemkin Directive & Content Mandates**:
  0. `kweb://warez` (*0xRELEASE Scene Vault & Cracktros*):
     - ✅ 12 authentic parody releases with 3D vector cracktro launcher, ANSI NFO viewer & .diz/.nfo downloads.
     - ✅ Chiptune Jukebox with dual stereo oscilloscope & 32-band peak LED equalizer across 6 procedural tracks.
     - ✅ Yamaha YM2612 2-operator FM Sound Chip Laboratory with clickable piano tiles and harmonic ratio knobs.
     - ✅ 3D Cracktro Workbench with 7 vector geometries (cube, octahedron, star, torus, icosahedron, helix, wavegrid).
     - ✅ CP437 ANSI NFO Generator Studio & downloadable x86 assembly intro source (.asm).
     - ✅ 1999 Scene Top-List voting poll & persistent underground courier shoutbox/guestbook.
     - ✅ Central KiloNet Webring node #013 integration with subtle darknet discovery hooks.
  1. `kweb://geocities` (*CyberSpire's Retro Shrine & MOD Vault*):
     - ✅ 5 GeoCities Neighborhood Themes (SiliconValley, Area51, BeverlyHills, SoHo, EnchantedForest) & live presence badge.
     - ✅ Web Audio 16-bit tracker MIDI jukebox with 4 demoscene/MOD tracks and dancing LED equalizer.
     - ✅ Amiga ProTracker (.MOD) File Dissector & Pattern Matrix Analyzer with 31-sample table & PCM waveform audition.
     - ✅ YM2612 2-Operator FM Synthesizer & Instrument Laboratory with 18-key interactive keyboard, oscilloscope & 8 presets.
     - ✅ Retro Web 1.0 GIF & Banner Studio (468x60 / 88x31 canvas badge generator, PNG download, HTML embed).
     - ✅ Webmaster Acolyte Workbench (1999 GeoCities personal page builder with live Netscape CRT preview & index.html download).
     - ✅ Live Firebase Realtime Database Shoutbox & Cyber Voyagers Presence with quick-stamps and local storage fallback.
     - ✅ 16-color Pixel Art Studio & Gallery with 8 retro sprites, zoom, and PNG/BMP/C-Hex export.
     - ✅ 3D wireframe Silicon Oracle '99 techno-divination & Y2K compliance diagnostic terminal.
  2. `kweb://portal` (*KiloNet Central 1999 Directory*):
     - ✅ KiloSearch 1.0 simulated search engine indexing all 98 KiloApps & webring nodes with live filtering.
     - ✅ Live simulated NASDAQ-1999 stock market ticker banner and $10k interactive portfolio brokerage desk.
     - ✅ Interactive classified ads board with localStorage persistence, posting modal & simulated KMail reply.
     - ✅ Multi-city meteorological station (NY, SF, London, Tokyo, Orbital Station) with live metrics & 3-day forecast.
     - ✅ Daily 1999 retro computing trivia challenge with streak tracking and rank scoring.
     - ✅ Yamaha YM2612 2-operator FM synthesis & SPC700 stereo delay sound effects.
  3. `kweb://webring` (*Central KiloNet Webring Hub & Badge Studio*):
     - ✅ 18-node verified directory with dynamic counters, category filtering, instant search & node inspector modal.
     - ✅ 88x31 Micro Button Studio & Pixel Art Generator (10 archetypes, 11 glyphs, 3D bevels, zoom, PNG/BMP/CSS export, pure client-side 24-bit .BMP file synthesis).
     - ✅ 8 Official HTML Webring Widget Styles (Classic text, 3D Beveled Box, Cyberpunk Neon HUD, 88x31 Button, Marquee Ticker, Netscape 4.7 Select, Lynx CP437 ASCII, Matrix Phosphor).
     - ✅ Interactive Ring Topology Map (880x420 HTML5 Canvas visualizing 18 nodes in closed loop, photon packets, Circular/Hub-Spoke/Radar modes, FM ping sound).
     - ✅ Backbone Traceroute Simulator (5-hop ICMP traceroute terminal across gateway, concentrator, MCI WorldCom backbone & KiloNet transit).
     - ✅ 1999 Baud Rate Bandwidth Benchmark (diagnostic speed matrix across V.32 to T1 leased lines).
     - ✅ Web Voyager Passport & Rank System (dynamic ranks & 5-category postal wax stamp collection book).
     - ✅ Dual Sega Genesis YM2612 FM synthesis tracks ("Hyperlink Voyager '99", "Ringmaster's Cadence") with SNES SPC700 stereo delay & procedural SFX.
     - ✅ Random Hypermedia Teleporter with 3D canvas starfield warp, staged countdown & Webmaster Application / Guestbook.
  4. `kweb://users/~neon_rider` (*Personal Hacker / Demoscene Homepage*):
     - ✅ Interactive 32-bit x86 CPU emulator, instruction sandbox, register stepper with EFLAGS and Pentium cycle counter.
     - ✅ Live Data RAM Hex Dump (0x00402000) with ASCII view, flash memory mutations, and diegetic 10.19.99.4 packet buffer.
     - ✅ Virtual Stack Inspector (0x0012FF80) with visual frame/ESP tracking, plus complete 16x16 Intel x86 Opcode Reference Map (00h-FFh).
     - ✅ Live Mode 13h VGA 320x200 60FPS demoscene canvas (TinyTunnel, Plasma99, FireBuffer, Starfield3D) with 4 authentic retro palettes.
     - ✅ YM2612 2-Operator FM Synthesizer Laboratory with interactive piano keyboard, SPC700 stereo delay & 4-track tracker jukebox.
     - ✅ Demoscene code vault with client-side .asm/.nfo downloads, persistent CGI guestbook, and KiloNet Webring #006 node interconnect.
  5. `kweb://asm-temple` (*x86 Assembly Programming Shrine & PE32 Dissector*):
     - ✅ 133-instruction Opcode Oracle with category filters, Pentium cycle counts, and encoding format deconstruction.
     - ✅ Two-way live x86 assembler & disassembler with preset library, C array / NASM / binary export, and .bin downloads.
     - ✅ Interactive 32-bit micro-CPU single-step emulator (EAX-EIP, flags, cycle counter, virtual stack, SUB/AND/OR/NOT/NEG/SHL/SHR/XCHG/CMP/TEST).
     - ✅ 32-bit interactive radix altar with IEEE-754 single float, ASCII char[4], and EFLAGS status simulation.
     - ✅ Win32 PE32 binary dissector (headers, Shannon entropy heatmaps, IAT imports, entrypoint disasm, hex dumper, RVA tool).
     - ✅ Win32 PE32 binary builder compiling valid downloadable 1.5KB .EXE executables directly in browser memory.
     - ✅ Yamaha YM2612 FM synthesis & SPC700 stereo delay chiptune jukebox (4 tracks) with real-time FM timbre tuner.
     - ✅ Persistent acolyte guestbook & Central KiloNet Webring node #005 interconnect with CyberCafe '99, ~neon_rider & Scene Vault.
  6. `kweb://cybercafe` (*The Underground BBS, ASCII Studio & mIRC Lounge*):
     - ✅ Threaded retro message boards with 4 channels, search, localStorage persistence & ASCII art embedding.
     - ✅ Interactive 60x20 ASCII/ANSI art studio with CP437 glyphs, 16-color palette, .ANS/.TXT export & 1-click forum posting.
     - ✅ Underground IRC terminal client (mIRC style) with 4 channels, slash commands, interactive CafeBot & real-time Firebase RTDB sync.
     - ✅ Procedural Genesis YM2612 2-operator FM synthesis & SNES SPC700 stereo delay audio jukebox with 3 tracks & 14-band LED CRT visualizer.
     - ✅ Terminal booth station telemetry, 56k V.90 throughput benchmark, cafe kiosk with downloadable thermal receipts & hardware vault NFOs.
  7. `kweb://darknet` (*Node 0x7F Transmission Subsystem*):
     - ✅ Tier 3 Ghost Node: VT-100 terminal with virtual spool filesystem, 13-algorithm cryptic decoder suite (Atbash, Morse audio, Binary, XOR), Bell 202 AFSK packet crafting & transmission injector with live responses, dual RF oscilloscope & cascading 2D waterfall spectrogram, downloadable client-side generated assets (.asc, .asm, .conf, .bin, .log) & Central KiloNet Webring #012.
  8. `kweb://10.19.99.4/classified` (*Corporate Network Leak & Signal Diagnostic*):
     - ✅ Signal Diagnostic Lab with dual-display time-domain oscilloscope & FFT frequency spectrum, tunable YM2612 FM / SPC700 stereo delay DSP controls & live subcarrier demodulator.
     - ✅ Subnet RF Sweep: 10.19.99.0/24 node sweep tracking signal-to-noise ratio, carrier lock, and audio DAC leakage.
     - ✅ Corporate Leak Suite: Sanitized diegetic memos, 4-sector memory hex inspector, packet sniffer with test frame injection & skunkworks CLI.
     - ✅ Discovery Integration: Linked node in KNet directory, portal category 5 / search index, portal classified ads, and webring node #015 / probe console.
  9. `kweb://echo-subsystem.net` (*Acoustic Research Lab, SIGINT Grid & Audio Steganography*):
     - ✅ 7-log diegetic acoustic research journal with redaction masks, categorized filters & persistent user observation logbook (.SIG export).
     - ✅ Yamaha YM2612 2-Operator FM synthesis engine with ADSR envelope, SPC700 stereo delay DSP, 14-key keyboard & dual-mode CRT oscilloscope / Lissajous XY phase goniometer.
     - ✅ Real-time 2D FFT waterfall sonogram with 4 false-color palettes (Phosphor, Amber, Cyan, Thermal), peak tracking & live visual steganography rendering.
     - ✅ 3-band parametric filter workbench with interactive live Bode magnitude plot & 1999Hz carrier lock acquisition.
     - ✅ Subsurface Acoustic Transducer Grid & Phased Beamformer with 360° polar radar canvas, 4 listening stations (Carlsbad, Pacific MCI, Cheyenne Mtn, Orbital) & live phased beam audio.
     - ✅ Spectrographic Audio Steganography Studio (visual glyph frequency encoding & .WAV export) + Bell 202 FSK teleprinter (RTTY) transceiver.
     - ✅ VT-100 diagnostic field console and client-side browser synthesis of genuine RIFF WAV, DAT, JSON & SIG files.
     - ✅ Registered as member node #016 in Central KiloNet Webring & linked across KNet portal directory.
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

- **2026-09-30T04:45:00Z — kilo-tester: KGraph (Interactive UI Audit, Calculus Rendering, Quicksave & Audio Polish)**
  - Status: PASS ✅ (6 issues fixed, 0 regressions, 0 perimeter glints, 141.5 KB < 999 KB).
  - Taylor Series Overlay: Fixed element ID mismatch, degree parsing, expansion point, and return object handling in canvas and SVG export.
  - Riemann Sum Method: Corrected method matching for 'midpoint' and 'trapezoid' partition shapes and SVG rendering.
  - Quicksave & State: Synchronized full calculus DOM controls (bounds, partitions, Taylor degree, HUD, mute) across F5/F9 and JSON import/export.
  - Audio Mute & Sound Effects: Added `updateMuteUI()` on init, save/load FM chimes, root/intersection alerts, and preset click feedback.
  - Tangent Line Redraw: Fixed `toggleTangent()` [T] to immediately trigger canvas redraw and tangent slope update.
  - Verification: MSVC clean (`KGraph.exe` 36.8 KB); Vite clean in 431ms; JS syntax verified; security linter 100% PASS; <999KB ceiling.

- **2026-09-30T04:26:00Z — kilo-creator: echo-subsystem.net (Acoustic Research & Signal Intelligence)**
  - Status: PASS ✅ (0 regressions, 0 perimeter glints, 123.5 KB < 999 KB ceiling).
  - Phased Beamformer Grid: Added Tab [05] with 360° polar radar array, 4 listening stations, azimuth steering & live beam audio.
  - Steganography & FSK Teleprinter: Added Tab [06] with visual glyph spectrogram audio synthesis, .WAV export & Bell 202 RTTY teleprinter.
  - Lissajous Goniometer: Added dual-mode CRT oscilloscope toggle plotting Modulator X vs Carrier Y phase vector figures.
  - Persistent Field Logbook: Added interactive observer registry storing local telemetry reports with `.SIG` archive export.
  - Reciprocal Web Links: Verified and updated links across KNet help table, KiloNet Portal directory/search index, and Webring #016.
  - Verification: Clean Vite build in 260ms; security lint 100% PASS; file size 123.5 KB strictly compliant with sacred 999 KB law.

- **2026-09-30T04:05:00Z — kilo-expander: KTask (Sockets I/O, RTDB Fleet Sentinel, Profiler Flamegraph & YM2612 FM Audio)**
  - Status: PASS ✅ (0 regressions, clean builds, 178.9 KB web / 31.2 KB native < 999 KB ceiling).
  - Network Sockets: Added Tab [4] Network Sockets monitoring TCP/UDP endpoints, RX/TX rates, latency, ping probe, and reset connection.
  - Fleet Sentinel: Added Tab [5] with live Firebase RTDB node telemetry broadcast, global fleet radar, ping probes, and solo offline simulation.
  - Profiler & Flamegraph: Added Tab [6] with 3-sec live CPU instruction cycle sampler, IPC/cache analysis, visual Flamegraph, and trace export.
  - Memory Delta & Leak Sentinel: Added [Δ RAM] toggle and [Zero Base] with automatic memory drift analysis and leak warning badges.
  - Genesis YM2612 FM Audio: Added 2-operator FM synth with SPC700 stereo delay warmth and audible alerts; added [S] mute/sound hotkey.
  - Native Alignment: Added Sockets and Profiler telemetry sections to Win32 C inspector; verified clean MSVC build (`KTask.exe` 31.2 KB).
  - Verification: MSVC clean; Vite clean in 359ms; security lint 100% clean; fleet icon checks PASS.

- **2026-09-30T03:50:00Z — kilo-qa: KColosseum (Pass 5: Tutorial & State Integrity, Quicksave/Load, Win32 I/O)**
  - Status: PASS ✅ (0 regressions, 0 perimeter glints, clean builds, 126.5 KB web / 32.5 KB native < 999 KB).
  - Quicksave & State: Added F5 snapshot quicksave and F9 quickload capturing active arena combat state, bosses, crowd favor, and ludus roster in web and native (`kcolosseum_quicksave.dat`).
  - Tutorial Safeguard: Enforced first-run tutorial only on uninitialized sessions (`kcolosseum_tutorialSeen` / `.dat`), never interrupting restored saves.
  - Toast & Modals: Relocated toast container to bottom-left with reverse stacking and `clearToasts()` on modal open/close, preventing top-nav button occlusion.
  - Audio Engine Mute: Added persistent audio mute controls (`soundToggleBtn`, `[M]` hotkey) across web and Win32 C.
  - Native Hotkeys: Added F5, F9, F1, H, M, Esc, R, 1-3, Space, and combat action shortcuts to Win32 message loop.
  - Verification: MSVC clean (`KColosseum.exe` 32.5 KB); Vite clean in 388ms; JS syntax verified; security linter & icon checks 100% PASS.

- **2026-09-30T03:35:00Z — kilo-graphics: KStarForge (YM2612 FM Audio, Modular Engineering, Boss Waves & Zero Glints)**
  - Status: PASS ✅ (0 regressions, 0 perimeter glints, clean builds, 201.1 KB web / 30.2 KB native < 999 KB).
  - Audio Synthesis: Integrated Yamaha YM2612 2-op FM and SNES SPC700 stereo delay DSP; added [M] mute toggle.
  - Engineering Depth: Added Point-Defense Flak, Subspace Booster, Capacitor Bank, and Tractor Beam modules.
  - Ship Archetypes: Added Sol Invictus Dreadnought & Project 1999 Echo templates; added [T] archetype cycling in native.
  - Proving Grounds Polish: Added wave 3 boss flagships, salvage recovery physics, tractor beam, and tactical radar HUD.
  - ARG Lore & Alignment: Implemented diegetic 1999Hz carrier lock & subnet 10.19.99.4 references; native binary synced.
  - Verification: MSVC clean (`KStarForge.exe` 30.2 KB); Vite clean in 290ms; security lint 100% clean; fleet icons valid.
