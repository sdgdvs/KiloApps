# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications (KTimer, KWrite, KZip, etc.). **Crucially, no individual kiloApp should exceed 999 kilobytes, even after polish and expansion.** (Note: The aggregated web platform at `kiloapps.web.app` and complete release `.zip` files are exempt from this limit and may exceed it as more apps are added).
- Maintain a minimal web-based OS environment (`KiloOS`) that packages HTML5 apps into sub-150KB Windows executables using Crinkler.
- Keep dependencies and framework overhead to an absolute minimum.

## Current State
- **Native Apps:** Over 40 native Windows applications exist in individual subdirectories.
- **Web Environment:** `KiloOS` is implemented using React + Vite. It uses ES modules and compiles into standalone HTML and executable binaries.

## Future Milestones
1. Ensure all native applications build flawlessly without heavy dependencies.
2. Refine the UI/UX of `MicrOS` and `KiloOS` while adhering to the strict <=50KB HTML and <=150KB EXE constraints.
3. Add further integrations or new micro-apps based on the template guidelines.

## Agent Workflow Rules
- **Continuous Deployment & Versioning:** On the completion of each agent turn, you must:
  1. Build/update the `kiloapps.web.app` assets and the `.zip` executable packages as applicable for your changes.
  2. Bump/update the `kiloapps.web.app` version number.
  3. Commit and push all changes to GitHub. This triggers the GitHub Actions pipeline to automatically deploy the changes to `kiloapps.web.app`.
