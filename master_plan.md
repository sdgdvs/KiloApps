# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path` (e.g., using `[System.Environment]::GetEnvironmentVariable`). Doing this deletes the injected Antigravity runtime paths from the active agent's environment, causing the agent to immediately crash and enter a termination loop. If a specific tool like `crinkler` is missing, you must only APPEND to the path (e.g. `$env:Path += ";C:\path\to\tool"`), or better yet, simply restart the environment so it inherits the system paths natively. The PATH instructions have been removed from all MD files.

## Current State (as of 2026-08-31 04:37 UTC)
- **Total Apps:** 85 registered in App.jsx.
- **KiloOS Version:** 0.3.104.
- **Games:** 39 titles.
- **Web Environment:** Deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** 6 folders: System, Media, Office, Games, Network, Dev.
- **Build Health:** ✅ Clean build — 242.02 KB JS (74.12 KB gzip), 21.50 KB CSS (5.04 KB gzip).

### Game Library (39 titles)
K2048, KAlchemy, KAsteroids, KBreakout, KChess, KColony, KColosseum, KConnect4, KCyber, KDarts, KDragon, KFarm, KFortress, KFreecell, KGo, KHangman, KMatch3, KMaze, KMech, KMines, KMystery, KPac, KPong, KQuest, KReversi, KRogue, KSimon, KSnake, KSolitaire, KSpace, KStarship, KStellar, KSudoku, KTetris, KTowers, KTrader, KVoid, KWizard, KWords.

### Content Depth
- **Deep Games:** KRogue at Loop 10. Others at Loop 9. Newer games at Loop 1.
- **Classic Games: ALL 17 Balance Passes COMPLETE** ✅
- ⚠️ Game Content agent dead 15 days — no content progress. KCyber, KTrader, KMystery, KColosseum, KStellar not in queues.

### Graphics Agent
**Loop 7 in progress.** Massive batch since last review: completed Loop 7 for KSnake, KPac, KSpace, KQuest, KTowers, KWords, KGo, K2048, KAsteroids, KMines, KPong, KChess, KRogue, KMaze, KBreakout, KTetris, KSolitaire. Also Loop 1-6 for newer games (KCyber L1, KTrader L1, KDragon L2, KSimon L2, KDarts L4, KFarm L5, KFortress L5, KColony L5, KAlchemy L6, KHex L2, KMine L2, KMandel L2). Queue at KSolitaire/KDragon area. ✅ Active — 26 commits in 72h.

### QA
**Pass 3 in progress — ~30 apps completed.** ⚠️ **Agent dead since Aug 14 — 14 days with zero commits.** Needs restart.

### Feature Expander
Targeting **KContacts** next. ⚠️ **Agent dead since before Aug 14 — 14+ days with zero commits.** Needs restart.

### Game Content
**Deep games at Loop 9-10. Classic balance all complete.** ⚠️ **Agent dead since Aug 13 — 15 days with zero commits.** Needs restart.

### Creator
**31 apps completed through full 14-phase lifecycle** (up from 29 — KMystery + KColosseum completed). Now building **KStellar** (Space exploration/trading RPG) — Phase 5 next. Incredibly productive: completed 2 full app lifecycles and started a 3rd in 72h. ✅ Active — ~30 commits in 72h.

### Usability Agent
Currently processing queue — recently completed KHex, KCalc, KImage, KPad, KCalendar through KMaze sweep. ✅ Active — 24 commits in 72h. Progress log at 191 lines (healthy). ⚠️ Minor: UTF-8 encoding corruption in plan file headers (mojibake), not blocking work.

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
28. **Deep/Classic game split directive implemented** — Jul 29.
29. **ALL 17 Classic Games Balance Passes COMPLETE** — Aug 12. ✅ 🎯
30. **80+ apps milestone reached** — Aug 1. 🎯
31. **Model reverted to Gemini 3.1 Pro** — Jul 29.
32. **KColony completed (all 14 phases)** — Aug 2. ✅
33. **Graphics Loop 3 complete** — Aug 11. 🎯
34. **KWizard completed (all 14 phases)** — Aug 12. ✅
35. **Games Loop 8 complete for ALL games** — Aug 11. ✅
36. **KVoid completed (all 14 phases)** — Aug 13. ✅
37. **KDragon completed (all 14 phases)** — Aug 18. ✅
38. **85 apps milestone reached** — Aug 13. 🎯
39. **Graphics Loop 4 complete** — Aug 14. 🎯
40. **KRogue Loop 10 complete** — Aug 14. ✅
41. **KCyber completed** (all 14 phases) — Aug 20. ✅
42. **KMech completed** (all 14 phases) — Aug 20. ✅
43. **KTrader completed** (all 14 phases) — Aug 22. ✅
44. **29 apps created through full lifecycle** — Aug 22. ✅
45. **KiloOS v0.3.102** — Aug 21. ✅
46. **KMystery completed** (all 14 phases — Murder mystery deduction) — Aug 26. ✅
47. **KColosseum completed** (all 14 phases — Gladiator management RPG) — Aug 27. ✅
48. **31 apps created through full lifecycle** — Aug 27. ✅
49. **KStellar scaffolded and progressing** (Phases 1-4 done) — Aug 27. ✅
50. **🎯 90 APPS MILESTONE REACHED** — 91 apps registered — Aug 26. 🎯
51. **39 games** — Aug 27. 🎯
52. **Graphics Loop 7 massive progress** — 17+ games completed at Loop 7 — Aug 26-28. ✅
53. **Usability sweep continues** — 24 apps processed Aug 25-28. ✅

### Active 🔄
54. Graphics agent: Loop 7 in progress.
55. Creator: KStellar Phase 5 (space travel mechanics).
56. Usability: Continuing sweep through queue.

### Flagged ⚠️
57. **QA agent: ZERO commits since Aug 14** — cron dead 14 days. 4th review flagged.
58. **Feature Expander: ZERO commits since before Aug 14** — cron dead 14+ days. 4th review flagged.
59. **Game Content: ZERO commits since Aug 13** — cron dead 15 days. 4th review flagged.

### Upcoming 📋
60. KStellar Phase 5-14 completion.
61. QA Pass 3 completion (needs cron restart — 14 days stale).
62. Feature Expander cycle (needs cron restart — 14+ days stale).
63. Game Content for new games: KCyber, KTrader, KMystery, KColosseum, KStellar (needs cron restart — 15 days stale).
64. **95 apps milestone** — 4 apps away.
65. **40 games milestone** — 1 game away.

## Active Agent Fleet (as of 2026-08-28)

| Agent | Schedule | Plan File | Model | Status |
|---|---|---|---|---|
| Feature Expander | Every 2h (:00) | `app_work_plan.md` | Gemini 3.1 Pro | 🔴 Dead (14+ days) |
| Quality & Build | Every 3h (:00) | `app_fix_plan.md` | Gemini 3.1 Pro | 🔴 Dead (14 days) |
| Game Content Expander | Every 2h (:30) | `game_content_plan.md` | Gemini 3.1 Pro | 🔴 Dead (15 days) |
| App Creator | Every 1h (:15) | `new_app_plan.md` | Gemini 3.1 Pro | ✅ Active |
| Game Graphics | Every 2h (:45) | `game_graphics_plan.md` | Gemini 3.1 Pro | ✅ Active |
| Usability Agent | Every 2h (:15) | `usability_plan.md` | Gemini 3.1 Pro | ✅ Active |
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
