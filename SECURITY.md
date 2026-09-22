# Security Policy & Autonomous Agent Governance

## 1. Reporting Security Vulnerabilities

We take the security of KiloApps seriously. If you discover a vulnerability or security defect within this repository, its applications, or its CI/CD workflows:

- **Please DO NOT open a public GitHub issue.**
- Report vulnerabilities privately through [GitHub Private Vulnerability Reporting](https://github.com/sdgdvs/KiloApps/security/advisories/new) or contact the repository owner directly.
- We will investigate and address legitimate security concerns promptly.

---

## 2. Autonomous Agent & Bot Contributor Governance

KiloApps operates an autonomous multi-agent development fleet. To defend against automated adversarial attacks, supply-chain attacks, and indirect prompt injections, the repository enforces strict security boundaries:

### A. Secret Airgapping & Zero Write Privileges
- External contributor bots and forks run in an airgapped execution context.
- Workflows triggered by external pull requests have **no access to repository secrets** (API keys, deployment tokens, cloud credentials).
- The automatic merge pipeline (`Gatekeeper`) strictly rejects automated merging from untrusted external contributors. External PRs require manual human maintainer review and authorization.

### B. Algorithmic Gatekeeper (`scripts/security_lint.py`)
All code changes—whether submitted by humans or bots—must pass automated algorithmic security linting:
1. **Infrastructure Immutability:** External PRs are strictly forbidden from modifying `.github/`, `scripts/`, `package.json`, root configurations, or core OS kernel files.
2. **Win32 C Safety:** Process injection (`VirtualAllocEx`, `WriteProcessMemory`, `CreateRemoteThread`), system keyboard hooks (`SetWindowsHookEx`), unmonitored payload downloaders (`URLDownloadToFile`), arbitrary shell invocations (`system`, `WinExec`), unsafe buffers (`gets`), and undocumented raw sockets are rejected.
3. **Web Safety:** Dynamic execution (`eval`, `new Function`, string-based `setTimeout`/`setInterval`, `javascript:` URIs) and external script/CDN tags are strictly banned.
4. **Adversarial & Prompt Injection Defense:** Diffs, comments, and documentation are scanned for jailbreak keywords, prompt overrides, system tag spoofing, and credential exfiltration webhooks.
5. **Trademark & ARG Parody Compliance:** Commercial trademarks and copyrighted titles are blocked; in-universe parodies must be used.

---

## 3. Structural Defenses

1. **Zero External Runtime Dependencies:** Applications are written in pure Win32 C and vanilla HTML5/Canvas/Web Audio. No third-party npm packages or external libraries are permitted in applications, eliminating 95% of software supply-chain attack surfaces.
2. **999 Kilobyte Boundary:** Every application executable and web file must remain strictly under 999 KB.
3. **Sandboxed Package Installation:** All CI/CD workflows enforce `npm ci --ignore-scripts` to prevent malicious preinstall or postinstall lifecycle scripts.
