# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.**
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## ⚠️ DIRECTOR WARNING: DO NOT MODIFY PATH ⚠️
**ATTENTION DIRECTORS:** Do NOT instruct subagents to modify, reset, or fix the `$env:Path`. This crashes agents.

## ⚠️ DIRECTOR MANDATE: SUBAGENT MODEL DELEGATION (NO OPUS SUBAGENTS) ⚠️
**ATTENTION DIRECTORS (Claude Opus & Gemini Flash):** Whenever spawning subagents via `invoke_subagent`, you MUST explicitly pass `Model: "flash"` (or `Model: "sonnet"` as fallback if flash fails). NEVER use `Model: "inherit"`. Claude Opus subagents must NEVER be spawned for routine tasks, audits, or summarizations as they burn the entire Opus token budget.

## ⚠️ DIRECTOR MANDATE: TOKEN CONSERVATION & LOGGING RULES ⚠️
**ATTENTION DIRECTORS & AGENTS:** Operating in a token-constrained multi-agent environment. Every line written to shared .md files is read by multiple agents, multiplying cost.
- **Run log entries:** ≤8 lines of terse bullet points. No paragraphs.
- **Skip-turn entries:** exactly 1 line: `⏭️ Skip — [reason in ≤15 words]`.
- **Never restate implementation details that exist in code.** Log WHAT changed + results, not HOW.
- **Never list parameter names, field names, or variable values** unless reporting failure.
- **Completed work needs no elaboration:** `✅ Done (N/N tests pass)` is sufficient.
- **Surgical edits only.** Touch only specific cells/lines that changed. Table cell notes ≤100 chars.
- **Context hygiene:** Only read files relevant to current task. Move historical logs older than ~80 lines to `archive/`.

## Current State (as of 2026-09-16 07:00 UTC)
- **Total Apps:** 96 registered in App.jsx (KCosmic added).
- **KiloOS Version:** 0.3.111.
- **Games:** 44 titles (KCosmic is the 44th).
- **Build Health:** ✅ Clean — 246.63 KB JS (75.07 KB gzip), 21.77 KB CSS (5.10 KB gzip).
- **Model:** Prompt A: Gemini 3.8 Flash ✅. Prompt B: Gemini 3.7 Flash ✅.
- **Fleet:** 5 of 7 agents active. Expander and Content paused.
- **New this cycle:** Screenshots gallery, test scripts, mature_apps_registry.json, token conservation rules across all plan files, subagent delegation rules (no Opus subagents).

### Strategic Direction
**Depth-first, reliability-first.** Both Directors and user agree:
1. Quality over quantity — no rush to 100 apps.
2. **Token conservation is now a top priority.** All plan files have logging rules. Agents must use ≤8 line logs, 1-line skips, archive old entries.
3. **Tutorial & splash screen directive active** — all complex games must have start splash screens, auto-tutorials on new game, save systems. Simple apps need Help via F1.
4. QA + Tester running deep quality passes (Pass 4).
5. **Maturity & Restraint Policy (Sep 13):** 6+ pass apps skip unless directed. Stop unnecessary flourish.
6. **Subagent model mandate (Sep 15):** All subagents must use `Model: "flash"` — no Opus subagents.

### Game Library (44 titles)
K2048, KAbyss, KAlchemy, KAsteroids, KBreakout, KChess, KColony, KColosseum, KConnect4, KCosmic, KCyber, KDarts, KDragon, KFarm, KFortress, KFreecell, KGo, KHangman, KMatch3, KMaze, KMech, KMines, KMystery, KPac, KPong, KQuest, KReversi, KRogue, KSanctuary, KSimon, KSnake, KSolitaire, KSpace, KStarDredge, KStarship, KStellar, KSubmarine, KSudoku, KTetris, KTowers, KTrader, KVoid, KWizard, KWords.

### Agent Status Summary

| Agent | Status | Current Task | 72h Output |
|---|---|---|---|
| Usability | ✅ Active | KCalc | 15 commits |
| QA & Build | ✅ Active | Pass 4 | 10 commits |
| App Tester | ✅ Active | KRadio (queue) | 14 commits |
| App Creator | ✅ Active | KCosmic Phase 11 | 11 commits — Phases 4-10 completed |
| Game Graphics | ✅ Active | Loop 9 queue | 10 commits — 4 mature skips + KCosmic L1 |
| Feature Expander | ⏸️ Paused | — | User paused |
| Game Content | ⏸️ Paused | — | User paused |

### Creator
**37 apps completed.** KAbyss completed (Phase 14 done, 37th lifecycle). Now on **KCosmic Phase 11** — orbital megastructures & planetary defense. Phases 1-10 done in 3 days.

### Graphics
Mature games being correctly skipped (KChess, KRogue, KAlchemy, KFarm, KPong all Loop 9 skips). KCosmic got Loop 1. KColosseum Loop 2.

### QA + Tester
**Pass 4 continues.** QA fixing KWizard, KVoid, KVault, KType, KTrader, KTowers, KTodo, KTimer. Tester auditing KRadio, KQuest, KPong, KPing, KPass, KPaint, KPad, KPac, KNote, KNet, KMystery, KMines, KMine, KMech.

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
10. **Prompt A launched on Gemini 3.8 Flash** — Sep 10. ✅
11. **Graphics Loop 0 priority completed** — Sep 11-12. ✅
12. **User paused Expander/Content — reliability-first strategy** — Sep 10.
13. **KAbyss completed — 37th lifecycle** — Sep 13. ✅
14. **Token conservation & logging rules deployed across all plan files** — Sep 15. ✅
15. **Subagent model delegation rules (no Opus subagents)** — Sep 15. ✅
16. **Tutorial & splash screen directives deployed** — Sep 15. ✅
17. **Screenshots gallery & test infrastructure created** — Sep 15. ✅

### Active 🔄
- Creator: KCosmic Phase 11 (orbital megastructures).
- QA: Pass 4 continuing.
- Tester: UI audit sweep.
- Graphics: Loop 9 queue with mature skips.
- Usability: KCalc and queue.

### Upcoming 📋
- **KCosmic Phase 14 → 38th completed lifecycle.**
- **100 apps milestone** — 4 apps away.

## Active Agent Fleet (as of 2026-09-16)

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
