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
- KDB
- KColony
- KFortress
- KAlchemy
- KFont
- KMail
- KMandel
- KPing
- KConnect4


## Progress Log
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
