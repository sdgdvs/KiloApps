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

**Target App:** KTrader
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
- **KTetris**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KTimer**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KTodo**: Completed. Full notes below.
- **KTowers**: Completed. Full notes below.

### Recent Completed Fixes (Full Detail)
- **KTodo**: In `ktodo.html`, guarded global keyboard event listeners against browser accelerators (`ctrlKey`, `altKey`, `metaKey`) to prevent browser shortcuts (Ctrl+C, Ctrl+S, Ctrl+F, Ctrl+N, Ctrl+I) from accidentally clearing completed tasks, toggling stats, or hijacking browser tab actions; implemented `getUniqueId()` with monotonic counter to eliminate task and subtask timestamp ID collisions during rapid creation and imports; fortified `loadTasks()` with deep array and object validation protecting subtasks and legacy migrations from unhandled crashes on corrupted storage; fortified Markdown, CSV, and JSON exports with Blob URLs, body attachment, and deferred revocation (`setTimeout(() => URL.revokeObjectURL(url), 1500)`) to eliminate memory leaks and ensure reliable downloads in sandboxed iframe contexts; reset `e.target.value = ''` on file input to allow re-importing the same file; and hardened task sorting against non-numeric IDs and null values. In `KTodo/main.c`, guarded `MainEntry` message loop accelerators against modifier keys (`ctrlDown` and `altDown`) preventing accidental deletion, stats display, or clearing completed tasks on Ctrl+C / Alt shortcuts; implemented `json_escape()` and updated `DoExportData()` to serialize valid escaped JSON including all subtask checklist arrays; enhanced `DoAddSubtask()` to detect text entered in the Task input field and use it to add custom named checklist items directly without discarding user input; synchronized subtask completed states in `DoToggleTask()` when toggling main task completion; updated `DoImportMarkdown()` to prompt user to replace or append when tasks already exist, avoiding duplicate task floods; and refined `StripTagsFromText` to preserve numeric hashtags (e.g. `#1`, `#42`) while stripping category tags. Recompiled native `KTodo.exe` (21.5 KB) and verified Vite web build.

- **KTowers**: In `ktowers.html`, enabled the Undo button by updating its disabled state dynamically in `render()` (`disabled = moveHistory.length === 0 || won || gameOver`), fixing a bug where Undo was permanently greyed out and unclickable in the web UI; guarded global keyboard shortcuts against browser accelerators (`ctrlKey`, `altKey`, `metaKey`) and form input targets, preventing unintended resets on browser shortcuts (Ctrl+R, Ctrl+S, Ctrl+F, Ctrl+A, Ctrl+U); added Escape key handling to dismiss modals; fortified `campaignStats` loading and saving with `try/catch` and object type validation to prevent crashes on corrupted storage; optimized BFS solver with $O(1)$ index pointer `qHead` and single-level `firstMove` references, eliminating $O(N)$ array shift overhead and thousands of array clones; fortified canvas click and hover handlers against zero/negative dimensions and bounds-checked peg indices; and added `beforeunload` cleanup to terminate audio context and interval timers. In `KTowers/main.c`, eliminated severe GDI resource leaks in `Draw3DSkyscraperBlockGDI` (deselected pens and brushes back to `oldBrush`/`oldPen` prior to calling `DeleteObject`, stopping exhaustion of the 10,000 GDI handle limit); fixed thread handle leak in `PlaySoundEffect` by closing `CreateThread` handle; added `WM_ERASEBKGND` returning 1 to eliminate screen repaint flicker; guarded `WM_KEYDOWN` against Ctrl/Alt modifiers; guarded `WM_MOUSEMOVE` and `WM_LBUTTONDOWN` against negative coordinates and integer division by zero (`w / numPegs`); fortified `historyCount` and `pegs` arrays against buffer overflows (> 4096 moves and > 10 discs); added visited state pruning hash table to native BFS solver to prevent queue overflow and redundant loops; and added `UpdateControlsVisibility` to `CheckWinOrLoss` so earned stars appear immediately on the stage banner. Recompiled native `KTowers.exe` (156 KB) cleanly and verified Vite web build.
