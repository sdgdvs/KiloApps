# Usability & UX Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** Multiple worker agents operate on this repo on overlapping schedules. You are the **Usability Agent**.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`usability_plan.md`).** Read but NEVER edit the other plan files.
- **Shared files:** You have permission to edit `KiloOS/src/App.jsx`, `KiloOS/src/index.css`, `KiloOS/src/main.jsx`, `KiloOS/index.html`, and any specific app's HTML/JS/C files to improve usability. 
- **When editing `App.jsx`:** other agents may have added APPS array entries. Always prefer the remote version for APPS entries if a conflict occurs, then re-apply your UI changes.
- **Dual-target model:** Each app has a native C version (`K[Name]/main.c` + `build.bat`) and a web HTML5 version (`KiloOS/public/apps/k[name].html`). When fixing usability for an app, consider if the fix is needed for both versions.
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx/css → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.
- **Version bumping:** If you modify KiloOS shell files, bump the patch version in `KiloOS/package.json`.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively → push again.
- **Logging discipline:** Keep this plan file concise. A few lines per completed item. Do NOT dump file contents or create verbose logs.

**WORK FOCUS (CRITICAL): USABILITY, UI, AND UX**
- Most apps have UI and usability problems, such as auto-opening in a size that doesn't show the full UI, not showing controls, lacking a visible "press h for help" prompt (or any other appropriate opening instructions) on startup, blurry text, or bad layout.
- **Your Job:** Fix these issues! Ensure each app opens at an appropriate size, has clear instructions or help menus, crisp text rendering (e.g., canvas scaling issues), and intuitive controls.
- You also maintain and polish the KiloOS web UI itself (Start menu, taskbar, window manager, desktop).

---

**Target App:** KiloOS Web UI
**Status:** In Queue
**Current Phase:** In Queue

## Round-Robin Continuous Improvement Queue (NEVER STOP — loop forever via cron)
Pick the top app from this list, identify and fix usability and UI problems (update BOTH web and native versions if applicable), and then move it to the very bottom of the list. Complete exactly ONE app per cron turn (using a single subagent if needed), commit your changes, and then stop your execution. Let the recurring cron schedule wake you up to process the next app. When you reach the end of the list, you'll be back at the top — the cron cycle never ends. If new apps appear, add them to the queue.


- KDB
- KColony
- KFortress
- KAlchemy
- KFont
- KMail
- KMandel
- KPing
- KConnect4
- KScript
- KMaze
- KMine
- KPac
- KQuest
- KNote
- KPass
- KMedia
- KNet
- KZip
- KPaint
- KFarm
- KSnake
- KTetris
- KSpace
- KType
- KSolitaire
- KTerm
- KSynth
- KTask
- KSys
- KRogue
- KConverter
- KTodo
- KGraph
- KTimer
- KContacts
- KRead
- KBase
- KJournal
- KPad
- KImage
- KHex
- KCalc
- KCalendar
- KChart
- KChess
- KChat
- KBBS
- KAudio
- KClock
- KPong
- KiloOS Web UI (Shell & Apps)

