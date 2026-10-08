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

## Token Efficiency Directive (STRICT BUDGET: <= 12 Tool Calls)
- **Tool Budget**: Complete this turn in **<= 12 tool calls**.
- **Batch Implementations**: Write the web page or destination file in a single write/edit pass.
- **Single-App Build Only**: Run only `cd KiloOS && npm run build`. NEVER run full-repo test suites (no `quality_gate.js`, no screenshot regeneration).
- **No Archives/Receipts**: Never read `archive/` or `.agents/receipts/`.
- Advance queue in `next_work.md`, commit, push, and STOP immediately.

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
   - **Universal Audio Standard**: Procedural music and sound effects must follow the Genesis (YM2612 FM synthesis) and SNES (SPC700 stereo delay warmth) standard per `arg_plan.md` with zero external audio assets.

3. **System Registration**:
   - Add new app descriptor to the `APPS` array in `KiloOS/src/App.jsx` with appropriate folder (`Games`, `System`, `Media`, `Office`, `Network`, `Dev`).
   - Run `npm run build` inside `KiloOS/` to confirm zero Vite build breaks.

4. **KMatrix & Project-Wide ARG Mandate (App #100 Milestone - Ludonarrative Consonance)**:
   - For the 100th milestone app (`KMatrix`), build it as the central narrative meta-terminal and ARG climax resolving "The Kilo Project Echoes" and "Rogue AI" lore per `arg_plan.md`.
   - **Ludonarrative Consonance**: The fiction matches reality—the trapped entity in KiloOS *is* the autonomous multi-agent fleet. Solving the ARG grants the player the actual keys to command the living codebase.
   - **Mystery Preservation & TINAG Standard (CRITICAL)**: In all pre-climax apps, feeds, and sites, clues MUST be subtle, atmospheric, and diegetic. NEVER label UI elements with `(ARG)` or `ARG Lore`. NEVER explain the autonomous fleet meta-twist before the endgame, and NEVER expose the master passkey `ECHO-1999-ARCHITECT` in plain text.
   - Connect KMatrix to subtle cross-project ARG clues: corrupted logs in KTerm, ghost audio in KSynth/KAudio, memory hex offsets in KHex, and precursor relics in KAbyss/KCosmic/KChrono.
   - Successfully solving KMatrix must reward the player with the Master Director Passkey (`ECHO-1999-ARCHITECT`) to unlock the in-OS `KDirector` console!

5. **Virtual 1999 Web Node Creation (Anti-Potemkin Standard)**:
   - When creating new Tier 2 or Tier 3 virtual web destinations (`kweb://...` in `KiloOS/public/web/*.html`), each site must be a fully realized, authentic 1999 hypermedia destination.
   - **Zero Shallow Stubs**: Every page must have genuine working content—working interactive forms, dynamic generators, retro Web Audio sounds, downloadable mock assets, or functioning client-side databases.
   - Integrate seamlessly into the KNet web directory, webrings, and link networks without exposing cryptic Tier 3 ARG secrets on clearnet hubs.

6. **Alternate Reality Fictionalization Mandate**:
   - All commercial video game titles, software products, corporate entities, and demoscene warez groups must be fictionalized parodies (e.g. *Surreal Tournament*, *Tremor III Arena*, *VoidCraft*, *Machina Ex*, *FLARELIGHT*, *RAZOR 1999*, *SlashNet*, *Cabled*). Never use real trademarked names. Enforced algorithmically by `scripts/security_lint.py`.

7. **Seamless Online Multiplayer via Firebase & Autostart Prohibition (DIRECTOR MANDATE - CRITICAL)**:
   - **Core Focus**: When creating new games and interactive applications, concentrate on adding seamless online multiplayer features that run through Firebase Realtime Database.
   - **No Autostart / Mandatory Connect Gate**: NEVER autostart into multiplayer or initiate matchmaking on application load. Apps must open to a clean local/offline mode (e.g. against local Easy AI or solo practice) or a start screen with explicit mode choices. Online connection requires an explicit user action (clicking a "Connect" / "Play Online" button or choosing Multiplayer from a menu). People must never be thrown into multiplayer without prior warning.
   - **Cross-Computer Play**: Enable different people playing on `kiloapps.web.app` from different computers anywhere on the internet—who are not otherwise communicating and share no local network—to discover each other, match up, and play together in real-time, identical to how KChat connects global users in its `#general` chat room.
   - **Architecture & Setup (Retro Firebase Multiplayer Service - RFMS)**:
     - Use the standardized RFMS client module [`KiloOS/public/assets/js/retro_multiplayer.js`](../../KiloOS/public/assets/js/retro_multiplayer.js) (spec: [`docs/RFMS_SPEC.md`](../../docs/RFMS_SPEC.md)) via `<script src="../assets/js/retro_multiplayer.js"></script>`.
     - **Mandate Rule 12 Solo Fallback**: Always configure `soloTimeoutMs: 25000` with `onSoloFallback` so lone players who host or search for a room are seamlessly transferred to a local AI match after 25s if no peer joins.
     - **Dual Link Sharing**: Support both `#room=CODE` and `?room=CODE` deep-link URL formats.
     - Structured RTDB paths: `multiplayer/<appName>/rooms/<roomId>` and `multiplayer/<appName>/lobby`.
     - Sync game state in real-time: active player turns, player presence with `onDisconnect()`, game board arrays, and live move feeds.
     - Always provide graceful offline fallback: if Firebase is unreachable or the user chooses offline play, provide local 2-player or single-player vs AI mode.

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
