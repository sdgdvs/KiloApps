---
name: kilo-tester
description: >-
  Executes an interactive UI element audit and in-line repairs for a single KiloApps web application.
  Use this skill to audit every button, modal, input, shortcut, and storage handler in KiloOS/public/apps/k<name>.html,
  fix broken or stubbed handlers inline, verify build integrity, log results tersely to next_work.md,
  advance the queue handoff, and commit/push changes.
---

# KiloApps App Tester Skill

This skill executes a self-contained interactive UI audit and repair on exactly ONE web application.

## Pre-flight
1. Ensure git working tree is clean: `git status`.
2. Open [next_work.md](../../next_work.md) to identify the current App Tester target (`current_targets.kilo_tester`).

## Audit & Repair Procedure
1. **Target Identification**:
   - Inspect web source: `KiloOS/public/apps/k<name>.html`

2. **Automated Human-Perspective Startup Audit (MANDATORY)**:
   - Run the automated gameplay startup test:
     `uv run scripts/test_app_startup.py --app <TargetApp>`
   - This test verifies from a real human user perspective:
     - **Static CSS Balance**: Ensures zero unclosed `{` braces that would invalidate modal or component stylesheets (catches silent CSS cascades like unclosed `.log-time {`).
     - **Console/Runtime Errors**: Catches unhandled JS exceptions thrown during page initialization.
     - **Startup Modal Obtrusiveness & Dismissal**: If a splash screen, manual, or tutorial covers the viewport on load, simulates clicking its close button and verifies the overlay **actually disappears** (`display: none` / detached).
     - **Gameplay Surface Occlusion**: Uses `document.elementFromPoint()` at the game canvas/board center to verify mouse clicks reach the game rather than being intercepted by stuck or transparent overlays.
     - **Canvas Liveness**: Verifies the game canvas has valid non-zero dimensions and is actively rendering.
   - If `test_app_startup.py` reports any failure (broken CSS, unclosable modal, or gameplay occlusion), investigate and fix it immediately in Step 4.

3. **Interactive UI Audit**:
   - Inventory every interactive element:
     - Buttons, tabs, dropdowns, inputs, checkboxes.
     - Modal dialogs (open, dismiss via backdrop click, Escape key, close buttons).
     - Keyboard shortcuts (F5 quicksave, F9 quickload, Space, Enter, Arrow keys, 1-9 hotkeys).
     - Storage persistence (JSON import/export, local storage state saving, tutorial flags).
     - **ARG Mystery & TINAG Standard**: Audit copy and labels for ARG mystery violations; remove un-diegetic `(ARG)` tags, explicit walkthroughs ("ARG Guidance"), premature fleet meta-spoilers, or leaked master passkeys.
   - Trace each JavaScript handler: ensure referenced functions exist, IDs are valid, and edge inputs do not crash.

4. **In-Line Repairs**:
   - Fix broken event handlers, missing element hooks, or typo bugs directly in the HTML file.
   - Wire unhandled modal dismissals (Escape key, backdrop click).
   - Wire missing quicksave/quickload or export/import features if missing or trivial.
   - Do NOT perform large architectural rewrites; log deep structural defects for QA if complex.

5. **Build & Size Verification**:
   - Run `npm run build` inside `KiloOS/` to confirm zero Vite build breaks.
   - Ensure the HTML file does not exceed 999 KB.

6. **Queue Handoff & Terse Logging (CRITICAL)**:
   - Edit [next_work.md](../../next_work.md):
     - Update YAML frontmatter:
       - Set `current_agent` to the next scheduled agent in `agent_rotation` (or `kilo-qa` if critical defects were found).
       - Advance `current_targets.kilo_tester` to the next app in the queue list.
       - Set `status: ready`.
       - Update `last_run.agent: kilo-tester`, `last_run.app: <TargetApp>`, and `last_run.timestamp`.
     - In the Execution Log section of `next_work.md`, append a run entry:
       - **Strict limit**: ≤8 lines of terse bullet points. No paragraphs.
       - Format: `PASS ✅ (N issues, M fixed)` or `PASS ✅ (No issues found)`.
       - Log WHAT changed + test results, not HOW.
     - Move the completed app to the bottom of the Tester queue table.

7. **Commit and Push**:
   - `git add <modified files> next_work.md`
   - `git commit -m "audit(test): UI audit and fixes for <AppName>"`
   - `git push` (if push fails due to remote update, run `git pull --rebase` and push again).
   - STOP immediately. Process exactly ONE app only.
