# App Bug Fix Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 4 worker agents + 1 director operate on this repo on overlapping schedules. You are the **Quality & Build Verification** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`app_fix_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Feature Expander agent), `game_content_plan.md` (Games agent), `new_app_plan.md` (Creator agent), `kiloos_ux_plan.md` (inactive)
- **Shared file `KiloOS/src/App.jsx`** — shared ownership. You may edit ONLY to fix bugs (not to add features or apps). Protocol: `git pull` → minimal surgical fix → commit and push IMMEDIATELY.
- **`KiloOS/src/index.css`** — Do NOT edit.
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

**Target App:** K2048
**Status:** Next (Pass 2)

## Perpetual Workflow (NEVER STOP — loop forever)

### Pass 1: Bug Fixes (alphabetical)
1. Pick the next untested app alphabetically.
2. Test web (.html) and native (build.bat) versions.
3. Log bugs found, fix them, verify.
4. Update this file with results.

### Pass 2: Deeper Quality (after Pass 1 completes)
When all apps have been tested in Pass 1, start over from the top with deeper checks:
- Edge case handling (empty inputs, very long strings, rapid clicking, resize behavior)
- Accessibility (keyboard navigation, focus management, ARIA labels on web)
- Error handling (what happens when localStorage is full, file I/O fails, network disconnects)
- Code quality (remove dead code, consolidate duplicate logic, optimize hot paths)
- Cross-browser issues in web versions

### Pass 3+: Continuous Improvement (after Pass 2 completes)
Start over again. By now other agents have added new features and new apps. There will always be new bugs to find. On each subsequent pass, focus on:
- Regression testing (did new features break old ones?)
- Performance (slow renders, memory leaks, unnecessary repaints)
- Security (XSS, injection, unsafe eval)
- Testing newly created apps that didn't exist during previous passes

**This agent NEVER runs out of work. After each pass, start the next pass.**

