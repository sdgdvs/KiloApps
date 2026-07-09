# App Expansion Plan

**Target App:** KNote
**Status:** In Progress
**Current Phase:** Phase 1

## Phases
- [x] **Phase 1 (Current Turn):** Review KNote's current implementation. Identify areas for adding depth. Draft a detailed feature expansion update (implementation_plan.md) for KNote.
- [ ] **Phase 2:** Update `knote.html` (Web version) to introduce Multi-Note support. Add a sidebar with a list of notes, a "New Note" button, and the ability to switch between notes. Migrate the single string in localStorage to a structured JSON array of notes.
- [ ] **Phase 3:** Update `KNote/main.c` (Native version) to bring it up to parity. Add a Win32 `LISTBOX` to the left of the text area for selecting notes, along with "New" and "Delete" buttons. Update the file I/O to read and write multiple distinct notes into `knote_data.txt` using a delimited format.
- [ ] **Phase 4:** Perform testing and packaging. Verify the new features work within KiloOS and update `KApps.zip`. Create the final walkthrough artifact.
