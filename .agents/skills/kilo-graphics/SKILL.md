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
7. **🖼️ 2D Sprite Sheet & Seamless Texture Pipeline (Technical Game Asset Pipeline)**:
   - When generating or expanding sprite animations and environment terrain textures for 2D games, execute the standardized 2-stage asset pipeline:
   - **Stage 1 (Asset Generation via `generate_image`)**:
     - *Character Sprites / Run Cycles*: Orthographic 2D view, flat diffuse lighting, zero directional/floor shadows, solid magenta (`#FF00FF`) background, 1:1 aspect ratio, 1024x1024. Keep character visual tokens consistent across all prompts.
     - *Terrain / Ground Textures*: "Seamless tileable texture, top-down albedo map, orthographic projection, no vignette, uniform diffuse lighting, 1024x1024".
   - **Stage 2 (Automated Post-Processing via `scripts/asset_pipeline.py`)**:
     - *Sprites*: `uv run scripts/asset_pipeline.py process-sprites --frames <f1> <f2> ... --out-strip <strip.png> --out-atlas <atlas.json> --box-size 128`
       - Keys out `#FF00FF` to `alpha = 0` (32-bit RGBA) with despill and dark fringe cleanup.
       - Auto-crops and centers each sprite into a uniform 128x128 bounding box.
       - Packs frames into a horizontal sprite sheet (e.g. 512x128 `run_strip4.png`).
       - Generates accompanying Phaser/Unity-compatible JSON coordinate atlas.
     - *Textures*: `uv run scripts/asset_pipeline.py process-texture --input <raw.jpg> --out-seamless <path.png> [--quantize 256]`
       - Applies `ImageChops.offset(512, 512)` seam test and blends quadrant seams to guarantee mathematical tileability.
       - Use `--quantize 256` if needed to guarantee the texture remains strictly below the `< 999 KB` size ceiling.

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
