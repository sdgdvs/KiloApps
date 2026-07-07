# KiloApps Master Plan

## Vision
The project aims to return software development to the lightweight, compute-efficient philosophy of 1999. The goal is to produce extremely minimal, fast, and bloat-free applications, fighting against the excessive resource consumption of modern software. 

## High-Level Objectives
- Maintain a suite of standalone, minimal native Windows applications (KTimer, KWrite, KZip, etc.).
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
- **Continuous Deployment:** On the completion of each agent turn, you must commit and push all changes to GitHub. This is required because GitHub Actions is configured to automatically deploy the changes to `kiloapps.web.app`.
