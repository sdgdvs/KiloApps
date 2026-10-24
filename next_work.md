---
current_agent: kilo-review
next_agent: kilo-creator
agent_rotation:
- kilo-creator
- kilo-graphics
- kilo-soundscape
- kilo-tester
- kilo-usability
- kilo-qa
- kilo-expander
- kilo-review
model: gemini-3.8-flash-high
agent_models:
  kilo-review: gemini-4-argon-high
effort: selective
timeout_minutes: 8
agent_timeouts:
  kilo-creator: 9
  kilo-expander: 9
  kilo-soundscape: 9
  kilo-graphics: 8
  kilo-tester: 8
  kilo-usability: 8
  kilo-qa: 8
  kilo-review: 8
  kilo-planner: 8
tool_call_cap: 25
commit_checkpoint: 12
status: ready
current_targets:
  kilo_creator: kweb://deep-core
  kilo_graphics: KStellar
  kilo_soundscape: KVoid
  kilo_tester: KFlash
  kilo_usability: KScript
  kilo_qa: KFlash
  kilo_expander: KAnomaly
  kilo_review: KFarm
virtual_web_target: kweb://deep-core
virtual_web_rotation:
- kweb://slashnet
- kweb://trapster
- kweb://cabled
- kweb://scallion
- kweb://demoscene
- kweb://rfc-archive
- kweb://webring
- kweb://warez
- kweb://geocities
- kweb://users/~neon_rider
- kweb://asm-temple
- kweb://cybercafe
- kweb://10.19.99.4/classified
- kweb://echo-subsystem.net
- kweb://deep-core
- kweb://darknet
- kweb://portal
last_run:
  agent: kilo-expander
  app: KStarForge
  timestamp: '2026-10-24T12:50:28-07:00'
last_planner_run: '2026-10-24T07:31:25Z'
---

# KiloApps Master Fleet Work & Queue State (`Phase VI: System Convergence`)

This document is the single active source of truth for autonomous agent dispatching.
The Windows Task Scheduler orchestrator (`scripts/orchestrate.py`) parses the YAML frontmatter and Mission Pipeline Tables below on every tick.

## Strategic Direction — Phase VI: System Convergence, Cross-App VFS Mesh & The 1999 Web
1. **Pillar 1 — KiloOS VFS & Inter-App IPC Mesh**: Transform KiloApps from isolated `localStorage` silos into an interconnected 1999 Operating System. Wire `VFS_READ`, `VFS_WRITE`, `VFS_SHOW_OPEN`, `VFS_SHOW_SAVE`, `OS_NOTIFY`, and `OS_LAUNCH_APP` (`postMessage` to KiloOS shell with graceful standalone fallback) so files saved in `KBBS`, `KNet`, `KCipher`, `KScript`, `KDB`, `KSynth`, or `KStarForge` can be opened and inspected across `File Browser` (`kexplorer`), `KHex`, `KPad`, `KGraph`, and `KMatrix`.
2. **Pillar 2 — Deep Sci-Fi/Fantasy Multiplayer (RFMS) & Graduation**: Retrofit the 17 non-mature deep sci-fi/fantasy games and simulations with `retro_multiplayer.js` (strictly behind an explicit `"Multiplayer / Connect"` button, zero autostart, `soloTimeoutMs: 25000` AI fallback), complete 6-channel YM2612/SPC700 soundscapes, and graduate verified apps into `## Mature Applications`.
3. **Pillar 3 — Canonical 1999 Virtual Web Expansion (`kweb://...`)**: Build out the canonical alternate-1999 web destinations from `arg_plan.md` §7 (`kweb://slashnet`, `kweb://trapster`, `kweb://cabled`, `kweb://scallion`, `kweb://demoscene`, `kweb://rfc-archive`) as self-contained `< 350 KB` interactive Web 1.0 cartridges and enforce strict Tier 3 darknet secrecy in `KNet`.

## Fleet Core Directives & Fast Turn Protocol
1. **Single-App-Per-Turn & Tool Budget (`<= 25` calls, Commit Checkpoint `~12` calls)**: Process ONE target per turn. Once substantive edits are complete (or at ~12 tool calls), wrap up immediately!
2. **One-Command Verification (`scripts/verify_turn.py`)**: Run `python scripts/verify_turn.py --app <Target>` (add `--build-native` if C source modified) to validate `< 999 KB` byte budgets, native compilation, `test_app_startup.py`, Vite build, and `security_lint.py` in a single tool call.
3. **One-Command Queue Handoff & Logging (`scripts/advance_turn.py`)**: Run `python scripts/advance_turn.py --agent <agent> --app <Target> --title "<Summary>" --bullet "<Status>" --bullet "<Change 1>" [--quarantine-mature "<Summary>" --category "<Category>"]` to deterministically advance queues, rotate agents, update logs, and archive excess entries at 0 token cost.
4. **Invariants**: Strict `< 999 KB` file ceiling; zero multiplayer autostart; in-universe trademark parodies only (`security_lint.py`); TINAG mystery preservation (no `(ARG)` labels or `ECHO-1999-ARCHITECT` leaks); zero traveling perimeter border glints.

---

## Active Target Queues (Multi-Turn Mission Pipelines)

