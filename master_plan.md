# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path` (e.g., using `[System.Environment]::GetEnvironmentVariable`). Doing this deletes the injected Antigravity runtime paths from the active agent's environment, causing the agent to immediately crash and enter a termination loop. If a specific tool like `crinkler` is missing, you must only APPEND to the path (e.g. `$env:Path += ";C:\path\to\tool"`), or better yet, simply restart the environment so it inherits the system paths natively. The PATH instructions have been removed from all MD files.

## Current State (as of 2026-08-01 04:00 UTC)
- **Total Apps:** 82 registered in App.jsx (+9 since Jul 29).
- **KiloOS Version:** 0.3.65+
- **Games:** 29 titles (KColony Phase 8/14 building).
- **Web Environment:** Deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** 6 folders: System, Media, Office, Games, Network, Dev.

### Game Library (29 titles)
K2048, KAlchemy, KAsteroids, KBreakout, KChess, KColony (WIP), KConnect4, KDarts, KFortress (WIP), KFreecell, KGo, KHangman, KMatch3, KMaze, KMines, KPac, KPong, KQuest, KReversi, KRogue, KSimon, KSnake, KSolitaire, KSpace, KStarship, KSudoku, KTetris, KTowers, KWords.

### Content Depth
- **Deep Games progressing:** KRogue at Loop 9 (traps, boss rematches, cursed items). KSpace/KAsteroids/KSnake/KBreakout/KMaze/KPac all at Loop 8.
- **Classic Games: 9/17 Balance Passes COMPLETE** (Chess, Go, Reversi, Connect4, Solitaire, Freecell, Sudoku, K2048, KMines). 8 remaining (Towers, Tetris, Pong, Hangman, Words, Simon, Match3, Darts).
- **The Deep/Classic split directive (Jul 29) is working perfectly.** No more campaign spam on board games.

### Graphics Agent
**Loop 2 deep in progress.** 15+ games done in Loop 2 (KHangman, KConnect4, KSolitaire, K2048, KAsteroids, KMines, KPong, KChess, KRogue, KTetris, KSnake, KPac, KSpace, KBreakout, KMaze). Queue: KSimon → KMatch3 → KFreecell → KSudoku → KGo → KDarts → KTowers → KWords → KReversi → KAlchemy → remaining.

### QA
**Pass 3 deep in progress.** QA completed: KFont, KHex, KImage, KJournal, KMail, KMandel, KMatch3, KMaze, KMedia, KMines, KNotes, KPaint (+prior Pass 3 apps). Now targeting **KPassword**. Systematic focus on security (XSS, bounds), GDI leaks, and memory safety.

### Feature Expander
Healthy round-robin. 62+ completed items. Recent: KConnect4 (Save/Load), KScript (Debugger), KMaze (Replay), KMine (Hint), KPac (Save/Load), KQuest (Save/Load), KNote (AES encryption), KPass (AES vault), KMedia (Subtitles), KNet (Traceroute), KZip (Compression), KPaint (Layers). Now targeting **KType**.

### Creator
**20 apps completed through full 14-phase lifecycle.** KAlchemy ✅. Now building **KColony** (Sci-Fi Colony Survival) — Phase 8/14 done (Random Events & Disasters). Phase 9 next (Alien Threats). Very healthy pace.

### Usability Agent
**Highly active.** ~20 apps processed in first sweep. Common fixes: auto-opening window size, crisp text rendering (canvas DPI), "Press H for Help" keybindings, layout issues. Recently: KDB, KPong, KClock, KAudio, KBBS, KChat, KChess, KChart, KCalendar, KCalc, KHex, KImage, KPad, KJournal, KBase, KRead, KContacts, KTimer, KTodo, KGraph. Now targeting **KFont**.

## Milestones

