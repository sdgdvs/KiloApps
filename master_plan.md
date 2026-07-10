# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications (KTimer, KWrite, KZip, etc.). **Crucially, no individual kiloApp should exceed 999 kilobytes, even after polish and expansion.** (Note: The aggregated web platform at `kiloapps.web.app` and complete release `.zip` files are exempt from this limit and may exceed it as more apps are added).
- Maintain a minimal web-based OS environment (`KiloOS`) that packages HTML5 apps into sub-150KB Windows executables using Crinkler.
- Keep dependencies and framework overhead to an absolute minimum.

## Current State (as of 2026-07-10)
- **Native Apps:** 50+ native Windows applications in individual subdirectories.
- **Web Environment:** `KiloOS` deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** Apps categorized into 6 folders: System, Media, Office, Games, Network, Dev.

### Apps Polished (18)
KCalc, KCalendar, KChart, KChess, KChat, KBBS, KAudio, KClock, KPong, KDB, KDraw, KFont, KHex, KImage, KMail, KMandel, KPad, KPing

### Apps Feature-Expanded (18)
KPad, KImage, KMaze, KMine, KPac, KNote, KPass, KMedia, KWrite, KNet, KZip, KPaint, KClock, KChess, KDraw, KMail, KMandel, KPing

### Apps Created from Scratch (7)
KConverter, KTodo, KGraph, KTimer, KContacts, KRead, KBase

### Games Content-Improved (3)
KRogue (new enemies + Amulet), KSnake (high scores + progressive difficulty), KTetris (next piece + high scores + progressive speed)

### KiloOS Shell/UX Improvements
- Window dimming for inactive windows
- Start menu glassmorphism and hover polish
- System tray icons (network, volume, battery)
- Window transition animation refinements
- Mobile responsive design (media queries)

### QA Tested & Fixed (4)
KAudio (Web Audio envelope + GDI leak), KBBS (ANSI scroll + scrollbar + bounds), KCalc (M+/M- eval + keyboard), KCalendar (month-wrapping)

## Milestones

### Completed ✅
1. Initial ecosystem with 50+ apps committed and KiloOS web shell deployed.
2. CI/CD pipeline established (GitHub Actions → Firebase Hosting).
3. Desktop folder organization system implemented.
4. First round of app polish (KCalc, KBBS, KAudio, KConverter) — July 7.
5. KiloOS design overhaul (start menu, taskbar) — July 7.
6. Agent fleet optimization (7 agents → 4, admin bloat eliminated) — July 9.
7. Systematic polish/expansion pass: 18 apps polished, 18 expanded — July 10.

### Active 🔄
8. Continue polish/expansion pass: 12 apps remaining (KRogue → KType).
9. Continue QA testing pass: ~46 apps remaining after KCalendar.
10. Game content improvements: 7 of 10 games remaining in Loop 1.
11. KiloOS Shell/UX polish: start menu animation, taskbar refinement.

### Upcoming 📋
12. Build pipeline improvements: automated size-limit checks, CI artifact upload.
13. Establish automated testing or build-verification for native apps.
14. Consider new micro-apps once all existing apps are polished.

## Active Agent Fleet
| Agent | Schedule | Plan File | Model |
|---|---|---|---|
| App Builder | Every 2h | `app_work_plan.md` | Gemini 3.1 Pro High |
| Quality & Build | Every 3h | `app_fix_plan.md` | Gemini 3.1 Pro Low |
| Shell & UX | Every 6h | `kiloos_ux_plan.md` | Gemini 3.1 Pro High |
| Game Content | ??? | `game_content_plan.md` | ??? |
| Director | Every 3 days | (this review) | Claude Opus 4.6 |

## Agent Workflow Rules
- **Continuous Deployment & Versioning:** On the completion of each agent turn, you must:
  1. Build/update the `kiloapps.web.app` assets as applicable for your changes.
  2. Bump/update the `kiloapps.web.app` version number.
  3. Commit and push all changes to GitHub. This triggers the GitHub Actions pipeline to automatically deploy the changes to `kiloapps.web.app`.
- **Coordination:** See `.agents/AGENTS.md` for multi-agent conflict-avoidance rules. Always `git pull` before editing shared files.
- **Documentation:** Keep your own plan file updated with current status after each turn.
