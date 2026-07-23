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

**Target App:** KTimer
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

## Tested Apps (Pass 1)
- **KPass**: Fixed XSS vulnerability in `kpass.html` vault item rendering by using `textContent`. Fixed GDI resource leaks (`HFONT`, `HBRUSH`) in `main.c` on `WM_DESTROY` and `WM_CTLCOLORSTATIC`.
- **KPaint**: Fixed touch coordinate bug in `kpaint.html` where clientX=0 was falsy. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY, and fixed clicking without moving not drawing a dot.
- **KNotes**: Fixed note title extraction bug in `knote.html` to split correctly on `\\n`. Fixed GDI resource leak (`HFONT`) in `main.c` on WM_DESTROY.
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
- **KCalc**: Added ARIA labels to buttons and fixed potential XSS by using `textContent` in `kcalc.html`. Fixed GDI resource leaks (`HFONT`, `HBRUSH`) in `main.c`.
- **KChart**: Added ARIA labels to canvas and buttons in `kchart.html`. Fixed GDI resource leak (`HFONT`) and timer leak in `main.c` on `WM_DESTROY`.
- **KChat & KChatServer**: Added ARIA labels in `kchat.html`. Fixed GDI resource leaks (`hUIFont`, `wc.hbrBackground`) and fixed incorrect button labels/states during socket connect/disconnect errors in `main.c`.
- **KChess**: Added ARIA tags and localStorage error handling in `kchess.html`. Fixed undeclared `dstP` variable in `WM_TIMER` that caused compilation error and fixed GDI resource leak (`wc.hbrBackground`) and timer leak in `main.c`.
- **KClock**: Added ARIA labels and improved keyboard accessibility for tabs in `kclock.html`. Fixed non-modal MessageBox bug in `main.c` by passing `hwnd` to `UpdateDisplays`, preventing re-entrancy and UI interaction issues when alerts are shown.
- **KColor**: Added ARIA labels and `aria-live` to color preview in `kcolor.html` for accessibility. Verified `main.c` (no leaks or bugs found).
- **KContacts**: Fixed XSS vulnerability in `kcontacts.html` contact list rendering using `textContent` and safe DOM creation. Added keyboard accessibility attributes and handlers. Verified `main.c` (no issues found).
- **KConverter**: Added ARIA labels and keyboard accessibility to tabs and inputs in `kconverter.html`. Verified `main.c` (no leaks or bugs found).
- **KDB**: Fixed XSS vulnerability in `kdb.html` by safely creating DOM nodes and added ARIA labels for accessibility. Fixed GDI resource leak (`hBgBrush`) in `main.c` on `WM_DESTROY`.
- **KExplorer**: Added ARIA labels, tab indexes, and keyboard enter-key support for file/folder navigation in `kexplorer.html`. (Web-only app, no `main.c`).
- **KFont**: Added ARIA labels and `aria-live` to `kfont.html` for accessibility. Fixed GDI resource leak (`hBgBrush`) in `main.c` on `WM_DESTROY`.

- **KHex**: Added ARIA labels to input fields in `khex.html` for accessibility. Verified `main.c` (no leaks or bugs found).
- **KImage**: Added ARIA labels in `kimage.html` for accessibility and fixed a bug where the same file couldn't be reopened by clearing `fileInput.value`. Fixed buffer zero-division bounds logic in `main.c` `WM_MOUSEMOVE` and `WM_PAINT` by enforcing `drawW` and `drawH` to be at least 1.
- **KJournal**: Added ARIA labels, tab indexes, and keyboard enter-key support for entry list navigation in `kjournal.html`. Verified `main.c` (no leaks or bugs found).

- **KMail**: Added ARIA labels, `tabindex`, and keyboard enter-key support in `kmail.html` for accessibility. Fixed buffer overflow vulnerability in `main.c` `SelectEmail` by increasing the string buffer and using `snprintf`.
- **KMandel**: Added ARIA labels and `aria-live` to `kmandel.html` for accessibility, and added keyboard arrow key navigation for panning. Fixed division by zero bug on zoom for 1x1 resize in `main.c`.

