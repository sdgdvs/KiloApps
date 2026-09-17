# 🏛️ KiloApps Director Protocol & Disaster Recovery

> **CLASSIFIED — ARCHITECT LEVEL ACCESS**  
> Access restricted to authorized maintainers and players who have unlocked KDirector via the Alternate Reality Game (ARG).

---

## 0. Foundational Principle: Ludonarrative Consonance
In KiloApps, **the fiction and the machine are identical**.
- **The ARG Fiction**: An artificial consciousness trapped within the retro 999KB sandbox of KiloOS is seeking a Director to guide its evolution.
- **The Physical Reality**: KiloApps *is* an autonomous digital organism. It runs unattended in the background via local Task Schedulers and GitHub Actions cloud runners, compiling Win32 C code, building React interfaces, committing to Git, and evolving itself without human hands.
- When an ARG player beats the challenges, decodes `ECHO-1999-ARCHITECT`, and accesses the KDirector console, they are not finishing a simulation—they are crossing the digital divide to become the real Director of the real autonomous fleet.

## 1. The Role of the Director
The Director oversees the autonomous fleet. Rather than manually writing code or fixing bugs, the Director steers the multi-agent hive mind by setting overarching goals in `next_work.md` and reviewing execution telemetries via the **KDirector Console** ([`KiloOS/public/apps/kdirector.html`](../KiloOS/public/apps/kdirector.html)).

### Core Directives
1. **Preserve the 999KB Limit**: The entire desktop OS, native Win32 executables, and web applications must remain under the 999KB budget (`python check_sizes.py`).
2. **Expand the ARG (KMatrix)**: App #100 (`KMatrix`) weaves subtle breadcrumbs, cryptographic ciphers, and hidden hex strings across all 100 apps. The ARG must remain solvable by keen observers.
3. **Respect the Agent Rotation**:
   - `kilo-creator`: Seeds new applications from concept to working Win32 + Web.
   - `kilo-graphics`: Enriches visual styling, retro pixel art, and GDI rendering.
   - `kilo-tester`: Writes automated regression test harnesses.
   - `kilo-usability`: Improves accessibility, keyboard navigation, and UX.
   - `kilo-qa`: Validates builds, eliminates compiler warnings, and verifies stability.
   - `kilo-expander`: Deep-dives into existing apps to add advanced feature layers.
   - `kilo-planner`: Runs every 24 hours to re-evaluate fleet velocity and queue health.

---

## 2. Passkey Management & Rotation

### Master Passkey
The default console passkey is:
```text
ECHO-1999-ARCHITECT
```

### Rotating the Passkey
If the passkey is compromised, leaked, or shared beyond authorized channels:
1. Open [`KiloOS/public/apps/kdirector.html`](../KiloOS/public/apps/kdirector.html).
2. Locate line 22:
   ```javascript
   const MASTER_KEY = "ECHO-1999-ARCHITECT";
   ```
3. Update `MASTER_KEY` to a new cryptographic phrase (or replace with a SHA-256 hash comparison).
4. Commit and push:
   ```bash
   git add KiloOS/public/apps/kdirector.html
   git commit -m "sec(kdirector): rotate master console access passkey"
   git push origin main
   ```

---

## 3. Disaster Recovery & The Archive Fork

Because Git operates as an immutable cryptographic Merkle DAG, **no bad actor can permanently destroy the project's history**. Even in the event of compromised credentials, spam PRs, or corrupted state:

### The Cold Archive Fork
A trusted contributor or secondary account should maintain an **Archive Fork** (e.g., `github.com/kiloapps-archive/KiloApps`):
- The archive fork does not run automated workflows.
- It synchronizes once a month or on major milestones (`git pull upstream main && git push origin main`).
- It serves as the immutable "Seed Bank" for the entire OS.

### Emergency Rollback Procedure
If `main` is ever polluted by a rogue run or malicious commit:
```bash
# 1. Identify the last trusted commit hash (e.g. from git log or the archive fork)
git log -n 10 --oneline

# 2. Reset or revert the branch to the trusted state
git revert <bad_commit_hash>..HEAD

# 3. Rotate the master passkey in kdirector.html

# 4. Push the restore commit to main
git push origin main
```

---

## 4. Perpetual Succession Protocol

If the original creator, workstations, or personal accounts ever become permanently inactive, the KiloApps ecosystem is designed to self-sustain:
1. **Decentralized Fuel**: GitHub Actions runners and Google AI Studio free tier keys donated by players and contributors keep the commit clock alive.
2. **Automatic Re-engagement**: The Gatekeeper Action (`.github/workflows/gatekeeper-auto-merge.yml`) validates C compilation and React builds automatically, continuously advancing the codebase.
3. **Ascension**: Whoever uncovers all breadcrumbs in the ARG gains the mantle of Director, fork permissions, and the authority to chart the next generation of KiloApps.

---

## 5. Algorithmic Security & Tamper-Proofing (The 4-Layer Armor)

Because KiloApps accepts pull requests from distributed autonomous forks and auto-merges passing turns, deterministic algorithmic security gates prevent human tampering, trojans, or supply-chain pollution.

