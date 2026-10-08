---
name: kilo-adhoc
description: >-
  Executes user-directed, ad-hoc feature changes, mechanics balancing, or bug fixes across KiloApps.
  Use this skill whenever making targeted, ad-hoc modifications to an application to enforce dual-target
  parity (Web HTML + Win32 C), retro 1999 constraints, clean builds (<999 KB), security lint,
  and seamless git commit/push without disrupting automated fleet schedules.
---

# KiloApps Ad-Hoc Modification Skill (`kilo-adhoc`)

Use this skill when responding to human user requests for specific features, rebalancing, or fixes on a KiloApps application. It ensures ad-hoc tasks maintain full workspace compliance, dual-client parity, and automated CI/CD readiness.

---

## 1. Pre-Flight & Scope Discovery

1. **Pull Latest Changes:** Always run `git pull --rebase` before touching any files to avoid conflicts with automated fleet runs.
2. **Identify Dual Parity Targets:**
   - **Web Client:** `KiloOS/public/apps/k<name>.html`
   - **Native Win32 Client:** `<AppName>/main.c` (alongside `<AppName>/build.bat` and output `<AppName>.exe`).
   - If an application has both Web and Native C versions, **both must be modified simultaneously** to maintain feature, mathematical, price, and keybind parity.

---

## 2. Design & Platform Constraints

- **Strict Size Ceiling:** Every `.html` and `.exe` file must remain strictly **< 999 KB** (999,000 bytes).
- **Input & Keybind Hygiene:**
  - Standard navigation/movement keys (`W`, `A`, `S`, `D` or arrow keys) must never be hijacked by stationary modal dialogs or docking hotkeys.
  - Dedicated interaction keys (e.g. `[O]` for spaceport/docking, `[ESC]` for dismissal, `[SPACE]` for primary action).
- **Anti-Trademark / In-Universe Parody Rule:**
  - Real-world commercial titles, trademarks, and real scene cracking groups are strictly forbidden. Use in-universe parodies (e.g. *Surreal Tournament*, *VoidCraft*, *RAZOR 1999*, *FLARELIGHT*). Must pass `scripts/security_lint.py`.
- **ARG Mystery Preservation (TINAG Standard):**
  - Never label user-facing features or copy with `(ARG)`, `ARG Lore`, or `ARG Guidance`.
  - Never leak the master passkey (`ECHO-1999-ARCHITECT`) or reveal the autonomous fleet meta-twist before endgame App #100 (`KMatrix`).
- **Multiplayer Autostart Prohibition:**
  - If adding or modifying multiplayer features, never autostart into matchmaking on load. Games must boot to local/offline play or a dedicated start screen requiring an explicit "Connect" click.
- **State Persistence (F5/F9):**
  - If adding new inventory, stats, weapons, or flags, ensure they are saved and loaded in both `localStorage` (Web) and the binary `SaveFileData` / `.dat` file (C client).

---

## 3. Implementation Workflow

1. **Implement in Web Client:** Update `KiloOS/public/apps/k<name>.html`.
2. **Implement in Native Client:** Mirror changes in `<AppName>/main.c`. Keep Win32 GDI rendering and simulation logic identical.
3. **Avoid Unrequested Bloat:** Adhere to clean retro 1999 aesthetics. Do not invent arbitrary particle engines, screen shake, or distracting glints.

---

## 4. Verification Pipeline (MANDATORY)

Before committing, run all verification steps:

1. **Native Compilation (if native C target exists):**
   ```powershell
   cmd.exe /c build.bat   # Run from inside <AppName>/
   ```
   Verify that `<AppName>.exe` compiles cleanly and is copied to `KiloOS/public/exe/<AppName>.exe`.
2. **Web Build Verification:**
   ```powershell
   npm run build          # Run from inside KiloOS/
   ```
   Must succeed with zero Vite build errors.
3. **File Size Check:**
   ```powershell
   Get-Item <AppName>/<AppName>.exe, KiloOS/public/apps/k<name>.html | Select-Object FullName, Length
   ```
   Verify each file is `< 999 KB`.
4. **Security & Anti-Trademark Linter:**
   ```powershell
   python scripts/security_lint.py
   ```
   Must exit with code 0 (`ALL SECURITY GATES PASSED`).

---

## 5. Fleet Hygiene, Logging & Deployment

1. **Terse Execution Log & Zero-Discard Rotation:**
   - In `next_work.md` under `## Recent Execution Logs`, prepend a terse entry (≤8 lines of bullet points):
     ```markdown
     - **YYYY-MM-DDTHH:MM:SS-07:00 — kilo-adhoc: <AppName> (<Feature/Fix Summary>)**
       - Status: PASS ✅ (0 regressions, clean builds, <webSize> KB web / <nativeSize> KB native < 999 KB ceiling).
       - <Key change 1>: <Short description of what changed>.
       - <Key change 2>: <Short description of what changed>.
       - Verification: MSVC clean (<exeName> <size> KB); Vite clean; security_lint 100% PASS.
     ```
   - **0-Token Zero-Discard Archival:** You do NOT need to manually delete or drop aging logs. The fleet workflow runs `scripts/rotate_logs.py` automatically (in pre-flight and post-turn orchestration at 0 token cost), moving entries older than 5 turns (or configured limit) to `archive/fleet_execution_archive.md` with zero data loss. You can also run `python scripts/rotate_logs.py` directly.
   - **Do NOT disrupt the active scheduled frontmatter** (`current_agent`, `current_targets`) in `next_work.md` so the automated fleet schedule continues uninterrupted.
2. **Commit & Push:**
   ```powershell
   git add <modified files>
   git commit -m "feat(<app>): <terse summary of adhoc changes>"
   git push origin main
   ```
   If rejected due to remote updates, run `git pull --rebase` and push again.
