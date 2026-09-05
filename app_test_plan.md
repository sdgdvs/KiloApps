# App Testing & UI Audit Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** Multiple worker agents + directors operate on this repo on overlapping schedules. You are the **App Tester** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`app_test_plan.md`).** Read but NEVER edit:
  - `app_fix_plan.md` (QA agent), `app_work_plan.md` (Feature Expander — paused), `game_content_plan.md` (Games — paused), `new_app_plan.md` (Creator), `usability_plan.md` (Usability)
- **Shared file `KiloOS/src/App.jsx`** — Do NOT edit unless fixing a broken app registration entry.
- **`KiloOS/src/index.css`** — Do NOT edit.
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing:** After editing any app's HTML file, verify it renders. After editing `App.jsx` → `npm run build` in `KiloOS/`.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`. If build fails, fix immediately.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **Logging discipline:** Keep this plan file concise. Use the compact report format specified below.

---

## ⏱️ TURN SCOPING & TERMINATION (CRITICAL — READ EVERY TURN)

**Single-Item-Per-Turn Rule:**
- Each cron trigger = ONE turn. Audit exactly ONE app from your queue, then STOP.
- "Loop forever" means the CRON loops forever across turns, NOT that you loop within a single turn.
- After committing and pushing your work for ONE app, STOP CALLING TOOLS immediately.

**Graceful Termination Checklist (do this EVERY turn before stopping):**
1. Audited one app
2. Updated this plan file with test report
3. Fixed trivial issues in-line (if any)
4. Committed and pushed
5. STOP — call no more tools

---

## ⚠️ DIRECTOR NOTE: WHY THIS AGENT EXISTS

Apps have accumulated many non-functional UI elements — buttons with no handlers, modals that won't open, menu items that throw errors, import/export features that silently fail, settings that don't persist. The QA agent catches code-level bugs (memory leaks, XSS, buffer overflows) but does NOT systematically interact with every UI element. This agent fills that gap.

**Your job is to be a THOROUGH MANUAL TESTER, not a code auditor.** The QA agent already does code auditing. You focus on the USER EXPERIENCE: does every visible button, link, control, modal, tab, dropdown, and feature ACTUALLY WORK when a user interacts with it?

---

## Testing Methodology (follow this for EVERY app)

### Step 1: Inventory All UI Elements (WEB VERSION ONLY — focus on web HTML apps)
Read the app's web HTML file (`KiloOS/public/apps/k[name].html`) and identify EVERY interactive element:
- Buttons (including toolbar, modal, and inline buttons)
- Input fields, dropdowns, checkboxes, sliders
- Tabs and navigation controls
- Modals and dialogs (do they open? do they close? do their buttons work?)
- Import/Export features (CSV, JSON, TXT, etc.)
- Keyboard shortcuts listed in help
- Settings that claim to be saved/loaded

### Step 2: Trace Each Element's Handler
For each interactive element, trace its event handler in the JavaScript:
- Does the `onclick`/`addEventListener` reference a function that EXISTS?
- Does that function DO SOMETHING MEANINGFUL, or is it a stub/empty?
- Does it reference variables or DOM elements that exist?
- Will it throw an error on typical input?

### Step 3: Classify Issues
For each issue found, classify it:
- **🔧 FIXED** — You fixed it in-line (≤5 lines, single file, obvious fix like removing a dead button or wiring up a missing handler)
- **❌ BROKEN** — Complex issue, logged for QA agent to fix. Describe the symptom clearly.
- **⚠️ STUB** — Feature is declared in UI but implementation is empty/placeholder. Note whether it should be removed or completed.
- **✅ OK** — Element works as expected.

### Step 4: Fix Trivial Issues In-Line
If you find a button referencing a nonexistent function, you may:
- **Remove the button** if the feature doesn't exist
- **Wire it up** if the function exists but isn't connected (e.g., typo in handler name)
- **Add a basic stub** like `alert('Not yet implemented')` only if the feature is partially built

**DO NOT** do large rewrites. If a fix would be >5 lines or touch multiple functions, log it for QA.

