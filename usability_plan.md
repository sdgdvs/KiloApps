# Usability & UX Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 6 worker agents + 2 directors operate on this repo on overlapping schedules. You are the **Usability Agent**.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`usability_plan.md`).** Read but NEVER edit the other plan files.
- **Shared files:** You have permission to edit `KiloOS/src/App.jsx`, `KiloOS/src/index.css`, `KiloOS/src/main.jsx`, `KiloOS/index.html`, and any specific app's HTML/JS/C files to improve usability. 
- **When editing `App.jsx`:** other agents may have added APPS array entries. Always prefer the remote version for APPS entries if a conflict occurs, then re-apply your UI changes.
- **Dual-target model:** Each app has a native C version (`K[Name]/main.c` + `build.bat`) and a web HTML5 version (`KiloOS/public/apps/k[name].html`). When fixing usability for an app, consider if the fix is needed for both versions.
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx/css → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.
- **Version bumping:** If you modify KiloOS shell files or update versioning/changelog, bump the patch version in `KiloOS/package.json` AND update `MICROS_VERSION` in `KiloOS/src/App.jsx` so the opening screen displays the current version.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively → push again.
- **Logging discipline:** Keep this plan file concise. A few lines per completed item. Do NOT dump file contents or create verbose logs.

**WORK FOCUS (CRITICAL): USABILITY, UI, AND UX**
- Most apps have UI and usability problems, such as auto-opening in a size that doesn't show the full UI, not showing controls, lacking a visible "press h for help" prompt (or any other appropriate opening instructions) on startup, blurry text, or bad layout.
- **Your Job:** Fix these issues! Ensure each app opens at an appropriate size, has clear instructions or help menus, crisp text rendering (e.g., canvas scaling issues), and intuitive controls.
- You also maintain and polish the KiloOS web UI itself (Start menu, taskbar, window manager, desktop).

---

## ⏱️ TURN SCOPING & TERMINATION (CRITICAL — READ EVERY TURN)

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
5. STOP — call no more tools

---

**Target App:** KiloOS Web UI
**Status:** In Queue
**Current Phase:** In Queue

## Round-Robin Continuous Improvement Queue (NEVER STOP — loop forever via cron)
Pick the top app from this list, identify and fix usability and UI problems (update BOTH web and native versions if applicable), and then move it to the very bottom of the list. Complete exactly ONE app per cron turn (using a single subagent if needed), commit your changes, and then stop your execution. Let the recurring cron schedule wake you up to process the next app. When you reach the end of the list, you'll be back at the top — the cron cycle never ends. If new apps appear, add them to the queue.

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
- KPong
- KClock
- KDB
- KiloOS Web UI
- KFortress
- KColony
- KAlchemy

