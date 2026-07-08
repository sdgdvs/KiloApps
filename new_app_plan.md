# New App Plan: KRead

**Status:** Not Started

## Description
KRead is a minimalist, distraction-free text reader. It allows users to load text documents and read them with configurable font sizes, automatically persisting their reading progress (scroll position) into the VFS, all while keeping under the < 999KB KiloOS limit.

## Phases
- [x] **Phase 1:** Scaffold the `KRead` application directory. Create `kread.html` with a clean typography-focused UI (file loader, reading pane, top toolbar) and integrate the web app into `KiloOS/src/App.jsx`.
- [ ] **Phase 2:** Implement the Javascript logic to load local text files via a file picker or drag-and-drop, parsing the text into the reading pane.
- [ ] **Phase 3:** Implement the Javascript logic for a font-size toggle, and auto-saving the current text and scroll position into the `localStorage` VFS to resume later.
- [ ] **Phase 4:** Create the native Windows wrapper (`main.c`, `app.rc`, `build.bat`) in the `KRead` directory using native Win32 controls, and add it to `rebuild_all.bat`. Finally, commit and push to GitHub.
