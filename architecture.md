# KiloApps Architecture

## Core Philosophy
Minimalism in size and compute efficiency is prioritized above all else. Every kilobyte and CPU cycle is accounted for. **Crucially, no individual kiloApp should exceed 999 kilobytes, even after polish and expansion.** (Note: The aggregated web platform at `kiloapps.web.app` and complete release `.zip` files are exempt from this limit and may exceed it as more apps are added).

## Repository Layout
- **Native Applications:** Subdirectories prefixed with 'K' (e.g., `KCalc`, `KWrite`, `KTetris`) contain minimal C/C++ or ASM based Windows applications.
- **Web OS Environment (`KiloOS`):** 
  - `src/`: React UI, CSS utilities, static assets.
  - `public/`: Static files and index.html shell.
  - `vite.config.js`: Drives the dev server and production build targeting `esnext`.
  - `pack_apps.bat`: Automates building the Vite project and running `crinkler` to produce the final executable.

## Build Process (Web Environments)
1. **Vite Build:** Outputs optimized assets to `dist/`.
2. **Concatenation:** All JS assets are combined into a single file (`kiloos.bundle.js`).
3. **Crinkler:** The bundle is aggressively compressed using XOR compression and dead-code elimination into a native Windows executable (`kiloos.exe`).
4. **Targets:** 
   - HTML bundle <= 50KB gzipped.
   - EXE size <= 150KB.
   - Load time < 500ms.
   - Startup time < 200ms.

## Design System
- **Palette:** Dark-mode base with HSL accent colors.
- **Typography:** Google Font "Inter".
- **Animations:** Lightweight CSS transitions (0.15s ease) and `@keyframes fadeIn`.
- **Layout:** Flexbox/Grid with max-width 800px for desktop. No external CSS frameworks are allowed.
