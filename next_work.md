---
current_agent: kilo-graphics
next_agent: kilo-tester
agent_rotation:
  - kilo-qa
  - kilo-expander
  - kilo-creator
  - kilo-graphics
  - kilo-tester
  - kilo-usability
model: gemini-3.8-flash-high
timeout_minutes: 15
status: ready
current_targets:
  kilo_tester: KMech
  kilo_usability: KHash
  kilo_graphics: KAbyss
  kilo_qa: KMine
  kilo_expander: KNote
  kilo_creator: "kweb://asm-temple (Win32 ASM Shrine & Opcode Converter)"
virtual_web_target: "kweb://asm-temple"
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
  agent: kilo-creator
  app: "kweb://users/~neon_rider (Personal Hacker / Demoscene Homepage)"
  timestamp: "2026-10-01T02:45:00Z"
last_planner_run: "2026-09-30T18:32:00Z"
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
12. **Seamless Online Multiplayer via Firebase (DIRECTOR MANDATE - CRITICAL)**:
    - `kilo-creator` and `kilo-expander` must concentrate on adding online multiplayer features that work seamlessly through Firebase Realtime Database (`https://kiloappschat-default-rtdb.firebaseio.com`) using the standardized RFMS service [`KiloOS/public/assets/js/retro_multiplayer.js`](KiloOS/public/assets/js/retro_multiplayer.js) (spec: [`docs/RFMS_SPEC.md`](docs/RFMS_SPEC.md)).
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

---

## Active Target Queues

