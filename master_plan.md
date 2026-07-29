# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path` (e.g., using `[System.Environment]::GetEnvironmentVariable`). Doing this deletes the injected Antigravity runtime paths from the active agent's environment, causing the agent to immediately crash and enter a termination loop. If a specific tool like `crinkler` is missing, you must only APPEND to the path (e.g. `$env:Path += ";C:\path\to\tool"`), or better yet, simply restart the environment so it inherits the system paths natively. The PATH instructions have been removed from all MD files.

## Current State (as of 2026-07-29)
- **Total Apps:** 73 registered in App.jsx.
- **KiloOS Version:** 0.3.63
- **Games:** 27 titles.
- **Web Environment:** Deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** 6 folders: System, Media, Office, Games, Network, Dev.

### Game Library (27 titles)
K2048, KAlchemy (Phase 9), KAsteroids, KBreakout, KChess, KConnect4, KDarts, KFreecell, KGo, KHangman, KMatch3, KMaze, KMines, KPac, KPong, KQuest, KReversi, KRogue, KSimon, KSnake, KSolitaire, KSpace, KStarship, KSudoku, KTetris, KTowers, KWords.

### Content Depth
- 18 games at **Loop 7** complete (original 13 + KAsteroids, KFreecell, KConnect4, KHangman, KSimon). 20-35 stage campaigns, boss encounters, active skill hotkeys.
- 9 games at **Loop 6** complete (KMatch3, KWords, KGo, KDarts, KTowers, KReversi, KStarship, KQuest). Loop 7 pending.
- KAlchemy: Phase 11 complete, Phase 12 next (Creator building).

### Graphics Agent
**Loop 1 complete for 24/27 games.** Custom sprites for nearly every game. Remaining: KQuest + Loop 2 starts for earlier games.

### QA
**Pass 3 in progress.** KChat, KChess, KClock done. Now on KColor. ~40+ apps done across all passes.

### Feature Expander
Healthy round-robin. Recent: KImage, KHex, KCalc, KCalendar, KChart all got major features. Currently targeting KChess.

### Creator
KAlchemy Phase 11 complete. Phase 12 (Game Modes & Challenges) next. On track to complete by ~Jul 31.

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
20. **Games Loop 7 complete for 18/27 games** — July 25-29.
21. **Graphics Loop 1 complete for 24/27 games** — July 22-29.
22. **QA Pass 2 complete** — July 25.
23. **Model migration to Gemini 3.6 Flash High** for all worker agents — July 22. ✅

### Active 🔄
24. Games agent: Loop 7 for remaining 9 games (KMatch3 → KQuest).
25. Graphics agent: Loop 1 finishing (KQuest), then Loop 2 begins.
26. Creator: KAlchemy Phase 12 of 14.
27. Feature Expander: Round-robin perpetual (targeting KChess).
28. QA: Pass 3 deeper checks (targeting KColor).

### Upcoming 📋
29. Games Loop 8 for original games — approaching commercial-grade depth.
30. Graphics Loop 2: animations and environmental art.
31. Build pipeline improvements: automated size-limit checks.

## Active Agent Fleet (as of 2026-07-29)

| Agent | Schedule | Plan File | Model |
|---|---|---|---|
| Feature Expander | Every 2h (:00) | `app_work_plan.md` | Gemini 3.6 Flash High |
| Quality & Build | Every 3h (:00) | `app_fix_plan.md` | Gemini 3.6 Flash High |
| Game Content Expander | Every 2h (:30) | `game_content_plan.md` | Gemini 3.6 Flash High |
| App Creator | Every 1h (:15) | `new_app_plan.md` | Gemini 3.6 Flash High |
| Game Graphics | Every 2h (:45) | `game_graphics_plan.md` | Gemini 3.6 Flash High |
| Director #1 | Every 3 days | (reviews all plan files) | Claude Opus 4.6 |
| Director #2 | Every 3 days (offset) | (reviews all plan files) | Claude Opus 4.6 |
| Usability Agent | Every 2h (:15) | `usability_plan.md` | Gemini 3.6 Flash High |

## Agent Workflow Rules
- **Workflow Rules:** Worker agents focus on feature expansion and new app creation. Visual polish is for the Usability agent and Graphics agent only.
- **Self-Contained Context:** Each agent reads ONLY its own .md plan file. Plan files contain all coordination rules inline.
- **Subagent Delegation:** Agents dispatch subagents for coding work, passing ONLY their plan .md file as context.
- **Continuous Deployment:** Commit and push after every turn. CI/CD auto-deploys.
- **Coordination:** Always `git pull` first. Own your plan file only.
- **Infinite Autonomy:** Agents never idle. When a queue/pass completes, they loop and start the next one.