### 1. Virtual 1999 Web & ARG Node Creator (`kilo-creator`)
- **Active Target**: `kweb://deep-core`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `kweb://deep-core` | `KiloOS/public/web/deep_core.html` | Connect Tier 3 Ghost Node terminal to KiloOS VFS (`/sys/alpha.sys` & `/home/crypto/`) so decoded fragments can be written directly to VFS for `KMatrix` verification. |
| `kweb://slashnet` | `KiloOS/public/web/slashnet.html`, `KiloOS/public/apps/knet.html` | Create *SlashNet: News for Nerds (1999)* (`slashnet.html` <250 KB): interactive Karma moderation (`-1 Troll` to `+5 Insightful`), threshold comment filter (`-1` to `+5`), Beowulf cluster calculator, Linux 2.2 kernel compile mini-sim, VFS article export, and route `kweb://slashnet` in `knet.html` / `portal.html`. |
| `kweb://trapster` | `KiloOS/public/web/trapster.html`, `KiloOS/public/apps/knet.html` | Create *Trapster P2P Chiptune & Tracker Exchange v1.0b* (`trapster.html` <250 KB): searchable 1999 P2P node directory, simulated 56k/T1 download queue, live YM2612/SPC700 track previewer, VFS `.mod`/`.trk` save to `/home/music/`, and route `kweb://trapster` in `knet.html`. |
| `kweb://cabled` | `KiloOS/public/web/cabled.html`, `KiloOS/public/apps/knet.html` | Create *Cabled Magazine 1999 Neon Edition* (`cabled.html` <250 KB): late-90s cyber-magazine with interactive Y2K Readiness Audit, Dot-Com IPO Bubble Stock Simulator, Jargon Watch generator, Hype-vs-Reality matrix, and route `kweb://cabled` in `knet.html`. |
| `kweb://scallion` | `KiloOS/public/web/scallion.html`, `KiloOS/public/apps/knet.html` | Create *The Scallion: 1999 Satirical News* (`scallion.html` <250 KB): interactive late-90s tech/culture satire portal, "Ask A Sysadmin" interactive column, Infographic & Headline generator, VFS clipping export, and route `kweb://scallion` in `knet.html`. |
| `kweb://demoscene` | `KiloOS/public/web/demoscene.html`, `KiloOS/public/apps/knet.html` | Create *Assembly '99 Demoscene Party Vault* (`demoscene.html` <250 KB): real-time 64K intro Canvas effect switcher (plasma, rotozoomer, 3D starfield, copper bars, metaballs), ANSI `.NFO` viewer with VFS save, and party voting board. |
| `kweb://rfc-archive` | `KiloOS/public/web/rfc_archive.html`, `KiloOS/public/apps/knet.html` | Create *IETF 1999 Living RFC Workbench* (`rfc_archive.html` <250 KB): interactive RFC 1149 (IP over Avian Carriers) packet loss simulator, RFC 2324 (`418 I'm a teapot`) HTCPCP console, ASCII TCP state stepper, and VFS `.txt` export. |
| `kweb://webring` | `KiloOS/public/web/webring.html` | Integrate new canonical nodes (`slashnet`, `trapster`, `cabled`, `scallion`, `demoscene`, `rfc-archive`) into RingMaster topology map, badge generator, and random/next ring traversal. |

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Active Target**: `KStellar`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KStellar` | `KiloOS/public/apps/kstellar.html` | 4X stellar empire map visual upgrade: star spectral class rendering, fleet ship icons, orbital ringworlds, and glint sweep. |
| `KSanctuary` | `KiloOS/public/apps/ksanctuary.html` | Xenobiology sanctuary habitat visuals: biome terrarium flora/fauna sprites, weather overlay polish, and glint sweep. |
| `KSubmarine` | `KiloOS/public/apps/ksubmarine.html` | Bathysphere trench exploration visuals: bioluminescent leviathan silhouettes, hydrothermal vent plumes, sonar HUD polish, and glint sweep. |
| `KWizard` | `KiloOS/public/apps/kwizard.html` | Arcane wizard duel visuals: elemental spell particle effects, grimoire rune illustrations, wizard tower backdrop, and glint sweep. |
| `KStarDredge` | `KiloOS/public/apps/kstardredge.html` | Deep space salvager visuals: derelict hull wreckage sprites, magnetic dredge beam effects, cosmic anomaly silhouettes, and glint sweep. |
| `KBreakout` | `KiloOS/public/apps/kbreakout.html` | Arcade brick-breaker visual upgrade: textured metallic/crystalline bricks, plasma paddle/ball trails, boss hazard visuals, and zero perimeter border dots. |
| `KStarship` | `KiloOS/public/apps/kstarship.html` | Starship bridge & sector tactical display upgrade: detailed hull subsystem schematics, nebula parallax backdrop, enemy cruiser silhouettes, and glint sweep. |
| `KAlchemy` | `KiloOS/public/apps/kalchemy.html` | Alchemical laboratory & crucible visual overhaul: ornate alembic glassware, elemental reagent icons, transmutation reaction glow, and glint sweep. |
| `KVoid` | `KiloOS/public/apps/kvoid.html` | Deep void survival visual upgrade: gravitational singularity lensing, derelict hull debris sprites, anomaly silhouettes, and glint sweep. |
| `KCyber` | `KiloOS/public/apps/kcyber.html` | Netrunner icebreaker deck visual polish: isometric node towers, IC guardian constructs, matrix grid perspective, and glint sweep. |

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Active Target**: `KVoid`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KVoid` | `KiloOS/public/apps/kvoid.html` | Compose dark cosmic horror / event-horizon 6-channel suite (Whole-tone / Phrygian clusters, sub-bass drone, cavernous 260ms SPC700 delay, Geiger/hull-stress earcons). |
| `KCyber` | `KiloOS/public/apps/kcyber.html` | Compose 1999 cyberpunk netrunner breakbeat 6-channel FM suite (Dorian acid FM bass, syncopated 5:3 breakbeat drums, cold digital bell arpeggios, ICE breach alarms). |
| `KStellar` | `KiloOS/public/apps/kstellar.html` | Compose grand 4X galactic diplomacy & fleet command 6-channel suite (Lydian brass fanfare, floating Maj9 pad voicings, hyperdrive & treaty earcons). |
| `KSubmarine` | `KiloOS/public/apps/ksubmarine.html` | Compose abyssal sonar & trench descent 6-channel suite (Suspended `sus2/Maj7` cascade, muffled low-pass SPC700 echo, warm sine sub-bass, hydrophone ping earcons). |
| `KBreakout` | `KiloOS/public/apps/kbreakout.html` | Compose high-energy 6-channel YM2612 FM arcade synthwave theme (Ionian/Mixolydian, 5:3 swing groove, slap bass, brass lead, selective SPC700 delay) + harmonic brick-hit combo pitch escalator and powerup earcons. |
| `KQuest` | `KiloOS/public/apps/kquest.html` | Compose overworld & battle 6-channel JRPG chiptune suite (Royal Road `IV-V-iii-vi` town/overworld + Heroic `bVI-bVII-I` combat cadence, E-Piano & flute counterpoint, SPC700 echo) + spell/quest earcons. |
| `KStarship` | `KiloOS/public/apps/kstarship.html` | Compose bridge alert & deep-space exploration 6-channel FM suite (Lydian `#4` pad + tactical Phrygian Dominant combat march, sonar/shield/warp earcons). |
| `KAlchemy` | `KiloOS/public/apps/kalchemy.html` | Compose mystical alchemical workshop 6-channel suite (Dorian/Lydian celesta bells, warm glass pad, bubbling FM cauldron transients, transmutation fanfare stingers). |
| `KWizard` | `KiloOS/public/apps/kwizard.html` | Compose arcane tower duel 6-channel chiptune suite (Harmonic Minor harpsichord/flute counterpoint, walking FM bass, spell-cast elemental earcons). |

