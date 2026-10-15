# KiloApps Workspace Rules

## 1. Environment & Tooling
- `git`, `node`, `npm`, `rg` (ripgrep), `fd`, and `uv` are available.
- **Search & Execution Speed:** Always use `rg` for searching file contents, `fd` for finding files, and `uv` for Python package management.

## 2. Multi-Agent Coordination & Turn Lifecycle
- **Always `git pull` first:** Pull before reading or modifying files to avoid conflicts with other agents or machines.
- **Queue & State (`next_work.md`):** `next_work.md` is the single source of truth for agent rotation and task handoffs. Each agent updates `next_work.md` (frontmatter handoff + terse log entry ≤6 lines), commits, and pushes upon turn completion.
- **Minimal Shared-File Edits:** When editing `KiloOS/src/App.jsx` (e.g. adding to `APPS` array), make minimal surgical changes. Pull immediately before editing, commit and push immediately after.

## 3. Commit & Deploy Protocol
- **Commit & Push:** Commit and push all changes at the end of each turn so CI/CD triggers on `main`.
- **CI/CD & Firebase:** Every push to `main` builds KiloOS and deploys to Firebase Hosting (`kiloapps.web.app`).
- **Version Bumping:** When updating KiloOS features, bump the patch version in `KiloOS/package.json` AND update `MICROS_VERSION` in `KiloOS/src/App.jsx`.

## 4. Size Limits & Testing
- **999 KB Limit:** No individual app may exceed 999 KB (native `.exe` or web `.html`). Verify with `python scripts/check_sizes.py`.
- **Pre-Commit Verification:**
  - Web apps: verify HTML renders cleanly without console errors.
  - KiloOS shell: run `npm run build` inside `KiloOS/` before committing changes to `App.jsx` or `index.css`.
  - Native apps: run `build.bat` to confirm clean C/ASM compilation.

## 5. Token Conservation & Logging Rules
- **Run log entries:** ≤6 lines of terse bullet points. No paragraphs.
- **Skip-turn entries:** exactly 1 line (`⏭️ Skip — [reason in ≤15 words]`).
- **Never restate code:** Log WHAT changed + test results, not implementation details or parameter lists.
- **Maturity restraint:** Apps with 5+ passes require no gratuitous churn. If an app is complete and lacks a specific directive, skipping the turn cleanly is expected.

## 6. Subagent Delegation Mandate (NO OPUS SUBAGENTS)
- Whenever invoking `invoke_subagent`, you MUST explicitly set `"Model": "flash"` (or fallback `"Model": "sonnet"` if flash encounters a 503 capacity limit).
- **NEVER use `"Model": "inherit"`** or omit `Model`. High-tier models (Claude Opus/Pro) must never spawn subagents for routine audits, fixes, or summaries.

## 7. On-Demand Rules Index
Read on-demand rules in `.agents/rules/` only when relevant:
- `architecture.md`: KiloOS React state isolation, window management, and iframe postMessage boundaries.
- `quality-standards.md`: JSDoc explicit typing, `safeGet`/`safeSet` storage safety, and `safeFetch` error handling.
- `arg-lore.md`: Alternate reality 1999 lore, anti-trademark parody replacements, passkey secrecy, and multiplayer gating.