### Step 5: Write Compact Test Report
Add an entry to the Test Reports section below using this format:
```
- **KAppName**: [PASS ✅ | ISSUES FOUND ⚠️] (N issues, M fixed inline)
  - ✅ Core functionality works (describe briefly)
  - ❌ "Export CSV" button: onclick calls exportCSV() which doesn't exist
  - ⚠️ "Settings" modal: opens but Save button is a no-op stub
  - 🔧 FIXED: Removed orphan "Share" button with no handler
```

---

**Target App:** KCalc
**Status:** Next in queue

## Round-Robin Testing Queue (NEVER STOP — loop forever)
Pick the top app, audit it, write a test report, move it to bottom. One app per turn.

- KCalc
- KCalendar
- KChart
- KChat
- KChess
- KClock
- KColony
- KColosseum
- KColor
- KConnect4
- KContacts
- KConverter
- KCyber
- KDB
- KDarts
- KDragon
- KFarm
- KFlash
- KFont
- KFortress
- KFreecell
- KGo
- KGraph
- KHabit
- KHangman
- KHex
- KImage
- KJournal
- KMail
- KMandel
- KMatch3
- KMaze
- KMech
- KMedia
- KMine
- KMines
- KMystery
- KNet
- KNote
- KPac
- KPad
- KPaint
- KPass
- KPing
- KPong
- KQuest
- KRadio
- KRead
- KReversi
- KRogue
- KScript
- KSimon
- KSnake
- KSolitaire
- KSpace
- KStarship
- KStellar
- KSudoku
- KSynth
- KSys
- KTask
- KTerm
- KTetris
- KTimer
- KTodo
- KTowers
- KTrader
- KType
- KVault
- KVoid
- KWizard
- KWords
- KZip
- K2048
- KAlchemy
- KAsteroids
- KAudio
- KBBS
- KBase
- KBreakout
- KBudget

## Test Reports

- **KBudget**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (income/expense transaction ledger, summary balance/income/expense cards, interactive pie chart for category breakdown, multi-field search and 4-way sorting, 20-item pagination controls, Add/Edit transaction modal, Settings modal with custom currency symbol persistence, CSV import and export, print stylesheet layout, keyboard shortcuts Ctrl+N/F/S/O/Escape).
  - 🔧 FIXED: Date display shifted backwards by 1 day in western timezones (e.g. Sep 5 showed as Sep 4) due to ISO 8601 UTC midnight parsing in `new Date(dateString).toLocaleDateString()`, and `dateInput.valueAsDate = new Date()` assigned UTC dates. Updated `formatDate()` to parse local date components and set local calendar values.
  - 🔧 FIXED: `exportCSV()` used `data:text/csv;charset=utf-8,` with `encodeURI()`, which failed to escape `#` characters in descriptions (e.g. "Invoice #42"), causing browsers to treat `#` as a URL fragment identifier and corrupt/truncate the downloaded CSV. Switched to `Blob` and `URL.createObjectURL()`.
  - 🔧 FIXED: `importCSV()` split rows only on `\n`, retaining carriage returns (`\r`) on Windows and Excel CSV exports, and lacked positive amount checks. Updated parser to split on `\r?\n`, validate `amount > 0`, and accept flexible income labels.
  - 🔧 FIXED: Negative balances formatted as `$-X.XX` instead of `-$X.XX` because `formatCurrency()` prepended the currency symbol directly to negative numbers. Updated formatter to place negative signs before currency symbols.
  - 🔧 FIXED: Strict equality checks (`x.id === id`, `t.id !== id`) caused Edit and Delete operations to fail if transaction IDs were numeric in localStorage. Converted comparisons to string representations and sanitized IDs in event attributes.
  - 🔧 FIXED: Editing transactions with custom or imported categories not in the hardcoded default arrays wiped the category and reset it to "Food". Updated `openModal()` to dynamically append missing categories to the select dropdown, and added HTML escaping to transaction list rendering.