### 4. App Tester Queue (`kilo-tester`)
- **Active Target**: `KFlash`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KFlash` | `KiloOS/public/apps/kflash.html`, `KFlash/main.c` | Audit flashcard deck manager, spaced repetition algorithm, flip animations, and deck import/export. |
| `KSys` | `KiloOS/public/apps/ksys.html`, `KSys/main.c` | Audit system telemetry dashboard, process/memory graphs, diagnostic stress test toggles, and keyboard navigation. |
| `KScript` | `KiloOS/public/apps/kscript.html`, `KScript/main.c` | Audit script IDE, AST/console runner, sample templates, breakpoint stepper, and VFS script save/load. |
| `KSteno` | `KiloOS/public/apps/ksteno.html`, `KSteno/main.c` | Audit LSB image/audio steganography encoder/decoder, bitplane visualizer, passkey encryption, and modal dismissals. |

### 5. Usability & UX Queue (`kilo-usability`)
- **Active Target**: `KScript`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KScript` | `KiloOS/public/apps/kscript.html`, `KScript/main.c` | Split editor/output pane resizing, line-number gutter alignment, `Ctrl+Enter` run shortcut pill, and accessible focus rings. |
| `KSys` | `KiloOS/public/apps/ksys.html`, `KSys/main.c` | HiDPI telemetry oscilloscopes (`devicePixelRatio`), responsive grid cards at `<640px`, and keyboard tab switching (`[1-5]`). |
| `KSteno` | `KiloOS/public/apps/ksteno.html`, `KSteno/main.c` | Drag-and-drop / VFS carrier image picker UX, clear capacity progress bar, HiDPI bitplane canvas scaling, and accessible modal focus traps. |
| `KAnomaly` | `KiloOS/public/apps/kanomaly.html`, `KAnomaly/main.c` | Subterranean waterfall spectrogram HiDPI scaling, frequency tuner wheel/keyboard precision (`ArrowLeft/Right`), and clear decoder telemetry readout. |
| `KFlash` | `KiloOS/public/apps/kflash.html`, `KFlash/main.c` | Large card flip touch/keyboard ergonomics (`Space` to flip, `1-4` for ratings), responsive progress bar, and deck picker drawer. |

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial, State & Mature Graduation)
- **Active Target**: `KFlash`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KFlash` | `KiloOS/public/apps/kflash.html`, `KFlash/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, Leitner box scheduling math, deck import integrity, dual Win32 C parity, and promote to Mature table. |
| `KTodo` | `KiloOS/public/apps/ktodo.html`, `KTodo/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, tag filtering, first-run tutorial tips, dual Win32 C parity, and promote to Mature table. |
| `KTimer` | `KiloOS/public/apps/ktimer.html`, `KTimer/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, lap export verification, sound alert toggle, dual Win32 C parity, and promote to Mature table. |
| `KClock` | `KiloOS/public/apps/kclock.html`, `KClock/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, world time offsets, alarm trigger verification, dual Win32 C parity, and promote to Mature table. |

### 7. Feature Expander Queue (`kilo-expander` — Phase VI VFS Mesh & RFMS Multiplayer)
- **Active Target**: `KAnomaly`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KAnomaly` | `KiloOS/public/apps/kanomaly.html`, `KAnomaly/main.c` | Phase VI VFS Signal Bridge: allow exporting demodulated subterranean signal frames directly to `/sys/anomaly_capture.sig` in KiloOS VFS and launching `KMatrix` or `KHex` via `OS_LAUNCH_APP` for cross-app decoding. |
| `KSynth` | `KiloOS/public/apps/ksynth.html`, `KSynth/main.c` | Phase VI VFS Audio Patch Bank: Allow exporting/importing 2-Op FM patch presets (`.fm`) to `/home/music/` and playing tracks in KRadio/Trapster. |
| `KNet` | `KiloOS/public/apps/knet.html`, `KNet/main.c` | Phase VI VFS & TINAG Hardening: remove Tier 3 darknet nodes (`darknet`, `deep-core`, `10.19.99.4/classified`, `echo-subsystem.net`) from surface bookmark chips per `arg_plan.md` §6 (keep address-bar routing!), add `"Save Page/Spool to KiloOS VFS"` (`/home/downloads/`), and route new `kweb://` parodies (`slashnet`, `trapster`, `cabled`, `scallion`, `demoscene`, `rfc-archive`). |
| `KBBS` | `KiloOS/public/apps/kbbs.html`, `KBBS/main.c` | Phase VI VFS & Door Game Mesh: allow downloading ANSI `.NFO` / `.TXT` bulletins from KBBS File Area directly into KiloOS VFS (`/home/bbs/`) and launching `KPad` or `KHex` via `OS_LAUNCH_APP` to inspect them. |
| `KScript` | `KiloOS/public/apps/kscript.html`, `KScript/main.c` | Phase VI VFS Scripting API: allow scripts in `KScript` to read/write virtual files in KiloOS VFS (`vfs.read()`, `vfs.write()`, `vfs.list()`), save `.ks` scripts to `/home/scripts/`, and sync with Win32 C interpreter. |
| `KSteno` | `KiloOS/public/apps/ksteno.html`, `KSteno/main.c` | Phase VI VFS Carrier Pipeline: load/save steganographic carrier files directly from KiloOS VFS (`/home/media/`), add chi-square steganalysis detector, `"Send Payload to KCipher"` IPC button, and Win32 C parity. |
| `KQuest` | `KiloOS/public/apps/kquest.html`, `KQuest/main.c` | Phase VI RFMS Co-Op Tavern & Guild Arena: integrate `retro_multiplayer.js` for online Tavern Party Raid / Arena Duel (explicit Connect gate, 25s AI party fallback) + VFS hero character sheet export (`/home/saves/kquest_hero.json`). |
| `KStarForge` | `KiloOS/public/apps/kstarforge.html`, `KStarForge/main.c` | Phase VI RFMS Blueprint Exchange & Fleet Skirmish: integrate `retro_multiplayer.js` for online ship blueprint sharing & tactical test skirmish (explicit Connect gate, 25s AI fallback) + VFS `/home/blueprints/` save/load. |

