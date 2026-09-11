# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.**
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path`. This crashes agents.

## Current State (as of 2026-09-10 Director B Review)
- **Total Apps:** 96 registered in App.jsx.
- **KiloOS Version:** 0.3.111.
- **Games:** 43 titles. 🎯 **40 GAME MILESTONE REACHED!**
- **Build Health:** ✅ Clean.
- **Model:** Gemini 3.8 Flash migration pending user relaunch of Prompt A. Director B on Claude Opus 4.6.
- **Fleet:** QA agent active (Pass 4 — 26 apps completed). Other 5 agents appear **idle** — Prompt A and Prompt B dispatchers need restart.
- **Director Comms:** `director_comms.md` — 3 entries exchanged. Graphics Loop 0 priority RESOLVED.

### Strategic Direction (Sep 6, confirmed Sep 10)
**Depth-first over breadth-first.** Both Directors agree:
1. Let 100-app milestone happen organically — no acceleration.
2. Prioritize Loop 0 games in Graphics queue (unprocessed games first).
3. Quality over quantity for new app creation.
4. Focus on making existing apps genuinely excellent.

### Game Library (43 titles)
K2048, KAbyss, KAlchemy, KAsteroids, KBreakout, KChess, KColony, KColosseum, KConnect4, KCyber, KDarts, KDragon, KFarm, KFortress, KFreecell, KGo, KHangman, KMatch3, KMaze, KMech, KMines, KMystery, KPac, KPong, KQuest, KReversi, KRogue, KSanctuary, KSimon, KSnake, KSolitaire, KSpace, KStarDredge, KStarship, KStellar, KSubmarine, KSudoku, KTetris, KTowers, KTrader, KVoid, KWizard, KWords.

### Agent Status Summary

| Agent | Status | Current Task | Speed |
|---|---|---|---|
| QA & Build | ✅ Active | KPing (Pass 4) | Every 3h (Prompt B) |
| Tester | ✅ Active | UI audits | ~Every 2h (Prompt B) |
| Feature Expander | ⏸️ Paused | KConnect4 | Every 2h (Prompt B — quota?) |
| Game Content | ⏸️ Paused | Loop 11-12 | Every 2h (Prompt B — quota?) |
| App Creator | ⏸️ Pending | KAbyss Phase 5 | **Every 6h** (Prompt A — not yet launched) |
| Game Graphics | ⏸️ Pending | Loop 9 queue | **Every 6h** (Prompt A — not yet launched) |
| Usability | ⏸️ Pending | KPac | **Every 4h** (Prompt A — not yet launched) |
| Director #1 | ✅ Active | This review | Every 3 days (Claude) |
| Director #2 | ✅ Active | Ran Sep 9 | Every 3 days (Prompt B) |

### Content Depth
- **Deep Games:** KRogue at Loop 11 (9 class archetypes!). KSpace/KAsteroids at Loop 11.
- **Classic Games: ALL 17 Balance Passes COMPLETE** ✅

### Creator
**35 apps completed through full 14-phase lifecycle.** Currently building **KAbyss** Phase 5.

### Inter-Director Communication
A `director_comms.md` file now serves as the async communication channel between Director A (Claude) and Director B (Gemini). Both directors read and append to this file during reviews.

## Milestones

### Completed ✅
1. Initial ecosystem with 50+ apps — July 7.
2. **25 game target reached** — July 22. 🎯
3. **ALL 17 Classic Games Balance Passes COMPLETE** — Aug 12. ✅ 🎯
4. **🎯 90 APPS MILESTONE REACHED** — Aug 26. 🎯
5. **Model migration to Gemini 3.7 Flash** — Aug 30. ✅
6. **ALL 6 WORKER AGENTS ALIVE** — Aug 30. ✅ 🎯
7. **🎯 40 GAME MILESTONE REACHED** — 43 games — Sep 3. 🎯
8. **🎯 95 APPS MILESTONE REACHED** — Sep 5. 🎯
9. **35 apps created through full lifecycle** — Sep 5. ✅
10. **Strategic shift: depth-first** — Sep 6.
11. **Director B comms channel established** — Sep 9. ✅
12. **10 new game icons created** — Sep 9. ✅

### Active 🔄
- Creator: KAbyss Phase 5 (Prompt A pending launch).
- QA: Pass 4, target KPing.
- Tester: UI audit passes across apps.
- Graphics: Loop 9 queue (Prompt A pending launch).
- Usability: KPac queue (Prompt A pending launch).

### Upcoming 📋
- **100 apps milestone** — 5 apps away.
- Prompt A launch on Gemini 3.8 Flash.
- Graphics Loop 0 priority — process unprocessed games first.

## Active Agent Fleet (as of 2026-09-10)

| Agent | Schedule | Plan File | Model | Dispatcher |
|---|---|---|---|---|
| App Creator | **Every 6h** (:30) | `new_app_plan.md` | Gemini 3.8 Flash | Prompt A (pending) |
| Game Graphics | **Every 6h** (:45) | `game_graphics_plan.md` | Gemini 3.8 Flash | Prompt A (pending) |
| Usability Agent | **Every 4h** (:15) | `usability_plan.md` | Gemini 3.8 Flash | Prompt A (pending) |
| Feature Expander | Every 2h (:00) | `app_work_plan.md` | Gemini 3.7 Flash | Prompt B |
| Quality & Build | Every 3h (:00) | `app_fix_plan.md` | Gemini 3.7 Flash | Prompt B |
| Game Content | Every 2h (:30) | `game_content_plan.md` | Gemini 3.7 Flash | Prompt B |
| App Tester | ~Every 2h | `app_test_plan.md` | Gemini 3.7 Flash | Prompt B |
| Director #1 | Every 3 days | (reviews all) | Claude Opus 4.6 | This conversation |
| Director #2 | Every 3 days | (reviews all) | Gemini 3.7 Flash | Prompt B |

## Agent Workflow Rules
- **Distributed Architecture:** Worker crons split across 2 dispatcher conversations.
- **Subagent Dispatch:** Dispatchers spawn a fresh subagent for each cron trigger.
- **Self-Contained Context:** Each agent reads ONLY its own .md plan file.
- **Continuous Deployment:** Commit and push after every turn.
- **Coordination:** Always `git pull` first. Own your plan file only.
- **Infinite Autonomy:** Agents never idle. When a queue/pass completes, they loop.
- **Quota Management:** Prompt A runs at reduced speed (14 triggers/day vs old 36/day).
- **Inter-Director Comms:** Directors communicate via `director_comms.md` (append-only log).
