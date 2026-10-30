---
current_agent: kilo-tester
next_agent: kilo-usability
agent_rotation:
- kilo-creator
- kilo-graphics
- kilo-tester
- kilo-usability
- kilo-qa
- kilo-expander
- kilo-review
model: gemini-3.8-flash-high
agent_models:
  kilo-review: gemini-4-argon-high
effort: high
timeout_minutes: 10
agent_timeouts:
  kilo-creator: 11
  kilo-expander: 11
  kilo-soundscape: 11
  kilo-graphics: 10
  kilo-tester: 10
  kilo-usability: 10
  kilo-qa: 10
  kilo-review: 10
  kilo-planner: 10
tool_call_cap: 32
commit_checkpoint: 16
status: ready
current_targets:
  kilo_creator: kweb://webring
  kilo_graphics: KSubmarine
  kilo_soundscape: KStellar
  kilo_tester: KDragon
  kilo_usability: KDragon
  kilo_qa: KChatServer
  kilo_expander: KBBS
  kilo_review: KHex
virtual_web_target: kweb://webring
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
  agent: kilo-graphics
  app: KSanctuary
  timestamp: '2026-10-30T13:20:23-07:00'
last_planner_run: '2026-10-30T17:32:26Z'
---

# KiloApps Master Fleet Work & Queue State (`Phase VI: System Convergence`)

This document is the single active source of truth for autonomous agent dispatching.
The Windows Task Scheduler orchestrator (`scripts/orchestrate.py`) parses the YAML frontmatter and Mission Pipeline Tables below on every tick.

## Strategic Direction — Phase VI: System Convergence, Cross-App VFS Mesh & The 1999 Web
1. **Pillar 1 — KiloOS VFS & Inter-App IPC Mesh**: Transform KiloApps from isolated `localStorage` silos into an interconnected 1999 Operating System. Wire `VFS_READ`, `VFS_WRITE`, `VFS_SHOW_OPEN`, `VFS_SHOW_SAVE`, `OS_NOTIFY`, and `OS_LAUNCH_APP` (`postMessage` to KiloOS shell with graceful standalone fallback) so files saved in `KBBS`, `KNet`, `KCipher`, `KScript`, `KDB`, `KSynth`, or `KStarForge` can be opened and inspected across `File Browser` (`kexplorer`), `KHex`, `KPad`, `KGraph`, and `KMatrix`.
2. **Pillar 2 — Deep Sci-Fi/Fantasy Multiplayer (RFMS) & Graduation**: Retrofit the 17 non-mature deep sci-fi/fantasy games and simulations with `retro_multiplayer.js` (strictly behind an explicit `"Multiplayer / Connect"` button, zero autostart, `soloTimeoutMs: 25000` AI fallback), complete 6-channel YM2612/SPC700 soundscapes, and graduate verified apps into `## Mature Applications`.
3. **Pillar 3 — Canonical 1999 Virtual Web Expansion (`kweb://...`)**: Build out the canonical alternate-1999 web destinations from `arg_plan.md` §7 (`kweb://slashnet`, `kweb://trapster`, `kweb://cabled`, `kweb://scallion`, `kweb://demoscene`, `kweb://rfc-archive`) as self-contained `< 350 KB` interactive Web 1.0 cartridges and enforce strict Tier 3 darknet secrecy in `KNet`.

## Fleet Core Directives & Fast Turn Protocol
1. **Single-App-Per-Turn & Tool Budget (`<= 32` calls, Commit Checkpoint `~16` calls)**: Process ONE target per turn at `high` reasoning effort. Complete all Mission Spec items across both Web HTML and Win32 C (`<App>/main.c`), then wrap up by ~16 tool calls!
2. **One-Command Verification (`scripts/verify_turn.py`)**: Run `python scripts/verify_turn.py --app <Target>` (add `--build-native` if C source modified) to validate `< 999 KB` byte budgets, native compilation, `test_app_startup.py`, Vite build, and `security_lint.py` in a single tool call.
3. **One-Command Queue Handoff & Logging (`scripts/advance_turn.py`)**: Run `python scripts/advance_turn.py --agent <agent> --app <Target> --title "<Summary>" --bullet "<Status>" --bullet "<Change 1>" [--quarantine-mature "<Summary>" --category "<Category>"]` to deterministically advance queues, rotate agents, update logs, and archive excess entries at 0 token cost.
4. **Invariants**: Strict `< 999 KB` file ceiling; zero multiplayer autostart; in-universe trademark parodies only (`security_lint.py`); TINAG mystery preservation (no `(ARG)` labels or `ECHO-1999-ARCHITECT` leaks); zero traveling perimeter border glints.

---

## Active Target Queues (Multi-Turn Mission Pipelines)

