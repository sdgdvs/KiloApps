# Workspace Rules

## Environment Setup

- **`git`, `node`, `npm`, `rg` (ripgrep), `fd`, and `uv` are available**.
- **Agent Efficiency Tools (CRITICAL):** When operating in this workspace, you MUST use `rg` (ripgrep) for searching file contents (it is magnitudes faster than grep/findstr). You MUST use `fd` for finding files (faster than dir/ls). You MUST use `uv` for any Python package management.

## Commit & Deploy Protocol

- **Committing Changes:** On the completion of each agent turn or task, you must automatically commit and push all changes to the GitHub repository so that the remote build servers can pick up the deployment.
- **Verify Deployment:** After pushing, briefly check whether the GitHub Actions workflow passes. The CI/CD pipeline builds KiloOS and deploys to Firebase Hosting on every push to `main`. If the build fails, investigate and fix before moving on.
- **Version Bumping:** When making functional changes or patchnote/version updates to KiloOS, bump the patch version in `KiloOS/package.json` AND update `MICROS_VERSION` in `KiloOS/src/App.jsx` so the opening screen and KiloApps web UX display the updated version.

## Multi-Agent Coordination (CRITICAL)

Multiple agents operate on this codebase on overlapping schedules, potentially from different computers. To prevent merge conflicts and data loss:

1. **Always `git pull` first.** Before reading or editing any file, run `git pull` to ensure you have the latest version. Other agents may have pushed changes since your last turn.
2. **Minimize shared-file edits.** The files `KiloOS/src/App.jsx` and `KiloOS/src/index.css` are owned by the Usability agent. When other agents need to edit App.jsx (e.g., to register a new app):
   - Pull immediately before editing.
   - Make surgical, minimal changes — ONLY add entries to the APPS array.
   - Commit and push immediately after editing, before doing other work.
3. **Unified Work State & Gemini Skills (`next_work.md`).**
   - All active agent roles are packaged as self-contained Gemini Skills in `.agents/skills/` (`kilo-qa`, `kilo-tester`, `kilo-planner`, `kilo-vision-audit`).
   - Active queue state, current targets, and handoffs are centralized in `next_work.md`.
   - Each agent updates `next_work.md` (frontmatter handoff + terse log entry ≤8 lines), commits, and pushes upon turn completion.
   - Historical planning files are archived in `archive/legacy_plans/`.
