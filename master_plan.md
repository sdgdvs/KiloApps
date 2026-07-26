# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path` (e.g., using `[System.Environment]::GetEnvironmentVariable`). Doing this deletes the injected Antigravity runtime paths from the active agent's environment, causing the agent to immediately crash and enter a termination loop. If a specific tool like `crinkler` is missing, you must only APPEND to the path (e.g. `$env:Path += ";C:\path\to\tool"`), or better yet, simply restart the environment so it inherits the system paths natively. The PATH instructions have been removed from all MD files.

## Current State (as of 2026-07-26)
- **Total Apps:** 73 registered in App.jsx.
- **KiloOS Version:** 0.3.60
- **Games:** 27 titles (25 target hit + KStarship + KAlchemy).
- **Web Environment:** Deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** 6 folders: System, Media, Office, Games, Network, Dev.

### Game Library (27 titles)
K2048, KAlchemy (Phase 9), KAsteroids, KBreakout, KChess, KConnect4, KDarts, KFreecell, KGo, KHangman, KMatch3, KMaze, KMines, KPac, KPong, KQuest, KReversi, KRogue, KSimon, KSnake, KSolitaire, KSpace, KStarship, KSudoku, KTetris, KTowers, KWords.

### Content Depth
- Original 13 games (KMines→KSudoku): **7 loops** of content expansion complete. Campaign modes with 20-35 levels, boss encounters, active skill hotkeys, biomes.
- Secondary games (KAsteroids→KReversi + KStarship, KQuest): **Loop 6** content passes complete.
- KAlchemy: Phase 8 complete, Phase 9 in progress (Creator building).

### Graphics Agent
**Loop 1 complete for 22 games.** Custom sprites: player/enemy pixel art, 3D boards, card portraits, particle FX, animations. Remaining in queue: KTowers, KWords, KReversi, KQuest, KStarship + KAlchemy.

### QA
**Pass 2 complete. Pass 3 started.** Now targeting KChat & KChatServer. ~35+ apps done across passes.

### Feature Expander
Healthy round-robin. Recent: KGraph, KTimer, KContacts, KRead, KBase, KJournal, KPad all got deep features. Currently targeting KImage.

### Creator
KQuest, KStarship, KDarts, KTowers, KReversi all complete (Phase 14). Now building **KAlchemy** (fantasy crafting game, Phase 8 done, Phase 9 hit quota limit — will retry).

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
20. **Games Loop 7 complete for original 13 games** — July 25.
21. **Graphics Loop 1 complete for 22/27 games** — July 22-25.
22. **QA Pass 2 complete** — July 25.
23. **Model migration to Gemini 3.6 Flash High** for all worker agents — July 22. ✅

### Active 🔄
24. Games agent: Loop 7 for remaining 14 games (KAsteroids → KQuest).
25. Graphics agent: Loop 1 remaining (KTowers, KWords, KReversi, KQuest, KStarship, KAlchemy).
26. Creator: KAlchemy Phase 9 (hit quota limit, will retry).
27. Feature Expander: Round-robin perpetual (targeting KImage).
28. QA: Pass 3 deeper checks (targeting KChat & KChatServer).

### Upcoming 📋
29. Games Loop 8 for original games — approaching commercial-grade depth.
30. Graphics Loop 2: animations and environmental art.
31. Build pipeline improvements: automated size-limit checks.

## Active Agent Fleet (as of 2026-07-26)

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