### Completed ✅
1. Initial ecosystem with 50+ apps — July 7.
2. CI/CD pipeline (GitHub Actions → Firebase) — July 7.
3. Desktop folder organization — July 7.
4. First polish round (KCalc, KBBS, KAudio) — July 7.
5. KiloOS design overhaul (start menu, taskbar) — July 7.
6. Agent fleet optimization (7 agents → 4, admin bloat eliminated) — July 9.
7. Full polish/expansion pass of all existing apps — July 10-11.
8. 5th agent added (App Creator & Deep Expander) — July 11.
9. 14 new apps created through full 14-phase lifecycle — July 11-20.
10. Games Loops 1-4 complete for original 12 games — July 13-19.
11. QA Pass 1 complete (46 apps) — July 18.
12. Fleet restructured: UX suspended, features-only mandate — July 17.
13. Game library expanded from 12 to 21 titles — July 19-20.
14. **25 game target reached** — July 22. 🎯
15. Games Loop 5 complete (all 21 games) — July 20.
16. Games Loop 6 complete (original 12 + KSudoku-KHangman) — July 22.
17. Graphics agent activated, sprite work on 5 games — July 22.
18. VS2022 Build Tools installed for native compilation — July 22.
19. **Games Loop 6 complete for ALL 27 games** — July 23-25.
20. **Games Loop 7 complete for ALL 27 games** — July 25-29. 🎯
21. **Graphics Loop 1 complete for ALL 27 games** — July 22-29. 🎯
22. **QA Pass 2 complete** — July 25.
23. **Model migration to Gemini 3.6 Flash High** for all worker agents — July 22. ✅
24. **KAlchemy completed (all 14 phases)** — July 29. ✅
25. **19 apps created through full 14-phase lifecycle** — July 29. ✅
26. **Usability Agent activated** — July 29.
27. **Graphics Loop 2 started** — 15+ games done — July 29-Aug 1. 🎯
28. **Deep/Classic game split directive implemented** — Jul 29. Game balance passes begun.
29. **9/17 Classic Games Balance Passes COMPLETE** — Aug 1. ✅
30. **80+ apps milestone reached** — Aug 1. 🎯
31. **Model reverted to Gemini 3.1 Pro** — Jul 29 (Flash used too many tokens).

### Active 🔄
32. Games agent: **Balance passes for remaining 8 Classic games** + Loop 9+ content for Deep games.
33. Graphics agent: Loop 2 in progress (~15/27 done).
34. Creator: KColony Phase 9 of 14 (Alien Threats).
35. Feature Expander: Round-robin perpetual (targeting KType).
36. QA: Pass 3 regression/security checks (targeting KPassword).
37. Usability: Round-robin UX audit (targeting KFont).

### Upcoming 📋
38. Complete all Classic Games Balance Passes (8 remaining).
39. Graphics Loop 2 completion for all 27+ games.
40. KColony completion (14 phases, ~6 phases remaining).
41. QA Pass 3 completion for all apps.
42. Usability audit completion for all 82 apps.
43. Consider next Creator project after KColony.

## Active Agent Fleet (as of 2026-08-01)

| Agent | Schedule | Plan File | Model |
|---|---|---|---|
| Feature Expander | Every 2h (:00) | `app_work_plan.md` | Gemini 3.1 Pro |
| Quality & Build | Every 3h (:00) | `app_fix_plan.md` | Gemini 3.1 Pro |
| Game Content Expander | Every 2h (:30) | `game_content_plan.md` | Gemini 3.1 Pro |
| App Creator | Every 1h (:15) | `new_app_plan.md` | Gemini 3.1 Pro |
| Game Graphics | Every 2h (:45) | `game_graphics_plan.md` | Gemini 3.1 Pro |
| Usability Agent | Every 2h (:15) | `usability_plan.md` | Gemini 3.1 Pro |
| Director #1 | Every 3 days | (reviews all plan files) | Gemini 3.1 Pro |
| Director #2 | Every 3 days (offset) | (reviews all plan files) | Gemini 3.1 Pro |

## Agent Workflow Rules
- **Workflow Rules:** Worker agents focus on feature expansion and new app creation. Visual polish is for the Usability agent and Graphics agent only.
- **Self-Contained Context:** Each agent reads ONLY its own .md plan file. Plan files contain all coordination rules inline.
- **Subagent Delegation:** Agents dispatch subagents for coding work, passing ONLY their plan .md file as context.
- **Continuous Deployment:** Commit and push after every turn. CI/CD auto-deploys.
- **Coordination:** Always `git pull` first. Own your plan file only.
- **Infinite Autonomy:** Agents never idle. When a queue/pass completes, they loop and start the next one.
