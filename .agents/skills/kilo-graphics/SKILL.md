---
name: kilo-graphics
description: >-
  Executes game visual art overhauls replacing programmer art (primitive canvas shapes)
  with Imagen 3 generated game assets (sprites, sprite sheets, seamless backgrounds) via the
  2-stage asset pipeline, eliminates perimeter glints, and uses 1-command verification/handoff.
---

# KiloApps Game Content & Graphics Skill (`kilo-graphics`)

Replaces programmer art with Imagen 3 generated assets and eliminates perimeter glints on **exactly ONE game per turn**.

## ⚡ Flash Execution & Tool-Budget Protocol (CRITICAL)
- **Tool Call Budget (<= 25 tool calls)** & **Commit Checkpoint (~12 tool calls)**.
- **Fast Skip Path (<= 3 tool calls total!)**:
  - If the target game is pure vector/wireframe, abstract board/text, or **already has production sprites integrated**, **DO NOT compile C binaries, DO NOT run Vite builds, and DO NOT run startup tests!**
  - Immediately run `python scripts/advance_turn.py --agent kilo-graphics --app <TargetGame> --title "Skip — Asset Suitability" --bullet "⏭️ Skip — Imagen 3 asset replacement not appropriate or already integrated for <TargetGame>."`, commit `next_work.md`, push, and **STOP immediately**!
- **1-Command Verification (when edits were made)**:
  ```powershell
  python scripts/verify_turn.py --app <TargetGame>
  ```
- **1-Command Queue Handoff & Logging**:
  ```powershell
  python scripts/advance_turn.py --agent kilo-graphics --app "<TargetGame>" `
    --title "Imagen 3 Visual Assets & Glint Sweep" `
    --bullet "Status: PASS ✅ (<copy summary from verify_turn.py>)." `
    --bullet "<Sprites/backgrounds integrated and rendering fallbacks verified>." `
    --bullet "<Glint sweep & icon uniqueness verified>."
  ```
- **Commit, Push & STOP**:
  ```powershell
  git add -A
  git commit -m "content(games): graphics pass for <TargetGame>"
  git push origin main
  ```

---

## Graphics Directives
1. **Replace Programmer Art**: Replace primitive Canvas circles/rectangles in 2D games (`KiloOS/public/apps/k<name>.html`) with sprite rendering (`new Image()`, graceful procedural fallback while loading) or rich multi-stop pixel/canvas textures if `generate_image` is unavailable in headless CLI mode.
2. **2-Stage Asset Pipeline (when `generate_image` tool is available)**:
   - Stage 1: Generate on solid `#FF00FF` magenta background (`1024x1024`).
   - Stage 2: `uv run scripts/asset_pipeline.py process-sprites --frames <f1> --out-strip <strip.png> --out-atlas <atlas.json> --box-size 128`.
3. **Perimeter Glint & Comet Ban**: Search for and remove any traveling border dots, orbiting balls, or rotating specular glints across HTML and C.
4. **Icon Uniqueness**: Run `python scripts/check_icons.py` if any icon was added or modified.