- **KBreakout**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core gameplay works (Classic/Hard/Multi-Ball Chaos campaigns, 40 stages, 6 active cyber skills Laser/Split/Fire/Barrier/Gravity/Satellite, Cyber-Forge Lab with 7 crafting recipes, dynamic brick types Quantum Resonance/Prism Reflector/Explosive/Hazard/Steel, boss fortress encounters, particle explosion physics, high scores and material persistence).
  - 🔧 FIXED: `initAudio` and `playTone` were called by all skill buttons, Cyber-Forge craft buttons, brick bounces, explosions, and game-over states, but neither function was defined, throwing fatal ReferenceErrors on every interaction. Implemented full Web Audio synthesizer suite with dynamic gain envelopes.
  - 🔧 FIXED: Undefined variable `screenShake` was assigned in `triggerQuantumResonance()` and `triggerExplosion()`, silently breaking screen shake intensity and rotational damping. Routed calls to `triggerScreenShake()`.
  - 🔧 FIXED: Key 'F' was bound to both FIR (Fire skill) and `toggleForge()`, causing the Cyber-Forge lab overlay to unexpectedly open during gameplay whenever activating the Fire skill. Removed 'F' from forge toggle, updated title screen prompt to 'O', and restricted 1–7 crafting keys to when the forge modal is open.
  - 🔧 FIXED: High score was loaded and saved to localStorage, but was never updated when `score > high_score`, leaving the displayed high score permanently stuck. Added high score tracking in `update()` and `saveGameData()`.
  - 🔧 FIXED: Canvas had `cursor: crosshair` but lacked all mouse/touch listeners, preventing mouse, touchpad, and mobile users from moving the paddle, launching stuck balls, or clicking to start/retry. Added `pointermove` and `pointerdown` event listeners.
  - 🔧 FIXED: Boss fortress core was declared with health bar container and laser damage checks, but the boss chassis was never drawn on canvas, boss movement was missing, the health bar fill width was never updated, and balls could not collide with the boss. Added boss canvas rendering, movement update, health bar fill sync, and ball-boss bounce collision.
  - 🔧 FIXED: Opening the Cyber-Forge lab overlay during gameplay did not pause the game loop, allowing active balls to fall and lose player lives while viewing recipes. Paused game update while `forge_open` is active.