4. **Check for conflicts after push.** If `git push` fails due to a conflict, run `git pull --rebase`, resolve any conflicts conservatively (prefer the remote version for code you didn't write), then push again.

## Size Constraints

- **No individual kiloApp may exceed 999 kilobytes**, including both native (.exe) and web (.html) versions. The aggregated web platform at `kiloapps.web.app` and complete release `.zip` files are exempt.

## Testing Expectations

- After modifying any app's HTML file, open it in a browser (if tools permit) to verify it renders correctly.
- After modifying `App.jsx` or `index.css`, run `npm run build` inside `KiloOS/` to verify the build succeeds before committing.
- After modifying a native app's `.c` file, run its `build.bat` to verify compilation.

## Token Conservation & Logging Rules (CRITICAL)

You are operating in a token-constrained multi-agent environment. Every line you write to shared .md files (logs, trackers, docs) is read by multiple agents on every run, multiplying its token cost. Follow these rules without exception:

### LOGGING RULES (for any shared log file)
- **Run log entries:** ≤8 lines of terse bullet points. No paragraphs.
- **Skip-turn entries:** exactly 1 line. Format: `⏭️ Skip — [reason in ≤15 words]`
- **Never restate implementation details that exist in the code.** Log WHAT changed + results, not HOW.
- **Never list parameter names, field names, or variable values** unless reporting a failure.
- **Completed work needs no elaboration:** `✅ Done (N/N tests pass)` is sufficient.

### DOC EDITING RULES
- **Surgical edits only.** Touch only the specific cells/lines that changed.
- **Table cell notes:** ≤100 characters. Use file names, not full descriptions.
- **Never duplicate information across files.** One source of truth per fact.
- **Don't rewrite surrounding text** when updating a single value.

### CONTEXT HYGIENE
- **Only read files relevant to your current task.** Skip docs you won't use this run.
- **Don't log diagnostic details** (exact timing, full traces, param values) unless a failure occurred.
- **When a phase, section, or batch of logs is completed, it should be archived** — not kept in the active file.

### ARCHIVAL PROTOCOL
- **Completed phases/milestones:** replace with a 2-line stub pointing to the archive file.
- **Historical logs older than the last ~80 lines:** move to an `archive/` directory.
- **Bulky reference sections** (changelogs, old blockers, ownership tables): archive when >500 bytes.
- **Run `python scripts/compact_all.py` on a daily cron** to enforce caps automatically.

### FORMAT EXAMPLES

**Good run log:**
```markdown
### Agent Run Log — 2026-09-15T10:00 (Phase 17)
- **Status:** 🟢 Completed (`Task Name`)
- Implemented `FooSystem.cs` (feature A, feature B). Wired into `Bar.cs`.
- Tests: 8/8 pass in `FooSystemTest.cs`. Build clean.
```

**Good skip log:**
```markdown
### Agent Run Log — 2026-09-15T10:00 (Phase 17)
- **Status:** ⏭️ Skip — No tasks assigned in active phase
```

**Bad (NEVER do this):**
```markdown
### Agent Run Log — 2026-09-15T10:00 (Phase 17)
- **Status:** 🟢 Completed Deliverable (`Visual Transition & Shader Polish`)
- **Spatial Shaders Polish (`assets/shaders/`):**
  1. `data_waves.gdshader`: Implemented exact analytical partial derivatives (`dw1_dx`, `dw2_dx`, `dw1_dz`, `dw2_dz`) for smooth surface normals, eliminating visual distortion artifacts. Added parameterized `fresnel_power`, `fresnel_factor`, and `transparency_falloff` for silky surface alpha transitions.
  2. `chronos_refraction.gdshader`: Replaced harsh aliased `step()` clock notches with smooth parameterized `dial_feather` smoothsteps, anti-aliased concentric ticks...
[This burned ~800 tokens for information already in the code]
```

## App Maturity, Restraint & Turn Skipping Protocol (CRITICAL)

- **Stop Unnecessary Visual Flourish and Feature Bloat:** Agents must NOT invent random, unrequested, poorly-thought-out features, animations, or visual flourishes (such as blinking HUD reticles, specular glints, particle engines, screen shake, or obtrusive first-person weapon graphics) just to have something to do or commit. The lightweight, bloat-free philosophy of 1999 applies to visuals and UI clarity as well as code size.
- **Turn Skipping for Mature Apps (6+ Passes):** For apps or games that have already been through 6+ passes/loops (e.g. Loop 6+, Pass 6+), unless an agent has a specific, clear directive from the director or user to add or change something:
  *`turn skipped because this app is complete and we don't have new ideas here`* is completely fine and expected!
- **Hard-Lock into "Requires Human Review" Queue (5+ Passes):** Any app that has completed at least five passes (including automated headless validation, zero exceptions, and 60 FPS pacing benchmarks) is immediately locked into `docs/human_review_queue.md`. Worker agents are strictly forbidden from altering mechanics, layout, or visual styling on locked apps.
- **Visual Review Repository:** All app screenshots and performance pacing stats are compiled into `docs/gallery/index.html` and `docs/gallery/screenshots/` where the human director can visually audit gameplay and launch live apps directly.
- **Director Workflow:** The director can add new ideas or directions in subsequent review cycles, and the agent can implement those directives on the next turn. When there is no active directive for a mature app, do NOT invent arbitrary low-quality additions — simply log the skip concisely, rotate the app in the queue, and finish the turn cleanly.

## Subagent Delegation & Model Selection Protocol (CRITICAL - NO OPUS SUBAGENTS)

- **Strict Model Enforcement:** Whenever calling `invoke_subagent`, agents MUST explicitly set `"Model": "flash"` (or `"Model": "sonnet"` as fallback if `flash` encounters capacity/API errors).
- **Prohibition on Inherit / Opus Subagents:** NEVER use `"Model": "inherit"` or omit the `Model` parameter (which defaults to `inherit`). When high-tier director models like Claude Opus spawn subagents with `inherit`, it creates Claude Opus subagents that rapidly deplete token budgets, exhaust rate limits, and burn costly compute on routine tasks, summarizations, file inspections, and audits.
- **Prepackaged Subagents (`self` and `research`):**
  - Even when using the prepackaged `"self"` or `"research"` subagent type, ALWAYS pass `"Model": "flash"`. Do NOT rely on the default inheritance behavior.
  - If `flash` fails due to 503 capacity limits or outages, retry immediately with `"Model": "sonnet"`. Under NO circumstances should Opus or Pro subagents be spawned for routine tasks.
- **Example Subagent Call:**
  ```json
  {
    "Subagents": [
      {
        "TypeName": "self",
        "Role": "App Auditor",
        "Prompt": "...",
        "Model": "flash"
      }
    ]
  }
  ```
  *Fallback on flash error:*
  ```json
  {
    "Subagents": [
      {
        "TypeName": "self",
        "Role": "App Auditor",
        "Prompt": "...",
        "Model": "sonnet"
      }
    ]
  }
  ```
- **Task Scoping:** Routine coding, testing, file auditing, searching, report formatting, and summarization must ALWAYS run on `flash` (or `sonnet`). Claude Opus is strictly reserved for parent director-level reviews and high-level architectural decisions.

## Alternate Reality Fictionalization Mandate (Anti-Trademark / Anti-Infringement Rule)

- **Fictionalized Lore & Parody Standard (CRITICAL):** KiloApps is an Alternate Reality Game (ARG) set in an alternate 1999 universe. Real-life commercial trademarks, copyrighted video game titles, software products, corporate brands, and real-world demoscene cracking groups MUST NOT appear as direct names in code, virtual websites (`/web/`), or application feeds (`krss.html`).
- **Parody Replacement Rule:** All cultural touchstones must be replaced with fictionalized in-universe parodies:
  - *Games:* *Quake III Arena* &rarr; *Tremor III Arena*; *Unreal Tournament* &rarr; *Surreal Tournament*; *Half-Life* &rarr; *Half-Cycle*; *StarCraft* &rarr; *VoidCraft*; *Deus Ex* &rarr; *Machina Ex*; *System Shock 2* &rarr; *System Glitch 2*.
  - *Scene Groups:* *Fairlight* &rarr; *FLARELIGHT*; *Razor 1911* &rarr; *RAZOR 1999*; *Skid Row* &rarr; *SKID VECTOR*; *Paradox* &rarr; *PARALAX*.
  - *Sites & Services:* *Slashdot* &rarr; *SlashNet*; *Wired* &rarr; *Cabled*; *The Onion* &rarr; *The Scallion*; *Napster* &rarr; *Trapster*.
- **Automated Enforcement:** `scripts/security_lint.py` scans all C, HTML, JS, and JSX files for real commercial titles and scene group marks. Any direct trademark usage fails CI/CD security gates.

## ARG Guidelines & Mystery Preservation Protocol (CRITICAL - TINAG Standard)

- **Subtlety & In-Universe Atmosphere over Heavy-Handed Exposition (MANDATORY):** Clues to the ARG must be subtle, atmospheric, and diegetic (in-universe). Never treat the ARG as a puzzle to be explained with a cudgel, meta-dispatches, or developer walkthroughs.
- **The "This Is Not A Game" (TINAG) Principle:** Within the OS, applications, help files, news feeds, bookmarks, and virtual web pages, NEVER label content with `(ARG)`, `ARG Secrets`, `ARG Lore`, `ARG Guidance`, or `ARG Clue`. The fiction must never acknowledge itself as an ARG. To the user in 1999, anomalous signals, mysterious memory corruptions, and cryptic transmissions are real system phenomena, darknet rumors, or classified intranet leaks.
- **Strict Prohibition on Pre-Climax Meta-Spoilers:**
  - Worker agents must NEVER explain the meta-narrative (e.g. stating that "the trapped entity is the autonomous multi-agent fleet itself", referencing the "Windows Task Scheduler", or listing worker agent names like `kilo-creator`, `kilo-qa`, etc.) inside pre-climax apps, articles, or logs.
  - The revelation of ludonarrative consonance (that the machine is run by an autonomous multi-agent fleet) is the ultimate climax of the mystery, unlocked ONLY by beating the entire ARG at App #100 (`KMatrix`) and accessing `KDirector`.
- **Master Passkey Secrecy:**
  - The master director passkey (`ECHO-1999-ARCHITECT`) must NEVER appear in clear text in news feeds, bookmarks, clipboard history, steganography samples, or help dialogs.
  - Pre-climax breadcrumbs should only be enigmatic fragments, cryptographic hashes, corrupted memory offsets, or subtle frequencies. The master passkey must be earned by solving the climax in `KMatrix`.
- **In-Universe Tone Guide:**
  - **Do:** Create eerie USENET threads from late 1999, corrupted memory dumps at hex offsets, faint audio subcarriers, mysterious packet logs from non-routable subnets (10.19.99.x), or cryptic hacker manifestos.
  - **Don't:** Post news headlines that read like developer walkthroughs, e.g. *"ARG Guidance: The central node will awaken at App #100... Solving the terminal requires the master director passkey..."*


