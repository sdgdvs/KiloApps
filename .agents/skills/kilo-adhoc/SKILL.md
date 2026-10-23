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

## 2. Design & Platform Constraints
- **Strict Size Ceiling**: Every `.html` and `.exe` file must remain strictly **< 999 KB** (999,000 bytes).
- **Phase VI Cross-App VFS & IPC Standard**: Support KiloOS `VFS_READ` / `VFS_WRITE` / `OS_LAUNCH_APP` postMessage integration where appropriate with graceful standalone fallback.
- **Anti-Trademark / Parody Rule**: Must pass `python scripts/security_lint.py`.
- **ARG Mystery Preservation (TINAG)**: Never use `(ARG)` labels or leak `ECHO-1999-ARCHITECT`.
- **Multiplayer Autostart Prohibition**: Never autostart into matchmaking on load; require explicit user Connect click and include 25s solo AI fallback (`retro_multiplayer.js`).

## 3. One-Command Verification Pipeline (MANDATORY)
Run the unified verification script:
```powershell
python scripts/verify_turn.py --app <TargetApp> --build-native
```
Or if modifying fleet-wide scripts/shell, run `npm run build` inside `KiloOS/` and `python scripts/security_lint.py`.

## 4. Logging & Deployment
1. Prepend a terse log entry (≤ 6 lines) under `## Recent Execution Logs` in `next_work.md` (and run `python scripts/rotate_logs.py` so old entries archive at 0 token cost). Preserve scheduled frontmatter unless the user asked to redirect the fleet queue.
2. Commit and push to `origin/main`.
