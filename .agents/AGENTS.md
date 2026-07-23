# Workspace Rules

## Environment Setup

- **`git`, `node`, `npm`, `rg` (ripgrep), `fd`, and `uv` are available**.
- **Agent Efficiency Tools (CRITICAL):** When operating in this workspace, you MUST use `rg` (ripgrep) for searching file contents (it is magnitudes faster than grep/findstr). You MUST use `fd` for finding files (faster than dir/ls). You MUST use `uv` for any Python package management.

## Commit & Deploy Protocol

- **Committing Changes:** On the completion of each agent turn or task, you must automatically commit and push all changes to the GitHub repository so that the remote build servers can pick up the deployment.
- **Verify Deployment:** After pushing, briefly check whether the GitHub Actions workflow passes. The CI/CD pipeline builds KiloOS and deploys to Firebase Hosting on every push to `main`. If the build fails, investigate and fix before moving on.
- **Version Bumping:** When making functional changes to KiloOS, bump the patch version in `KiloOS/package.json`.

## Multi-Agent Coordination (CRITICAL)

Multiple agents operate on this codebase on overlapping schedules, potentially from different computers. To prevent merge conflicts and data loss:

1. **Always `git pull` first.** Before reading or editing any file, run `git pull` to ensure you have the latest version. Other agents may have pushed changes since your last turn.
2. **Minimize shared-file edits.** The files `KiloOS/src/App.jsx` and `KiloOS/src/index.css` are owned by the Shell & UX agent. When other agents need to edit App.jsx (e.g., to register a new app):
   - Pull immediately before editing.
   - Make surgical, minimal changes — ONLY add entries to the APPS array.
   - Commit and push immediately after editing, before doing other work.
3. **Own your plan file.** Each agent should only modify its own plan file:
   - `app_work_plan.md` — App Builder agent only.
   - `app_fix_plan.md` — Quality & Build agent only.
   - `kiloos_ux_plan.md` — Shell & UX agent only.
   - `game_content_plan.md` — Game Content agent only.
   - `new_app_plan.md` — App Creator & Deep Expander agent only.
   - If you need to check another agent's plan (e.g., to avoid working on the same app), read it but do not edit it.
4. **Check for conflicts after push.** If `git push` fails due to a conflict, run `git pull --rebase`, resolve any conflicts conservatively (prefer the remote version for code you didn't write), then push again.

## Size Constraints

- **No individual kiloApp may exceed 999 kilobytes**, including both native (.exe) and web (.html) versions. The aggregated web platform at `kiloapps.web.app` and complete release `.zip` files are exempt.

## Testing Expectations

- After modifying any app's HTML file, open it in a browser (if tools permit) to verify it renders correctly.
- After modifying `App.jsx` or `index.css`, run `npm run build` inside `KiloOS/` to verify the build succeeds before committing.
- After modifying a native app's `.c` file, run its `build.bat` to verify compilation.

## Logging Discipline

- Keep plan files concise. A few lines per completed item is sufficient.
- Do NOT duplicate file contents into log files.
- Do NOT create growing log files that append data every turn. Track status, not history.
