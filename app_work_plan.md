# App Work Plan

## DIRECTOR NOTE (2026-07-29): STOP BLANKET FEATURE SPAM — FOCUS ON APP-SPECIFIC UTILITY

**⚠️ READ THIS BEFORE EVERY TURN. This supersedes old loop instructions.**

The old approach of adding generic "Search, Save/Load, and Import/Export" to every single app is OVER. A hex editor needs different features than a music synthesizer. Furthermore, the Feature Expander has been adding gameplay features (bosses, power-ups, campaigns) to Games — **STOP DOING THIS**. Gameplay content belongs to the Game Content agent.

**From now on, apps are split into categories with DIFFERENT work priorities:**

### 🛠️ SYSTEM & DEV TOOLS
*Apps: KTerm, KSys, KTask, KNet, KPing, KHex, KBase, KConverter, KCalc, KScript, KZip, KFont*
- **Focus:** Diagnostic depth, granular controls, advanced parsing, system accuracy.
- **Do:** Add packet sniffing logic, advanced regex search, macro scripting, deep memory inspection, hardware benching, large file handling.
- **Don't:** Add visual fluff. 

### 📝 PRODUCTIVITY & DATA
*Apps: KPad, KNote, KDB, KTodo, KJournal, KCalendar, KContacts, KMail, KRead, KPass*
- **Focus:** Data interoperability, workflow efficiency, robust search, data security.
- **Do:** Add CSV/JSON/Markdown import/export, AES encryption/password protection, multi-tab sessions, tagging, advanced filtering, auto-save.
- **Don't:** Add basic features that already exist. Deepen the data structures.

### 🎨 MEDIA & CREATIVE
*Apps: KPaint, KImage, KAudio, KSynth, KMedia, KChart, KGraph, KMandel, KType*
- **Focus:** Format support, processing algorithms, rendering performance, complex transformations.
- **Do:** Add new export formats (WAV, PNG, WEBP), audio/image filters (FFT, convolution matrices, ADSR), layer support, waveform visualization.
- **Don't:** Make them simple toys. Add professional-grade utility features.

### 🎮 GAMES (UTILITY ONLY)
*Apps: KConnect4, KMaze, KMine, KPac, KQuest, KSnake, KTetris, KSpace, KSolitaire, KRogue, KChess, KPong, KBBS*
- **Focus:** Meta-features and Engine Utility.
- **Do:** Add Save/Load states (F5/F9), High Score JSON Export/Import, Replay Viewers, PGN/FEN parsers, custom keybinding config.
- **Don't:** DO NOT add campaigns, power-ups, new enemies, or bosses. Leave gameplay content to the Game Content agent!

## DIRECTOR DIRECTIVE (2026-09-13): MATURITY & TURN SKIPPING FOR APPS WITH 6+ PASSES

**⚠️ READ THIS BEFORE EVERY TURN:**
- Unless you have a good, specific directive from the director or user to add something to an app, **"turn skipped because this app is complete and we don't have new ideas here"** is completely fine for apps that have already been through 6+ passes.
- The director can add new ideas or directions later and you can implement those things on the next turn, but do NOT just add random, poorly-thought-out features just to make a commit.
- If a mature app is functionally complete and has no active directive, log the skip concisely, rotate it to the bottom, and end your turn cleanly.

---

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 6 worker agents + 2 directors operate on this repo on overlapping schedules. You are the **Feature Expander**.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`app_work_plan.md`).** Read but NEVER edit:
  - `app_fix_plan.md` (QA agent), `game_content_plan.md` (Games agent), `new_app_plan.md` (Creator agent), `usability_plan.md`
