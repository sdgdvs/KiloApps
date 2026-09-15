# App Bug Fix Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 6 worker agents + 2 directors operate on this repo on overlapping schedules. You are the **Quality & Build Verification** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`app_fix_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Feature Expander agent), `game_content_plan.md` (Games agent), `new_app_plan.md` (Creator agent), `usability_plan.md`
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

## ⏱️ TURN SCOPING & TERMINATION (CRITICAL — READ EVERY TURN)

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
5. STOP — call no more tools

---

**Target App:** KTodo
**Status:** Next (Pass 4)

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

## Archived Passes (Pass 1 - 3)
*Passes 1–3 have been archived to app_fix_history.md to keep the active plan lean and token-efficient. See [app_fix_history.md](app_fix_history.md) for full historical fix notes for all tested apps across Passes 1 through 3.*

## Pass 4 Completed Apps

> 📁 **Archived Pass 4 Notes**: Detailed fix narratives are archived in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md) to preserve token efficiency.

### Completed Apps Index
- **K2048**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KAudio**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KBBS**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KMaze**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KSnake**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KSolitaire**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KSpace**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KSynth**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KSys**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KTask**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KTaskMgr**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KTerm**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KTetris**: Completed. Full notes below.
- **KTimer**: Completed. Full notes below.

### Recent Completed Fixes (Full Detail)
- **KTetris**: In `ktetris.html`, sanitized `showToast` using safe text node insertion (`textContent`) to completely eliminate DOM XSS injection vectors; added `safeGetJSON` and `safeSetStorage` error-handled persistence wrappers around `localStorage` to prevent unhandled `SyntaxError` or `QuotaExceededError` page crashes in private browsing or full-storage states; fortified `exportStats` and `exportLeaderboard` by appending download anchors to `document.body` before `.click()` and deferring Blob URL revocation (`setTimeout(() => URL.revokeObjectURL(u), 1500)`) to eliminate memory leaks and ensure reliable downloads in sandboxed iframe environments; added lazy `AudioContext` initialization with `beforeunload` lifecycle teardown to eliminate audio context leaks; added `visibilitychange` listener that pauses active gameplay when tab is hidden and resets timer delta; clamped `deltaTime` in `update()` to 250ms max to prevent physics spiral and time-drain jumps when resuming from inactive tabs; guarded `keydown` against browser accelerators (`ctrlKey`, `altKey`, `metaKey`) to prevent intercepting tab closing, page reloading, or saving; called `e.preventDefault()` on game controls (Arrow keys, Space) to prevent parent page scrolling; and added `tabindex="0"`, `role="application"`, and `aria-label` to the game canvas. In `KTetris/main.c`, eliminated severe GDI brush and pen leaks in `DrawTetrisBlock` (dozens of GDI leaks per frame) where temporary brushes (`wB`, `topB`, `emB`, `amB`, `ruB`, `ordB`, `redBrush`) were deleted while still selected into `hdc` without restoring the previous DC brush; restored and deleted bevel brushes (`hiBrush`, `shBrush`, `nullPen`) immediately after bevel polygon rasterization; fixed an undefined format string bug in `ExportStats` where `wsprintfA` was passed `%.1f` (unsupported by User32 `wsprintfA`, causing corrupted stats and variadic stack misalignment) by computing scaled integer percentages (`%d.%d`); added NULL pointer safety checks to `my_strstr` and `my_atoi` and secured `ImportLeaderboardJSON` so missing `"mode"` or `"lines"` keys cannot corrupt pointer traversal or cause access violation crashes; added bounds checks on piece index and rotation in `check_collision` (`p < 0 || p >= 13 || rot < 0 || rot >= 4`); reset replay and modal states with user toast/popup feedback in `LoadGameStateFromFile`; handled `WM_ERASEBKGND` returning 1 and set `wc.hbrBackground = NULL` to eliminate screen resize and repaint flicker; and added missing `KillTimer(hwnd, TIMER_ID)` in `WM_DESTROY`. Recompiled `KTetris.exe` (56.8 KB) cleanly and verified Vite web build.

- **KTimer**: In `ktimer.html`, fortified `showToast` using safe DOM `textContent` nodes, explicit close button ARIA attributes, and child capping (max 5) to eliminate DOM XSS risks and toast element bloat; replaced repeated `AudioContext` creation with a lazy shared instance (`getAudioCtx()`) and added `beforeunload` cleanup to eliminate audio context and interval timer leaks; fixed interval rest and prep configuration parsing by replacing `parseInt(...) || default` with `isNaN` checks so 0-second Rest and 0-second Prep inputs are properly respected; guarded global keyboard shortcuts against browser accelerators (`ctrlKey`, `altKey`, `metaKey`) preventing unintended timer resets on browser shortcuts (Ctrl+R, Ctrl+S, Ctrl+P, Ctrl+C); validated loaded `localStorage` structures (`Array.isArray` for custom presets and multi-timers, object checks for Pomodoro stats) preventing script crashes on corrupted storage; fortified CSV and TXT lap exports by appending anchors to `document.body` before `.click()` and deferring `URL.revokeObjectURL(url)` via `setTimeout`; and cleared `mtTickInterval` when no multi-timers are active to eliminate continuous 5Hz `localStorage` writes and DOM re-renders while idle. In `KTimer/main.c`, eliminated severe listbox flicker and selection-wiping in `UpdateMultiTimers` by throttling listbox refreshes to 250ms, wrapping in `WM_SETREDRAW`, and saving/restoring `LB_GETCURSEL` and `LB_GETTOPINDEX` so users can select and delete timers while they are running; eliminated over 95% of redundant GDI static text redraws across Timer, Pomodoro, and Interval modes by caching displayed text strings; replaced quadratic `O(N^2)` string concatenation in `CopyLapsToClipboard` with pointer advancement; enhanced `SimpleStrToInt` and `ParseTimerInput` with whitespace and unit ('m'/'s') handling; handled `WM_ERASEBKGND` returning 1 to eliminate screen repaint flicker; and added modifier key checks in `MainEntry` preventing accidental timer resets when Ctrl or Alt is held. Recompiled native `KTimer.exe` (28.7 KB) cleanly and verified Vite web build.
