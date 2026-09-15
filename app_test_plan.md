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

**Target App:** KPing
**Status:** Next in queue

## Round-Robin Testing Queue (NEVER STOP — loop forever)
Pick the top app, audit it, write a test report, move it to bottom. One app per turn.

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

## Test Reports

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

- **KPad**: PASS ✅ (8 issues, 8 fixed inline)
  - Full report archived in [archive/app_test_reports_round2.md](archive/app_test_reports_round2.md).
  - Fixed tab closing data corruption and context action buffer syncing.
  - Ensured edit menu commands focus editor and update syntax overlay.
  - Hardened replaceOne/findNext case matching and zero-length pattern safety.
  - Added fallback file triggers for Native File System API in sandboxed iframes.
  - Added VFS standalone timeout and localStorage persistence for theme/font/wrap.

> 📁 **Archived Reports**: Historical test reports have been archived to [archive/app_test_reports_archive.md](archive/app_test_reports_archive.md) to preserve token efficiency.

> 📁 **Archived Round 2 Reports**: Recent detailed audit reports (KPac through KImage) have been archived to [archive/app_test_reports_round2.md](archive/app_test_reports_round2.md).

### Round 2 Completed Index
- KPass ✅ (7 fixed), KPaint ✅ (7 fixed), KPad ✅ (8 fixed), KPac ✅ (8 fixed), KNote ✅ (7 fixed), KNet ✅ (3 fixed), KMystery ✅ (5 fixed), KMines ✅ (8 fixed), KMine ✅ (2 fixed), KMedia ✅ (8 fixed), KMech ✅ (6 fixed), KMaze ✅ (7 fixed), KMatch3 ✅ (8 fixed), KMandel ✅ (6 fixed), KMail ✅ (7 fixed), KJournal ✅ (7 fixed), KImage ✅ (8 fixed)