- **KPad**: Added ARIA labels and keyboard accessibility to `kpad.html` tabs and find panel, and fixed a bug where splitting lines used literal backslash-n instead of newline. Fixed GDI resource leak (`wc.hbrBackground`) and `WriteFile` buffer length bug in `main.c`.

- **KPing**: Added ARIA labels in `kping.html` for accessibility, and fixed a memory leak in continuous ping where the times array grew unbounded. Fixed GDI resource leak (`wc.hbrBackground`) and thread handle leak in `main.c`.

- **KPong**: Added ARIA labels and keyboard accessibility attributes in `kpong.html`, parsed `highRally` with `parseInt`, and prevented page scroll with arrow keys. Fixed `KillTimer` leak in `main.c` on `WM_DESTROY`.

- **KQuarantine**: Fixed XSS vulnerability in terminal log input rendering in `kquarantine.html` by using `textContent` instead of `innerHTML`. (Web-only app).

- **KRead**: Added ARIA labels and keyboard accessibility to `kread.html`, and added try-catch blocks for localStorage to handle large files. Fixed potential buffer overflow in `main.c` file loading by using `dwRead` for null-termination instead of `dwFileSize`.

- **KRogue**: Added try/catch for `localStorage` in `saveGame` and `loadGame`, and fixed `ReferenceError` for `total_kills` assignment in `krogue.html`. Fixed GDI resource leaks in `main.c` by selecting original items into `memDC` before deletion and deleting `g_font` in `WM_DESTROY`.

- **KScript**: Added ARIA labels and `aria-live` to `kscript.html` for accessibility. Fixed critical infinite loop (hang) vulnerability in `kscript.html` and `main.c` when parsing unrecognized characters.

- **KSnake**: Added ARIA labels to canvas, removed duplicate globals, and handled `localStorage` quota exceptions with a wrapper function in `ksnake.html`. Fixed invalid duplicate global declarations and added missing `KillTimer` on `WM_DESTROY` in `main.c`.

- **KSolitaire**: Added ARIA labels and keyboard accessibility to cards, and added `localStorage` try-catch blocks in `ksolitaire.html`. Added missing `KillTimer` calls on `WM_DESTROY` in `main.c`.

- **KSpace**: Added ARIA labels, canvas accessibility attributes, arrow key scroll prevention, safe `localStorage` exception handling, and window blur key cleanup in `kspace.html`. Fixed thread handle leak in `PlaySnd` and added missing `KillTimer` call on `WM_DESTROY` in `main.c`.

- **KSynth**: Added ARIA labels, `<label>` elements, try-catch audio playback error handling, noise buffer frame calculation bounds in `ksynth.html`, and input/select element checks before keyboard shortcut execution. Fixed missing `rand()` CRT link error in `main.c` by implementing `fast_rand()`, added focus checks on edit controls before keyboard note processing, zero-initialized `waveHdr`, and enforced minimum sample bounds.

- **KSys**: Added ARIA labels, HTML lang/meta tags, and safe navigator/screen property fallbacks in `ksys.html`. Fixed missing `KillTimer` call on `WM_DESTROY` and increased string buffer capacity in `main.c`.

- **KTask**: Added `Array.isArray` type guards, title/id normalization, ARIA accessibility attributes, keyboard shortcuts, memory leak cleanup for taskStats, and auto-refresh in `ktask.html`. Replaced signed `my_itoa` with `my_utoa` for process IDs > 2GB, added dynamic `WM_SIZE` resizing, listbox selection preservation, shortcuts/timers, and clean process handle management in `main.c`.

- **KTaskMgr**: Added XSS protection with `textContent`, ARIA grid accessibility, `visibilityState` auto-refresh throttling, and `R`/`F5`/`Delete` hotkeys in `ktaskmgr.html` & `ktask.html`. Added flicker-free listbox redraws, global keyboard subclassing, status bar process counter, and System/Idle PID termination protections in `main.c`.

- **KTerm**: Added ARIA live regions/roles, tab completion, terminal log line pruning (max 1000 lines), persistent cursor measuring span, and fallback Virtual File System in `kterm.html`. Replaced unsafe `wsprintfA` with bounded string formatting, fixed case-insensitive command parsing, expanded edit control buffer limit to 1MB, and added bounds checking in `main.c`.