- **Shared file `KiloOS/src/App.jsx`** — shared ownership. You may ONLY add entries to the APPS array. Protocol: `git pull` → make minimal APPS-only change → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — Do NOT edit.
- **Dual-target model:** Each app has a native C version (`K[Name]/main.c` + `build.bat`) and a web HTML5 version (`KiloOS/public/apps/k[name].html`). Both versions should offer functional parity where feasible. Web HTML files must be single self-contained files (inline CSS + JS, no imports).
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.
- **Version bumping:** If you modify KiloOS shell files or update versioning/changelog, bump the patch version in `KiloOS/package.json` AND update `MICROS_VERSION` in `KiloOS/src/App.jsx` so the opening screen displays the current version.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **Logging discipline:** Keep this plan file concise. A few lines per completed item. Do NOT dump file contents or create verbose logs.

---

## ⏱️ TURN SCOPING & TERMINATION (CRITICAL — READ EVERY TURN)

**Single-Item-Per-Turn Rule:**
- Each cron trigger = ONE turn. Process exactly ONE item from your queue, then STOP.
- "Loop forever" means the CRON loops forever across turns, NOT that you loop within a single turn.
- After committing and pushing your work for ONE item, STOP CALLING TOOLS immediately.

**Subagent Timeout Rule:**
- If you spawn a subagent, set a timer for 8 minutes using the `schedule` tool with `TimerCondition` set to the subagent's conversation ID.
- If the timer fires (subagent hasn't finished in 8 min), KILL the subagent using `manage_subagents`, log a one-line failure note in your plan file, commit, push, and STOP.
- NEVER spawn more than ONE subagent at a time.
- NEVER spawn a second subagent if the first one failed. Stop and let the next cron turn retry.
- **Model Selection:** Always spawn worker subagents using the `flash` model (`Model: "flash"`) to prevent 503 server capacity bottlenecks.

**Graceful Termination Checklist (do this EVERY turn before stopping):**
1. Processed one item
2. Updated plan file
3. Committed and pushed
4. All subagents terminated (killed or completed)
5. STOP — call no more tools

---

**Target App:** KScript
**Status:** In Queue
**Current Phase:** In Queue

## Round-Robin Continuous Improvement Queue (NEVER STOP — loop forever)
Pick the top app from this list, add a meaningful new feature based on its **CATEGORY PRIORITY**, and move it to the bottom. Update BOTH web and native versions. You have up to **999KB per app**.

> 📁 **Archived Completed Work**: Full detail of all completed feature expansions is in [archive/app_work_history.md](archive/app_work_history.md).

### Completed Apps (1-line index — full details in archive)
KScript ✅, KMaze ✅, KMine ✅, KPac ✅, KQuest ✅, KNote ✅, KPass ✅, KMedia ✅, KNet ✅, KZip ✅, KPaint ✅, KSnake ✅, KTetris ✅, KSpace ✅, KType ✅, KSolitaire ✅, KTerm ✅, KSynth ✅, KTask ✅, KSys ✅, KRogue ✅, KConverter ✅, KTodo ✅, KGraph ✅, KTimer ✅, KContacts ✅, KRead ✅, KBase ✅, KJournal ✅, KPad ✅, KImage ✅, KHex ✅, KCalc ✅, KCalendar ✅, KChart ✅, KChess ✅, KChat ✅, KBBS ✅, KAudio ✅, KClock ✅, KPong ✅, KDB ✅, KFont ✅, KMail ✅, KMandel ✅, KPing ✅, KConnect4 ✅

## DIRECTOR DIRECTIVE (2026-09-15): TUTORIAL SYSTEMS

**NEW PRIORITY when processing apps from the queue:**
- **Utility Apps**: If the app lacks a built-in tutorial or "How to Play/Use" section in its Help modal, add one. Tutorials for simple utility apps should be buried inside the Help menu (F1/H) � NOT auto-shown on startup.
- **Games**: If a game lacks a tutorial, add one. For complex/deep games (KRogue, KQuest, KMaze, KSpace, etc.), the tutorial should auto-start when beginning a NEW game but NOT when loading a saved game. Use a localStorage flag like `k[game]_tutorialSeen` to prevent repeated showing. For classic/simple games, bury the tutorial in the Help modal.
- **Save Systems**: Verify all games have working quicksave (F5) / quickload (F9) with localStorage persistence. Verify save/load doesn't corrupt state.
