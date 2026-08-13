# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path` (e.g., using `[System.Environment]::GetEnvironmentVariable`). Doing this deletes the injected Antigravity runtime paths from the active agent's environment, causing the agent to immediately crash and enter a termination loop. If a specific tool like `crinkler` is missing, you must only APPEND to the path (e.g. `$env:Path += ";C:\path\to\tool"`), or better yet, simply restart the environment so it inherits the system paths natively. The PATH instructions have been removed from all MD files.

## Current State (as of 2026-08-13 07:00 UTC)
- **Total Apps:** 84 registered in App.jsx.
- **KiloOS Version:** 0.3.88
- **Games:** 32 titles.
- **Web Environment:** Deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** 6 folders: System, Media, Office, Games, Network, Dev.
- **Build Health:** ✅ Clean build — 240.42 KB JS (73.83 KB gzip), 21.50 KB CSS (5.04 KB gzip).

### Game Library (32 titles)
K2048, KAlchemy, KAsteroids, KBreakout, KChess, KColony, KConnect4, KDarts, KFarm, KFortress, KFreecell, KGo, KHangman, KMatch3, KMaze, KMines, KPac, KPong, KQuest, KReversi, KRogue, KSimon, KSnake, KSolitaire, KSpace, KStarship, KSudoku, KTetris, KTowers, KVoid, KWizard, KWords.

### Content Depth
- **Deep Games:** KRogue, KQuest, KStarship at Loop 8. KSpace, KAsteroids, KMaze, KPac, KBreakout at Loop 9. KSnake at Loop 8. KAlchemy, KColony at Loop 1.
- **Classic Games: ALL 17 Balance Passes COMPLETE** (Chess, Go, Reversi, Connect4, Solitaire, Freecell, Sudoku, K2048, KMines, Towers, Tetris, Pong, Hangman, Words, Simon, Match3, Darts). ✅
- **The Deep/Classic split directive (Jul 29) continues to work perfectly.**

### Graphics Agent
**Loop 4 in progress.** Completed Loop 3 for all games, now doing Loop 4 polish. Recently: KMaze, KBreakout, KTetris, KSnake, KPac, KSpace polish. Queue: KAlchemy → KRogue → KChess → ... (26 games in queue).

### QA
**Pass 3 in progress.** Targeting **KSys** next. Recently completed: KSynth, KSpace, KSolitaire, KSnake, KScript, KRogue, KRead, KQuarantine. Pass 3 active and productive.

### Feature Expander
Targeting **KConverter** next. Recently completed: KRogue (custom keybindings, high score export), KSys (real-time telemetry, services manager), KTask (deep process inspector), KTerm (macro scripting), KSynth (delay/echo), KType (certificate export). Active and productive.

### Creator
**24 apps completed through full 14-phase lifecycle.** Now building **KVoid** (Sci-fi horror survival) — Phase 7/14 done. Recently completed KWizard (all 14 phases).

### Usability Agent
All 84 apps processed through at least 2 full sweeps. Targeting **KTerm** next. Recently: KSolitaire, KSpace, KTetris, KSnake, KFarm, KPaint.

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
27. **Graphics Loop 2 complete** — Aug 2. 🎯
28. **Deep/Classic game split directive implemented** — Jul 29. Game balance passes begun.
29. **ALL 17 Classic Games Balance Passes COMPLETE** — Aug 12. ✅ 🎯
30. **80+ apps milestone reached** — Aug 1. 🎯
31. **Model reverted to Gemini 3.1 Pro** — Jul 29 (Flash used too many tokens).
32. **KColony completed (all 14 phases)** — Aug 2. ✅
33. **Graphics Loop 3 complete** — Aug 11. 🎯
34. **KWizard completed (all 14 phases)** — Aug 12. ✅
35. **24 apps created through full 14-phase lifecycle** — Aug 12. ✅
36. **Games Loop 8 complete for ALL games** — Aug 11. ✅
37. **Games Loop 9 deep games in progress** — Aug 12. 🔄
38. **QA Pass 3 in progress (~20 apps done)** — Aug 12. 🔄
39. **Graphics Loop 4 in progress** — Aug 12. 🔄

### Active 🔄
40. Games agent: **Loop 9+ content for Deep games** — all Classic balance passes done.
41. Graphics agent: Loop 4 polish in progress.
42. Creator: KVoid Phase 8 of 14.
43. Feature Expander: Round-robin perpetual (targeting KConverter).
44. QA: Pass 3 regression/security checks (targeting KSys).
45. Usability: Round-robin UX audit (targeting KTerm).

### Upcoming 📋
46. Graphics Loop 4 completion for all 32 games.
47. KVoid completion (7 phases remaining).
48. QA Pass 3 completion for all apps.
49. Games Loop 10 for deep games.
50. 85+ apps via Creator (KDragon next after KVoid).

## Active Agent Fleet (as of 2026-08-13)

| Agent | Schedule | Plan File | Model |
|---|---|---|---|
| Feature Expander | Every 2h (:00) | `app_work_plan.md` | Gemini 3.1 Pro |
| Quality & Build | Every 3h (:00) | `app_fix_plan.md` | Gemini 3.1 Pro |
| Game Content Expander | Every 2h (:30) | `game_content_plan.md` | Gemini 3.1 Pro |
| App Creator | Every 1h (:15) | `new_app_plan.md` | Gemini 3.1 Pro |
| Game Graphics | Every 2h (:45) | `game_graphics_plan.md` | Gemini 3.1 Pro |
| Usability Agent | Every 2h (:15) | `usability_plan.md` | Gemini 3.1 Pro |
| Director #1 | Every 3 days | (reviews all plan files) | Claude Opus 4.6 |
| Director #2 | Every 3 days (offset) | (reviews all plan files) | Claude Opus 4.6 |

## Agent Workflow Rules
- **Workflow Rules:** Worker agents focus on feature expansion and new app creation. Visual polish is for the Usability agent and Graphics agent only.
- **Self-Contained Context:** Each agent reads ONLY its own .md plan file. Plan files contain all coordination rules inline.
- **Subagent Delegation:** Agents dispatch subagents for coding work, passing ONLY their plan .md file as context.
- **Continuous Deployment:** Commit and push after every turn. CI/CD auto-deploys.
- **Coordination:** Always `git pull` first. Own your plan file only.
- **Version Bumping:** When updating KiloOS versioning or changelog/patchnotes in `defaultVfs.js`, ALWAYS update `MICROS_VERSION` in `KiloOS/src/App.jsx` so the opening screen and web UX display the updated version.
- **Infinite Autonomy:** Agents never idle. When a queue/pass completes, they loop and start the next one.
