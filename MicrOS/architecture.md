# Architecture Overview for MicrOS

## High‑Level Components

1. **Source (`src/`)** – Contains the React UI, CSS utilities, and any static assets. All JavaScript is written as ES modules (type: module) and uses Vite for fast development and bundling.
2. **Vite Configuration (`vite.config.js`)** – Drives the dev server, hot‑module replacement, and production build. The build target is **esnext** to keep the bundle size minimal.
3. **Public Assets (`public/`)** – Holds `index.html` and any static files that need to be copied verbatim to the build output.
4. **Packaging (`crinkler/`)** – After Vite emits a tiny `dist/` folder, we invoke **crinkler** to compress the JavaScript bundle into a single **EXE**. Crinkler performs aggressive dead‑code elimination and XOR compression, yielding sub‑150 KB binaries.
5. **Distribution (`dist/`)** – Contains two artifacts after a successful build:
   - `micros.html` – A fully‑self‑contained HTML5 app that can be hosted anywhere.
   - `micros.exe` – A native Windows executable produced by crinkler.

## Data Flow
```
src/ → Vite (dev/dev server) → Vite build → dist/ (HTML+JS) → crinkler → dist/micros.exe
```

The **MicrOS** runtime loads the bundled JavaScript directly from the HTML file, so no external dependencies are required at runtime.

## Token‑Efficient Naming Conventions
- Directories are short, lower‑case, and **kebab‑case** (e.g., `micro‑calc`).
- Files that are frequently referenced in code use concise names (`main.jsx`, `app.css`).
- Avoid deep nesting – keep the hierarchy to three levels max to minimize token overhead for future agents.

---
*Performance Metrics*
- Target HTML bundle size: **≤ 50 KB** (gzip). 
- Target EXE size after crinkler: **≤ 150 KB**. 
- Load time on typical broadband: **< 500 ms**. 
- Startup time for the EXE: **< 200 ms**.
