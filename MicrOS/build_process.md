# Build Process Documentation for MicrOS Minimal Apps

## Prerequisites
- **Node.js** (v20+ recommended) with npm
- **Vite** (installed via `npm install` – already in `devDependencies`)
- **crinkler** binary (included in the repository under `crinkler/`). Ensure `crinkler.exe` is on the system `PATH` or reference it via relative path.
- **Git** (optional, for version control)

## Typical Workflow
1. **Initialize a New App** (see *app_integration_guide.md*)
2. **Development**
   ```bash
   npm install          # install react, vite, etc.
   npm run dev          # start Vite dev server at http://localhost:5173
   ```
   - Edit files in `src/` – changes hot‑reload instantly.
3. **Production Build (HTML)**
   ```bash
   npm run build        # Vite outputs optimized assets to `dist/`
   ```
   - Result: `dist/index.html` + `dist/assets/*.js` (bundle < 50 KB gzipped).
4. **Package as Windows Executable**
   - **Step 4.1 – Bundle into a Single JS File**
     ```bash
     # Concatenate Vite output into one file for crinkler
     cat dist/assets/*.js > dist/micros.bundle.js
     ```
   - **Step 4.2 – Create Minimal HTML Wrapper**
     ```bash
     cp dist/index.html dist/micros.html
     ```
   - **Step 4.3 – Invoke Crinkler**
     ```bash
     crinkler.exe \
       -entry dist/micros.bundle.js \
       -o dist/micros.exe \
       -compress -tiny -optimize -nofloat
     ```
     - Flags explained:
       - `-compress` – aggressive XOR compression.
       - `-tiny` – removes C runtime overhead.
       - `-optimize` – enables dead‑code elimination.
       - `-nofloat` – disables floating‑point support (not needed for UI).
   - **Step 4.4 – Verify EXE Size**
     Ensure `dist/micros.exe` ≤ 150 KB.
5. **Distribution**
   - Publish `dist/micros.html` to any static host (GitHub Pages, CDN).
   - Ship `dist/micros.exe` via releases or installers.

## Automated Script (`pack_apps.bat`)
A Windows batch file already present in the repo simplifies steps 3‑4:
```bat
@echo off
REM Build HTML
npm run build
REM Combine JS bundle
type dist\assets\*.js > dist\micros.bundle.js
REM Package EXE using crinkler (assumes crinkler.exe in PATH)
crinkler.exe -entry dist\micros.bundle.js -o dist\micros.exe -compress -tiny -optimize -nofloat
echo Build complete. HTML: dist\micros.html  EXE: dist\micros.exe
```
Running `pack_apps.bat` from the app root produces both artifacts in one step.

## CI/CD (Optional)
Add the following to a GitHub Actions workflow to auto‑publish on push to `main`:
```yaml
name: Build MicrOS App
on:
  push:
    branches: [ main ]
jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      - name: Set up Node
        uses: actions/setup-node@v4
        with:
          node-version: '20'
      - run: npm ci
      - name: Build & Package
        run: pack_apps.bat
      - name: Upload Artifacts
        uses: actions/upload-artifact@v4
        with:
          name: micros-dist
          path: dist/*
```

---
**Performance Targets** (re‑stated for clarity)
- HTML bundle ≤ 50 KB gzipped.
- EXE ≤ 150 KB.
- Build time on a typical CI runner < 30 seconds.

*The above steps are intentionally concise to keep token usage low for downstream agents.*
