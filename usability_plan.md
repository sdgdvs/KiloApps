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

**Target App:** KRogue
**Status:** In Queue
**Current Phase:** In Queue

## Round-Robin Continuous Improvement Queue (NEVER STOP — loop forever via cron)
Pick the top app from this list, identify and fix usability and UI problems (update BOTH web and native versions if applicable), and then move it to the very bottom of the list. Complete exactly ONE app per cron turn (using a single subagent if needed), commit your changes, and then stop your execution. Let the recurring cron schedule wake you up to process the next app. When you reach the end of the list, you'll be back at the top — the cron cycle never ends. If new apps appear, add them to the queue.

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

## Progress Log

> 📁 **Archived Progress Log**: Older entries have been archived to [archive/usability_plan_archive.md](archive/usability_plan_archive.md) to preserve token efficiency.

- KTask: Usability and UI issues fixed (fixed missing .toast.show CSS restoring non-blocking cybernetic toast notification system with startup welcome prompt and click-to-dismiss [✕] button, added 1-click quick filter chips [All Tasks, ⚡ High CPU, 💾 High RAM, ⭐ High Priority, ＋ Spawn Demo Task], added search clear [✕] button with Esc/Enter handling, added visual keyboard shortcut badges across tabs and header buttons [[1] Processes, [2] Performance, [3] Summary, CSV [E], JSON [J], Help [F1], Refresh [R]], added keyboard navigation with Arrow Up/Down across process cards, double-click to Deep Inspect, P to cycle priority, E/J for exports, and backdrop click dismissals for modals in web; added ShowNativeToast temporary status bar toast system with startup welcome guide and non-blocking export/priority feedback, updated native buttons with explicit shortcut badges [Refresh [F5], Priority [P], Inspect [I], CSV [C], JSON [J], Help [F1], End Task [Del]], added InspectEditProc for Escape dismissal of Deep Inspector, implemented global message loop accelerator interception for F1/H/F5/R/Del/I/P/C/J/Esc preventing child control swallowing, updated window title with shortcut hints, and verified clean compilation across both builds).

- KSynth: Usability and UI issues fixed (integrated cybernetic non-blocking toast notification system with startup welcome prompt and click-to-dismiss [✕] button, added 1-click Quick Starter preset and demo chips [Beat Demo, Arp Demo, C-Major Chord, Randomize, and Presets 1-6], added visual keyboard shortcut badges across all toolbar, sequencer, and arpeggiator buttons [▶ Play Seq [Space], Clear [C], Random [R], Latch [L], 💾 Save [Ctrl+S], 📂 Load [Ctrl+O], 🔊 WAV [Ctrl+E], 🔇 Panic [Esc], ❓ Help [F1]], added hotkeys for Space/P, C, R, L, 1-6, Ctrl+S, Ctrl+O, Ctrl+E, synced interactive piano key lighting during sequencer and arpeggiator playback in web; built interactive on-screen virtual piano keyboard control with mouse click trigger and visual key highlighting in native C, added edit control subclassing for Enter-to-play and Esc-to-unfocus, implemented message loop accelerators for Space/P/E/1-6/Z/X/Esc, added live status HUD bar and shortcuts guide, updated window title with shortcut hints, and verified clean compilation across both builds).

- KTerm: Usability and UI issues fixed (integrated cybernetic non-blocking toast notification system with startup welcome prompt and click-to-dismiss [✕] buttons, added 1-click Quick Starter command chips [help, dir, sysinfo, date, whoami, alias, export-log, clear], added font size zoom buttons [A- / A+] with [Ctrl++/Ctrl--/Ctrl+0] hotkeys, added [Ctrl+Tab] and [Ctrl+1..8] direct tab switching shortcuts with tab numbering and double-click tab renaming, made Help modal command table rows clickable to insert/run commands directly, added [Ctrl+S] session log export accelerator and Escape clear in web; added native Windows toolbar buttons [+ Tab, Close, Clear, Export, Help (F1)] alongside tab control, added bottom status bar showing keyboard shortcuts guide and real-time toast feedback, implemented message loop accelerators for Ctrl+T, Ctrl+W, Ctrl+S, Ctrl+Tab, Ctrl+1..8, and Escape, updated cue banner to persist when focused, dynamic window title synchronization with active tab and directory path, and verified clean compilation across both builds).

- KSpace: Usability and UI issues fixed (integrated click-to-dismiss toast notifications with [✕] button and real-time feedback for all 7 skills and cooldowns, implemented full pointerdown/pointermove/pointerup mouse and touch drag steering with continuous firing, added backdrop-click resume to Pause menu in web; added native floating toast banner system with auto-fade and click-to-dismiss for startup guidance, save/load, exports, and audio mute [M], implemented mouse drag steering and left-click continuous firing in WM_LBUTTONDOWN and WM_MOUSEMOVE, added clickable 7-button interactive skill toolbar at bottom matching web, fixed 15px vertical click hitbox mismatches on main menu and pause screens, added backdrop-click pause resume, updated window title with shortcut hints, and verified clean compilation across both builds).

- KTetris: Usability and UI issues fixed (fixed start menu keybinding collision where [K] and [V] overlapped at same Y coordinate, redesigned start menu with hoverable cards and precise hitboxes, added interactive toast notification system with [✕] click-to-dismiss and startup guide, implemented functional Hold Reserve piece [C/Shift] with sound/toast/replay recording, added in-game interactive HUD buttons for Pause [P], Help [H/F1], and Hold reserve slot, added board touch/mouse steering with left/right/rotate/drop zones, redesigned stylized Game Paused screen with Resume and Main Menu buttons, added clickable buttons for Help/Leaderboard/Keybinds/Game Over, synchronized 520x720 window bounds in App.jsx to prevent screen overflow in web; added native floating toast banner system with continuous fade timer, implemented UseHoldPiece in native C, fixed start menu 3D button drawing without text overlap, added board click steering and touch navigation in WM_LBUTTONDOWN, added stylized Pause dialog with click-to-resume, handled VK_ESCAPE and Hold keys, updated window title with shortcut hints, and verified clean compilation across both builds).
