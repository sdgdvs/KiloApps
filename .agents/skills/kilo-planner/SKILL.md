---
name: kilo-planner
description: >-
  Executes daily phase planning, queue compaction, and token optimization across the KiloApps fleet.
  Use this skill to archive aging execution logs from next_work.md, evaluate pass progression,
  adjust priority targets, enforce token hygiene caps, and commit/push changes.
---

# KiloApps Daily Planner & Compactor Skill

This skill performs daily high-level fleet maintenance and token compaction.

## Pre-flight
1. Ensure git working tree is clean: `git status`.
2. Inspect [next_work.md](../../next_work.md).

## Planning & Compaction Procedure
1. **Log Compaction**:
   - Inspect the Execution Log section in `next_work.md`.
   - Keep only the 5 most recent agent run entries in `next_work.md`.
   - Append older entries to `archive/fleet_execution_archive.md`.

2. **Phase Progression Review**:
   - Audit the queue tables for QA (Pass 5) and App Tester.
   - If an entire pass is completed, advance the phase counter.
   - Adjust `current_targets` if manual priority interventions are needed.

3. **Queue Handoff Update**:
   - Verify YAML frontmatter in `next_work.md`:
     - Ensure `status: ready`.
     - Confirm next assigned agent (`current_agent`).
     - Set `last_planner_run` timestamp.

4. **Commit and Push**:
   - `git add next_work.md archive/`
   - `git commit -m "chore(plan): daily fleet compaction and phase sync"`
   - `git push` (if push fails due to remote update, run `git pull --rebase` and push again).
   - STOP immediately.
