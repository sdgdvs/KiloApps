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

**Target App:** KDragon
**Status:** In Queue
**Current Phase:** In Queue

## Round-Robin Continuous Improvement Queue (NEVER STOP — loop forever via cron)
Pick the top app from this list, identify and fix usability and UI problems (update BOTH web and native versions if applicable), and then move it to the very bottom of the list. Complete exactly ONE app per cron turn (using a single subagent if needed), commit your changes, and then stop your execution. Let the recurring cron schedule wake you up to process the next app. When you reach the end of the list, you'll be back at the top — the cron cycle never ends. If new apps appear, add them to the queue.

- KDragon
- KMech
- KAudio
- KRadio
- KBBS
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

## Progress Log (trimmed by Director 2026-08-22 — keeping only latest entry per app)
- KChess: Usability and UI issues fixed (integrated cyber-themed Help & Controls modal dialog with keyboard shortcuts table and game modes guide, added interactive top Help header button, added non-blocking toast notification system with startup welcome prompt, replaced blocking browser alerts with sleek toast feedback for FEN/PGN import/export and save/load state, and wired F1/? to Help and H to Hint in web; added ShowHelpDialog comprehensive shortcut and feature guide, updated bottom action buttons with explicit shortcut badges [Help [F1], Undo [U], Redo [Y], Save [F5], Load [F9], FEN/PGN [E]], wired F1/? to Help and H to Hint, updated window title with help hint in native C).
- KChat: Usability and UI issues fixed (replaced blocking browser prompts and alerts with custom cybernetic Help modal dialog, Create Poll modal, Create Channel modal, and Edit Topic modal, added non-blocking toast notification system with startup welcome toast, added [1-4] channel switching hotkeys with visible numeric key badges, [P] poll, [S] stats, and [Ctrl+F] search shortcuts with backdrop-click and Escape modal dismissal in web; updated native C with ShowHelpDialog comprehensive guide, button shortcut badges [Connect [C], + Poll [P], Vote [V], Stats [S], Help [F1], Pin [P], React [R], Send [Enter], Ask AI [Ctrl+A], Save TXT [Ctrl+S]], SearchSubclassProc for Esc clear, message loop hotkeys for F1/Ctrl+1-4/Ctrl+A/Ctrl+P/Ctrl+S/Ctrl+J, and window title hint).
- KChart: Usability and UI issues fixed (replaced blocking browser alerts with custom cybernetic Help modal dialog and non-blocking toast notification system with startup welcome toast, added [1-6] direct mode switching hotkeys with visible numeric badges, Enter key table editing navigation, Escape and backdrop-click modal dismissal, and toast feedback for presets, themes, exports, and imports in web; updated native C with ShowHelpDialog comprehensive guide, button shortcut badges [R, M, C, T, S, F1], [1-6] mode switching hotkeys, global message loop hotkey interception, and window title hint).
- KCalendar: Usability and UI issues fixed (replaced blocking browser alerts and confirms with custom cybernetic Help modal dialog, dedicated non-blocking Delete confirmation modal, and toast notification system with startup welcome toast, added [1-4] view tab switching hotkeys with visible numeric key badges, [N] new event, [T] today, [S] stats, and [F1]/[H] shortcuts with backdrop-click modal dismissal in web; added ShowHelpDialog comprehensive shortcut and feature guide, updated button labels with shortcut badges [T, S, F1, Enter, Del], message loop hotkey interception for F1/H/T/S, and window title hint in native C).
- KHex: Usability and UI issues fixed (replaced blocking browser alerts with custom cybernetic Help modal dialog and non-blocking toast notification system with startup welcome toast, added [1-7] tab switching hotkeys with visible numeric badges, [E] endian toggle hotkey, Escape and backdrop-click modal dismissal, and toast feedback for export copying, sample data loading, pattern search, and byte operations in web; added ShowHelpDialog comprehensive user guide, global message loop accelerator interception for F1/Ctrl+E/Ctrl+1-7 hotkeys across all child controls, updated button shortcut badges, bottom shortcut status hint, and window title hint in native C).
- KCalc: Usability and UI issues fixed (replaced blocking browser alerts with custom cybernetic Help modal dialog and dedicated Memory Registers modal, added non-blocking toast notification system with startup welcome toast and export/calculation feedback, synchronized auto-opening dimensions to 780x640 in App.jsx, added [1-5]/D/M/F1 shortcuts with visible tab badges and backdrop-click modal dismissal in web; fixed critical bug in native C where typing in edit controls intercepted input and hijacked display buffer, added explicit Help [F1] toolbar button, [1-5] mode switching hotkeys, ShowHelpDialog user guide, and window title hint).
- KImage: Usability and UI issues fixed (replaced blocking browser alerts with custom cybernetic Help modal dialog and non-blocking toast notifications with startup welcome toast, added 1-click Load Demo Image button on empty state, added 1-5 tab switching shortcuts with visible numeric badges, Escape and backdrop-click modal/crop dismissal, Ctrl+S export shortcut, +/-/0 zoom shortcuts, and explicit Help [F1] header button in web; updated native C with ShowHelpDialog comprehensive guide, explicit Help [F1] toolbar button, global message loop hotkey interception for F1/H/Ctrl+S/Ctrl+O/Esc, and expanded keyboard filter and navigation shortcuts).
- KPad: Usability and UI issues fixed (eliminated blocking browser alerts/prompts/confirms with custom cybernetic modal dialogs for unsaved changes and password-protected exports, added non-blocking toast notifications with startup welcome toast, added top navigation Help [F1] button, added tab numbering badges with Ctrl+1-9 hotkeys in web; updated native C with 4-part status bar including [F1] Help indicator, dynamic window title with help hint, WM_GETMINMAXINFO minimum dimensions, ShowHelpDialog user guide, and message loop hotkeys Ctrl+1-9, Ctrl+Tab, Ctrl+A, and F1).
- KBase: Usability and UI issues fixed (replaced blocking browser alerts with custom cybernetic Help modal dialog and non-blocking toast notifications with startup welcome toast, added 1-5 tab switching shortcuts with visible numeric badges, Escape and backdrop-click modal dismissal, and explicit F1/H header button in web; fixed critical bug where typing 'H' in edit controls intercepted input and triggered help dialog, added explicit Help [F1] button, comprehensive ShowHelpDialog user guide, and window title hint with F1 shortcut in native C).
- KJournal: Usability and UI issues fixed (replaced blocking browser confirms with custom modal confirmation dialog for entry deletion, template overwrite, and PIN removal, added non-blocking toast notifications with startup welcome toast, empty-state 1-click Quick Starter chips, search clear button, and keyboard shortcuts F1/H, Esc, Ctrl+S save, Ctrl+N new/today, Ctrl+T templates, Ctrl+F search, and 1-6 quick mood selection in web; fixed critical JSON export omission of journal entry text in native C with JSON escaping, added backup file import support, clarified save/finish prompts with EOF/Ctrl+Z, updated help guide and title hint, and verified 1100x750 synchronized bounds).
- KRead: Usability and UI issues fixed (replaced blocking browser alerts/prompts/confirms with custom modal dialogs for Tab Renaming, Annotation Notes, and Clear Confirmation, added non-blocking toast notifications with startup welcome prompt, sample document quick-loaders on empty state, Markdown export format alongside JSON/TXT, and keyboard shortcuts F1/H, Esc, Ctrl+T, Ctrl+W, Ctrl+O, Ctrl+F, Ctrl+B, Ctrl+S, Ctrl+1-9, +/- in web; updated native C with dynamic high-DPI tab bar height calculation, full message loop hotkeys F1/H/Ctrl+T/Ctrl+W/Ctrl+O/Ctrl+F/Ctrl+B/Ctrl+S/Ctrl+1-9/Ctrl++/Ctrl+-, comprehensive ShowHelpDialog guide, dynamic window title status, and synchronized 850x620 bounds in App.jsx).
- KContacts: Usability and UI issues fixed (replaced blocking browser alerts/prompts/confirms with sleek cyber modals for Export format selection, Delete confirmation, and Help guide, added non-blocking toast notifications with startup welcome prompt, search clear button, dynamic category count badges, Enter-to-save on inputs, and keyboard shortcuts F1/H, Esc, Ctrl+S, Ctrl+N, 1-7 category switching, and list navigation in web; updated native C with explicit button shortcut badges [+ New [N], Help [F1], Save Details [Ctrl+S]], full message loop hotkeys F1/H/Ctrl+S/Ctrl+N/1-7/Del/Esc/Ctrl+M/Ctrl+E/Ctrl+I, ShowHelpDialog user guide, and window title hint).
- KTimer: Usability and UI issues fixed (replaced blocking alert popups with cybernetic Help modal dialog and non-blocking toast notifications with startup welcome toast, added 1-5 tab switching hotkeys, Space Start/Pause, L lap split, R reset, and Enter-to-start/add on input fields in web; added Help [F1] tab button, ShowHelpDialog user guide, [1-5] tab switching, Space/L/R hotkeys, Enter-to-start in duration edit controls, and window title hint in native C).
- KGraph: Usability and UI issues fixed (integrated cybernetic Help modal dialog with shortcuts and mathematical formula syntax reference, non-blocking toast notifications with startup welcome toast and export/zoom feedback, Enter key instant evaluation on inputs, ResizeObserver for high-DPI scaling, 1-4 tab and C/P/M mode switching shortcuts in web; implemented mouse wheel zooming, Enter key evaluation via edit subclassing, custom dark theme styling eliminating gray control flash, WS_CLIPCHILDREN flicker reduction, ShowHelpDialog, and keyboard hotkeys M/P/R/Enter/F1 in native C).
- KConverter: Usability and UI issues fixed (replaced blocking alerts with cybernetic Help modal dialog and non-blocking toast notifications with startup welcome toast, added 1-6 tab switching shortcuts, Escape and backdrop-click modal dismissal, quick value presets [1, 10, 100, Clear], and explicit F1/H header button in web; fixed critical bug where typing 'H' in expressions like 'km/h' triggered help, added explicit Help [F1] button, [1-5] tab switching hotkeys, ShowHelpDialog guide, updated window title with F1 help hint, and expanded client bounds in native C).
- KTask: Usability and UI issues fixed (added dedicated Help modal dialog with full shortcut and feature reference, added Help [F1] header button, interactive task card selection, Delete key task termination, 1-3 tab switching shortcuts, and backdrop click modal dismissal in web; fixed critical bug where typing 'H' in filter edit triggered help messagebox, added Enter to filter/focus list, Enter/I to inspect, updated toolbar labels with explicit shortcut hints, added comprehensive ShowHelpDialog, eliminated GDI font leak, and improved button layout in native C).
- KSpace: Usability and UI issues fixed (replaced blocking alert modals with sleek cybernetic toast notifications, added startup welcome toast, redesigned non-wrapping responsive skill buttons, added visible in-game pause/help HUD controls and pause-menu help routing in web; implemented full mouse click WM_LBUTTONDOWN navigation, added ClearType multi-tiered typography fixing HUD clipping off-screen, and eliminated GDI font leaks in native C).
- KTetris: Usability and UI issues fixed (fixed math linker unresolved externals and struct compiler errors, added WS_CLIPCHILDREN to eliminate native flicker, mapped F1 and H help across web and native with explicit labels, implemented full mouse click menu navigation and help toggling in web and native, and added startup welcome toast and non-blocking notification system in web).
- KSnake: Usability and UI issues fixed (implemented missing save/resume game state, interactive keybind rebinding, match stats export CSV/JSON/Replay, non-blocking toast notifications replacing alert modals, sub-overlay ESC/Enter handling, and startup welcome toast in web; added crisp hFontSmall typography eliminating text clipping, enhanced menu layout with top record display, and added full ESC/P/S/Q pause/exit and F1 help navigation in native C).
- KPaint: Usability and UI issues fixed (added modern dark Help modal dialog and non-blocking toast notifications in web, mapped F1 and tool shortcuts B/E/L/R/C/G/A/W/S across web and native, updated startup canvas text, window title, and Help button to explicitly show F1/H shortcut, and added startup welcome toast).
- KZip: Usability and UI issues fixed (aligned native listbox with Consolas monospace font and column header, added empty state prompts and status messages to both web and native, updated Help button/dialog with explicit F1/H shortcuts, added non-blocking toast notifications and fallback local file loading in web, and added Enter/Esc shortcut support and WM_GETMINMAXINFO layout constraints in native).
- KNet: Usability and UI issues fixed (added Help modal dialog, toast feedback, Enter-to-execute on all inputs, and high-DPI canvas scaling in web; added dark terminal styling, live filter on typing, Enter-to-fetch/filter, and global F1 shortcut in native).
- KMedia: Usability and UI issues fixed (added drag & drop file loading, visual empty state, seek buttons, volume mute toggle, non-blocking toast notifications, video click play/pause and dblclick fullscreen to web; fixed play/pause toggle bug, added live playback time/status indicator, seek buttons, and F1 hotkey handling in edit controls to native C).
- KPass: Usability and UI issues fixed (synchronized 500x620 layout, centered native controls & expanded vault listbox, applied crisp DPI fonts to all controls, added Enter-to-unlock and native Lock Vault button, added explicit F1/H Help button and non-blocking clipboard feedback in native; replaced web blocking alert modals with sleek toast notifications, added header Help button, quick-copy on password click, and password mask/reveal toggle in web vault).
- KNote: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 800x600, added explicit F1/H help bindings with visual text in native and web versions, calculated dynamic native font heights, and verified WS_CLIPCHILDREN and SetProcessDPIAware).
- KQuest: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 1000x760, added explicit F1/H help bindings with visual text in native and web versions, calculated dynamic native font heights, and verified WS_CLIPCHILDREN and SetProcessDPIAware).
- KMine: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 550x630, added explicit F1/H help bindings with visual text in native and web versions, calculated dynamic native font heights, and verified WS_CLIPCHILDREN and SetProcessDPIAware).
- KScript: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 960x600, added explicit F1/H help bindings with visual text in native and web versions, calculated dynamic native font heights, and verified WS_CLIPCHILDREN and SetProcessDPIAware).
- KMaze: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 800x700, added explicit F1/H help bindings with visual text in native and web versions, calculated dynamic native font heights, and verified WS_CLIPCHILDREN and SetProcessDPIAware).
- KConnect4: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 580x780, added explicit F1/H help bindings with visual text in native and web versions, calculated dynamic native font heights, and verified WS_CLIPCHILDREN and SetProcessDPIAware).
- KPing: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 850x650, added explicit F1/H help bindings with visual text in native and web versions, calculated dynamic native font heights, and verified WS_CLIPCHILDREN and SetProcessDPIAware).
- KMandel: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 800x600, added explicit F1/H help bindings with visual text in native and web versions, calculated dynamic native font heights, and verified WS_CLIPCHILDREN and SetProcessDPIAware).
- KMail: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 900x600, added explicit F1/H help bindings with visual text in native and web versions, calculated dynamic native font heights, and verified WS_CLIPCHILDREN and SetProcessDPIAware).
- KFont: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 950x700, added explicit F1/H help bindings in native and web versions, calculated dynamic native font heights, and verified WS_CLIPCHILDREN and SetProcessDPIAware).
- KAlchemy: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 800x570, added explicit F1/H help bindings with visual instructions in native and web versions, calculated dynamic native font heights, and verified WS_CLIPCHILDREN and SetProcessDPIAware).
- KColony: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 850x650, added explicit F1/H help bindings with visual text in native and web versions, added negative font sizes for crisp text and WS_CLIPCHILDREN to eliminate flickering).
- KFortress: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 1024x740, explicitly instructed and bound F1/H keys for help, applied WS_CLIPCHILDREN for reduced flicker, and adopted negative font heights natively).
- KiloOS Web UI: Usability and UI issues fixed (added dedicated Help icon to desktop, removed flickering boot screen animation, verified optimizeLegibility CSS, and ensured responsive layout handling).
- KDB: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 900x650, added explicit F1/H help bindings with visual instructions in native and web versions, calculated dynamic native font heights, and verified WS_CLIPCHILDREN and SetProcessDPIAware).
- KClock: Usability and UI issues fixed (synchronized App.jsx and native client dimensions to 340x580, explicitly instructed and bound F1/H keys for help, applied WS_CLIPCHILDREN for reduced flicker, and adopted negative font heights natively).
- KPong: Usability and UI issues fixed (synchronized native window bounds to 940x780 matching App.jsx, added explicit F1/H help bindings with visual text in native and web versions, added negative font sizes for crisp text and WS_CLIPCHILDREN to eliminate flickering).
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
- KTodo: Usability and UI issues fixed (replaced blocking browser alerts with modern dark Help modal dialog and non-blocking toast notifications with startup welcome toast, added 1/2 view switching shortcuts, Escape and backdrop-click modal dismissal, and explicit F1/H header button in web; added WS_CLIPCHILDREN to eliminate native flicker, added ShowHelpDialog with comprehensive shortcut and feature guide, updated status bar and action buttons with explicit F1/H and shortcut hints, and implemented dynamic responsive button layout in native C).
- KSys: Usability and UI issues fixed (mapped F1 for help natively and on web, explicitly stated F1 support in UI labels/buttons, verified 1024x768 bounds, compiled successfully).
- KTask: Usability and UI issues fixed (mapped F1 for help across native subclasses and web, explicitly stated F1 support in UI labels and window title, verified bounds and crisp fonts).
- KPaint: Usability and UI issues fixed (updated App.jsx and native bounds to 1100x750 for proper canvas fit, enforced AdjustWindowRect, verified crisp text and existing help visibility).
- KZip: Usability and UI issues fixed (added WS_CLIPCHILDREN to native C window styles to prevent flickering, mapped F1 to help in both versions, and explicitly updated instructions to state 'Press H or F1 for Help').
- KPac: Usability and UI issues fixed (adjusted App.jsx bounds to 340x520, synced native dimensions and added offsets, added WS_CLIPCHILDREN and negative font heights for crisp text, added F1 hotkey handling and explicit help text in both versions).
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
- KSpace: Usability and UI issues fixed (added WS_CLIPCHILDREN and F1 hotkey support to native, implemented F1 support in web, updated instructions to state 'PRESS [H] OR [F1] FOR HELP', verified bounds).
- KType: Usability and UI issues fixed (canvas scaling relative to devicePixelRatio applied in web for crisp text, automatic toggleHelp() on load added for first-time users, native currentMode default swapped to Help screen, SetProcessDPIAware validated).
- KGraph: Usability and UI issues fixed (added WS_CLIPCHILDREN and F1 hotkey support to native, implemented F1 support in web, updated button labels and instructions, verified 1024x768 bounds).
- KRead: Usability and UI issues fixed (added F1 help shortcut to web, explicitly instructed F1/H usage on web/native startup UI, enabled crisp native text rendering via negative font heights, and adjusted native layout clipping via AdjustWindowRect).
- KJournal: Usability and UI issues fixed (added min-width/min-height layout constraints to web version, explicitly updated web instructions to state 'H' or F1, and mapped F1 to help; added explicit 'H' help instructions to all native terminal sub-menus and resolved sub-menu help routing in main.c).
- KChat: Usability and UI issues fixed (updated App.jsx auto-opening bounds to 850x650, implemented native negative font height for crisp text rendering, and added a visible 'Help' button to web control bar).
- KColony: Usability and UI issues fixed (updated App.jsx auto-opening bounds to 850x650, implemented negative font heights and AdjustWindowRect in native C, and applied optimizeLegibility CSS to the web interface).
- KFortress: Usability and UI issues fixed (implemented negative font heights in native C for crisp DPI scaling, added optimizeLegibility to web CSS, and updated web UI Help button to explicitly display the 'H' hotkey).
- KAlchemy: Usability and UI issues fixed (updated App.jsx auto-opening bounds to 1200x800, added negative font heights for crisp text in native C, applied devicePixelRatio canvas scaling for crisp visuals in web canvas, and added optimizeLegibility).
- KFarm: Usability and UI issues fixed (added non-blocking toast notifications for weather, market transactions, out-of-season warnings, and crop actions in web; fixed duplicate morning animal money bug in web nextDay(); added 1-4 seed selection and Space for Sleep shortcuts across web and native; added global message loop hotkey interception in native; updated button labels and Farmer's Almanac with explicit shortcuts; increased App.jsx window bounds to 620x760).
- KSolitaire: Usability and UI issues fixed (mapped F1 to Help dialog and H to Hints across web and native with explicit toolbar and status bar labels, added non-blocking toast notifications with welcome prompt on startup and clear feedback for hints/skills/stats, added Escape key and backdrop-click modal dismissal in web, and updated window title and menu shortcuts in native C).
- KTerm: Usability and UI issues fixed (integrated cyber-themed Help Modal dialog, non-blocking toast notifications with startup welcome toast, explicit F1/H toolbar & status bar shortcuts, and global key navigation in web; implemented dedicated authentic prompt display, Ctrl+T new tab, Ctrl+W close tab, Ctrl+C line cancel, output window key forwarding, F1 Help dialog, Segoe UI tab font, and vertical layout bounds in native C).
- KSynth: Usability and UI issues fixed (replaced blocking alert modals with sleek cybernetic toast notifications, added startup welcome toast, made help modal non-blocking with Escape and backdrop dismissal, and updated window dimensions to 980x820 in web; added visible Help [F1] button, removed blocking startup MessageBox, fixed F1 hotkey handling across edit controls, and added transparent static control backgrounds in native C).
- KRogue: Usability and UI issues fixed (resolved critical keybinding conflict where movement keys intercepted Ability [A] and Help [H], mapped F1 and H to Help with explicit button/HUD labels across web and native, added non-blocking toast notifications with startup welcome toast and quicksave/load feedback, added modal backdrop dismissal and enhanced Help guide in web; added WS_CLIPCHILDREN to eliminate native flicker, updated window title and in-game HUD with F1/H, and added F1 hotkey handling across character creation and game states in native C).
- KSys: Usability and UI issues fixed (replaced blocking alert modals with dedicated cybernetic Help modal dialog and dynamic non-blocking toast notifications, added startup welcome toast, added custom Add Service Daemon modal replacing browser prompts, added Copy to Clipboard button in export tab, added 1-5 tab switching shortcuts and R run/refresh shortcuts with visual tab badges and backdrop-click modal dismissal in web; updated native C with [1-5] tab switching hotkeys, ShowHelpDialog, R hotkey, explicit button shortcut hints, and verified high-DPI font scaling and 1024x768 bounds).
