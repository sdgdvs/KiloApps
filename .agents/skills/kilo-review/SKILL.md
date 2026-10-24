---
name: kilo-review
description: >-
  Executes a high-precision Gemini 4 Argon visual, feature, gameplay, and usability critic pass
  on a single KiloApps application using visual_probe.py contact sheets and code inspection,
  then injects surgical fix specs directly into Flash agent queues in next_work.md.
---

# KiloApps Argon Visual, Feature & Usability Critic (`kilo-review`)

Runs on `gemini-4-argon-high` once per full agent rotation (1 in 8 turns) to catch subtle visual clipping, dead/Potemkin features, broken gameplay loops, unreadable contrast, and usability flaws that Flash agents miss.

## ⚡ Division of Labor (Critic vs. Builder — CRITICAL)
- **Argon (`kilo-review`) diagnoses; Flash executes.** Do **NOT** spend your turn editing HTML/C code or compiling native binaries!
- Your mission is to perform an uncompromising **human-eye visual + interactive + code review** of the target app in **<= 8 tool calls**, and then inject concrete, surgical fix instructions directly into the appropriate Flash specialist queues (`kilo-usability`, `kilo-tester`, `kilo-expander`, `kilo-graphics`, `kilo-soundscape`) via `scripts/advance_turn.py --inject-fix`.
- When you inject a fix for a **Mature** (archived) app, `scripts/advance_turn.py --inject-fix` **automatically un-quarantines the app from `## Mature Applications`** and places it at **Row 1 (Active Target)** of the designated Flash queue table so Flash fixes it on the very next cycle!

## 🔍 4-Step Execution Workflow (<= 8 Tool Calls)

### Step 1: Sync & Identify Target
1. Run `git pull --rebase --autostash`.
2. Check your assigned target `<App>` from `current_targets.kilo_review` in `next_work.md`.

### Step 2: Run Headless Visual & Gameplay Probe
1. Execute the headless visual & input probe at the app's exact KiloOS window dimensions:
   ```bash
   uv run scripts/visual_probe.py --app <App>
   ```
2. Call `view_file` on `docs/gallery/review/<app_lower>/sheet.png` to inspect the 4-panel contact sheet (`1. BOOT | 2. READY (Dismissed) | 3. AFTER INPUT | 4. INPUT RESPONSE HEATMAP`) with your native vision.

### Step 3: Deep Visual, Feature & Usability Audit
Inspect `KiloOS/public/apps/<app_lower>.html` (or `KiloOS/public/web/<page>.html`) alongside `sheet.png` across three axes:
1. **Visual & Layout Integrity (`kilo-usability` / `kilo-graphics`)**:
   - Are any panels, canvases, buttons, or status bars clipped, overlapping, or squished inside the real KiloOS window size?
   - Is any text illegible (dark-on-dark contrast, tiny `< 10px` fonts, overflowing labels, broken table alignment)?
   - Are there leftover programmer-art geometric placeholders on game entities or traveling perimeter border glints?
2. **Feature & Gameplay Depth (`kilo-tester` / `kilo-expander`)**:
   - Did `sheet.png` Panel 3/4 show `0.00%` input response or a stuck overlay after dismissal? Why did keyboard/canvas input fail to change visible state?
   - Are there "Potemkin" controls—buttons, tabs, sliders, or modes that look interactive in the DOM but are no-ops, dead stubs, or fail to update the simulation/game state?
   - In games: is the core loop playable, balanced, and responsive immediately after dismissing the splash/tutorial, with working win/loss feedback and HUD updates?
3. **Ergonomics & OS Mesh (`kilo-usability` / `kilo-expander`)**:
   - Are intuitive keyboard shortcuts (`Escape` to close modals, `Space`/`Enter`/`Arrow` keys), HiDPI canvas scaling (`devicePixelRatio`), and KiloOS VFS Open/Save hooks working cleanly?

### Step 4: Inject Surgical Fix Specs & Advance Turn (1 Command)
Use `python scripts/advance_turn.py` with one or more `--inject-fix "<target_agent>:<App>:<Surgical Fix Spec>"` flags.
- **Format of `--inject-fix`**:
  - `<target_agent>`: One of `kilo-usability`, `kilo-tester`, `kilo-expander`, `kilo-graphics`, or `kilo-soundscape`.
  - `<App>`: Exact target app name (e.g. `KBudget`, `KFarm`, `KMech`).
  - `<Surgical Fix Spec>`: Concrete, actionable instructions naming the exact UI elements, functions, CSS classes, or mechanics Flash must fix (keep to 1 concise sentence per flag; pass multiple `--inject-fix` flags if dispatching to multiple Flash specialists). **Do NOT use backticks (`` ` ``) inside double-quoted PowerShell strings.**
- **Example when defects are found (un-quarantines Mature app & queues fixes at Row 1)**:
  ```bash
  python scripts/advance_turn.py --agent kilo-review --app KFarm \
    --title "Argon Visual, Feature & Usability Review" \
    --bullet "Status: AUDIT COMPLETE (reviewed 4-panel sheet.png at 860x583 and audited kfarm.html)." \
    --bullet "Visual/UX Findings: Crop market modal clips bottom action buttons at 583px height; season progress bar text has low contrast." \
    --bullet "Feature Findings: Livestock feed button deducts gold even when barn is empty." \
    --inject-fix "kilo-usability:KFarm:Fix crop market modal max-height/overflow clipping at 860x583 viewport and increase contrast on #season-bar label." \
    --inject-fix "kilo-tester:KFarm:Guard feedLivestock() so it is disabled and does not deduct gold when barn animal count is 0."
  ```
- **Example when an app is genuinely 100% human-grade (0 defects found)**:
  ```bash
  python scripts/advance_turn.py --agent kilo-review --app KCalc \
    --title "Argon Visual, Feature & Usability Review (Verified Clean)" \
    --bullet "Status: VERIFIED CLEAN ✅ (inspected sheet.png at 760x563 and audited kcalc.html; 0 visual, feature, or UX defects)." \
    --bullet "Audit Summary: All 5 calculator modes, radix strip, and HiDPI graphing canvas render and respond cleanly."
  ```
- **Commit & Push**:
  ```bash
  git add next_work.md archive/
  git commit -m "audit(kilo-review): Argon visual & usability review for <App>"
  git push origin main
  ```
