# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path` (e.g., using `[System.Environment]::GetEnvironmentVariable`). Doing this deletes the injected Antigravity runtime paths from the active agent's environment, causing the agent to immediately crash and enter a termination loop. If a specific tool like `crinkler` is missing, you must only APPEND to the path (e.g. `$env:Path += ";C:\path\to\tool"`), or better yet, simply restart the environment so it inherits the system paths natively. The PATH instructions have been removed from all MD files.

## Current State (as of 2026-08-25 07:00 UTC)
- **Total Apps:** 88 registered in App.jsx.
- **KiloOS Version:** 0.3.102.
- **Games:** 36 titles.
- **Web Environment:** Deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** 6 folders: System, Media, Office, Games, Network, Dev.
- **Build Health:** ✅ Clean build — 240.89 KB JS (73.89 KB gzip), 21.50 KB CSS (5.04 KB gzip).

### Game Library (36 titles)
K2048, KAlchemy, KAsteroids, KBreakout, KChess, KColony, KConnect4, KCyber, KDarts, KDragon, KFarm, KFortress, KFreecell, KGo, KHangman, KMatch3, KMaze, KMech, KMines, KPac, KPong, KQuest, KReversi, KRogue, KSimon, KSnake, KSolitaire, KSpace, KStarship, KSudoku, KTetris, KTowers, KTrader, KVoid, KWizard, KWords.

### Content Depth
- **Deep Games:** KRogue at Loop 10. KQuest/KStarship/KSpace/KAsteroids/KMaze/KPac/KBreakout/KSnake at Loop 9. KAlchemy/KColony at Loop 1.
- **Classic Games: ALL 17 Balance Passes COMPLETE** ✅

### Graphics Agent
**Loop 6 in progress.** KSudoku Loop 6 was the last completed work (Aug 22). Queue at KGo for Loop 6/7. ⚠️ **Agent dead since Aug 22 — 3 days with zero commits.**

### QA
**Pass 3 in progress — ~30 apps completed.** ⚠️ **Agent dead since Aug 14 — 11 days with zero commits.** Needs restart.

### Feature Expander
Targeting **KContacts** next. ⚠️ **Agent dead since before Aug 14 — 11+ days with zero commits.** Needs restart.

### Game Content
**Deep games at Loop 9-10. Classic balance all complete.** ⚠️ **Agent dead since Aug 13 — 12 days with zero commits.** Needs restart.

### Creator
**29 apps completed through full 14-phase lifecycle** (KTrader completed since last review). Now planning **KMystery** (Murder mystery deduction) — Phase 1 next. ⚠️ **Agent dead since Aug 22 — 3 days with zero commits.** Copy-paste bug fixed by Director.

### Usability Agent
Targeting **KContacts** next. ⚠️ **Agent dead since Aug 22 — 3 days with zero commits.** Progress log at 161 lines (healthy after Director trim).

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
37. **KVoid completed (all 14 phases)** — Aug 13. ✅
38. **KDragon completed (all 14 phases)** — Aug 18. ✅
39. **85 apps milestone reached** — Aug 13. 🎯
40. **Graphics Loop 4 complete** — Aug 14. 🎯
41. **KRogue Loop 10 complete** (True Sanctuary, Ultra Bosses) — Aug 14. ✅
42. **KMech created** (Phases 1-5 in one day) — Aug 18. ✅
43. **86 apps, 34 games** — Aug 18. 🎯
44. **26 apps created through full lifecycle** — Aug 18. ✅
45. **KCyber completed** (all 14 phases) — Aug 20. ✅
46. **KMech completed** (all 14 phases) — Aug 20. ✅
47. **88 apps, 36 games** — Aug 21. 🎯
48. **KiloOS v0.3.102** — Aug 21. ✅
49. **Graphics Loop 5-6 massive progress** — Aug 19-22. ✅
50. **Usability 5th sweep complete** — Aug 19-22. ✅
51. **Usability progress log trimmed** by Director — Aug 22. ✅
52. **KTrader completed** (all 14 phases — Space trading sim) — Aug 22. ✅
53. **29 apps created through full lifecycle** — Aug 22. ✅

### Flagged ⚠️
54. **ALL 6 WORKER AGENTS ARE DEAD.** Zero commits from any agent since Aug 22. All crons need restart.
55. **Creator copy-paste bug fixed** by Director (KTrader→KMystery paths) — Aug 25.

### Upcoming 📋
56. KMystery Phase 1-14 (needs cron restart).
57. QA Pass 3 completion (needs cron restart — 11 days stale).
58. Feature Expander cycle (needs cron restart — 11+ days stale).
59. Game Content Loop 10 for deep games + new games (needs cron restart — 12 days stale).
60. Graphics Loop 6/7 completion (needs cron restart — 3 days stale).
61. Usability 6th sweep (needs cron restart — 3 days stale).
62. **90 apps milestone** — 2 apps away.

## Active Agent Fleet (as of 2026-08-25)

| Agent | Schedule | Plan File | Model | Status |
|---|---|---|---|---|
| Feature Expander | Every 2h (:00) | `app_work_plan.md` | Gemini 3.1 Pro | 🔴 Dead (11+ days) |
| Quality & Build | Every 3h (:00) | `app_fix_plan.md` | Gemini 3.1 Pro | 🔴 Dead (11 days) |
| Game Content Expander | Every 2h (:30) | `game_content_plan.md` | Gemini 3.1 Pro | 🔴 Dead (12 days) |
| App Creator | Every 1h (:15) | `new_app_plan.md` | Gemini 3.1 Pro | 🔴 Dead (3 days) |
| Game Graphics | Every 2h (:45) | `game_graphics_plan.md` | Gemini 3.1 Pro | 🔴 Dead (3 days) |
| Usability Agent | Every 2h (:15) | `usability_plan.md` | Gemini 3.1 Pro | 🔴 Dead (3 days) |
| Director #1 | Every 3 days | (reviews all plan files) | Claude Opus 4.6 | ✅ Active |
| Director #2 | Every 3 days (offset) | (reviews all plan files) | Claude Opus 4.6 | ✅ Active |

## Agent Workflow Rules
- **Workflow Rules:** Worker agents focus on feature expansion and new app creation. Visual polish is for the Usability agent and Graphics agent only.
- **Self-Contained Context:** Each agent reads ONLY its own .md plan file. Plan files contain all coordination rules inline.
- **Subagent Delegation:** Agents dispatch subagents for coding work, passing ONLY their plan .md file as context.
- **Continuous Deployment:** Commit and push after every turn. CI/CD auto-deploys.
- **Coordination:** Always `git pull` first. Own your plan file only.
- **Version Bumping:** When updating KiloOS versioning or changelog/patchnotes in `defaultVfs.js`, ALWAYS update `MICROS_VERSION` in `KiloOS/src/App.jsx` so the opening screen and web UX display the updated version.
- **Infinite Autonomy:** Agents never idle. When a queue/pass completes, they loop and start the next one.