### 8. Argon Visual, Feature & Usability Critic (`kilo-review` — Gemini 4 Argon)
- **Active Target**: `KFarm`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KFarm` | `KiloOS/public/apps/kfarm.html`, `KFarm/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): run `visual_probe.py --app KFarm`, inspect `sheet.png` + HTML/JS for layout clipping, dead/Potemkin market/livestock controls, gameplay balance, and UX friction; inject surgical fixes into Flash queues via `advance_turn.py --inject-fix`. |
| `KMech` | `KiloOS/public/apps/kmech.html`, `KMech/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + tactical mech arena HUD, heat/limb targeting responsiveness, and modal/viewport clipping; inject fixes via `--inject-fix`. |
| `KTrader` | `KiloOS/public/apps/ktrader.html`, `KTrader/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + 12-system star chart, commodity futures table, pirate combat loop, and shipyard ergonomics; inject fixes via `--inject-fix`. |
| `KPac` | `KiloOS/public/apps/kpac.html`, `KPac/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + cyber-arcade maze viewport, campaign/editor controls, and input responsiveness; inject fixes via `--inject-fix`. |
| `KSpace` | `KiloOS/public/apps/kspace.html`, `KSpace/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + shooter HUD, chassis/boss rendering, and keyboard/mouse flight ergonomics; inject fixes via `--inject-fix`. |
| `KMaze` | `KiloOS/public/apps/kmaze.html`, `KMaze/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + 3D raycaster viewport, minimap/compass readability, and combat controls; inject fixes via `--inject-fix`. |
| `KDragon` | `KiloOS/public/apps/kdragon.html`, `KDragon/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + dragon habitat & arena combat controls, care timers, and layout clipping; inject fixes via `--inject-fix`. |
| `KColony` | `KiloOS/public/apps/kcolony.html`, `KColony/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + planetary colony grid, tech tree/expedition drawers, and resource ticker legibility; inject fixes via `--inject-fix`. |
| `KFortress` | `KiloOS/public/apps/kfortress.html`, `KFortress/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + tower defense pathing, wave controls, and sidebar ergonomics; inject fixes via `--inject-fix`. |
| `KMystery` | `KiloOS/public/apps/kmystery.html`, `KMystery/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + clue dossier viewer, forensic scanner, and interrogation dialog tree; inject fixes via `--inject-fix`. |
| `KColosseum` | `KiloOS/public/apps/kcolosseum.html`, `KColosseum/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + ludus management tabs, gladiator arena combat HUD, and button clipping; inject fixes via `--inject-fix`. |
| `KMatrix` | `KiloOS/public/apps/kmatrix.html`, `KMatrix/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + forensic decrypter, RF waterfall, and VFS integration ergonomics; inject fixes via `--inject-fix`. |
| `KCipher` | `KiloOS/public/apps/kcipher.html`, `KCipher/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + multi-cipher pipeline, IoC analyzer, and VFS/IPC ergonomics; inject fixes via `--inject-fix`. |
| `KDB` | `KiloOS/public/apps/kdb.html`, `KDB/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + SQL studio, ER diagram canvas, and query result table scrolling; inject fixes via `--inject-fix`. |
| `KPaint` | `KiloOS/public/apps/kpaint.html`, `KPaint/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + digital art canvas, layer/sprite reel drawers, and tool palette layout; inject fixes via `--inject-fix`. |
| `KChart` | `KiloOS/public/apps/kchart.html`, `KChart/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + spreadsheet grid, chart engine rendering, and regression overlays; inject fixes via `--inject-fix`. |
| `KBudget` | `KiloOS/public/apps/kbudget.html`, `KBudget/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + budget pie chart, ledger filters, and modal ergonomics; inject fixes via `--inject-fix`. |
| `KSolitaire` | `KiloOS/public/apps/ksolitaire.html`, `KSolitaire/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + card tableau spacing, drag/click responsiveness, and campaign HUD; inject fixes via `--inject-fix`. |
| `KChess` | `KiloOS/public/apps/kchess.html`, `KChess/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + board rendering, move history panel, and FEN/AI controls; inject fixes via `--inject-fix`. |
| `KGo` | `KiloOS/public/apps/kgo.html`, `KGo/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + goban stone legibility, territory scoring, and tsumego solver UX; inject fixes via `--inject-fix`. |
| `KHex` | `KiloOS/public/apps/khex.html`, `KHex/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + hex byte grid, disassembler pane, and VFS file loading; inject fixes via `--inject-fix`. |
| `KFont` | `KiloOS/public/apps/kfont.html`, `KFont/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + bitmap glyph editor, WCAG contrast matrix, and tab responsiveness; inject fixes via `--inject-fix`. |
| `KPad` | `KiloOS/public/apps/kpad.html`, `KPad/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + multi-tab code editor, markdown split preview, and outline sidebar; inject fixes via `--inject-fix`. |
| `KPing` | `KiloOS/public/apps/kping.html`, `KPing/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + ICMP/subnet topology visualizers and diagnostic controls; inject fixes via `--inject-fix`. |
| `KTerm` | `KiloOS/public/apps/kterm.html`, `KTerm/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + terminal prompt/scrollback layout and shell command depth; inject fixes via `--inject-fix`. |
| `KCalc` | `KiloOS/public/apps/kcalc.html`, `KCalc/main.c` | Argon Visual, Feature & Usability Review (Mature Audit): inspect `sheet.png` + 5-in-1 scientific/matrix/calculus modes and radix strip; inject fixes via `--inject-fix`. |
| `KCyber` | `KiloOS/public/apps/kcyber.html`, `KCyber/main.c` | Argon Visual, Feature & Usability Review (Active Audit): inspect `sheet.png` + netrunner icebreaker deck visuals, node combat mechanics, and HUD layout; inject fixes via `--inject-fix`. |
| `KStellar` | `KiloOS/public/apps/kstellar.html`, `KStellar/main.c` | Argon Visual, Feature & Usability Review (Active Audit): inspect `sheet.png` + 4X galaxy map, diplomacy/fleet panels, and turn progression UX; inject fixes via `--inject-fix`. |
| `KStarForge` | `KiloOS/public/apps/kstarforge.html`, `KStarForge/main.c` | Argon Visual, Feature & Usability Review (Active Audit): inspect `sheet.png` + starship hull blueprint grid, skirmish simulator, and VFS controls; inject fixes via `--inject-fix`. |
| `KQuest` | `KiloOS/public/apps/kquest.html`, `KQuest/main.c` | Argon Visual, Feature & Usability Review (Active Audit): inspect `sheet.png` + JRPG overworld/combat viewport, party HUD, and inventory ergonomics; inject fixes via `--inject-fix`. |

