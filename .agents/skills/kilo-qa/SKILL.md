---
name: kilo-qa
description: >-
  Executes Pass 5 QA & Build Quality audit and fixes for a single KiloApps application: F5/F9 state
  persistence, first-run tutorial flags, lifecycle cleanup, dual Win32 C parity, and Mature quarantine
  via 1-command verification/handoff.
---

# KiloApps QA & Build Quality Skill (`kilo-qa` — Pass 5)

Executes Pass 5 QA, state persistence verification, and dual Win32 C build parity on **exactly ONE application per turn**.

## ⚡ Flash Execution & Tool-Budget Protocol (CRITICAL)
- **Tool Call Budget (<= 32 tool calls)** & **Commit Checkpoint (~16 tool calls)**:
  - Inspect `KiloOS/public/apps/k<name>.html` and `<AppName>/main.c` (if present), fix any state persistence (`F5`/`F9`), tutorial flag, lifecycle leak, or Win32 C parity gaps, then run 1-command verification!
- **1-Command Verification (with Native C Build & Headless Visual/Gameplay Probe)**:
  ```powershell
  python scripts/verify_turn.py --app <TargetApp> --build-native
  ```
  - `verify_turn.py` runs `scripts/visual_probe.py --compare-head` and writes `docs/gallery/review/<targetapp>/sheet.png` (`boot | ready | after-input | diff heatmap`). Call `view_file` on `sheet.png` if any visual/layout warnings are reported.
- **1-Command Queue Handoff & Mature Quarantine**:
  - When Pass 5 QA passes cleanly and the app is complete, **promote it to Mature Applications** so the fleet continuously graduates finished apps:
  ```powershell
  python scripts/advance_turn.py --agent kilo-qa --app "<TargetApp>" `
    --title "Pass 5 QA, Dual Parity & Mature Quarantine" `
    --bullet "Status: PASS ✅ (<copy summary from verify_turn.py>)." `
    --bullet "State & Tutorial: Verified F5/F9 quicksave/quickload, k<name>_tutorialSeen flag, and Escape modal dismissals." `
    --bullet "Dual Parity & Lifecycle: Verified Web HTML + Win32 C parity and visibilitychange timer cleanup." `
    --quarantine-mature "<Concise 1-line summary of the app's complete feature set, F5/F9 state, and dual Win32 C parity>" `
    --category "<Category e.g. Games / RPG or System / Security>"
  ```
- **Commit, Push & STOP**:
  ```powershell
  git add -A
  git commit -m "fix(qa): Pass 5 audit and fixes for <TargetApp>"
  git push origin main
  ```

---

## Pass 5 QA Checklist
1. **Complete State Persistence (`F5` / `F9`)**: Captures all scores, inventory, settings, active tabs/modes, and progress in both `localStorage` (wrapped in `try/catch`) and Win32 C `.dat` binary save file.
2. **First-Run Tutorial Integrity**: Fires once on fresh session (`k<name>_tutorialSeen`), never on restored save, skippable via `Escape`.
3. **Lifecycle & Resource Hygiene**: Long-running `requestAnimationFrame` / timer loops pause on `document.hidden` (`visibilitychange`).
4. **Dual Win32 Native C Parity**: `<AppName>/main.c` compiles cleanly via `build.bat` with zero errors and matches Web features/keybinds (`< 999 KB`).
