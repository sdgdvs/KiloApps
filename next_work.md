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
  kilo_creator: kweb://geocities
  kilo_graphics: KColony
  kilo_soundscape: KStellar
  kilo_tester: KNetMap
  kilo_usability: KNetMap
  kilo_qa: KAnomaly
  kilo_expander: KNetMap
  kilo_review: KBreakout
virtual_web_target: kweb://geocities
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
  app: KRogue
  timestamp: '2026-11-03T18:33:09-08:00'
last_planner_run: '2026-11-03T19:34:53Z'
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
- **Active Target**: `kweb://geocities`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `kweb://geocities` | `KiloOS/public/web/geocities.html`, `KiloOS/public/apps/knet.html` | Create *GeoCities Neighborhood Portal ('99 Retro Neighborhoods)* (`geocities.html` <250 KB): SiliconValley, Area51, and Tokyo99 homestead personal web rings with hit counters, under-construction GIFs, guestbook signing, and VFS HTML export. |
| `kweb://demoscene` | `KiloOS/public/web/demoscene.html`, `KiloOS/public/apps/knet.html` | Create *Assembly '99 Demoscene Party Vault* (`demoscene.html` <250 KB): real-time 64K intro Canvas effect switcher (plasma, rotozoomer, 3D starfield, copper bars, metaballs), ANSI `.NFO` viewer with VFS save, and party voting board. |
| `kweb://rfc-archive` | `KiloOS/public/web/rfc_archive.html`, `KiloOS/public/apps/knet.html` | Create *IETF 1999 Living RFC Workbench* (`rfc_archive.html` <250 KB): interactive RFC 1149 (IP over Avian Carriers) packet loss simulator, RFC 2324 (`418 I'm a teapot`) HTCPCP console, ASCII TCP state stepper, and VFS `.txt` export. |
| `kweb://webring` | `KiloOS/public/web/webring.html` | Integrate new canonical nodes (`slashnet`, `trapster`, `cabled`, `scallion`, `demoscene`, `rfc-archive`) into RingMaster topology map, badge generator, and random/next ring traversal. |
| `kweb://deep-core` | `KiloOS/public/web/deep_core.html` | Connect Tier 3 Ghost Node terminal to KiloOS VFS (`/sys/alpha.sys` & `/home/crypto/`) so decoded fragments can be written directly to VFS for `KMatrix` verification. |
| `kweb://slashnet` | `KiloOS/public/web/slashnet.html`, `KiloOS/public/apps/knet.html` | Create *SlashNet: News for Nerds (1999)* (`slashnet.html` <250 KB): interactive Karma moderation (`-1 Troll` to `+5 Insightful`), threshold comment filter (`-1` to `+5`), Beowulf cluster calculator, Linux 2.2 kernel compile mini-sim, VFS article export, and route `kweb://slashnet` in `knet.html` / `portal.html`. |
| `kweb://trapster` | `KiloOS/public/web/trapster.html`, `KiloOS/public/apps/knet.html` | Create *Trapster P2P Chiptune & Tracker Exchange v1.0b* (`trapster.html` <250 KB): searchable 1999 P2P node directory, simulated 56k/T1 download queue, live YM2612/SPC700 track previewer, VFS `.mod`/`.trk` save to `/home/music/`, and route `kweb://trapster` in `knet.html`. |
| `kweb://cabled` | `KiloOS/public/web/cabled.html`, `KiloOS/public/apps/knet.html` | Create *Cabled Magazine 1999 Neon Edition* (`cabled.html` <250 KB): late-90s cyber-magazine with interactive Y2K Readiness Audit, Dot-Com IPO Bubble Stock Simulator, Jargon Watch generator, Hype-vs-Reality matrix, and route `kweb://cabled` in `knet.html`. |
| `kweb://scallion` | `KiloOS/public/web/scallion.html`, `KiloOS/public/apps/knet.html` | Create *The Scallion: 1999 Satirical News* (`scallion.html` <250 KB): interactive late-90s tech/culture satire portal, "Ask A Sysadmin" interactive column, Infographic & Headline generator, VFS clipping export, and route `kweb://scallion` in `knet.html`. |
| `kweb://warez` | `KiloOS/public/web/warez.html`, `KiloOS/public/apps/knet.html` | Create *The Underground Courier / Warez BBS Web Ring* (`warez.html` <250 KB): ANSI top-site release board, simulated keygen music player, download quota tracker, and VFS export. |

### 2. Game Content & Graphics Queue (kilo-graphics)
- **Active Target**: `KColony`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| KColony | KiloOS/public/apps/kcolony.html, KColony/main.c | Colony simulation visuals: planetary structures, colonists, exploration rovers, alien megafauna, terraforming atmosphere overlays, and glint sweep. |
| KTrader | KiloOS/public/apps/ktrader.html, KTrader/main.c | Galactic trade visuals: merchant vessel chassis, starport market stalls, trade good icons, interstellar pirate silhouettes, and glint sweep. |
| KMine | KiloOS/public/apps/kmine.html, KMine/main.c | Subterranean excavation crawler visuals: drill rigs, mineral veins, seismic hazard indicators, ore refinery equipment, and glint sweep. |
| KChrono | KiloOS/public/apps/kchrono.html, KChrono/main.c | Temporal paradox visual suite: time-portal distortion effects, historic epoch landmark cards, timeline branching node glyphs, and glint sweep. |
| KStarDredge | KiloOS/public/apps/kstardredge.html | Deep space salvager visuals: derelict hull wreckage sprites, magnetic dredge beam effects, cosmic anomaly silhouettes, and glint sweep. |
| KStarship | KiloOS/public/apps/kstarship.html | Starship bridge & sector tactical display upgrade: detailed hull subsystem schematics, nebula parallax backdrop, enemy cruiser silhouettes, and glint sweep. |
| KCyber | KiloOS/public/apps/kcyber.html, KCyber/main.c | Netrunner icebreaker deck visual polish: isometric node towers, 5 distinct IC guardian constructs, matrix grid perspective, and glint sweep. |
| KStellar | KiloOS/public/apps/kstellar.html, KStellar/main.c | 4X stellar empire map visual upgrade: star spectral class rendering, fleet ship icons, orbital ringworlds, and glint sweep. |
| KCosmic | KiloOS/public/apps/kcosmic.html, KCosmic/main.c | Cosmic space exploration & trade visuals: celestial bodies, cosmic anomaly nebulae, jump-gate warp animations, and glint sweep. |
| KWizard | KiloOS/public/apps/kwizard.html | Arcane wizard duel visuals: elemental spell particle effects, grimoire rune illustrations, wizard tower backdrop, and glint sweep. |
| KRogue | KiloOS/public/apps/krogue.html, KRogue/main.c | Dungeon crawler sprite overhaul: hero chassis/classes, procedural dungeon floor/wall tiles, 5 creature archetypes, item icons, and glint sweep. |

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Active Target**: `KStellar`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KStellar` | `KiloOS/public/apps/kstellar.html` | Compose grand 4X galactic diplomacy & fleet command 6-channel suite (Lydian brass fanfare, floating Maj9 pad voicings, hyperdrive & treaty earcons). |
| `KSubmarine` | `KiloOS/public/apps/ksubmarine.html` | Compose abyssal sonar & trench descent 6-channel suite (Suspended `sus2/Maj7` cascade, muffled low-pass SPC700 echo, warm sine sub-bass, hydrophone ping earcons). |
| `KMaze` | `KiloOS/public/apps/kmaze.html`, `KMaze/main.c` | **[ARGON REVIEW]:** Replace bare single-oscillator playTone() in kmaze.html with a 6-channel YM2612 2-Op FM and cavernous SPC700 echo dungeon soundtrack (adapting across Catacombs/Cyber/Frost/Abyssal/Inferno biomes and Minotaur Overlord boss combat) plus [M] HUD mute toggle and procedural footstep, pickaxe cleave, trap, and shrine earcons. |
| `KQuest` | `KiloOS/public/apps/kquest.html` | Compose overworld & battle 6-channel JRPG chiptune suite (Royal Road `IV-V-iii-vi` town/overworld + Heroic `bVI-bVII-I` combat cadence, E-Piano & flute counterpoint, SPC700 echo) + spell/quest earcons. |
| `KStarship` | `KiloOS/public/apps/kstarship.html` | Compose bridge alert & deep-space exploration 6-channel FM suite (Lydian `#4` pad + tactical Phrygian Dominant combat march, sonar/shield/warp earcons). |
| `KAlchemy` | `KiloOS/public/apps/kalchemy.html` | Compose mystical alchemical workshop 6-channel suite (Dorian/Lydian celesta bells, warm glass pad, bubbling FM cauldron transients, transmutation fanfare stingers). |
| `KWizard` | `KiloOS/public/apps/kwizard.html` | Compose arcane tower duel 6-channel chiptune suite (Harmonic Minor harpsichord/flute counterpoint, walking FM bass, spell-cast elemental earcons). |
| `KCyber` | `KiloOS/public/apps/kcyber.html` | Compose 1999 cyberpunk netrunner breakbeat 6-channel FM suite (Dorian acid FM bass, syncopated 5:3 breakbeat drums, cold digital bell arpeggios, ICE breach alarms). |

### 4. App Tester Queue (`kilo-tester`)
- **Active Target**: `KNetMap`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KNetMap` | `KiloOS/public/apps/knetmap.html`, `KNetMap/main.c` | **[ARGON REVIEW]:** Fix createNode() and loadPresetTopology() in knetmap.html to match node.subnetId via isIpInSubnet() (assigning sub-dmz to GW-BORDER-7200, FW-BASTION, and SW-DMZ-3CON instead of sub-eng), implement drawCollisionDomains() in drawScene() when #chkCollisionDomains is checked, fix onTouchStart() calling undefined updateInspector() instead of updateInspectorPane(), dynamically update #cidrStatsLabel/#cidrParentLabel in updateCidrBar() and wire oninput=recalcSubnetTool() + dynamic split child preview in #subnetModal, dispatch an initial ICMP ping pulse on preset load, and in KNetMap/main.c compute full IP/mask/range in CalculateSubnet(), make ID_BTN_CUT enter click-to-sever link mode (g_activeMode=3), and route WM_KEYDOWN in MainEntry GetMessageA loop. |
| `KColony` | `KiloOS/public/apps/kcolony.html`, `KColony/main.c` | **[ARGON REVIEW]:** Preserve unlimited resources (maxPower = 9999, power = 9999, food = 9999, mat = 9999, advm = 9999, sci = 9999) during gameTick() when gameMode === 4 (Sandbox Colony) instead of clamping maxPower to 50 + batCount*50, and allow Space key on #start-screen when tick === 0 to launch startGame(0). |
| `KFortress` | `KiloOS/public/apps/kfortress.html`, `KFortress/main.c` | **[ARGON REVIEW]:** Fix tower target priority in update() (kfortress.html & KFortress/main.c) to rank enemies by path progress (e.waypointIndex * 10000 - distToNextWaypoint) instead of adding tower distance; wire missing mutator effects (mutators.eclipse +30% tower fire rate, mutators.phaseshift +75% kill gold, mutators.titan 2x boss spawns); and route poison/trap/chain kills through the unified death handler so boss kills award 100g and increment hsBoss. |
| `KMystery` | `KiloOS/public/apps/kmystery.html`, `KMystery/main.c` | **[ARGON REVIEW]:** Rebalance impossible Medium/Hard hour budgets to Easy 18h / Medium 20h / Hard 22h and fix advanceTime()/AdvanceTime() so reaching 0h finishes the current action and still permits final Accuse [A] (only failing if hours > timeLeft); fix resetScannerFreq() to restore moves=12 instead of decrementing moves--, clamp scanner current >= 0, and add Reset [R] to KMystery/main.c; call dead spawnShockwave() on clue/scanner/lie/win events, mute rainGain and sfxGain in NoirAudioEngine.toggleMusic(), and route WM_KEYDOWN in KMystery/main.c GetMessageA loop so child-control focus stops breaking hotkeys. |
| `KMaze` | `KiloOS/public/apps/kmaze.html`, `KMaze/main.c` | **[ARGON REVIEW]:** Change default speed keybind from s/S to b/B (and migrate saved kmaze_keybinds) so walking backward with S stops firing useSpeedShoes(); stop tryMove()/TryMove() from auto-consuming hasPickaxe when brushing normal walls (1,20,21,22,27) and require [P] for cracked walls (7); render computed isPathTile[cellX][cellY] as a glowing golden trail in floor casting when pathfinderTimer > 0; grant Save Shrine (tile 28) +1 charges/+300 score BEFORE saveCheckpointGame(); remove permanent speedBoost=1 on tile 9 pickup; use drawPX/drawPY/drawMap in wall/light raycasting so Replay Mode renders accurately; handle waitingForKey before global hotkeys in keydown; and decay/clear shockwaves[] in both kmaze.html and KMaze/main.c. |
| `KGo` | `KiloOS/public/apps/kgo.html`, `KGo/main.c` | **[ARGON REVIEW]:** Add isAIMove flag to placeStone(x, y, isAIMove) in kgo.html and PlaceStone(hwnd, x, y, isAIMove) in KGo/main.c so the VS-AI turn guard stops blocking makeAIMove()/MakeAIMove() (and clear pending AI timeouts on reset/undo/replay/load); stop calculateScore()/CalculateScore() from wiping board and moveRecord via initBoard() so finished games can be replayed or exported to SGF, and add interactive dead-stone click-to-mark scoring review plus live Black/White capture and territory counts in HUD; fix Tsumego puzzles in kgo.html and KGo/main.c (Stage 7 Crane's Nest enclosure and occupied (4,2) check, Stage 11 straight-three vital point (1,0) only, Stage 14 center net, Goban A9/G13 coord labels, native PlaceStone Tsumego win check, and standalone Tsumego progression + Retry); fix Replay off-by-one (#btn-replay-first -> -1, #replay-slider -> value - 1); guard all single-key hotkeys when typing in INPUT/TEXTAREA/SELECT; and use BS_PUSHBUTTON + SetFocus(hwnd)/GetMessage key routing in KGo/main.c. |
| `KTrader` | `KiloOS/public/apps/ktrader.html`, `KTrader/main.c` | **[ARGON REVIEW]:** Call dead advanceFuturesOnJump() and updateDuelOnJump(target) inside travel() after generatePrices() so futures settle and 10-jump duels progress, serialize futuresContracts and futuresHistory in getSavePayload(), add broadcastDuelState() to buyMaxGood() and sellAllGood(), wire [U] and [G] hotkeys, and add #futures-modal and #multiplayer-modal to closeAllModals(), Escape/activeModal focus trap, and SELECT keydown guards. |
| `KRogue` | `KiloOS/public/apps/krogue.html`, `KRogue/main.c` | Audit roguelite dungeon crawler, turn-based combat, procedural room generation, inventory management, and Win32 C parity. |
| `KDragon` | `KiloOS/public/apps/kdragon.html`, `KDragon/main.c` | **[ARGON REVIEW]:** Fix buyItem() calling getItemPrice(number) instead of string keys ('food','toy','nectar','crown','polish') which charges 50g for all items; fix getMood().label === 'Ecstatic' in startMinigame() and battleTurn() (+15% crit); and apply the +10 Sovereign Aura stat bonus upon collecting all 6 relics. |
| `KCyber` | `KiloOS/public/apps/kcyber.html`, `KCyber/main.c` | **[ARGON REVIEW]:** Stop accept from overwriting native node_files and stop download in kcyber.html from double-paying 2x contract reward (keep contract file separate alongside node paydata in ls/download and persist/award active_mission_reward once in both kcyber.html and KCyber/main.c); fix KCyber/main.c WM_TIMER (id 2) attacking while connected_node is active after PIN crack and call KillTimer(hwnd, 2) on breach/abort/fail/reboot; handle reboot globally before in_shop/hacking_node/connected_node branches (resetting all 3 modes + updatePrompt) so the toolbar Reboot button never fails; persist ice_slowed in F5/F9 saves, null-guard audioCtx.state + add e.stopPropagation() on modal Esc in cmdInput keydown so closing F1 Help never aborts an active hack, pause renderLoop and ice_interval when document.hidden, and add connect 2-6 shorthand + VK_UP/VK_DOWN history to KCyber/main.c. |
| `KStellar` | `KiloOS/public/apps/kstellar.html`, `KStellar/main.c` | **[ARGON REVIEW]:** Fix KStellar/main.c dead station buttons in WM_COMMAND by implementing missing handlers for ID_BTN_BUY_MED (108), ID_BTN_SELL_MED (109), ID_BTN_REPAIR (110), and ID_BTN_REFUEL (111), include cargoMedicine in all cargoMax checks, hide hBtnBuyMed/hBtnSellMed/hBtnRepair/hBtnRefuel in ShowManualView, non-docked WM_LBUTTONDOWN, and combat start so they stop overlapping Manual/Combat controls, add Emergency Distress Beacon fallback when stranded without fuel, implement missing phenomenon 4 (Alien Monolith) and 5 (Ion Storm) rewards in ID_BTN_INVESTIGATE + WM_PAINT rings, spawn/label enemyClass 3 (Stealth Infiltrator) properly while fixing engineLevel flee scaling and shieldLevel damage absorption in combat, fix GDI HBRUSH leaks in WM_DRAWITEM, route WM_KEYDOWN in GetMessage loop so button clicks stop breaking hotkeys (mapping M to sound mute and H/F1 to Manual); in kstellar.html fix saveGameState/loadGameState quota fallback to persist s.market (food, minerals, tech, medicine) instead of undefined s.food_price, make emergencyBeacon() add +30% fuel (clamped to 100) to prevent >30%-jump soft-locks, and pause renderCanvasLoop rAF when document.hidden. |
| `KImage` | `KiloOS/public/apps/kimage.html`, `KImage/main.c` | **[ARGON REVIEW]:** Fix all 4 dead Optical DSP filters in kimage.html (applyVignetteFilter, applyChromaticAberrationFilter, applyMedianFilter, applyHistogramEqualization) where mutatePixels passes 3 args (d, w, h) instead of (src, dst, w, h) (copy const src = new Uint8ClampedArray(d) so pixels actually update, default #sliderVignette to 45 and #sliderChromatic to 6 with a toast if 0, and sync #playlistStrip thumbnails when item.img mutates); add missing link.href = url in exportSessionJson(); reset rotation/flip/draw.paths/adjustments inside encodeStegoPayload() after baking ui.mainCanvas so renderCanvas() stops re-applying CSS filters and destroying LSB bits, and slice UTF-8 bytes via extracted.subarray(16, 16 + byteLen) in decodeStegoPayload(); support imgType 3 in parseTGA(), 256-color 0x0C VGA palettes in parsePCX(), and 1999-11-03 EXIF date in loadSampleImage(); in KImage/main.c restore hwnd focus / route WM_KEYDOWN after button clicks so all 16 hotkeys work and Space stops re-clicking toolbar buttons, and guard ID_BTN_PLAY when g_fileCount == 0. |
| `KScript` | `KiloOS/public/apps/kscript.html`, `KScript/main.c` | **[ARGON REVIEW]:** Fix broken multi-line string literals in PRESETS.vfs_demo caused by unescaped \n inside template literal, implement documented 0o octal literals in parsePrimary() and loop/endloop control flow in createScriptStepper(), strip string literals before identifier scanning and support compound assignments in analyzeScript() to eliminate false-positive linter warnings on DEFAULT_SCRIPT, and HTML-escape renderMemoryTable() cells/copy handlers to prevent XSS and quote SyntaxErrors. |

### 5. Usability & UX Queue (`kilo-usability`)
- **Active Target**: `KNetMap`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KNetMap` | `KiloOS/public/apps/knetmap.html`, `KNetMap/main.c` | **[ARGON REVIEW]:** Update resetView() in knetmap.html to auto-fit and center the TOPOLOGY.nodes bounding box (with 55px label/hull padding) inside canvasContainer and compact preset node Y-coordinates (y=75..345) so all 8 Corp DMZ nodes and labels render 100% visible in the 905x428 canvas; compact aside.palette (2-column grids for Add Hardware, Diagnostics, and Media Standard with tighter gaps) so all tools, cable selectors, overlays, and Clear Grid fit at 1080x683 without vertical scroll clipping; and add Arrow/WASD viewport/node navigation and Enter ping trigger in onKeyDown. |
| `KImage` | `KiloOS/public/apps/kimage.html`, `KImage/main.c` | **[ARGON REVIEW]:** Define missing .hidden (display none !important) and .row (display flex; align-items center; gap 8px) CSS in kimage.html so #emptyState hides on image load (fixing sheet.png Panel 3 canvas clipping), inactive #tabKernel..#tabDraw and #rightStegoContent stop stacking simultaneously, #cropControls stays hidden until enabled, and Kernel/Resize/ASCII .row inputs align side-by-side; auto-show #rightPanel (unless user-toggled off) with #btnToggleRight.active and live renderHistogram() on image load so #histCanvas is not blank black; fit .toolbar on 1 row at 1060px (--panel-width 255px, nowrap compact buttons, remove redundant Press H/F1 subtitle), cap #toastContainer to 2 toasts, and in KImage/main.c use 9pt Segoe UI (-MulDiv(9,dpi,72)) and clean ASCII button/help labels (RotL, RotR, <, Play, Pause, >, -) to fix clipped 12pt UTF-8 mojibake. |
| `KFortress` | `KiloOS/public/apps/kfortress.html`, `KFortress/main.c` | **[ARGON REVIEW]:** Convert .tower-selection in .sidebar to a compact 2-column CSS grid (grid-template-columns: 1fr 1fr; gap: 4px) and tighten .sidebar padding/gaps so all 7 towers, 4 traps, modes, map switcher, #waveBtn, #academyBtn/#resetBtn, and HERO & SKILLS [1-5] fit 100% visible inside the 1024x703 KiloOS viewport without vertical scroll clipping; also add right-click / [Delete] tower demolish for 70% gold refund. |
| `KMystery` | `KiloOS/public/apps/kmystery.html`, `KMystery/main.c` | **[ARGON REVIEW]:** Compact #help-modal > div padding/margins and 2-column Tips/Guide layout so first-boot manual fits 960x673 without mid-line text clipping; preserve [S]/[I] hotkeys and show hour costs (2h/1h) on #search-btn, #interrogate-btn, #lab-btn, and #travel-buttons; fix #scanner-game button labels from kHz to Hz; persist discovered suspect alibis and CAUGHT/NERVOUS reactions in #suspect-list (plus patience dots and [PRESENT] in KMystery/main.c hListSuspects); and wire #scene-canvas click hotspots. |
| `KChess` | `KiloOS/public/apps/kchess.html`, `KChess/main.c` | Shift chessboard left (TS=64, OX=64, OY=124, Eval Bar at x=18..34, left rank labels at x=48, right rank labels at x=590) to fix drawEvalBar() colliding with rank 8..1 labels, fix KChess/main.c capRc ({20,46,W-20,94}) painting over top file labels and Rank 8, and render a live interactive Move History & Clocks panel (x=606..744, y=118..642) in both kchess.html and KChess/main.c with White/Black clocks, scrollable 2-column SAN move list, active-ply highlight, wheel scroll, and click-to-jump; add warm champagne/ivory outer rim stroke on Black pieces (!isWhite) in drawChessPieceSprite() and DrawChessPiece() for dark-square contrast; compact #help-modal .modal-card (2-col shortcuts) so #help-close-btn fits 800x903 without clipping, add white-space nowrap to .menu-bar/#menu-status-badge/.menu-btn-quickhelp (removing duplicate canvas Help badge), and move #toast-container to top:48px right:16px while skipping redundant boot welcome toast; add pointer drag-and-drop piece movement in kchess.html and KChess/main.c (WM_MOUSEMOVE/WM_LBUTTONUP), wire ArrowLeft/Right in #replay-modal to replayStepPrev()/replayStepNext(), let focused modal buttons handle Enter/Space without primaryBtn hijacking, and replace prompt() in editorExportFEN() with #fen-modal. |
| `KGo` | `KiloOS/public/apps/kgo.html`, `KGo/main.c` | **[ARGON REVIEW]:** Eliminate 216px vertical overflow at 700x723 in kgo.html by locking body to 100vh flex column with overflow hidden, merging h1 and .status-container into a compact top bar, collapsing the redundant 3-row #hotkey-bar (and removing misleading /WASD label since S toggles Analyzer), tightening .board/.board-container/.controls padding, and making getCellSize() viewport-responsive so 9x9, 13x13, and 19x19 boards plus all controls fit 100% visible with 0px scroll; raise kbd, badge, .stone-num-label, .liberty-badge, and .ko-overlay font sizes to >= 10px (0 tiny-text warnings); remove random rotate() on .stone so 3D lighting stays top-left and move numbers (M) never render upside-down, and append .last-move-aura/.ripple/.ripple-outer to .cell instead of overflow-hidden .stone; replace blocking alert() dialogs for Stats/Campaign/Score with Esc-dismissible in-DOM modals, pause idle updateScreenShake rAF loop, and change KGo/main.c Goban coordinates from invisible mahogany RGB(75,48,24) to gold RGB(255,220,110). |
| `KCosmic` | `KiloOS/public/apps/kcosmic.html`, `KCosmic/main.c` | Orbit & navigation viewport ergonomics, touch/keyboard flight controls, HiDPI radar canvas scaling, and compact HUD. |
| `KAnomaly` | `KiloOS/public/apps/kanomaly.html`, `KAnomaly/main.c` | Subterranean waterfall spectrogram HiDPI scaling, frequency tuner wheel/keyboard precision (`ArrowLeft/Right`), and clear decoder telemetry readout. |
| `KTrader` | `KiloOS/public/apps/ktrader.html`, `KTrader/main.c` | **[ARGON REVIEW]:** Convert absolute 12-button top toolbar into non-overlapping flex header so it stops colliding with H1 title at 960x683, compact layout so Shipyard/Missions/Log fit without 807px vertical scroll overflow, add flex-wrap and upgrade stat labels (+20 Cargo/+10 Shield, -2 Fuel/+50 Tank, +15 Dmg) to #ui-shipyard, remove misleading [1]-[5] prefixes from #market-table rows, and add devicePixelRatio HiDPI scaling to #gfx-canvas. |
| `KDragon` | `KiloOS/public/apps/kdragon.html`, `KDragon/main.c` | **[ARGON REVIEW]:** Compact #main-container, .stats, .entity-display, #controls, #shop-controls, and .message-log to fit 660x623 without vertical scroll, add preventDefault() for Arrow/Space keys so gameplay never scrolls the page, and prevent #toast-container from covering #controls and #message-log. |
| `KColony` | `KiloOS/public/apps/kcolony.html`, `KColony/main.c` | **[ARGON REVIEW]:** Arrange #start-screen buttons in a 2-column grid so all 11 fit within 870x623 without bottom clipping; lock body to 100vh overflow:hidden with scrollable .panel-col drawers (max-height:460px; overflow-y:auto) and e.preventDefault() on Arrow/Space keys; resize #grid, #grid-container, #bg-canvas, and #fg-canvas from 400x400 to 419x419 to match the 20x20 cells + 19px gaps; raise small font-size from 8px to 9.5px. |
| `KHex` | `KiloOS/public/apps/khex.html`, `KHex/main.c` | **[ARGON REVIEW]:** Fix 980x783 layout and UX issues in khex.html (sheet.png Panel 3): add white-space nowrap to #entropyClassBadge and Shannon Entropy header row, change .output-box from word-break break-all to overflow-wrap break-word so #sigDetailsBox stops splitting words mid-character, and pass silent=true from jumpOffsetStep() to gotoOffset() so Arrow-key byte navigation stops spamming #toastContainer popups over #byteFreqHistogram; add flex-wrap wrap (gap 6px) to Tab 2 .panel-title/.form-row and tighten #rawTextBuffer min-height to 64px; guard global keydown handlers when e.target.tagName === 'SELECT', refresh active Tab 2/3/0 analyses on Swap Endian [E], make #searchResultsBox match offsets clickable to jump+highlight in Hex View, read actual 8 buffer bytes (or zero-extended uint64) for #valDouble, and list Tab [0] Struct & Signatures in #helpModal and #tutorialModal. |
| `KScript` | `KiloOS/public/apps/kscript.html`, `KScript/main.c` | **[ARGON REVIEW]:** Compact 3-row header .toolbar and 2-row .presets-bar vertical bloat at 1040x643 and set white-space nowrap on #panelEditor .panel-header so Script Editor does not wrap, map AST category keys in renderAstDistribution() (STMT_ASSIGN, STMT_BRANCH_*, STMT_LOOP_*, STMT_PRINT, EXPRESSION) to distinct colors and short labels so #astCanvas is not solid purple with overflowing legend text, raise .gutter contrast from #475569 to >= #94a3b8, differentiate Print/Return/trace lines in #outputArea, move .toast-container to bottom 42px so toasts do not cover zoom controls, and unify v2.2 labels. |

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial, State & Mature Graduation)
- **Active Target**: `KAnomaly`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KAnomaly` | `KiloOS/public/apps/kanomaly.html`, `KAnomaly/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, subterranean waterfall spectrogram, audio decoder telemetry, dual Win32 C parity, and promote to Mature table. |
| `KBBS` | `KiloOS/public/apps/kbbs.html`, `KBBS/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, ANSI terminal bulletin board, door games, VFS file transfer, dual Win32 C parity, and promote to Mature table. |
| `KSynth` | `KiloOS/public/apps/ksynth.html`, `KSynth/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, 2-Op FM patch synthesizer, envelope generator, piano roll, dual Win32 C parity, and promote to Mature table. |
| `KNet` | `KiloOS/public/apps/knet.html`, `KNet/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, Sovereign Web 1.0 browser, bookmarks, history, VFS spooling, dual Win32 C parity, and promote to Mature table. |
| `KPing` | `KiloOS/public/apps/kping.html`, `KPing/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, ICMP network ping graph, host reachability radar, dual Win32 C parity, and promote to Mature table. |
| `KChrono` | `KiloOS/public/apps/kchrono.html`, `KChrono/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, timeline event organizer, epoch markers, dual Win32 C parity, and promote to Mature table. |
| `KMine` | `KiloOS/public/apps/kmine.html`, `KMine/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, mine excavation crawler, ore inventory telemetry, dual Win32 C parity, and promote to Mature table. |
| `KDB` | `KiloOS/public/apps/kdb.html`, `KDB/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, SQL/relational table operations, CSV/JSON dataset export, query optimizer tests, dual Win32 C parity, and promote to Mature table. |
| `KFont` | `KiloOS/public/apps/kfont.html`, `KFont/main.c` | Pass 5 QA & Mature Graduation: F5/F9 state persistence, font editor glyph grid, export BDF/TTF preview, dual Win32 C parity, and promote to Mature table. |

### 7. Feature Expander Queue (`kilo-expander` — Phase VI VFS Mesh & RFMS Multiplayer)
- **Active Target**: `KNetMap`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KNetMap` | `KiloOS/public/apps/knetmap.html`, `KNetMap/main.c` | **[ARGON REVIEW]:** Wire Phase VI KiloOS VFS and IPC Mesh into knetmap.html per architecture.md (event.origin === window.location.origin checks, window.location.origin targetOrigin, 60000ms picker timeouts, standalone Blob/file-input fallbacks) adding Export/Import VFS (/home/saves/knetmap_topology.json via VFS_SHOW_SAVE/VFS_WRITE + OS_NOTIFY and VFS_SHOW_OPEN/VFS_READ), incoming FILE_OPEN/VFS_OPEN topology loading, and Export Capture to VFS (/home/docs/knetmap_capture.log) + Inspect in KPad (OS_LAUNCH_APP kpad) + Launch KPing (kping) buttons in .drawer-actions; add Traceroute [T] mode and rubber-band link preview to KNetMap/main.c. |
| `KChess` | `KiloOS/public/apps/kchess.html`, `KChess/main.c` | **[ARGON REVIEW]:** Wire Phase VI KiloOS VFS & IPC Mesh into kchess.html using VFS_SHOW_SAVE -> VFS_WRITE + OS_NOTIFY and VFS_SHOW_OPEN -> VFS_READ with mandatory event.origin === window.location.origin checks, 60s dialog cleanup timeouts, and standalone Blob download / file-input fallbacks when window.parent === window for .json game saves (/home/saves/kchess_save.json) in Game menu, .pgn match records (/home/docs/kchess_match.pgn) in #pgn-modal, and .fen positions (/home/docs/kchess_position.fen) in #fen-modal and #editor-modal, plus handle incoming FILE_OPEN / VFS_OPEN messages from KiloOS File Browser to auto-load .pgn/.fen/.json files and add an Open PGN in KPad IPC button (VFS_WRITE + OS_LAUNCH_APP appId kpad) in #pgn-modal. |
| `KGo` | `KiloOS/public/apps/kgo.html`, `KGo/main.c` | **[ARGON REVIEW]:** Wire Phase VI KiloOS VFS and IPC Mesh into kgo.html (#sgf-modal and boot listener) using VFS_SHOW_SAVE -> VFS_WRITE + OS_NOTIFY and VFS_SHOW_OPEN -> VFS_READ with event.origin === window.location.origin checks, 60s dialog timeouts, and standalone Blob/file-input fallbacks when window.parent === window for .sgf match records (/home/docs/kgo_match.sgf) and .json saves (/home/saves/kgo_save.json), handle incoming FILE_OPEN/VFS_OPEN messages from File Browser to auto-load .sgf/.json files, add an Open SGF in KPad IPC button (VFS_WRITE + OS_LAUNCH_APP appId kpad), and add SGF file import parity to KGo/main.c. |
| `KImage` | `KiloOS/public/apps/kimage.html`, `KImage/main.c` | **[ARGON REVIEW]:** Wire Phase VI KiloOS VFS and IPC Mesh into kimage.html per architecture.md (event.origin === window.location.origin checks, window.location.origin targetOrigin, 60000ms picker timeouts, standalone Blob/file-input fallbacks when window.parent === window) adding Open from VFS (VFS_SHOW_OPEN -> VFS_READ) and Save to VFS (VFS_SHOW_SAVE -> VFS_WRITE + OS_NOTIFY) for exported images (/home/media/), .JSON sessions (/home/saves/kimage_session.json), and ASCII .txt art (/home/docs/), handling incoming FILE_OPEN/VFS_OPEN messages and URL #/path hash to auto-load .png/.jpg/.bmp/.tga/.pcx/.json files, and adding Open ASCII in KPad (OS_LAUNCH_APP kpad), Send Stego to KCipher (kcipher), and Inspect Image in KHex (khex) buttons; upgrade KImage/main.c (<999 KB) by wiring ID_BTN_RESIZE_HALF (0.5x) and ID_BTN_RESIZE_DOUBLE (2x) and adding a Dither [T] Floyd-Steinberg retro palette button cycling 1-Bit Mono, DMG-4 Green, CGA, and C64 palettes. |
| `KHex` | `KiloOS/public/apps/khex.html`, `KHex/main.c` | **[ARGON REVIEW]:** Harden Phase VI KiloOS VFS and IPC Mesh in khex.html per architecture.md: enforce event.origin === window.location.origin in window.addEventListener('message'), post with window.location.origin instead of wildcard (removing dead OS_SET_WINDOW_SIZE), raise sendToOS() timeout to 60000ms for interactive VFS_SHOW_OPEN and VFS_SHOW_SAVE pickers, emit OS_NOTIFY after VFS_WRITE, handle incoming FILE_OPEN/VFS_OPEN messages to auto-load VFS files, add Save to VFS (/home/docs/ or /home/crypto/) and Open in KPad / Send to KCipher OS_LAUNCH_APP buttons in Tab 7 Export, Tab 8 Patch Builder, and Tab 9 Disassembler, and add native Win32 .bin file open/save parity in KHex/main.c (<999 KB). |
| `KAnomaly` | `KiloOS/public/apps/kanomaly.html`, `KAnomaly/main.c` | Phase VI VFS Signal Bridge: allow exporting demodulated subterranean signal frames directly to `/sys/anomaly_capture.sig` in KiloOS VFS and launching `KMatrix` or `KHex` via `OS_LAUNCH_APP` for cross-app decoding. |
| `KTrader` | `KiloOS/public/apps/ktrader.html`, `KTrader/main.c` | **[ARGON REVIEW]:** Build interactive 12-system Galactic Star Chart [C] (plus KTrader/main.c parity) rendering planets (x,y) jump-gate topology, ecoType/techLevel badges, current location, and active mission target highlight, add player cur/max shield bars to #ui-combat-panel, and enforce Navy patrol confiscation when jumping with Contraband cargo. |
| `KSynth` | `KiloOS/public/apps/ksynth.html`, `KSynth/main.c` | Phase VI VFS Audio Patch Bank: Allow exporting/importing 2-Op FM patch presets (`.fm`) to `/home/music/` and playing tracks in KRadio/Trapster. |
| `KNet` | `KiloOS/public/apps/knet.html`, `KNet/main.c` | Phase VI VFS & TINAG Hardening: remove Tier 3 darknet nodes (`darknet`, `deep-core`, `10.19.99.4/classified`, `echo-subsystem.net`) from surface bookmark chips per `arg_plan.md` §6 (keep address-bar routing!), add `"Save Page/Spool to KiloOS VFS"` (`/home/downloads/`), and route new `kweb://` parodies (`slashnet`, `trapster`, `cabled`, `scallion`, `demoscene`, `rfc-archive`). |
| `KBBS` | `KiloOS/public/apps/kbbs.html`, `KBBS/main.c` | Phase VI VFS & Door Game Mesh: allow downloading ANSI `.NFO` / `.TXT` bulletins from KBBS File Area directly into KiloOS VFS (`/home/bbs/`) and launching `KPad` or `KHex` via `OS_LAUNCH_APP` to inspect them. |
| `KMystery` | `KiloOS/public/apps/kmystery.html`, `KMystery/main.c` | **[ARGON REVIEW]:** Wire Phase VI KiloOS VFS Open/Save (/home/saves/kmystery_case.json via VFS_SHOW_OPEN and VFS_SHOW_SAVE postMessage with standalone JSON fallback), add Export Forensic Dossier Report (/home/docs/kmystery_dossier.txt via VFS_WRITE + OS_NOTIFY on case solved) and a Present Clue shortcut inside #dossier-modal, and add Clue Dossier Viewer [C] / double-click inspection parity to KMystery/main.c. |
| `KCyber` | `KiloOS/public/apps/kcyber.html`, `KCyber/main.c` | **[ARGON REVIEW]:** Wire Phase VI KiloOS VFS and IPC Mesh into kcyber.html (event.origin === window.location.origin checks, 60s dialog timeouts, standalone Blob/file-input fallbacks) connecting Export/Import to VFS_SHOW_SAVE/VFS_WRITE and VFS_SHOW_OPEN/VFS_READ (/home/saves/kcyber_deck.json), handling incoming FILE_OPEN/VFS_OPEN messages, exporting stolen node/contract paydata payloads to /home/crypto/ on download with an Inspect in KHex/KPad OS_LAUNCH_APP button, and adding ../assets/js/retro_multiplayer.js NetLink Intrusion Race duel mode strictly behind an explicit toolbar button (zero autostart, soloTimeoutMs: 25000 rival AI runner fallback). |
| `KScript` | `KiloOS/public/apps/kscript.html`, `KScript/main.c` | **[ARGON REVIEW]:** Enforce event.origin === window.location.origin and window.location.origin targetOrigin across all KiloVFS postMessage handlers in kscript.html per architecture.md, populate KiloVFS.cache in the empty VFS_LIST_DONE stub, avoid direct kiloos_vfs_state_v2 writes when embedded in KiloOS, add 60000ms timeouts and standalone fallbacks for VFS Open and VFS_SHOW_SAVE plus Open in KPad IPC in #exportModal, and upgrade KScript/main.c parity with ternary operator for Template 7 Collatz, if/else/endif and while/endwhile in StepScript(), hex/bin/fact built-ins, single-dialog SaveJsonDump, and unclipped hBtnRec label. |

### 8. Argon Visual, Feature & Usability Critic (`kilo-review` — Gemini 4 Argon)
- **Active Target**: `KBreakout`

| Target | Primary Files | Phase VI Mission Spec (<999 KB) |
|---|---|---|
| `KBreakout` | `KiloOS/public/apps/kbreakout.html`, `KBreakout/main.c` | Visual, feature & usability review: evaluate paddle/ball physics responsiveness, CRT scanline shader contrast, high score HUD, YM2612 audio balance, and Win32 C parity. |
| `KSanctuary` | `KiloOS/public/apps/ksanctuary.html`, `KSanctuary/main.c` | Visual, feature & usability review: evaluate xenobiology habitat ecosystem balance, weather overlays, creature sprite rendering, and Win32 C parity. |
| `KGraph` | `KiloOS/public/apps/kgraph.html`, `KGraph/main.c` | Visual, feature & usability review: evaluate Cartesian/polar/parametric function plotting precision, axis label contrast, calculus root finder ergonomics, and Win32 C parity. |
| `KMandel` | `KiloOS/public/apps/kmandel.html`, `KMandel/main.c` | Visual, feature & usability review: evaluate deep fractal zoom coordinate HUD, palette cycling smoothness, landmark tour preset drawer, and Win32 C parity. |
| `KTask` | `KiloOS/public/apps/ktask.html`, `KTask/main.c` | Visual, feature & usability review: evaluate process list sort order, memory telemetry sparklines, kill/suspend modal confirmations, and Win32 C parity. |
| `KColor` | `KiloOS/public/apps/kcolor.html`, `KColor/main.c` | Visual, feature & usability review: evaluate color wheel/slider responsiveness, WCAG contrast ratio indicator legibility, harmony palette generator, and Win32 C parity. |
| `KVoid` | `KiloOS/public/apps/kvoid.html`, `KVoid/main.c` | Visual, feature & usability review: evaluate void survival map visibility, resource depletion HUD, permadeath dialog ergonomics, and Win32 C parity. |
| `KType` | `KiloOS/public/apps/ktype.html`, `KType/main.c` | Visual, feature & usability review: evaluate typing test font readability, WPM/accuracy real-time gauge, keyboard heatmap display, and Win32 C parity. |
| `KZip` | `KiloOS/public/apps/kzip.html`, `KZip/main.c` | Visual, feature & usability review: evaluate archive tree navigation, compression algorithm benchmark view, hex inspector modal, and Win32 C parity. |
| `KContacts` | `KiloOS/public/apps/kcontacts.html`, `KContacts/main.c` | Visual, feature & usability review: evaluate contact card grid responsiveness, search filter speed, vCard/CSV export modal, and Win32 C parity. |
| `KScript` | `KiloOS/public/apps/kscript.html`, `KScript/main.c` | [Argon Audited 2026-11-03: Fixes queued -> kilo-expander, kilo-tester, kilo-usability] Visual, feature & usability review. |
| `KNetMap` | `KiloOS/public/apps/knetmap.html`, `KNetMap/main.c` | [Argon Audited 2026-11-03: Fixes queued -> kilo-expander, kilo-tester, kilo-usability] Visual, feature & usability review. |

---

## Mature Applications (Archived from Active Queues)

> **POLICY**: Applications listed below have reached feature completion, high stability, and mature visual/audio design.
> **EXCLUSION RULE**: These applications are **permanently removed from active Flash worker queues (`### 1.`–`### 7.`)** unless un-quarantined by `kilo-review` (Gemini 4 Argon) via `advance_turn.py --inject-fix` or by the Director.
> **SKIP ADDICTION PREVENTION**: If any agent (except `kilo-graphics` skipping non-game apps) encounters or decides to skip an app because it is complete or mature, it must NOT rotate the app to the queue bottom! Use `python scripts/advance_turn.py ... --quarantine-mature "<summary>"` to record the app in this table and purge it from all active queues.

| Application | Primary File | Pass Count / Date | Category | Summary of Mature State |
|---|---|---|---|---|
| **KSteno** | `KiloOS/public/apps/ksteno.html` | Pass 5+ (2026-11-03) | Security / Steganography & Cryptography | Complete stenographic carrier suite & covert channel workbench with image LSB, 16-bit PCM WAV audio, whitespace/zero-width text modulation, Chi-Square PoV steganalysis, spectral bitplane visualizer, F5/F9 state persistence, and dual Win32 C parity. |
| **KMatrix** | `KiloOS/public/apps/kmatrix.html` | Pass 5+ (2026-11-03) | System / Security & ARG Climax | Complete Milestone #100 retro forensic master terminal with 5-sector cryptographic analysis, 2-Op FM Morse ECHO waterfall telemetry, VFS kernel verification, breach state JSON persistence, F5/F9 state, and dual Win32 C parity. |
| **KSettings** | `KiloOS/public/apps/ksettings.html` | Pass 5+ (2026-11-03) | System / Configuration | Complete KiloOS control panel with 5-tab responsive navigation, live accent/theme synchronization, VFS persistence, F5/F9 state, and dual Win32 C parity. |
| **KTerm** | `KiloOS/public/apps/kterm.html` | Pass 5+ (2026-11-02) | System / Utilities | Complete multi-tab terminal emulator with ANSI escape codes, command history, echo -e, piping, math evaluator, VFS integration, F5/F9 state persistence, and full dual Win32 C parity. |
| **KSolitaire** | `KiloOS/public/apps/ksolitaire.html` | Pass 5+ (2026-11-02) | Games / Card & Board | Complete Klondike solitaire workstation with Classic/Vegas/20-Stage Campaign modes, Mulberry32 PRNG deal selection [F3], undo/redo history [U/Ctrl+Z/Ctrl+Y], win cascade & fireworks, active skills, F5/F9 state persistence, and dual Win32 C parity. |
| **KBreakout** | `KiloOS/public/apps/kbreakout.html` | Pass 5+ (2026-11-02) | Games / Arcade | Arcade brick-breaker with textured metallic/crystalline bricks, plasma paddle/ball trails, boss hazards, YM2612 soundscape, dual Win32 C parity. |
| **KBudget** | `KiloOS/public/apps/kbudget.html` | Pass 5+ (2026-11-01) | Productivity / Finance | Complete retro personal finance ledger and workstation with dual Category Pie & 30-Day Balance Projection forecasts [P], F5/F9 state persistence, dynamic categories, VFS/CSV I/O, and Win32 C parity. |
| **KSanctuary** | `KiloOS/public/apps/ksanctuary.html` | Pass 5+ (2026-11-01) | Games / Simulation | Complete xenobiology vault simulation; Imagen 3 flora/fauna sprites, weather overlays, F5/F9 persistence, Win32 C parity. |
| **KChart** | `KiloOS/public/apps/kchart.html` | Pass 5+ (2026-11-01) | Productivity / Analytics | Complete data visualization studio with 10 chart modes, presets (bar/line/pie/radar & 1999 logs), statistical regressions, CSV/JSON import/export, YM2612 audio sonification, F5/F9 quicksave persistence, and dual Win32 C parity. |
| **KPaint** | `KiloOS/public/apps/kpaint.html` | Pass 5+ (2026-10-31) | Media / Imaging & Graphics Studio | Complete retro paint studio with multi-layer canvas, F5/F9 snapshot persistence, retro palettes (C64/GameBoy/CGA/Amiga), tool drawer, flipbook animation, and Win32 C parity. |
| **KCipher** | `KiloOS/public/apps/kcipher.html` | Pass 5+ (2026-10-31) | System / Security | Complete cryptographic suite with classical ciphers, RC4/XOR streams, AES-128 CTR, RSA demo, pipeline chaining, frequency cryptanalysis, steganography, F5/F9 persistence, and dual Win32 C parity. |
| **KGraph** | `KiloOS/public/apps/kgraph.html` | Pass 5+ (2026-10-31) | System | Scientific function graphing studio with Cartesian, Polar, Parametric plotting, numerical calculus, root finding, CSV dataset import/export, and dual Win32 C parity. |
| **KPad** | `KiloOS/public/apps/kpad.html` | Pass 5+ (2026-10-31) | Office / Productivity | Complete multi-tab text and code workstation with single-row responsive toolbars, dynamic gutter height synchronization under word wrap, regex workbench, Myers diff viewer, AES-256 encryption, live Markdown preview, F5/F9 state persistence, and dual Win32 C parity. |
| **KColosseum** | `KiloOS/public/apps/kcolosseum.html` | Pass 5+ (2026-10-29) | Games / RPG & Simulation | Complete Roman gladiator ludus & arena RPG; RFMS multiplayer duels, F5/F9 persistence, VFS mesh, HiDPI canvas, and dual Win32 C parity. |
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

---

## Recent Execution Logs (Max 5 Entries)

- **2026-11-03T18:33:09-08:00 — kilo-graphics: KRogue (Imagen 3 Visual Assets & Glint Sweep)**
  - Status: PASS ✅ (0 errors, web 356.2 KB, native 82.0 KB < 999 KB).
  - Verified hero, monster, and dungeon texture sprite integration.
  - Fixed sprite aura template literal syntax and confirmed zero glints.
  - Handoff: Advanced `kilo-graphics` to `KColony`; rotated active agent to `kilo-tester`.

- **2026-11-03T18:18:04-08:00 — kilo-creator: kweb://warez (The Underground Courier / Warez BBS Web Ring & VFS Polish)**
  - Status: PASS ✅ (0 errors, web 534.4 KB < 999 KB; startup, Vite & security_lint PASS; visual PASS (anim 0.0%, input 18.55%, HEAD Δ 0.72%, 0 fail, 0 warn, 7.2s)).
  - Enhanced webring navigation with direct Portal and KNet interconnects alongside 0xRELEASE courier top-site board.
  - Synchronized Win32 C parity in KNet/main.c for top-site release board, keygen chiptune player, download quota, and VFS export.
  - Handoff: Advanced `kilo-creator` to `kweb://geocities`; rotated active agent to `kilo-graphics`.

- **2026-11-03T17:31:49-08:00 — kilo-review: KNetMap (Argon Visual, Feature & Usability Review (Dispatched to Flash))**
  - Status: AUDIT COMPLETE (reviewed 4-panel sheet.png at 1080x683 and audited knetmap.html + KNetMap/main.c).
  - Visual/UX Findings: Preset topologies clip bottom switch labels and cut off all 4 bottom-tier servers/workstations below the 428px canvas at default zoom/pan; left .palette overflows vertically hiding Sever Link, Traffic Burst, Media Standard, Overlays, and Clear Grid.
  - Feature/Parity Findings: #chkCollisionDomains is a dead Potemkin checkbox never read in drawScene(); DMZ nodes default to Engineering LAN subnet hull; onTouchStart calls undefined updateInspector(); CIDR strip stats are static; KNetMap/main.c CalculateSubnet() ignores IP input and ID_BTN_CUT only toggles the last link; VFS/IPC mesh is missing.
  - Argon Fix Injection & Handoff: Injected Row-1 specs [`kilo-usability`->`KNetMap`, `kilo-tester`->`KNetMap`, `kilo-expander`->`KNetMap`] (un-quarantined `KNetMap` from Mature); advanced `kilo-review` to `KBreakout`; next agent `kilo-creator`.

- **2026-11-03T17:23:19-08:00 — kilo-expander: KScript (Verified Turn (Auto-Salvaged at Timeout))**
  - Status: PASS ✅ (verified by orchestrator post-turn salvage).
  - Completed kilo-expander updates for KScript across web and native targets.
  - Handoff: Advanced `kilo-expander` to `KChess`; rotated active agent to `kilo-review`.

- **2026-11-03T16:39:45-08:00 — kilo-qa: KSteno (Pass 5 QA, Dual Parity & Mature Graduation)**
  - Status: PASS ✅ (0 errors, web 187.0 KB, native 26.5 KB < 999 KB; startup, Vite & security_lint PASS; visual PASS).
  - State & Tutorial: Verified F5/F9 quicksave/quickload (active tab, bitplanes, carrier payloads/keys), ksteno_tutorialSeen flag, and modal Escape dismissals.
  - Dual Parity & Lifecycle: Verified Web HTML + Win32 C parity (F5/F9 .dat snapshots, first-run briefing, bitplane visualizer, BMP/WAV stego, Chi-Square scan) and visibilitychange audio hygiene.
  - Mature Quarantine & Handoff: Promoted `KSteno` to Mature archive; advanced `kilo-qa` to `KAnomaly`; next agent `kilo-expander`.
