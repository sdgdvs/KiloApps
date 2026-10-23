---
name: kilo-creator
description: >-
  Designs and implements new applications, canonical 1999 Virtual Web destinations (kweb://...),
  and deep game worlds for the KiloApps fleet. Enforces Anti-Potemkin depth, KiloOS VFS/IPC
  integration, RFMS online multiplayer, YM2612/SPC700 audio, <999 KB budgets, and 1-command verification/handoff.
---

# KiloApps Creator Skill (`kilo-creator`)

Designs and implements new applications, canonical 1999 Virtual Web destinations (`kweb://...`), or deep expansions on **exactly ONE target per turn**.

## ⚡ Flash Execution & Tool-Budget Protocol (CRITICAL)
- **Tool Call Budget (<= 25 tool calls)** & **Commit Checkpoint (~12 tool calls)**:
  - Plan your edits immediately after reading the target file (or reference template).
  - Batch edits cleanly into 1–3 `edit_file` / `write_file` calls.
  - Once substantive edits are complete (or at ~12 tool calls), wrap up immediately!
- **1-Command Verification (Saves 4+ tool calls)**:
  ```powershell
  python scripts/verify_turn.py --app <TargetAppOrKwebUrl>
  ```
- **1-Command Queue Handoff & Logging (Saves 3+ tool calls)**:
  ```powershell
  python scripts/advance_turn.py --agent kilo-creator --app "<Target>" `
    --title "<Short Feature/Site Summary>" `
    --bullet "Status: PASS ✅ (<copy summary from verify_turn.py>)." `
    --bullet "<Key interactive feature 1 implemented>." `
    --bullet "<Key VFS/audio/linking feature 2 implemented>."
  ```
- **Commit, Push & STOP**:
  ```powershell
  git add -A
  git commit -m "feat(creator): <terse summary for Target>"
  git push origin main
  ```
  If push is rejected, run `git pull --rebase --autostash && git push origin main`. Then **STOP immediately**.
- **Never read** `archive/` or `.agents/receipts/`. Never run repo-wide screenshot or test suites.

---

## 1. Pre-Flight
1. Run `git pull --rebase --autostash`.
2. Check the prompt (or `next_work.md` `### 1.`) for `current_targets.kilo_creator` and its **Mission Spec**.

---

## 2. Phase VI Direction: Canonical 1999 Virtual Web Nodes (`kweb://...`)
When creating or expanding a Virtual 1999 Web destination in `KiloOS/public/web/<slug>.html`:
1. **Anti-Potemkin Interactive Standard**:
   - Every site must be a **genuine, self-contained 1999 Web 1.0 application** (`< 999 KB`, zero external CDN dependencies)—never a static mock page.
   - Include real client-side state (`safeGet`/`safeSet` `localStorage`), interactive calculators/simulators/filters, downloadable artifacts, or playable mini-tools.
2. **KiloOS VFS & Cross-App Bridge**:
   - Allow users to **Save to KiloOS VFS** (`/home/downloads/<filename>`) or **Open in KHex / KPad / KCipher** via `window.parent.postMessage`:
     ```javascript
     function saveToKiloVfs(vfsPath, contentStr) {
       try {
         window.parent.postMessage({ type: 'VFS_WRITE', path: vfsPath, data: contentStr, requestId: 'vfs_' + Date.now() }, '*');
         window.parent.postMessage({ type: 'OS_NOTIFY', title: document.title, message: 'Saved to VFS: ' + vfsPath }, '*');
       } catch (e) {}
     }
     ```
3. **KNet & Webring Integration**:
   - Ensure new `kweb://` clearnet/community nodes are routed in `KiloOS/public/apps/knet.html` (URL resolver map) and linked in `portal.html` / `webring.html`.
   - **Subterranean / Tier 3 Rule**: Darknet/ARG nodes (`kweb://darknet`, `kweb://deep-core`, `kweb://10.19.99.4/classified`, `kweb://echo-subsystem.net`) must NEVER appear on surface bookmark bars; link them only via diegetic hex dumps, NFO checksums, or keygen seeds.

---

## 3. Mandatory Requirements for New OS Applications (`KiloOS/public/apps/k<name>.html`)
1. **Start Splash & First-Run Tutorial**: Title overlay with version, New/Continue, Settings, Help (`[F1]`/`[H]`), and one-time skippable tutorial (`k<name>_tutorialSeen`).
2. **State Persistence & VFS**: `[F5]` Quicksave, `[F9]` Quickload, JSON Export/Import, and KiloOS VFS (`VFS_READ`/`VFS_WRITE`) support.
3. **Hardware Chiptune Audio**: Sega Genesis YM2612 2-Op FM + SNES SPC700 stereo delay (`180ms`, `2200Hz` LP filter) with `[M]` mute toggle and zero external audio files.
4. **Online Multiplayer (RFMS) & No-Autostart Rule**:
   - Use `../assets/js/retro_multiplayer.js` (`soloTimeoutMs: 25000` with `onSoloFallback`).
   - **NEVER autostart into matchmaking on load**; require an explicit user click on "Connect / Multiplayer".
5. **Anti-Trademark & TINAG Mystery Standard**:
   - Use in-universe parodies (*Surreal Tournament*, *Tremor III Arena*, *VoidCraft*, *Machina Ex*, *FLARELIGHT*, *RAZOR 1999*, *SlashNet*, *Cabled*, *The Scallion*, *Trapster*).
   - Never use `(ARG)` labels or leak `ECHO-1999-ARCHITECT` before App #100 (`KMatrix`).
