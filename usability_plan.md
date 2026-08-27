# Usability & UX Plan

## Coordination Rules (DO NOT DELETE â€” required for subagent context)

**Multi-Agent System:** 6 worker agents + 2 directors operate on this repo on overlapping schedules. You are the **Usability Agent**.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership â€” only edit YOUR file (`usability_plan.md`).** Read but NEVER edit the other plan files.
- **Shared files:** You have permission to edit `KiloOS/src/App.jsx`, `KiloOS/src/index.css`, `KiloOS/src/main.jsx`, `KiloOS/index.html`, and any specific app's HTML/JS/C files to improve usability. 
- **When editing `App.jsx`:** other agents may have added APPS array entries. Always prefer the remote version for APPS entries if a conflict occurs, then re-apply your UI changes.
- **Dual-target model:** Each app has a native C version (`K[Name]/main.c` + `build.bat`) and a web HTML5 version (`KiloOS/public/apps/k[name].html`). When fixing usability for an app, consider if the fix is needed for both versions.
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing:** After editing HTML â†’ verify in browser if possible. After editing App.jsx/css â†’ `cd KiloOS && npm run build`. After editing `.c` files â†’ run the app's `build.bat`.
- **Version bumping:** If you modify KiloOS shell files or update versioning/changelog, bump the patch version in `KiloOS/package.json` AND update `MICROS_VERSION` in `KiloOS/src/App.jsx` so the opening screen displays the current version.
- **CI/CD:** Every push to `main` triggers GitHub Actions â†’ Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails â†’ `git pull --rebase` â†’ resolve conservatively â†’ push again.
- **Logging discipline:** Keep this plan file concise. A few lines per completed item. Do NOT dump file contents or create verbose logs.

**WORK FOCUS (CRITICAL): USABILITY, UI, AND UX**
- Most apps have UI and usability problems, such as auto-opening in a size that doesn't show the full UI, not showing controls, lacking a visible "press h for help" prompt (or any other appropriate opening instructions) on startup, blurry text, or bad layout.
- **Your Job:** Fix these issues! Ensure each app opens at an appropriate size, has clear instructions or help menus, crisp text rendering (e.g., canvas scaling issues), and intuitive controls.
- You also maintain and polish the KiloOS web UI itself (Start menu, taskbar, window manager, desktop).

---

## â�±ï¸� TURN SCOPING & TERMINATION (CRITICAL â€” READ EVERY TURN)

**Single-Item-Per-Turn Rule:**
- Each cron trigger = ONE turn. Process exactly ONE item from your queue, then STOP.
- "Loop forever" means the CRON loops forever across turns, NOT that you loop within a single turn.
- After committing and pushing your work for ONE item, STOP CALLING TOOLS immediately.

