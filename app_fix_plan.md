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
- **Token Conservation & Logging Rules (CRITICAL):**
  - Run log entries: ≤8 lines of terse bullet points. No paragraphs.
  - Skip-turn entries: exactly 1 line: `⏭️ Skip — [reason in ≤15 words]`.
  - Never restate implementation details that exist in code. Log WHAT changed + results, not HOW.
  - Never list parameter names, field names, or variable values unless reporting failure.
  - Completed work needs no elaboration: `✅ Done (N/N tests pass)` is sufficient.
  - Surgical edits only. Touch only specific cells/lines that changed. Table cell notes ≤100 chars.
  - Only read files relevant to current task. Move historical logs older than ~80 lines to `archive/`.

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
- **Model Selection (MANDATORY):** ALWAYS spawn subagents using `Model: "flash"` (or `Model: "sonnet"` if flash fails). NEVER use `Model: "inherit"` or spawn Claude Opus/Pro subagents for routine tasks, audits, or summarization.

**Graceful Termination Checklist (do this EVERY turn before stopping):**
1. Processed one item
2. Updated plan file
3. Committed and pushed
4. All subagents terminated (killed or completed)
5. STOP — call no more tools

---

**Target App:** KBBS
**Status:** Next (Pass 5: Tutorial & Save System Integrity)

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

### Pass 5+: Tutorial & Save System Integrity (DIRECTOR DIRECTIVE 2026-09-15)
Focus on verifying tutorial and save system infrastructure across all games:
- **Save/Load corruption**: Does quicksave/quickload preserve ALL game state? Test after multiple saves, after browser refresh, after clearing other localStorage keys.
- **Tutorial localStorage flags**: Verify `k[game]_tutorialSeen` flags work correctly — tutorial fires on new game, NOT on saved game load.
- **Splash screen buttons**: Verify New Game / Continue / Help buttons are all wired and functional.
- **Complex games must have auto-start tutorials** that teach core mechanics in ~60 seconds. Skippable with Esc.
- **Classic/simple games**: Tutorial should be in Help modal only.

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
- **KTodo**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KTowers**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KTrader**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KType**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KVault**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KVoid**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KWizard**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KWords**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)
- **KZip**: Completed. Full notes in [archive/app_fix_pass4_archive.md](archive/app_fix_pass4_archive.md)

### Recent Completed Fixes (Terse Summary — ≤8 lines)
- **KVault**: ✅ Done. Hardened crypto random passgen; secure_zero memory; IsDialogMessage guards; web clipboard fallbacks & download revocations. Native (15.3 KB) and web verified.
- **KVoid**: ✅ Done. Restored native build; alien buffer overflow (MAX_ALIENS 32) fixed; softlock restart; tutorial persistence. Native (22.5 KB) and web verified.
- **KWizard**: ✅ Done. Restored native build; eliminated per-frame GDI leaks; campaign opponentMaxHp clamping; Esc/shortcuts & tutorial persistence. Native (29.6 KB) and web verified.
- **KWords**: ✅ Done. Closed sound thread handle leaks; hoisted cell fonts in WM_PAINT; clamped EndSelection buffer; added Esc modal/drag cancel & SetCapture; hardened full save/load state & tutorial persistence. Native (161 KB) and web verified.
- **KZip**: ✅ Done. Sort selection desync fixed; standalone web/batch fallbacks & storage persistence added; native OpenArchive bounds & dialog Enter routing hardened; dotfile traversal fixed. Native (23.0 KB) and web verified.
- **K2048**: ✅ Done (Pass 5). Full game state save/load implemented for web & native; tutorialSeen flag wired (fires on new game only); false game-over on bomb merges fixed; duplicate campaign stage alert removed; F1 help & backdrop click added. Native (38.5 KB) and web verified.
- **KAudio**: ✅ Done (Pass 5). Full workstation session save/load implemented for web & native; kaudio_tutorialSeen flag wired (fires on new session only); hoisted GDI pen/brushes in FFT spectrum loop; Ctrl/Alt/Meta modifier guards added; sound preset node leaks and URL revocation lifetime fixed. Native (20.9 KB) and web verified.
