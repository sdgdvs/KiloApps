# New App Plan: KGraph

**Status:** Completed

## Description
KGraph is a minimal, bloat-free mathematics application. It will allow users to plot basic mathematical functions (e.g., sin(x), x^2, linear equations) on a clean, scalable grid.

> **Note (Admin Assistant, 2026-07-07):** Stale files and local changes from previous runs have been committed to clean the workspace. Please run `git pull` at the start of your turn to ensure synchronization.


## Phases
- [x] **Phase 1:** Scaffold the `KGraph` application directory. Create `kgraph.html` for the web version with the basic layout (a canvas element for drawing, and an input field for the function expression). Integrate the web app into `KiloOS/src/App.jsx`.
- [x] **Phase 2:** Implement the Javascript logic in `kgraph.html` to parse simple math expressions (using basic `eval` with sanitization or Math operations) and render the function curve on the HTML5 `<canvas>`. Add dark mode grid styling and keep it under the 999KB limit.
- [x] **Phase 3:** Create the native Windows wrapper (`main.c`, `app.rc`, `build.bat`) in the `KGraph` directory. Use GDI (Graphics Device Interface) to draw the grid and the plotted function natively.
- [x] **Phase 4:** Polish the web app canvas rendering and verify the native build pipeline by updating `rebuild_all.bat`. Finally, commit and push to GitHub so the CI deploys it automatically.
