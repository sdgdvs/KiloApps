# App Bug Fix Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 4 agents operate on this repo on overlapping schedules. You are the **Quality & Build Verification** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`app_fix_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Builder agent), `kiloos_ux_plan.md` (UX agent), `game_content_plan.md` (Games agent)
- **Shared file `KiloOS/src/App.jsx`** — owned by the UX agent. You may edit ONLY to fix bugs (not to add features or apps). Protocol: `git pull` → minimal surgical fix → commit and push IMMEDIATELY.
- **`KiloOS/src/index.css`** — owned by the UX agent. Do NOT edit.
- **Your scope is surgical bug fixes ONLY.** Do NOT do UI polish, feature expansion, or large code rewrites — those are the Builder and Games agents' jobs.
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing expectations:**
  - After editing any app's HTML file → open in browser to verify it renders correctly.
  - After editing `App.jsx` or `index.css` → `cd KiloOS && npm run build` to verify the build.
  - After editing a native `.c` file → run its `build.bat` to verify compilation.
- **Build health is your HIGHEST priority.** If `cd KiloOS && npm run build` fails, fix it before doing anything else.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`. If the build/deploy fails, investigate and fix.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **Logging discipline:** Keep this plan file concise. A few lines per tested app. Do NOT dump file contents or create verbose logs.

---

**Target App:** KExplorer
**Status:** In Progress

## Workflow
1. Pick the next untested app alphabetically.
2. Test web (.html) and native (build.bat) versions.
3. Log bugs found, fix them, verify.
4. Update this file with results.

## Tested Apps
- **KCalendar**: Fixed month-wrapping bug in kcalendar.html where changing months on the 31st skipped a month. Checked main.c (no issues found).
- **KAudio**: Fixed Web Audio API envelope bug in kaudio.html and prevented arrow key scrolling. Fixed GDI resource leak (HBITMAP) in main.c.
- **KBBS**: Fixed ANSI 'T' (scroll down) sequence to shift visible area rather than top of buffer in HTML. Fixed native scrolling behavior where mouse wheel changed scrollbar but not view; fixed out-of-bounds array bug when native scrollback hit limit.
- **KCalc**: Fixed M+ and M- evaluation bug in kcalc.html to format the expression before evaluating. Fixed VK_OEM_PLUS keyboard input in main.c so '=' types '=' instead of '+'.
- **KChart**: Fixed requestAnimationFrame leak on multiple randomizations in kchart.html. Fixed GDI resource leak where memBM was deleted while still selected in main.c.
- **KChat & KChatServer**: Fixed native socket resource leak in KChatServer's accept loop. Fixed socket leak on `gethostbyname` failure and improper reconnect logic in KChat native. Verified HTML web chat connection workflow.
- **KChess**: Fixed infinite loop / array out-of-bounds in main.c path checking for Knight moves. Verified web logic is safe.
- **KClock**: Fixed implicit global event bug in kclock.html tab switching. Fixed GDI resource leak (HFONT) in main.c by storing and deleting hFont and hFontMono on destroy.
- **KColor**: Verified kcolor.html logic. Fixed GDI resource leak (HFONT) in main.c by destroying it on WM_DESTROY.
- **KContacts**: Fixed `toLowerCase` undefined bug in `kcontacts.html` search filter. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY.
- **KConverter**: Verified kconverter.html logic. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY.
- **KDB**: Verified kdb.html logic. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY.
- **KDraw** *(DELETED — merged into KPaint)*: Fixed `main.c` bug where lines were drawn from updated coordinates instead of old coordinates (causing dotted lines). Fixed `kdraw.html` bug where clicking without moving didn't draw a dot.

## Backlog
All apps need systematic testing. Continue alphabetically from KExplorer.

