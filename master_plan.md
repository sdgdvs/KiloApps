# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications. **No individual kiloApp should exceed 999 kilobytes.** (The aggregated web platform at `kiloapps.web.app` and release `.zip` files are exempt.)
- Maintain a minimal web-based OS environment (`KiloOS`) — a React-based desktop shell deployed to Firebase Hosting at `kiloapps.web.app`.
- Keep dependencies and framework overhead to an absolute minimum.

## Current State (as of 2026-07-16)
- **Total Apps:** 58 native Windows applications in individual subdirectories.
- **KiloOS Version:** 0.3.56
- **Web Environment:** Deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** 6 folders: System, Media, Office, Games, Network, Dev.
- **Redundant apps deleted:** KWrite (merged into KPad), KDraw (merged into KPaint).

### New Apps Created with Deep Expansion (6, all through Phase 14)
KVault, KBudget, KHabit, KFlash (flashcards), K2048 (puzzle game), KSudoku (logic puzzle). KBBS also received a full 14-phase deep expansion.

### All Existing Apps Polished + Feature-Expanded
The Builder agent completed a full pass of all ~45 existing apps. It has transitioned to a **round-robin continuous improvement** model, cycling through every app adding one new feature per turn.

### Games: 12 games, Loop 3 in progress
KBreakout and K2048 created. Loops 1-2 complete. Loop 3: 9 of 12 done. Features getting genuinely deep (wall kicks, procedural generation, bullet-hell mechanics, AI improvements).

### QA: 27+ apps tested and fixed
Systematic alphabetical pass through KTerminal. Common bug patterns: **GDI resource leaks**, XSS vulnerabilities, logic bugs, memory issues.

### KiloOS Shell/UX: 15 improvements delivered
Window management (dimming, snap-to-edge previews, drag feedback, minimize animation), start menu (folder navigation, glassmorphic polish, animations), taskbar (scrolling, system tray), responsive design, theme consistency.

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
9. 3 new apps created through full 14-phase lifecycle — July 11-13.
10. Games Loop 1 complete + KBreakout created + Loop 2 nearly done — July 13.

### Active 🔄
11. **🎮 PRIORITY: Expand game library** — Creator agent pivoting to games-first after KFlash. Target: 19 new game titles with deep 14-phase expansion each.
12. Builder: Round-robin feature additions across all apps (perpetual).
13. QA: Systematic testing, currently at KPing (~33 apps remaining).
14. Games agent: Loop 2 completion, then Loop 3 with deeper content passes.
15. Shell/UX: Window management animation polish, general refinements.

### Upcoming 📋
16. Approach commercial-grade quality for key games (leverage 999KB budget fully).
17. Build pipeline improvements: automated size-limit checks, CI artifact upload.
18. Automated testing or build-verification for native apps.

## Active Agent Fleet (2 computers, 2 accounts)

| Agent | Computer | Schedule | Plan File | Model |
|---|---|---|---|---|
| App Builder | Other | Every 2h | `app_work_plan.md` | Gemini 3.1 Pro High |
| Quality & Build | Other | Every 3h | `app_fix_plan.md` | Gemini 3.1 Pro Low |
| Shell & UX | Other | Every 6h | `kiloos_ux_plan.md` | Gemini 3.1 Pro High |
| Game Content | Other | ~2h | `game_content_plan.md` | Gemini 3.1 Pro High |
| App Creator | This | Every 1h | `new_app_plan.md` | Gemini 3.1 Pro High |
| Director | This | Every 3 days | (reviews) | Claude Opus 4.6 |

## Agent Workflow Rules
- **Continuous Deployment:** Commit and push after every turn. CI/CD auto-deploys.
- **Coordination:** See `.agents/AGENTS.md`. Always `git pull` first. Own your plan file only.
- **Subagent Delegation:** Agents should dispatch subagents for coding work to keep orchestrator context clean.
- **Documentation:** Keep plan files concise and current.
