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

**Target App:** KTimer
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
- **KTerm**: Completed. Full notes below.
- **KTetris**: Completed. Full notes below.

### Recent Completed Fixes (Full Detail)
- **KTerm**: In `kterm.html`, added `escapeHtml` sanitization helper to prevent DOM XSS injection vectors across toast notifications, renamed tab titles, and session badges; added `isOptionFlag` helper to fix command argument parsing in `dir`, `ls`, `cd`, `type`, `cat`, and `mkdir` so absolute paths starting with `/` (e.g. `/Documents`, `/System`, `/`) are correctly processed instead of being discarded as switches; enhanced `resolvePath` to handle backslashes, drive prefixes (`C:`), and properly resolve `.` and `..` for both absolute and relative paths; enforced `MAX_TABS = 8` tab limit in `createNewTab` to prevent memory and DOM bloat; capped command history buffer (`s.history`) to 500 entries across Enter and reverse search acceptance to prevent unbounded memory growth; deferred Blob URL revocation in `exportSessionLog` (`setTimeout(() => URL.revokeObjectURL(url), 1500)`) to prevent download cancellation/truncation in sandboxed iframe environments; added WAI-ARIA roles (`role="tablist"`, `role="tab"`, `role="toolbar"`, `role="status"`, `aria-selected`, `aria-controls`) and accessible button attributes; and added `beforeunload` cleanup to prevent timer leaks. In `KTerm/main.c`, fixed a critical command branching bug where the `type`/`cat` block was not closed before `macro`, which broke the entire `macro` scripting subsystem and caused every `type`/`cat` command to erroneously display macro usage errors; fortified `ExpandEnvVars` with bounded string concatenation against the stack buffer to prevent potential buffer overflow on long values; added case-insensitive matching (`my_strstri`) to `PerformReverseSearch`; added interactive keystroke/backspace handling in `EditProc` during search mode (`WM_CHAR`) so incremental reverse search works seamlessly; corrected tab capacity check in `AddNewTab` (`>= MAX_TABS`) to permit all 8 tabs with user feedback; added `WS_TABSTOP` styles to buttons; and handled `WM_ERASEBKGND` returning 1 to eliminate screen resize flicker. Verified native compilation and verified Vite web build cleanly.

- **KTetris**: In `ktetris.html`, sanitized `showToast` using safe text node insertion (`textContent`) to completely eliminate DOM XSS injection vectors; added `safeGetJSON` and `safeSetStorage` error-handled persistence wrappers around `localStorage` to prevent unhandled `SyntaxError` or `QuotaExceededError` page crashes in private browsing or full-storage states; fortified `exportStats` and `exportLeaderboard` by appending download anchors to `document.body` before `.click()` and deferring Blob URL revocation (`setTimeout(() => URL.revokeObjectURL(u), 1500)`) to eliminate memory leaks and ensure reliable downloads in sandboxed iframe environments; added lazy `AudioContext` initialization with `beforeunload` lifecycle teardown to eliminate audio context leaks; added `visibilitychange` listener that pauses active gameplay when tab is hidden and resets timer delta; clamped `deltaTime` in `update()` to 250ms max to prevent physics spiral and time-drain jumps when resuming from inactive tabs; guarded `keydown` against browser accelerators (`ctrlKey`, `altKey`, `metaKey`) to prevent intercepting tab closing, page reloading, or saving; called `e.preventDefault()` on game controls (Arrow keys, Space) to prevent parent page scrolling; and added `tabindex="0"`, `role="application"`, and `aria-label` to the game canvas. In `KTetris/main.c`, eliminated severe GDI brush and pen leaks in `DrawTetrisBlock` (dozens of GDI leaks per frame) where temporary brushes (`wB`, `topB`, `emB`, `amB`, `ruB`, `ordB`, `redBrush`) were deleted while still selected into `hdc` without restoring the previous DC brush; restored and deleted bevel brushes (`hiBrush`, `shBrush`, `nullPen`) immediately after bevel polygon rasterization; fixed an undefined format string bug in `ExportStats` where `wsprintfA` was passed `%.1f` (unsupported by User32 `wsprintfA`, causing corrupted stats and variadic stack misalignment) by computing scaled integer percentages (`%d.%d`); added NULL pointer safety checks to `my_strstr` and `my_atoi` and secured `ImportLeaderboardJSON` so missing `"mode"` or `"lines"` keys cannot corrupt pointer traversal or cause access violation crashes; added bounds checks on piece index and rotation in `check_collision` (`p < 0 || p >= 13 || rot < 0 || rot >= 4`); reset replay and modal states with user toast/popup feedback in `LoadGameStateFromFile`; handled `WM_ERASEBKGND` returning 1 and set `wc.hbrBackground = NULL` to eliminate screen resize and repaint flicker; and added missing `KillTimer(hwnd, TIMER_ID)` in `WM_DESTROY`. Recompiled `KTetris.exe` (56.8 KB) cleanly and verified Vite web build.
