---
name: kilo-qa
description: >-
  Executes the QA & Build Quality audit and fixes (Pass 5) for a single KiloApps application.
  Use this skill to inspect both web (HTML) and native (C) builds, fix stability/logic bugs,
  verify compiler clean build and file sizes (<999 KB), log results tersely to next_work.md,
  advance the queue handoff, and commit/push changes.
---

# KiloApps QA & Build Quality Skill (Pass 5)

This skill executes a self-contained Pass 5 QA and build audit on exactly ONE application.

## Quality & Craft Directive
- **Tool Call Budget (<= 30 tool calls)**: Complete this turn in <= 30 tool calls.
- **Commit Checkpoint (~15 tool calls)**: Once substantive edits are made or you reach ~15 tool calls, wrap up modifications, run verification, update `next_work.md`, and commit/push.
- **Iterative Craft**: Rigorously audit state persistence, onboarding tutorials, and modal dismissals. Inspect diffs and code after editing to verify syntax, error catching, and parity between web (HTML) and native (C).
- **Parity & Validation**: Run `cd KiloOS && npm run build` and `uv run scripts/test_app_startup.py --app <TargetApp>`. If native C source exists, compile via `cmd.exe /c build.bat`.
- **No Full-Repo Test Suites**: Run only target build/startup verification. NEVER run repo-wide suites (`quality_gate.js`, screenshot generation).
- **No Archives/Receipts**: Never read `archive/` or `.agents/receipts/`.
- Advance queue in `next_work.md`, commit, push, and STOP immediately.

## Pre-flight
1. Ensure git working tree is clean: `git status`.
2. Open [next_work.md](../../next_work.md) to identify the current QA target app (`current_targets.kilo_qa`).

## Audit & Repair Procedure
1. **Target Identification**:
   - Web source: `KiloOS/public/apps/k<name>.html`
   - Native source: `apps/k<name>/` (if present, check `main.c` and `build.bat`)

2. **Pass 5 Focus Areas**:
   - **Full Game/App State Persistence**: Quicksave (F5) and quickload (F9) capturing complete state (timers, hazards, boss/rival states, upgrades, scores) in local storage and native files without data loss or corruption.
   - **First-run Tutorial Integrity**: Ensure tutorial prompts fire only on fresh sessions using `k<name>_tutorialSeen` / `.dat` flags, never interrupting restored save states.
   - **Interactive Splash & Overlays**: Verify all modal dialogs, victory/game over screens, and help guides have working buttons and keyboard shortcuts (Esc, Enter, Space).
   - **ARG Mystery & TINAG Standard Verification**: Ensure no un-diegetic `(ARG)` tags, explicit walkthroughs ("ARG Guidance"), premature fleet meta-spoilers, or leaked master passkeys exist in user-facing UI or copy.
   - **Safety & Resource Cleanliness**: Catch storage quota errors gracefully and ensure URL object/interval leaks are cleaned up.

3. **Build & Size Verification**:
   - Web: Run `npm run build` inside `KiloOS/` to confirm zero Vite build breaks.
   - Native: If native source exists, run its `build.bat` to ensure compilation produces a clean `.exe`.
   - Startup Audit: Run `uv run scripts/test_app_startup.py --app <TargetApp>`.
   - Size limit: Ensure neither the `.exe` nor the `.html` exceeds 999 KB.

4. **Mature Application Quarantine Protocol (DIRECTOR MANDATE - CRITICAL)**:
   - If an app passes Pass 5 with zero defects, full state persistence, clean modal handling, and is functionally mature:
     - **DO NOT rotate it to the queue bottom!** (Rotating to the bottom causes endless skip loops that waste tokens repeatedly).
     - **Quarantine to Mature**: Move the app into `## Mature Applications (Archived from Active Queues)` in `next_work.md`.
     - **Remove from Queues**: Remove the app completely from all active queues and upcoming lists across `next_work.md`.
     - **Advance Target**: Advance `current_targets.kilo_qa` to the next non-mature app in line.
     - Log: `⏭️ Skip — promoted <TargetApp> to Mature Applications quarantine.` Advance rotation, commit, push, and STOP cleanly.

5. **Queue Handoff & Terse Logging (CRITICAL)**:
   - Edit [next_work.md](../../next_work.md):
     - Update YAML frontmatter:
        - Set `current_agent` to the next scheduled agent in `agent_rotation` (or `kilo-tester` / `kilo-creator` if specific follow-up is needed).
        - Advance `current_targets.kilo_qa` to the next app in the queue list.
        - Set `status: ready`.
       - Update `last_run.agent: kilo-qa`, `last_run.app: <TargetApp>`, and `last_run.timestamp`.
     - In the Execution Log section of `next_work.md`, append a run entry:
       - **Strict limit**: ≤8 lines of terse bullet points. No paragraphs.
       - Log WHAT changed + test results, not HOW. Never list parameter names or regurgitate code.
     - If the app was not promoted to Mature, move it to the bottom of the QA upcoming list.

5. **Commit and Push**:
   - `git add <modified files> next_work.md`
   - `git commit -m "fix(qa): Pass 5 audit and fixes for <AppName>"`
   - `git push` (if push fails due to remote update, run `git pull --rebase` and push again).
   - STOP immediately. Process exactly ONE app only.
