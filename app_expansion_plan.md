# App Expansion Plan

**Target App:** KChess
**Status:** In Progress
**Current Phase:** Phase 1

## Phases
- [x] **Phase 1 (Current Turn):** Review KChess's current implementation. Identify areas for adding depth. Draft a detailed feature expansion update (implementation_plan.md) for KChess.
- [ ] **Phase 2:** Update `kchess.html` (Web version) to introduce Move Validation. The current game allows any piece to move anywhere. Implement standard movement rules for Pawns, Knights, Bishops, Rooks, Queens, and Kings (including capturing, but excluding complex rules like En Passant or Check).
- [ ] **Phase 3:** Update `KChess/main.c` (Native version) to bring it up to parity. Implement the exact same move validation logic in C so that pieces can only move in their designated patterns.
- [ ] **Phase 4:** Perform testing and packaging. Verify the new features work within KiloOS and update `KApps.zip`. Create the final walkthrough artifact.