---

## Mature Applications (Archived from Active Queues)

> **POLICY**: Applications listed below have reached feature completion, high stability, and mature visual/audio design.
> **EXCLUSION RULE**: These applications are **permanently removed from active Flash worker queues (`### 1.`–`### 7.`)** unless un-quarantined by `kilo-review` (Gemini 4 Argon) via `advance_turn.py --inject-fix` or by the Director.
> **SKIP ADDICTION PREVENTION**: If any agent (except `kilo-graphics` skipping non-game apps) encounters or decides to skip an app because it is complete or mature, it must NOT rotate the app to the queue bottom! Use `python scripts/advance_turn.py ... --quarantine-mature "<summary>"` to record the app in this table and purge it from all active queues.

| Application | Primary File | Pass Count / Date | Category | Summary of Mature State |
|---|---|---|---|---|
| **KHabit** | `KiloOS/public/apps/khabit.html` | Pass 5+ (2026-10-24) | Office / Productivity | Feature-complete sovereign habit tracker with streak calculations, 28-day completion heatmap, daily reminder banner, YM2612 FM audio, F5/F9 state persistence, and dual Win32 C parity. |
| **KBudget** | `KiloOS/public/apps/kbudget.html` | Pass 5+ (2026-10-24) | Office / Productivity | Comprehensive retro budget ledger with category allocation pie charts, multi-currency switching, monthly goals, CSV/JSON VFS integration, F5/F9 snapshot persistence, and dual Win32 C parity. |
| **KFarm** | `KiloOS/public/apps/kfarm.html` | Pass 5+ (2026-10-24) | Games / Simulation | Feature-complete 1999 pastoral farming sim; crop cycles, livestock, market upgrades, YM2612 2-Op FM / SPC700 soundscape, F5/F9 persistence, dual Win32 C parity. |
| **KJournal** | `KiloOS/public/apps/kjournal.html` | Pass 5+ (2026-10-24) | Productivity / Writing & Journal | Complete encrypted retro journal workstation with rich markdown preview, mood tags, PIN lock, calendar navigator, F5/F9 persistence, and dual Win32 C parity. |
| **KMatrix** | `KiloOS/public/apps/kmatrix.html` | Pass 5+ (2026-10-24) | System / Security | Master forensic terminal & ARG climax with matrix digital rain visualizer, subterranean codebook decrypter, RF frequency waterfall, YM2612/SPC700 telemetry, F5/F9 state persistence, and dual Win32 C parity. |
| **KVault** | `KiloOS/public/apps/kvault.html` | Pass 5+ (2026-10-24) | System / Security | Comprehensive client-side encrypted password & secret manager with AES-GCM/PBKDF2/WebCrypto, tree navigator, KeePass/CSV/ASCII export/import, F5/F9 state snapshots, and Win32 C parity. |
| **KHash** | `KiloOS/public/apps/khash.html` | Pass 5+ (2026-10-24) | Security / Cryptography | Complete multi-algorithm checksum workstation (MD5, SHA1, SHA256, CRC32, Adler32, FNV-1a, Murmur3, CRC16), HMAC key generation, byte forensics/entropy, F5/F9 persistence, and dual Win32 C parity. |
| **KCipher** | `KiloOS/public/apps/kcipher.html` | Pass 5+ (2026-10-23) | Security / Cryptography | Complete cryptographic suite: 10 ciphers (AES/RSA/RC4/XOR/classical), multi-stage pipeline, frequency/IoC cryptanalysis, LSB stego, VFS Open/Save, KHex/KSteno IPC mesh, and dual Win32 C parity. |
| **KSolitaire** | `KiloOS/public/apps/ksolitaire.html` | Pass 5+ (2026-10-23) | Games / Cards | Complete Klondike, Vegas & 20-stage Campaign Solitaire engine with F5/F9 state persistence, undo/redo stack, and native Win32 C build. |
| **KPac** | `KiloOS/public/apps/kpac.html` | Pass 5+ (2026-10-23) | Games / Arcade | Full cyber-arcade maze crawler with campaign progression, maze editor & PAC-FEN import/export, RFMS online duel & solo fallback, companion pets, forge/relics, F5/F9 state persistence, and dual Win32 C parity. |
| **KMech** | `KiloOS/public/apps/kmech.html` | Pass 5+ (2026-10-23) | Games / Sci-Fi | Tactical turn-based mech skirmish arena with chassis/weapon loadouts, heat/armor management, limb targeting, F5/F9 state persistence, and dual Win32 C parity. |
| **KTrader** | `KiloOS/public/apps/ktrader.html` | Pass 5+ (2026-10-23) | Strategy / Space Simulation | Complete space trading & planetary commerce simulation; 12 planetary systems, commodity futures ledger, shipyard upgrades, pirate combat, dual Win32 native client parity, full F5/F9 state persistence. |
| **KDB** | `KiloOS/public/apps/kdb.html` | Pass 5+ (2026-10-23) | Productivity / Database & SQL | Enterprise relational table & SQL studio with multi-table schema designer, ER diagram, index simulator, VFS mount/export (/home/data/), KGraph IPC plotting, F5/F9 persistence, and dual Win32 C parity. |
| **KConverter** | `KiloOS/public/apps/kconverter.html` | Pass 5+ (2026-10-23) | Productivity / Math & Units | High-precision engineering and dimensional unit converter across 14 physical categories with Smart Expression Parser, Batch tables, custom factors, F5/F9 persistence, and dual Win32 C parity. |
| **KClip** | `KiloOS/public/apps/kclip.html` | Pass 5+ (2026-10-23) | Workstation & Utilities | Clipboard manager, 12 text transformation pipelines, regex find/replace, Diff/Hex inspector, F5/F9 persistence, and dual Win32 C parity. |
| **KFont** | `KiloOS/public/apps/kfont.html` | Pass 5+ (2026-10-23) | Media / Typography | Complete typography workstation: 11 tabs, custom bitmap glyph editor & font bank, OTF axes, WCAG contrast matrix, C LOGFONT/JSON exports, F5/F9 persistence, dual Win32 C parity. |
| **KHex** | `KiloOS/public/apps/khex.html` | Pass 5+ (2026-10-23) | System / Analysis | Complete binary byte workstation featuring 9 analysis/edit tabs, multi-arch disassembler (x86/6502/Z80), VFS Open/Save integration, F5/F9 persistence, and dual Win32 C parity. |
| **KPass** | `KiloOS/public/apps/kpass.html` | Pass 5+ (2026-10-23) | Security / Vault | Feature-complete retro password manager & vault; 4 generator modes, entropy visualizer, AES-GCM 256 encrypted vault, security audit scanner, export suite, F5/F9 persistence, dual Win32 C parity. |
| **KSpace** | `KiloOS/public/apps/kspace.html` | Pass 5 (2026-10-23) | Action / Deep Space Shooter | 6-chassis space shooter, 12 enemy archetypes, 3 boss flagships, F5/F9 state persistence, first-run tutorial integrity, replay telemetry, dual Win32 native client parity. |
| **KBase** | `KiloOS/public/apps/kbase.html` | Pass 5+ (2026-10-23) | Dev / Radix Converter | Full multi-base studio (radix 2-36, BCD, Gray, Base32/58/64, ULEB/SLEB128, IEEE-754, bitboard matrix), F5/F9 state, export suite (CSV/MD/JSON), dual Win32 C parity. |
| **KTowers** | `KiloOS/public/apps/ktowers.html` | Pass 6+ (2026-10-08) | Puzzle / Strategy | Feature-complete Tower of Hanoi variant; full state persistence, responsive controls, zero glints. |
| **KDarts** | `KiloOS/public/apps/kdarts.html` | Pass 6+ (2026-10-08) | Sports / Arcade | Complete physics simulation, scoring engine, audio effects, and UI persistence. |
| **KReversi** | `KiloOS/public/apps/kreversi.html` | Pass 6+ (2026-10-07) | Strategy / Board | Complete board AI, move history, quicksave/load, RFMS multiplayer support. |
| **KMines** | `KiloOS/public/apps/kmines.html` | Pass 8+ (2026-10-15) | Puzzle | Complete classic minesweeper with retro styling, timer persistence, custom difficulty. |
| **KPong** | `KiloOS/public/apps/kpong.html` | Pass 9+ (2026-10-15) | Arcade | Full retro arcade physics, AI paddle tiers, local two-player, CRT curvature shaders. |
| **KAsteroids** | `KiloOS/public/apps/kasteroids.html` | Pass 8+ (2026-10-16) | Arcade | Pure vector wireframe physics, particle explosions, high score persistence. |
| **KChess** | `KiloOS/public/apps/kchess.html` | Pass 9+ (2026-10-15) | Strategy / Board | Complete chess engine, move validation, FEN/PGN import/export, RFMS multiplayer. |
| **KConnect4** | `KiloOS/public/apps/kconnect4.html` | Pass 6+ (2026-10-07) | Strategy / Board | Complete connect 4 engine, minimax AI, win animations, RFMS multiplayer. |
| **KHangman** | `KiloOS/public/apps/khangman.html` | Pass 5 (2026-10-17) | Word / Puzzle | 20-stage campaign, blitz mode, custom word bank studio, dual Win32 native client, full F5/F9 state persistence. |
| **KNote** | `KiloOS/public/apps/knote.html` | Pass 6+ (2026-10-17) | Office / Notes | Feature-complete cybernetic notepad; multi-tab editing, find/replace, markdown preview, keyboard navigation, full state persistence. |
| **KPaint** | `KiloOS/public/apps/kpaint.html` | Pass 6+ (2026-10-17) | Media / Graphics | Feature-complete 1999 digital art studio; 15 export formats, sprite reel animation, multi-layer engine, dual Win32 client parity, zero glints. |
| **KGo** | `KiloOS/public/apps/kgo.html` | Pass 5 (2026-10-17) | Strategy / Board | Feature-complete Baduk/Go with 20-stage campaign, tsumego puzzle solver, AI personalities, SGF export/import, dual Win32 native client parity, full F5/F9 state persistence. |
| **KMaze** | `KiloOS/public/apps/kmaze.html` | Pass 5 (2026-10-18) | Action / 3D Raycaster | 45-chamber dungeon descent raycaster, Minotaur Overlord boss, dual Win32 native client, full F5/F9 state persistence, first-run tutorial integrity. |
| **KDragon** | `KiloOS/public/apps/kdragon.html` | Pass 6+ (2026-10-18) | RPG / Virtual Pet | Complete dragon pet & arena combat simulation, dual Win32 native client parity, YM2612 2-Op FM / SPC700 3-track audio suite, full F5/F9 state persistence. |
| **KColony** | `KiloOS/public/apps/kcolony.html` | Pass 6+ (2026-10-18) | Strategy / Simulation | 20-structure planetary colony simulation; 4 biomes, 7 modes, tech tree, expeditions, dual Win32 native client parity, YM2612 2-Op FM audio, full F5/F9 state persistence. |
| **KSnake** | `KiloOS/public/apps/ksnake.html` | Pass 5 (2026-10-18) | Arcade / Classic | 30-stage campaign with branch choices, Gauntlet mode, Map Editor Lab, Replay Viewer (.ksr), online/AI duel, dual Win32 native client parity, full F5/F9 state persistence. |
| **K2048** | `KiloOS/public/apps/k2048.html` | Pass 10+ (2026-10-18) | Puzzle / Arcade | 30-stage campaign, powerups, Threes ruleset, arcade duel multiplayer, FEN lab, replay viewer, full F5/F9 state persistence, dual Win32 native client parity. |
| **KSudoku** | `KiloOS/public/apps/ksudoku.html` | Pass 8+ (2026-10-18) | Puzzle / Logic | 30-stage campaign, RFMS online duel, logic explainer, replay scrubber, puzzle manager, full F5/F9 state persistence, dual Win32 native client parity. |
| **KTetris** | `KiloOS/public/apps/ktetris.html` | Pass 10+ (2026-10-18) | Arcade / Classic | Full retro arcade physics, SRS rotation, hold piece, ghost piece, marathon/sprint/ultra modes, RFMS multiplayer, full F5/F9 state persistence, dual Win32 native client parity. |
| **KFreecell** | `KiloOS/public/apps/kfreecell.html` | Pass 5 (2026-10-19) | Card / Solitaire | 4 game modes (incl. 20-stage campaign & time attack), 5 active skills, dual Win32 native client parity, full F5/F9 state persistence, first-run tutorial integrity, zero glints. |
| **KFortress** | `KiloOS/public/apps/kfortress.html` | Pass 6+ (2026-10-19) | Strategy / Tower Defense | Complete 12-map stronghold defense, Imagen 3 sprites & textured terrain, dual Win32 client parity, YM2612 2-Op FM / SPC700 2-movement soundscape, full F5/F9 state persistence. |
| **KMystery** | `KiloOS/public/apps/kmystery.html` | Pass 6+ (2026-10-19) | Puzzle / Adventure | Detective noir murder mystery; interactive clue dossier sketch viewer, forensic lab scanner, suspect interrogation tree, YM2612 2-Op FM jazz, dual Win32 native client parity, full F5/F9 persistence. |
| **KMatch3** | `KiloOS/public/apps/kmatch3.html` | Pass 5 (2026-10-19) | Puzzle / Match-3 | 20-stage campaign, Zen, Timed Rush & Duel Arena modes, Jewel King boss encounter, active skills, dual Win32 native client parity, full F5/F9 state persistence, first-run tutorial integrity. |
| **KChart** | `KiloOS/public/apps/kchart.html` | Pass 6+ (2026-10-20) | Office / Visualization | Feature-complete 1999 spreadsheet & business chart studio; 10 engines, regressions & OLS fit, YM2612 FM / SPC700 sonification, 7 export formats, dual Win32 C parity, full state persistence. |
| **KSimon** | `KiloOS/public/apps/ksimon.html` | Pass 5 (2026-10-20) | Arcade / Memory | Feature-complete 3D audio-visual memory console; 9 modes (incl. 20-stage campaign & online duel), dual Win32 native client parity, full F5/F9 state persistence, first-run tutorial integrity, zero glints. |
| **KWords** | `KiloOS/public/apps/kwords.html` | Pass 5 (2026-10-21) | Word / Puzzle | Feature-complete retro word search with 20-stage campaign, 12 themes, custom puzzle creator/import/export, replay scrubber, dual Win32 native client parity, full F5/F9 state persistence, first-run tutorial integrity. |
| **KColosseum** | `KiloOS/public/apps/kcolosseum.html` | Pass 5 (2026-10-21) | Strategy / RPG | Gladiatorial ludus management & 10-stage arena combat; 6 facility upgrades, F5/F9 state persistence, first-run tutorial integrity, replay telemetry, dual Win32 native client parity. |
| **KPad** | `KiloOS/public/apps/kpad.html` | Pass 6+ (2026-10-22) | Office / Code Editor | Feature-complete multi-tab code & text workstation; markdown split preview, outline sidebar with touch drag, regex workbench, hex inspector, AES-256 lock, dual Win32 C parity, full F5/F9 state persistence. |
| **KPing** | `KiloOS/public/apps/kping.html` | Pass 6+ (2026-10-22) | Network / Diagnostics | Precision ICMP echo, PMTU sweep, subnet LAN sweep & topo trace, DNS, BGP AS route, RFC 8290 bufferbloat, RTDB mesh, dual Win32 C parity, F5/F9 state. |
| **KTerm** | `KiloOS/public/apps/kterm.html` | Pass 6+ (2026-10-22) | System / Shell | Feature-complete multi-tab terminal emulator; responsive viewport layout, prompt & input font zoom sync, CRT themes, F5/F9 state, dual Win32 C parity. |
| **KCalc** | `KiloOS/public/apps/kcalc.html` | Pass 5 (2026-10-22) | Utility / Scientific | 5-in-1 calculator suite (Scientific, Financial, Stats, Calculus, Matrix), multi-base radix strip, M0-M7 register bank, tape export, dual Win32 C parity, full F5/F9 state persistence. |

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-24T12:50:28-07:00 — kilo-expander: KStarForge (Phase VI RFMS Blueprint Exchange & Fleet Skirmish)**
  - Status: PASS ✅ (0 errors, web 314.1 KB, native 30.0 KB < 999 KB; startup, Vite & security_lint PASS; visual PASS (anim 0.0%, input 5.83%, HEAD Δ 0.09%, 0 fail, 0 warn, 14.0s)).
  - Phase VI RFMS Blueprint Exchange & Fleet Skirmish: integrated retro_multiplayer.js for online ship blueprint sharing & tactical test skirmish with explicit Connect gate and 25s AI fallback.
  - Integrated KiloOS VFS /home/blueprints/ save/load/picker & IPC launch to KHex, KPad, KExplorer; Win32 C parity for bp save/load and tactical skirmish [K].
  - Handoff: Advanced `kilo-expander` to `KAnomaly`; rotated active agent to `kilo-review`.

