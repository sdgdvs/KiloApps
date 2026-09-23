---
name: kilo-planner
description: >-
  Executes daily phase planning, queue compaction, and token optimization across the KiloApps fleet.
  Use this skill to archive aging execution logs from next_work.md, evaluate pass progression,
  adjust priority targets, enforce token hygiene caps, and commit/push changes.
---

# KiloApps 24-Hour Fleet Planner & Master Compactor Skill

This skill is invoked automatically every 24 hours by the orchestrator (`scripts/orchestrate.py`) to assess overall project health, rework the daily agent rotation schedule, rebalance target queues across all skills, and compact execution logs.

## Pre-flight
1. Ensure git working tree is clean: `git status`.
2. Pull latest changes: `git pull --rebase`.
3. Inspect [next_work.md](../../next_work.md) and recent commits (`git log -n 15 --oneline`).

## 24-Hour Planning & Rework Procedure
1. **Fleet Velocity & Health Assessment**:
   - Review recent logs in `next_work.md` and `archive/fleet_execution_archive.md`.
   - Identify bottlenecks: Are there persistent QA failures? Has an app failed UI audit? Is a new app concept needed? Are certain categories needing graphical or usability polish?
   - Identify mature apps: Any app with 6+ clean passes should be deprioritized or marked mature to avoid unproductive churn.

2. **Daily Agent Schedule & Target Rework**:
   - In `next_work.md`, adjust the `agent_rotation` list for the upcoming 24-hour window across the 6 worker agents:
     - `kilo-creator` (new app creation / deep genre worlds)
     - `kilo-graphics` (game content, visual polish, AI balance)
     - `kilo-tester` (interactive UI element & button audits)
     - `kilo-usability` (layout, window sizing, crisp canvas, help UX)
     - `kilo-qa` (Pass 5 tutorial & state persistence, build verification)
     - `kilo-expander` (deep features for dev, productivity, media tools)
   - Update `current_targets` for each active queue to align with current priorities.
   - Evaluate `virtual_web_target`: review progress against `virtual_web_rotation` in `next_work.md`, verify sites meet the Anti-Potemkin Quality Standard (genuine interactive depth, Web Audio, simulated backends), and advance `virtual_web_target` to the next site in rotation.
   - If a new app was recently created, ensure subsequent turns prioritize its usability, testing, and graphical polish.

3. **Daily Fleet Icon Uniqueness Audit (DIRECTOR MANDATE - CRITICAL)**:
   - Run `python scripts/check_icons.py`.
   - Verify that all apps in `KiloOS/src/App.jsx` point to existing `.ico` files and have 0 duplicate SHA256 hashes.
   - If any duplicate or missing icons are detected, immediately resolve via `python scripts/check_icons.py --fix` or assign `kilo-graphics` to generate unique icons for them.

4. **Log Compaction**:
   - Retain only the 5 most recent agent execution log entries in `next_work.md`.
   - Move older entries into `archive/fleet_execution_archive.md`.

5. **Queue Handoff & Timestamp Update**:
   - In YAML frontmatter of `next_work.md`:
     - Update `last_planner_run` to the current UTC timestamp (e.g., `2026-09-17T...Z`).
     - Set `current_agent` to the first scheduled worker agent in `agent_rotation`.
     - Set `status: ready`.

5. **Commit and Push**:
   - `git add next_work.md archive/`
   - `git commit -m "chore(plan): daily 24h fleet queue rework and log compaction"`
   - `git push` (if rejected, run `git pull --rebase` then push).
   - STOP immediately.