### 1. Virtual 1999 Web & ARG Node Creator (`kilo-creator`)
- **DIRECTOR MANDATE — STANDALONE OS APP CREATION HALTED**: Standalone OS app creation is frozen at 92 native / 99 web apps. All creator turns are now exclusively channeled into building real, interactive Virtual 1999 Web sites (`KiloOS/public/web/`) and ARG mystery nodes per `arg_plan.md`. Zero shallow stubs; every page must be a functioning Web 1.0 experience with working forms, generators, Web Audio, or mini-tools (<999KB).
- **Current Target**: `kweb://asm-temple` (Win32 ASM Shrine & Opcode Converter)
- **Upcoming Queue**:
  `kweb://cybercafe` (Underground BBS Lounge), `kweb://10.19.99.4/classified` (Corporate Leak Intranet), `kweb://echo-subsystem.net` (Research Journal)
  *(Completed: kweb://geocities, kweb://portal, kweb://cybercafe, kweb://asm-temple, kweb://users/~neon_rider, kweb://darknet, kweb://deep-core, kweb://echo-subsystem.net, kweb://10.19.99.4/classified, kweb://webring, kweb://warez)*.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Current Target**: `KAbyss`
- **Upcoming Queue**:
  `KColosseum`, `KWizard`, `KStarship`, `KChrono`, `KFortress`, `KCosmic`, `KSanctuary`, `KDragon`, `KStarDredge` *(Completed: KMystery, KMech, KColosseum, KAbyss, KWizard, KStarship, KChrono, KStarForge, KFortress, KCosmic, KStellar, KSanctuary, KDragon, KSubmarine, KStarDredge)*.

### 3. App Tester Queue (`kilo-tester`)
- **Current Target**: `KMech`
- **Upcoming Queue**:
  `KMedia`, `KMystery`, `KNet`, `KNote`, `KPad`, `KPaint`, `KPass`, `KPing`, `KQuest`, `KRadio`, `KRead`, `KSanctuary`, `KScript`, `KStarDredge`, `KStarship`, `KStellar`, `KSubmarine`, `KSynth`, `KSys`, `KChrono`, `KTask`, `KStarForge`, `KTerm`, `KHash`, `KRSS`, `KClip`, `KCipher`, `KPomodoro`, `KCalc`, `KHangman`, `KTodo`, `KTrader`, `KType`, `KVault`, `KVoid`, `KWizard`, `KZip`, `KAbyss`, `KAudio`, `KBBS`, `KBase`, `KBudget`, `KCalendar`, `KChart`, `KChat`, `KColosseum`, `KContacts`, `KCosmic` *(Completed: KCyber, KCosmic, KContacts, KDB, KDragon, KFlash, KFont, KFortress, KGraph, KHabit, KHex, KImage, KJournal, KMail, KMandel)*.

### 4. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KHash`
- **Upcoming Queue**:
  `KRSS`, `KClip`, `KCalc`, `KHex`, `KContacts`, `KFarm`, `KPaint`, `KAudio`, `KFont`, `KGraph`, `KImage`, `KJournal`, `KMail`, `KMandel`, `KMedia`, `KMystery`, `KNet`, `KNote`, `KPass`, `KPing` *(Completed: KSynth, KScript, KRead, KRadio, KSys, KTodo, KTrader, KType, KVault, KVoid, KWizard, KZip, KChrono, KTask, KStarForge, KPad, KBookmark)*.

### 5. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KMine`
- **Upcoming Queue**:
  `KCosmic`, `KContacts`, `KPad`, `KPaint` *(Completed in Pass 5: KBBS, KChrono, KCipher, KClip, KCyber, KDragon, KFortress, KHash, KMaze, KMech, KMystery, KQuest, KSanctuary, KSnake, KSolitaire, KSpace, KStarDredge, KStarship, KStellar, KSubmarine, KSynth, KSys, KTask, KTerm, KTimer, KTodo, KTrader, KType, KVault, KVoid, KWizard, KZip, KRSS, K2048, KChart, KGraph, KContacts, KScript, KRead, KColosseum, KAbyss, KMedia, KAudio, KRadio, KPad, KPaint, KCalc)*.

### 6. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KNote`
- **Upcoming Queue**:
  `KContacts`, `KPass`, `KVault` *(Completed: KFont, KRSS, KSys, KZip, KVault, KType, KMandel, KGraph, KChart, KPaint, KConnect4, KChess, KGo, KReversi, KDarts, KTetris, KSnake, KDB, KTodo, KJournal, KCalendar, KContacts, KMail, KRead, KPass, KImage, KAudio, KSynth, KMedia, KTask, KNet, KPing, KHash, KPad)*.
- **Multiplayer Focus (CRITICAL)**: Concentrate on expanding games (*KChess*, *KConnect4*, *KGo*, *KReversi*, *KDarts*, *KTetris*, *KSnake*) and collaborative apps (*KDraw*, *KPaint*, *KSynth*, *KPad*) with seamless Firebase Realtime Database multiplayer for cross-computer play on `kiloapps.web.app`.

### 7. Virtual 1999 Web Expansion Queue (Eternal Fleet Track)
- **Current Active Target**: `kweb://asm-temple` (`KiloOS/public/web/asm_temple.html`)
  - *Next in Rotation*: `kweb://cybercafe` ➔ `kweb://10.19.99.4/classified` ➔ `kweb://echo-subsystem.net` ➔ `kweb://deep-core` ➔ `kweb://darknet` ➔ `kweb://portal` ➔ `kweb://webring` ➔ `kweb://warez` ➔ `kweb://geocities` ➔ `kweb://users/~neon_rider`.
- **Anti-Potemkin Directive & Content Mandates**:
  0. `kweb://warez` (*0xRELEASE Scene Vault & x86 Reverse Engineering Lab*):
     - ✅ 12 parody releases with 3D vector cracktro launcher, custom NFO viewer & .diz/.nfo downloads.
     - ✅ x86 Reverse Engineering Sandbox: SoftICE '99 simulator with disassembler, registers, NOP/invert patching & PE32 binary builder.
     - ✅ Chiptune Jukebox: 6 tracks, time-domain oscilloscope, 32-band peak LED equalizer & live 4-channel Tracker Pattern visualizer with mute/solo.
     - ✅ Yamaha YM2612 FM Sound Chip Laboratory: interactive piano tiles, ADSR envelope & harmonic ratio knobs.
     - ✅ 3D Cracktro Workbench: 7 vector geometries with custom text scroller, copper raster bars & downloadable NASM source.
     - ✅ CP437 ANSI Studio: 6 scene group presets, CP437 character insertion palette & 1999Hz subcarrier intercept injector.
     - ✅ 1999 Scene Top-List & Demoscene Trivia Challenge: persistent voting polls, 10-question challenge & credential certificate.
     - ✅ Live Firebase RTDB Scene Shoutbox: live courier presence, dead-drop keyword daemon (`Ghost_SysOp_0x7F`) & local fallback.
  1. `kweb://geocities` (*CyberSpire's Retro Shrine & MOD Vault*):
     - ✅ 5 GeoCities Neighborhood Themes (SiliconValley, Area51, BeverlyHills, SoHo, EnchantedForest) & live presence badge.
     - ✅ Web Audio 16-bit tracker MIDI jukebox with 4 demoscene/MOD tracks and dancing LED equalizer.
     - ✅ Amiga ProTracker (.MOD) File Dissector & Pattern Matrix Analyzer with 31-sample table & PCM waveform audition.
     - ✅ YM2612 2-Operator FM Synthesizer & Instrument Laboratory with 18-key interactive keyboard, oscilloscope & 8 presets.
     - ✅ Demoscene Real-Time Visual FX Laboratory (Amiga Copper sine bars with scroller, 256-color sine plasma, Doom fire, 3D warp starfield, phosphor rain, PNG snapshot).
     - ✅ 8-bit Amiga PCM Chip-Sample Sculptor & Audio Waveform Lab (interactive canvas drawing, 8 presets, normalize, 4-bit crush, smooth, reverse, loop points, C-2..C-5 pitches, RIFF/WAV & C array export).
     - ✅ 1999 Cyber Voyagers Web Survey & Millennial Poll (3 interactive questions with animated progress bars, localStorage persistence, and live Firebase RTDB sync).
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
     - ✅ Interactive 32-bit x86 CPU emulator, opcode assembler/stepper, and interactive RAM Hex Memory Editor with live byte patching & 10.19.99.4 packet injection.
     - ✅ Real-time Mode 13h VGA 60FPS Demoscene Canvas: 3D vector mesh engine (Cube, Tesseract, Torus, Octahedron, Star; wireframe & flat Lambertian), Voxel Land '99 Comanche raycaster & 1KB cracktro intro with 8x8 font text scroller.
     - ✅ Yamaha YM2612 FM Synthesis & SPC700 tracker jukebox (6 tracks), interactive piano keyboard & 16-step tracker sequencer.
     - ✅ Pentium II 450MHz synthetic silicon benchmark, 8 x86 optimization articles, and 11-file ASM vault (.asm/.hex downloads).
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
  7. `kweb://darknet` (*Node 0x7F Subterranean Relay & Warez NFO Cryptography*):
     - ✅ Tier 3 Ghost Node: VT-100 terminal shell with 17 directives & virtual spool filesystem (`routes.conf`, `transponder.log`, `precursor_cipher.nfo`, `fleet_heartbeat.hex`, `hardware.cfg`).
     - ✅ Warez NFO Steganography Lab: CP437 ANSI viewer, hex dumper, live steganography scanner (trailing whitespace / XOR-0x7F) & custom NFO injector with 5 scene releases (*Surreal Tournament '99 [FLT]*, *Tremor III Arena [RZR]*, *Half-Cycle 1.1 [PDX]*, *Machina Ex Preview [SKD]*, *Carlsbad Bedrock Relay*).
     - ✅ CRC32 & MD5 hash calculator with anomaly detection matching subterranean ARG relay seeds.
     - ✅ 14-algorithm packet decoder suite (Hex, XOR, Rot13, Base64, Polybius, Atbash, CW Morse audio, Whitespace Stego) with Shannon entropy.
     - ✅ Subnet 10.19.99.0/24 packet monitor & Bell 202 AFSK frame crafting/injector with destination node replies.
     - ✅ Dual 60FPS RF oscilloscope & cascading 2D waterfall spectrogram with 4 phosphor palettes, 144.39MHz / 1999Hz tuner & S-meter.
     - ✅ Gated Middle-Game Puzzle Relay (Sector 0x7F) validating sequential cross-node artifacts (acoustic carrier, sector 03, warez checksum) and converging on Deep Core (10.19.99.127).
     - ✅ Collaborative Subterranean Signal Mesh via Firebase RTDB (`arg/signals/subterranean_darknet`) with instant Carlsbad salt-vault solo loopback fallback.
     - ✅ Universal procedural audio engine: Genesis YM2612 2-op FM synthesis + SNES SPC700 stereo delay across 3 chiptune tracks + procedural SFX.
     - ✅ Client-side asset synthesis & download (.asc, .asm, .nfo, .rom, .conf, .log) & Central KiloNet Webring #012 interconnect.
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
  10. `kweb://deep-core` (*Ghost Node Terminal & Passkey Analyzer*):
      - ✅ Multi-mode CRT visualizer: 3 display modes (Time-Domain Wave, 2D Phosphor Waterfall Spectrogram, Lissajous XY Phase Goniometer).
      - ✅ 5-sector quarantine defusal workbench (MEM_HEAP, AUDIO_DSP, NET_RELAY, STORAGE_VFS, CORE_AI) with dynamic parity scoring (0% to 100%).
      - ✅ Subterranean ghost spool vault (/core/spool/) with 5 diegetic files and client-side download synthesis (.log, .rules, .json, .sig, .nfo).
      - ✅ Subterranean raw AFSK/TCP diagnostic packet injector transmitting frames to 10.19.99.1, 10.19.99.4, 10.19.99.19, 10.19.99.127.
      - ✅ Procedural Sega Genesis YM2612 2-op FM chiptune jukebox with SNES SPC700 stereo delay DSP across 3 ambient vault tracks.
      - ✅ Cryptographic tools (SHA-256, CRC32, Shannon entropy, bitwise XOR, memory decode) with F5/F9 state persistence.
      - ✅ Registered as node #017 in Central KiloNet Webring, linked in KNet browser & KiloNet Portal directory.
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

- **2026-10-01T02:45:00Z — kilo-creator: kweb://users/~neon_rider (Demoscene Homepage Deep Expansion)**
  - 3D Vector Engine: Built software rasterizer (Cube, 4D Tesseract, Torus, Octahedron, Star) with wireframe, Lambertian flat shading & depth buffer.
  - Voxel Land '99: Added Comanche column height-raycasting engine with rolling canyons, alpine glaciers, Martian terrain & altitude flight slider.
  - 1KB Demoscene Cracktro: Built composite intro with copper raster splits, 3D star, and real-time 8x8 bitmap font text scroller with live text input.
  - Interactive RAM Hex Editor: Added live byte inspector/patcher, 10.19.99.4 packet injector (1999Hz tone), XOR 0x7F mask & .HEX export.
  - Audio & Vault: Added Tracks 05 & 06 to FM Jukebox; added Articles 07 & 08 to devlog; added `mesh3d.asm`, `voxel_land.asm`, `intro1k.asm` downloads.
  - Verification: Security linter 100% PASS; `npm run build` clean in 311ms; `neon_rider.html` 203.7 KB (<999 KB ceiling).

- **2026-10-01T02:30:00Z — kilo-expander: KPad (Real-Time Collab Suite, RFMS, Document Diff & Hashes)**
  - RFMS Multiplayer: Integrated `retro_multiplayer.js` with room matchmaking (`KPD-XXXX`), URL sharing (`#room=`), live presence & remote cursor sync.
  - Mandate 12 Solo Fallback: 25s auto-fallback to diegetic "Ghost Typist" local AI copilot with in-session chat & code assistance.
  - Document Diff Suite: Built line-by-line Myers/LCS visual document comparator across tabs, snapshots, and clipboard.
  - Text Transforms: Added Title Case, ROT13 cipher, camelCase, snake_case, kebab-case, and invert casing in web and native.
  - Hashes & Checksums: Implemented instant MD5, SHA-256, and CRC-32 integrity calculators with Win32 CryptoAPI parity.
  - Templates & Exports: Added retro HTML 4.01 and x86 ASM templates, and BBCode, RTF, and LaTeX export generation.
  - Verification: MSVC clean (`KPad.exe` 33 KB); Vite clean in 281ms; security lint & check_icons 100% PASS (<999KB ceiling).

- **2026-10-01T02:15:00Z — kilo-qa: KCalc (Pass 5: Tutorial & State Integrity, Quicksave/Load, Toast Safe Zone)**
  - State Persistence: Implemented F5 / F9 full workspace snapshot save/load in web and native (`kcalc_quicksave.dat`).
  - Extended Snapshot: Expanded snapshot to capture all financial form inputs and descriptive/linear statistics datasets.
  - Tutorial Integrity: Added native first-run modal (`kcalc_tutorial.dat`) and wired web dismissal to prevent popup on restored state.
  - Toast Occlusion Remediation: Relocated toast notifications to bottom-center safe zone, clearing all keypad and formula inputs.
  - Keyboard & Modal Audit: Verified Esc/Enter dismissals, mode shortcuts (1-5), and updated shortcuts documentation in help dialog.
  - Verification: MSVC clean (`KCalc.exe` 28.7 KB); Vite clean in 351ms; security lint & check_icons 100% PASS (<999KB ceiling).

- **2026-10-01T02:00:00Z — kilo-usability: KBookmark (UX & Usability Ergonomics Pass)**
  - Protocol Launching: Integrated intelligent scheme dispatcher resolving `internal:<app>` and `kweb://<site>` in web & native.
  - Card URL Links: Converted raw anchor navigations into safe in-app launches preventing broken browser scheme errors.
  - Category Ergonomics: Added [1]-[8] hotkey badges and tooltips to category items for fast keyboard switching.
  - Drag & Drop Import: Added visual drop overlay & drag-and-drop file ingestion for Netscape HTML and JSON vaults.
  - Onboarding & Modals: Added startup checkbox to splash screen, synced tutorial flags, and added status pulse feedback.
  - Lore & TINAG: Replaced ARG Secrets with diegetic Subcarrier Relays in native C and web HTML.
  - Verification: MSVC clean (`KBookmark.exe` 22.5 KB); Vite clean in 289ms; security lint 100% PASS; check_icons PASS.

- **2026-10-01T01:45:00Z — kilo-tester: KMandel (Interactive UI Audit & Inline Repairs)**
  - Status: PASS ✅ (6 issues, 6 fixed, 0 perimeter glints, 121.9 KB < 999 KB ceiling).
  - Navigation & History: Replaced shadowed history array with viewHistory stack, fixing coordinate URL link sharing and clipboard write.
  - Modal Dismissals: Added backdrop click dismissal to import dialog; fixed Escape key to close modal even when textarea is focused.
  - State & Bookmarks: Handled prompt cancel gracefully in bookmarking; added array guard to bookmarks storage; fixed Perp Ship bounds.
  - Audio Persistence: Wired startup restoration of saved audio mute state across localStorage and UI controls.
  - Peer Teleportation: Enhanced peer discovery beacon toast handler to dismiss toast notification automatically upon coordinate jump.
  - Verification: `npm run build` clean in 411ms; security lint 100% PASS; check_icons PASS; zero exceptions.