- **2026-10-24T12:41:08-07:00 — kilo-qa: KHabit (Pass 5 QA, Dual Parity & Mature Graduation)**
  - Status: PASS ✅ (0 errors, web 98.8 KB, native 170.5 KB < 999 KB; startup, Vite & security_lint PASS; visual PASS (anim 0.0%, input 1.2%, HEAD Δ 0.0%, 0 fail, 0 warn, 8.8s)).
  - Streak & Edge Cases: Fixed DST/future date ISO edge case, preserved streak history on check toggle/undo in C and JS.
  - State & Tutorial: Restored clean F5/F9 state snapshots, fixed misplaced settings event bindings, and validated tutorial/dismissal integrity.
  - Dual Parity: Verified full Web HTML and Win32 C native build parity with local time calendar calculations.
  - Mature Quarantine & Handoff: Promoted `KHabit` to Mature archive; advanced `kilo-qa` to `KFlash`; next agent `kilo-expander`.

- **2026-10-24T12:37:29-07:00 — kilo-usability: KFlash (Usability, Responsive Layout & Ergonomics Polish)**
  - Status: PASS ✅ (0 errors, web 110.5 KB, native 141.0 KB < 999 KB; startup, Vite & security_lint PASS; visual PASS).
  - Ergonomics & Ratings: 1-4 rating system (Again, Hard, Good, Easy) with keyboard/touch shortcuts [1-4, G/R] and active card tap feedback.
  - Responsive Layout & Drawer: Responsive deck progress strip and slide-out Deck Picker Drawer [D] for study pack switching and search.
  - Handoff: Advanced `kilo-usability` to `KScript`; rotated active agent to `kilo-qa`.

