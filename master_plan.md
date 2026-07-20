# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## Current State (as of 2026-07-20)
- **Total Apps:** 67 registered in App.jsx.
- **KiloOS Version:** 0.3.58
- **Games:** 21 registered game titles.
- **Web Environment:** Deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** 6 folders: System, Media, Office, Games, Network, Dev.

### New Apps Created with Deep Expansion (14 through Phase 14)
KVault, KBudget, KHabit, KFlash, K2048, KBBS, KSudoku, KFreecell, KConnect4, KHangman, KSimon, KAsteroids, KMatch3, KWords. KGo in progress (Phase 5).

### Feature Expander: Round-Robin Continuous Improvement
Cycling through all apps adding one meaningful feature per turn. Recently completed: Traceroute for KPing. Added KConnect4 to queue. Working through round-robin normally.

### Games: 21 titles, Loop 5 in progress (2/21 done)
**Game titles:** K2048, KAsteroids, KBreakout, KChess, KConnect4, KFreecell, KGo, KHangman, KMatch3, KMaze, KMines, KPac, KPong, KRogue, KSimon, KSnake, KSolitaire, KSpace, KSudoku, KTetris, KWords.
Loops 1-4 fully complete for original 12 games. Loop 5 started: KMines and KRogue done. 9 new Creator games added to inventory awaiting their first content loop.
Content depth is genuinely impressive — campaign modes, power-ups, procedural generation, boss enemies, biomes, sound effects, stats tracking.

### QA: Pass 1 complete (46 apps), Pass 2 in progress
Pass 2 deeper quality checks: 8 apps done (K2048, KAudio, KBBS, KCalc, KChart, KChat, KChess, KClock). Next: KColor.

### Shell/UX: Suspended
15+ improvements delivered previously. Agent suspended to focus compute on content.

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

### Active 🔄
14. **🎮 PRIORITY: Reach 25+ game titles.** Creator agent building KGo (4 more titles needed after KGo).
15. Games agent: Loop 5 with 21 games (2/21 done). New Creator games getting first content pass.
16. Feature Expander: Round-robin feature additions across all apps (perpetual).
17. QA: Pass 2 deeper quality checks (8 of ~67 done).

### Upcoming 📋
18. Approach commercial-grade quality for key games (leverage 999KB budget fully).
19. Build pipeline improvements: automated size-limit checks, CI artifact upload.
20. Automated testing or build-verification for native apps.
21. Consider reactivating Shell/UX agent once game library reaches 25+ titles.

## Active Agent Fleet (as of 2026-07-20)

| Agent | Schedule | Plan File | Model |
|---|---|---|---|
| Feature Expander | Every 2h (:00) | `app_work_plan.md` | Gemini 3.1 Pro High |
| Quality & Build | Every 3h (:00) | `app_fix_plan.md` | Gemini 3.1 Pro Low |
| Game Content Expander | Every 2h (:30) | `game_content_plan.md` | Gemini 3.1 Pro High |
| App Creator | Every 1h (:15) | `new_app_plan.md` | Gemini 3.1 Pro High |
| Director #1 | Every 3 days | (reviews all plan files) | Claude Opus 4.6 |
| Director #2 | Every 3 days (offset) | (reviews all plan files) | Claude Opus 4.6 |
| ~~Shell & UX~~ | ~~Suspended~~ | `kiloos_ux_plan.md` | — |

## Agent Workflow Rules
- **Features Only, No Polish:** All agents focus on feature expansion, game content depth, and new app creation. Visual polish is explicitly out of scope.
- **Self-Contained Context:** Each agent reads ONLY its own .md plan file. Plan files contain all coordination rules inline. No cross-referencing master_plan.md or architecture.md.
- **Subagent Delegation:** Agents dispatch subagents for coding work, passing ONLY their plan .md file as context. This prevents context bloat over many cron cycles.
- **Continuous Deployment:** Commit and push after every turn. CI/CD auto-deploys.
- **Coordination:** Always `git pull` first. Own your plan file only.
- **Infinite Autonomy:** Agents never idle. When a queue/pass completes, they loop and start the next one. Designed for month-long unattended operation.
