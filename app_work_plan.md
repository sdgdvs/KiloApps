# App Work Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 4 worker agents + 1 director operate on this repo on overlapping schedules. You are the **Feature Expander** (formerly App Builder).
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`app_work_plan.md`).** Read but NEVER edit:
  - `app_fix_plan.md` (QA agent), `game_content_plan.md` (Games agent), `new_app_plan.md` (Creator agent), `kiloos_ux_plan.md` (inactive)
- **Shared file `KiloOS/src/App.jsx`** — shared ownership. You may ONLY add entries to the APPS array. Protocol: `git pull` → make minimal APPS-only change → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — Do NOT edit.
- **App folder categories** for the `folder` property in APPS: `System`, `Media`, `Office`, `Games`, `Network`, `Dev`.
- **Dual-target model:** Each app has a native C version (`K[Name]/main.c` + `build.bat`) and a web HTML5 version (`KiloOS/public/apps/k[name].html`). Both versions should offer functional parity where feasible. Web HTML files must be single self-contained files (inline CSS + JS, no imports).
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.
- **Version bumping:** If you modify KiloOS shell files, bump the patch version in `KiloOS/package.json`.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **Logging discipline:** Keep this plan file concise. A few lines per completed item. Do NOT dump file contents or create verbose logs.

**WORK FOCUS (CRITICAL): FEATURES ONLY, NO POLISH**
- Do NOT spend time on visual polish, color schemes, glassmorphism, animations, or aesthetic improvements.
- DO add: new game modes, difficulty levels, save/load, undo/redo, statistics, import/export, search, multi-tab, keyboard shortcuts, new content/levels, new mechanics.
- Each pass through the queue should add DEEPER, more substantial features than the last. You have up to 999KB per app — use that budget for functionality, not eye candy.

---

**Target App:** KMedia
**Status:** In Queue
**Current Phase:** In Queue

## Round-Robin Continuous Improvement Queue (NEVER STOP — loop forever)
Pick the top app from this list, add a meaningful new feature (update BOTH web and native versions), and then move it to the very bottom of the list. When you reach the end, you'll be back at the top — the cycle never ends. Each pass through the list should add deeper, more substantial features. You have up to **999KB per app** — use that budget to build genuinely feature-rich applications. If new apps appear (created by other agents), add them to the queue.
- KMedia
- KNet
- KZip
- KPaint
- KSnake
- KTetris
- KSpace
- KType
- KSolitaire
- KTerm
- KSynth
- KTask
- KSys
- KRogue
- KConverter
- KTodo
- KGraph
- KTimer
- KContacts
- KRead
- KBase
- KJournal
- KPad
- KImage
- KHex
- KCalc
- KCalendar
- KChart
- KChess
- KChat
- KBBS (Added Auto-Login feature for Dialing Directory)
- KAudio (Added Sequencer - Record and Playback)
- KClock (Added Alarms feature)
- KPong (Added Difficulty Levels)
- KDB (Added ability to add and delete records)
- KFont (Added Custom Text, Bold, and Italic options)
- KMail (Added Search functionality)
- KMandel (Added Undo/Redo viewport history and Save Image features)
- KPing (Added Traceroute functionality)
- KConnect4
- KScript (Added Load and Save script functionality)
- KMaze (Added Save/Load game functionality)
- KMine (Added Safe First-Click, Chording, and Hint System)
- KPac (Added Difficulty Levels and Save/Load Game State)
- KQuest
- KNote (Added Real-Time Search, Note Pinning/Unpinning, and Export TXT features)
- KPass (Added Password Strength & Entropy Assessment and Searchable Vault System)



