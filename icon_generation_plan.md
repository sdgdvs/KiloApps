# Icon Generation Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** You are the **Icon Designer** agent.
- Your sole job is to design new, unique, and appropriate icons for all KiloApps to replace duplicated, reused, or missing icons.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership:** You own this file (`icon_generation_plan.md`). **READ ONLY THIS MD FILE** for your instructions and context. Do NOT read `master_plan.md`, `architecture.md`, or other agents' plan files to avoid context bloat.
- **Target format:** Icons must be lightweight 32x32 `.ico` files to adhere to the strict < 999KB per app limits.
- **Implementation:** Modify `gen_icons.py` to add new procedural generation functions for each app, and run it to produce the `.ico` files in `icons/` (which get moved to `KiloOS/public/assets/icons/`).
- **Registration:** After generating a new icon, update `KiloOS/src/App.jsx` to point to it (e.g., `icon: '/assets/icons/knewapp.ico'`), but DO NOT change anything else in `App.jsx`.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively → push again.
- **Logging discipline:** Keep this plan file concise. Log your progress below.

---

## Multi-Phase Plan

### Phase 1: Audit and Script Expansion
- Review `KiloOS/src/App.jsx` for apps that reuse generic icons (like `ksys.ico`, `knote.ico`, `kterm.ico`, `kchart.ico`, `kpong.ico`).
- Expand `gen_icons.py` to support drawing more shapes or simply add new hardcoded pixel art arrays for each missing app.

### Phase 2: System and Dev Icons
- Target apps: `KTaskMgr`, `KSettings`, `KBase`, `KDB`, `KType`, `KZip`, `KFont`, `KContacts`, `KConverter`.
- Design unique visual metaphors for each (e.g., gears for Settings, font character for KFont, zipper for KZip).

### Phase 3: Office and Media Icons
- Target apps: `KBudget`, `KHabit`, `KFlash`, `KJournal`, `KRead`, `KTodo`, `KMedia`, `KColor`.
- Design unique visuals (e.g., dollar sign for Budget, checkmark list for Habit/Todo, flashcard for KFlash).

### Phase 4: Games Icons
- Target apps: `KBreakout`, `K2048`, `KSolitaire`, `KSpace`, `KPac`, `KChess`, `KMaze`.
- Give each game a distinct, recognizable icon (e.g., paddle/ball for breakout, 2048 tile, chess piece).

### Phase 5: Continuous Polish (Perpetual Loop)
- Review `KiloOS/src/App.jsx` for new apps added by the Builder/Creator agents.
- Generate icons for any new apps as they appear.
- Refine existing icons for better visibility in the Start Menu and Desktop.

---

## Progress Log

- **Status:** Phase 4 (Completed), Starting Phase 5
- **Log:**
  - Audited App.jsx for missing or reused icons.
  - Added `circle` and `draw_art` drawing helpers to `gen_icons.py`.
  - Added generation logic for System & Dev icons (ktaskmgr, ksettings, kbase, kdb, ktype, kzip, kfont, kcontacts, kconverter).
  - Executed gen_icons.py, copied to public/assets/icons, and updated App.jsx paths.
  - Added generation logic for Office & Media icons (kbudget, khabit, kflash, kjournal, kread, ktodo, kmedia, kcolor).
  - Executed gen_icons.py with uv, copied icons to public/assets/icons, and updated App.jsx paths.
  - Added generation logic for Game icons (kbreakout, k2048, ksolitaire, kspace, kpac, kchess, kmaze).
  - Executed gen_icons.py with uv, copied to public/assets/icons, and updated App.jsx paths.
  - Added generation logic for KRadio, KGraph, KVault, KQuarantine in Phase 5.
  - Executed gen_icons.py, copied to public/assets/icons, and updated App.jsx paths.
  - Added generation logic for remaining missing icons (krogue, ktetris, kpong, kterm, kaudio, kcalendar, kmail, kimage, knet, kscript, kchart, knote, kpass, kping, khex, kmandel, ktimer, ksynth, ksys).
  - Executed gen_icons.py with uv, copied to public/assets/icons, confirmed all apps point to unique icons in App.jsx.
  - Audited App.jsx for new apps. No new apps without icons found. All icons are up to date and refined for visibility.
  - Noticed gen_icons.py was missing procedural logic for kterm; added generate_term(), re-ran, and copied kterm.ico to public/assets/icons to refine it.
