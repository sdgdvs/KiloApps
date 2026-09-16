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

**Target App:** KScript
**Status:** Next in queue

## Round-Robin Testing Queue (NEVER STOP — loop forever)
Pick the top app, audit it, write a test report, move it to bottom. One app per turn.

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

## Test Reports

- **KRogue**: PASS ✅ (8 issues, 8 fixed inline)
  - Added tutorial auto-trigger on new game via krogue_tutorialSeen while skipping on load.
  - Eliminated WASD/Save/Ability key collision and added Numpad & Y/U/B/N diagonal controls.
  - Fixed high-DPI canvas click distortion by scaling coordinates with logical canvas dimensions.
  - Resolved dropped food corruption where missing values caused NaN hunger and starvation death.
  - Restored Magic Shrine blessings and Crafting Anvil upgrades instead of broken inventory slots.
  - Fixed Quest NPC so interactions award gold/XP without combat or monster turn retaliation.
  - Built gainXP level-up progression system with stat increases, heals, and celebratory effects.
  - Added floor gold spawns, monster gold drops, and functional stat bonuses for equipped rings.

> 📁 **Archived Records**: Historical entries older than 80 lines moved to [app_test_reports_round2.md](archive/app_test_reports_round2.md).
  - Preserved custom timer in campaign mode and eliminated forced untimed reset on stage initialization.
  - Fixed stage 10 bonus, stage 14 bonus, and stage 16 hole overlaps with starting center discs.
  - Added F5 quicksave and F9 quickload shortcuts alongside JSON save file export and import parity.
  - Added localStorage preferences persistence for audio, hints, timer, board size, and AI difficulty.

- **KRead**: PASS ✅ (8 issues, 8 fixed inline)
  - Raw-text search regex engine built to prevent HTML entity corruption and tag mutation.
  - Restored scroll position preservation in highlight and note DOM renderer to prevent viewport jumping.
  - Added empty tab statistics reset and active dynamic reading speed (WPM) telemetry calculation.
  - Periodic localStorage session auto-save added to reading timer to preserve active reading time.
  - Tab title reset to 'Untitled' and open drawers refreshed on document clear.
  - Synchronized search highlight state on tab switching and refreshed open drawers on tab close.
  - Tab rename modal hardened with empty title validation and autofocus retention.
  - Exported TXT format upgraded to include bookmarks matching JSON and Markdown export parity.

- **KRadio**: PASS ✅ (8 issues, 8 fixed inline)
  - Fixed hotkey collision where typing `?` or `h` in URL input triggered Help modal.
  - Handled browser autoplay rejection gracefully without triggering false red error badge.
  - Prevented live stream pause/resume buffering stalls by reconnecting fresh live stream.
  - Eliminated browser error events on stream stop by clearing src with load reset.
  - Synchronized visualizer bar animation with volume and muted status.
  - Added localStorage persistence for station selection, custom URL, and volume level.
  - Added ArrowLeft/ArrowRight keyboard shortcuts for cycling through station presets.
  - Sanitized empty URL input on Tune and synchronized dynamic station title in document.

- **KQuest**: PASS ✅ (8 issues, 8 fixed inline)
  - Fixed updateHeroUI runtime crash in Tavern ale and side quest completion handlers.
  - Implemented missing STATE.REPLAYS and STATE.CONFIG screens and action rebinding.
  - Connected Quick Save (Slot 0) to Save/Load view and prevented out-of-sync screen clobbering.
  - Fixed combat hotkeys (S, L, B, P) and Escape navigation broken by undefined gameState.
  - Wired title screen Continue/Load Save and Help buttons; auto-triggered tutorial on new game.
  - Added Tavern, Milestones, and Combat Log navigation buttons to Town controls panel.
  - Synchronized inventory filter/sort dropdowns and added consumable tags to crafted items.
  - Wired JSON Save Export/Import buttons to file reader and unified Phoenix Elixir usage.

