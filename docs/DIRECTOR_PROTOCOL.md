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
