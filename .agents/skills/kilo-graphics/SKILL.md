---
name: kilo-graphics
description: >-
  Executes game visual art overhauls replacing programmer art (vector cutouts, primitive canvas shapes)
  with Imagen 3 generated game assets (sprites, sprite sheets, seamless backgrounds) via the 2-stage asset pipeline.
  If the target app is not appropriate for Imagen 3 assets, cleanly skips the turn.
---

# KiloApps Game Content & Graphics Skill

This skill executes visual art overhauls replacing programmer art with Imagen 3 generated assets on exactly ONE game per turn.

## Token Efficiency Directive (STRICT BUDGET: <= 10 Tool Calls)
- **Tool Budget**: Complete this turn in **<= 10 tool calls**.
- **Skip Turn Fast-Path**: If target is inappropriate for Imagen 3 assets, log skip, advance queue in `next_work.md`, commit, and STOP immediately (do NOT inspect glints, rebuild binaries, or read archives).
- **Single-App Build Only**: Run only `cd KiloOS && npm run build`. NEVER run full-repo test suites (no `quality_gate.js`, no screenshot regeneration).
- **No Archives/Receipts**: Never read `archive/` or `.agents/receipts/`.
- Advance queue in `next_work.md`, commit, push, and STOP immediately.

## Pre-flight
1. Ensure git working tree is clean: `git status`.
2. Pull latest changes: `git pull --rebase`.
3. Open [next_work.md](../../next_work.md) to inspect the graphics/games target (`current_targets.kilo_graphics`).
4. **Target Suitability Assessment**: Evaluate if the target game has programmer art suitable for replacement with Imagen 3 assets. If NOT appropriate (e.g. pure vector/wireframe arcade games, abstract board games, or games that already have production art), SKIP THE TURN immediately per Rule 2 below.

## Category Directives & Rules
1. **🖼️ Exclusive Mission: Replace Programmer Art with Imagen 3 Assets (DIRECTOR MANDATE - CRITICAL)**:
   - For all upcoming turns, the `kilo-graphics` agent must do NOTHING BUT replace programmer art (primitive vector shapes, Canvas fills, geometric cutouts, procedural line art) with authentic Imagen 3 generated game assets (sprites, sprite sheets, seamless textures, and backgrounds) via the 2-stage asset pipeline.
   - Do not add random gameplay mechanics, new campaign stages, or unrequested features. Channel all turns into visual asset generation, chroma keying, sprite strip packing, and engine rendering integration.
2. **⏭️ Turn Skipping for Inappropriate Targets (DIRECTOR MANDATE - CRITICAL)**:
   - If `kilo-graphics` reaches an application where Imagen 3 asset generation would NOT be appropriate:
     - *Pure Vector / Wireframe Classics*: Games whose foundational aesthetic is authentic retro vector math or line art (e.g. *KAsteroids*, *Battlezone*-style wireframes).
     - *Classic Abstract Board / Puzzle Games*: Traditional board games where standard geometric pieces or symbols are appropriate (*KChess*, *KGo*, *KReversi*, *KTowers*).
     - *Mature / Completed Art*: Games that already possess complete, production-grade custom art with zero programmer cutouts.
     - *Text / Terminal Apps*: Pure text or command-line style utilities.
   - **ACTION**: Skip the turn immediately! Format: `⏭️ Skip — Imagen 3 asset replacement not appropriate for <TargetGame>`.
   - Rotate the target to the bottom of the queue, advance `current_agent`, commit, push, and STOP without touching code or inventing arbitrary additions.
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
