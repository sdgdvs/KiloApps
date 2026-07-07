# Workspace Rules

## Environment Setup

- **Terminal Environment Path:** When operating in this workspace, the default terminal environment `Path` variable may be truncated or corrupted, rendering standard utilities like `git` and `npm` unrecognizable. Before running any `git`, `npm`, or similar system commands, you MUST prepend the following to your command to restore the PATH: `$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User");`.

## Commit & Deploy Protocol

- **Committing Changes:** On the completion of each agent turn or task, you must automatically commit and push all changes to the GitHub repository using the restored PATH above, so that the remote build servers can pick up the deployment.
- **Verify Deployment:** After pushing, briefly check whether the GitHub Actions workflow passes. The CI/CD pipeline builds KiloOS and deploys to Firebase Hosting on every push to `main`. If the build fails, investigate and fix before moving on.
- **Version Bumping:** When making functional changes to KiloOS, bump the patch version in `KiloOS/package.json`.

## Multi-Agent Coordination (CRITICAL)

Multiple agents operate on this codebase on overlapping schedules. To prevent merge conflicts and data loss:

1. **Always `git pull` first.** Before reading or editing any file, run `git pull` to ensure you have the latest version. Other agents may have pushed changes since your last turn.
2. **Minimize shared-file edits.** The files `KiloOS/src/App.jsx` and `KiloOS/src/index.css` are edited by multiple agents and are high-conflict. When editing these files:
   - Pull immediately before editing.
   - Make surgical, minimal changes — do not rewrite large blocks.
   - Commit and push immediately after editing, before doing other work.
3. **Own your plan file.** Each scheduled agent should only modify its own plan file:
   - `app_polish_plan.md` — Polish/expansion agent only.
   - `app_fix_plan.md` — Bug-fix/maintenance agent only.
   - `new_app_plan.md` — New app development agent only.
   - If you need to check another agent's plan (e.g., to avoid working on the same app), read it but do not edit it.
4. **Check for conflicts after push.** If `git push` fails due to a conflict, run `git pull --rebase`, resolve any conflicts conservatively (prefer the remote version for code you didn't write), then push again.

## Size Constraints

- **No individual kiloApp may exceed 999 kilobytes**, including both native (.exe) and web (.html) versions. The aggregated web platform at `kiloapps.web.app` and complete release `.zip` files are exempt.
- **KiloOS targets:** HTML bundle ≤ 50KB gzipped, EXE ≤ 150KB.

## Testing Expectations

- After modifying any app's HTML file, open it in a browser (if tools permit) to verify it renders correctly.
- After modifying `App.jsx` or `index.css`, run `npm run build` inside `KiloOS/` to verify the build succeeds before committing.
- After modifying a native app's `.c` file, run its `build.bat` to verify compilation.

## ARG / Easter Egg Documentation

- Any hidden behavior, easter eggs, or narrative elements added to the codebase **must** be documented in a dedicated manifest file (e.g., `arg_plan.md` or similar).
- The manifest must list: what was added, which files were modified, how to trigger it, and any side effects (animations, console output, fake errors, etc.).
- Do not add hidden features that could be mistaken for bugs (e.g., random screen glitches, unresponsive UI elements) without clear documentation.