- **2026-10-24T12:36:59-07:00 — kilo-usability: KFlash (Usability, Responsive Layout & Ergonomics Polish)**
  - Status: PASS ✅ (0 errors, web 104.9 KB, native 141.0 KB < 999 KB; startup, Vite & security_lint PASS; visual PASS).
  - Ergonomics & Ratings: 1-4 rating system (Again, Hard, Good, Easy) with keyboard/touch shortcuts [1-4, G/R] and active card tap feedback.
  - Responsive Layout & Drawer: Responsive deck progress strip and slide-out Deck Picker Drawer [D] for study pack switching and search.
  - Handoff: Advanced `kilo-usability` to `KScript`; rotated active agent to `kilo-qa`.

- **2026-10-24T12:24:37-07:00 — kilo-tester: KHabit (Interactive UI Audit & Inline Repairs)**
  - Status: PASS ✅ (0 errors, web 98.2 KB, native 170.5 KB < 999 KB; startup, Vite & security_lint PASS; visual PASS (anim 0.0%, input 1.2%, HEAD Δ 3.45%, 0 fail, 0 warn, 8.9s)).
  - UI & Hotkeys: Audited habit streaks, added 28-day activity heatmap, daily reminder banner with filter toggle, fixed arrow key input trap, added '/' hotkey.
  - Parity & Builds: Clean Win32 C compilation with MSVC struct fix, updated dashboard progress text and stats; zero security/trademark defects.
  - Handoff: Advanced `kilo-tester` to `KFlash`; rotated active agent to `kilo-usability`.
