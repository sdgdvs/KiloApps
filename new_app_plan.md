# New App Plan: KBase

**Status:** Not Started

## Description
KBase is a minimal developer utility for rapid Base64 encoding/decoding and basic string hashing (MD5/SHA). It operates entirely locally and fits perfectly into the KiloOS ecosystem by remaining strictly under the < 999KB constraints.

## Phases
- [x] **Phase 1:** Scaffold the `KBase` application directory. Create `kbase.html` with a UI featuring input/output text areas and conversion buttons, and integrate it into `KiloOS/src/App.jsx`.
- [x] **Phase 2:** Implement the Javascript logic for Base64 encoding and decoding directly in the browser.
- [x] **Phase 3:** Implement the Javascript logic for basic string hashing (using Web Crypto API for SHA-256) and a quick 'Copy to Clipboard' function.
- [ ] **Phase 4:** Create the native Windows wrapper (`main.c`, `app.rc`, `build.bat`) in the `KBase` directory using native Win32 controls, and add it to `rebuild_all.bat`. Finally, commit and push to GitHub.