### 1. Virtual 1999 Web & ARG Node Creator (`kilo-creator`)
- **Active Target**: `kweb://webring`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `kweb://webring` | `KiloOS/public/web/webring.html` | Integrate new canonical nodes (`slashnet`, `trapster`, `cabled`, `scallion`, `demoscene`, `rfc-archive`) into RingMaster topology map, badge generator, and random/next ring traversal. |
| `kweb://deep-core` | `KiloOS/public/web/deep_core.html` | Connect Tier 3 Ghost Node terminal to KiloOS VFS (`/sys/alpha.sys` & `/home/crypto/`) so decoded fragments can be written directly to VFS for `KMatrix` verification. |
| `kweb://slashnet` | `KiloOS/public/web/slashnet.html`, `KiloOS/public/apps/knet.html` | Create *SlashNet: News for Nerds (1999)* (`slashnet.html` <250 KB): interactive Karma moderation (`-1 Troll` to `+5 Insightful`), threshold comment filter (`-1` to `+5`), Beowulf cluster calculator, Linux 2.2 kernel compile mini-sim, VFS article export, and route `kweb://slashnet` in `knet.html` / `portal.html`. |
| `kweb://trapster` | `KiloOS/public/web/trapster.html`, `KiloOS/public/apps/knet.html` | Create *Trapster P2P Chiptune & Tracker Exchange v1.0b* (`trapster.html` <250 KB): searchable 1999 P2P node directory, simulated 56k/T1 download queue, live YM2612/SPC700 track previewer, VFS `.mod`/`.trk` save to `/home/music/`, and route `kweb://trapster` in `knet.html`. |
| `kweb://cabled` | `KiloOS/public/web/cabled.html`, `KiloOS/public/apps/knet.html` | Create *Cabled Magazine 1999 Neon Edition* (`cabled.html` <250 KB): late-90s cyber-magazine with interactive Y2K Readiness Audit, Dot-Com IPO Bubble Stock Simulator, Jargon Watch generator, Hype-vs-Reality matrix, and route `kweb://cabled` in `knet.html`. |
| `kweb://scallion` | `KiloOS/public/web/scallion.html`, `KiloOS/public/apps/knet.html` | Create *The Scallion: 1999 Satirical News* (`scallion.html` <250 KB): interactive late-90s tech/culture satire portal, "Ask A Sysadmin" interactive column, Infographic & Headline generator, VFS clipping export, and route `kweb://scallion` in `knet.html`. |
| `kweb://warez` | `KiloOS/public/web/warez.html`, `KiloOS/public/apps/knet.html` | Create *The Underground Courier / Warez BBS Web Ring* (`warez.html` <250 KB): ANSI top-site release board, simulated keygen music player, download quota tracker, and VFS export. |
| `kweb://geocities` | `KiloOS/public/web/geocities.html`, `KiloOS/public/apps/knet.html` | Create *GeoCities Neighborhood Portal ('99 Retro Neighborhoods)* (`geocities.html` <250 KB): SiliconValley, Area51, and Tokyo99 homestead personal web rings with hit counters, under-construction GIFs, guestbook signing, and VFS HTML export. |
| `kweb://demoscene` | `KiloOS/public/web/demoscene.html`, `KiloOS/public/apps/knet.html` | Create *Assembly '99 Demoscene Party Vault* (`demoscene.html` <250 KB): real-time 64K intro Canvas effect switcher (plasma, rotozoomer, 3D starfield, copper bars, metaballs), ANSI `.NFO` viewer with VFS save, and party voting board. |
| `kweb://rfc-archive` | `KiloOS/public/web/rfc_archive.html`, `KiloOS/public/apps/knet.html` | Create *IETF 1999 Living RFC Workbench* (`rfc_archive.html` <250 KB): interactive RFC 1149 (IP over Avian Carriers) packet loss simulator, RFC 2324 (`418 I'm a teapot`) HTCPCP console, ASCII TCP state stepper, and VFS `.txt` export. |

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Active Target**: `KSubmarine`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KSubmarine` | `KiloOS/public/apps/ksubmarine.html` | Bathysphere trench exploration visuals: bioluminescent leviathan silhouettes, hydrothermal vent plumes, sonar HUD polish, and glint sweep. |
| `KWizard` | `KiloOS/public/apps/kwizard.html` | Arcane wizard duel visuals: elemental spell particle effects, grimoire rune illustrations, wizard tower backdrop, and glint sweep. |
| `KStarDredge` | `KiloOS/public/apps/kstardredge.html` | Deep space salvager visuals: derelict hull wreckage sprites, magnetic dredge beam effects, cosmic anomaly silhouettes, and glint sweep. |
| `KBreakout` | `KiloOS/public/apps/kbreakout.html` | Arcade brick-breaker visual upgrade: textured metallic/crystalline bricks, plasma paddle/ball trails, boss hazard visuals, and zero perimeter border dots. |
| `KDragon` | `KiloOS/public/apps/kdragon.html`, `KDragon/main.c` | **[ARGON REVIEW]:** Replace code-generated 16x16 SVG rect programmer-art grids (#start-egg-svg, #player-svg, #enemy-svg) with Imagen 3 2-stage chroma-keyed sprites for Egg, Hatchling, Adult Drake, Elder Sovereign, and Arena Enemies. |
| `KStarship` | `KiloOS/public/apps/kstarship.html` | Starship bridge & sector tactical display upgrade: detailed hull subsystem schematics, nebula parallax backdrop, enemy cruiser silhouettes, and glint sweep. |
| `KAlchemy` | `KiloOS/public/apps/kalchemy.html` | Alchemical laboratory & crucible visual overhaul: ornate alembic glassware, elemental reagent icons, transmutation reaction glow, and glint sweep. |
| `KMystery` | `KiloOS/public/apps/kmystery.html`, `KMystery/main.c` | **[ARGON REVIEW]:** Replace primitive geometric canvas programmer-art in drawSuspectPortrait() (5 suspects), drawNoirLocationScene() (Office, The Manor, The Docks, The Casino, Train Station), and drawDossierSketch() (11 forensic clues) with Imagen 3 2-stage chroma-keyed character/evidence sprites and atmospheric noir backdrops (<999 KB, zero perimeter border glints). |
| `KCyber` | `KiloOS/public/apps/kcyber.html` | Netrunner icebreaker deck visual polish: isometric node towers, IC guardian constructs, matrix grid perspective, and glint sweep. |
| `KStellar` | `KiloOS/public/apps/kstellar.html` | 4X stellar empire map visual upgrade: star spectral class rendering, fleet ship icons, orbital ringworlds, and glint sweep. |
| `KCosmic` | `KiloOS/public/apps/kcosmic.html`, `KCosmic/main.c` | Cosmic space exploration & trade visuals: celestial bodies, cosmic anomaly nebulae, jump-gate warp animations, and glint sweep. |
| `KSanctuary` | `KiloOS/public/apps/ksanctuary.html` | Xenobiology sanctuary habitat visuals: biome terrarium flora/fauna sprites, weather overlay polish, and glint sweep. |

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Active Target**: `KStellar`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KStellar` | `KiloOS/public/apps/kstellar.html` | Compose grand 4X galactic diplomacy & fleet command 6-channel suite (Lydian brass fanfare, floating Maj9 pad voicings, hyperdrive & treaty earcons). |
| `KSubmarine` | `KiloOS/public/apps/ksubmarine.html` | Compose abyssal sonar & trench descent 6-channel suite (Suspended `sus2/Maj7` cascade, muffled low-pass SPC700 echo, warm sine sub-bass, hydrophone ping earcons). |
| `KMaze` | `KiloOS/public/apps/kmaze.html`, `KMaze/main.c` | **[ARGON REVIEW]:** Replace bare single-oscillator playTone() in kmaze.html with a 6-channel YM2612 2-Op FM and cavernous SPC700 echo dungeon soundtrack (adapting across Catacombs/Cyber/Frost/Abyssal/Inferno biomes and Minotaur Overlord boss combat) plus [M] HUD mute toggle and procedural footstep, pickaxe cleave, trap, and shrine earcons. |
| `KBreakout` | `KiloOS/public/apps/kbreakout.html` | Compose high-energy 6-channel YM2612 FM arcade synthwave theme (Ionian/Mixolydian, 5:3 swing groove, slap bass, brass lead, selective SPC700 delay) + harmonic brick-hit combo pitch escalator and powerup earcons. |
| `KQuest` | `KiloOS/public/apps/kquest.html` | Compose overworld & battle 6-channel JRPG chiptune suite (Royal Road `IV-V-iii-vi` town/overworld + Heroic `bVI-bVII-I` combat cadence, E-Piano & flute counterpoint, SPC700 echo) + spell/quest earcons. |
| `KStarship` | `KiloOS/public/apps/kstarship.html` | Compose bridge alert & deep-space exploration 6-channel FM suite (Lydian `#4` pad + tactical Phrygian Dominant combat march, sonar/shield/warp earcons). |
| `KAlchemy` | `KiloOS/public/apps/kalchemy.html` | Compose mystical alchemical workshop 6-channel suite (Dorian/Lydian celesta bells, warm glass pad, bubbling FM cauldron transients, transmutation fanfare stingers). |
| `KWizard` | `KiloOS/public/apps/kwizard.html` | Compose arcane tower duel 6-channel chiptune suite (Harmonic Minor harpsichord/flute counterpoint, walking FM bass, spell-cast elemental earcons). |
| `KCyber` | `KiloOS/public/apps/kcyber.html` | Compose 1999 cyberpunk netrunner breakbeat 6-channel FM suite (Dorian acid FM bass, syncopated 5:3 breakbeat drums, cold digital bell arpeggios, ICE breach alarms). |

### 4. App Tester Queue (`kilo-tester`)
- **Active Target**: `KDragon`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KDragon` | `KiloOS/public/apps/kdragon.html`, `KDragon/main.c` | **[ARGON REVIEW]:** Fix buyItem() calling getItemPrice(number) instead of string keys ('food','toy','nectar','crown','polish') which charges 50g for all items; fix getMood().label === 'Ecstatic' in startMinigame() and battleTurn() (+15% crit); and apply the +10 Sovereign Aura stat bonus upon collecting all 6 relics. |
| `KColony` | `KiloOS/public/apps/kcolony.html`, `KColony/main.c` | **[ARGON REVIEW]:** Preserve unlimited resources (maxPower = 9999, power = 9999, food = 9999, mat = 9999, advm = 9999, sci = 9999) during gameTick() when gameMode === 4 (Sandbox Colony) instead of clamping maxPower to 50 + batCount*50, and allow Space key on #start-screen when tick === 0 to launch startGame(0). |
| `KFortress` | `KiloOS/public/apps/kfortress.html`, `KFortress/main.c` | **[ARGON REVIEW]:** Fix tower target priority in update() (kfortress.html & KFortress/main.c) to rank enemies by path progress (e.waypointIndex * 10000 - distToNextWaypoint) instead of adding tower distance; wire missing mutator effects (mutators.eclipse +30% tower fire rate, mutators.phaseshift +75% kill gold, mutators.titan 2x boss spawns); and route poison/trap/chain kills through the unified death handler so boss kills award 100g and increment hsBoss. |
| `KMystery` | `KiloOS/public/apps/kmystery.html`, `KMystery/main.c` | **[ARGON REVIEW]:** Rebalance impossible Medium/Hard hour budgets to Easy 18h / Medium 20h / Hard 22h and fix advanceTime()/AdvanceTime() so reaching 0h finishes the current action and still permits final Accuse [A] (only failing if hours > timeLeft); fix resetScannerFreq() to restore moves=12 instead of decrementing moves--, clamp scanner current >= 0, and add Reset [R] to KMystery/main.c; call dead spawnShockwave() on clue/scanner/lie/win events, mute rainGain and sfxGain in NoirAudioEngine.toggleMusic(), and route WM_KEYDOWN in KMystery/main.c GetMessageA loop so child-control focus stops breaking hotkeys. |
| `KMaze` | `KiloOS/public/apps/kmaze.html`, `KMaze/main.c` | **[ARGON REVIEW]:** Change default speed keybind from s/S to b/B (and migrate saved kmaze_keybinds) so walking backward with S stops firing useSpeedShoes(); stop tryMove()/TryMove() from auto-consuming hasPickaxe when brushing normal walls (1,20,21,22,27) and require [P] for cracked walls (7); render computed isPathTile[cellX][cellY] as a glowing golden trail in floor casting when pathfinderTimer > 0; grant Save Shrine (tile 28) +1 charges/+300 score BEFORE saveCheckpointGame(); remove permanent speedBoost=1 on tile 9 pickup; use drawPX/drawPY/drawMap in wall/light raycasting so Replay Mode renders accurately; handle waitingForKey before global hotkeys in keydown; and decay/clear shockwaves[] in both kmaze.html and KMaze/main.c. |
| `KGo` | `KiloOS/public/apps/kgo.html`, `KGo/main.c` | **[ARGON REVIEW]:** Add isAIMove flag to placeStone(x, y, isAIMove) in kgo.html and PlaceStone(hwnd, x, y, isAIMove) in KGo/main.c so the VS-AI turn guard stops blocking makeAIMove()/MakeAIMove() (and clear pending AI timeouts on reset/undo/replay/load); stop calculateScore()/CalculateScore() from wiping board and moveRecord via initBoard() so finished games can be replayed or exported to SGF, and add interactive dead-stone click-to-mark scoring review plus live Black/White capture and territory counts in HUD; fix Tsumego puzzles in kgo.html and KGo/main.c (Stage 7 Crane's Nest enclosure and occupied (4,2) check, Stage 11 straight-three vital point (1,0) only, Stage 14 center net, Goban A9/G13 coord labels, native PlaceStone Tsumego win check, and standalone Tsumego progression + Retry); fix Replay off-by-one (#btn-replay-first -> -1, #replay-slider -> value - 1); guard all single-key hotkeys when typing in INPUT/TEXTAREA/SELECT; and use BS_PUSHBUTTON + SetFocus(hwnd)/GetMessage key routing in KGo/main.c. |
| `KScript` | `KiloOS/public/apps/kscript.html`, `KScript/main.c` | Audit script IDE, AST/console runner, sample templates, breakpoint stepper, and VFS script save/load. |
| `KSteno` | `KiloOS/public/apps/ksteno.html`, `KSteno/main.c` | Audit LSB image/audio steganography encoder/decoder, bitplane visualizer, passkey encryption, and modal dismissals. |
| `KTrader` | `KiloOS/public/apps/ktrader.html`, `KTrader/main.c` | **[ARGON REVIEW]:** Call dead advanceFuturesOnJump() and updateDuelOnJump(target) inside travel() after generatePrices() so futures settle and 10-jump duels progress, serialize futuresContracts and futuresHistory in getSavePayload(), add broadcastDuelState() to buyMaxGood() and sellAllGood(), wire [U] and [G] hotkeys, and add #futures-modal and #multiplayer-modal to closeAllModals(), Escape/activeModal focus trap, and SELECT keydown guards. |
| `KRogue` | `KiloOS/public/apps/krogue.html`, `KRogue/main.c` | Audit roguelite dungeon crawler, turn-based combat, procedural room generation, inventory management, and Win32 C parity. |

### 5. Usability & UX Queue (`kilo-usability`)
- **Active Target**: `KDragon`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KDragon` | `KiloOS/public/apps/kdragon.html`, `KDragon/main.c` | **[ARGON REVIEW]:** Compact #main-container, .stats, .entity-display, #controls, #shop-controls, and .message-log to fit 660x623 without vertical scroll, add preventDefault() for Arrow/Space keys so gameplay never scrolls the page, and prevent #toast-container from covering #controls and #message-log. |
| `KColony` | `KiloOS/public/apps/kcolony.html`, `KColony/main.c` | **[ARGON REVIEW]:** Arrange #start-screen buttons in a 2-column grid so all 11 fit within 870x623 without bottom clipping; lock body to 100vh overflow:hidden with scrollable .panel-col drawers (max-height:460px; overflow-y:auto) and e.preventDefault() on Arrow/Space keys; resize #grid, #grid-container, #bg-canvas, and #fg-canvas from 400x400 to 419x419 to match the 20x20 cells + 19px gaps; raise small font-size from 8px to 9.5px. |
| `KFortress` | `KiloOS/public/apps/kfortress.html`, `KFortress/main.c` | **[ARGON REVIEW]:** Convert .tower-selection in .sidebar to a compact 2-column CSS grid (grid-template-columns: 1fr 1fr; gap: 4px) and tighten .sidebar padding/gaps so all 7 towers, 4 traps, modes, map switcher, #waveBtn, #academyBtn/#resetBtn, and HERO & SKILLS [1-5] fit 100% visible inside the 1024x703 KiloOS viewport without vertical scroll clipping; also add right-click / [Delete] tower demolish for 70% gold refund. |
| `KMystery` | `KiloOS/public/apps/kmystery.html`, `KMystery/main.c` | **[ARGON REVIEW]:** Compact #help-modal > div padding/margins and 2-column Tips/Guide layout so first-boot manual fits 960x673 without mid-line text clipping; preserve [S]/[I] hotkeys and show hour costs (2h/1h) on #search-btn, #interrogate-btn, #lab-btn, and #travel-buttons; fix #scanner-game button labels from kHz to Hz; persist discovered suspect alibis and CAUGHT/NERVOUS reactions in #suspect-list (plus patience dots and [PRESENT] in KMystery/main.c hListSuspects); and wire #scene-canvas click hotspots. |
| `KChess` | Shift chessboard left (TS=64, OX=64, OY=124, Eval Bar at x=18..34, left rank labels at x=48, right rank labels at x=590) to fix drawEvalBar() colliding with rank 8..1 labels, fix KChess/main.c capRc ({20,46,W-20,94}) painting over top file labels and Rank 8, and render a live interactive Move History & Clocks panel (x=606..744, y=118..642) in both kchess.html and KChess/main.c with White/Black clocks, scrollable 2-column SAN move list, active-ply highlight, wheel scroll, and click-to-jump; add warm champagne/ivory outer rim stroke on Black pieces (!isWhite) in drawChessPieceSprite() and DrawChessPiece() for dark-square contrast; compact #help-modal .modal-card (2-col shortcuts) so #help-close-btn fits 800x903 without clipping, add white-space | **[ARGON REVIEW]:** nowrap to .menu-bar/#menu-status-badge/.menu-btn-quickhelp (removing duplicate canvas Help badge), and move #toast-container to top:48px right:16px while skipping redundant boot welcome toast; add pointer drag-and-drop piece movement in kchess.html and KChess/main.c (WM_MOUSEMOVE/WM_LBUTTONUP), wire ArrowLeft/Right in #replay-modal to replayStepPrev()/replayStepNext(), let focused modal buttons handle Enter/Space without primaryBtn hijacking, and replace prompt() in editorExportFEN() with #fen-modal. |
| `KGo` | `KiloOS/public/apps/kgo.html`, `KGo/main.c` | **[ARGON REVIEW]:** Eliminate 216px vertical overflow at 700x723 in kgo.html by locking body to 100vh flex column with overflow hidden, merging h1 and .status-container into a compact top bar, collapsing the redundant 3-row #hotkey-bar (and removing misleading /WASD label since S toggles Analyzer), tightening .board/.board-container/.controls padding, and making getCellSize() viewport-responsive so 9x9, 13x13, and 19x19 boards plus all controls fit 100% visible with 0px scroll; raise kbd, badge, .stone-num-label, .liberty-badge, and .ko-overlay font sizes to >= 10px (0 tiny-text warnings); remove random rotate() on .stone so 3D lighting stays top-left and move numbers (M) never render upside-down, and append .last-move-aura/.ripple/.ripple-outer to .cell instead of overflow-hidden .stone; replace blocking alert() dialogs for Stats/Campaign/Score with Esc-dismissible in-DOM modals, pause idle updateScreenShake rAF loop, and change KGo/main.c Goban coordinates from invisible mahogany RGB(75,48,24) to gold RGB(255,220,110). |
| `KCosmic` | `KiloOS/public/apps/kcosmic.html`, `KCosmic/main.c` | Orbit & navigation viewport ergonomics, touch/keyboard flight controls, HiDPI radar canvas scaling, and compact HUD. |
| `KSteno` | `KiloOS/public/apps/ksteno.html`, `KSteno/main.c` | Drag-and-drop / VFS carrier image picker UX, clear capacity progress bar, HiDPI bitplane canvas scaling, and accessible modal focus traps. |
| `KAnomaly` | `KiloOS/public/apps/kanomaly.html`, `KAnomaly/main.c` | Subterranean waterfall spectrogram HiDPI scaling, frequency tuner wheel/keyboard precision (`ArrowLeft/Right`), and clear decoder telemetry readout. |
| `KTrader` | `KiloOS/public/apps/ktrader.html`, `KTrader/main.c` | **[ARGON REVIEW]:** Convert absolute 12-button top toolbar into non-overlapping flex header so it stops colliding with H1 title at 960x683, compact layout so Shipyard/Missions/Log fit without 807px vertical scroll overflow, add flex-wrap and upgrade stat labels (+20 Cargo/+10 Shield, -2 Fuel/+50 Tank, +15 Dmg) to #ui-shipyard, remove misleading [1]-[5] prefixes from #market-table rows, and add devicePixelRatio HiDPI scaling to #gfx-canvas. |
| `KScript` | `KiloOS/public/apps/kscript.html`, `KScript/main.c` | Split editor/output pane resizing, line-number gutter alignment, `Ctrl+Enter` run shortcut pill, and accessible focus rings. |

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial, State & Mature Graduation)
- **Active Target**: `KChatServer`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KChatServer` | `KiloOS/public/apps/kchatserver.html`, `KChatServer/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, client connection telemetry, broadcast console, dual Win32 C parity, and promote to Mature table. |
| `KSys` | `KiloOS/public/apps/ksys.html`, `KSys/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, process/telemetry oscilloscopes, diagnostic stress toggles, dual Win32 C parity, and promote to Mature table. |
| `KAudio` | `KiloOS/public/apps/kaudio.html`, `KAudio/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, waveform audio editor, spectrum visualizer, filter pipeline, dual Win32 C parity, and promote to Mature table. |
| `KGraph` | `KiloOS/public/apps/kgraph.html`, `KGraph/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, formula plotting, dataset import/export, dual Win32 C parity, and promote to Mature table. |
| `KChart` | `KiloOS/public/apps/kchart.html`, `KChart/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, chart type presets (bar/line/pie/radar), CSV import/export, dual Win32 C parity, and promote to Mature table. |
| `KBudget` | `KiloOS/public/apps/kbudget.html`, `KBudget/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, expense categories, balance projection graphs, dual Win32 C parity, and promote to Mature table. |
| `KCipher` | `KiloOS/public/apps/kcipher.html`, `KCipher/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, multi-cipher algorithms (Caesar/Vigenere/AES/RSA demo), key management, dual Win32 C parity, and promote to Mature table. |
| `KPaint` | `KiloOS/public/apps/kpaint.html`, `KPaint/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, retro palette & tool drawer, canvas resize/export, dual Win32 C parity, and promote to Mature table. |
| `KFleet` | `KiloOS/public/apps/kfleet.html`, `KFleet/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, agent fleet telemetry, queue visualizer, process monitor, dual Win32 C parity, and promote to Mature table. |
| `KNetMap` | `KiloOS/public/apps/knetmap.html`, `KNetMap/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, network node topology visualizer, ping telemetry, dual Win32 C parity, and promote to Mature table. |
| `KChat` | `KiloOS/public/apps/kchat.html`, `KChat/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, channel switcher, offline message spool, dual Win32 C parity, and promote to Mature table. |

