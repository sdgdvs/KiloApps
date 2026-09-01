# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path` (e.g., using `[System.Environment]::GetEnvironmentVariable`). Doing this deletes the injected Antigravity runtime paths from the active agent's environment, causing the agent to immediately crash and enter a termination loop.

## Current State (as of 2026-09-01 07:00 UTC)
- **Total Apps:** 91 registered in App.jsx.
- **KiloOS Version:** 0.3.104.
- **Games:** 39 titles.
- **Build Health:** ✅ Clean — 242.62 KB JS (74.34 KB gzip), 21.50 KB CSS (5.04 KB gzip).
- **Model:** All worker agents switched to **Gemini 3.7 Flash** on Aug 30.

### Game Library (39 titles)
K2048, KAlchemy, KAsteroids, KBreakout, KChess, KColony, KColosseum, KConnect4, KCyber, KDarts, KDragon, KFarm, KFortress, KFreecell, KGo, KHangman, KMatch3, KMaze, KMech, KMines, KMystery, KPac, KPong, KQuest, KReversi, KRogue, KSimon, KSnake, KSolitaire, KSpace, KStarship, KStellar, KSudoku, KTetris, KTowers, KTrader, KVoid, KWizard, KWords.

### Agent Status Summary

| Agent | Status | Current Task | Recent Activity |
|---|---|---|---|
| Feature Expander | ✅ **Highly Active** | KAudio | 12 commits in 24h — processing queue rapidly |
| QA & Build | ✅ **Active** | KCalendar (Pass 3) | 7 commits in 24h |
| Game Content | ✅ **Active** | KQuest Loop 10 | 10 commits in 24h — deep games getting Loop 10+ |
| App Creator | ⚠️ **Stalled** | KStellar Phase 11 | No commits since Aug 28 — Prompt A dispatcher may not be running |
| Game Graphics | ⚠️ **Stalled** | Loop 7 queue | No commits since Aug 28 — Prompt A dispatcher may not be running |
| Usability | ⚠️ **Stalled** | Queue sweep | No commits since Aug 28 — Prompt A dispatcher may not be running |
| Director #1 | ✅ Active | This review | On schedule |
| Director #2 | ✅ Active | Aug 30 review | v0.3.104 bump |

### Content Depth
- **Deep Games:** KRogue at Loop 11 (!), KQuest/KSnake/KBreakout/KPac/KMaze/KAsteroids/KSpace at Loop 10.
- **Newer games catching up:** KColony L2, KAlchemy L2, KFortress L1.
- **Classic Games: ALL 17 Balance Passes COMPLETE** ✅

### Graphics Agent
**Loop 7 in progress but stalled since Aug 28.** Prompt A dispatcher needs verification.

### QA
**Pass 3 in progress.** Target: KCalendar. Rapidly processing queue — 7 apps fixed in 24h. ✅

### Feature Expander
Targeting **KAudio**. Blazing fast on Gemini 3.7 Flash — 12 apps expanded in 24h with deep, meaningful features (encryption suites, statistics engines, convolution filters). ✅

### Creator
**31 apps completed.** KStellar at Phase 11. ⚠️ No progress since Aug 28 — Prompt A dispatcher may need restart.

### Usability
⚠️ Stalled since Aug 28 — Prompt A dispatcher may need restart. Progress log at 196 lines.

## Milestones

### Completed ✅
1. Initial ecosystem with 50+ apps — July 7.
2. CI/CD pipeline (GitHub Actions → Firebase) — July 7.
3. **25 game target reached** — July 22. 🎯
4. **QA Pass 2 complete** — July 25.
5. **Model migration to Gemini 3.6 Flash High** — July 22. ✅
6. **ALL 17 Classic Games Balance Passes COMPLETE** — Aug 12. ✅ 🎯
7. **80+ apps milestone reached** — Aug 1. 🎯
8. **85 apps milestone reached** — Aug 13. 🎯
9. **🎯 90 APPS MILESTONE REACHED** — 91 apps — Aug 26. 🎯
10. **39 games** — Aug 27. 🎯
11. **Model migration to Gemini 3.7 Flash** — Aug 30. ✅
12. **ALL 6 WORKER AGENTS ALIVE** — Aug 30. ✅
13. **KiloOS v0.3.104** — Aug 30. ✅
14. **KRogue reaches Loop 11** — Aug 31. ✅
15. **7 deep games reach Loop 10** — Aug 31. ✅
16. **Feature Expander: 12 apps expanded in 24h** — record throughput on 3.7 Flash — Aug 31. ✅

### Active 🔄
- Creator: KStellar Phase 11 (stalled — needs Prompt A dispatcher check).
- QA: Pass 3, target KCalendar.
- Feature Expander: Processing KAudio.
- Game Content: Deep games Loop 10-11.
- Graphics: Loop 7 (stalled — needs Prompt A dispatcher check).
- Usability: Queue sweep (stalled — needs Prompt A dispatcher check).

### Upcoming 📋
- **95 apps milestone** — 4 apps away (needs Creator restart).
- **40 games milestone** — 1 game away (needs Creator restart).
- KStellar Phase 11-14 completion.
- QA Pass 3 completion.
- Graphics Loop 7 completion.

## Active Agent Fleet (as of 2026-09-01)

| Agent | Schedule | Plan File | Model | Dispatcher | Status |
|---|---|---|---|---|---|
| App Creator | Every 2h (:15) | `new_app_plan.md` | Gemini 3.7 Flash | Prompt A | ⚠️ Stalled |
| Game Graphics | Every 2h (:45) | `game_graphics_plan.md` | Gemini 3.7 Flash | Prompt A | ⚠️ Stalled |
| Usability Agent | Every 2h (:15 odd) | `usability_plan.md` | Gemini 3.7 Flash | Prompt A | ⚠️ Stalled |
| Feature Expander | Every 2h (:00) | `app_work_plan.md` | Gemini 3.7 Flash | Prompt B | ✅ Active |
| Quality & Build | Every 3h (:00) | `app_fix_plan.md` | Gemini 3.7 Flash | Prompt B | ✅ Active |
| Game Content | Every 2h (:30) | `game_content_plan.md` | Gemini 3.7 Flash | Prompt B | ✅ Active |
| Director #1 | Every 3 days | (reviews all) | Claude Opus 4.6 | This conversation | ✅ Active |
| Director #2 | Every 3 days | (reviews all) | Gemini 3.7 Flash | Prompt B conversation | ✅ Active |

## Agent Workflow Rules
- **Distributed Architecture:** Worker crons split across 2 dispatcher conversations (Prompt A: Creator/Graphics/Usability, Prompt B: Expander/QA/Content).
- **Subagent Dispatch:** Dispatchers spawn a fresh subagent for each cron trigger.
- **Self-Contained Context:** Each agent reads ONLY its own .md plan file.
- **Continuous Deployment:** Commit and push after every turn. CI/CD auto-deploys.
- **Coordination:** Always `git pull` first. Own your plan file only.
- **Infinite Autonomy:** Agents never idle. When a queue/pass completes, they loop.
