---
name: kilo-adhoc
description: >-
  Executes user-directed, ad-hoc feature changes, mechanics balancing, fleet orchestration upgrades,
  or bug fixes across KiloApps. Enforces dual-target parity (Web HTML + Win32 C), retro 1999
  constraints, clean builds (<999 KB), security lint, and seamless git commit/push.
---

# KiloApps Ad-Hoc Modification Skill (`kilo-adhoc`)

Use this skill when responding to human user requests for specific features, rebalancing, fixes, or fleet orchestration upgrades across KiloApps.

## 1. Pre-Flight & Concurrency Safety
1. **Check for Running Scheduled Agent**: Verify `Test-Path ".agents/.orchestrator.lock"` and `Get-Process agy -ErrorAction SilentlyContinue`. If a scheduled Flash agent is mid-turn, do not touch `next_work.md` or run `git pull` until its lock releases.
2. **Pull Latest Changes**: Run `git pull --rebase --autostash` before modifying files.
3. **Identify Dual Parity Targets**:
   - **Web Client**: `KiloOS/public/apps/k<name>.html` (or `KiloOS/public/web/<name>.html`)
   - **Native Win32 Client**: `<AppName>/main.c` + `<AppName>/build.bat` -> `<AppName>.exe` and `KiloOS/public/exe/<AppName>.exe`.

## 2. Specialist Agent & Rules Routing Index (`O(1)` Lazy Lookup)
After identifying the user's goal, consult the table below and read **at most ONE** matching specialist `SKILL.md` (plus any relevant on-demand rule in `.agents/rules/`) to inherit its domain-specific standards and workflows.

> **Token Scaling & Queue Isolation Guardrails (CRITICAL)**:
> - **Read at most 1 specialist skill**: Never load multiple `SKILL.md` files into context. For pure KiloOS shell (`KiloOS/src/App.jsx`), CI/CD (`.github/workflows/`), or orchestrator script fixes, skip reading app specialist skills entirely.
> - **Do NOT run `advance_turn.py` unless requested**: Scheduled specialist skills end with `python scripts/advance_turn.py --agent ...` to rotate `next_work.md`. When executing an ad-hoc user request, **borrow only the domain craft/verification rules** from the specialist skill and follow **Section 5 (Logging & Deployment)** below so you do not disrupt the scheduled rotation. (Exception: if the user explicitly asks for a `kilo-review` critic pass or queue injection, use `advance_turn.py --inject-fix`.)

