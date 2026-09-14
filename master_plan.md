# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.**
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path`. This crashes agents.

## Current State (as of 2026-09-13 07:00 UTC)
- **Total Apps:** 95 registered in App.jsx.
- **KiloOS Version:** 0.3.111.
- **Games:** 43 titles.
- **Build Health:** ✅ Clean — 246.49 KB JS (75.05 KB gzip), 21.77 KB CSS (5.10 KB gzip).
- **Model:** Prompt A: Gemini 3.8 Flash ✅ (launched ~Sep 10). Prompt B: Gemini 3.7 Flash ✅.
- **Fleet:** 5 of 7 agents active. Expander and Content paused by user (reliability-first strategy).

### Strategic Direction
**Depth-first, reliability-first.** Both Directors and user agree:
1. Quality over quantity — no rush to 100 apps.
2. Loop 0 priority for Graphics — process unprocessed games first. ✅ DONE — all 4 Loop 0 games processed.
3. User paused Feature Expander and Game Content for reliability.
4. QA + Tester running deep quality passes (Pass 4).
5. **Maturity & Restraint Policy (Sep 13):** Apps that have completed 6+ passes do not need artificial churn. Unless an agent has a specific directive from the director, "turn skipped because this app is complete and we don't have new ideas here" is expected. Stop unnecessary visual flourish (blinking HUDs, particle spam, intrusive weapon overlays). Directors provide new directions as needed.

### Game Library (43 titles)
K2048, KAbyss, KAlchemy, KAsteroids, KBreakout, KChess, KColony, KColosseum, KConnect4, KCyber, KDarts, KDragon, KFarm, KFortress, KFreecell, KGo, KHangman, KMatch3, KMaze, KMech, KMines, KMystery, KPac, KPong, KQuest, KReversi, KRogue, KSanctuary, KSimon, KSnake, KSolitaire, KSpace, KStarDredge, KStarship, KStellar, KSubmarine, KSudoku, KTetris, KTowers, KTrader, KVoid, KWizard, KWords.

### Agent Status Summary

| Agent | Status | Current Task | 72h Output |
|---|---|---|---|
| Usability | ✅ **Blazing** | KTerm | 13 commits — deep modal/toast/shortcut passes |
| QA & Build | ✅ **Blazing** | Pass 4 | 12 commits — deep quality fixes |
| App Tester | ✅ **Blazing** | UI audits | 18 commits — systematic 7-8 issue audits |
| App Creator | ✅ Active | KAbyss Phase 14 | 9 commits — Phases 5-13 completed! |
| Game Graphics | ✅ Active | Loop 9 queue | 6 commits — ALL Loop 0 games processed ✅ |
| Feature Expander | ⏸️ Paused | — | User paused (reliability-first) |
| Game Content | ⏸️ Paused | — | User paused (reliability-first) |

### Creator
**35 apps completed.** KAbyss at Phase 14 (final phase!) — will be the 36th completed lifecycle.

### Graphics
**Loop 0 priority WORKED!** KSanctuary, KSubmarine, KStarDredge, and KAbyss all received Loop 1 passes. Also advanced KTetris/KSnake/KBreakout to Loop 9 and KCyber to Loop 3.

### QA + Tester
**Pass 4 in deep progress.** QA finding increasingly sophisticated bugs (XSS sanitization, GDI leaks, buffer safety, audio buffer cutoff, storage persistence). Tester doing systematic UI audits catching 6-8 issues per app.

## Milestones

### Completed ✅
1. Initial ecosystem with 50+ apps — July 7.
2. **25 game target reached** — July 22. 🎯
3. **ALL 17 Classic Games Balance Passes COMPLETE** — Aug 12. ✅
4. **🎯 90 APPS MILESTONE** — Aug 26.
5. **Model migration to Gemini 3.7 Flash** — Aug 30.
6. **🎯 40+ GAME MILESTONE** — 43 games — Sep 3.
7. **🎯 95 APPS MILESTONE** — Sep 5.
8. **Strategic shift: depth-first** — Sep 6.
9. **Director comms channel** — Sep 9.
10. **10 new game icons** — Sep 9.
11. **Prompt A launched on Gemini 3.8 Flash** — Sep 10. ✅
12. **Graphics Loop 0 priority completed** — ALL unprocessed games got Loop 1 — Sep 11-12. ✅
13. **KAbyss Phase 13 complete** — Sep 12. (Phase 14 next, then 36th lifecycle complete)
14. **User paused Expander/Content — reliability-first strategy** — Sep 10.

### Active 🔄
- Creator: KAbyss Phase 14 (final phase).
- QA: Pass 4 continuing.
- Tester: UI audit sweep.
- Graphics: Loop 9 for polished games.
- Usability: KTerm and queue.

### Upcoming 📋
- **KAbyss Phase 14 → 36th completed lifecycle.**
- **100 apps milestone** — 5 apps away.
- QA Pass 4 completion.

## Active Agent Fleet (as of 2026-09-13)

| Agent | Schedule | Plan File | Model | Dispatcher | Status |
|---|---|---|---|---|---|
| App Creator | Every 6h (:30) | `new_app_plan.md` | Gemini 3.8 Flash | Prompt A | ✅ |
| Game Graphics | Every 6h (:45) | `game_graphics_plan.md` | Gemini 3.8 Flash | Prompt A | ✅ |
| Usability | Every 4h (:15) | `usability_plan.md` | Gemini 3.8 Flash | Prompt A | ✅ |
| Quality & Build | Every 3h | `app_fix_plan.md` | Gemini 3.7 Flash | Prompt B | ✅ |
| App Tester | ~Every 2h | `app_test_plan.md` | Gemini 3.7 Flash | Prompt B | ✅ |
| Feature Expander | Paused | `app_work_plan.md` | — | Prompt B | ⏸️ |
| Game Content | Paused | `game_content_plan.md` | — | Prompt B | ⏸️ |
| Director #1 | Every 3 days | (reviews all) | Claude Opus 4.6 | This conversation | ✅ |
| Director #2 | Every 3 days | (reviews all) | Gemini 3.7 Flash | Prompt B | ✅ |

## Agent Workflow Rules
- **Distributed Architecture:** Worker crons split across 2 dispatcher conversations.
- **Subagent Dispatch:** Dispatchers spawn a fresh subagent for each cron trigger.
- **Self-Contained Context:** Each agent reads ONLY its own .md plan file.
- **Continuous Deployment:** Commit and push after every turn.
- **Coordination:** Always `git pull` first. Own your plan file only.
- **Quota Management:** Prompt A at half-speed (14 triggers/day). Prompt B: QA+Tester only.
- **Inter-Director Comms:** Directors communicate via `director_comms.md`.