**Subagent Timeout Rule:**
- If you spawn a subagent, set a timer for 8 minutes using the `schedule` tool with `TimerCondition` set to the subagent's conversation ID.
- If the timer fires (subagent hasn't finished in 8 min), KILL the subagent using `manage_subagents`, log a one-line failure note in your plan file, commit, push, and STOP.
- NEVER spawn more than ONE subagent at a time.
- NEVER spawn a second subagent if the first one failed. Stop and let the next cron turn retry.

**Graceful Termination Checklist (do this EVERY turn before stopping):**
1. Processed one item
2. Updated plan file
3. Committed and pushed
4. All subagents terminated (killed or completed)
5. STOP â€” call no more tools

---

**Target App:** KiloOS Web UI
**Status:** In Queue
**Current Phase:** In Queue

## Round-Robin Continuous Improvement Queue (NEVER STOP â€” loop forever via cron)
Pick the top app from this list, identify and fix usability and UI problems (update BOTH web and native versions if applicable), and then move it to the very bottom of the list. Complete exactly ONE app per cron turn (using a single subagent if needed), commit your changes, and then stop your execution. Let the recurring cron schedule wake you up to process the next app. When you reach the end of the list, you'll be back at the top â€” the cron cycle never ends. If new apps appear, add them to the queue.

- KPong
- KClock
- KDB
- KiloOS Web UI
- KFortress
- KColony
- KAlchemy
- KFont
- KMail
- KMandel
- KPing
- KConnect4
- KMaze
- KScript
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
- KSolitaire
- KTerm
- KSynth
- KTask
- KRogue
- KSys
- KTodo
- KConverter
- KGraph
- KTimer
- KContacts
- KRead
- KJournal
- KBase
- KPad
- KImage
- KCalc
- KHex
- KCalendar
- KChart
- KChat
- KChess
- KDragon
- KMech
- KAudio
- KRadio
- KBBS

## Progress Log (trimmed by Director 2026-08-22 — keeping only latest entry per app)
- KBBS: Usability and UI issues fixed (adjusted App.jsx dimensions to 850x650, implemented F1/H help bindings with visible text instructions, added WS_CLIPCHILDREN to eliminate flickering, and enabled crisp DPI-scaled fonts).
- KRadio: Usability and UI issues fixed (adjusted App.jsx dimensions to 400x250, mapped explicit F1/H help bindings with visual text in native and web versions, added negative font sizes for crisp text and WS_CLIPCHILDREN to eliminate flickering).
- KAudio: Usability and UI issues fixed (adjusted App.jsx auto-opening bounds to 1040x840, explicit F1/H help bindings set with visual text in native and web versions, negative font sizes for crisp text, WS_CLIPCHILDREN added to prevent flickering).
- KMech: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 600x500, added explicit F1/H help hotkeys to garage UI, added WS_CLIPCHILDREN and crisp negative font heights natively).
- KDragon: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 600x500, added explicit F1/H help hotkeys, added WS_CLIPCHILDREN and crisp negative font heights natively).
- KChess: Usability and UI issues fixed (adjusted App.jsx bounds to 760x900 to fit canvas and titlebar without scrollbars, added WS_CLIPCHILDREN and native negative font height for crisp text, added on-screen help instructions mapping to F1/H).
- KChat: Usability and UI issues fixed (adjusted bounds to 850x650 in App.jsx and native, ensured explicit F1 help instructions, added WS_CLIPCHILDREN and crisp text via DPI).
- KChart: TIMEOUT (Subagent failed to complete usability fixes within 8 minutes).
- KCalendar: Usability and UI issues fixed (standardized dimensions to 800x600, integrated AdjustWindowRect and WS_CLIPCHILDREN for flawless native rendering, applied crisp DPI scaling, explicit F1/H help hotkey mapped across platforms).
- KHex: Usability and UI issues fixed (increased App.jsx dimensions to 860x780 to match native layout, added AdjustWindowRect and WS_CLIPCHILDREN for flawless native rendering, configured crisp text via DPI, explicit F1/H keys shown for Help).
- KCalc: Usability and UI issues fixed (synchronized App.jsx and native dimensions to 350x430, added WS_CLIPCHILDREN and crisp DPI scaling, explicitly showed F1/H help hotkey in web and native).
- KImage: Usability and UI issues fixed (adjusted bounds to 1000x700 in App.jsx and native, ensured explicit H/F1 help instructions, and applied crisp native text via DPI awareness and negative font heights).
- KPad: Usability and UI issues fixed (synchronized App.jsx and native client bounds to 800x600, implemented SetProcessDPIAware and WS_CLIPCHILDREN natively, and verified explicit Help hotkeys).
- KBase: Usability and UI issues fixed (synchronized App.jsx and native client bounds to 900x600, applied negative font heights and SetProcessDPIAware for crisp text, added WS_CLIPCHILDREN, and ensured explicit Help shortcuts).
- KJournal: Usability and UI issues fixed (added SetProcessDPIAware and WS_CLIPCHILDREN to native, crisp console font, adjusted native bounds to 1100x750; verified web help hotkeys and explicitly visible instructions).
- KRead: Usability and UI issues fixed (adjusted App.jsx bounds to 800x600, ensured explicit F1/H Help instructions, enabled crisp native text via negative font heights and WS_CLIPCHILDREN).
- KContacts: Usability and UI issues fixed (updated App.jsx bounds to 830x565, applied negative font heights and SetProcessDPIAware for crisp native rendering, and ensured explicit Help shortcuts).
- KCalendar: Usability and UI issues fixed (auto-opening size to 1024x768, DPI_AWARENESS_CONTEXT applied in native C, help toast duration increased to 8s, layout wrapping fixed).
- KPad: Usability and UI issues fixed (auto-opening size adjusted, DPI awareness enabled for crisp fonts, taskbar visibility fixed, F1 help instructions added to welcome text, floating panel layout adjusted).
- KBase: Usability and UI issues fixed (auto-opening size adjusted, DPI awareness and crisp text enabled, visible help controls and F1/H shortcuts added).
- KTimer: Usability and UI issues fixed (added WS_CLIPCHILDREN to native, mapped F1 for help across both platforms, explicitly stated F1 support in UI labels, verified 460x580 bounds and crisp text).
- KConverter: Usability and UI issues fixed (mapped F1 for help on web, added WS_CLIPCHILDREN to native, explicitly stated F1 support in web title hint, verified native text and window logic).
- KTodo: Usability and UI issues fixed (fixed native bug where typing 'H' in EDIT controls incorrectly triggered help, verified 800x600 bounds and existing web hotkey logic).
- KSys: Usability and UI issues fixed (mapped F1 for help natively and on web, explicitly stated F1 support in UI labels/buttons, verified 1024x768 bounds, compiled successfully).
- KTask: Usability and UI issues fixed (mapped F1 for help across native subclasses and web, explicitly stated F1 support in UI labels and window title, verified bounds and crisp fonts).
- KSynth: Usability and UI issues fixed (increased App.jsx bounds to 950x750, added WS_CLIPCHILDREN to native, verified F1/? help hotkeys, checked explicit help instructions and crisp typography).
- KPaint: Usability and UI issues fixed (updated App.jsx and native bounds to 1100x750 for proper canvas fit, enforced AdjustWindowRect, verified crisp text and existing help visibility).
- KZip: Usability and UI issues fixed (added WS_CLIPCHILDREN to native C window styles to prevent flickering, mapped F1 to help in both versions, and explicitly updated instructions to state 'Press H or F1 for Help').
- KPac: Usability and UI issues fixed (verified native C negative font heights and scaling, verified App.jsx bounds, applied optimizeLegibility to web CSS, and bumped OS version to 0.3.102).
- KContacts: Usability and UI issues fixed (added responsive grid layout for form, WS_CLIPCHILDREN for native window, and ES_WANTRETURN with EM_SETMARGINS for native edit control legibility).
- KMine: Usability and UI issues fixed (updated App.jsx bounds to 1000x750 to accommodate Expert mode width, and added text-rendering: optimizeLegibility to web HTML).
- KMaze: Usability and UI issues fixed (modified native CreateFont parameter to a negative height for crisp DPI rendering, and added text-rendering: optimizeLegibility to web HTML).
- KScript: Usability and UI issues fixed (updated App.jsx bounds to 1000x700, added WS_CLIPCHILDREN to native C window styles, applied optimizeLegibility globally to web HTML body).
- KConnect4: Usability and UI issues fixed (updated App.jsx bounds to 580x780, modified native C CreateFont parameters to negative heights for crisp DPI grid-fitting, and validated help visibility).
- KPing: Usability and UI issues fixed (updated App.jsx bounds to 850x650, implemented native C negative font heights/SetProcessDPIAware/AdjustWindowRect for crisp DPI-aware UI, applied optimizeLegibility to web CSS).
- KMandel: Usability and UI issues fixed (updated App.jsx auto-opening bounds to 1280x720, modified native CreateFont parameter to a negative height for crisp DPI rendering, and added text-rendering: optimizeLegibility to web HTML).
- KMail: FAILED - Subagent timed out after 8 minutes. Some changes were pushed but task didn't complete successfully.
- KFont: Usability and UI issues fixed (updated App.jsx bounds to 950x700, modified native C CreateFontA parameters to negative heights for proper DPI grid-fitting and crisp rendering, bumped OS version to 0.3.100).
- KDB: Usability and UI issues fixed (updated App.jsx bounds to 1100x800, added optimizeLegibility to web HTML, applied negative font heights, SetProcessDPIAware, AdjustWindowRect, and WS_CLIPCHILDREN to native C version).
- KPong: Usability and UI issues fixed (synchronized native window bounds to 950x750 matching App.jsx, added optimizeLegibility to web version for crisp font rendering, verified native font heights and web devicePixelRatio scaling, confirmed clear help instructions).
- KClock: Usability and UI issues fixed (updated native CreateFontA parameters to negative heights for crisp DPI scaling, added text-rendering: optimizeLegibility to web layout for crisp typography, and verified App.jsx bounds 950x650 match content size).
- KAudio: Usability and UI issues fixed (auto-opening size explicitly scaled to 1000x800, help instructions default to open, native font crispness fixed with negative font height, web canvas interaction gestures locked, and font-smoothing applied).
- KBBS: Usability and UI issues fixed (implemented negative font height in native C for crisp text, added AdjustWindowRect to fix UI clipping, fixed native mouse selection offsets, updated native title with Help instruction, and optimized web text rendering).
- KChess: Usability and UI issues fixed (adjusted auto-opening size to 760x860 in App.jsx, implemented negative font height in native C for crisp text rendering, verified devicePixelRatio and AdjustWindowRect usage).
- KChart: Usability and UI issues fixed (synchronized App.jsx auto-opening bounds to match 1024x768, added explicit Help button and 'H' instructions in web, added explicit Help button to native layout, used AdjustWindowRect for accurate native client dimensions, applied text-rendering optimizeLegibility for crisp text).
- KCalc: Usability and UI issues fixed (implemented EnumChildWindows to apply crisp font rendering to all native controls natively, handled WM_CTLCOLORSTATIC for readable labels against dark background, increased native layout padding with AdjustWindowRect, applied text-rendering optimizeLegibility to web version).
- KHex: Usability and UI issues fixed (auto-opening size adjusted to 850x750 in App.jsx, added explicit Help button to web app header, added AdjustWindowRect to fix native layout clipping, applied negative font height for crisp native text rendering, and added Help button to native interface).
- KImage: Usability and UI issues fixed (auto-opening bounds set to 1200x800, explicit Help button added, AdjustWindowRect used in native for proper client area, negative font heights implemented for crisp text, text-rendering optimized in web).
- KiloOS Web UI: Usability and UI issues fixed (replaced transient startup help toast notification with a persistent modal dialog in App.jsx to ensure visibility).
- KQuest: Usability and UI issues fixed (fixed math linker error, verified native C negative font heights and scaling, verified App.jsx bounds 1000x720, and verified optimizeLegibility web CSS).
- KNote: Usability and UI issues fixed (added WS_CLIPCHILDREN to native C window styles, updated native and web Help buttons to show F1 shortcut, repositioned web status bar below textarea).
- KPass: Usability and UI issues fixed (enforced 500x620 bounds with AdjustWindowRect, added WS_CLIPCHILDREN for native, added explicit F1 hotkey support and "Press 'H' or F1 for Help" labels in both versions).
- KMedia: Usability and UI issues fixed (added WS_CLIPCHILDREN to native C window styles to prevent flickering, and applied text-rendering: optimizeLegibility to web CSS).
- KNet: Usability and UI issues fixed (added WS_CLIPCHILDREN to native C window styles, added explicit F1 hotkey support and help labels to both versions, verified App.jsx bounds 960x720).
- KSnake: Usability and UI issues fixed (added WS_CLIPCHILDREN to native C window styles to prevent flickering, mapped F1 to help overlay in both versions, updated visual instructions to mention F1, and verified 540x680 bounds in App.jsx).
- KTetris: FAILED (Subagent timed out after 8 minutes).
- KSpace: Usability and UI issues fixed (added WS_CLIPCHILDREN and F1 hotkey support to native, implemented F1 support in web, updated instructions to state 'PRESS [H] OR [F1] FOR HELP', verified bounds).
- KType: Usability and UI issues fixed (canvas scaling relative to devicePixelRatio applied in web for crisp text, automatic toggleHelp() on load added for first-time users, native currentMode default swapped to Help screen, SetProcessDPIAware validated).
- KSolitaire: Usability and UI issues fixed (added F1 hotkey to native and web, added WS_CLIPCHILDREN to native, fixed native compilation errors, updated explicit instructions for F1/H help, verified 920x800 bounds).
- KTerm: Usability and UI issues fixed (added WS_CLIPCHILDREN to native, explicitly mapped F1 for Help in both versions, added missing native linkers, updated UI text, verified 960x600 bounds).
- KRogue: Usability and UI issues fixed (implemented negative font height and SetProcessDPIAware for crisp native text, added optimizeLegibility to web, verified 1000x720 bounds, updated help instructions).
- KGraph: Usability and UI issues fixed (added WS_CLIPCHILDREN and F1 hotkey support to native, implemented F1 support in web, updated button labels and instructions, verified 1024x768 bounds).
- KRead: Usability and UI issues fixed (added F1 help shortcut to web, explicitly instructed F1/H usage on web/native startup UI, enabled crisp native text rendering via negative font heights, and adjusted native layout clipping via AdjustWindowRect).
- KJournal: Usability and UI issues fixed (added min-width/min-height layout constraints to web version, explicitly updated web instructions to state 'H' or F1, and mapped F1 to help; added explicit 'H' help instructions to all native terminal sub-menus and resolved sub-menu help routing in main.c).
- KChat: Usability and UI issues fixed (updated App.jsx auto-opening bounds to 850x650, implemented native negative font height for crisp text rendering, and added a visible 'Help' button to web control bar).
- KColony: Usability and UI issues fixed (updated App.jsx auto-opening bounds to 850x650, implemented negative font heights and AdjustWindowRect in native C, and applied optimizeLegibility CSS to the web interface).
- KFortress: Usability and UI issues fixed (implemented negative font heights in native C for crisp DPI scaling, added optimizeLegibility to web CSS, and updated web UI Help button to explicitly display the 'H' hotkey).
- KAlchemy: Usability and UI issues fixed (updated App.jsx auto-opening bounds to 1200x800, added negative font heights for crisp text in native C, applied devicePixelRatio canvas scaling for crisp visuals in web canvas, and added optimizeLegibility).
- KFarm: Usability and UI issues fixed (added WS_CLIPCHILDREN to native, explicitly mapped F1 for Help in both versions, wrapped web bottom controls in flex containers, updated App.jsx width to 600).
