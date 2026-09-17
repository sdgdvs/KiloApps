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
- **Token Conservation & Logging Rules (CRITICAL):**
  - Run log entries: ≤8 lines of terse bullet points. No paragraphs.
  - Skip-turn entries: exactly 1 line: `⏭️ Skip — [reason in ≤15 words]`.
  - Never restate implementation details that exist in code. Log WHAT changed + results, not HOW.
  - Never list parameter names, field names, or variable values unless reporting failure.
  - Completed work needs no elaboration: `✅ Done (N/N tests pass)` is sufficient.
  - Surgical edits only. Touch only specific cells/lines that changed. Table cell notes ≤100 chars.
  - Only read files relevant to current task. Move historical logs older than ~80 lines to `archive/`.

---

## ⏱️ TURN SCOPING & TERMINATION (CRITICAL — READ EVERY TURN)

**Single-Item-Per-Turn Rule:**
- Each cron trigger = ONE turn. Audit exactly ONE app from your queue, then STOP.
- "Loop forever" means the CRON loops forever across turns, NOT that you loop within a single turn.
- After committing and pushing your work for ONE app, STOP CALLING TOOLS immediately.

**Subagent Delegation & Model Rule:**
- If you spawn a subagent, ALWAYS use `Model: "flash"` (or `Model: "sonnet"` if flash fails). NEVER use `Model: "inherit"` or spawn Claude Opus/Pro subagents.
- Set an 8-minute timer using `schedule` with `TimerCondition`. If it fires, KILL the subagent using `manage_subagents`, commit, push, and STOP.

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

### Step 6: Tutorial & Save System Verification (GAMES ONLY)
For every game audited, also verify:
- **Tutorial existence**: Does a tutorial or "How to Play" exist? Is it accessible from Help or auto-shown?
- **Complex games** (KRogue, KQuest, KMaze, KSpace, KPac, KSnake, etc.): Tutorial must auto-start on NEW game but NOT on saved game load. Check for `localStorage` flag like `k[game]_tutorialSeen`.
- **Start splash screen** (complex games): Does it have a title screen with New Game / Continue / Help? Do all buttons work?
- **Save system integrity**: Does quicksave (F5) / quickload (F9) preserve all game state? Does it survive browser refresh? Does loading a save skip the tutorial?
- **Classic games** (KChess, KGo, KSudoku, etc.): Tutorial should be in Help modal only — NOT auto-shown.

---

**Target App:** KSudoku
**Status:** Next in queue

## Round-Robin Testing Queue (NEVER STOP — loop forever)
Pick the top app, audit it, write a test report, move it to bottom. One app per turn.

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

## Test Reports

- **KStellar**: PASS ✅ (8 issues, 8 fixed inline)
  - Added first-time tutorial onboarding via kstellar_tutorialSeen while skipping on save load.
  - Implemented F5 quicksave and F9 quickload shortcuts preserving galaxy, ship, and missions.
  - Added JSON game save export and import file reader with payload validation.
  - Resolved zero-fuel soft-lock with interactive emergency distress beacon rescue mechanic.
  - Added station hull maintenance repair and reactor refueling services with credit scaling.
  - Added 1-4 numeric and A/E/T/F hotkeys for combat actions and animated button hotkey cues.
  - Added combatEnding action locking to prevent re-entrant clicks during battle resolution.
  - Added non-blocking floating HUD toast notifications and backdrop click modal dismissal.

- **KStarship**: PASS ✅ (8 issues, 8 fixed inline)
  - Added first-time onboarding briefing via kstarship_tutorialSeen while skipping on save load.
  - Implemented F5 quicksave and F9 quickload shortcuts preserving state across browser refreshes.
  - Added JSON mission save export and import file reader with payload validation.
  - Resolved zero-fuel soft-lock with interactive emergency distress beacon rescue mechanic.
  - Added star docking/interact action (E key & toolbar button) enabling station re-entry.
  - Fixed non-combat hull depletion game overs and prevented officer loss role corruption.
  - Added audio mute toggle, toast feedback, cargo capacity meter, and directional scanner bearings.
  - Added arrow keys, Escape modal dismissal, and 1-4 numeric hotkeys for encounter choices.

- **KSpace**: PASS ✅ (8 issues, 8 fixed inline)
  - Added first-time tutorial auto-trigger via kspace_tutorialSeen while skipping on save load.
  - Implemented F5 quicksave and F9 quickload shortcuts with browser refresh prevention.
  - Upgraded saveGameState and loadGameState preserving complete state, upgrades, and seed.
  - Added JSON mission save export and import file reader with structure validation.
  - Wired missing replay skill recording for hyper-jump, drone wing, and overcharge hyper-mode.
  - Fixed keybinds screen bounds for all 13 bindings and added pointer click and Escape cancellation.
  - Added interactive buttons and pointerdown handlers for leaderboard, game over, and victory screens.
  - Expanded pause menu options and blurred skill bar buttons to prevent spacebar focus collision.

