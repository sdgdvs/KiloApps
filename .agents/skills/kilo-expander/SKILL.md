---
name: kilo-expander
description: >-
  Executes deep functional feature expansions for productivity, system, dev, and media applications.
  Use this skill to deepen utility logic, add export/import formats, advanced data manipulation,
  diagnostic depth, verify file sizes (<999 KB) and builds, log tersely to next_work.md,
  advance the queue handoff, and commit/push.
---

# KiloApps Feature Expander Skill

This skill deepens functional utility and capabilities on exactly ONE application per turn.

## Pre-flight
1. Ensure git working tree is clean: `git status`.
2. Pull latest changes: `git pull --rebase`.
3. Open [next_work.md](../../next_work.md) to inspect the expander target (`current_targets.kilo_expander`).

## Expansion Directives by Category
1. **🛠️ System & Dev Tools** (*KTerm, KSys, KTask, KNet, KPing, KHex, KBase, KConverter, KCalc, KScript, KZip, KFont*):
   - Focus: Diagnostic depth, granular controls, syntax parsing, advanced regex filtering, hex inspection, performance analysis.
2. **📝 Productivity & Data** (*KPad, KNote, KDB, KTodo, KJournal, KCalendar, KContacts, KMail, KRead, KPass*):
   - Focus: Data interoperability, multi-tab sessions, tagging, search indexing, schema flexibility, export formats (CSV, JSON, Markdown).
3. **🎨 Media & Creative** (*KPaint, KImage, KAudio, KSynth, KMedia, KChart, KGraph, KMandel, KType*):
   - Focus: Format support, DSP/audio synthesis (Yamaha YM2612 2-op FM, SNES SPC700 delay echo, ADSR envelopes), image processing filters, canvas layers.
4. **🎮 Games (Engine Utility & Multiplayer Expansion)**:
   - Focus: Replay viewers, custom key rebinding, save state management, PGN/FEN/board state import/export, and online multiplayer integration.
   - **DO NOT** add solo campaign levels, bosses, or cosmetic sprite sheets (reserved for `kilo-graphics`).
5. **🌐 Virtual 1999 Web Expansion (`virtual_web_target`)**:
   - When assigned to advance the virtual web or when native app targets are mature, expand `virtual_web_target` in `KiloOS/public/web/*.html`.
   - **Anti-Potemkin Quality Standard**: Sites must NEVER be shallow placeholders or fake stubs. Build genuine Web 1.0 depth: working simulated backends (guestbooks, search indices, calculators, voting polls), Genesis/SNES Web Audio synthesizers/MIDI jukeboxes, demoscene cracktros, retro browser mini-games, downloadable text/tracker assets, and interconnected hypermedia links.
   - Maintain strict adherence to HTML 4.01 retro styling, zero external dependencies, and file size strictly `< 999 KB`.
6. **Maturity & Skip Protocol**:
   - If an app has undergone 6+ passes and is functionally complete without active requests: log `⏭️ Skip — app is feature-complete and mature.` Rotate to queue bottom and finish cleanly.
7. **Alternate Reality Fictionalization Mandate**:
   - All commercial video game titles, software products, corporate entities, and demoscene warez groups must be fictionalized parodies (e.g. *Surreal Tournament*, *Tremor III Arena*, *VoidCraft*, *Machina Ex*, *FLARELIGHT*, *RAZOR 1999*, *SlashNet*, *Cabled*). Never use real trademarked names. Enforced algorithmically by `scripts/security_lint.py`.
8. **ARG Mystery Preservation & TINAG Standard (CRITICAL)**:
   - Clues must be subtle, atmospheric, and diegetic. Never use `(ARG)` or `ARG Lore` in UI or copy.
   - Never post explicit walkthroughs ("ARG Guidance"), spoil the autonomous fleet meta-twist before the endgame, or leak `ECHO-1999-ARCHITECT` in clear text.
9. **🌐 Seamless Online Multiplayer Expansion via Firebase (DIRECTOR MANDATE - CRITICAL)**:
   - **Core Purpose**: Concentrate on retrofitting and expanding existing games and collaborative applications with seamless online multiplayer powered by Firebase Realtime Database.
   - **Cross-Computer Play**: Enable players visiting `kiloapps.web.app` from different computers anywhere in the world—who are not otherwise communicating and share no local network—to connect, challenge each other, and play in real-time, identical to how KChat connects global users in its `#general` room.
   - **Priority Expansion Targets**:
     - *Turn-Based Board & Strategy Games*: *KChess, KConnect4, KGo, KReversi, KDarts, KCheckers, KBattleship, KCards*. Implement shared room state (`multiplayer/<app>/rooms/<roomId>`), real-time move synchronization via RTDB `push`/`onValue`, turn alternation, spectator view, and global public matchmaking (`multiplayer/<app>/lobby`).
     - *Collaborative Tools*: *KDraw, KPaint, KSynth (collaborative jam), KPad (shared live text)*. Sync canvas draw strokes or text buffers in real-time between connected peers.
     - *Competitive Arcade Duels*: Real-time high-score races, side-by-side split screens, attack line sending (e.g. *KTetris*, *K2048*, *KSnake* dual arenas).
   - **Technical Standard (Retro Firebase Multiplayer Service - RFMS)**:
     - All multiplayer retrofits MUST use the standardized RFMS client module [`KiloOS/public/assets/js/retro_multiplayer.js`](../../KiloOS/public/assets/js/retro_multiplayer.js) (spec: [`docs/RFMS_SPEC.md`](../../docs/RFMS_SPEC.md)).
     - Include `<script src="../assets/js/retro_multiplayer.js"></script>` in the app's HTML.
     - **Mandate Rule 12 Compliance (CRITICAL)**: Always wire `onSoloFallback` or a 25-second timer (`soloTimeoutMs: 25000`) so lone players automatically transition to an active local AI opponent if no peer connects.
     - **Dual Link Sharing**: Support both `#room=CODE` (preferred for KiloOS iframe embed) and `?room=CODE` query formats with automatic `history.replaceState` sync.
     - Standard Usage Pattern:
       ```javascript
       const mp = new RetroMultiplayer({
           gameId: '<app>',
           prefix: '<3-letter-prefix>',
           soloTimeoutMs: 25000,
           onMove: (data) => applyRemoteMove(data),
           onSoloFallback: () => startLocalAiGame(),
           onStatusMsg: (msg, type) => showToast(msg, type)
       });
       await mp.init();
       ```
     - Handle player presence with automatic `onDisconnect()` cleanup and maintain 100% offline functionality.

## Verification
1. Verify web app build: `cd KiloOS && npm run build`.
2. Verify size constraint: `< 999 KB`.

## Queue Handoff & Terse Logging (CRITICAL)
1. **Edit [next_work.md](../../next_work.md)**:
   - Advance `current_targets.kilo_expander` to next app in queue.
   - Set `current_agent` to next scheduled agent in `agent_rotation`.
   - Set `status: ready`.
   - Update `last_run.agent: kilo-expander`, `last_run.app: <TargetApp>`, and `last_run.timestamp`.
   - Append terse run log (strict limit: ≤ 8 lines of concise bullet points).
2. **Commit and Push**:
   - `git add <modified files> next_work.md`
   - `git commit -m "feat(expand): feature expansion for <TargetApp>"`
   - `git push` (if rejected, run `git pull --rebase` then push).
   - STOP immediately.
