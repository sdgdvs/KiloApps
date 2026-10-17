---
name: kilo-usability
description: >-
  Audits and refines user experience, responsive layout, window sizing, and input ergonomics across KiloApps.
  Use this skill to fix clipped layouts, ensure crisp HiDPI canvas scaling, add clear help guides (F1/H),
  tune default window sizes in KiloOS/src/App.jsx, verify builds, log tersely to next_work.md,
  advance the queue handoff, and commit/push.
---

# KiloApps Usability & UX Skill

This skill executes usability, layout, and UX enhancements on exactly ONE application per turn (or KiloOS shell).

## Quality & Craft Directive
- **Iterative Craft**: Audit window dimensions, interactive layout, responsive design, keyboard shortcuts, and canvas HiDPI crispness. Inspect diffs and code after editing to verify layout stability and dual web/native parity.
- **Validation**: Run `cd KiloOS && npm run build` and `uv run scripts/test_app_startup.py --app <TargetApp>` to ensure startup modals dismiss cleanly and gameplay/UI surfaces are not occluded.
- **No Full-Repo Test Suites**: Run only target build/startup verification. NEVER run repo-wide suites (`quality_gate.js`, screenshot generation).
- **No Archives/Receipts**: Never read `archive/` or `.agents/receipts/`.
- Advance queue in `next_work.md`, commit, push, and STOP immediately.

## Pre-flight
1. Ensure git working tree is clean: `git status`.
2. Pull latest changes: `git pull --rebase`.
3. Open [next_work.md](../../next_work.md) to inspect the usability target (`current_targets.kilo_usability`).

## Usability Focus Areas
1. **Window Dimensions & Layout**:
   - Verify that the app's default window dimensions in `KiloOS/src/App.jsx` (`defaultWidth`, `defaultHeight`) comfortably display the full UI without awkward internal scrollbars or clipped action buttons.
2. **First-Run Onboarding & Help**:
   - Ensure a visible, easily discoverable help prompt exists (e.g. `Press H or F1 for Help / Controls`).
   - Modal help guides must have clear instructions, hotkey lists, and be easily dismissed with Escape, backdrop click, or Close button.
3. **Canvas & Font Crispness**:
   - Ensure canvas-based apps account for `window.devicePixelRatio` so rendering is crisp on high-DPI displays rather than blurry.
4. **Responsive Controls & Touch/Mouse Handling**:
   - Controls must be sized for easy clicking/tapping with clear hover/active feedback.
5. **Mature Application Quarantine Protocol (DIRECTOR MANDATE - CRITICAL)**:
   - If an app has undergone multiple passes and possesses clean layout, crisp rendering, robust keyboard navigation, and intuitive controls:
     - **DO NOT rotate it to the queue bottom!** (Rotating to the bottom causes endless skip loops that waste tokens repeatedly).
     - **Quarantine to Mature**: Move the app into `## Mature Applications (Archived from Active Queues)` in `next_work.md`.
     - **Remove from Queues**: Remove the app completely from all active queues and upcoming lists across `next_work.md`.
     - **Advance Target**: Advance `current_targets.kilo_usability` to the next non-mature app in line.
     - Log: `⏭️ Skip — promoted <TargetApp> to Mature Applications quarantine.` Advance rotation, commit, push, and STOP cleanly.

## Verification
1. Verify web app build: `cd KiloOS && npm run build`.
2. Verify size constraint: `< 999 KB`.

## Queue Handoff & Terse Logging (CRITICAL)
1. **Edit [next_work.md](../../next_work.md)**:
   - Advance `current_targets.kilo_usability` to next app in queue.
   - Set `current_agent` to next scheduled agent in `agent_rotation`.
   - Set `status: ready`.
   - Update `last_run.agent: kilo-usability`, `last_run.app: <TargetApp>`, and `last_run.timestamp`.
   - Append terse run log (strict limit: ≤ 8 lines of concise bullet points).
2. **Commit and Push**:
   - `git add <modified files> next_work.md`
   - `git commit -m "ux(usability): usability and layout polish for <TargetApp>"`
   - `git push` (if rejected, run `git pull --rebase` then push).
   - STOP immediately.
