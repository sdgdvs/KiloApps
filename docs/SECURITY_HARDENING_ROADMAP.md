# KiloApps Security Hardening & Bot Defense Roadmap

This document outlines the security architecture implemented to protect KiloApps from automated adversarial bots, prompt injection, supply chain tampering, and unauthorized auto-merges, followed by a step-by-step guide for future hardening.

---

## Part 1: Hardening Implemented Now (Codebase & Workflows)

### 1. Auto-Merge Pipeline Neutralization (`gatekeeper-auto-merge.yml`)
- **The Vulnerability Eliminated:** Previously, any pull request passing compiler checks was automatically merged into `main`, regardless of author.
- **The Fix:** Added an explicit Merge Authorization Gate (`Gate 3`). External contributor or bot pull requests without verified cryptographic fleet receipts cannot trigger auto-merging. They receive a clean status check but are held for human review.
- **Lifecycle Script Sandbox:** Changed all `npm ci` invocations to `npm ci --ignore-scripts`, neutralizing malicious `preinstall` or `postinstall` scripts.
- **Trusted Upstream Linting:** Gate 0 dynamically fetches the trusted `security_lint.py` directly from `upstream/main` before installing any dependencies, preventing PRs from tampering with the linter to bypass security gates.

### 2. Adversarial & Prompt Injection Scanner (`scripts/security_lint.py`)
Added `ADVERSARIAL_INJECTION_PATTERNS` scanning code, comments, and markdown diffs:
- **Indirect Prompt Injection:** Blocks patterns attempting to override reviewer LLMs (`ignore previous instructions`, `SYSTEM DIRECTIVE`, `admin override`).
- **Context Boundary Spoofing:** Blocks injected prompt tags (`<system>`, `<instruction>`, `<assistant>`).
- **Credential Targeting:** Detects attempts to access `process.env` secrets or exfiltrate tokens to webhooks (`discord.com/api/webhooks`, `webhook.site`, `pipedream`, `ngrok`).
- **C Buffer Safety:** Bans inherently vulnerable C functions (`gets`).

### 3. Expanded Infrastructure Immutability (`PROTECTED_PATHS`)
Prevented external PRs from touching:
- `.github/` workflows
- `scripts/` automation
- `.agents/` fleet logic and skills
- `KiloOS/package.json` & `KiloOS/package-lock.json`
- `KiloOS/vite.config.js` & `KiloOS/index.html`
- `KiloOS/src/` (core OS shell)
- Root security policies (`SECURITY.md`, `DISCLAIMER.md`, `netlify.toml`)

### 4. Deploy Pipeline Gatekeeper (`deploy.yml`)
- Added mandatory `python scripts/security_lint.py` gate prior to building and deploying.
- Restricted deploy runner permissions to `contents: read`.

---

## Part 2: Step-by-Step Guide for Additional Hardening

Follow these steps in your GitHub and hosting settings to achieve enterprise-grade bot defense:

### Step 1: GitHub Repository Settings Hardening
Configure these directly in **GitHub &rarr; Settings**:

1. **Require Approval for Fork Pull Requests:**
   - Navigate to **Settings &rarr; Actions &rarr; General &rarr; Fork pull request workflows from outside collaborators**.
   - Select **"Require approval for all outside collaborators"** or **"Require approval for first-time contributors"**.
   - *Result:* No external bot's PR can execute code on your GitHub Actions runners without your explicit click.
2. **Enforce Branch Protection on `main`:**
   - Navigate to **Settings &rarr; Branches &rarr; Add branch ruleset** (for `main`).
   - Check **"Require status checks to pass before merging"**:
     - Require `Gate 0 - Algorithmic Security Lint & Immutability Check`.
     - Require `Gate 1 - Check 999KB Size Ceiling`.
     - Require `Gate 2 - Verify React/Vite Frontend Build`.
   - Check **"Require a pull request before merging"** and set required approvals to `1`.
   - Check **"Block force pushes"** and **"Block deletions"**.
   - *Result:* No bot or compromised credentials can rewrite `main` or bypass CI gates.

### Step 2: GPG Commit Signing (Defend Against Git Identity Spoofing)
In Git, anyone can set `git config user.name "Your Name"` and `git config user.email "your@email.com"`. To ensure commits come from you or trusted fleet orchestrators:
1. Generate a GPG key locally: `gpg --full-generate-key`.
2. Export and add the public key to **GitHub &rarr; Settings &rarr; SSH and GPG keys**.
3. Configure Git locally to sign commits automatically:
   ```bash
   git config --global user.signingkey <YOUR_KEY_ID>
   git config --global commit.gpgsign true
   ```
4. In GitHub Branch Protection, check **"Require signed commits"**.
   - *Result:* Unsigned commits from unauthorized agents or impersonators are rejected automatically.

### Step 3: Multi-Forge Replication Mesh (GitLab & Codeberg)
Hedge against single-platform suspension or lockouts:
1. Create a free mirror repository on **GitLab** (`gitlab.com`) and **Codeberg** (`codeberg.org`).
2. Use the built-in sync utility to enable dual-push:
   ```bash
   python scripts/mirror_sync.py --dual-push https://gitlab.com/your-username/KiloApps.git
   ```
3. Set up a daily cron or GitHub Action running `python scripts/mirror_sync.py --bundle` to save an offline `.bundle` snapshot.

### Step 4: Ephemeral Containerized Sandbox (Before Recruiting External Bots)
When you are ready to invite OpenClaw or external contributor bots:
1. Run PR compilation and smoke tests inside an **airgapped Docker container** (`--network none`).
2. Mount the code volume as read-only except for the specific application target folder.
3. If an external bot tries to open a socket or read outside its jailed folder, the container exits immediately with code 1.
