---
name: kilo-planner
description: >-
  Executes daily 24-hour phase planning, multi-turn Mission Pipeline Table replenishment, Mature
  Quarantine enforcement, and log compaction across the KiloApps fleet.
---

# KiloApps 24-Hour Fleet Planner Skill (`kilo-planner`)

Invoked automatically every 24 hours by `scripts/orchestrate.py` to replenish the multi-turn **Mission Pipeline Tables** in `next_work.md`, enforce Mature Application Quarantine, audit icon uniqueness, and compact logs.

## ⚡ Flash Execution & Tool-Budget Protocol (CRITICAL)
- **Tool Call Budget (<= 20 tool calls)**:
  1. `git pull --rebase --autostash`
  2. Read `next_work.md` and run `python scripts/check_icons.py` + `python scripts/rotate_logs.py`.
  3. Replenish any queue table in `next_work.md` that has fewer than 6 upcoming targets using non-mature apps from `KiloOS/src/App.jsx`.
  4. Update `last_planner_run` to current UTC ISO timestamp (`YYYY-MM-DDTHH:MM:SSZ`), commit, push, and STOP!

## Planning Rules (CRITICAL — Prevent Queue Collapse!)
1. **Never Pile Multiple Agents Onto the Same App Simultaneously**:
   - Stagger `current_targets` across the 7 queues so `kilo-graphics`, `kilo-soundscape`, `kilo-tester`, `kilo-usability`, `kilo-qa`, and `kilo-expander` each work on **different** non-mature applications in parallel without git merge conflicts or redundant passes.
2. **Maintain 8–12 Target Rows Per Mission Pipeline Table**:
   - Every section (`### 1.` through `### 7.`) in `next_work.md` uses a 3-column Markdown table (`| Target | Primary Files | Phase VI Mission Spec (<999 KB) |`).
   - Keep each table stocked with 8–12 distinct non-mature targets so `scripts/advance_turn.py` and `scripts/orchestrate.py` have a deep, self-advancing runway for 50+ turns.
3. **Strict Mature & Skip-List Hygiene**:
   - Never place any app listed in `## Mature Applications` into any active target or pipeline table.
   - Never assign pure utility/text apps to `kilo-graphics` or `kilo-soundscape`.
   - Never assign games that already have complete sprite packs (`KSpace`, `KFortress`, `KQuest`, `KMaze`, `KMech`, `KAbyss`, `KColosseum`) to `kilo-graphics`.
