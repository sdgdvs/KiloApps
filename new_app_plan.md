# New App Plan: KPass

**Status:** Completed

## Description
KPass is a minimal, bloat-free password generator and local vault. It allows users to instantly generate secure passwords and safely store them in their virtual filesystem. It strictly maintains the < 999KB KiloOS aesthetic constraints.

## Phases
- [x] **Phase 1:** Scaffold the `KPass` application directory. Create `kpass.html` with a tabbed UI (Generator and Vault) and integrate the web app into `KiloOS/src/App.jsx`.
- [x] **Phase 2:** Implement the Javascript logic for the Password Generator (slider for length, toggles for character types, and a prominent 'Generate' button).
- [x] **Phase 3:** Implement the Javascript logic for the Local Vault (saving credentials to the VFS with a rudimentary XOR-based obfuscation for themed security).
- [x] **Phase 4:** Create the native Windows wrapper (`main.c`, `app.rc`, `build.bat`) in the `KPass` directory using native Win32 controls, and add it to `rebuild_all.bat`. Finally, commit and push to GitHub.
