---
name: kilo-expander
description: >-
  Executes deep functional feature expansions across KiloApps: Phase VI KiloOS VFS/IPC mesh
  integration, RFMS online multiplayer retrofits, workstation diagnostic depth, dual Win32 C parity,
  and 1-command verification/handoff.
---

# KiloApps Feature Expander Skill (`kilo-expander`)

Deepens functional utility, cross-app OS integration, and multiplayer capabilities on **exactly ONE application per turn**.

## ⚡ Flash Execution & Tool-Budget Protocol (CRITICAL)
- **Tool Call Budget (<= 32 tool calls)** & **Commit Checkpoint (~16 tool calls)**:
  - Inspect the target file (`KiloOS/public/apps/k<name>.html` and `<AppName>/main.c` if present) and plan a high-craft expansion.
  - Once substantive edits are complete (or at ~16 tool calls), wrap up immediately!
- **1-Command Verification (Saves 4+ tool calls)**:
  ```powershell
  python scripts/verify_turn.py --app <TargetApp>
  ```
- **1-Command Queue Handoff & Logging (Saves 3+ tool calls)**:
  ```powershell
  python scripts/advance_turn.py --agent kilo-expander --app "<TargetApp>" `
    --title "<Short Expansion Title>" `
    --bullet "Status: PASS ✅ (<copy summary from verify_turn.py>)." `
    --bullet "<Domain feature / VFS / RFMS expansion 1>." `
    --bullet "<Dual Win32 C parity / export feature 2>."
  ```
  *(If the app is already 100% feature-complete after 6+ passes, pass `--quarantine-mature "<Summary>" --category "<Category>"` to archive it to Mature).*
- **Commit, Push & STOP**:
  ```powershell
  git add -A
  git commit -m "feat(expand): <terse summary for TargetApp>"
  git push origin main
  ```
  Then **STOP immediately**. Never read `archive/` or `.agents/receipts/`.

---

## 1. Phase VI Primary Directive: KiloOS VFS & Inter-App IPC Mesh
Transform isolated apps into interconnected KiloOS workstations! When expanding any utility, dev, office, media, or game app, wire **KiloOS Virtual Filesystem (`VFS`) & Cross-App Launch (`OS_LAUNCH_APP`)** alongside existing `localStorage` / file download handlers:

```javascript
// Standard KiloOS VFS & Cross-App IPC Bridge (graceful no-op when standalone)
const KiloVFS = {
  write(path, data) {
    try {
      window.parent.postMessage({ type: 'VFS_WRITE', path, data: typeof data === 'string' ? data : JSON.stringify(data, null, 2), requestId: 'w_' + Date.now() }, '*');
      window.parent.postMessage({ type: 'OS_NOTIFY', title: document.title, message: `Saved to VFS: ${path}` }, '*');
    } catch (e) {}
  },
  read(path, cb) {
    const reqId = 'r_' + Date.now() + '_' + Math.random().toString(36).slice(2, 6);
    const handler = (e) => {
      if (e.data && e.data.type === 'VFS_READ_DONE' && e.data.requestId === reqId) {
        window.removeEventListener('message', handler);
        cb(e.data.data, e.data.path);
      }
    };
    window.addEventListener('message', handler);
    try { window.parent.postMessage({ type: 'VFS_READ', path, requestId: reqId }, '*'); } catch (e) {}
  },
  openPicker(cb) {
    const reqId = 'open_' + Date.now();
    const handler = (e) => {
      if (e.data && e.data.type === 'VFS_SHOW_OPEN_DONE' && e.data.requestId === reqId) {
        window.removeEventListener('message', handler);
        if (e.data.path) KiloVFS.read(e.data.path, cb);
      }
    };
    window.addEventListener('message', handler);
    try { window.parent.postMessage({ type: 'VFS_SHOW_OPEN', requestId: reqId }, '*'); } catch (e) {}
  },
  launchApp(targetApp, path = '') {
    try { window.parent.postMessage({ type: 'OS_LAUNCH_APP', targetApp, path }, '*'); } catch (e) {}
  }
};
```
- **Examples of High-Value VFS Workflows**:
  - **Dev & System (`KHex`, `KCipher`, `KSteno`, `KScript`, `KDB`, `KHash`, `KVault`, `KAnomaly`)**: Save/load buffers directly to `/home/work/<file>` in VFS; add a `"Send to KHex"` / `"Decrypt in KCipher"` action button that writes to VFS and calls `KiloVFS.launchApp('khex', path)`.
  - **Office & Media (`KJournal`, `KCalendar`, `KTodo`, `KContacts`, `KBudget`, `KAudio`, `KSynth`, `KImage`, `KColor`)**: Export/import snapshots or patches to `/home/docs/` or `/home/media/` in VFS so `File Browser` (`kexplorer`) and `KPad` can browse them live!
  - **Games (`KTrader`, `KQuest`, `KStarForge`, `KAlchemy`, `KMech`, `KStellar`, etc.)**: Export ship blueprints, cargo ledgers, or replay logs to `/home/saves/<app>_blueprint.json` in VFS!

---

## 2. Seamless Online Multiplayer (RFMS) & No-Autostart Mandate
When expanding games or collaborative tools with online multiplayer:
- Include `<script src="../assets/js/retro_multiplayer.js"></script>`.
- **Mandatory Connect Gate**: NEVER autostart into matchmaking on load. Boot to local/offline play; require an explicit user click on `"Multiplayer / Connect"`.
- **25s Solo AI Fallback**: Configure `soloTimeoutMs: 25000` with `onSoloFallback` so lone players automatically enter an active AI match or simulated guild exchange after 25 seconds.

---

## 3. Dual-Target Parity (Web HTML + Win32 Native C)
- If `<AppName>/main.c` exists, mirror core calculation/simulation logic, keybinds, and `.dat` state persistence in `<AppName>/main.c` and verify compilation via `python scripts/verify_turn.py --app <TargetApp> --build-native`.
