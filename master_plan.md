# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications (KTimer, KWrite, KZip, etc.). **Crucially, no individual kiloApp should exceed 999 kilobytes, even after polish and expansion.** (Note: The aggregated web platform at `kiloapps.web.app` and complete release `.zip` files are exempt from this limit and may exceed it as more apps are added).
- Maintain a minimal web-based OS environment (`KiloOS`) that packages HTML5 apps into sub-150KB Windows executables using Crinkler.
- Keep dependencies and framework overhead to an absolute minimum.

## Current State (as of 2026-07-07)
- **Native Apps:** 50+ native Windows applications exist in individual subdirectories (KCalc, KChess, KDraw, KMail, KPong, KRogue, KSnake, KTerm, KWrite, KZip, etc.).
- **Web Environment:** `KiloOS` is implemented using React + Vite, deployed to Firebase Hosting at `kiloapps.web.app` via GitHub Actions CI/CD.
- **Desktop Organization:** Apps are categorized into 6 folders: System, Media, Office, Games, Network, Dev.
- **Recent Completions:**
  - KCalc: Full UI polish (glassmorphic dark mode, native theme update).
  - KConverter: Created from scratch (all 4 phases).
  - KBBS & KAudio: Polished.
  - KiloOS Desktop: Start menu redesigned (two-column glassmorphic layout), taskbar improved (horizontal scroll, animations).
  - CI/CD: Firebase deployment pipeline fixed (entryPoint correction).
  - ARG Layer: Two narrative arcs ("The Kilo Project Echoes", "The Rogue AI's Canvas") with 10 hidden components.
- **In Progress:**
  - KTodo: Phase 1 scaffolding complete, Phases 2-4 pending.
  - App Bug Fixing: KAudio targeted but not yet started.

## Milestones

### Completed ✅
1. Initial ecosystem with 50+ apps committed and KiloOS web shell deployed.
2. CI/CD pipeline established (GitHub Actions → Firebase Hosting).
3. Desktop folder organization system implemented.
4. First round of app polish (KCalc, KBBS, KAudio, KConverter).
5. KiloOS design overhaul (start menu, taskbar).

### Active 🔄
6. Complete KTodo (Phases 2-4: JS logic, native wrapper, packaging).
7. Begin systematic bug-fix pass starting with KAudio.
8. ARG Arc 3 planning ("The Operator's Intervention").

### Upcoming 📋
9. Systematic polish pass across all 50+ apps (oldest/roughest first).
10. Build pipeline improvements: automated size-limit checks, CI artifact upload.
11. Add new micro-apps based on community interest.
12. Establish automated testing or build-verification for native apps.

## Agent Workflow Rules
- **Continuous Deployment & Versioning:** On the completion of each agent turn, you must:
  1. Build/update the `kiloapps.web.app` assets and the `.zip` executable packages as applicable for your changes.
  2. Bump/update the `kiloapps.web.app` version number.
  3. Commit and push all changes to GitHub. This triggers the GitHub Actions pipeline to automatically deploy the changes to `kiloapps.web.app`.
- **Coordination:** See `.agents/AGENTS.md` for multi-agent conflict-avoidance rules. Always `git pull` before editing shared files.
- **Documentation:** Keep plan files (`app_polish_plan.md`, `app_fix_plan.md`, `new_app_plan.md`) updated with current status after each turn.