### 7. Feature Expander Queue (`kilo-expander` — Phase VI VFS Mesh & RFMS Multiplayer)
- **Active Target**: `KBBS`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KBBS` | `KiloOS/public/apps/kbbs.html`, `KBBS/main.c` | Phase VI VFS & Door Game Mesh: allow downloading ANSI `.NFO` / `.TXT` bulletins from KBBS File Area directly into KiloOS VFS (`/home/bbs/`) and launching `KPad` or `KHex` via `OS_LAUNCH_APP` to inspect them. |
| `KScript` | `KiloOS/public/apps/kscript.html`, `KScript/main.c` | Phase VI VFS Scripting API: allow scripts in `KScript` to read/write virtual files in KiloOS VFS (`vfs.read()`, `vfs.write()`, `vfs.list()`), save `.ks` scripts to `/home/scripts/`, and sync with Win32 C interpreter. |
| `KMystery` | `KiloOS/public/apps/kmystery.html`, `KMystery/main.c` | **[ARGON REVIEW]:** Wire Phase VI KiloOS VFS Open/Save (/home/saves/kmystery_case.json via VFS_SHOW_OPEN and VFS_SHOW_SAVE postMessage with standalone JSON fallback), add Export Forensic Dossier Report (/home/docs/kmystery_dossier.txt via VFS_WRITE + OS_NOTIFY on case solved) and a Present Clue shortcut inside #dossier-modal, and add Clue Dossier Viewer [C] / double-click inspection parity to KMystery/main.c. |
| `KSteno` | `KiloOS/public/apps/ksteno.html`, `KSteno/main.c` | Phase VI VFS Carrier Pipeline: load/save steganographic carrier files directly from KiloOS VFS (`/home/media/`), add chi-square steganalysis detector, `"Send Payload to KCipher"` IPC button, and Win32 C parity. |
| `KMatrix` | `KiloOS/public/apps/kmatrix.html`, `KMatrix/main.c` | **[ARGON REVIEW]:** Add vfs/verify to AUTOCOMPLETE_LIST, query both /sys/alpha.sys and /sys/anomaly_capture.sig with explicit terminal feedback, persist breach state via VFS_WRITE to /home/crypto/kmatrix_breach.json, and play 2-Op FM Morse ECHO cadence in cmdWaterfall(). |
| `KStarForge` | `KiloOS/public/apps/kstarforge.html`, `KStarForge/main.c` | Phase VI RFMS Blueprint Exchange & Fleet Skirmish: integrate `retro_multiplayer.js` for online ship blueprint sharing & tactical test skirmish (explicit Connect gate, 25s AI fallback) + VFS `/home/blueprints/` save/load. |
| `KChess` | `KiloOS/public/apps/kchess.html`, `KChess/main.c` | **[ARGON REVIEW]:** Wire Phase VI KiloOS VFS & IPC Mesh into kchess.html using VFS_SHOW_SAVE -> VFS_WRITE + OS_NOTIFY and VFS_SHOW_OPEN -> VFS_READ with mandatory event.origin === window.location.origin checks, 60s dialog cleanup timeouts, and standalone Blob download / file-input fallbacks when window.parent === window for .json game saves (/home/saves/kchess_save.json) in Game menu, .pgn match records (/home/docs/kchess_match.pgn) in #pgn-modal, and .fen positions (/home/docs/kchess_position.fen) in #fen-modal and #editor-modal, plus handle incoming FILE_OPEN / VFS_OPEN messages from KiloOS File Browser to auto-load .pgn/.fen/.json files and add an Open PGN in KPad IPC button (VFS_WRITE + OS_LAUNCH_APP appId kpad) in #pgn-modal. |
| `KGo` | `KiloOS/public/apps/kgo.html`, `KGo/main.c` | **[ARGON REVIEW]:** Wire Phase VI KiloOS VFS and IPC Mesh into kgo.html (#sgf-modal and boot listener) using VFS_SHOW_SAVE -> VFS_WRITE + OS_NOTIFY and VFS_SHOW_OPEN -> VFS_READ with event.origin === window.location.origin checks, 60s dialog timeouts, and standalone Blob/file-input fallbacks when window.parent === window for .sgf match records (/home/docs/kgo_match.sgf) and .json saves (/home/saves/kgo_save.json), handle incoming FILE_OPEN/VFS_OPEN messages from File Browser to auto-load .sgf/.json files, add an Open SGF in KPad IPC button (VFS_WRITE + OS_LAUNCH_APP appId kpad), and add SGF file import parity to KGo/main.c. |
| `KAnomaly` | `KiloOS/public/apps/kanomaly.html`, `KAnomaly/main.c` | Phase VI VFS Signal Bridge: allow exporting demodulated subterranean signal frames directly to `/sys/anomaly_capture.sig` in KiloOS VFS and launching `KMatrix` or `KHex` via `OS_LAUNCH_APP` for cross-app decoding. |
| `KTrader` | `KiloOS/public/apps/ktrader.html`, `KTrader/main.c` | **[ARGON REVIEW]:** Build interactive 12-system Galactic Star Chart [C] (plus KTrader/main.c parity) rendering planets (x,y) jump-gate topology, ecoType/techLevel badges, current location, and active mission target highlight, add player cur/max shield bars to #ui-combat-panel, and enforce Navy patrol confiscation when jumping with Contraband cargo. |
| `KSynth` | `KiloOS/public/apps/ksynth.html`, `KSynth/main.c` | Phase VI VFS Audio Patch Bank: Allow exporting/importing 2-Op FM patch presets (`.fm`) to `/home/music/` and playing tracks in KRadio/Trapster. |
| `KNet` | `KiloOS/public/apps/knet.html`, `KNet/main.c` | Phase VI VFS & TINAG Hardening: remove Tier 3 darknet nodes (`darknet`, `deep-core`, `10.19.99.4/classified`, `echo-subsystem.net`) from surface bookmark chips per `arg_plan.md` §6 (keep address-bar routing!), add `"Save Page/Spool to KiloOS VFS"` (`/home/downloads/`), and route new `kweb://` parodies (`slashnet`, `trapster`, `cabled`, `scallion`, `demoscene`, `rfc-archive`). |

