# KiloApps Postmortem & 2.0 Restart Architecture Roadmap

**Date:** November 5, 2026  
**Baseline:** `main` (`MICROS_VERSION 0.4.38`, fleet frozen)

This document captures the architectural root causes identified during the November 2026 KiloApps postmortem and defines the **12 Priority-0 Gemini 4 Argon engineering tickets (`PM-01` through `PM-12`)** queued at the top of `next_work.md` (`### 0.`). When the fleet is unfrozen, `scripts/orchestrate.py` executes all `PM-*` tickets on `gemini-4-argon-high` before resuming standard Flash application turns.

---

## 1. Root Causes Identified in Phase V–VI

1. **`returncode != 0` Circuit-Breaker Blind Spot (`scripts/orchestrate.py`):**
   - `27.4%` (`809 / 2,949`) of recorded turns hit the 10–11m `--print-timeout` (`returncode = 3`).
   - Both Post-Turn Salvage and the 2-Strike Stall Circuit Breaker were nested inside `if process.returncode == 0:`, causing 9x–21x consecutive hourly timeout loops on oversized specs (`KSanctuary`, `KBreakout`) until quota hit `429 RESOURCE_EXHAUSTED`.
   - `git pull --rebase --autostash` bled half-finished timeout edits into subsequent turns, and `--no-visual` salvage fallback committed visually broken UI states (`110` auto-salvage commits).

2. **Mega-Specs & Dual-Target Turn Overload:**
   - `kilo-review` routinely injected 1,000-character specs bundling 10–15 Web `.html` + Win32 `main.c` repairs into one table cell, forcing a single 10m Flash turn to read and patch 10,000+ lines of HTML/JS and 6,000+ lines of Win32 GDI C simultaneously.

3. **Porous Graduation (`22 / 22` Mature Lock Breaches):**
   - `mature_apps_registry.json` (`LOCKED_STABLE`) was never enforced by `orchestrate.py` or `advance_turn.py`, and `advance_turn.py --inject-fix` automatically un-quarantined apps from `## Mature Applications`.

4. **"Arcade-Workstation Maximalism" (Feature Homogenization):**
   - Endless 7-agent rotation across simple utilities and classic board/card games injected out-of-place mechanics (freeze powerups, 512-particle shockwaves, and screen-shake in `KChess`; RPG combat skills in `KSolitaire`/`KFreecell`; unsolicited FM BGM in office utilities).

5. **External Sprite Byte-Gate Loophole & Git Binary Bloat:**
   - `check_sizes.py` and `verify_turn.py` checked only standalone `.html` and `.exe` files, missing `KiloOS/public/assets/sprites/<app>/` (`KDragon` reached `2.53 MB` total, `KQuest` reached `1.85 MB` total).
   - Committing duplicate `.exe` binaries, 2,950+ `.agents/receipts/*.json` files, and an orphaned 41 MB `KiloOS_Server/KiloOS_Server.exe` bloated `.git` to `241.5 MB`.

6. **KiloOS Shell `localStorage` 5 MB Cliff & IPC Listener Churn:**
   - `KiloOS/src/App.jsx` serializes the entire OS VFS into a single `localStorage` key (`kiloos_vfs_state_v2`), risking silent data loss once Phase VI VFS usage crosses ~5 MB.
   - Each `<Window>` re-binds `window.addEventListener('message', ...)` on every 1Hz `OS_TELEMETRY_BEAT` and posts wildcard `'*'` origins.

---

## 2. Priority-0 Argon Execution Order (`### 0.` in `next_work.md`)

