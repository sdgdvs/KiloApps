# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.**
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path`. This crashes agents.

## Current State (as of 2026-09-04 07:00 UTC)
- **Total Apps:** 93 registered in App.jsx (+2 since Sep 1).
- **KiloOS Version:** 0.3.107 (+3 since Sep 1).
- **Games:** 41 titles. 🎯 **40 GAME MILESTONE REACHED!**
- **Build Health:** ✅ Clean — 243.27 KB JS (74.52 KB gzip), 21.50 KB CSS (5.04 KB gzip).
- **Model:** All 6 worker agents on **Gemini 3.7 Flash** (since Aug 30).
- **Fleet:** ✅ **ALL 6 AGENTS ACTIVE** — producing massive output on 3.7 Flash.

### Game Library (41 titles)
K2048, KAlchemy, KAsteroids, KBreakout, KChess, KColony, KColosseum, KConnect4, KCyber, KDarts, KDragon, KFarm, KFortress, KFreecell, KGo, KHangman, KMatch3, KMaze, KMech, KMines, KMystery, KPac, KPong, KQuest, KReversi, KRogue, KSanctuary, KSimon, KSnake, KSolitaire, KSpace, KStarship, KStellar, KSubmarine, KSudoku, KTetris, KTowers, KTrader, KVoid, KWizard, KWords.

### Agent Status Summary

| Agent | Status | Current Task | 72h Output |
|---|---|---|---|
| App Creator | ✅ **Blazing** | KSubmarine Phase 12 | KStellar completed, KSanctuary completed (14 phases), KSubmarine Phases 1-11 (~23 commits) |
| Game Graphics | ✅ **Blazing** | Loop 8 queue | ~19 commits — Loop 7-8 across 19 games |
| Usability | ✅ **Blazing** | KChess | ~19 commits — deep usability passes with modals, toasts, keyboard shortcuts |
| Feature Expander | ✅ Active | KConnect4 | Processing queue |
| QA & Build | ✅ Active | KContacts (Pass 3) | Processing queue |
| Game Content | ✅ Active | Deep games | Processing queue |

### Content Depth
- **Deep Games:** KRogue at Loop 11. Multiple games at Loop 10.
- **Classic Games: ALL 17 Balance Passes COMPLETE** ✅

### Graphics Agent
**Loop 8 in progress!** Advanced from Loop 7 to Loop 8 across most games. Incredible detail level — each game gets multi-layered particle engines, dual-tier shockwaves, ornate filigree HUD brackets, specular sheen sweeps, and atmospheric motes.

### Creator
**33 apps completed through full 14-phase lifecycle!** (+2 since Sep 1: KStellar, KSanctuary). Currently building **KSubmarine** (Deep-sea underwater exploration & abyssal survival RPG) — Phase 12 next.

### Usability Agent
Deep passes with cybernetic Help modals, non-blocking toast notifications, keyboard shortcut badges, and modal backdrop dismissal. Processing KChess. Progress log at 218 lines — **needs trimming**.

## Milestones

### Completed ✅
1. Initial ecosystem with 50+ apps — July 7.
2. CI/CD pipeline (GitHub Actions → Firebase) — July 7.
3. **25 game target reached** — July 22. 🎯
4. **ALL 17 Classic Games Balance Passes COMPLETE** — Aug 12. ✅ 🎯
5. **80+ apps milestone reached** — Aug 1. 🎯
6. **85 apps milestone reached** — Aug 13. 🎯
7. **🎯 90 APPS MILESTONE REACHED** — 91 apps — Aug 26. 🎯
8. **Model migration to Gemini 3.7 Flash** — Aug 30. ✅
9. **ALL 6 WORKER AGENTS ALIVE** — Aug 30. ✅ 🎯
10. **KRogue reaches Loop 11** — Aug 31. ✅
11. **7 deep games reach Loop 10** — Aug 31. ✅
12. **KStellar completed (all 14 phases)** — Sep 1. ✅
13. **KSanctuary completed (all 14 phases)** — Sep 2-3. ✅
14. **33 apps created through full lifecycle** — Sep 3. ✅
15. **🎯 40 GAME MILESTONE REACHED** — 41 games — Sep 3. 🎯
16. **Graphics Loop 8 in progress** — Sep 2-3. ✅
17. **KiloOS v0.3.107** — Sep 3. ✅
18. **Creator completes 2 full app lifecycles in 48h** — KSanctuary + KSubmarine started — Sep 2-3. ✅

### Active 🔄
- Creator: KSubmarine Phase 12.
- Graphics: Loop 8 in progress.
- Usability: KChess — deep modal/toast/shortcut passes.
- QA: Pass 3, target KContacts.
- Feature Expander: Processing KConnect4.
- Game Content: Deep games Loop 10-11.

### Upcoming 📋
- **95 apps milestone** — 2 apps away.
- KSubmarine Phase 12-14 completion.
- QA Pass 3 completion.
- Graphics Loop 8 completion.

## Active Agent Fleet (as of 2026-09-04)

| Agent | Schedule | Plan File | Model | Status |
|---|---|---|---|---|
| App Creator | Every 2h (:15) | `new_app_plan.md` | Gemini 3.7 Flash | ✅ Blazing |
| Game Graphics | Every 2h (:45) | `game_graphics_plan.md` | Gemini 3.7 Flash | ✅ Blazing |
| Usability Agent | Every 2h (:15 odd) | `usability_plan.md` | Gemini 3.7 Flash | ✅ Blazing |
| Feature Expander | Every 2h (:00) | `app_work_plan.md` | Gemini 3.7 Flash | ✅ Active |
| Quality & Build | Every 3h (:00) | `app_fix_plan.md` | Gemini 3.7 Flash | ✅ Active |
| Game Content | Every 2h (:30) | `game_content_plan.md` | Gemini 3.7 Flash | ✅ Active |
| Director #1 | Every 3 days | (reviews all) | Claude Opus 4.6 | ✅ Active |
| Director #2 | Every 3 days | (reviews all) | Gemini 3.7 Flash | ✅ Active |

## Agent Workflow Rules
- **Distributed Architecture:** Worker crons split across 2 dispatcher conversations.
- **Subagent Dispatch:** Dispatchers spawn a fresh subagent for each cron trigger.
- **Self-Contained Context:** Each agent reads ONLY its own .md plan file.
- **Continuous Deployment:** Commit and push after every turn.
- **Coordination:** Always `git pull` first. Own your plan file only.
- **Infinite Autonomy:** Agents never idle. When a queue/pass completes, they loop.
