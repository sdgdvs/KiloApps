---
name: kilo-tester
description: >-
  Executes an interactive UI element audit, VFS/IPC check, and in-line repairs for a single KiloApps
  web application. Uses 1-command verification (verify_turn.py) and 1-command handoff (advance_turn.py).
---

# KiloApps App Tester Skill (`kilo-tester`)

Executes a self-contained interactive UI audit and in-line repair on **exactly ONE application per turn**.

## ⚡ Flash Execution & Tool-Budget Protocol (CRITICAL)
- **Tool Call Budget (<= 32 tool calls)** & **Commit Checkpoint (~16 tool calls)**:
  - Run `uv run scripts/visual_probe.py --app <TargetApp>` first (renders at real KiloOS window size over local HTTP, tests trusted-input gameplay & layout clipping, and writes `docs/gallery/review/<targetapp>/sheet.png`), call `view_file` on `docs/gallery/review/<targetapp>/sheet.png`, inspect `KiloOS/public/apps/k<name>.html` (and `<AppName>/main.c` if present), fix any broken handlers/modals/clipping/hotkeys inline, and wrap up by ~16 tool calls!
- **1-Command Verification (includes before/after `git HEAD` visual probe)**:
  ```powershell
  python scripts/verify_turn.py --app <TargetApp>
  ```
- **1-Command Queue Handoff & Logging**:
  ```powershell
  python scripts/advance_turn.py --agent kilo-tester --app "<TargetApp>" `
    --title "Interactive UI Audit & Inline Repairs" `
    --bullet "Status: PASS ✅ (<copy summary from verify_turn.py>)." `
    --bullet "UI & Hotkey Audit: <buttons, modals, Escape dismissal, F1/F5/F9, input focus guards verified>." `
    --bullet "Repairs: <summary of inline fixes or 0 defects confirmed>."
  ```
  *(If the app passes with zero remaining defects and is functionally mature across UI, VFS/persistence, and C parity, add `--quarantine-mature "<Mature summary>" --category "<Category>"` to promote it to Mature Applications!)*
- **Commit, Push & STOP**:
  ```powershell
  git add -A
  git commit -m "audit(test): UI audit and fixes for <TargetApp>"
  git push origin main
  ```

---

## Audit & Repair Checklist
1. **Startup & Modal Occlusion (`test_app_startup.py`)**: Zero CSS brace errors, zero console exceptions, startup splash/tutorial modals dismiss cleanly on click/Escape, and `document.elementFromPoint()` reaches the primary workspace/canvas.
2. **Interactive Controls & Input Guards**:
   - Every `<button>`, tab, select, and toolbar control has a valid handler and non-null DOM target.
   - Global keyboard hotkeys (`Space`, `W/A/S/D`, `1-9`, `H`, `M`) MUST ignore events when `document.activeElement` is an `<input>`, `<textarea>`, or `<select>`!
3. **Persistence & KiloOS VFS Safety**:
   - `[F5]` Quicksave and `[F9]` Quickload restore full state without throwing on empty/corrupt `localStorage`.
   - Verify `localStorage` calls are wrapped in `try/catch`.
4. **TINAG & Multiplayer Connect Gate**:
   - Zero `(ARG)` labels or plain-text `ECHO-1999-ARCHITECT` leaks.
   - Multiplayer never autostarts on load; requires explicit button click.
