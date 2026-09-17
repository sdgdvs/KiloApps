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
5. **Maturity & Skip Protocol**:
   - If an app has undergone 6+ passes and possesses clean layout, crisp rendering, and intuitive controls: log `⏭️ Skip — app usability is complete and mature.` Rotate to queue bottom and stop cleanly.

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
