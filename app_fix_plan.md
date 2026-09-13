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

**Target App:** KTerm
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
*Passes 1–3 have been archived to  pp_fix_history.md to keep the active plan lean and token-efficient. See [app_fix_history.md](app_fix_history.md) for full historical fix notes for all tested apps across Passes 1 through 3.*

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

### Recent Completed Fixes (Full Detail)
- **KSys**: In `ksys.html`, guarded global keyboard shortcuts (`1`–`5`, `h`, `F1`, `r`) against browser accelerators (`ctrlKey`, `altKey`, `metaKey`) to prevent hijacking browser tab switching and page shortcuts; added active modal suppression so shortcuts do not trigger background actions while Help, Details, or Add Service modals are open; fixed download truncation by deferring Blob URL revocation (`setTimeout(() => URL.revokeObjectURL(url), 1500)`); made CPU benchmark resilient by adding try/catch and single-threaded stress fallback in case Web Workers are blocked by sandbox/CSP policies; made RAM throughput test memory-safe with 32M/8M/2M Float64 fallback allocation; guarded sparklines against zero-dimension layout exceptions when rendered on hidden tabs; added custom daemon deletion (`deleteCustomService`) and exception-safe `localStorage` persistence (`ksys_custom_services_v1`); restored memory footprint values upon restarting services; added Enter key submission in daemon registration inputs; added Web Audio synthesized acoustic feedback (clicks, success chords, warning beeps); added WAI-ARIA tab semantics (`role="tablist"`, `role="tab"`, `role="tabpanel"`, `aria-selected`, `aria-controls`) and Arrow key navigation across tabs; and added ARIA dialog semantics (`role="dialog"`, `aria-modal="true"`, `aria-labelledby`) to all 3 modal dialogs. In `KSys/main.c`, prevented `g_LogBuffer` buffer overflow crashes during long runtimes by pruning the oldest log lines when approaching buffer capacity; updated `SaveReportFile` to return boolean write status and fortified `ExportReport` with error notifications and parent modal ownership (`hwndOwner`); added `WS_TABSTOP` to all interactive controls and integrated `IsDialogMessage` message routing for Win32 Tab keyboard navigation; added shortcut keys in the message loop ('C' CPU, 'M' RAM, 'D' Disk, 'S' Service Filter, 'E' TXT, 'J' JSON, 'T' HTML, 'R' Run All/Refresh, '1'–'5' Tabs, 'F1'/'H' Help, Esc); handled `WM_ERASEBKGND` returning 1 to eliminate repaint flicker; and updated the F1/Help guide dialog. Verified native compilation readiness and verified Vite web build cleanly.

- **KTask**: In `ktask.html`, added `escapeHtml` sanitization helper to eliminate DOM XSS injection vectors across toast notifications, task icon initials, and deep inspection module tables; fortified `exportCSV` and `exportJSON` by attaching download anchors to `document.body` before `.click()` and deferring Blob object URL revocation (`setTimeout(() => URL.revokeObjectURL(url), 1500)`) to prevent silent download failures or zero-byte file truncation in sandboxed iframe environments; added `document.hidden` guard to the 2-second dynamic telemetry interval to prevent CPU/memory resource burn when tab is inactive; added full WAI-ARIA tab semantics (`role="tablist"`, `role="tab"`, `role="tabpanel"`, `aria-selected`, `aria-controls`, `aria-labelledby`) across main navigation tabs and deep inspection tabs; added `role="radiogroup"` and `role="radio"` with dynamic `aria-checked` states across quick filter chips; guarded global keyboard event listeners against browser accelerator combinations (`ctrlKey`, `altKey`, `metaKey`), enabled `Ctrl+E` (CSV) and `Ctrl+J` (JSON) export shortcuts as documented, scoped task list keyboard actions (Arrow keys, Enter, P, I, Delete) to the active tasks tab, and added textarea fallback copying for deep inspection reports. In `KTask/main.c`, implemented `my_strncat` bounded string concatenation routine and replaced all `my_strcat(report, ...)` in `PerformInspectProcess` with bounds checking against the 64KB buffer to eliminate potential buffer overflow vulnerabilities on systems with numerous threads, modules, or memory pages; added `SendMessageA(hEdit, EM_SETLIMITTEXT, 0, 0);` in `InspectWndProc` `WM_CREATE` to remove the default 30KB/64KB Windows edit control buffer limitation, ensuring deep inspection reports are displayed completely without truncation; added `WS_TABSTOP` styles across child controls (`hSearchBox`, `hListBox`, and all toolbar buttons) and integrated `IsDialogMessage` message routing into the message loop for full Win32 keyboard navigation; and handled `case WM_ERASEBKGND:` returning 1 with `FillRect` and `case WM_CTLCOLORSTATIC:` to completely eliminate screen redraw/resize flicker and maintain uniform button-face styling. Verified native compilation readiness and verified Vite web build cleanly.

- **KTaskMgr**: In `ktaskmgr.html`, resolved a 3-second startup and refresh timeout lag in standalone mode by detecting standalone execution (`window.parent === window`) and immediately resolving with local process table state; fortified "End Task" in standalone mode to dynamically remove terminated processes and clear `selectedTaskId` instead of hanging on unanswered postMessages; added dynamic system telemetry metrics (`#system-metrics`) calculating simulated CPU load and estimated RAM based on active process count; implemented interactive column header sorting on "Application Title" and "Process ID" with ascending/descending toggle, sort indicators (▲/▼), and WAI-ARIA `aria-sort` attributes synced with the sort selector; implemented full keyboard navigation on the task table (`ArrowUp`/`ArrowDown` task switching with automatic scrolling, `Home`/`End`, `Enter`/`Delete` to end task); guarded global keyboard event listeners against browser accelerator keys (`ctrlKey`, `altKey`, `metaKey`) to prevent hijacking browser tab reload and history shortcuts; added "New Task" Run modal dialog (`#runModal`, hotkey `Ctrl+N`) with datalist suggestions allowing users to launch apps via `OS_LAUNCH_APP`; added process list export (`exportTasks`) with clipboard fallback; added `escapeHtml` protection for empty state filter displays; and added `beforeunload` and `pagehide` teardown listeners to prevent interval leaks. Verified Vite web build cleanly.
