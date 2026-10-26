---
name: kilo-usability
description: >-
  Audits and refines user experience, responsive layout, HiDPI canvas scaling, focus trapping,
  keyboard/VFS ergonomics, and window sizing across KiloApps using 1-command verification/handoff.
---

# KiloApps Usability & UX Skill (`kilo-usability`)

Executes UX, responsive layout, accessibility, and input ergonomics enhancements on **exactly ONE application per turn**.

## ⚡ Flash Execution & Tool-Budget Protocol (CRITICAL)
- **Tool Call Budget (<= 32 tool calls)** & **Commit Checkpoint (~16 tool calls)**:
  - Inspect `KiloOS/public/apps/k<name>.html` (and `<AppName>/main.c` if native C exists), apply high-craft responsive layout, HiDPI canvas, focus trap, and keyboard/VFS UX improvements, then verify and hand off!
- **1-Command Verification & Visual Sheet Review**:
  ```powershell
  python scripts/verify_turn.py --app <TargetApp>
  ```
  - `verify_turn.py` automatically runs `scripts/visual_probe.py --compare-head` at the app's exact `KiloOS/src/App.jsx` window dimensions, reporting any clipped controls (`cutOff`), overflow, or canvas aspect distortion, and saves a before/after contact sheet at `docs/gallery/review/<targetapp>/sheet.png`. Call `view_file` once on `sheet.png` to visually confirm zero clipped controls and clean layout!
- **1-Command Queue Handoff & Logging**:
  ```powershell
  python scripts/advance_turn.py --agent kilo-usability --app "<TargetApp>" `
    --title "Usability, Responsive Layout & Ergonomics Polish" `
    --bullet "Status: PASS ✅ (<copy summary from verify_turn.py>)." `
    --bullet "Layout & HiDPI: <responsive wrapping, compact viewport support, devicePixelRatio crispness>." `
    --bullet "Keyboard & Modals: <F1/H help, Escape dismissal, focus trapping, high-contrast focus rings>."
  ```
  *(If the app is now completely polished and mature across all passes, add `--quarantine-mature "<Mature summary>" --category "<Category>"` to promote it to Mature Applications!)*
- **Commit, Push & STOP**:
  ```powershell
  git add -A
  git commit -m "ux(usability): usability and layout polish for <TargetApp>"
  git push origin main
  ```

---

## Usability Focus Areas
1. **Responsive Viewport & Window Ergonomics**:
   - Ensure toolbars, sidebars, and tables wrap or scroll cleanly at both default window size (`KiloOS/src/App.jsx`) and narrow/snapped (`50%` width / `<600px`) dimensions without clipped buttons.
2. **HiDPI Canvas & Crisp Typography**:
   - Scale canvas buffers by `window.devicePixelRatio || 1` and handle `resize` via `requestAnimationFrame`.
3. **Discoverable Help (`[F1]` / `[H]`) & Modal Focus Trapping**:
   - Visible statusbar or header pill showing key shortcuts (`[F1/H] Help | [F5/F9] Save/Load | [M] Mute`).
   - Modals trap `Tab` focus, restore focus on close, and dismiss cleanly on `Escape` or backdrop click.
4. **High-Contrast Focus & Touch/Pointer Feedback**:
   - Clear `:focus-visible` outlines and active button states; `touch-action: manipulation` (or `none` on game canvases).