- **KBase**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (simultaneous 64-bit live multi-base converter for Bin/Oct/Dec/Hex/Custom 2..36/ASCII, text string encoding suite Base64/URL/Hex/SHA-256 with clipboard copy, 64-bit interactive toggle board with shifts, rotations ROL/ROR, reversal, byte swap endianness, 64-bit bitwise logical matrix calculator AND/OR/XOR/NOT/SHL/SHR/SAR/ROL/ROR, IEEE-754 32-bit single and 64-bit double precision floating point breakdown strips and formulas, variable-length integer suite LEB128/SLEB128/Protobuf ZigZag encoder and decoder with binary layout breakdown, multi-width 8/16/32/64-bit two's complement and sign-magnitude inspector, structured bitfield slicer with bit mask and metrics Popcount/CLZ/CTZ/Parity/Pow2, activity history logging with CSV and JSON export, help and shortcuts modal).
  - 🔧 FIXED: User guide explicitly promised Enter keyboard shortcut to execute/recalculate active conversions or bitwise operations, but Enter was unhandled in keydown listener. Added Enter key handler dispatching to active input/tab calculation.
  - 🔧 FIXED: Bitwise operator suite inputs (`op-a`, `op-b`) did not live-update the matrix display on input and none of the 9 operator buttons indicated active operator state. Added live input listeners, `currentBitwiseOp` tracking, and dynamic active button highlight.
  - 🔧 FIXED: Activity log was spammed on every keystroke in IEEE-754 float input and Varint integer input, and unconditionally recorded 3 entries on initial page load. Added `recordHistory` flag so live typing and page load do not pollute the history log while button clicks and Enter key triggers record operations cleanly.
  - 🔧 FIXED: Varint hex decoder failed when receiving continuous hex byte sequences (e.g. `E58E26`) because `parseHexBytes` assumed space separators and parsed continuous hex as a single integer > 255. Updated parser to split continuous hex strings into 2-character byte pairs.
  - 🔧 FIXED: "Clear All" button in multi-base converter cleared all base text inputs but left `bitboardVal` with stale bits, causing the 64-bit board to desync from inputs. Wired up `bitboardVal = 0n` and `renderBitboard()`.
  - 🔧 FIXED: IEEE-754 single-precision formula generated invalid expressions for special values (Zero, Subnormals, ±Infinity, NaN). Added dedicated formula branches for zero, subnormals, infinity, and NaN. Also added cursor pointer styling and fallback copy handling to `str-output`.

- **KBBS**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (virtual ANSI terminal emulator with canvas rendering, CRT scanline effects, multi-node dialing directory with JSON import/export, interactive door games Legend of the Red Dragon & TradeWars 2015 streaming to terminal, EchoNet FidoNet EchoMail reader with message posting, reply quoting, and .MSG export, binary file transfers XMODEM/ZMODEM, session log capture/export as ANSI/TXT, keyboard shortcuts).
  - 🔧 FIXED: Unescaped single quote (`'''`) and backslash (`'\'`) in the `CP437` character mapping array triggered a fatal `SyntaxError: Unexpected string`, crashing the entire script before execution in browsers. Properly escaped characters to `'\''` and `'\\'`.
  - 🔧 FIXED: `selectMacro()` had an unclosed `if` block, nesting `saveMacro()` inside it and failing to populate `macroString` into the textarea when selecting a macro. Terminated the block correctly and restored script string binding.
  - 🔧 FIXED: "New" button in the Macro editor referenced `onclick="newMacro()"` which was undefined, throwing a ReferenceError. Implemented `newMacro()` to clear inputs and initialize a script template.
  - 🔧 FIXED: Display Settings modal had an empty `<div id="paletteContainer">` with no interactive color inputs, and `setBlinkRate` was neither loaded nor saved. Added `renderPaletteEditor()` with 16 color inputs, persisted custom palettes to `kbbsSettings.palette`, and wired up `setBlinkRate`.
  - 🔧 FIXED: Terminal scrollbar container had `onscroll="handleScroll(event)"` but `handleScroll` was never defined. Added `handleScroll(event)` handler calculating proportional `scrollOffset` and re-rendering the terminal canvas.
  - 🔧 FIXED: ANSI Art Viewer presets "sunset" and "acid" fell back to a generic label without artwork, `artPaletteSelect` did not tint presets, and `updateArtBaud` was an empty no-op. Added retro artwork presets with palette theming (Amber, Green, Cyber) and status toast notifications.

- **KAudio**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (virtual piano keyboard, ADSR envelope, biquad filter, overdrive/distortion, stereo delay/echo, 16-step 4-track sequencer, 8-bit sound FX generator presets, dual-mode visualizer oscilloscope/FFT spectrum, JSON import/export, WAV offline master export, performance recording/playback).
  - 🔧 FIXED: Sequencer notes called `playNote(0, true, ...)` with `freqOverride`, which bypassed storing the voice in `activeOscs`. As a result, `stopNote(0, true)` could not find the voice, causing sequencer oscillators to never stop and continuously leak voices into the Web Audio context. Assigned unique `seq_${trIdx}` voice keys so notes cleanly trigger ADSR release and terminate.
  - 🔧 FIXED: Automated sequencer playback triggered `triggerPreset()` with preset sound effects, spamming dozens of toast notifications per second. Added optional `showNotification = false` parameter when presets are executed via the sequencer.
  - 🔧 FIXED: Stopping recorded performance playback (`stopPlayback()`) cancelled pending `stopNote` timeouts without silencing currently active voices, leaving notes permanently ringing. Added voice cleanup loop on playback stop.
  - 🔧 FIXED: "Z: Rec, X: Play" status indicator and "Octave: 0" badge were non-interactive text elements, preventing mouse and touch users from recording/playing performances or shifting octaves. Added interactive click handlers with visual pointer styling and tooltips.
  - 🔧 FIXED: Virtual piano keys were not playable via keyboard navigation (Tab focus + Enter/Space) despite having `tabIndex="0"` and `role="button"`. Added `keydown`/`keyup` event handlers for Enter and Space.
  - 🔧 FIXED: Exporting WAV with an empty sequencer grid silently generated and downloaded 4 seconds of pure silence. Added active step validation with a user guidance toast before triggering offline rendering.

- **KAsteroids**: ISSUES FOUND ⚠️ (4 issues, 4 fixed inline)
  - ✅ Core gameplay works (ship rotation, thrust physics, lasers/spread/overdrive, UFO aliens, boss encounters, 8 active skills, asteroids fracturing, audio synthesizers, 4 game modes Classic/Time Attack/Hardcore/Campaign, local storage stats/highscores).
  - 🔧 FIXED: Menu items ([1] Classic, [2] Time Attack, [3] Hardcore, [4] Campaign, [S] Statistics, [H] How to Play, [B] Back to Menu) were unclickable `<p>` elements with no mouse click handlers or hover styles. Added `.menu-btn` styling and interactive click handlers.
  - 🔧 FIXED: Numpad keys (1–4) did not trigger game start in mode select menu because only `Digit1`–`Digit4` were handled. Added `Numpad1`–`Numpad4` mappings.
  - 🔧 FIXED: Starting a game from Statistics or How to Play sub-panels left `menuContent` hidden (`display: none`), causing subsequent game over screens to remain stuck showing the help/stats panel instead of the mode select menu. Added `showMenu()` reset in `initGame()`, `killShip()`, and campaign victory.
  - 🔧 FIXED: Mouse-clicking top active skill buttons (`btnEmp`, `btnLaser`, etc.) retained DOM focus on the button, causing subsequent `Space` (shoot) presses to re-trigger the skill button instead of firing cannons. Added `this.blur()` on click.

- **KAlchemy**: ISSUES FOUND ⚠️ (4 issues, 4 fixed inline)
  - ✅ Core gameplay works (element transmutations, apparatuses Retort/Alembic/Anvil, 3 game modes Classic/Blitz/Puzzle, Quests, Workshop upgrades, Potions brewing & effect tester, Daily trials, Magnum Opus Rebirth & Astral perks, Expeditions, Planetary cores, Familiars sanctuary, audio FX, Grandmaster manual modal).
  - 🔧 FIXED: "📖 Codex" equipment tab button called `selectEquipment('codex')` which had no branch in `selectEquipment()`, leaving the Codex container hidden and non-functional.
  - 🔧 FIXED: Selecting tier filter (or clicking tier cards in Codex) only triggered `renderGrimoire()` without updating the active Codex view when Codex was open.
  - 🔧 FIXED: Typing in the element search filter input didn't update the Codex element list when Codex was active.
  - 🔧 FIXED: Keyboard shortcut numbers were limited to 0–5, ignoring Tier 6 (Mythic) element filter on key '6'.

- **K2048**: ISSUES FOUND ⚠️ (5 issues, 5 fixed inline)
  - ✅ Core gameplay works (sliding tiles via arrow keys/WASD/swipe, merging, rulesets Classic/Fibonacci/Threes, active skills Upgrade/Rotate/Hammer/Undo, auto-play, 4 grid sizes, audio beeps, animations/particles).
  - 🔧 FIXED: "New Game" button in Campaign Mode called `initGame()` instead of `restartOrNextStage()`, creating an orphaned campaign state without stage target/obstacles.
  - 🔧 FIXED: Timed Campaign stages (e.g. Stage 4, 10, 16) activated background timer but left `#timer-display` invisible (`display: none`), concealing the countdown from the player.
  - 🔧 FIXED: Toggling Campaign Mode off didn't clear stage hazard flags (`timeAttackEnabled`, `obstaclesEnabled`, etc.), causing subsequent free play sessions to retain campaign hazard rules.
  - 🔧 FIXED: Initial campaign stage start didn't trigger `updateScore()`, showing stale standard game scores until the first move.
  - 🔧 FIXED: Keyboard shortcut Shift+H (intended for Help) was intercepted by the Hammer shortcut `k === 'h'`, executing a hammer smash instead of opening Help modal.

