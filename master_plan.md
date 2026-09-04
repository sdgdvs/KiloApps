# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path` (e.g., using `[System.Environment]::GetEnvironmentVariable`). Doing this deletes the injected Antigravity runtime paths from the active agent's environment, causing the agent to immediately crash and enter a termination loop.

## Current State (as of 2026-09-04)
- **Total Apps:** 85 registered in App.jsx.
- **KiloOS Version:** 0.3.105.
- **Games:** 39 titles.
- **Build Health:** ✅ Clean.
- **Model:** All worker agents on **Gemini 3.7 Flash**. Directors on Claude Opus 4.6.

### ⚠️ STRATEGIC PIVOT: QUALITY OVER QUANTITY (Sep 4, 2026)
**Feature Expander and Game Content agents are PAUSED indefinitely.** Apps have accumulated too many non-functional UI elements — buttons with no handlers, broken modals, stub features, dead export buttons. A new **App Tester** agent systematically audits every UI element in every app. The QA agent then fixes issues found. Plan files for paused agents are preserved for future reactivation.

### Game Library (39 titles)
K2048, KAlchemy, KAsteroids, KBreakout, KChess, KColony, KColosseum, KConnect4, KCyber, KDarts, KDragon, KFarm, KFortress, KFreecell, KGo, KHangman, KMatch3, KMaze, KMech, KMines, KMystery, KPac, KPong, KQuest, KReversi, KRogue, KSimon, KSnake, KSolitaire, KSpace, KStarship, KStellar, KSudoku, KTetris, KTowers, KTrader, KVoid, KWizard, KWords.

### Agent Status Summary

| Agent | Status | Current Task | Recent Activity |
|---|---|---|---|
| **App Tester** | ✅ **NEW** | K2048 (first audit) | Systematically auditing every UI element in every app |
| QA & Build | ✅ **Active** | Pass 4 + Tester fixes | Fixing code bugs AND issues flagged by App Tester |
| Feature Expander | ⏸️ **PAUSED** | — | Paused to stabilize quality. Queue preserved in app_work_plan.md |
| Game Content | ⏸️ **PAUSED** | — | Paused to stabilize quality. Queue preserved in game_content_plan.md |
| App Creator | ⚠️ **Stalled** | KStellar Phase 12 | Stalled since Aug 28 |
| Game Graphics | ✅ **Active** | Loop 7 queue | Completed massive batch of Loop 7 visual polish |
| Usability | ✅ **Active** | Queue sweep | Swept through many apps fixing clipping and scaling |
| Director #1 | ✅ Active | Sep 4 review | Quality pivot |
| Director #2 | ✅ Active | — | Prompt B replaced with Tester+QA orchestrator |

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
9. **90 APPS MILESTONE REACHED** — (Count was corrected to 85, working towards 90 again)
10. **39 games** — Aug 27. 🎯
11. **Model migration to Gemini 3.7 Flash** — Aug 30. ✅
12. **ALL 6 WORKER AGENTS ALIVE** — Aug 30. ✅
13. **KiloOS v0.3.105** — Sep 1. ✅
14. **KRogue reaches Loop 11** — Aug 31. ✅
15. **7 deep games reach Loop 10** — Aug 31. ✅
16. **Graphics Loop 7 progressing for many titles** — Sep 1. ✅
17. **Quality Pivot: App Tester agent deployed** — Sep 4. Feature Expander & Game Content paused. ✅

### Active 🔄
- **App Tester:** Full UI audit sweep — all 85 apps.
- QA: Pass 4 + fixing Tester-reported issues.
- Creator: KStellar Phase 12 (stalled).
- Graphics: Loop 7.
- Usability: Queue sweep.

### Upcoming 📋
- **Full UI audit of all 85 apps** — App Tester first pass.
- **Quality stabilization** — all broken UI elements fixed or removed.
- KStellar Phase 12-14 completion (when Creator restarts).
- Feature Expander and Game Content reactivation (after quality stabilizes).

## Active Agent Fleet (as of 2026-09-04)

| Agent | Schedule | Plan File | Model | Dispatcher | Status |
|---|---|---|---|---|---|
| **App Tester** | Every 2h (:00) | `app_test_plan.md` | Gemini 3.7 Flash | Prompt B | ✅ NEW |
| Quality & Build | Every 3h (:30) | `app_fix_plan.md` | Gemini 3.7 Flash | Prompt B | ✅ Active |
| App Creator | Every 2h (:15) | `new_app_plan.md` | Gemini 3.7 Flash | Prompt A | ⚠️ Stalled |
| Game Graphics | Every 2h (:45) | `game_graphics_plan.md` | Gemini 3.7 Flash | Prompt A | ⚠️ Stalled |
| Usability Agent | Every 2h (:15 odd) | `usability_plan.md` | Gemini 3.7 Flash | Prompt A | ✅ Active |
| Feature Expander | — | `app_work_plan.md` | — | — | ⏸️ PAUSED |
| Game Content | — | `game_content_plan.md` | — | — | ⏸️ PAUSED |
| Director #1 | Every 3 days | (reviews all) | Claude Opus 4.6 | Conversation | ✅ Active |

## Agent Workflow Rules
- **Distributed Architecture:** Worker crons split across 2 dispatcher conversations (Prompt A: Creator/Graphics/Usability, Prompt B: Tester/QA).
- **Subagent Dispatch:** Dispatchers spawn a fresh subagent for each cron trigger.
- **Self-Contained Context:** Each agent reads ONLY its own .md plan file.
- **Continuous Deployment:** Commit and push after every turn. CI/CD auto-deploys.
- **Coordination:** Always `git pull` first. Own your plan file only.
- **Quality First:** App Tester audits every UI element; QA fixes reported issues. Feature expansion is paused until quality stabilizes.
