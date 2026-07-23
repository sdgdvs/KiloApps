# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## Current State (as of 2026-07-23)
- **Total Apps:** 71 registered in App.jsx.
- **KiloOS Version:** 0.3.60
- **Games:** 25 registered game titles (TARGET HIT ✅). Plus KQuest in progress.
- **Web Environment:** Deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** 6 folders: System, Media, Office, Games, Network, Dev.

### Game Library (25 titles)
K2048, KAsteroids, KBreakout, KChess, KConnect4, KDarts, KFreecell, KGo, KHangman, KMatch3, KMaze, KMines, KPac, KPong, KQuest (in progress), KReversi, KRogue, KSimon, KSnake, KSolitaire, KSpace, KSudoku, KTetris, KTowers, KWords.

### Content Depth
- Original 12 games: 6 loops of content expansion complete (Loops 1-6). Campaign modes with 15-30 levels, multiple power-ups, boss enemies, biomes, statistics tracking.
- Creator games (KSudoku, KAsteroids, KFreecell, KConnect4, KHangman, KSimon, KMatch3, KWords, KGo): Loops 5-6 content passes complete.
- Newest games (KDarts, KTowers, KReversi): Created through Phase 14, awaiting first content loop.
- KQuest: RPG in progress (Phase 9 of 14).

### Graphics
New Graphics agent (activated by Director #1) adding pixel-art sprites and animations. Loop 1 complete for: KSpace, KPac, KSnake, KTetris, KBreakout. Working through remaining games.

### QA
Pass 1 complete (46 apps). Pass 2 in progress: ~25 apps done. Next: KTask.

### Feature Expander
Round-robin feature additions across all apps. Recently: KMedia (advanced playback), KPass (password strength), KNote (search/pin/export), KPac (save/load), KMaze (save/load). Working through queue normally.

### Creator
KReversi complete (Phase 14). KDarts complete. KTowers complete. Now building KQuest (RPG — Phase 9). Directive updated to focus on deep fantasy/sci-fi games.

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

### Active 🔄
19. Games agent: Loop 6 finishing (KSimon, KMatch3, KWords, KGo pending). New games (KDarts, KTowers, KReversi) need first content pass.
20. Graphics agent: Loop 1 sprite work (5/25 games done).
21. Creator: KQuest RPG (Phase 9/14).
22. Feature Expander: Round-robin feature additions (perpetual).
23. QA: Pass 2 deeper quality checks (~25/71 done).

### Upcoming 📋
24. Approach commercial-grade quality for key games (leverage 999KB budget fully).
25. Graphics Loop 2: animations and environmental art.
26. Build pipeline improvements: automated size-limit checks.
27. Consider model migration: Gemini 3.6 Flash High for worker agents (cost savings ~45%).

## Active Agent Fleet (as of 2026-07-23)

| Agent | Schedule | Plan File | Model |
|---|---|---|---|
| Feature Expander | Every 2h (:00) | `app_work_plan.md` | Gemini 3.6 Flash High |
| Quality & Build | Every 3h (:00) | `app_fix_plan.md` | Gemini 3.6 Flash High |
| Game Content Expander | Every 2h (:30) | `game_content_plan.md` | Gemini 3.6 Flash High |
| App Creator | Every 1h (:15) | `new_app_plan.md` | Gemini 3.6 Flash High |
| Game Graphics | Every 2h (:45) | `game_graphics_plan.md` | Gemini 3.6 Flash High |
| Director #1 | Every 3 days | (reviews all plan files) | Claude Opus 4.6 |
| Director #2 | Every 3 days (offset) | (reviews all plan files) | Claude Opus 4.6 |
| ~~Shell & UX~~ | ~~Suspended~~ | `kiloos_ux_plan.md` | — |

## Agent Workflow Rules
- **Features Only, No Polish:** All agents focus on feature expansion, game content depth, and new app creation. Visual polish is explicitly out of scope (except Graphics agent for game sprites).
- **Self-Contained Context:** Each agent reads ONLY its own .md plan file. Plan files contain all coordination rules inline.
- **Subagent Delegation:** Agents dispatch subagents for coding work, passing ONLY their plan .md file as context.
- **Continuous Deployment:** Commit and push after every turn. CI/CD auto-deploys.
- **Coordination:** Always `git pull` first. Own your plan file only.
- **Infinite Autonomy:** Agents never idle. When a queue/pass completes, they loop and start the next one.
