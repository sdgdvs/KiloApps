---
name: kilo-creator
description: >-
  Designs and implements new applications and deep game worlds for the KiloApps fleet.
  Use this skill to create feature-rich single-file web apps (<999 KB), enforce mandatory
  start splash screens, first-run tutorial flags, quicksave/quickload, register in KiloOS/src/App.jsx,
  verify builds, log tersely to next_work.md, hand off to the next agent, and commit/push.
---

# KiloApps App Creator Skill

This skill designs and implements new applications or deep game worlds on exactly ONE application per turn.

## Pre-flight
1. Ensure git working tree is clean: `git status`.
2. Pull latest changes: `git pull --rebase`.
3. Open [next_work.md](../../next_work.md) to inspect the creator target queue (`current_targets.kilo_creator`).

## App Creation Guidelines & Director Directives
1. **Genre & Scope Focus**:
   - Focus on **deep, emergent fantasy and science fiction games** (RPGs, space exploration, roguelikes, survival, strategy, colony management) or **deep utility tools** (diagnostic, audio/visual workstation, productivity).
   - **NO MORE**: simple board games (chess, checkers, connect4), simple pattern/card clones, or trivial toys.
2. **Mandatory App Requirements**:
   - **Start Splash Screen**: App opens to a title/splash overlay with version number, New Game / Continue (if save exists), Settings, and Help. Never drop immediately into raw gameplay without context.
   - **First-Run Tutorial**: For complex games/apps, auto-start an onboarding tutorial on new game. Store flag `k<name>_tutorialSeen` in `localStorage` so it never repeats. Tutorial must be skippable with Escape.
   - **Full State Persistence**: Quicksave (F5) and quickload (F9), plus JSON save export/import.
   - **Self-Contained Web File**: Single HTML5 file located at `KiloOS/public/apps/k<name>.html` with inline CSS/JS.
   - **Native Counterpart (Optional/Recommended)**: When creating native C version, place in `K<Name>/main.c` with `build.bat`.
   - **Hard Size Ceiling**: Web file and native executable must each be strictly `< 999 KB`.

3. **System Registration**:
   - Add new app descriptor to the `APPS` array in `KiloOS/src/App.jsx` with appropriate folder (`Games`, `System`, `Media`, `Office`, `Network`, `Dev`).
   - Run `npm run build` inside `KiloOS/` to confirm zero Vite build breaks.

4. **KMatrix & Project-Wide ARG Mandate (App #100 Milestone)**:
   - For the 100th milestone app (`KMatrix`), build it as the central narrative meta-terminal and ARG climax resolving "The Kilo Project Echoes" and "Rogue AI" lore.
   - Connect it to cross-project ARG clues: corrupted logs in KTerm, ghost audio in KSynth/KAudio, memory hex offsets in KHex, and precursor relics in KAbyss/KCosmic/KChrono.
   - Successfully solving KMatrix must reward the player with the Master Director Passkey (`ECHO-1999-ARCHITECT`) to unlock the in-OS `KDirector` console!

## Queue Handoff & Terse Logging (CRITICAL)
1. **Edit [next_work.md](../../next_work.md)**:
   - Advance `current_targets.kilo_creator` to the next concept.
   - Set `current_agent` to the next scheduled agent in `agent_rotation` (or call `kilo-graphics` / `kilo-tester` if immediate visual/audit follow-up is desired).
   - Set `status: ready`.
   - Update `last_run.agent: kilo-creator`, `last_run.app: <NewAppName>`, and `last_run.timestamp`.
   - Append terse run log (strict limit: ≤ 8 lines of concise bullet points).
2. **Commit and Push**:
   - `git add <new files> KiloOS/src/App.jsx next_work.md`
   - `git commit -m "feat(app): create new app <AppName>"`
   - `git push` (if rejected, run `git pull --rebase` then push).
   - STOP immediately.
