# App Expansion Plan

**Target App:** KZip
**Status:** In Progress
**Current Phase:** Phase 1

## Phases
- [x] **Phase 1 (Current Turn):** Review KZip's current implementation. Identify areas for adding depth. Draft a detailed feature expansion update (implementation_plan.md) for KZip.
- [ ] **Phase 2:** Update `kzip.html` (Web version) to implement a functional virtual archiver. Replace the hardcoded dummy files with real Virtual File System (VFS) interactions, allowing users to pack and unpack real text/image files into a JSON-based virtual `.kza` (KiloZip Archive) format.
- [ ] **Phase 3:** Update `KZip/main.c` (Native version) to implement a real native archiver. We will rip out the dummy `MessageBoxA` logic and replace it with Win32 File I/O (`CreateFile`, `ReadFile`, `WriteFile`). It will physically pack/unpack files using a custom lightweight `.kza` format structure (filename length, filename, data length, data).
- [ ] **Phase 4:** Perform testing and packaging. Verify the new features work within KiloOS and update `KApps.zip`. Create the final walkthrough artifact.
