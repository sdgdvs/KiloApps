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

**Target App:** KBBS
**Status:** Next in queue

## Round-Robin Testing Queue (NEVER STOP — loop forever)
Pick the top app, audit it, write a test report, move it to bottom. One app per turn.

- KBBS
- KBase
- KBreakout
- KBudget
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

## Test Reports

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