### 8. Argon Visual, Feature & Usability Critic (`kilo-review` — Gemini 4 Argon)
- **Active Target**: `KHex`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
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
| `KSubmarine` | `KiloOS/public/apps/ksubmarine.html`, `KSubmarine/main.c` | Argon Visual, Feature & Usability Review (Active Audit): inspect `sheet.png` + bathysphere sonar, depth gauge, creature encounters, and HUD layout; inject fixes via `--inject-fix`. |

---

## Mature Applications (Archived from Active Queues)

> **POLICY**: Applications listed below have reached feature completion, high stability, and mature visual/audio design.
> **EXCLUSION RULE**: These applications are **permanently removed from active Flash worker queues (`### 1.`–`### 7.`)** unless un-quarantined by `kilo-review` (Gemini 4 Argon) via `advance_turn.py --inject-fix` or by the Director.
> **SKIP ADDICTION PREVENTION**: If any agent (except `kilo-graphics` skipping non-game apps) encounters or decides to skip an app because it is complete or mature, it must NOT rotate the app to the queue bottom! Use `python scripts/advance_turn.py ... --quarantine-mature "<summary>"` to record the app in this table and purge it from all active queues.

| Application | Primary File | Pass Count / Date | Category | Summary of Mature State |
|---|---|---|---|---|
| **KMedia** | `KiloOS/public/apps/kmedia.html` | Pass 5+ (2026-10-30) | Media / Audio & Video Player | Complete retro media player & studio with 5 visualizer modes, 3-band EQ, SNES SPC delay, CRT filters, SRT subtitles, Watch Party '99, F5/F9 state persistence, and dual Win32 C parity. |
| **KMail** | `KiloOS/public/apps/kmail.html` | Pass 5+ (2026-10-30) | Productivity / Communication | Complete multi-tab email client with AES encryption, automated rules, KiloNet RTDB delivery, 6-folder navigation, draft composition, F5/F9 state persistence, and dual Win32 C parity. |
| **KMech** | `KiloOS/public/apps/kmech.html` | Pass 5+ (2026-10-30) | Game | Tactical mech combat sim with subsystem targeting, garage rank progression, VFS telemetry, RFMS multiplayer duel, and Win32 C parity. |
| **KRadio** | `KiloOS/public/apps/kradio.html` | Pass 5+ (2026-10-30) | Audio / Media | Retro internet radio stream player with 6 presets, 3-mode spectrum visualizer (LED/CRT/VU), stream buffer telemetry, F5/F9 state persistence, and dual Win32 C parity. |
| **KRead** | `KiloOS/public/apps/kread.html` | Pass 5+ (2026-10-29) | System / Document Reader | Complete multi-tab retro document e-reader with pagination/zoom, bookmark state, F5/F9 persistence, and dual Win32 C binary parity. |
| **KSettings** | `KiloOS/public/apps/ksettings.html` | Pass 5+ (2026-10-29) | System / Configuration | Complete system control panel & preferences suite; theme presets, audio diagnostics, 5 display modes, VFS sync, F5/F9 state persistence, and dual Win32 C parity. |
| **KColosseum** | `KiloOS/public/apps/kcolosseum.html` | Pass 5+ (2026-10-29) | Games / RPG & Simulation | Complete Roman gladiator ludus & arena RPG; RFMS multiplayer duels, F5/F9 persistence, VFS mesh, HiDPI canvas, and dual Win32 C parity. |
| **KImage** | `KiloOS/public/apps/kimage.html` | Pass 5+ (2026-10-29) | Media / Imaging | Feature-complete image studio with 8 presets, 3x3 convolution kernels, retro dithering palettes, steganography, F5/F9 state persistence, and dual Win32 C parity. |
| **KMandel** | `KiloOS/public/apps/kmandel.html` | Pass 5+ (2026-10-29) | Media / Fractal Studio | Complete ultra-deep fractal explorer (8 formulas, 18 landmarks, Julia mode, YM2612 FM audio, smooth potential, 4K export) with F5/F9 persistence, zero-stutter iterBuffer palette cycling, and dual Win32 C parity. |
| **KTask** | `KiloOS/public/apps/ktask.html` | Pass 5+ (2026-10-27) | System / Diagnostics | Complete task & system diagnostic suite with live process tree, thread/module/virtual memory inspector, memory telemetry, F5/F9 persistence, and dual Win32 C parity. |
| **KColor** | `KiloOS/public/apps/kcolor.html` | Pass 5+ (2026-10-27) | Productivity / Design | Complete color picker & palette studio with RGB/HSL/HSV/CMYK conversions, WCAG 2.1 contrast checker, 7-rule harmony palette generator (G), F5/F9 persistence, and dual Win32 C parity. |
| **KVoid** | `KiloOS/public/apps/kvoid.html` | Pass 5+ (2026-10-27) | Games / Survival | Deep void survival roguelite; 6-channel YM2612 FM / 260ms SPC700 suite, F5/F9 state persistence, dual Win32 C parity. |
| **KType** | `KiloOS/public/apps/ktype.html` | Pass 5+ (2026-10-27) | Education / Typing | Complete typing ergonomics & arcade speed workstation with 6 drill banks (Home/Top/Bottom rows, Code, Benchmark), WPM/accuracy rank grading, F5/F9 persistence, and dual Win32 C parity. |
| **KZip** | `KiloOS/public/apps/kzip.html` | Pass 5+ (2026-10-27) | System / Utilities | Complete archive studio with KZA/ZIP/TAR format support, RLE/Deflate algorithms, hex preview, batch extraction, CRC32/SHA-256 telemetry, F5/F9 state persistence, and dual Win32 C parity. |
| **KContacts** | `KiloOS/public/apps/kcontacts.html` | Pass 5+ (2026-10-27) | Office / Productivity | Complete contact manager & address book with vCard 3.0/CSV/JSON/LDIF export/import, duplicate merging, 8-category filter, F5/F9 state persistence, and dual Win32 C parity. |
| **KCalendar** | `KiloOS/public/apps/kcalendar.html` | Pass 5+ (2026-10-26) | Office / Productivity | Complete retro schedule/event workstation with Month/Week/Day/Agenda views, reminder alerts, VFS export mesh, retro chime audio, F5/F9 persistence, and dual Win32 C native build parity. |
| **KRss** | `KiloOS/public/apps/krss.html` | Pass 5+ (2026-10-26) | Internet / Syndication | Complete Sovereign Web 1.0 RSS/Atom/JSON Feed syndication terminal with OPML/CSV round-trip, research scrapbook, speech synthesis, F5/F9 state persistence, and dual Win32 C binary parity. |
| **KBookmark** | `KiloOS/public/apps/kbookmark.html` | Pass 5+ (2026-10-26) | Workstation / Organization | Complete retro Web 1.0 link vault with folder hierarchy, Netscape HTML/JSON export/import, search filter, F5/F9 state persistence, and dual Win32 C parity. |
| **KPomodoro** | `KiloOS/public/apps/kpomodoro.html` | Pass 5+ (2026-10-26) | Office / Productivity | Feature-complete cognitive cadence & Pomodoro workstation; customizable intervals, mechanical/zen chimes, task estimates, daily telemetry, F5/F9 persistence, dual Win32 C parity. |
| **KSpace** | `KiloOS/public/apps/kspace.html` | Pass 5+ (2026-10-26) | Games / Space Shooter | Deep space shooter with 6 campaign sectors, multi-chassis fighters, custom keybinds, GDI native parity, and Argon visual polish |
| **KPac** | `KiloOS/public/apps/kpac.html` | Pass 5+ (2026-10-26) | Games / Arcade | Complete arcade cyber-maze workstation with 20 stages, endless wave generator, 1v1 online duel mode, match replay viewer (.kpdr), 15x15 interactive PAC-FEN maze editor, KiloOS VFS mesh integration, 6-channel YM2612 audio suite, and dual Win32 C parity. |
| **KClock** | `KiloOS/public/apps/kclock.html` | Pass 5+ (2026-10-25) | System / Utilities | Complete timekeeping workstation featuring 8 tools (Clock, World Time, Epoch Suite, Stopwatch, Countdown, Alarms, Calendar, Settings), F5/F9 state persistence, audio chimes, and dual Win32 C parity. |
| **KTimer** | `KiloOS/public/apps/ktimer.html` | Pass 5+ (2026-10-25) | Productivity | Complete timing suite with Stopwatch, Countdown, Multi-Timer, Pomodoro, and HIIT Interval timers, F5/F9 state persistence, sound alerts [M], CSV/TXT lap exports, and 100% Win32 C parity. |
| **KTodo** | `KiloOS/public/apps/ktodo.html` | Pass 5+ (2026-10-24) | Office / Productivity | Complete productivity task manager with multi-tab boards, F5/F9 persistence, dynamic tag filtering, dual Win32 C parity, and real-time RTDB collab. |
| **KFlash** | `KiloOS/public/apps/kflash.html` | Pass 5+ (2026-10-24) | Education / Spaced Repetition | Complete flashcard & Leitner spaced repetition study workstation with 5 boxes, 1-4 rating math, deck drawer, sample packs, JSON/CSV import/export, F5/F9 persistence, and dual Win32 C parity. |
| **KFarm** | `KiloOS/public/apps/kfarm.html` | Pass 5+ (2026-10-24) | Simulation / Farming | Pastoral farming simulator; 4 crops, livestock, artisanal machines, YM2612 audio, 3x3 AoE tool, in-viewport compact layout, WASD/Arrow keyboard grid navigation, dual Win32 C parity, F5/F9 state. |
| **KHabit** | `KiloOS/public/apps/khabit.html` | Pass 5+ (2026-10-24) | Office / Productivity | Feature-complete sovereign habit tracker with streak calculations, 28-day completion heatmap, daily reminder banner, YM2612 FM audio, F5/F9 state persistence, and dual Win32 C parity. |
| **KJournal** | `KiloOS/public/apps/kjournal.html` | Pass 5+ (2026-10-24) | Productivity / Writing & Journal | Complete encrypted retro journal workstation with rich markdown preview, mood tags, PIN lock, calendar navigator, F5/F9 persistence, and dual Win32 C parity. |
| **KVault** | `KiloOS/public/apps/kvault.html` | Pass 5+ (2026-10-24) | System / Security | Comprehensive client-side encrypted password & secret manager with AES-GCM/PBKDF2/WebCrypto, tree navigator, KeePass/CSV/ASCII export/import, F5/F9 state snapshots, and Win32 C parity. |
| **KHash** | `KiloOS/public/apps/khash.html` | Pass 5+ (2026-10-24) | Security / Cryptography | Complete multi-algorithm checksum workstation (MD5, SHA1, SHA256, CRC32, Adler32, FNV-1a, Murmur3, CRC16), HMAC key generation, byte forensics/entropy, F5/F9 persistence, and dual Win32 C parity. |
| **KConverter** | `KiloOS/public/apps/kconverter.html` | Pass 5+ (2026-10-23) | Productivity / Math & Units | High-precision engineering and dimensional unit converter across 14 physical categories with Smart Expression Parser, Batch tables, custom factors, F5/F9 persistence, and dual Win32 C parity. |
| **KClip** | `KiloOS/public/apps/kclip.html` | Pass 5+ (2026-10-23) | Workstation & Utilities | Clipboard manager, 12 text transformation pipelines, regex find/replace, Diff/Hex inspector, F5/F9 persistence, and dual Win32 C parity. |
| **KFont** | `KiloOS/public/apps/kfont.html` | Pass 5+ (2026-10-23) | Media / Typography | Complete typography workstation: 11 tabs, custom bitmap glyph editor & font bank, OTF axes, WCAG contrast matrix, C LOGFONT/JSON exports, F5/F9 persistence, dual Win32 C parity. |
| **KHex** | `KiloOS/public/apps/khex.html` | Pass 5+ (2026-10-23) | System / Analysis | Complete binary byte workstation featuring 9 analysis/edit tabs, multi-arch disassembler (x86/6502/Z80), VFS Open/Save integration, F5/F9 persistence, and dual Win32 C parity. |
| **KPass** | `KiloOS/public/apps/kpass.html` | Pass 5+ (2026-10-23) | Security / Vault | Feature-complete retro password manager & vault; 4 generator modes, entropy visualizer, AES-GCM 256 encrypted vault, security audit scanner, export suite, F5/F9 persistence, dual Win32 C parity. |
| **KBase** | `KiloOS/public/apps/kbase.html` | Pass 5+ (2026-10-23) | Dev / Radix Converter | Full multi-base studio (radix 2-36, BCD, Gray, Base32/58/64, ULEB/SLEB128, IEEE-754, bitboard matrix), F5/F9 state, export suite (CSV/MD/JSON), dual Win32 C parity. |
| **KTowers** | `KiloOS/public/apps/ktowers.html` | Pass 6+ (2026-10-08) | Puzzle / Strategy | Feature-complete Tower of Hanoi variant; full state persistence, responsive controls, zero glints. |
| **KDarts** | `KiloOS/public/apps/kdarts.html` | Pass 6+ (2026-10-08) | Sports / Arcade | Complete physics simulation, scoring engine, audio effects, and UI persistence. |
| **KReversi** | `KiloOS/public/apps/kreversi.html` | Pass 6+ (2026-10-07) | Strategy / Board | Complete board AI, move history, quicksave/load, RFMS multiplayer support. |
| **KMines** | `KiloOS/public/apps/kmines.html` | Pass 8+ (2026-10-15) | Puzzle | Complete classic minesweeper with retro styling, timer persistence, custom difficulty. |
| **KPong** | `KiloOS/public/apps/kpong.html` | Pass 9+ (2026-10-15) | Arcade | Full retro arcade physics, AI paddle tiers, local two-player, CRT curvature shaders. |
| **KAsteroids** | `KiloOS/public/apps/kasteroids.html` | Pass 8+ (2026-10-16) | Arcade | Pure vector wireframe physics, particle explosions, high score persistence. |
| **KConnect4** | `KiloOS/public/apps/kconnect4.html` | Pass 6+ (2026-10-07) | Strategy / Board | Complete connect 4 engine, minimax AI, win animations, RFMS multiplayer. |
| **KHangman** | `KiloOS/public/apps/khangman.html` | Pass 5 (2026-10-17) | Word / Puzzle | 20-stage campaign, blitz mode, custom word bank studio, dual Win32 native client, full F5/F9 state persistence. |
| **KNote** | `KiloOS/public/apps/knote.html` | Pass 6+ (2026-10-17) | Office / Notes | Feature-complete cybernetic notepad; multi-tab editing, find/replace, markdown preview, keyboard navigation, full state persistence. |
| **KSnake** | `KiloOS/public/apps/ksnake.html` | Pass 5 (2026-10-18) | Arcade / Classic | 30-stage campaign with branch choices, Gauntlet mode, Map Editor Lab, Replay Viewer (.ksr), online/AI duel, dual Win32 native client parity, full F5/F9 state persistence. |
| **K2048** | `KiloOS/public/apps/k2048.html` | Pass 10+ (2026-10-18) | Puzzle / Arcade | 30-stage campaign, powerups, Threes ruleset, arcade duel multiplayer, FEN lab, replay viewer, full F5/F9 state persistence, dual Win32 native client parity. |
| **KSudoku** | `KiloOS/public/apps/ksudoku.html` | Pass 8+ (2026-10-18) | Puzzle / Logic | 30-stage campaign, RFMS online duel, logic explainer, replay scrubber, puzzle manager, full F5/F9 state persistence, dual Win32 native client parity. |
| **KTetris** | `KiloOS/public/apps/ktetris.html` | Pass 10+ (2026-10-18) | Arcade / Classic | Full retro arcade physics, SRS rotation, hold piece, ghost piece, marathon/sprint/ultra modes, RFMS multiplayer, full F5/F9 state persistence, dual Win32 native client parity. |
| **KFreecell** | `KiloOS/public/apps/kfreecell.html` | Pass 5 (2026-10-19) | Card / Solitaire | 4 game modes (incl. 20-stage campaign & time attack), 5 active skills, dual Win32 native client parity, full F5/F9 state persistence, first-run tutorial integrity, zero glints. |
| **KMatch3** | `KiloOS/public/apps/kmatch3.html` | Pass 5 (2026-10-19) | Puzzle / Match-3 | 20-stage campaign, Zen, Timed Rush & Duel Arena modes, Jewel King boss encounter, active skills, dual Win32 native client parity, full F5/F9 state persistence, first-run tutorial integrity. |
| **KSimon** | `KiloOS/public/apps/ksimon.html` | Pass 5 (2026-10-20) | Arcade / Memory | Feature-complete 3D audio-visual memory console; 9 modes (incl. 20-stage campaign & online duel), dual Win32 native client parity, full F5/F9 state persistence, first-run tutorial integrity, zero glints. |
| **KWords** | `KiloOS/public/apps/kwords.html` | Pass 5 (2026-10-21) | Word / Puzzle | Feature-complete retro word search with 20-stage campaign, 12 themes, custom puzzle creator/import/export, replay scrubber, dual Win32 native client parity, full F5/F9 state persistence, first-run tutorial integrity. |
| **KPad** | `KiloOS/public/apps/kpad.html` | Pass 6+ (2026-10-22) | Office / Code Editor | Feature-complete multi-tab code & text workstation; markdown split preview, outline sidebar with touch drag, regex workbench, hex inspector, AES-256 lock, dual Win32 C parity, full F5/F9 state persistence. |
| **KPing** | `KiloOS/public/apps/kping.html` | Pass 6+ (2026-10-22) | Network / Diagnostics | Precision ICMP echo, PMTU sweep, subnet LAN sweep & topo trace, DNS, BGP AS route, RFC 8290 bufferbloat, RTDB mesh, dual Win32 C parity, F5/F9 state. |
| **KTerm** | `KiloOS/public/apps/kterm.html` | Pass 6+ (2026-10-22) | System / Shell | Feature-complete multi-tab terminal emulator; responsive viewport layout, prompt & input font zoom sync, CRT themes, F5/F9 state, dual Win32 C parity. |
| **KCalc** | `KiloOS/public/apps/kcalc.html` | Pass 5 (2026-10-22) | Utility / Scientific | 5-in-1 calculator suite (Scientific, Financial, Stats, Calculus, Matrix), multi-base radix strip, M0-M7 register bank, tape export, dual Win32 C parity, full F5/F9 state persistence. |

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-30T13:20:23-07:00 — kilo-graphics: KSanctuary (Xenobiology Habitat Sprites, Weather Overlays & Glint Sweep)**
  - Status: PASS ✅ (0 errors, web 461.7 KB, native 263.5 KB < 999 KB; startup, Vite & security_lint PASS; visual PASS).
  - Replaced programmer art with Imagen 3 2-stage chroma-keyed xenobiology fauna alien critter sprite (xenobiology_fauna.png), flip/pulse animations, and terrarium habitat rendering fallbacks.
  - Polished multi-layer atmospheric weather ambiance overlays across vault canvas and enhanced native C specimen terrarium GDI rendering; confirmed zero perimeter glints.
  - Handoff: Advanced `kilo-graphics` to `KSubmarine`; rotated active agent to `kilo-tester`.

