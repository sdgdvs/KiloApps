# App Expansion Plan

**Target App:** KMaze
**Status:** In Progress
**Current Phase:** Phase 1

## Phases
- [x] **Phase 1 (Current Turn):** Review KMaze's current implementation. Identify areas for adding depth. Draft a detailed feature expansion update (implementation_plan.md) for KMaze.
- [ ] **Phase 2:** Update `kmaze.html` (Web version) to introduce "Keys and Doors" mechanics. Add logic to collect keys (which removes them from the map) and open doors (which require a key). Create 2 entirely new levels that utilize this mechanic.
- [ ] **Phase 3:** Update `KMaze/main.c` (Native version) to bring it up to parity. Implement the same "Keys and Doors" mechanic and collision checks in the native C raycaster. Port the 2 new levels into the C arrays.
- [ ] **Phase 4:** Perform testing and packaging. Verify the new features work within KiloOS and update `KApps.zip`. Create the final walkthrough artifact.