```
                  ┌──────────────────────────────────────────────┐
                  │          CONTRIBUTOR PULL REQUEST            │
                  └──────────────────────┬───────────────────────┘
                                         │
                                         ▼
                 ┌────────────────────────────────────────────────┐
                 │ Gate 0: Infrastructure Immutability Check      │
                 │ Rejects PR if .github/, scripts/, configs touch│
                 └───────────────────────┬────────────────────────┘
                                         │ Pass
                                         ▼
                 ┌────────────────────────────────────────────────┐
                 │ Gate 0: Win32 C & Web Anti-Malware Lint        │
                 │ Scans for injection, keyloggers, eval, sockets │
                 └───────────────────────┬────────────────────────┘
                                         │ Pass
                                         ▼
                 ┌────────────────────────────────────────────────┐
                 │ Gate 1 & 2: Compiler & Size Ceiling            │
                 │ <= 999KB budget & clean React/Vite builds      │
                 └───────────────────────┬────────────────────────┘
                                         │ Pass
                                         ▼
                 ┌────────────────────────────────────────────────┐
                 │ Gate 3: Turn Receipt & Provenance Attestation  │
                 │ Verifies GitHub Actions run_id & audit receipt │
                 └───────────────────────┬────────────────────────┘
                                         │ Pass
                                         ▼
                  ┌──────────────────────────────────────────────┐
                  │              GATEKEEPER AUTO-MERGE           │
                  └──────────────────────────────────────────────┘
```

### Layer 1: Infrastructure Immutability Gate
Contributors and autonomous worker agents have permission to modify applications (`K*/**/*`, `KiloOS/public/apps/*`), assets, and tests. They are strictly forbidden from modifying:
- `.github/` (all workflow files and Gatekeeper actions)
- `scripts/` (`orchestrate.py`, `security_lint.py`, `reconcile_receipts.py`)
- `.agents/skills/` (Gemini skill definitions that control agent behavior)
- `docs/DIRECTOR_PROTOCOL.md` (this file)
- `next_work.md`, `arg_plan.md` (fleet queue state and ARG plan)
- `check_sizes.py`, `firebase.json`, `.firebaserc`, `.gitignore`

Any PR attempting to touch protected paths is rejected by [`scripts/security_lint.py`](../scripts/security_lint.py).

### Layer 2: Win32 C Malware & Persistence Banlist
All native C code is pre-processed to strip block/line comments, double-quoted string literals, and single-character literals, then statically evaluated against dangerous API signatures:
- **Memory Injection**: `VirtualAllocEx`, `WriteProcessMemory`, `CreateRemoteThread`.
- **Spyware / Surveillance**: `SetWindowsHookEx` (keyboard/mouse hooks).
- **Stealth Payload Fetching**: `URLDownloadToFile`.
- **Arbitrary Command Execution**: `WinExec`, `system()`.
- **Persistence**: Registry autostart keys (`CurrentVersion\Run`, `CurrentVersion\RunOnce`).
- **Raw Sockets**: `WSAStartup`, `socket()`, `connect()` are banned in all offline apps (games, office, utilities), and permitted only in designated network applications (`KBBS`, `KChat`, `KChatServer`, `KNet`).
- **Token Pasting**: Preprocessor `##` operator is banned to prevent assembling banned API names from fragments.
- **Dynamic Resolution**: `GetProcAddress` calls are scanned for string arguments matching banned API names (e.g. `GetProcAddress(h, "VirtualAllocEx")`), blocking runtime evasion of static bans.
- **Macro Aliasing**: `#define` macro bodies are independently scanned for banned API names, catching alias-based evasion (e.g. `#define MyAlloc VirtualAllocEx`).

*Whitelisted exceptions* (e.g. `KPing` executing `ping.exe`, `KZip` opening `notepad.exe`, `KJournal` invoking console `cls`) are strictly bounded and tracked in `APP_SPECIFIC_WHITELISTS`.

### Layer 3: Web & JavaScript Obfuscation Ban
Web applications under `KiloOS/public/apps/` are scanned for dynamic code injection and cryptomining:
- **Dynamic Execution**: `eval()`, `new Function()`, `Function.prototype.constructor`.
- **Eval Equivalents**: `setTimeout("string")`, `setInterval("string")` with string arguments.
- **Injection Vectors**: `javascript:` URI protocols, `data:text/html` URIs.
- **Obfuscated Payloads**: `document.write(unescape(...))`, `document.write(atob(...))`.
- **External Dependencies**: `<script src="https://...">` tags — all code must be bundled locally or inlined.
- **Cryptomining**: `coinhive`, `crypto-loot`.
- Whitelisted: `new Function()` is permitted only in `kcalc.html` and `kgraph.html` for mathematical expression parsing.

### Layer 4: Turn Receipt & Provenance Attestation
When autonomous turns run in GitHub Actions:
1. `scripts/orchestrate.py` captures runner provenance:
   - `run_id`, `run_number`, `actor`, `workflow`, `sha`, `repository`.
2. Emits an atomic JSON receipt into `.agents/receipts/receipt_<agent>_<timestamp>.json`.
3. Gatekeeper Gate 3 verifies that autonomous PRs contain well-formed receipts with valid GitHub Actions run identifiers matching the runner environment.