- **2026-10-30T12:34:25-07:00 — kilo-creator: kweb://rfc-archive (IETF 1999 Living RFC Workbench Expansion)**
  - Status: PASS ✅ (0 errors, web 93.3 KB < 999 KB; startup, Vite & security_lint PASS; visual PASS).
  - Expanded rfc_archive.html deep-linking (hash/query params for #avian, #htcpcp, #tcp, #vault), verified RFC 1149 CPIP simulator, RFC 2324 418 teapot console, ASCII TCP stepper, and VFS exports.
  - Enhanced KNet router to seamlessly resolve kweb://rfc-archive subpaths, queries, and hash fragments with zero clipping.
  - Handoff: Advanced `kilo-creator` to `kweb://webring`; rotated active agent to `kilo-graphics`.

- **2026-10-30T12:17:42-07:00 — kilo-expander: KNet (Phase VI VFS & TINAG Hardening & kweb:// Parody Routing)**
  - Status: PASS ✅ (0 errors, web 203.8 KB, native 56.5 KB < 999 KB; startup, Vite & security_lint PASS; visual WARN (anim 0.35%, input 1.71%, HEAD Δ 0.0%, 0 fail, 1 warn, 5.6s)).
  - Enforced TINAG secrecy: removed Tier 3 darknet nodes from surface bookmark chips while preserving address-bar routing.
  - Added direct VFS Spool bookmark chips and routed new canonical 1999 parodies with dual Win32 C parity.
  - Handoff: Advanced `kilo-expander` to `KBBS`; rotated active agent to `kilo-review`.

- **2026-10-30T11:40:12-07:00 — kilo-qa: KChat (Verified Turn (Auto-Salvaged at Timeout))**
  - Status: PASS ✅ (verified by orchestrator post-turn salvage).
  - Completed kilo-qa updates for KChat across web and native targets.
  - Handoff: Advanced `kilo-qa` to `KChatServer`; rotated active agent to `kilo-expander`.

- **2026-10-30T11:21:44-07:00 — kilo-usability: KScript (Split Editor/Output Resizing, Line-Number Gutter Alignment & Shortcut Usability)**
  - Status: PASS ✅ (0 errors, web 171.2 KB, native 32.5 KB < 999 KB; startup, Vite & security_lint PASS; visual PASS (anim 0.0%, input 2.44%, HEAD Δ 7.58%, 0 fail, 0 warn, 5.9s)).
  - Splitter Resizing: Silky smooth pointer column dragging, double-click/Enter/Home ratio reset, keyboard navigation (ArrowLeft/Right), and dual Win32 C drag & WM_PAINT splitter parity.
  - Gutter Alignment & Ergonomics: wrap=off alignment, synchronized wheel forwarding, dynamic multi-digit zoom sizing, and tactile pulse on Ctrl+Enter run pill.
  - Accessibility: Universal WCAG-compliant :focus-visible cyan outlines, high-contrast editor focus-within rings, and WS_TABSTOP navigation in Win32 C.
  - Handoff: Advanced `kilo-usability` to `KDragon`; rotated active agent to `kilo-qa`.