## Tested Apps
- **KPass**: Fixed XSS vulnerability in `kpass.html` vault item rendering by using `textContent`. Fixed GDI resource leaks (`HFONT`, `HBRUSH`) in `main.c` on `WM_DESTROY` and `WM_CTLCOLORSTATIC`.
- **KPaint**: Fixed touch coordinate bug in `kpaint.html` where clientX=0 was falsy. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY, and fixed clicking without moving not drawing a dot.
- **KNotes**: Fixed note title extraction bug in `knote.html` to split correctly on `\n`. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY.
- **KGraph**: Fixed performance and memory issue in kgraph.html by moving `new Function` constructor out of the drawing loop. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY.
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
- **KExplorer**: Fixed XSS vulnerability in file/folder rendering and made file extension checking case-insensitive in kexplorer.html. (Web-only app)
- **KFont**: Verified kfont.html font preview logic. Fixed GDI resource leaks (HFONT and HBRUSH) in main.c on WM_DESTROY.
- **KHex**: Verified khex.html hex conversion logic. Fixed GDI resource leaks (HFONT, HBRUSH, HBRUSH) in main.c on WM_DESTROY.
- **KImage**: Verified `kimage.html` rotation and filter logic. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY.
- **KPac**: Fixed TypeError in `kpac.html` Web Audio API where `exponentialRampToValueAtTime` was called on the `GainNode` instead of its `gain` AudioParam.
- **KJournal**: Fixed UTC timezone bug in `kjournal.html` date logic. Removed redundant `renderList` call. Checked `main.c` (no issues found).
- **KMail**: Fixed XSS vulnerability in `kmail.html` by safely creating DOM nodes for subject and sender. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY.
- **KMandel**: Fixed negative color calculation bug in Cyberpunk theme that caused visual artifacts. Added bounds checking to prevent division by zero on zero/1x1 resize.
- **KPad**: Fixed literal newlines in alert dialog in kpad.html. Fixed GDI resource leak (`HFONT`) in main.c.
- **KPing**: Verified `kping.html` web logic. Fixed GDI resource leak (`HFONT`, `HFONT`) in `main.c` on WM_DESTROY.
- **KPong**: Fixed missing variable declarations (`ball_x`, `ball_y`) in `main.c`. Added window `blur` listener in `kpong.html` to prevent keys getting stuck on tab switch.
- **KQuarantine**: Fixed XSS vulnerability in terminal log input rendering in `kquarantine.html`.
- **KRead**: Fixed file input `change` event bug in `kread.html` when re-selecting the same file. Fixed GDI resource leaks (`HFONT`, `HBRUSH`) in `main.c` on `WM_DESTROY` and `WM_CTLCOLOREDIT`.
- **KRogue**: Fixed turn logic in `krogue.html` where pressing wait `.` skipped both player and monster turns. Verified native `main.c` logic.
- **KScript**: Fixed negative integer division in `kscript.html` using `Math.trunc`. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY.
- **KSnake**: Fixed suicide bug (self-intersection on rapid keypresses) and prevented arrow keys from scrolling the page in `ksnake.html`. Fixed suicide bug by adding `last_dir_x/y` checks in `main.c`.
- **KSolitaire**: Verified `ksolitaire.html` logic. Fixed GDI resource leak (HBITMAP) in `main.c` on `WM_PAINT`.
- **KSpace**: Verified `kspace.html` logic. Fixed GDI resource leak (HBITMAP) in `main.c` on `WM_PAINT`.
- **KSynth**: Verified `ksynth.html` web logic. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY.
- **KSys**: Verified `ksys.html` web logic. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY.
- **KTask**: Fixed XSS vulnerability in `ktask.html` by using `textContent` for task rendering. Verified `main.c` (no issues found).
- **KTaskMgr**: Fixed XSS vulnerability in `ktaskmgr.html` process title rendering. (Web-only app)
- **KTerm**: Fixed buffer overflows in `main.c` `ProcessCommand` and `dir` logic. Fixed `kterm.html` `mkdir` creating `undefined` directory.
- **KTetris**: Fixed page scrolling bug in `ktetris.html` by calling `e.preventDefault()` on game keys. Verified `main.c` (no leaks or bugs found).
- **KTimer**: Verified `ktimer.html` logic. Fixed GDI resource leaks (`HFONT`, `HFONT`) in `main.c` on `WM_DESTROY`.
- **KTodo**: Verified `ktodo.html` logic. Fixed GDI resource leak (`HFONT`) in `main.c` on `WM_DESTROY`.
- **KType**: Verified `ktype.html` logic. Fixed GDI resource leak (`HBITMAP`) in `main.c` on `WM_PAINT`, fixed negative number handling in `IntToStr`, added missing `KillTimer`, and adjusted window rect to prevent canvas clipping.
- **KVault**: Fixed XSS vulnerability in category rendering in `kvault.html`. Fixed GDI resource leak (`wc.hbrBackground` initial brush leak) in `main.c`.
- **KZip**: Fixed size calculation bug in `kzip.html`. Fixed GDI resource leak (`HFONT`) and buffer overflow vulnerability in `main.c`.

## Backlog
Pass 1 Complete.

## Pass 2 Completed Apps
- **K2048**: Added missing hotkeys in `k2048.html` keydown handler. Fixed GDI resource leak in `main.c` `WM_PAINT` by selecting original bitmap back into the memDC before deleting.
- **KAudio**: Added ARIA labels and accessibility attributes in `kaudio.html`, and added window blur listener to prevent stuck keys. Enforced bounds checking on recording array. In `main.c`, ensured note-off events are recorded on focus loss or octave shift.

- **KBBS**: Fixed view jump bug on line add in `kbbs.html` by correctly incrementing `scrollOffset`. In `main.c`, fixed GDI resource leaks by tracking and deleting `hUIFont` and `hBgBrush` in `WM_DESTROY`, and removed dead code (`PlayChime`).

Start Pass 2 (Deeper Quality checks). Next target: Calculator.
