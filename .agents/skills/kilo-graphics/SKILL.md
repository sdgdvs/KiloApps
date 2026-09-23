---
name: kilo-graphics
description: >-
  Executes gameplay content expansions, visual art polish, and balance passes for KiloApps games.
  Use this skill to enhance ASCII/canvas graphics, enemy/stage variety, AI difficulty balance,
  sound effects, verify file sizes (<999 KB) and builds, log tersely to next_work.md,
  advance the queue handoff, and commit/push.
---

# KiloApps Game Content & Graphics Skill

This skill executes content depth, visual polish, or balance passes on exactly ONE game per turn.

## Pre-flight
1. Ensure git working tree is clean: `git status`.
2. Pull latest changes: `git pull --rebase`.
3. Open [next_work.md](../../next_work.md) to inspect the graphics/games target (`current_targets.kilo_graphics`).

## Category Directives & Rules
1. **🎮 Deep Games** (*KRogue, KQuest, KStarship, KAlchemy, KSpace, KAsteroids, KMaze, KPac, KBreakout, KSnake*):
   - **Focus**: Add procedural depth, enemy variety, storyline/lore, crafting recipes, combat mechanics, and particle/visual feedback.
   - Expand toward the 999 KB budget with rich, replayable systems.
2. **♟️ Classic & Board/Puzzle Games** (*KChess, KGo, KReversi, KConnect4, KSolitaire, KSudoku, KTowers, KMines, KTetris, KPong*):
   - **DO NOT** add campaigns, boss battles, or combat skills.
   - **Focus**: AI intelligence, difficulty curves (Easy/Medium/Hard tuning), responsive controls, clean board rendering, victory animations, and audio feedback (Genesis YM2612 FM & SNES SPC700 chiptunes per `arg_plan.md`).
3. **Maturity & Skip Protocol**:
   - For mature games (6+ passes) that are already cohesive, balanced, and complete without pending directives: log `⏭️ Skip — app is mature, balanced, and complete.` Rotate to queue bottom and finish turn cleanly without churning code.
4. **Alternate Reality Fictionalization Mandate**:
   - All commercial video game titles, software products, corporate entities, and demoscene warez groups must be fictionalized parodies (e.g. *Surreal Tournament*, *Tremor III Arena*, *VoidCraft*, *Machina Ex*, *FLARELIGHT*, *RAZOR 1999*, *SlashNet*, *Cabled*). Never use real trademarked names. Enforced algorithmically by `scripts/security_lint.py`.
5. **🚫 Specular Glint & Traveling Comet Ban (DIRECTOR MANDATE - CRITICAL)**:
   - On every pass, search for and REMOVE any traveling/rotating specular glint comets, perimeter glint dots, or moving border balls across both web (HTML) and native (C).
   - These traveling dots are distracting, look like rogue projectiles/balls, and annoy players across games.
   - Replace with clean, static, or period-accurate borders without traveling dots or orbital glint particles. NEVER add new perimeter traveling glints.
6. **🎨 Distinctive App Icon Uniqueness Audit (DIRECTOR MANDATE - CRITICAL)**:
   - On every pass, run `python scripts/check_icons.py` to audit icon uniqueness across all applications in `KiloOS/src/App.jsx`.
   - Ensure every app possesses a unique, distinctive 32x32 `.ico` file in `KiloOS/public/assets/icons/`. Reusing icons or copying existing `.ico` files is strictly prohibited.
   - If missing or duplicate icon hashes are detected, run `python scripts/check_icons.py --fix` (or generate unique pixel art) to ensure 100% icon uniqueness across the fleet.

## Verification
1. Verify web app build: `cd KiloOS && npm run build`.
2. Verify size constraint: `< 999 KB`.
3. Verify icon uniqueness: `python scripts/check_icons.py`.

## Queue Handoff & Terse Logging (CRITICAL)
1. **Edit [next_work.md](../../next_work.md)**:
   - Advance `current_targets.kilo_graphics` to next game in queue.
   - Set `current_agent` to next scheduled agent in `agent_rotation` (or hand off to `kilo-tester` / `kilo-qa` if extensive logic changed).
   - Set `status: ready`.
   - Update `last_run.agent: kilo-graphics`, `last_run.app: <TargetGame>`, and `last_run.timestamp`.
   - Append terse run log (strict limit: ≤ 8 lines of concise bullet points).
2. **Commit and Push**:
   - `git add <modified files> next_work.md`
   - `git commit -m "content(games): graphics and balance pass for <TargetGame>"`
   - `git push` (if rejected, run `git pull --rebase` then push).
   - STOP immediately.
