# New App Plan: KTimer

**Status:** Not Started

## Description
KTimer is a minimal, bloat-free time management application featuring a Stopwatch and a Countdown Timer. It maintains the core KiloOS aesthetic and strict < 999KB constraints.

## Phases
- [x] **Phase 1:** Scaffold the `KTimer` application directory. Create `ktimer.html` with a tabbed UI (Stopwatch and Timer) and integrate the web app into `KiloOS/src/App.jsx`.
- [ ] **Phase 2:** Implement the Javascript logic for the Stopwatch feature (Start, Stop, Lap, Reset) with millisecond precision and a clean digital display.
- [ ] **Phase 3:** Implement the Javascript logic for the Countdown Timer (Hours/Minutes/Seconds input, Start, Pause, and an audio notification using the Web Audio API on completion).
- [ ] **Phase 4:** Create the native Windows wrapper (`main.c`, `app.rc`, `build.bat`) in the `KTimer` directory using native Win32 controls, and add it to `rebuild_all.bat`. Finally, commit and push to GitHub.
