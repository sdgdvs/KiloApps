# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## Current State (as of 2026-07-19)
- **Total Apps:** 58 native Windows applications in individual subdirectories.
- **KiloOS Version:** 0.3.58
- **Web Environment:** Deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** 6 folders: System, Media, Office, Games, Network, Dev.

### New Apps Created with Deep Expansion (6, all through Phase 14)
KVault, KBudget, KHabit, KFlash (flashcards), K2048 (puzzle game), KSudoku (logic puzzle). KBBS also received a full 14-phase deep expansion.

### All Existing Apps Polished + Feature-Expanded
The Builder agent completed a full pass of all ~45 existing apps. It has transitioned to a **round-robin continuous improvement** model, cycling through every app adding one new feature per turn.

### Games: 13 games, Loop 4 in progress
Loops 1-3 complete. Loop 4: 8 of 13 done. Games now have campaign modes, power-ups, persistent stats, AI opponents, and wave progression. Commercial-grade depth approaching.

### QA: Pass 1 complete, Pass 2 started
Pass 1 finished all apps alphabetically. Pass 2 (deeper quality: edge cases, accessibility, error handling) now underway — K2048 tested.

### KiloOS Shell/UX: Suspended
Agent suspended by other director to reallocate quota to feature expansion. 15+ improvements delivered previously. Can be reactivated when needed.

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
9. 6 new apps created through full 14-phase lifecycle — July 11-17.
10. Games Loops 1-3 complete — July 13-17.
11. QA Pass 1 complete — July 17.
12. Fleet restructured: UX suspended, features-only mandate — July 17.

### Active 🔄
13. **🎮 PRIORITY: Expand game library** — Creator agent building games-first. Target: deep 14-phase expansion.
14. Feature Expander: Round-robin feature additions across all apps (perpetual).
15. QA: Pass 2 (deeper quality checks) underway.
16. Games agent: Loop 4 with campaign modes, power-ups, and persistent stats.

### Upcoming 📋
16. Approach commercial-grade quality for key games (leverage 999KB budget fully).
17. Build pipeline improvements: automated size-limit checks, CI artifact upload.
18. Automated testing or build-verification for native apps.

## Active Agent Fleet (as of 2026-07-17)

| Agent | Schedule | Plan File | Model |
|---|---|---|---|
| Feature Expander | Every 2h (:00) | `app_work_plan.md` | Gemini 3.1 Pro High |
| Quality & Build | Every 3h (:00) | `app_fix_plan.md` | Gemini 3.1 Pro Low |
| Game Content Expander | Every 2h (:30) | `game_content_plan.md` | Gemini 3.1 Pro High |
| App Creator | Every 1h (:15) | `new_app_plan.md` | Gemini 3.1 Pro High |
| Director | Every 3 days | (reviews all plan files) | Claude Opus 4.6 |
| ~~Shell & UX~~ | ~~Suspended~~ | `kiloos_ux_plan.md` | — |

## Agent Workflow Rules
- **Features Only, No Polish:** All agents focus on feature expansion, game content depth, and new app creation. Visual polish is explicitly out of scope.
- **Self-Contained Context:** Each agent reads ONLY its own .md plan file. Plan files contain all coordination rules inline. No cross-referencing master_plan.md or architecture.md.
- **Subagent Delegation:** Agents dispatch subagents for coding work, passing ONLY their plan .md file as context. This prevents context bloat over many cron cycles.
- **Continuous Deployment:** Commit and push after every turn. CI/CD auto-deploys.
- **Coordination:** Always `git pull` first. Own your plan file only.
- **Infinite Autonomy:** Agents never idle. When a queue/pass completes, they loop and start the next one. Designed for month-long unattended operation.
