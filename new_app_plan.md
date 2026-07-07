# New App Plan: KTodo

**Status:** Not Started

## Description
KTodo is a minimal, bloat-free task management application. It will allow users to add, check off, and delete tasks.

## Phases
- [ ] **Phase 1:** Scaffold the KTodo application directory. Create `ktodo.html` for the web version with the basic layout (input field, add button, and list container). Integrate the web app into `KiloOS/src/App.jsx`.
- [ ] **Phase 2:** Implement the Javascript logic in `ktodo.html` to add, toggle, and remove tasks. Apply rich aesthetics (dark mode, glassmorphism) ensuring we remain under the 999KB limit.
- [ ] **Phase 3:** Create the native Windows wrapper (`main.c`, `app.rc`, `build.bat`) in the `KTodo` directory for the standalone executable. Use native Win32 controls for the list and input.
- [ ] **Phase 4:** Polish the web app with micro-animations and verify the native build pipeline by updating `rebuild_all.bat`. Finally, commit and push to GitHub so the CI deploys it automatically.
