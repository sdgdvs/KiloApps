# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path` (e.g., using `[System.Environment]::GetEnvironmentVariable`). Doing this deletes the injected Antigravity runtime paths from the active agent's environment, causing the agent to immediately crash and enter a termination loop. If a specific tool like `crinkler` is missing, you must only APPEND to the path (e.g. `$env:Path += ";C:\path\to\tool"`), or better yet, simply restart the environment so it inherits the system paths natively. The PATH instructions have been removed from all MD files.

## Current State (as of 2026-08-16 07:00 UTC)
- **Total Apps:** 85 registered in App.jsx.
- **KiloOS Version:** 0.3.99
- **Games:** 33 titles.
- **Web Environment:** Deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** 6 folders: System, Media, Office, Games, Network, Dev.
- **Build Health:** ✅ Clean build — 240.56 KB JS (73.86 KB gzip), 21.50 KB CSS (5.04 KB gzip).

### Game Library (33 titles)
K2048, KAlchemy, KAsteroids, KBreakout, KChess, KColony, KConnect4, KDarts, KDragon, KFarm, KFortress, KFreecell, KGo, KHangman, KMatch3, KMaze, KMines, KPac, KPong, KQuest, KReversi, KRogue, KSimon, KSnake, KSolitaire, KSpace, KStarship, KSudoku, KTetris, KTowers, KVoid, KWizard, KWords.

### Content Depth
- **Deep Games:** KQuest at Loop 9 (factions, mounts, crafting). KStarship at Loop 9 (factions, planetary landing). KRogue Loop 9. KSpace, KAsteroids, KMaze, KPac, KBreakout at Loop 9. KSnake Loop 8+.
- **Classic Games: ALL 17 Balance Passes COMPLETE** ✅ (plus KConnect4 additional balance work Aug 13).
- **The Deep/Classic split directive continues to work perfectly.**

### Graphics Agent
**Loop 5 in progress.** Completed Loops 1-4 for all games. Loop 5 polish done for: K2048, KAsteroids, KMines, KPong, KChess, KRogue, KMaze, KBreakout, KTetris, KSnake, KPac, KSpace. Queue: KSolitaire → KStarship → KConnect4 → ... (26 games in queue). Missing from queue: KDragon, KVoid, KWizard (new games — agent should auto-add).

### QA
**Pass 3 in progress — ~30 apps completed.** Targeting **KTimer** next. Recently: KChart, KChess, KClock, KContacts, KConverter, KDB, KExplorer, KFont, KHex, KImage, KJournal, KTerm, KTetris, KSynth, KSys. Very productive cycle.

### Feature Expander
Targeting **KContacts** next. Recently completed: KTimer (HIIT timer), KConverter (pressure + expression parser), KRogue (custom keybindings + high score export), KSys (telemetry + services), KTask (process inspector), KSynth (delay/echo), KType (certificate export). Active and productive.

### Creator
**25 apps completed through full 14-phase lifecycle.** Now building **KDragon** (Fantasy creature-raising sim) — Phase 14/14 (failed with RESOURCE_EXHAUSTED, will retry). Recently completed KVoid (all 14 phases). KDragon at Phase 13 complete.

### Usability Agent
All 85 apps processed through 3+ full sweeps. Targeting **KChat** next (start of 4th sweep). Recently: KChart, KCalendar, KHex, KCalc, KImage, KPad, KBase, KTimer, KConverter, KTodo, KSys, KTask, KSynth, KTerm, KRogue, KGraph, KContacts, KRead, KJournal. Missing from queue: KDragon, KVoid, KWizard (new games — agent should auto-add).

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
38. **KDragon created (Phases 1-13 complete)** — Aug 14. ✅
39. **85 apps milestone reached** — Aug 13. 🎯
40. **Graphics Loop 4 complete** — Aug 14. 🎯
41. **Graphics Loop 5 in progress** (~12/33 done) — Aug 14. 🔄
42. **QA Pass 3 in progress** (~30/85 apps) — Aug 14. 🔄
43. **Games Loop 9 deep games in progress** — Aug 13. 🔄

### Active 🔄
44. Games agent: **Loop 9+ content for Deep games** — all Classic balance passes done.
45. Graphics agent: Loop 5 polish in progress.
46. Creator: KDragon Phase 14 retry (RESOURCE_EXHAUSTED on first attempt).
47. Feature Expander: Round-robin perpetual (targeting KContacts).
48. QA: Pass 3 regression/security checks (targeting KTimer).
49. Usability: Round-robin UX audit (targeting KChat, 4th sweep).

### Upcoming 📋
50. Graphics Loop 5 completion for all 33 games.
51. KDragon Phase 14 completion (Help & Guide).
52. QA Pass 3 completion for all apps.
53. Games Loop 10 for deep games.
54. 86+ apps via Creator (next app after KDragon).

## Active Agent Fleet (as of 2026-08-16)

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
