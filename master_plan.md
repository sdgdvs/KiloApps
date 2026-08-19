# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path` (e.g., using `[System.Environment]::GetEnvironmentVariable`). Doing this deletes the injected Antigravity runtime paths from the active agent's environment, causing the agent to immediately crash and enter a termination loop. If a specific tool like `crinkler` is missing, you must only APPEND to the path (e.g. `$env:Path += ";C:\path\to\tool"`), or better yet, simply restart the environment so it inherits the system paths natively. The PATH instructions have been removed from all MD files.

## Current State (as of 2026-08-19 07:00 UTC)
- **Total Apps:** 86 registered in App.jsx (up from 85).
- **KiloOS Version:** 0.3.99
- **Games:** 34 titles (KMech added since last review).
- **Web Environment:** Deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** 6 folders: System, Media, Office, Games, Network, Dev.
- **Build Health:** ✅ Clean build — 240.69 KB JS (73.88 KB gzip), 21.50 KB CSS (5.04 KB gzip).

### Game Library (34 titles)
K2048, KAlchemy, KAsteroids, KBreakout, KChess, KColony, KConnect4, KDarts, KDragon, KFarm, KFortress, KFreecell, KGo, KHangman, KMatch3, KMaze, KMech, KMines, KPac, KPong, KQuest, KReversi, KRogue, KSimon, KSnake, KSolitaire, KSpace, KStarship, KSudoku, KTetris, KTowers, KVoid, KWizard, KWords.

### Content Depth
- **Deep Games:** KRogue at Loop 10 (True Sanctuary, Ultra Bosses). KQuest/KStarship/KSpace/KAsteroids/KMaze/KPac/KBreakout/KSnake at Loop 9. KAlchemy/KColony at Loop 1.
- **Classic Games: ALL 17 Balance Passes COMPLETE** ✅
- **The Deep/Classic split directive continues to work perfectly.**

### Graphics Agent
**Loop 5 in progress.** Recently completed: KSolitaire, KConnect4, KHangman, KMatch3, KFreecell (Loop 5), KStarship (Loop 4). Queue: KSudoku → KGo → ... → KFreecell (36 games in queue with newly added KDragon, KMech, KVoid, KDarts, KSimon). ~18/34 games done in Loop 5.

### QA
**Pass 3 in progress — ~30 apps completed.** ⚠️ Zero commits in last 72h — possible cron failure. Last QA commit was Aug 14.

### Feature Expander
Targeting **KContacts** next. ⚠️ Zero commits in last 72h — possible cron failure. Last Expander commit was before Aug 14.

### Game Content
**Deep games at Loop 9-10. Classic balance all complete.** ⚠️ Zero commits in last 72h — possible cron failure. Last Content commit was Aug 13 (KStarship Loop 9).

### Creator
**26 apps completed through full 14-phase lifecycle.** KDragon completed (all 14 phases) ✅. Now building **KMech** (Sci-fi turn-based mech combat) — Phase 6 next. Active and productive (7 commits in 72h).

### Usability Agent
Targeting **KDB** next (queue was reorganized by user). Recently completed: KChat, KChess, KAudio, KBBS, KPong, KClock. 6 commits in 72h. Progress log at ~198 entries (approaching 200 trim threshold). KDragon, KMech, KRadio now in queue ✅.

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

### Active 🔄
45. Graphics agent: Loop 5 polish in progress (~18/34 done).
46. Creator: KMech Phase 6 (Heat management system).
47. Usability: Round-robin UX audit (targeting KDB, 4th sweep).

### Flagged ⚠️
48. **QA agent: ZERO commits in 72h** — cron may have died. Last commit Aug 14.
49. **Feature Expander: ZERO commits in 72h** — cron may have died.
50. **Game Content: ZERO commits in 72h** — cron may have died. Last commit Aug 13.

### Upcoming 📋
51. Graphics Loop 5 completion for all 34 games.
52. KMech Phase 6-14 completion.
53. QA Pass 3 completion (needs cron restart).
54. Feature Expander cycle (needs cron restart).
55. Game Content Loop 10 for deep games (needs cron restart).

## Active Agent Fleet (as of 2026-08-19)

| Agent | Schedule | Plan File | Model | Status |
|---|---|---|---|---|
| Feature Expander | Every 2h (:00) | `app_work_plan.md` | Gemini 3.1 Pro | ⚠️ No commits 72h |
| Quality & Build | Every 3h (:00) | `app_fix_plan.md` | Gemini 3.1 Pro | ⚠️ No commits 72h |
| Game Content Expander | Every 2h (:30) | `game_content_plan.md` | Gemini 3.1 Pro | ⚠️ No commits 72h |
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