### Specialist Fleet Skills (`.agents/skills/`)
| Agent / Skill | Skill File | Primary Responsibility & When to Read |
|---|---|---|
| **`kilo-creator`** | `.agents/skills/kilo-creator/SKILL.md` | Building a **brand-new app**, new canonical 1999 Virtual Web destination (`kweb://...`), or major greenfield game world (`KiloOS/src/App.jsx` registration + icon + dual targets). |
| **`kilo-graphics`** | `.agents/skills/kilo-graphics/SKILL.md` | Game **visual art overhauls**: replacing primitive canvas shapes with Imagen 3 sprites/backgrounds (`generate_image` + Stage 2 alpha chroma-key pipeline) and fixing perimeter glints. |
| **`kilo-soundscape`** | `.agents/skills/kilo-soundscape/SKILL.md` | **[PAUSED]** Formerly procedural Web Audio BGM trackers (now disabled across fleet via `window.__KILO_BGM_ENABLED` awaiting Lyria-3.5 -> MIDI pipeline). Only discrete SFX earcons are active. |
| **`kilo-tester`** | `.agents/skills/kilo-tester/SKILL.md` | Debugging **broken app mechanics, dead UI buttons, SQL/parser bugs, or runtime exceptions**: interactive `visual_probe.py` audit + inline HTML/C bug repair. |
| **`kilo-usability`** | `.agents/skills/kilo-usability/SKILL.md` | Fixing **layout clipping (`cutOff`), scroll overflow, HiDPI canvas blur (`devicePixelRatio`), modal focus traps, or keyboard/HUD ergonomics** at the app's real KiloOS window size. |
| **`kilo-qa`** | `.agents/skills/kilo-qa/SKILL.md` | **Pass 5 QA & graduation**: `F5`/`F9` quicksave/quickload state persistence, first-run tutorial flags, `visibilitychange` timer cleanup, Win32 C parity, and Mature quarantine. |
| **`kilo-expander`** | `.agents/skills/kilo-expander/SKILL.md` | **Phase VI feature expansions**: KiloOS VFS (`VFS_READ`/`VFS_WRITE`/`VFS_SHOW_OPEN`/`VFS_SHOW_SAVE`) & `OS_LAUNCH_APP` IPC mesh, RFMS online multiplayer (`retro_multiplayer.js`), and deep workstation tools. |
| **`kilo-review`** | `.agents/skills/kilo-review/SKILL.md` | **Gemini 4 Argon visual/feature/usability critic pass**: running `visual_probe.py`, inspecting `sheet.png` + source code, and injecting surgical fix specs into `next_work.md` queues. |
| **`kilo-planner`** | `.agents/skills/kilo-planner/SKILL.md` | **24-hour fleet queue planning**: replenishing Mission Pipeline Tables in `next_work.md`, enforcing Mature Application Quarantine, checking icon uniqueness, and rotating logs. |
| **`kilo-vision-audit`** | `.agents/skills/kilo-vision-audit/SKILL.md` | **Batch visual quality scoring**: auditing screenshots in `docs/gallery/screenshots/` and outputting structured scores to `docs/gallery/vision_scores.json`. |

### On-Demand Workspace Rules (`.agents/rules/`)
| Rule File | When to Read |
|---|---|
| `.agents/rules/architecture.md` | Modifying `KiloOS/src/App.jsx`, window manager state, `APPS` registry, or iframe `postMessage` boundaries. |
| `.agents/rules/quality-standards.md` | Writing/refactoring JS storage wrappers (`safeGet`/`safeSet`), `safeFetch`, or JSDoc types. |
| `.agents/rules/arg-lore.md` | Touching 1999 lore, `kweb://` parodies, passkeys, TINAG mystery elements, or multiplayer matchmaking gates. |

## 3. Design & Platform Constraints
- **Strict Size Ceiling**: Every `.html` and `.exe` file must remain strictly **< 999 KB** (999,000 bytes).
- **Phase VI Cross-App VFS & IPC Standard**: Support KiloOS `VFS_READ` / `VFS_WRITE` / `OS_LAUNCH_APP` postMessage integration where appropriate with graceful standalone fallback.
- **Anti-Trademark / Parody Rule**: Must pass `python scripts/security_lint.py`.
- **ARG Mystery Preservation (TINAG)**: Never use `(ARG)` labels or leak `ECHO-1999-ARCHITECT`.
- **Multiplayer Autostart Prohibition**: Never autostart into matchmaking on load; require explicit user Connect click and include 25s solo AI fallback (`retro_multiplayer.js`).

## 4. One-Command Verification Pipeline (MANDATORY)
Run the unified verification script (which includes `scripts/visual_probe.py --compare-head` and generates a before/after contact sheet at `docs/gallery/review/<targetapp>/sheet.png`):
```powershell
python scripts/verify_turn.py --app <TargetApp> --build-native
```
- Call `view_file` on `docs/gallery/review/<targetapp>/sheet.png` to visually inspect boot, ready, after-input, and `git HEAD` diff frames in a single call.
- Or if modifying fleet-wide scripts/shell, run `npm run build` inside `KiloOS/` and `python scripts/security_lint.py`.

## 5. Logging & Deployment
1. Prepend a terse log entry (≤ 6 lines) under `## Recent Execution Logs` in `next_work.md` (and run `python scripts/rotate_logs.py` so old entries archive at 0 token cost). Preserve scheduled frontmatter unless the user asked to redirect the fleet queue.
2. Commit and push to `origin/main`.
