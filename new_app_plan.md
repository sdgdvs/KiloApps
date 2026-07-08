# New App Plan: KContacts

**Status:** Not Started

## Description
KContacts is a minimal, bloat-free local address book and contact manager. It allows users to quickly save, search, and manage their contacts directly inside the KiloOS virtual filesystem, adhering strictly to the < 999KB constraints.

## Phases
- [x] **Phase 1:** Scaffold the `KContacts` application directory. Create `kcontacts.html` with a split UI (List view on left, Details/Edit view on right) and integrate the web app into `KiloOS/src/App.jsx`.
- [x] **Phase 2:** Implement the Javascript logic for adding, editing, and deleting contacts (Name, Email, Phone, Notes).
- [x] **Phase 3:** Implement the Javascript logic for saving/loading from the VFS (`localStorage`) and add a real-time search/filter bar for the contact list.
- [ ] **Phase 4:** Create the native Windows wrapper (`main.c`, `app.rc`, `build.bat`) in the `KContacts` directory using native Win32 controls, and add it to `rebuild_all.bat`. Finally, commit and push to GitHub.