- **KPong**: PASS ✅ (8 issues, 8 fixed inline)
  - Fixed replay frame obstacle rendering referencing live campaign level instead of frame data.
  - Resolved Stage 20 boss shield instant respawn bug when depleted by regular balls.
  - Eliminated leaderboard/games counter corruption triggered on every individual paddle bounce.
  - Added debuff timers to save/load payload and synchronized difficulty button text on load.
  - Persisted theme, AI difficulty, and game mode preferences across browser restarts.
  - Added canvas click/tap game over recovery to prevent mouse and touch user input lock.
  - Implemented simultaneous multi-touch control for 2-Player local PvP on touchscreens.
  - Added Web Audio AudioContext gesture unlock and rewind-to-start replay toggle handling.

- **KPing**: PASS ✅ (8 issues, 8 fixed inline)
  - Prevented textarea selection clashing where Ctrl+C dumped full log instead of copying selection.
  - Guarded against accidental console clearing via 'C' hotkey when text was highlighted in log.
  - Linked preset dropdown with host input and locked preset switches during active scans.
  - Connected route tracing to telemetry canvas to plot latency hop progression.
  - Eliminated delayed interval completion across ping, route trace, and MTU sweep passes.
  - Added route trace cancellation messaging and toast feedback matching PMTU sweep.
  - Unlocked canvas flexbox shrinkage on smaller viewports and added modal focus trapping.
  - Sanitized target host input against protocol prefixes, trailing slashes, and port suffixes.

- **KPass**: PASS ✅ (7 issues, 7 fixed inline)
  - Shifted revealed keys index map on entry deletion to prevent credential mask desync.
  - Hardened CSV parser column mapping, supporting username integration and headerless files.
  - Isolated strength calculation from generator DOM to prevent edit/import UI clobbering.
  - Added localStorage preference persistence for character sets, length, and sort modes.
  - Added double-click vault row copying matching status bar and inline button feedback.
  - Added backdrop dismissal for edit/delete modals and global Ctrl+S inside label input.
  - Guaranteed character representation across selected pools with cryptographic shuffling.

- **KPaint**: PASS ✅ (7 issues, 7 fixed inline)
  - Composited all visible layers into VFS save payload instead of saving base layer only.
  - Guarded base canvas layer from deletion in deleteLayer when multiple layers exist.
  - Resolved mobile touch coordinate NaN in touchend by reading changedTouches.
  - Hardened floodFill against similar-shade loops and added pre-index bounds checking.
  - Added deleteSelection via Del/Bksp keys to erase magic wand and lasso regions.
  - Synchronized canvas transforms (rotation/flip) across all layers and updated emboss offset.
  - Added click/keyboard toggle for export menu and localStorage preferences persistence.

> 📁 **Archived Reports**: Historical test reports have been archived to [archive/app_test_reports_archive.md](archive/app_test_reports_archive.md) to preserve token efficiency.

> 📁 **Archived Round 2 Reports**: Recent detailed audit reports (KPad through KImage) have been archived to [archive/app_test_reports_round2.md](archive/app_test_reports_round2.md).

### Round 2 Completed Index
- KRogue ✅ (8 fixed), KReversi ✅ (8 fixed), KRead ✅ (8 fixed), KRadio ✅ (8 fixed), KQuest ✅ (8 fixed), KPong ✅ (8 fixed), KPing ✅ (8 fixed), KPass ✅ (7 fixed), KPaint ✅ (7 fixed), KPad ✅ (8 fixed), KPac ✅ (8 fixed), KNote ✅ (7 fixed), KNet ✅ (3 fixed), KMystery ✅ (5 fixed), KMines ✅ (8 fixed), KMine ✅ (2 fixed), KMedia ✅ (8 fixed), KMech ✅ (6 fixed), KMaze ✅ (7 fixed), KMatch3 ✅ (8 fixed), KMandel ✅ (6 fixed), KMail ✅ (7 fixed), KJournal ✅ (7 fixed), KImage ✅ (8 fixed)

