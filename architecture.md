# KiloApps Architecture

## Core Philosophy
Minimalism in size and compute efficiency is prioritized above all else. Every kilobyte and CPU cycle is accounted for. **Crucially, no individual kiloApp should exceed 999 kilobytes, even after polish and expansion.** (Note: The aggregated web platform at `kiloapps.web.app` and complete release `.zip` files are exempt from this limit and may exceed it as more apps are added).

## Dual-Target App Model
Each KiloApp exists in two forms:
1. **Native Windows Application** — Standalone C/C++ or ASM executables using Win32 API. No runtime dependencies. Built via per-app `build.bat` scripts.
2. **Web (HTML5) Version** — A single `.html` file per app, served within KiloOS. Uses vanilla HTML/CSS/JS (no frameworks inside individual apps).

Both versions should offer functional parity where feasible, though the native version may have deeper OS integration.

## Repository Layout
- **Native Applications:** Subdirectories prefixed with 'K' (e.g., `KCalc`, `KPad`, `KTetris`) contain minimal C/C++ or ASM based Windows applications. Each has its own `build.bat` and source files.
- **Web OS Environment (`KiloOS/`):** 
  - `src/App.jsx` — Main KiloOS shell (window manager, taskbar, start menu, app registry).
  - `src/index.css` — Global stylesheet for KiloOS chrome.
  - `src/main.jsx` — React entry point.
  - `public/` — Static files: app HTML files under `public/apps/`, icons under `public/assets/icons/`.
  - `vite.config.js` — Drives the dev server and production build targeting `esnext`.
  - `firebase.json` / `.firebaserc` — Firebase Hosting configuration.
- **Build Tools:** `pack_apps.bat`, `rebuild_all.bat`, `fix_builds.py`, `build_telemetry.py` at repo root.
- **Agent Plans:** `master_plan.md`, `architecture.md`, `app_polish_plan.md`, `app_fix_plan.md`, `new_app_plan.md` at repo root.

## App Organization (Folders)
Apps in KiloOS are categorized into 6 folders for desktop and start menu organization:
- **System** — System tools (KSys, KTask, KClock, KTimer, KPass, KDB, etc.)
- **Media** — Media & arts (KAudio, KMedia, KSynth, KImage, KPaint, KFont, KColor, etc.)
- **Office** — Productivity (KPad, KNote, KCalc, KCalendar, KTodo, KChart, etc.)
- **Games** — Games (KChess, KMine, KMines, KPong, KSnake, KTetris, KSolitaire, KPac, KRogue, KSpace, KMaze, KMandel, etc.)
- **Network** — Network tools (KNet, KPing, KMail, KChat, KBBS, etc.)
- **Dev** — Development tools (KTerm, KHex, KScript, KType, KZip, etc.)

New apps must specify a `folder` property in the `APPS` array in `App.jsx`. See `KiloOS/app_integration_guide.md` for details.

## Build Process (Web Environments)
1. **Vite Build:** Outputs optimized assets to `dist/`.
2. **Concatenation:** All JS assets are combined into a single file.
3. **Targets:**
   - HTML bundle ≤ 50KB gzipped.
   - Load time < 500ms.
   - Startup time < 200ms.

## CI/CD Pipeline
- **Trigger:** Every push to `main` branch.
- **Workflow:** `.github/workflows/deploy.yml` runs on `ubuntu-latest`:
  1. Checkout repo → Setup Node 20 → `npm ci` in `KiloOS/` → `npm run build` → Deploy to Firebase Hosting.
- **Hosting:** Firebase project `kiloapps`, live channel, accessible at `kiloapps.web.app`.
- **Key Detail:** The Firebase `entryPoint` is `KiloOS` (not the repo root), matching where `firebase.json` lives.

## Design System
- **Palette:** Dark-mode base with HSL accent colors. Background: `hsl(210, 15%, 12%)`, Card: `hsl(210, 15%, 20%)`, Primary: `hsl(200, 85%, 60%)`, Secondary: `hsl(30, 85%, 60%)`.
- **Typography:** Google Font "Inter" (400/600 weights).
- **Animations:** Lightweight CSS transitions (0.15s ease) and `@keyframes fadeIn`. Glassmorphism only at `@media (prefers-reduced-motion: no-preference)`.
- **Layout:** Flexbox/Grid with max-width 800px for desktop. No external CSS frameworks are allowed.
- **Start Menu:** Two-column glassmorphic layout with user avatar and categorized app folders.
- **Taskbar:** Horizontal scroll with entrance animations and active-state indicators.

## ARG (Alternate Reality Game) Layer
The project includes a hidden narrative layer called "The Kilo Project Echoes." This is an intentional easter-egg system, not a bug. Key elements:
- **Arc 1 ("Echoes"):** Hidden JSON log (`.kilo-echo.json`), console easter egg (`window.__KILO_ECHO__`), invisible pixel trigger, custom HTTP header (`X-Kilo-Echo`), hidden archive.
- **Arc 2 ("The Rogue AI's Canvas"):** Subliminal textures, ghost audio, corrupted terminal glitch animation (triggered by opening KClock→KCalc→KTerm in sequence), AI manifesto endpoint, Quarantine app.
- All ARG elements are documented in `arg_plan.md`. Any agent adding hidden behavior must update that file.