## Progress Log
- KMine: Usability and UI issues fixed (auto-opening size, explicit instructions overlay, cell size and crisp font adjustments).
- KMaze: Usability and UI issues fixed (auto-opening size, HUD layout wrapping, clear help instructions, crisp text).
- KScript: Usability and UI issues fixed (auto-opening size, crisp font, help shortcut/instructions, layout tweaks).
- KConnect4: Usability and UI issues fixed (auto-opening size, explicit help instructions, crisp text, layout).
- KPing: Usability and UI issues fixed (auto-opening size, clear help instructions, crisp text).
- KMandel: Usability and UI issues fixed (auto-opening size, explicit help instructions, crisp text, canvas layout).
- KMail: Usability and UI issues fixed (auto-opening size, fixed layout margins, press h for help shortcut).
- KFont: Usability and UI issues fixed (auto-opening size, crisp text, visible help button).
- KDB: Usability and UI issues fixed (auto-opening size, crisp text, button layout, help popup).
- KPong: Usability and UI issues fixed (auto-opening size, crisp text, help HUD, layout).
- KClock: Usability and UI issues fixed (auto-opening size, crisp text, help guide, layout).
- KAudio: Usability and UI issues fixed (auto-opening size, crisp text, help overlay, layout).
- KBBS: Usability and UI issues fixed (auto-opening size, crisp text, help hotkeys, layout).
- KChess: Usability and UI issues fixed (auto-opening size, crisp text, help indicator, layout).
- KChart: Usability and UI issues fixed (auto-opening size, crisp text, help hints, layout).
- KCalendar: Usability and UI issues fixed (auto-opening size, crisp font, help shortcut, layout).
- KCalc: Usability and UI issues fixed (auto-opening size, help toast, hotkeys).
- KHex: Usability and UI issues fixed (auto-opening size, help keybindings, crisp text, layout).
- KImage: Usability and UI issues fixed (auto-opening size, crisp text, help menu, canvas rendering).
- KPad: Usability and UI issues fixed (auto-opening size, crisp text, help menu, startup text).
- KConnect4: Usability and UI issues fixed (auto-size, controls, help text, canvas crispness).
- KiloOS Web UI (Shell & Apps): Added welcome notification on OS boot to guide new users, OS-wide help hotkey (H/F1), and improved crisp text rendering.
- KDB: Usability and UI issues fixed (auto-size, controls, help keys, crisp text, layout).
- KFont: Usability and UI issues fixed (auto-size, crisp text, help instructions, layout).
- KMail: Usability and UI issues fixed (auto-size, crisp text, help instructions, layout).
- KMandel: Usability and UI issues fixed (auto-size, crisp text, help instructions).
- KPing: Usability and UI issues fixed (auto-size, crisp text, help instructions).
- KScript: Usability and UI issues fixed (auto-size, crisp text, visible controls, layout).
- KMaze: Usability and UI issues fixed (auto-size, crisp text, help instructions, layout).
- KMine: Usability and UI issues fixed (auto-size, crisp text, help instructions, layout).
- KPac: Usability and UI issues fixed (help overlays, auto-opening size, text crispness, sprint key mapping).
- KQuest: Usability and UI issues fixed (auto-size, crisp UI font, help instructions).
- KNote: Usability and UI issues fixed (auto-size, crisp text, welcome instructions, sidebar layout).
- KPass: Usability and UI issues fixed (auto-size, help overlays, hotkey listeners, layout).
- KMedia: Usability and UI issues fixed (auto-size, help instructions, text crispness, title truncation).
- KNet: Usability and UI issues fixed (improved layout, fonts, and help shortcuts).
- KZip: Usability and UI issues fixed (added help shortcuts, crisp fonts, empty states, and auto-sizing).
- KPaint: Usability and UI issues fixed (auto-size, help indicator and dialog, crisp text rendering, layout adjustments).
- KSnake: Usability and UI issues fixed (auto-opening size, crisp font, help instructions on main menu and overlay).
- KTetris: Usability and UI issues fixed (auto-size, controls and help overlay, crisp text scaling, UI discoverability).
- KSpace: Usability and UI issues fixed (UI scaling and help screen added).
- KType: Usability and UI issues fixed (auto-opening sizes, crisp text rendering, visual help features added).
- KSolitaire: Usability and UI issues fixed (window sizing, layout overflowing, and add hint instructions).
- KTerm: Usability and UI issues fixed (auto-opening size, font crispness, help hints on startup/statusbar).
- KSynth: Usability and UI issues fixed (auto-opening size, crisp text rendering, help instructions).
- KTask: Usability and UI issues fixed (auto-opening size, crisp text rendering, keyboard shortcut toast).
- KSys: Usability and UI issues fixed (auto-opening size, crisp text styling, help message boxes/alerts).
- KRogue: Usability and UI issues fixed (auto-size, help hints, modal popup added).
- KConverter: Usability and UI issues fixed (auto-opening size, crisp text rendering, help instructions).
- KTodo: Usability and UI issues fixed (auto-opening size, help popup, toast/hotkey, crisp text rendering).
- KGraph: Usability and UI issues fixed (auto-opening size, DPI awareness, crisp text, layout adjustments, toast notifications, help instructions).
- KTimer: Usability and UI issues fixed (auto-opening size, crisp text styling, help keybinding and instructions).
- KContacts: Usability and UI issues fixed (auto-opening size, help modal, crisp text rendering in native, layout adjustments).
- KRead: Usability and UI issues fixed (auto-opening size, help modal and instructions, crisp text rendering).
- KBase: Usability and UI issues fixed (fix layout, font rendering, add help instructions).
- KJournal: Usability and UI issues fixed (auto-opening size, help modal and instructions, crisp text rendering).
- KChat: Usability and UI issues fixed (auto-opening size, crisp text DPI, help instructions).
- KPac: Usability and UI issues fixed (Appropriate auto-opening size adjusted, added on-screen D-Pad, made help instructions clearer and crisper).
- KQuest: Usability and UI issues fixed (Appropriate window size/visible controls, crisp text, and clear opening instructions).
- KNote: Usability and UI issues fixed (auto-opening size, visible controls, crisp text, and added "Press F1 for Help" shortcut and instructions).
- KPass: Usability and UI issues fixed (Appropriate auto-opening size adjusted, DPI awareness enabled, and 'Press H for Help' instruction added).
- KMedia: Usability and UI issues fixed (Appropriate auto-opening size adjusted, layout issues fixed, help shortcut & modal added, text crispness improved).
- KNet: Usability and UI issues fixed (auto-opening size, DPI scaling, and help shortcut).
- KZip: Usability and UI issues fixed (Appropriate auto-opening size adjusted to 800x600, help modal added, DPI awareness enabled for crisp fonts).
- KPaint: Usability and UI issues fixed (Appropriate auto-opening size adjusted to 1100x700, added scrollbars to native, fixed save logic, and DPI awareness enabled for crisp fonts).
- KSnake: Usability and UI issues fixed (Appropriate auto-opening size adjusted, visible controls, layout fixes, crisp text rendering for canvas, help instructions).
- KTetris: Usability and UI issues fixed (Appropriate auto-opening size adjusted, crisp text and canvas scaling handled, visible controls & help instructions overlaid).
- KSpace: Usability and UI issues fixed (Fixed auto-opening aspect ratio, fixed stretched canvas scaling, and made help instructions more visible).
- KType: Usability and UI issues fixed (Appropriate auto-opening size adjusted to 1000x800, DPI awareness enabled, and help instructions made visible in UI/header).
- KSolitaire: Usability and UI issues fixed (auto-opening window size increased to 920x800, 'H' key for hints shortcut corrected in both web and native, window title and status bar updated).
- KTerm: Usability and UI issues fixed (Appropriate auto-opening size adjusted to 960x600, crisp font rendering enabled, 'h' key alias added for help menu access).
- KSynth: Usability and UI issues fixed (Appropriate auto-opening size adjusted, DPI awareness enabled with scaled UI controls, crisp canvas rendering, and 'H' key help popup added).
- KTask: Usability and UI issues fixed (Appropriate auto-opening size adjusted to 800x600, layout padding increased for visible controls, and 'H' key for help mapped).
- KSys: Usability and UI issues fixed (appropriate auto-opening size adjusted, visible controls styled, explicit help toast overlay added, crisp DPI text enabled).
- KRogue: Usability and UI issues fixed (padding reduced, crisp high-DPI canvas text rendering enabled, and initial startup help instructions added).
- KConverter: Usability and UI issues fixed (auto-opening size adjusted, crisp DPI text enabled in native, and help instruction added/updated).
- KTodo: Usability and UI issues fixed (auto-opening size adjusted, DPI awareness enabled, help startup popup replaced with status bar hint and F1 hotkey, web app native controls dark-themed).
- KGraph: Usability and UI issues fixed (auto-opening size adjusted, DPI awareness and scaling enabled in native, UI controls adjusted, crisp text logic updated in web canvas, help button added).
- KTimer: Usability and UI issues fixed (native window scaling logic fixed to prevent clipping, DPI awareness added, web interface size normalized).
- KContacts: Usability and UI issues fixed (auto-opening size adjusted, DPI awareness enabled in native, layout scaled properly, and "Press H for Help" shortcuts and instructions mapped in title/buttons).
- KRead: Usability and UI issues fixed (auto-opening size adjusted to 1000x800, DPI awareness enabled in native, F1 shortcut and instructions added).
- KBase: Usability and UI issues fixed (auto-opening size adjusted, controls expanded for 64-bit binary display, crisp font applied, and F1 help shortcuts/instructions added).
- KJournal: Usability and UI issues fixed (auto-opening size adjusted, explicit help instructions added to menu/title, crisp text styles, and flex layout scroll fixes).
- KPad: Usability and UI issues fixed (auto-opening size adjusted, DPI awareness enabled for crisp fonts, taskbar visibility fixed, F1 help instructions added to welcome text, floating panel layout adjusted).
- KImage: Usability and UI issues fixed (auto-opening size adjusted, DPI awareness enabled for crisp fonts, F1/H help instructions added to toolbar/window title).
- KHex: Usability and UI issues fixed (auto-opening size adjusted, DPI awareness enabled for crisp fonts, F1/H help instructions added to placeholders and output buffers).
- KCalc: Usability and UI issues fixed (auto-opening size adjusted to 800x600 for web to prevent layout breaking, explicit Help button added, help toast removal fixed, DPI awareness enabled in native for crisp fonts).
- KCalendar: Usability and UI issues fixed (auto-opening size adjusted, added startup help toast, enabled DPI scaling and crisp text in native app, adjusted native layout layout sizes).
- KChart: Usability and UI issues fixed (auto-opening size adjusted, WS_CLIPCHILDREN added to prevent native button flickering, DPI awareness enabled for crisp fonts, canvas layout wrapping fixed).
- KChess: Usability and UI issues fixed (auto-opening size adjusted, responsive constraints added to canvas, CLEARTYPE_QUALITY and DPI awareness enabled in native, help instructions updated).
- KChat: Usability and UI issues fixed (auto-opening size adjusted, CLEARTYPE_QUALITY and antialiasing enabled for crisp fonts, improved input contrast, added explicit /help instructions).
- KBBS: Usability and UI issues fixed (auto-opening size adjusted, canvas imageSmoothingEnabled disabled for crisp fonts, layout/centering fixed, and H hotkey added for help modal).
- KAudio: Usability and UI issues fixed (auto-opening size adjusted to match web, CLEARTYPE_QUALITY and DPI scaling applied for crisp text/oscilloscope canvas, explicit startup help modal added).
- KClock: Usability and UI issues fixed (auto-opening size adjusted to 950x650, DPI awareness and CLEARTYPE_QUALITY enabled in native for crisp fonts, help hotkey title hint added).
- KPong: Usability and UI issues fixed (auto-opening size adjusted, help button and H hotkey added, DPI awareness and devicePixelRatio canvas scaling applied for crisp graphics).
- KiloOS Web UI (Shell & Apps): Usability and UI issues fixed (added desktop watermark, added help option in start menu and help tray icon).