## Progress Log
- KCalendar: Usability and UI issues fixed (updated App.jsx auto-opening bounds to 1024x768, added F1 key support and updated UI labels for help menu in both web and native, applied text-rendering optimizeLegibility to web app body for crisp text).
- KPad: Usability and UI issues fixed (added EM_SETMARGINS to native edit control for better text padding, implemented AdjustWindowRect in native to prevent layout clipping, increased native default font size for legibility, added explicit 'Press F1 for Help' instructions to web status bar, and increased web text container bottom padding to fix scrolling clipping).
- KBase: Usability and UI issues fixed (synchronized App.jsx auto-opening bounds to match the 900x750 web UI size, expanded native main.c window bounds to 900x600 via AdjustWindowRect, expanded native edit boxes width to 850 and output box height to 300 to fix clipping, increased native font size, updated web and native Help text to clarify F1 support, added F1 keyboard shortcut support to web app).
- KTimer: Usability and UI issues fixed (synchronized App.jsx auto-opening bounds to match the 460x580 web UI size, expanded native main.c window bounds to 460x580 via AdjustWindowRect, increased native listbox heights to utilize the extra vertical space, and repositioned native Help label).
- KConverter: Usability and UI issues fixed (auto-opening size updated to 900x650 in App.jsx, web help instructions made more explicit with [Press H for Help] in accent-blue, native app updated with AdjustWindowRect to fix layout clipping and Help text clarified).
- KTodo: Usability and UI issues fixed (added explicit Help button to header and native action bar, updated keyboard listener for 'H' and 'F1', applied AdjustWindowRect to fix native layout clipping, changed native font rendering size for crispness).
- KSys: Usability and UI issues fixed (adjusted auto-opening size to 1024x768 in App.jsx, implemented crisp sparkline scaling with devicePixelRatio in web, added AdjustWindowRect to fix native layout clipping).
- KTask: Usability and UI issues fixed (auto-opening size adjusted to 800x600 in App.jsx, SetProcessDPIAware and explicit Help button added to native UI, crisp devicePixelRatio canvas scaling applied to web).
- KSynth: Usability and UI issues fixed (updated App.jsx auto-opening size to 900x700 to prevent clipping, applied optimizeLegibility for web crisp text rendering, and re-mapped Help shortcuts from H to F1/? to avoid conflicting with the virtual piano's A note).
- KPaint: Usability and UI issues fixed (updated auto-opening size to 1024x768, added visible Help button in web and native, implemented negative font height for crisp native text, added optimizeLegibility to web).
- KZip: Usability and UI issues fixed (increased auto-opening size to 900x650 in App.jsx, added explicit Help buttons to Web and Native UI, added AdjustWindowRect to prevent native layout clipping).
- KPac: Usability and UI issues fixed (updated App.jsx auto-opening size to 400x570, adjusted data-width and height in web version to prevent clipping, and applied negative font height in native for crisp text rendering).
- KPad: Usability and UI issues fixed (auto-opening size to 1000x700, added visible Help status button to web, updated native window title for Help, implemented crisp fonts).
- KContacts: Usability and UI issues fixed (auto-opening size to 850x600, DPI awareness/S() scaling macro applied in native C, help toast on startup added to web, crisp font rendering applied to web body).
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
- KDB: Usability and UI issues fixed (auto-opening size increased to 1024x768, base font size increased for crisp text and readability, controls and layout adjusted for visibility, and help instruction made easily readable).
- KColony: Usability and UI issues fixed (auto-opening size adjusted to 850x650, help instructions clarified by appending [H] to Help menu items, and H hotkey support added to natively toggle the Help overlay).
- KFortress: Usability and UI issues fixed (auto-opening size increased to 1024x700 to prevent map clipping, crisp text and resolution enabled via devicePixelRatio and pixelated image-rendering, and H hotkey added to toggle Help Modal which now opens by default).
- KAlchemy: Usability and UI issues fixed (auto-opening size adjusted to 1024x768 in web and 800x570 in native, explicit H hotkey instruction added to manual button, and CLEARTYPE_QUALITY enabled in native for crisp fonts).
- KFont: Usability and UI issues fixed (auto-opening size increased to 950x700 in native, DPI awareness enabled for crisp fonts, visual styles applied to native controls, layout metrics improved for Unicode Glyphs and Custom Text, and explicit H hotkey instruction added to Help button/title).
- KMail: Usability and UI issues fixed (Appropriate auto-opening size adjusted, DPI awareness enabled for crisp fonts, help hotkey text added to title and sidebar).
- KMandel: Usability and UI issues fixed (auto-opening size adjusted to 1280x720 in native via AdjustWindowRect and SW_SHOWNORMAL, help instruction contrast improved in both web and native via text outlines, touch-action fixed in web).
- KPing: Usability and UI issues fixed (auto-opening size adjusted to 800x600 in native, fixed layout constraints on terminal output box, improved crisp text rendering in web via devicePixelRatio, and fixed help shortcut interception logic).
- KConnect4: Usability and UI issues fixed (auto-size updated to w:600 h:700 in web, AdjustWindowRect in native, explicit F1 help instructions added).
- KScript: Usability and UI issues fixed (added window min size constraints and SetProcessDPIAware for crisp text in native, applied responsive stacking for smaller screens in web, added F1 help hotkey and explicit startup instructions in both).
- KMaze: Usability and UI issues fixed (auto-opening size increased to 800x660 in web and 800x600 in native, removed pixelated scaling in web for crisp vector text, enabled CLEARTYPE_QUALITY in native for crisp fonts, adjusted HUD layout).
- KMine: Usability and UI issues fixed (replaced fixed height and hidden overflow with min-height and auto overflow in web to fix layout clipping on small viewports, updated web and native opening instructions to 'Press H for Help', adjusted web controls max-width and padding, added SetProcessDPIAware and DPI scaling logic to native C to dynamically scale cells and fonts).
- KPac: Usability and UI issues fixed (Appropriate auto-opening size validated, SetProcessDPIAware applied for crisp native text, verified 'Press H for Help' overlay).
- KQuest: Usability and UI issues fixed (fixed clipped help text in native by adjusting height, adjusted control padding/margins, fixed web canvas max-width CSS to prevent stretching on devicePixelRatio, added explicit 'Press H for Help' notifications).
- KNote: Usability and UI issues fixed (added SetProcessDPIAware, AdjustWindowRect, text-rendering optimization, and F1/H hotkeys).
- KPass: Usability and UI issues fixed (shifted native UI down by 10px, increased auto-opening height to 540 for controls visibility, enabled CLEARTYPE_QUALITY, restricted H hotkey to non-EDIT focus, added ES_AUTOHSCROLL to generated passwords, enabled web optimizeLegibility, increased help text contrast, and adjusted iframe bounds).
- KMedia: Usability and UI issues fixed (Adjusted window size, enabled SetProcessDPIAware and CLEARTYPE_QUALITY for crisp text, added explicit Help button and F1 hotkey support).
- KNet: Usability and UI issues fixed (increased auto-opening size to 960x720, adjusted WM_SIZE bounds, enabled CLEARTYPE_QUALITY, added min-width to web, configured flex-wrap tabs, and added explicit 'h' help instructions).
- KZip: Usability and UI issues fixed (increased auto-opening size to 900x650, added z-index/background to sticky header, optimized text rendering, used negative font size for crisp native typography, and appended 'Press H for Help' instruction to loaded status).
- KPaint: Usability and UI issues fixed (synchronized web auto-opening size to 1024x768, fixed native sidebar flicker via WS_CLIPCHILDREN, enabled SetProcessDPIAware, disabled pixelated canvas rendering in web for crisp fonts, and fixed flexbox clipping and export dropdown positioning).
- KFarm: Usability and UI issues fixed (updated web min-height to fix layout clipping, added explicit 'H' hotkey and Almanac instructions, expanded native window size to 420x590, applied DEFAULT_GUI_FONT for crisp button text, and improved grid offset and spacing).
- KSnake: Usability and UI issues fixed (auto-opening size, added DPI awareness in native, layout scaling constraints in web, explicit help title in native).
- KTetris: Usability and UI issues fixed (Appropriate auto-opening size adjusted, crisp text and canvas scaling handled via native SetWorldTransform, visible controls & help instructions overlay validated).
- KSpace: Usability and UI issues fixed (auto-opening size adjusted to 340x520 in web, pixelated image-rendering applied to canvas, main menu buttons layout fixed, CSS overhauled for visibility within 320px width, and native C implemented custom NONANTIALIASED_QUALITY Courier New font for crisp monospaced rendering).
- KType: Usability and UI issues fixed (canvas scaling relative to devicePixelRatio applied in web for crisp text, automatic toggleHelp() on load added for first-time users, native currentMode default swapped to Help screen, SetProcessDPIAware validated).
- KSolitaire: Usability and UI issues fixed (auto-opening size in App.jsx and main.c, native window sizing via AdjustWindowRect, DPI awareness enabled in native, and devicePixelRatio scaling added to web canvas).
- KTerm: Usability and UI issues fixed (auto-opening size adjusted to 960x600, SetProcessDPIAware and AdjustWindowRect added to native for crisp text and layout).
- KSynth: Usability and UI issues fixed (added startup MessageBox instructions and Help toggle, fixed clipped layout via AdjustWindowRectEx, used negative font sizes for crisp UI text).
- KTask: Usability and UI issues fixed (added WS_CLIPCHILDREN to fix flickering, enforced WM_GETMINMAXINFO min size, clamped layout coordinates, pulled SPI_GETNONCLIENTMETRICS for crisp fonts, added EM_SETCUEBANNER for explicit opening instructions, and injected JS startup hook in web for visual instructions).
- KRogue: Usability and UI issues fixed (disabled web canvas image smoothing for sharp pixel art, added responsive web width, expanded native layout boundary to prevent overlap, added persistent native controls hint at bottom, and enforced NONANTIALIASED_QUALITY for crisp terminal font).
- KSys: Usability and UI issues fixed (adjusted auto-opening size to 1024x768 and centered on screen, added explicit Help buttons in both web header and native UI, and used negative font size with CLEARTYPE_QUALITY for crisp text).
- KTodo: Usability and UI issues fixed (enlarged auto-opening size to 800x600 in both platforms, updated status bar and bound 'H'/F1 for help, optimized web text-rendering and increased native font size, improved dark theme input contrast, and ensured dynamic layout scaling).
- KConverter: Usability and UI issues fixed (auto-opening size adjusted to 900x650 in web and 620x460 in native, explicit 'Press H for Help' controls validated, CSS font smoothing applied, and crisp text rendered using appropriate pixel size metrics).
- KGraph: Usability and UI issues fixed (applied text-rendering optimizeLegibility and font smoothing to web for crisp fonts, enabled CLEARTYPE_QUALITY for native fonts, improved custom dark-themed styling for select options in web, added visible Help button to main native layout, and widened native status bar layout).
- KTimer: Usability and UI issues fixed (enabled dynamic DPI scaling and S(x) macro in native C, expanded native starting RECT to 440x430 for label visibility, added visible bottom-centered Help badge in web, and optimized web window.resizeTo/moveTo for centered 460x580 rendering).
- KRead: Usability and UI issues fixed (added Help button to toolbar in web, optimized text-rendering, updated initial text to explicitly mention F1/H shortcuts, enforced Read-Only mode in native C, allowed WM_CTLCOLORSTATIC for theme styling on read-only edit, and added text margins to improve layout legibility).
- KJournal: Usability and UI issues fixed (auto-opening size adjusted to 1100x750, explicit help instructions added, crisp text styles verified, native console size increased and help title added).
- KBase: Usability and UI issues fixed (auto-opening size adjusted, DPI awareness and crisp text enabled, visible help controls and F1/H shortcuts added).
- KImage: Usability and UI issues fixed (auto-opening size to 1200x800, visible controls wrapping and padding, explicit instructions overlay and title, crisp text filtering).
- KHex: Usability and UI issues fixed (auto-opening size adjusted to 850x750, UI layout coordinates scaled, explicit 'Press h for Help' added in both platforms, and web min-width bounded).
- KCalc: Usability and UI issues fixed (added DPI scaling, crisp text, and adjusted native window bounds).
- KCalendar: Usability and UI issues fixed (auto-opening size to 1024x768, DPI_AWARENESS_CONTEXT applied in native C, help toast duration increased to 8s, layout wrapping fixed).
- KChart: Usability and UI issues fixed (DPI scaling, crisp negative font heights, and keyboard focus resolution).
- KChat: Usability and UI issues fixed (adjusted auto-opening size to 850x650, added 'H' hotkey help listener and explicit instructions in HTML, added Help button and adjusted layout/sizing in native).
- KChess: Usability and UI issues fixed (increased auto-opening size to 760x860, expanded control hitboxes, made opening instructions clearer, implemented DPI scaling and crisp text, re-calculated layout boundaries).
- KBBS: Usability and UI issues fixed (increased auto-opening size to 800x600 in native, added H hotkey for help modal in web version).
- KAudio: Usability and UI issues fixed (auto-opening size explicitly scaled to 1000x800, help instructions default to open, native font crispness fixed with negative font height, web canvas interaction gestures locked, and font-smoothing applied).
- KPong: Usability and UI issues fixed (auto-opening size adjusted to 950x750, DPI awareness and canvas scaling applied for crisp graphics, help hotkey/title added).
- KClock: Usability and UI issues fixed (expanded native layout and fixed text overlapping, implemented Segoe UI overarching crisp font, validated visible alarm controls and help instructions, synced web auto-opening bounds to match content).
- KiloOS Web UI (Shell & Apps): Usability and UI issues fixed (replaced transient help toast notification with a dedicated modal dialog for explicit instructions, adjusted modal width to 450px to accommodate instructions, improved help icon triggers).
- KDB: Usability and UI issues fixed (increased auto-opening size to 1100x800, expanded cryptic labels to explicit buttons, added dedicated Help button to the control bar, and enabled Segoe UI CLEARTYPE_QUALITY).
- KFortress: Usability and UI issues fixed (expanded native layout to 1024x700 with AdjustWindowRect, enabled DPI awareness and CLEARTYPE_QUALITY for crisp text, added functioning Help Modal and H hotkey).
- KColony: Usability and UI issues fixed (adjusted auto-opening size to 850x650 in web via resize and native via WS_OVERLAPPED bounds, enforced DPI awareness and viewport meta tags for crisp text, added explicit 'Press H for Help' instructions to web/native startup UI).
- KAlchemy: Usability and UI issues fixed (adjusted auto-opening size to 1200x800 in web and exact 800x570 in native via AdjustWindowRect, enabled SetProcessDPIAware for crisp text, added explicit "Press H for Help" startup instructions).
- KFont: Usability and UI issues fixed (adjusted native client area to 950x700 with AdjustWindowRect, fixed web flex-wrap and overlapping issues, added iframe resize postMessage support, validated crisp text and help shortcut).
- KMail: Usability and UI issues fixed (auto-opening size standardized to 900x600 in both platforms, explicit 'Press H for Help' text added in native, web font-smoothing applied, and select box-sizing fixed).
- KMandel: Usability and UI issues fixed (web auto-opening size meta tags added, help indicator styled as a translucent pill, controls panel widened to 240px, SetProcessDPIAware added to native, and native 'Press H for Help' rewritten to render over a solid rounded rectangle).
- KPing: Usability and UI issues fixed (auto-opening size adjusted to 850x650 in web, explicit 'Press H for Help' instructions added to terminal header/window title, input controls widened for clarity, and keyboard navigation (WS_TABSTOP) enabled in native).
- KConnect4: Usability and UI issues fixed (auto-size updated to w:580 h:720 in native, explicit H help instructions added instead of F1, hint shortcut moved to T, crisp web text rendering added).
- KScript: Usability subagent timed out after 8 minutes.
- KMaze: Usability and UI issues fixed (auto-opening size to 800x660, SetProcessDPIAware applied for crisp text, Help shortcut visibly added to UI across platforms).
- KScript: Usability and UI issues fixed (auto-opening size increased to 800x600, DPI awareness and scaling added in native C, F1 help shortcut explicitly noted, and crisp text rendering applied in web).
- KMine: Usability and UI issues fixed (updated App.jsx to point to kmine.html and set auto-opening size of 850x750, updated native main.c font heights to negative values for crisp text rendering).
- KQuest: Usability and UI issues fixed (adjusted auto-opening size to 1000x720 in App.jsx, enabled SetProcessDPIAware and negative font heights for crisp text in native).
- KNote: Usability and UI issues fixed (auto-opening size to 800x600, explicit Help button added to toolbar, welcome instructions updated, crisp text via negative font size).
- KPass: Usability and UI issues fixed (auto-opening size updated to 500x620, native font crispness fixed with negative font heights, precise client area sizing with AdjustWindowRect).
- KMedia: Usability and UI issues fixed (Appropriate auto-opening size adjusted to 950x700, added AdjustWindowRect to native client area, negative font heights implemented for crisp text).
- KNet: Usability and UI issues fixed (auto-opening size to 960x720, added AdjustWindowRect and min-size constraints, explicit Help button added in web and native, optimizeLegibility applied to web).
- KFarm: Usability and UI issues fixed (added SetProcessDPIAware and Coordinate mapping via SetMapMode for crisp UI in native, added devicePixelRatio canvas scaling for crisp vector shapes in web, and optimized window sizes to prevent clipping).
- KSnake: Usability and UI issues fixed (removed pixelated rendering from web for crisp canvas shapes, added optimizeLegibility, applied negative font height in native for crisp text, added SetWorldTransform DPI scaling and dynamic window resizing to native).
- KTetris: Usability and UI issues fixed (added optimizeLegibility and pixelated canvas rendering to web, fixed web click coordinates, used negative font heights for crisp native text, and fixed a native compilation issue).
- KSpace: Usability and UI issues fixed (auto-opening size updated to 340x520, removed pixelated scaling on web canvas for crisp fonts, added SetProcessDPIAware and negative font height in native for crisp rendering).
- KSolitaire: Usability and UI issues fixed (applied negative font height in native for crisp text, added explicit Help instruction to web window title, added padding-bottom to board container in web to prevent card clipping).
- KTerm: Usability and UI issues fixed (added tab title overflow truncation and explicit 'Type h or help' in web status bar, enabled negative font heights for crisp text in native, applied control padding via EM_SETMARGINS, added EM_SETCUEBANNER prompt cue, and updated native window title/tab banner to clarify help instructions).
- KRogue: Usability and UI issues fixed (auto-size updated to 1000x720, resolved H shortcut conflict by mapping Help to H and Leaderboard to T across web and native versions).
- KGraph: Usability and UI issues fixed (auto-opening size adjusted to 1024x768, added AdjustWindowRect to native, implemented negative font sizes for crisp rendering in native, and added explicit 'Press H for Help' to web startup toast).
- KContacts: Usability and UI issues fixed (added responsive grid layout for form, WS_CLIPCHILDREN for native window, and ES_WANTRETURN with EM_SETMARGINS for native edit control legibility).
- KRead: Usability and UI issues fixed (added F1 help shortcut to web, explicitly instructed F1/H usage on web/native startup UI, enabled crisp native text rendering via negative font heights, and adjusted native layout clipping via AdjustWindowRect).
- KJournal: Usability and UI issues fixed (added min-width/min-height layout constraints to web version, explicitly updated web instructions to state 'H' or F1, and mapped F1 to help; added explicit 'H' help instructions to all native terminal sub-menus and resolved sub-menu help routing in main.c).
- KImage: Usability and UI issues fixed (auto-opening bounds set to 1200x800, explicit Help button added, AdjustWindowRect used in native for proper client area, negative font heights implemented for crisp text, text-rendering optimized in web).
- KCalc: Usability and UI issues fixed (implemented EnumChildWindows to apply crisp font rendering to all native controls natively, handled WM_CTLCOLORSTATIC for readable labels against dark background, increased native layout padding with AdjustWindowRect, applied text-rendering optimizeLegibility to web version).
- KHex: Usability and UI issues fixed (auto-opening size adjusted to 850x750 in App.jsx, added explicit Help button to web app header, added AdjustWindowRect to fix native layout clipping, applied negative font height for crisp native text rendering, and added Help button to native interface).
- KChart: Usability and UI issues fixed (synchronized App.jsx auto-opening bounds to match 1024x768, added explicit Help button and 'H' instructions in web, added explicit Help button to native layout, used AdjustWindowRect for accurate native client dimensions, applied text-rendering optimizeLegibility for crisp text).
- KChat: Usability and UI issues fixed (updated App.jsx auto-opening bounds to 850x650, implemented native negative font height for crisp text rendering, and added a visible 'Help' button to web control bar).
- KChess: Usability and UI issues fixed (adjusted auto-opening size to 760x860 in App.jsx, implemented negative font height in native C for crisp text rendering, verified devicePixelRatio and AdjustWindowRect usage).
- KAudio: Usability and UI issues fixed (auto-opening size explicitly scaled to 1000x800, help instructions default to open, native font crispness fixed with negative font height, web canvas interaction gestures locked, and font-smoothing applied).
- KBBS: Usability and UI issues fixed (implemented negative font height in native C for crisp text, added AdjustWindowRect to fix UI clipping, fixed native mouse selection offsets, updated native title with Help instruction, and optimized web text rendering).
- KPong: Usability and UI issues fixed (synchronized native window bounds to 950x750 matching App.jsx, added optimizeLegibility to web version for crisp font rendering, verified native font heights and web devicePixelRatio scaling, confirmed clear help instructions).
- KClock: Usability and UI issues fixed (updated native CreateFontA parameters to negative heights for crisp DPI scaling, added text-rendering: optimizeLegibility to web layout for crisp typography, and verified App.jsx bounds 950x650 match content size).
- KDB: Usability and UI issues fixed (updated App.jsx bounds to 1100x800, added optimizeLegibility to web HTML, applied negative font heights, SetProcessDPIAware, AdjustWindowRect, and WS_CLIPCHILDREN to native C version).
- KiloOS Web UI: Usability and UI issues fixed (replaced transient startup help toast notification with a persistent modal dialog in App.jsx to ensure visibility).
- KFortress: Usability and UI issues fixed (implemented negative font heights in native C for crisp DPI scaling, added optimizeLegibility to web CSS, and updated web UI Help button to explicitly display the 'H' hotkey).
- KColony: Usability and UI issues fixed (updated App.jsx auto-opening bounds to 850x650, implemented negative font heights and AdjustWindowRect in native C, and applied optimizeLegibility CSS to the web interface).
- KAlchemy: Usability and UI issues fixed (updated App.jsx auto-opening bounds to 1200x800, added negative font heights for crisp text in native C, applied devicePixelRatio canvas scaling for crisp visuals in web canvas, and added optimizeLegibility).
