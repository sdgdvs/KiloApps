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
8. **🌐 Seamless Online Multiplayer Expansion via Firebase (DIRECTOR MANDATE - CRITICAL)**:
   - **Core Purpose**: Concentrate on retrofitting and expanding existing games and collaborative applications with seamless online multiplayer powered by Firebase Realtime Database.
   - **Cross-Computer Play**: Enable players visiting `kiloapps.web.app` from different computers anywhere in the world—who are not otherwise communicating and share no local network—to connect, challenge each other, and play in real-time, identical to how KChat connects global users in its `#general` room.
   - **Priority Expansion Targets**:
     - *Turn-Based Board & Strategy Games*: *KChess, KConnect4, KGo, KReversi, KDarts, KCheckers, KBattleship, KCards*. Implement shared room state (`multiplayer/<app>/rooms/<roomId>`), real-time move synchronization via RTDB `push`/`onValue`, turn alternation, spectator view, and global public matchmaking (`multiplayer/<app>/lobby`).
     - *Collaborative Tools*: *KDraw, KPaint, KSynth (collaborative jam), KPad (shared live text)*. Sync canvas draw strokes or text buffers in real-time between connected peers.
     - *Competitive Arcade Duels*: Real-time high-score races, side-by-side split screens, attack line sending (e.g. *KTetris*, *K2048*, *KSnake* dual arenas).
   - **Technical Standard**:
     - Load Firebase via ES modules directly from Google CDN (adds 0 bytes to git build, strictly preserving <999 KB):
       ```javascript
       import { initializeApp } from "https://www.gstatic.com/firebasejs/10.9.0/firebase-app.js";
       import { getDatabase, ref, set, get, push, onValue, onChildAdded, off, serverTimestamp, onDisconnect } from "https://www.gstatic.com/firebasejs/10.9.0/firebase-database.js";
       const firebaseConfig = {
           apiKey: "AIzaSyDns9KBDxyd4v-TbAvi5xLrVkbXaUt_9GE",
           authDomain: "kiloappschat.firebaseapp.com",
           databaseURL: "https://kiloappschat-default-rtdb.firebaseio.com",
           projectId: "kiloappschat",
           storageBucket: "kiloappschat.firebasestorage.app",
           messagingSenderId: "290208566057",
           appId: "1:290208566057:web:deacd8c7457d0cc7ec0538"
       };
       const app = initializeApp(firebaseConfig);
       const db = getDatabase(app);
       ```
     - Handle room joining, player presence (`onDisconnect()`), clean unmounting (`off()`), and always preserve local / solo / AI play as a default fallback.

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