| Ticket ID | Scope | Architectural Deliverable |
|---|---|---|
| `PM-01-ORCHESTRATOR-CIRCUIT-BREAKER` | `scripts/orchestrate.py` | Move stall circuit breaker outside `returncode == 0`; parse `AGY_ERROR` (`429` cooldown vs timeout stash+rotate); remove `--no-visual` salvage bypass; replace `--autostash` with clean pre-pull dirty-tree parking. |
| `PM-02-HARNESS-GUARD-MERGE` | `scripts/harness_guard.py`, `scripts/regression_sweep.py`, `scripts/verify_turn.py` | Port `harness_guard.py` and `regression_sweep.py` from `harness-hardening` design: cryptographic `.agents/verify/<slug>.json` PASS tokens, `Agent-Turn:` trailers, and harness path protection. |
| `PM-03-ATOMIC-TICKETS-WEB-C-SPLIT` | `scripts/advance_turn.py`, `scripts/orchestrate.py`, `next_work.md` | Enforce atomic single-focus specs (`<= 350` chars, max 2–3 cohesive items), split Web `.html` turns from Win32 `main.c` parity turns, and decompose existing 1,000-char `[ARGON REVIEW]` mega-rows in `next_work.md`. |
| `PM-04-IMMUTABLE-MATURE-LOCK` | `mature_apps_registry.json`, `scripts/orchestrate.py`, `scripts/advance_turn.py` | Unify `mature_apps_registry.json` and `## Mature Applications` into an immutable lock enforced across `orchestrate.py`, `advance_turn.py`, and `kilo-review` (unlockable only via explicit human CLI `--unlock <App>`). |
| `PM-05-TOTAL-FOOTPRINT-BYTE-GATE` | `check_sizes.py`, `scripts/verify_turn.py`, `KiloOS/public/assets/sprites/` | Enforce canonical `< 999 KB` (`999 * 1024` B) total cartridge footprint (`app.html + sprites/<app>/** + exclusive assets`) and optimize `KDragon` and `KQuest` sprite packs under 999 KB total. |
| `PM-06-INDEXEDDB-VFS-UPGRADE` | `KiloOS/src/App.jsx`, `KiloOS/src/vfsStore.js` | Migrate `kiloos_vfs_state_v2` from 5 MB `localStorage` to asynchronous IndexedDB with seamless legacy migration, quota toast telemetry, and zero blocking `alert()`/`prompt()` dialogs in `App.jsx`. |
| `PM-07-IPC-ROUTER-AND-KILO-SDK` | `KiloOS/src/App.jsx`, `KiloOS/public/sdk/kilo-sdk.js` | Centralize KiloOS `postMessage` handling into one `useRef`-backed router with strict `window.location.origin` targetOrigin (eliminating 1Hz listener churn and `'*'`) and ship `/sdk/kilo-sdk.js` (`< 8 KB`). |
| `PM-08-QUEUE-SHARDING-AND-TELEMETRY-DIET` | `scripts/orchestrate.py`, `scripts/advance_turn.py`, `.agents/` | Stop committing per-turn `.agents/receipts/receipt_*.json` and heartbeat churn to `main`; compact historical receipts into a rolled-up ledger and gitignore ephemeral local telemetry. |
| `PM-09-BINARY-GIT-DIET-AND-SERVER-CLEANUP` | `KiloOS_Server/`, `.gitignore`, `scripts/verify_turn.py` | Remove orphaned 41 MB `KiloOS_Server/KiloOS_Server.exe` and stale duplicate `KiloOS_Server/public/apps/*.html` copies, deduplicate root `<App>/<App>.exe` binaries (`KiloOS/public/exe/` as single binary artifact output), and document release packaging. |
| `PM-10-CI-CONTRIBUTOR-SECURITY-ALIGNMENT` | `.github/workflows/gatekeeper-auto-merge.yml`, `scripts/security_lint.py` | Validate PR-only diff receipts/files (`origin/main...HEAD`) in `security_lint.py` and `gatekeeper-auto-merge.yml` while preserving strict read-only manual-merge enforcement. |
| `PM-11-TWO-TIER-CURATION-BOARD-CARD` | `AGENTS.md`, `KChess`, `KSolitaire`, `KFreecell` | Codify Two-Tier Fleet Architecture in `AGENTS.md` (Tier 1 crisp `<180 KB` classics vs Tier 2 flagship worlds) and strip out-of-place powerups/shockwaves/combat skills from `KChess`, `KSolitaire`, and `KFreecell` (Web + Win32 C). |
| `PM-12-TWO-TIER-CURATION-UTILITIES` | `KChart`, `KHabit`, `mature_apps_registry.json` | Strip unsolicited BGM/arcade bloat from Tier-1 productivity utilities (`KChart`, `KHabit`), verify startup/visuals, and lock all graduated Tier-1 classics in `mature_apps_registry.json`. |
