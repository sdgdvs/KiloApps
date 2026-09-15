# Usability & UX Plan

## Coordination Rules (DO NOT DELETE â€” required for subagent context)

**Multi-Agent System:** 6 worker agents + 2 directors operate on this repo on overlapping schedules. You are the **Usability Agent**.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership â€” only edit YOUR file (`usability_plan.md`).** Read but NEVER edit the other plan files.
- **Shared files:** You have permission to edit `KiloOS/src/App.jsx`, `KiloOS/src/index.css`, `KiloOS/src/main.jsx`, `KiloOS/index.html`, and any specific app's HTML/JS/C files to improve usability. 
- **When editing `App.jsx`:** other agents may have added APPS array entries. Always prefer the remote version for APPS entries if a conflict occurs, then re-apply your UI changes.
- **Dual-target model:** Each app has a native C version (`K[Name]/main.c` + `build.bat`) and a web HTML5 version (`KiloOS/public/apps/k[name].html`). When fixing usability for an app, consider if the fix is needed for both versions.
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing:** After editing HTML â†’ verify in browser if possible. After editing App.jsx/css â†’ `cd KiloOS && npm run build`. After editing `.c` files â†’ run the app's `build.bat`.
- **Version bumping:** If you modify KiloOS shell files or update versioning/changelog, bump the patch version in `KiloOS/package.json` AND update `MICROS_VERSION` in `KiloOS/src/App.jsx` so the opening screen displays the current version.
- **CI/CD:** Every push to `main` triggers GitHub Actions â†’ Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails â†’ `git pull --rebase` â†’ resolve conservatively â†’ push again.
- **Logging discipline:** Keep this plan file concise. A few lines per completed item. Do NOT dump file contents or create verbose logs.

**WORK FOCUS (CRITICAL): USABILITY, UI, AND UX**
- Most apps have UI and usability problems, such as auto-opening in a size that doesn't show the full UI, not showing controls, lacking a visible "press h for help" prompt (or any other appropriate opening instructions) on startup, blurry text, or bad layout.
- **Your Job:** Fix these issues! Ensure each app opens at an appropriate size, has clear instructions or help menus, crisp text rendering (e.g., canvas scaling issues), and intuitive controls.
- You also maintain and polish the KiloOS web UI itself (Start menu, taskbar, window manager, desktop).
- **🛑 Maturity & Turn Skipping Directive (2026-09-13 — CRITICAL):** For apps that have already been through 6+ passes and have solid usability, clean rendering, proper window sizing, and responsive controls: unless you have a specific directive from the director or user, **"turn skipped because this app is complete and we don't have new ideas here"** is completely fine. Do NOT invent arbitrary layout churn or unneeded redesigns. The director can add new directions later. Log the skip concisely, rotate the item, and finish cleanly.

---

## â�±ï¸� TURN SCOPING & TERMINATION (CRITICAL â€” READ EVERY TURN)

**Single-Item-Per-Turn Rule:**
- Each cron trigger = ONE turn. Process exactly ONE item from your queue, then STOP.
- "Loop forever" means the CRON loops forever across turns, NOT that you loop within a single turn.
- After committing and pushing your work for ONE item, STOP CALLING TOOLS immediately.

**Subagent Timeout Rule:**
- If you spawn a subagent, set a timer for 8 minutes using the `schedule` tool with `TimerCondition` set to the subagent's conversation ID.
- If the timer fires (subagent hasn't finished in 8 min), KILL the subagent using `manage_subagents`, log a one-line failure note in your plan file, commit, push, and STOP.
- NEVER spawn more than ONE subagent at a time.
- NEVER spawn a second subagent if the first one failed. Stop and let the next cron turn retry.

**Graceful Termination Checklist (do this EVERY turn before stopping):**
1. Processed one item
2. Updated plan file
3. Committed and pushed
4. All subagents terminated (killed or completed)
5. STOP â€” call no more tools

---

**Target App:** KImage
**Status:** In Queue
**Current Phase:** In Queue

## Round-Robin Continuous Improvement Queue (NEVER STOP — loop forever via cron)
Pick the top app from this list, identify and fix usability and UI problems (update BOTH web and native versions if applicable), and then move it to the very bottom of the list. Complete exactly ONE app per cron turn (using a single subagent if needed), commit your changes, and then stop your execution. Let the recurring cron schedule wake you up to process the next app. When you reach the end of the list, you'll be back at the top — the cron cycle never ends. If new apps appear, add them to the queue.

- KImage
- KCalc
- KHex
- KCalendar
- KChart
- KChat
- KChess
- KDragon
- KMech
- KAudio
- KRadio
- KBBS
- KPong
- KClock
- KDB
- KiloOS Web UI
- KFortress
- KColony
- KAlchemy
- KFont
- KMail
- KMandel
- KPing
- KConnect4
- KMaze
- KScript
- KMine
- KPac
- KQuest
- KNote
- KPass
- KMedia
- KNet
- KZip
- KPaint
- KFarm
- KSnake
- KTetris
- KSpace
- KSolitaire
- KTerm
- KSynth
- KTask
- KRogue
- KSys
- KTodo
- KConverter
- KGraph
- KTimer
- KContacts
- KRead
- KJournal
- KBase
- KPad

## Progress Log

> 📁 **Archived Progress Log**: Older entries have been archived to [archive/usability_plan_archive.md](archive/usability_plan_archive.md) to preserve token efficiency.


> ?? **Archived Sep 15 Progress**: Recent entries (KPad through KTetris) archived to [archive/usability_plan_archive.md](archive/usability_plan_archive.md).

### Recent Completed Index
KPad ?, KBase ?, KJournal ?, KRead ?, KContacts ?, KTimer ?, KGraph ?, KConverter ?, KTodo ?, KSys ?, KRogue ?, KTask ?, KSynth ?, KTerm ?, KSpace ?, KTetris ?

## DIRECTOR DIRECTIVE (2026-09-15): TUTORIAL & SPLASH SCREEN UX PASS

**When processing any game from the queue, verify these UX requirements:**
1. **Complex games** (KRogue, KQuest, KMaze, KSpace, KAsteroids, KPac, KSnake, KBreakout, KColosseum, KCyber, KMech, KDragon, KVoid, KFarm, KColony, KFortress, KSanctuary, KSubmarine, KStarDredge, KAbyss, KCosmic, KStellar, KTrader, KMystery, KWizard, KStarship, KAlchemy) must have a **start splash screen** with New Game / Continue / Help menu.
2. **Complex games** must **auto-show a tutorial** on first new game start (check for `localStorage` flag like `k[game]_tutorialSeen`). Tutorial must NOT fire when loading a saved game.
3. **Simple apps and classic games** (KChess, KGo, KSudoku, K2048, etc.) should have a tutorial buried in the Help modal � NOT auto-shown.
4. **All apps** should have a visible "press H or F1 for help" prompt on first open if they don't have one already.
