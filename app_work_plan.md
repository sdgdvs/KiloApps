# App Work Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 4 agents operate on this repo on overlapping schedules. You are the **App Builder**.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`app_work_plan.md`).** Read but NEVER edit:
  - `app_fix_plan.md` (QA agent), `kiloos_ux_plan.md` (UX agent), `game_content_plan.md` (Games agent)
- **Shared file `KiloOS/src/App.jsx`** — owned by the UX agent. You may ONLY add entries to the APPS array. Protocol: `git pull` → make minimal APPS-only change → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — owned by the UX agent. Do NOT edit.
- **App folder categories** for the `folder` property in APPS: `System`, `Media`, `Office`, `Games`, `Network`, `Dev`.
- **Dual-target model:** Each app has a native C version (`K[Name]/main.c` + `build.bat`) and a web HTML5 version (`KiloOS/public/apps/k[name].html`). Both versions should offer functional parity where feasible. Web HTML files must be single self-contained files (inline CSS + JS, no imports).
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.
- **Version bumping:** If you modify KiloOS shell files, bump the patch version in `KiloOS/package.json`.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **Logging discipline:** Keep this plan file concise. A few lines per completed item. Do NOT dump file contents or create verbose logs.

---

**Target App:** KJournal
**Status:** Created from Scratch
**Current Phase:** Done

## Round-Robin Continuous Improvement Queue
Pick the top app from this list, add a meaningful new feature (update BOTH web and native versions), and then move it to the very bottom of the list.

- KChart
- KChess
- KChat
- KBBS
- KAudio
- KClock
- KPong
- KDB
- KFont
- KMail
- KMandel
- KPing
- KScript
- KMaze
- KMine
- KPac
- KNote
- KPass
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