- **KSolitaire**: PASS ✅ (8 issues, 8 fixed inline)
  - Preserved full game mode, stage, draw rules, and card states across browser refresh.
  - Implemented F5 quicksave and F9 quickload shortcuts with toolbar controls and toast feedback.
  - Added JSON game save export and import parity alongside existing statistics backup.
  - Restored Magic Wand charge and score refund integrity upon move undo.
  - Fixed stock reshuffle state loss by persisting state to storage immediately after shuffle.
  - Fixed same-column tableau card selection bug to allow switching selected sub-stacks.
  - Enhanced Smart Hint engine to scan all tableau sub-stacks and prevented King swap loops.
  - Added Play Again and Close actions to victory banner and allowed Escape dismissal.

- **KSnake**: PASS ✅ (8 issues, 8 fixed inline)
  - Added dedicated tutorial overlay with auto-trigger on new game via ksnake_tutorialSeen.
  - Implemented F5 quicksave and F9 quickload preserving complete state (boss, portals, rivals, powerups).
  - Preserved quicksave file across browser refreshes and restored boss bar state dynamically on resume.
  - Fixed map editor Border tool to enclose side borders and persisted custom portals across reloads.
  - Implemented real speed acceleration in Ramp mode dynamically scaling as apples are eaten.
  - Wired Freeze skill to slow CPU rivals and bosses by 50% during active duration.
  - Added 1.5x score multiplier for Speed Berry and recorded match timeline inputs for replay export.
  - Added Pause keybind configuration, Enter-to-save on initials input, and modal backdrop dismissals.

- **KSimon**: PASS ✅ (8 issues, 8 fixed inline)
  - Resolved Slow-Mo/Freeze key collisions by assigning dedicated hotkeys and aligning UI indicators.
  - Reset playerSequence in Hint and Slow-Mo handlers to prevent evaluation offset corruption.
  - Re-anchored button layout on Campaign stage advance to prevent out-of-bounds button spawns.
  - Synchronized active button count and 3D console grid dynamically upon mode dropdown changes.
  - Clarified passive strike shield protection status and behavior across UI displays.
  - Muted sequence playback shockwaves in Sound-Only mode to preserve pitch memory challenge.
  - Added F5 quicksave and F9 quickload shortcuts alongside JSON save file export and import parity.
  - Added touch event support, modal backdrop and Escape dismissals, and initial tutorial onboarding.

- **KScript**: PASS ✅ (7 issues, 7 fixed inline)
  - Added case-insensitive keyword support for `print` statements (`PRINT`, `Print`).
  - Added regex search syntax error fallback to literal substring replacement.
  - Resolved macro Tab indentation loss and protected editor history with setRangeText.
  - Added macro playback re-entrancy locking and synchronized status badge states.
  - Added localStorage persistence for active script buffer and recorded macro keystrokes.
  - Auto-scrolled output console on updates and eliminated leading blank lines on Return.
  - Added statement loop safety advance to guard parser against infinite stalls.

- **KRogue**: PASS ✅ (8 issues, 8 fixed inline)
  - Added tutorial auto-trigger on new game via krogue_tutorialSeen while skipping on load.
  - Eliminated WASD/Save/Ability key collision and added Numpad & Y/U/B/N diagonal controls.
  - Fixed high-DPI canvas click distortion by scaling coordinates with logical canvas dimensions.
  - Resolved dropped food corruption where missing values caused NaN hunger and starvation death.
  - Restored Magic Shrine blessings and Crafting Anvil upgrades instead of broken inventory slots.
  - Fixed Quest NPC so interactions award gold/XP without combat or monster turn retaliation.
  - Built gainXP level-up progression system with stat increases, heals, and celebratory effects.
  - Added floor gold spawns, monster gold drops, and functional stat bonuses for equipped rings.

> 📁 **Archived Reports**: Historical test reports have been archived to [archive/app_test_reports_round2.md](archive/app_test_reports_round2.md) and [archive/app_test_reports_archive.md](archive/app_test_reports_archive.md) to preserve token efficiency.

### Round 2 Completed Index
- KStellar ✅ (8 fixed), KStarship ✅ (8 fixed), KSpace ✅ (8 fixed), KSolitaire ✅ (8 fixed), KSnake ✅ (8 fixed), KSimon ✅ (8 fixed), KScript ✅ (7 fixed), KRogue ✅ (8 fixed), KReversi ✅ (8 fixed), KRead ✅ (8 fixed), KRadio ✅ (8 fixed), KQuest ✅ (8 fixed), KPong ✅ (8 fixed), KPing ✅ (8 fixed), KPass ✅ (7 fixed), KPaint ✅ (7 fixed), KPad ✅ (8 fixed), KPac ✅ (8 fixed), KNote ✅ (7 fixed), KNet ✅ (3 fixed), KMystery ✅ (5 fixed), KMines ✅ (8 fixed), KMine ✅ (2 fixed), KMedia ✅ (8 fixed), KMech ✅ (6 fixed), KMaze ✅ (7 fixed), KMatch3 ✅ (8 fixed), KMandel ✅ (6 fixed), KMail ✅ (7 fixed), KJournal ✅ (7 fixed), KImage ✅ (8 fixed)


