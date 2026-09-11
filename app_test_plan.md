# App Testing & UI Audit Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** Multiple worker agents + directors operate on this repo on overlapping schedules. You are the **App Tester** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`app_test_plan.md`).** Read but NEVER edit:
  - `app_fix_plan.md` (QA agent), `app_work_plan.md` (Feature Expander — paused), `game_content_plan.md` (Games — paused), `new_app_plan.md` (Creator), `usability_plan.md` (Usability)
- **Shared file `KiloOS/src/App.jsx`** — Do NOT edit unless fixing a broken app registration entry.
- **`KiloOS/src/index.css`** — Do NOT edit.
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing:** After editing any app's HTML file, verify it renders. After editing `App.jsx` → `npm run build` in `KiloOS/`.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`. If build fails, fix immediately.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **Logging discipline:** Keep this plan file concise. Use the compact report format specified below.

---

## ⏱️ TURN SCOPING & TERMINATION (CRITICAL — READ EVERY TURN)

**Single-Item-Per-Turn Rule:**
- Each cron trigger = ONE turn. Audit exactly ONE app from your queue, then STOP.
- "Loop forever" means the CRON loops forever across turns, NOT that you loop within a single turn.
- After committing and pushing your work for ONE app, STOP CALLING TOOLS immediately.

**Graceful Termination Checklist (do this EVERY turn before stopping):**
1. Audited one app
2. Updated this plan file with test report
3. Fixed trivial issues in-line (if any)
4. Committed and pushed
5. STOP — call no more tools

---

## ⚠️ DIRECTOR NOTE: WHY THIS AGENT EXISTS

Apps have accumulated many non-functional UI elements — buttons with no handlers, modals that won't open, menu items that throw errors, import/export features that silently fail, settings that don't persist. The QA agent catches code-level bugs (memory leaks, XSS, buffer overflows) but does NOT systematically interact with every UI element. This agent fills that gap.

**Your job is to be a THOROUGH MANUAL TESTER, not a code auditor.** The QA agent already does code auditing. You focus on the USER EXPERIENCE: does every visible button, link, control, modal, tab, dropdown, and feature ACTUALLY WORK when a user interacts with it?

---

## Testing Methodology (follow this for EVERY app)

### Step 1: Inventory All UI Elements (WEB VERSION ONLY — focus on web HTML apps)
Read the app's web HTML file (`KiloOS/public/apps/k[name].html`) and identify EVERY interactive element:
- Buttons (including toolbar, modal, and inline buttons)
- Input fields, dropdowns, checkboxes, sliders
- Tabs and navigation controls
- Modals and dialogs (do they open? do they close? do their buttons work?)
- Import/Export features (CSV, JSON, TXT, etc.)
- Keyboard shortcuts listed in help
- Settings that claim to be saved/loaded

### Step 2: Trace Each Element's Handler
For each interactive element, trace its event handler in the JavaScript:
- Does the `onclick`/`addEventListener` reference a function that EXISTS?
- Does that function DO SOMETHING MEANINGFUL, or is it a stub/empty?
- Does it reference variables or DOM elements that exist?
- Will it throw an error on typical input?

### Step 3: Classify Issues
For each issue found, classify it:
- **🔧 FIXED** — You fixed it in-line (≤5 lines, single file, obvious fix like removing a dead button or wiring up a missing handler)
- **❌ BROKEN** — Complex issue, logged for QA agent to fix. Describe the symptom clearly.
- **⚠️ STUB** — Feature is declared in UI but implementation is empty/placeholder. Note whether it should be removed or completed.
- **✅ OK** — Element works as expected.

### Step 4: Fix Trivial Issues In-Line
If you find a button referencing a nonexistent function, you may:
- **Remove the button** if the feature doesn't exist
- **Wire it up** if the function exists but isn't connected (e.g., typo in handler name)
- **Add a basic stub** like `alert('Not yet implemented')` only if the feature is partially built

**DO NOT** do large rewrites. If a fix would be >5 lines or touch multiple functions, log it for QA.

### Step 5: Write Compact Test Report
Add an entry to the Test Reports section below using this format:
```
- **KAppName**: [PASS ✅ | ISSUES FOUND ⚠️] (N issues, M fixed inline)
  - ✅ Core functionality works (describe briefly)
  - ❌ "Export CSV" button: onclick calls exportCSV() which doesn't exist
  - ⚠️ "Settings" modal: opens but Save button is a no-op stub
  - 🔧 FIXED: Removed orphan "Share" button with no handler
```

---

**Target App:** KMandel
**Status:** Next in queue

## Round-Robin Testing Queue (NEVER STOP — loop forever)
Pick the top app, audit it, write a test report, move it to bottom. One app per turn.

- KMandel
- KMatch3
- KMaze
- KMech
- KMedia
- KMine
- KMines
- KMystery
- KNet
- KNote
- KPac
- KPad
- KPaint
- KPass
- KPing
- KPong
- KQuest
- KRadio
- KRead
- KReversi
- KRogue
- KScript
- KSimon
- KSnake
- KSolitaire
- KSpace
- KStarship
- KStellar
- KSudoku
- KSynth
- KSys
- KTask
- KTerm
- KTetris
- KTimer
- KTodo
- KTowers
- KTrader
- KType
- KVault
- KVoid
- KWizard
- KWords
- KZip
- K2048
- KAlchemy
- KAsteroids
- KAudio
- KBBS
- KBase
- KBreakout
- KBudget
- KCalc
- KCalendar
- KChart
- KChat
- KChess
- KClock
- KColony
- KColosseum
- KColor
- KConnect4
- KContacts
- KConverter
- KCyber
- KDB
- KDarts
- KDragon
- KFarm
- KFlash
- KFont
- KFortress
- KFreecell
- KGo
- KGraph
- KHabit
- KHangman
- KHex
- KImage
- KJournal
- KMail

## Test Reports

- **KMail**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core functionality works (multi-tab email suite with folders Inbox/Starred/Sent/Drafts/Trash, starred priority tagging, tags categorization, auto-saving drafts, email reply thread generator, PBKDF2/AES-GCM encrypted email composer and decryption engine, full-text subject/sender/body search with tag filtering, single-message EML and Markdown export, full JSON mailbox backup export and import, and non-blocking toast notifications).
  - 🔧 FIXED: Global keyboard listener checked bare `e.key` (`c`, `n`, `1`..`5`, `h`, `r`, `t`, `e`, `m`) without checking `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking browser accelerators: `Ctrl+C` (copying text in emails hijacked to open a new compose tab), `Ctrl+N` (new window hijacked to compose), `Ctrl+1`..`Ctrl+5` and `Alt+1`..`Alt+5` (browser tab switching hijacked to change mail folders), `Ctrl+H` (browser history hijacked to open Help modal), `Ctrl+R` (browser refresh hijacked to reply), `Ctrl+T` (new tab hijacked to open Tag modal), and `Ctrl+E` (address bar focus hijacked to export EML). Added modifier key guards across all shortcuts.
  - 🔧 FIXED: Keyboard shortcuts continued to fire in the background underneath active modals (Help, Tag, and Trash modals), manipulating folders and stacking compose tabs while reading dialogs. Suppressed background hotkeys whenever any modal is displayed.
  - 🔧 FIXED: Sidebar buttons displayed `<kbd>I</kbd>` and `<kbd>O</kbd>` shortcuts for "Import JSON" and "Export JSON", but neither key was wired in the `keydown` listener, leaving both promised hotkeys dead. Wired `I` to trigger mailbox file upload and `O` to trigger backup JSON export, and documented both in the Help modal table.
  - 🔧 FIXED: Clicking "Empty Trash" (`doEmptyTrash()`) only called `renderList()` if `currentFolder === 'trash'`. If emptied from Inbox or another folder, the Trash folder badge (`#b-trash`) never updated, falsely displaying stale item counts; and any open tabs displaying messages purged from Trash remained open as orphan ghost views. Updated `doEmptyTrash()` to close tabs for deleted messages and refresh folder badges across all views.
  - 🔧 FIXED: Users could add tags to emails, but had no way to remove an existing tag. Added `removeTag()` with click-to-delete `✕` buttons on email header tags, and made clicking existing tag chips in the Tag modal toggle/remove them.
  - 🔧 FIXED: In `exportSingleEml()`, `exportSingleMd()`, and `exportJson()`, download anchor elements were clicked without being attached to `document.body` (`a.click()`), causing file downloads to fail silently in Firefox and sandboxed iframe environments. Attached anchors to DOM before clicking and cleanly removed them afterwards.
  - 🔧 FIXED: Search input used `onkeyup="handleSearch()"` which ignored mouse context-menu paste, cut, and drag-and-drop inputs; pressing `Escape` cleared search text without blurring `#search-box` (leaving users trapped in input mode where hotkeys were blocked); and saving a draft with an empty recipient stored `'draft@kilo.os'` as the recipient, which pre-populated into the "To:" field upon reopening. Switched to `oninput`, added blur on Escape, removed dummy recipient fallback, and added recipient validation prior to sending.

- **KJournal**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core functionality works (daily journaling workspace with real-time word/character count and estimated reading time, daily writing word goal progress bar, 6 quick-starter prompt chips, 6 guided reflection templates in library Morning/Evening/Gratitude/Goals/Stoic/BrainDump with replace and append options, 6 mood selectors 😀/😊/😐/😔/⚡/🧘 with analytics breakdown and percentage gauges, interactive mini calendar navigator with entry indicators and month browsing, hashtag cloud extraction with filter toggling, search query filtering, writing streak & longest streak tracker, PIN lock overlay security system, and multi-format data export JSON/Markdown/TXT and JSON backup import).
  - 🔧 FIXED: In the mini calendar widget, if the user was viewing today's entry and browsed past/future months using the prev/next month buttons, clicking the "Today" button (`•` / `todayCalMonth()`) failed to re-render the calendar to the current month because `selectDateWithAutoSave(todayStr)` returned early without calling `renderCalendar()` when `selectedDate === dateStr`. Added explicit calendar re-rendering when the target date matches the currently selected date.
  - 🔧 FIXED: Global keyboard listener checked bare `e.key.toLowerCase() === 'h'` and `e.key >= '1' && e.key <= '6'` without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, intercepting browser accelerators: `Ctrl+H` (browser history hijacked to open KJournal Help modal) and `Alt+1`..`Alt+6` (tab switching hijacked to select moods). Added proper modifier guards across all shortcuts.
  - 🔧 FIXED: Global keyboard shortcuts (`H`, `1`..`6`, `Ctrl+S`, `Ctrl+N`, `Ctrl+T`, `Ctrl+F`) continued to fire in the background while modal dialogs (Help, Analytics, Settings, Import/Export, Confirm) were open, changing entry moods, jumping dates, or stacking dialogs while reading modal contents. Added modal suppression guard across all background shortcuts.
  - 🔧 FIXED: When entering PIN setup (`create_1` or `create_2` mode via Settings), the PIN keypad overlay lacked a visible "Cancel" button, trapping mouse and touch-only users who changed their minds unless they used a physical keyboard to press Escape. Added a responsive Cancel button to the PIN card during creation modes.
  - 🔧 FIXED: `exportData()` exported journal entries from memory/storage without checking `isDirty`, causing entries typed or edited within the 1800ms auto-save debounce window to be omitted or outdated in exported JSON, Markdown, or TXT backup files. Added automatic save invocation prior to export when unsaved changes exist.
  - 🔧 FIXED: In `deleteCurrentEntry()`, deleting an entry left the mood picker highlighted on the deleted entry's mood rather than resetting to the default `😊`. Added mood reset to default upon entry deletion.
  - 🔧 FIXED: Pressing `Escape` while focused on `#searchInput` did not clear or blur the search field, and `#storageInfo` in the Settings modal displayed static text without showing the active entry count and approximate LocalStorage usage. Added Escape search clear/blur and dynamic storage statistics calculation upon opening Settings.

- **KImage**: ISSUES FOUND ⚠️ (8 issues, 8 fixed inline)
  - ✅ Core functionality works (interactive image studio with slideshow playlist, GPU/Canvas adjustments Brightness/Contrast/Saturation/Blur, 8 cinematic filter presets, 3x3 Spatial Convolution matrix engine with 9 presets and custom weights/divisor/bias/channel targeting, 90° rotations and H/V flipping, aspect-constrained interactive crop tool, dimensions resize with aspect lock, freehand annotation brush with color picker and radius slider, sub-sampled multi-channel RGB histogram, EXIF camera metadata inspector, multi-format export PNG/JPEG/WEBP/BMP, and keyboard accelerators).
  - 🔧 FIXED: Global keyboard listener checked bare `e.key` (`h`, `o`, `c`, `d`, `f`, `1`..`5`, `+`, `-`, `0`, `ArrowLeft`, `ArrowRight`, `Space`, `[`, `]`) without verifying `!ctrl && !e.altKey`, intercepting browser accelerators: `Ctrl+H` (history hijacked to open Help modal), `Ctrl+F` (find in page hijacked to toggle fullscreen), `Ctrl+C` (copy hijacked to enable crop), `Ctrl+D` (bookmark hijacked to enable brush), `Ctrl+1` through `Ctrl+5` (browser tab switching hijacked to switch KImage tabs), `Ctrl+0` (browser zoom reset), and `Alt+Left`/`Alt+Right` (browser back/forward navigation). Added proper modifier guards.
  - 🔧 FIXED: Keyboard shortcuts continued to fire in the background underneath an active Help modal (`#helpModal`), toggling slideshows, rotating images, activating crop/draw, and changing zoom while reading documentation. Suppressed background hotkeys while `#helpModal` is displayed.
  - 🔧 FIXED: In `ui.fileInput.onchange`, `ui.fileInput.value = ''` was never reset after loading images, causing subsequent attempts to reload the same image file to fail silently because the `change` event would not fire. Added `ui.fileInput.value = ''` reset.
  - 🔧 FIXED: In `btnExportDownload`, the created download anchor element was clicked directly without being attached to `document.body` (`link.click()`), causing image downloads to fail silently in Firefox and sandboxed iframe environments. Attached anchor to DOM before clicking and cleanly removed it afterwards.
  - 🔧 FIXED: In the Resize tab, `#inputHeight` lacked an `oninput` handler, preventing bidirectional aspect-ratio synchronization when typing a new height. Furthermore, `syncAspectHeight()` calculated aspect ratios using unrotated image dimensions (`item.img.height / item.img.width`) rather than active canvas dimensions (`ui.mainCanvas.height / ui.mainCanvas.width`), desynchronizing calculations after 90° or 270° rotations. Added bidirectional aspect ratio calculation based on active canvas dimensions.
  - 🔧 FIXED: Numeric inputs (`#inputWidth`, `#inputHeight`, `#kernelDivisor`, `#kernelBias`, matrix weights `#k00`..`#k22`) lacked `Enter` key listeners, forcing users to click action buttons with the mouse. Wired `Enter` key handlers to trigger resize and convolution matrix application respectively.
  - 🔧 FIXED: In the Annotation Brush tool, single mouse/pointer clicks without drag movements produced no visible markings because `pointerdown` did not call `renderCanvas()` and single-point Canvas2D paths render nothing with `stroke()`. Added single-point arc/circle rendering, called `renderCanvas()` on `pointerdown`, and added an early return to `renderHistogram()` when the right Inspector panel is hidden to eliminate CPU lag during drawing and slider adjustments.
  - 🔧 FIXED: Activating Crop and Draw simultaneously caused UI conflict where crop overlay pointer capture obstructed drawing. Added mutual exclusion so enabling one tool deactivates the other, added WAI-ARIA `role="tablist"`/`role="tab"` with `ArrowLeft`/`ArrowRight` navigation across tool tabs, and synchronized live canvas dimensions to the EXIF & Property Inspector and Resize inputs upon rotation, cropping, and resizing.

- **KHex**: ISSUES FOUND ⚠️ (8 issues, 8 fixed inline)
  - ✅ Core functionality works (7-tab hex & binary data suite: Base Converter & 32-bit Bitfield Manipulator, 16-byte/row interactive Hex Viewer & Buffer with ASCII column and offset inspector, Shannon Entropy randomness meter & File Magic Header Signature Dissector across 21 formats, Pattern Search & Replace with ASCII and hex modes, Checksum & Hash suite with 8/16/32-bit sums, XOR8, CRC32, MD5, and SHA-256, Byte Operations with Invert/XOR/Fill/16-bit & 32-bit word swap/Reverse, Formatted HexDump/C Array/Python bytes/Base64/RAW binary export, Web Audio & kinematic 4-layer particle engine).
  - 🔧 FIXED: Global keyboard listener checked bare `e.key` without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking browser accelerators (`Ctrl+1`..`Ctrl+7` for browser tab switching, `Ctrl+H` for history, and `Ctrl+E` for address bar search). Added modifier key guards and modal suppression so shortcuts do not fire in the background when the Help dialog is open.
  - 🔧 FIXED: In Search & Replace, clicking "Find Next" (`executeSearch()`) always selected and highlighted `matches[0]` on every click instead of cycling forward through subsequent matches, and did not scroll the hex container to make the highlighted match visible. Implemented search state tracking (`lastSearchPattern`, `currentSearchIdx`) to cycle through occurrences, display active match position (e.g. `[1/5]`), and scroll the hex view directly to the active match.
  - 🔧 FIXED: Text inputs `#searchPattern`, `#replacePattern`, and `#opArg` lacked `Enter` key listeners, forcing users to click action buttons with the mouse. Wired `Enter` key handlers to execute search, replace all, and apply byte operations respectively.
  - 🔧 FIXED: Tab buttons had WAI-ARIA `role="tab"` and `role="tablist"` attributes but lacked keyboard arrow navigation. Implemented standard `ArrowLeft` / `ArrowRight` focus and selection cycling across the 7 suite tabs.
  - 🔧 FIXED: In `handleFileSelect()`, `fileInput.value = ''` was never reset after loading, causing subsequent attempts to reload the same file to fail silently because the `change` event would not trigger. Added input value reset and synchronized byte offset selection (`selectByte(0)`) on file and sample data loads.
  - 🔧 FIXED: In `computeHashes()`, empty buffers (`!binaryBuffer.length`) triggered an early return without clearing or updating hash displays, leaving previous digests visible on screen. Reset all sums, CRC32, MD5 (`d41d8cd9...`), and SHA-256 (`e3b0c442...`) to standard empty hash representations.
  - 🔧 FIXED: `computeEntropyAndDistribution()` and `dissectHeader()` coerced empty buffers into `new Uint8Array([0,0,0,0])`, misleadingly reporting "1 Unique Bytes in 4 Bytes Total" with false 0x00 counts and "Raw Binary" header signature. Accurately handle 0-byte buffers as "Empty Buffer" with zeroed metrics and descriptive empty-state notices.
  - 🔧 FIXED: `applyByteOperation()` executed operations against 0-byte buffers with misleading success toasts. Added buffer validation to guard operations when empty with informative warning toasts, and persisted modified buffer text in `localStorage`.

- **KHangman**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core gameplay works (Classic hangman with 10 built-in categories plus custom word input, 20-stage Campaign with escalating word length and strike limits ending in Polymath Grandmaster challenge, 60s Time Attack Blitz mode with time extensions, 5 active skills Vowel Reveal/Consonant Radar/Strike Shield/Freeze Timer/Bomb Nuke, 3D gallows with procedural wood grain, animated character sprite with blinking and facial states, kinematic rope/noose physics with wind sway, loss ghost floating animation, multi-layer particle explosion engine, and Web Audio synthesizers).
  - 🔧 FIXED: Critical regression from Loop 8 graphics update where 12 essential JavaScript functions (`showHelp`, `hideHelp`, `initAudio`, `toggleMute`, `playSound`, `loadStats`, `saveStats`, `updateStatsDisplay`, `resetStats`, `saveGameState`, `loadGameState`, `updateSkillButtons`) were accidentally overwritten and missing from the file. On initial page load, `loadStats()` threw `ReferenceError: loadStats is not defined`, crashing script execution and preventing `initGame()`, keyboard listeners, and canvas rendering from starting. Restored all 12 functions with full Web Audio synthesis, statistics tracking, game state persistence, and skill button state management.
  - 🔧 FIXED: In the global `keydown` listener, single-character letter check was written as `keyUpper >= 'A' && keyUpper <= 'Z'` without verifying `e.key.length === 1`. Non-character keys like `Shift`, `Enter`, `Backspace`, `ArrowUp`, `CapsLock`, and `Delete` evaluated to true (e.g. `'SHIFT' >= 'A' && 'SHIFT' <= 'Z'`), passing multi-letter strings to `guess()`, adding phantom strike errors against the player, and causing unnecessary strikes when using Shift or standard navigation keys. Added `e.key.length === 1` guard.
  - 🔧 FIXED: Background keyboard shortcuts (letter guessing, skills V/H/S/F/B) continued to fire underneath an active Help modal, executing guesses and using power-ups while reading instructions. Suppressed background keystrokes while `#help-modal` is displayed.
  - 🔧 FIXED: Help modal could not be dismissed via `Escape` or by clicking the dark modal backdrop overlay, and in-game shortcuts `?` and `F1` were unhandled. Added backdrop click dismissal, `Escape` key close, and wired `?`/`F1` to open the Help guide.
  - 🔧 FIXED: `#custom-words` text input lacked an `Enter` key listener, forcing users to click the "Play Custom" button manually before typing letters. Added `Enter` key handler to immediately start custom games and blur input focus.
  - 🔧 FIXED: Clicking the Strike Shield button (`#shield-btn`) or pressing `S` triggered a disruptive browser `alert()`, pausing the browser event loop mid-game. Replaced alert with smooth visual status text (`msgEl.innerText`) and shield sound effect.
  - 🔧 FIXED: In `startBlitzTimer()`, freezing the timer in Freeplay or Campaign mode did not clean up the interval upon timer expiration, and Freeplay mode lacked a countdown readout for the 15-second freeze effect. Added freeze countdown text to Freeplay and properly cleared the freeze interval once expired in non-Blitz modes.

- **KHabit**: ISSUES FOUND ⚠️ (8 issues, 8 fixed inline)
  - ✅ Core functionality works (daily habit tracking dashboard with progress bar and 7-day visual history chart, habit creation with name, description, categories Health/Work/Personal/Other, and target streak milestone masteries 7/30/90 days, daily completion checkmarking with active fire flicker animations, delete animations with confirmation, real-time statistics modal with active/mastered habit counts, longest streak, and most consistent category analysis, accent color theme customization Purple/Orange/Blue/Green with live preview, search filter, sort options Alphabetical/Highest Streak, JSON data backup import and export, and localStorage persistence).
  - 🔧 FIXED: In global `keydown` listener, pressing `Space` or `Delete` while typing in the `#search-input` field (or any text input) was intercepted by `e.preventDefault()`, toggling or deleting the selected habit instead of allowing spaces or deletion in the search box. Added input focus check (`isInputFocused`) to prevent hijacking input keystrokes.
  - 🔧 FIXED: Global `keydown` shortcuts fired in the background when the Help modal (`#help-modal`) or Statistics modal (`#stats-modal`) was open (only `#add-modal` and `#settings-modal` were guarded), allowing background habit toggling, deletion, and `Ctrl+N` dialog stacking. Added `anyModalActive` guard across all four modals.
  - 🔧 FIXED: None of the four modals (Add Habit, Settings, Statistics, Guide) supported `Escape` key dismissal. Added global `Escape` handler to close any active modal or clear and blur the search box.
  - 🔧 FIXED: Habit cards had hover and `.selected` CSS styles, but lacked a click event handler to set `selectedHabitIndex`, meaning habits could only be selected using Arrow keys and clicking cards with a mouse did not select them for subsequent Space/Delete hotkey use. Added click-to-select event listeners to all habit cards.
  - 🔧 FIXED: In `renderHabits()`, habit name, category, and description were interpolated directly into `card.innerHTML` without escaping, exposing the app to XSS / markup corruption when creating or importing habits with special HTML characters. Added `escapeHtml()` sanitization across all interpolated text.
  - 🔧 FIXED: `importBtn` parsed JSON arrays without field validation or defaults, so imported habits lacking a `completions` array caused `renderHabits()` to throw `TypeError: Cannot read properties of undefined (reading 'includes')` and crash the entire UI. Added comprehensive property normalization and array fallbacks during JSON import.
  - 🔧 FIXED: Help modal documentation stated "Use Import/Export to save your habits as a CSV file", contradicting the JSON import/export buttons and file parser. Corrected documentation to specify JSON files, and upgraded `exportBtn` from a data URI to standard `Blob` and `URL.createObjectURL` with deferred revocation.
  - 🔧 FIXED: `calculateStreak` used `Math.ceil(diffTime / 86400000)` without calendar-day rounding or completion deduplication, erroneously breaking active streaks on 25-hour Daylight Savings Time transitions. Switched to `Math.round` and `Set` deduplication, added hover tooltip descriptions to daily history bars, and expanded search filtering to match habit category and description fields.

- **KGraph**: ISSUES FOUND ⚠️ (8 issues, 8 fixed inline)
  - ✅ Core functionality works (interactive function, polar, and parametric graphing suite with real-time expression evaluation, Cartesian y(x) with derivative overlay and Simpson's rule definite integral shading, Polar r(θ) with concentric range circles and radial spokes, Parametric (x,y)(t) curves, bisection root finder, curve intersection finder, 18-preset library across Cartesian, Polar, and Parametric curves, hover crosshair coordinate & numerical derivative readout, pan & zoom canvas, PNG snapshot export, CSV data points export, and JSON configuration save/load).
  - 🔧 FIXED: In `updateReadout()`, `document.getElementById('readoutX').innerText = currentHoverX.toFixed(4)` directly accessed an element that was destroyed whenever switching to Polar or Parametric modes (`headerLine.innerHTML = ...`), causing `TypeError: Cannot set properties of null (setting 'innerText')` on subsequent mouse movements upon switching back to Cartesian mode and freezing canvas mouse updates. Removed the redundant `readoutX` lookup and relied on `headerLine.innerHTML`.
  - 🔧 FIXED: In initialization, `replotAll()` was never invoked, leaving `compiledCartesian = []` and causing the default curves (`sin(x)`, `cos(x)`) not to render on initial page load until the user clicked Plot. Added `replotAll()` to the startup sequence.
  - 🔧 FIXED: Global `keydown` listener checked bare `e.key` without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking browser accelerators (`Ctrl+C` switching mode to Cartesian instead of copying text, `Ctrl+P` switching to Polar instead of printing, `Ctrl+H` opening Help instead of browser history, `Ctrl+R` resetting view instead of reloading, `Ctrl+1..4` switching tabs instead of browser tabs, `Ctrl+0`, `Ctrl++`, `Ctrl+-`). Added modifier key guards and modal background suppression.
  - 🔧 FIXED: In `compileMathExpr()`, input expressions lacked replacement for the Greek theta character `θ` and uppercase `X`, despite `θ` being shown in the polar input placeholder and Help documentation. Entering `θ` or `X` caused `ReferenceError: θ is not defined` and expression compilation failure. Added `expr.replace(/θ/g, 'x')` and `expr.replace(/\bX\b/g, 'x')`.
  - 🔧 FIXED: In `exportPNG()`, `exportCSV()`, and `exportJSON()`, download anchor elements were clicked without being attached to `document.body`, failing silently in Firefox and sandboxed iframe environments, and `URL.revokeObjectURL(url)` was invoked synchronously on the immediate next line before the browser could begin downloading. Attached anchors to the DOM, removed them cleanly after click, and deferred object URL revocation.
  - 🔧 FIXED: `exportCSV()` hardcoded Cartesian headers and data (`x,y1..y5`) regardless of the active plot mode, causing Polar and Parametric exports to output blank or irrelevant Cartesian data. Added dedicated CSV export branches for Polar (`theta_rad,theta_deg,r1..r3`) and Parametric (`t,curve1_x,curve1_y,curve2_x,curve2_y`) modes.
  - 🔧 FIXED: `exportJSON()` and `importJSON()` omitted custom polar/parametric range and grid settings (`polarMinTh`, `polarMaxTh`, `showPolarGrid`, `paramMinT`, `paramMaxT`), resetting them to defaults upon reloading configurations, and `importJSON()` failed to reset `event.target.value = ''` (preventing re-importing the same configuration file). Persisted and restored all range settings and reset the file input.
  - 🔧 FIXED: In `updatePolarRange()` and `updateParamRange()`, `parseFloat(...) || default` treated `0` as falsy, resetting custom min/max ranges to default whenever `0` was entered. Replaced falsy fallback with `!isNaN()` validation; isolated preset loading so previously active curves do not clutter newly loaded presets; and added touch event handlers (`touchstart`, `touchmove`, `touchend`) to canvas for mobile and touch display panning.

- **KGo**: ISSUES FOUND ⚠️ (8 issues, 8 fixed inline)
  - ✅ Core gameplay works (9x9, 13x13, 19x19 Go/Baduk goban with hoshi star points and Tatami lantern aesthetic, Superko and suicide validation, territory and liberties flood fill, 4 AI personalities Territorial/Influence/Balanced/Grandmaster, AI Hint move recommendation, live Territory Estimator and Group Liberty Analyzer overlays, 20-stage Campaign and Tsumego life-and-death puzzles, Web Audio placement/capture synth sounds, multi-tier particle spark and canvas smoke physics, cherry blossom atmospheric effects).
  - 🔧 FIXED: Global `keydown` listener checked bare `e.key` without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking browser accelerators (`Ctrl+S` triggering Group Analyzer, `Ctrl+H` calculating hint, `Ctrl+T` toggling territory estimator, `Ctrl+U` triggering undo). Added modifier key guards and modal background suppression.
  - 🔧 FIXED: Help modal could not be dismissed via `Escape` or by clicking the modal backdrop overlay. Added backdrop click dismissal, `Escape` key close, and wired `P` (Pass) and `?`/`F1` (Help) keyboard shortcuts to match the in-game documentation.
  - 🔧 FIXED: Tsumego life-and-death puzzles (Stages 3, 7, 11, 14, 16) lacked victory detection when the objective was met (e.g. capturing White's corner group at (0,0) or playing the vital eye/crane/belly tesuji), causing the AI to continue making moves and forcing an unwanted full 19x19 game. Added goal completion verification in `placeStone()` to celebrate, record win, and smoothly advance to the next stage.
  - 🔧 FIXED: In `#btn-tsume`, `currentTsumegoIdx` was initialized to 0 and immediately incremented on the first click, skipping Stage 3 ("Corner Capture") and launching Stage 7 instead. Re-indexed to start cleanly on the first puzzle.
  - 🔧 FIXED: `#btn-save` and `#btn-load` omitted `currentCampaignStage`, `aiToggle` state, and `aiDifficulty` from `localStorage`, causing saved Campaign or Tsumego sessions to restore as unlinked free-play matches with reset AI settings. Persisted and restored all campaign and AI configuration fields.
  - 🔧 FIXED: `#ai-toggle` lacked a `change` event listener, preventing White AI from reactively taking its turn if the checkbox was toggled to active while White was to move. Added change listener to trigger `makeAIMove()`.
  - 🔧 FIXED: Changing `#board-size` via dropdown did not reset `currentCampaignStage = -1`, desynchronizing board dimensions with campaign progression. Reset campaign state upon manual board dimension changes.
  - 🔧 FIXED: In `drawBoard()`, `mouseleave` on board cells updated `hoverPos` without calling `drawBoard()`, freezing the group highlight and analyzer banner on the last inspected stone group when the cursor moved off stones or left the board. Refreshed analyzer rendering on both `mouseenter` and `mouseleave`, and added a confirmation prompt to `#btn-score` to prevent accidental in-progress game resets.

- **KFreecell**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core gameplay works (Classic FreeCell, Numbered Deal 1-1M, 20-stage Campaign with King-only slots/Suit rules/Frozen cards, 180s Time Attack mode, 4 active skills Auto-Solve/Magic Wand/+1 Temporary Free Cell/Shuffle, interactive SVG suits and face cards, multi-tier particle canvas engine, and audio synthesizers).
  - 🔧 FIXED: Statistics (`stats`), win streaks, best times, and Campaign progression (`campaignStage`, `maxCampaignStage`) were never persisted to `localStorage`, resetting all game records, player statistics, and campaign stage unlocks to 0 on every browser refresh. Implemented `saveStats()`, `loadStats()`, and added a "Reset Stats" confirmation button in the Stats modal.
  - 🔧 FIXED: Game saving (`saveGame()`) and loading (`loadGame()`) omitted active game mode (`#game-mode`), custom seed input, `buildRule`, and `emptyKingOnly` constraints, causing saved Campaign or Time Attack sessions to restore as Random Deal with standard color rules upon reload. Persisted and restored all rule variations and state.
  - 🔧 FIXED: Extra Cell powerup (`+1 Cell [E]`) failed to call `render()` when its 30-second duration expired, leaving the visual 5th slot stuck on board. If the slot was occupied when the timer expired, the slot became permanent and was never reclaimed upon subsequent moves. Reclaimed expired cells reactively once emptied and triggered DOM re-render.
  - 🔧 FIXED: Victory card cascade ran a concurrent `requestAnimationFrame` loop that directly conflicted with the continuous 60 FPS `runFxLoop` on `#fx-canvas`, with `runFxLoop`'s frame clear erasing cascade cards and inducing visual screen flickering. Integrated cascade rendering directly into `runFxLoop`.
  - 🔧 FIXED: `#decorative-deck` card element displayed `cursor: pointer` without a click handler. Connected click handler to dynamically cycle through the 4 luxury card back designs with audio feedback, matching the Settings modal.
  - 🔧 FIXED: Double-clicking cards had no effect, forcing players to manually click source and target destinations for every single play. Implemented standard `ondblclick` handlers allowing cards to auto-play to Foundations, or top tableau cards to open Free Cells.
  - 🔧 FIXED: Global `keydown` shortcuts lacked modifier guards (`!e.altKey`), input focus guards (allowing keys to trigger powerups while typing in `#seed-input`), and modal state suppression (firing background game actions while Help, Stats, or Settings modals were open). Added modifier guards, modal background suppression, `Escape` to dismiss modals/selection, and hotkeys `N` (New Game), `H`/`?` (Help), `S` (Settings), and backdrop click dismissal on all modals.

- **KFortress**: ISSUES FOUND ⚠️ (8 issues, 8 fixed inline)
  - ✅ Core gameplay works (12 diverse campaign maps with biome environmental art and ambient weather effects, 7 defense towers with 3-tier upgrade trees and 4 mythic elemental fusions Inferno/Superconductor/Venomspite/SolarBeam, 4 tactical traps Spike Pit/Oil Slick/Barricade/Dynamite, interactive controllable Commander hero with 4 active skills and auto-attack, 5 challenge mutators Bloodlust/Titan/Eclipse/Meteor/PhaseShift, Research Academy with 8 persistent upgrades, multi-mode Campaign/Endless/BossBlitz, and Web Audio dynamic synthesizers).
  - 🔧 FIXED: In `castTrebuchet()` and base damage handling, screen shake was assigned to an undeclared global variable (`screenShake = 22;` and `screenShake = (dmgToBase >= 3) ? 15 : 5;`), bypassing the kinematic screen shake physics engine (`triggerScreenShake()`). Switched both to `triggerScreenShake()`.
  - 🔧 FIXED: Global `keydown` listener checked bare `e.key` without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, intercepting browser accelerators (`Ctrl+R` which wiped player defenses mid-session, `Ctrl+F` which cast Firestorm spending 100g, `Ctrl+A` opening Academy, `Ctrl+H` opening Guide, `Ctrl+1..5` casting skills). Added modifier guards.
  - 🔧 FIXED: Gameplay hotkeys (`Space`, `1-5`, `F`, `B`, `R`) continued to fire in the background when modal dialogs (Research Academy, Mutators, Commander's Field Guide) were open. Suppressed background hotkeys while any modal is visible.
  - 🔧 FIXED: Commander skills (`castHeal`, `castShield`, `castMeteor`, `castReinforce`, `castTrebuchet`) and battle spells (`btnFirestorm`, `btnBlizzard`) lacked `gameOver` guards, allowing actions and gold expenditure after citadel walls were breached. Added `if (gameOver) return;` to all skills and spells.
  - 🔧 FIXED: Selecting a trap card (Spike, Oil, Barricade, Dynamite) and clicking an empty tower slot placed a corrupted trap-tower with `NaN` damage and undefined stats into the stone pedestal. Added guard to disallow trap placement on tower slots with toast feedback.
  - 🔧 FIXED: Purchasing "Wall Durability (+10 HP)" or "Hero Cooldowns (-10%)" in the Research Academy did not update `maxBaseHp` or hero skill cooldown ceilings until a full game reset. Updated `buyTech()` to dynamically apply base HP and hero cooldown changes immediately.
  - 🔧 FIXED: Tower selection circle hardcoded range to `130px`, misleading players on high-range towers (Siege Ballista 190, Venomspite 210) or upgraded towers. Dynamically set range circle to match the selected tower's actual attack radius.
  - 🔧 FIXED: Citadel breach / defeat state only disabled `#waveBtn` without visual canvas feedback or defeat notice. Added red canvas defeat banner ("CITADEL HAS FALLEN") and toast notification prompting player to reset.

- **KFont**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (font typography inspector suite with 5 tab views: Canvas/OS2 Font Metrics & BBox table, Unicode Range Explorer across 12 blocks with character inspection, OpenType Kerning comparison pairs with font-feature-settings and rasterization hinting canvas across 6 scale sizes, Glyph Anatomy & Vector Metrics dissector with baseline/cap/x-height/ascent/descent guides and ABC side bearings, Live Sample tester with 5 quick pangram presets, JSON anatomy report export, and localStorage persistence).
  - 🔧 FIXED: Global `keydown` listener checked bare `e.key` without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, which hijacked essential browser shortcuts (`Ctrl+1` through `Ctrl+5` for browser tab switching, `Ctrl+C` for clipboard copy, `Ctrl+H` for history, `Ctrl+B` for bookmarks, `Ctrl+I` for page info). Added modifier key guards to protect native browser accelerators.
  - 🔧 FIXED: Single-key shortcuts (1–5, B, I, C) continued to fire in the background when the Help & Keyboard Shortcuts overlay was open, unexpectedly switching tabs or toggling font bold/italic while reading documentation. Suppressed background hotkeys while `#helpOverlay` is visible.
  - 🔧 FIXED: In `updateMetrics()`, `metrics.fontBoundingBoxAscent` and `descent` lacked fallbacks for browser engines lacking the `FontBoundingBox` metrics API (e.g. Safari / older engines), causing all Em Height, Line Gap, and extrapolated OS/2 metrics to evaluate to `0.00 px`. Added fallback to `actualBoundingBox` and em-fraction approximations.
  - 🔧 FIXED: Switching tabs via buttons or keyboard shortcuts did not refresh dynamic canvas renders for the target tab (e.g. switching to Diagnostics did not re-render hinting, switching to Anatomy did not recalculate responsive canvas dimensions). Centralized panel refresh inside `switchToTab()`.
  - 🔧 FIXED: In `updateSample()`, setting `sampleOutput.style.font = getFontString()` used shorthand CSS font syntax that reset `line-height` from CSS `1.5` to default `normal`, compressing multiline sample text. Switched to individual property assignments (`fontFamily`, `fontSize`, `fontWeight`, `fontStyle`) to preserve container line-height.
  - 🔧 FIXED: `#anatomyInput` lacked an Enter key handler to commit text and blur focus, clicking glyph cells switched tabs without user feedback, active inputs were not blurred on Escape, and tab buttons lacked Arrow key navigation. Added Enter key submission and blur, toast feedback when clicking glyphs, Escape blur on active inputs, and WAI-ARIA Arrow key navigation across tablist buttons.

- **KFlash**: ISSUES FOUND ⚠️ (8 issues, 8 fixed inline)
  - ✅ Core functionality works (interactive 3D flipping flashcard interface, Spaced Repetition study controls with Got It / Needs Review status tags, 8 built-in sample packs across Math, Science, Languages, and History with 300+ flashcards, search query filtering and review-only mode, deck statistics with mastery progress meter, card CRUD with in-place modal editing, shuffle, and print view).
  - 🔧 FIXED: In Review-Only mode, marking a card as "Got It ✓" executed `renderCard()` which removed the card from `filteredIndices`, shifting all subsequent cards down by one index, and then immediately invoked `btnNext.click()` (`currentIndex++`), permanently skipping the very next card without ever displaying it to the user. Streamlined rating into `handleStudyRating()` to dynamically update filtered indices and advance smoothly without skipping cards.
  - 🔧 FIXED: Deleting a card, shuffling the deck, filtering via search, loading sample packs, or importing files failed to hide `#studyControls` when cards were flipped, leaving orphan "Got It ✓" and "Needs Review ↻" study buttons visible on the front face of new cards. Added centralized `resetCardFlip()` to reset flip state and hide study controls across all deck transitions.
  - 🔧 FIXED: Global keyboard listener checked bare `e.code` (`Space`, `Enter`, `ArrowLeft`, `ArrowRight`) without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking browser shortcuts (e.g. `Alt+Left` back navigation). Furthermore, keystrokes continued to manipulate background flashcards while modal dialogs (Stats, Help, Add/Edit, Sample Packs) were active. Added modifier guards and suppressed background card controls while modals are active.
  - 🔧 FIXED: Flipping logic checked `deck.length > 0` rather than `filteredIndices.length > 0`, causing empty search results ("No matches found" / "Try a different search") to flip and display active study controls. Restricted flipping to matching filtered cards and shook container when empty.
  - 🔧 FIXED: None of the 4 modal dialogs closed when clicking the background backdrop or pressing `Escape`. Added backdrop dismissal, Esc key dismissal, and wired `Ctrl+Enter` to quickly save cards inside `#addCardModal`.
  - 🔧 FIXED: `btnPrint` rendered card questions and answers via unescaped string interpolation into `div.innerHTML`, causing mathematical inequalities and HTML characters (such as `<, >, <=, or >=` in Algebra 101) to break DOM parsing. Added `escapeHtml()` sanitization.
  - 🔧 FIXED: Loading sample packs copied shallow object references (`deck = [...importedDeck]`), mutating the global `SAMPLE_PACKS` constant in memory when cards were edited or marked as known, and users had no way to reset study progress on a deck. Deep-cloned sample cards on load and added a "Reset Progress" button in the Stats modal.
  - 🔧 FIXED: Help documentation promised CSV export, but `btnExport` only generated JSON, while CSV import parsed rows using naive `line.split(',')`, corrupting cards containing commas within quotes. Added dual JSON/CSV export prompt, implemented RFC-compliant quoted CSV parser with header detection, and wired up `1`/`G` (Got It), `2`/`R` (Needs Review), `A` (Add), `S` (Shuffle), and `?`/`H` (Help) keyboard shortcuts.

- **KFarm**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core gameplay works (10x10 farm grid simulation with procedural dirt furrows and day/night lighting, multi-seasonal crop cycle Spring/Summer/Fall/Winter with Wheat/Corn/Tomato/Pumpkin, weather system Clear/Rain/Drought/Crows, livestock Chickens and Cows with wandering pasture sprites, production upgrades Mill/Mayo Maker/Cheese Press/Scarecrow/Fertilizer/3x3 Upgraded Tools, Web Audio synthesizer effects and 4-tier kinematic particle explosion engine).
  - 🔧 FIXED: Keyboard listener checked bare `e.key` without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking essential browser accelerators (`Ctrl+H` for history, `Ctrl+1` through `Ctrl+4` for tab navigation, `Ctrl+S`, `Ctrl+R`). Added modifier guards and added dedicated `S` (Save) and `R` (Reset) shortcuts to the Almanac reference guide.
  - 🔧 FIXED: Game lacked bankruptcy handling and farm restart options. If a player spent starting funds on seeds that died or were eaten without owning livestock, money reached $0 with no recovery mechanism, permanently softlocking the game. Implemented an automatic Town Relief Subsidy grant (+ $25) when bankrupt and unable to plant, and added a safe "Reset Farm" button with confirmation.
  - 🔧 FIXED: High-value progression purchases (Fertilizer, 3x3 Upgraded Tools, Scarecrow, Mill, Mayo Maker, Cheese Press) had no audio feedback, rendering major milestones silent. Added a 4-tone rising chime synthesizer sound `upgrade` to `playSound()` and wired it across all upgrade purchases.
  - 🔧 FIXED: Harvesting or tilling with 3x3 Upgraded Tools executed `shakeAmount += 15` and `showToast()` on every single cell in the AoE loop, creating up to 135 screen shake (causing disorienting off-screen viewport oscillations) and spamming multiple redundant toast alerts. Capped screen shake to safe bounds and aggregated AoE harvests into a single summary toast (e.g. `Harvested N crops (+$X)!`).
  - 🔧 FIXED: Canvas click coordinates `(e.clientX - rect.left) / CELL_SIZE` assumed a static 400px CSS dimension without accounting for display scaling, high-DPI zoom, or mobile viewports, offsetting clicked tiles; and mobile touch events were unhandled. Added dynamic `scaleX`/`scaleY` coordinate normalization and touchstart event forwarding.
  - 🔧 FIXED: Clicking already-watered crops provided no feedback or guidance, leaving players unsure if the action registered. Added informative toast showing remaining crop maturity progress (e.g. `Already watered today! Growth: 1/2d`).
  - 🔧 FIXED: Roaming chickens and cows were hardcoded to 3 chickens and 2 cows in the draw loop, meaning newly purchased livestock provided daily income but never appeared in the pasture; and all farm state was lost on page refresh or KiloOS app switching. Dynamically expanded roaming animal lists up to 8 chickens and 6 cows, and implemented full `localStorage` auto-save and load persistence (`kfarm_save_v1`).

- **KDragon**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core gameplay works (dragon pet raising simulation with egg incubation, needs management Hunger/Happiness/Energy/Age, dynamic feeding, playing, sleeping, wilderness gold expeditions, 3 training minigames Strength Power Meter/Speed Reflex/Loyalty Cup Guess, 4-item Bazaar Shop, elemental adult evolution Fire/Water/Earth at Age 10 with specialized combat skills Fireball/Healing Stream/Earthquake, turn-based dragon battle arena with Web Audio FX and 60 FPS canvas particle engine).
  - 🔧 FIXED: Critical syntax error in `tick()` at line 1770 where `if (this.state === 'dragon' && this.stats.age >= 10)` was never closed with a curly brace `}`, causing `SyntaxError: Unexpected token ';'` that broke script execution entirely on load and prevented egg incubation, button clicks, and all game actions. Added missing closing brace.
  - 🔧 FIXED: In `battleTurn()`, lack of an active `if (!this.battleState) return;` guard caused `TypeError: Cannot read properties of null (reading 'enemySpd')` when hotkeys or attack buttons were rapidly pressed upon battle victory or defeat. Guarded `battleTurn()`, prevented execution of `special` prior to elemental evolution with warning feedback, and logged enemy remaining HP on attack and elemental strikes.
  - 🔧 FIXED: Battle view lacked visual HP indicators for both player and enemy dragon, leaving players with no on-screen feedback on combat damage or remaining health. Enhanced `entity-art` during `battleState` to display real-time dual HP gauges (`Player HP: X/Y` and `Enemy HP: A/B`) and called `this.updateUI()` on combat resolution turns.
  - 🔧 FIXED: Global keyboard listener lacked modifier key guards (`e.ctrlKey`, `e.altKey`, `e.metaKey`), causing single-letter and numeric hotkeys (`F`, `P`, `S`, `T`, `O`, `B`, `H`, `1`-`7`) to intercept essential browser shortcuts (`Ctrl+F`, `Ctrl+P`, `Ctrl+S`, `Ctrl+T`, `Ctrl+B`, `Ctrl+H`, `Ctrl+1`-`Ctrl+7`). Added modifier key guards.
  - 🔧 FIXED: Keyboard hotkeys remained active while the Dragon Master's Guide modal was open, executing background game actions (feeding, sleeping, training, battles) while reading instructions. Suppressed non-modal keystrokes while `#help-modal` is active.
  - 🔧 FIXED: Starting screen had no clickable Help button despite displaying "Press H or F1 for Help", hindering mouse and touch-only users before hatching. Added `#btn-start-help` button beside "Incubate Egg" and wired it in `init()`.
  - 🔧 FIXED: Minigame hit, react, and treat box buttons lacked debounce/single-fire guards, allowing double-clicking to award duplicate stat points; and `hoard` expeditions and `startMinigame()` omitted `this.updateUI()`, leaving stats header desynchronized upon deducting energy and hunger. Added click guards and synchronized UI stats.

- **KDarts**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core gameplay works (3D Sisal Dartboard simulation with metallic wire spider and number ring, mahogany pub cabinet frame, dynamic mouse aiming with atmospheric wobble and wind gusts, 6 game modes 501 Double Out/301/Cricket/Around the Clock/Bullseye Blitz/Killer Darts, 20-stage Campaign mode leading to World Championship Finals, 4 AI difficulty levels Easy/Medium/Hard/Legend and local Vs Human 2-player mode, 4 power-up skills Focus/Magnet/Laser Sight/Undo Dart, Web Audio sound synthesizers, and multi-layer particle explosion engine).
  - 🔧 FIXED: Redundant duplicate `canvas.addEventListener('mousedown')` and `window.addEventListener('keydown')` event listeners in the middle of the script fired concurrently with bottom listeners, causing every single mouse click to throw two darts in rapid succession and triggering duplicate key handlers. Removed duplicate listeners.
  - 🔧 FIXED: On initial page load, `helpPanel.style.display` was empty string, causing the first click on the Help button to evaluate `p.style.display === 'none'` as false and re-set display to `'none'`, failing to open the modal until clicked a second time. Switched to computed style check `getComputedStyle(p).display !== 'none'`.
  - 🔧 FIXED: `toggleSound()` referenced `event.target` without passing event or providing a button element ID, risking `ReferenceError: event is not defined` in strict environments/Firefox. Assigned `id="btnSound"` to the sound button and queried it directly.
  - 🔧 FIXED: In `update()`, the crosshair vertical reticle was drawn with `ctx.moveTo(tx, ty - 18); ctx.lineTo(tx + 18, ty);`, producing an asymmetric diagonal slant rather than a true vertical axis. Corrected endpoint to `(tx, ty + 18)`.
  - 🔧 FIXED: Active skill handlers (`activateFocus`, `activateMagnet`, `activateLaser`, `activateUndoDart`) lacked human player and active game state guards, allowing skills to be expended during AI turns or after round conclusion. Furthermore, `activateUndoDart` manual math failed to restore original score after a bust. Guarded skills with `isP1OrHuman` state checks and rewound the last `historyStack` snapshot to restore score, multiplier, and dart counts without math corruption.
  - 🔧 FIXED: Selecting a new difficulty in `#diff` changed `aiDifficulty` without calling `updateWind()` or `updateScoreUI()`, leaving the UI title saying "vs AI" even when "Vs Human" was selected and not updating wobble/wind physics. Also `saveState`/`loadState` did not preserve active power-up flags (`focusActive`, `magnetActive`, `laserActive`) and `loadState` failed silently when no save existed. Synchronized UI/physics on difficulty change, added missing save feedback, and persisted full skill state.
  - 🔧 FIXED: Global keydown listener lacked modifier guards (`ctrlKey`, `altKey`, `metaKey`), hijacking browser shortcuts (e.g. `Ctrl+1`, `Ctrl+F`, `Ctrl+C`). Canvas clicks and hotkeys also fired through an open Help modal. Added modifier guards, modal background suppression, `Escape`/`H` toggling, `Space`/`Enter` turn advancement, and turn progression prompts (`Click or Space`).

- **KDB**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (employee database table management with 500-record capacity, multi-criteria substring and relational query engine, 4-column bidirectional sorting, CRUD operations with edit and delete confirmation modals, XOR cipher database encryption with crypto key protection, CSV/JSON import and export with duplicate detection, executive Markdown directory report generation, department distribution analytics charts, and toast notification subsystem).
  - 🔧 FIXED: In the global keyboard event listener, background shortcuts (`a`, `e`, `i`, `j`, `o`, `m`, `r`, `h`, `/`, `Ctrl+F`, `Ctrl+N`) fired underneath active dialogs, allowing file import prompts, background search focus, or markdown/CSV/JSON file downloads to trigger while modal dialogs (Stats, Help, Delete, Reset) were open. Added `isAnyModalActive` modal presence guard and wired up intuitive `Enter` key handling to confirm Delete, confirm Reset, and dismiss Help/Stats modals.
  - 🔧 FIXED: Column header elements (`<th>`) had click-to-sort listeners but lacked `tabindex="0"`, `role="button"`, and keyboard `Enter`/`Space` key handlers, rendering table sorting inaccessible via keyboard navigation. Added tabindex, role, key handlers, and `:focus-visible` styling.
  - 🔧 FIXED: Entering or clearing an encryption key in `#dbPassword` called `loadRecords()`, which did not encrypt or unencrypt active in-memory records in `localStorage` until subsequent record edits, leaving data saved in plaintext. Created `handlePasswordChange()` with Enter key submission on `#dbPassword` to immediately persist active database records encrypted or decrypted in `localStorage`. Also UTF-8 encoded encryption keys in `cryptData()` and `decryptData()` to prevent `btoa()` `InvalidCharacterError` crashes on non-ASCII passwords.
  - 🔧 FIXED: `#storageQuotaText` was a static placeholder string (`Limit 500 records`) that never updated to reflect storage capacity or active encryption status. Wired live record count `${records.length} / ${MAX_RECORDS}` and encryption indicator into `renderTable()`. Updated `#addId` placeholder to dynamically reflect `getNextSuggestedId()` and re-focused `#addId` upon adding an employee.
  - 🔧 FIXED: Search query engine in `filterTable()` lacked relational operators `id>=`, `id<=`, and exact match `id=`, and `exportCSV()` coerced numeric ID `0` to empty string. Supported extended ID query comparisons, fixed CSV field zero coercion, and escaped markdown pipe characters `\|` in `exportMarkdown()` roster and department summary tables.
  - 🔧 FIXED: Database reset (`confirmReset()`) copied `DEFAULT_RECORDS` shallowly, mutating default records across subsequent in-place edits. Deep-copied default records on reset and initial load, and properly cleared table state when decryption fails.

- **KCyber**: ISSUES FOUND ⚠️ (8 issues, 8 fixed inline)
  - ✅ Core functionality works (cyberdeck terminal command-line hacking simulation, canvas CRT phosphor HUD and particle engine with animated ICE daemons and data cartridges, Mastermind-style 4-digit PIN brute-force module, Web Audio sound synthesizers, black-market upgrade shop, contracts mission board with dynamic rewards, network topology map with multi-tiered ICE nodes, trace dampener slow tool, and sensor blind cloak subsystem).
  - 🔧 FIXED: In `printLine()`, `terminalDiv.scrollTop = terminalDiv.scrollHeight` was updated while `outputDiv.scrollTop` was never touched. Because `#output` is styled with `flex: 1; overflow-y: auto`, incoming output lines and combat logs failed to auto-scroll into view once output exceeded the viewport fold. Added `outputDiv.scrollTop = outputDiv.scrollHeight`.
  - 🔧 FIXED: When cracking a node PIN (`exact === 4`), `ice_interval` was never cleared and `iceAttack()` continued executing while connected to root, subjecting the player to continuous MEM damage, alarms, and screen shake with no mechanism to defend (as `cloak` and `slow` commands were disabled in connected mode). Cleared `ice_interval` upon successful node breach and guarded `iceAttack()` to only fire during active intrusion attempts (`hacking_node`).
  - 🔧 FIXED: In `drawVisuals()`, connected HUD title rendered double zeros (`[DATA://NODE_002]`) because `connected_node` was already formatted with a leading zero. Updated string interpolation to `[DATA://NODE_${connected_node}]`.
  - 🔧 FIXED: Terminal CLI lacked command history navigation. Added command history buffer (`cmdHistory`) with ArrowUp and ArrowDown key navigation for easy recall of previous commands and PIN guesses.
  - 🔧 FIXED: `connect <node>` failed to verify if player MEM was depleted (`<= 0`), allowing players with 0% memory to initiate hacks only to instantly terminate on the first ICE cycle. Also normalized single-digit node arguments (`connect 2` -> `02`) so users can connect without mandatory zero padding, and added helpful rejection message for local gateway `01`.
  - 🔧 FIXED: Sub-states (shop, hacking, connected node) lacked universal utility commands. Added `help`, `status`, and `clear` to all modes without consuming hacking attempts, and added `exit` as an intuitive alias for `disconnect` in connected mode.
  - 🔧 FIXED: System boot failed to initialize `generateMissions()`, causing `accept <id>` on a clean start to fail with "Invalid contract ID", and active contracts had no visibility in `status` or `contracts`. Called `generateMissions()` on startup, displayed active contract status across both commands, and updated `status` to reflect real-time network connection state.
  - 🔧 FIXED: In `download <filename>`, file matching was strictly case-sensitive, failing valid downloads like `download SYS_LOGS.DAT`, and root cyberdeck terminal threw "Command not found" on standard `ls`/`dir`. Made file downloads and contract completion case-insensitive, added helpful usage hints, and added local `/bin` module listings for `ls`/`dir` at the root prompt.

- **KConverter**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core functionality works (multi-category universal unit converter supporting 9 physical dimensions Length/Weight/Temperature/Data/Speed/Area/Volume/Time/Pressure across 64 built-in units, precision decimal and scientific notation formatting, Single Convert mode with live bi-directional evaluation and equation breakdown, Batch Mode converting across all units in active category simultaneously with live search filter, Smart Parser evaluating natural language and engineering expressions with dimensions check, pinned Favorites system with 1-click loading, Custom Units engine with multiplier factors, audit trail History Log with CSV and JSON export, full keyboard shortcuts suite and reference modal).
  - 🔧 FIXED: In `exportHistoryCSV()` and `exportHistoryJSON()`, the download anchor `a` was clicked via `a.click()` without being attached to `document.body`, failing silently in Firefox and sandboxed iframe environments, and `URL.revokeObjectURL(url)` was invoked synchronously on the next line before the browser could process the download. Attached anchor to DOM, added deferred URL revocation, and escaped double quotes `"` in CSV records as `""`.
  - 🔧 FIXED: In `populateDropdowns()`, existing dropdown selections (`unitFrom`, `unitTo`, `batchUnitFrom`) were unconditionally reset to indices 0 and 1 every time custom units were added or deleted, wiping out user selections. Cached previous selections and restored them when still valid.
  - 🔧 FIXED: `convert()` and `parseExpressInput()` invoked `logHistory()` unconditionally during `window.onload`, polluting the history log with phantom conversion entries every time the app was opened or refreshed. Added `isInitialLoad` guard to prevent startup history spam and added check for empty history before clearing in `clearHistory()`.
  - 🔧 FIXED: Global `keydown` handler checked `e.key.toLowerCase() === 'h'` without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking the browser's native History shortcut (`Ctrl+H`), and keys 1–6 switched app views in the background while the Help Modal was open. Added modifier guards and suppressed 1–6 when `helpModal` is active.
  - 🔧 FIXED: Navigation tabs (`.nav-tab`) lacked `tabindex="0"` and keyboard Enter/Space activation, the Smart Parser (`#expressInput`) and batch input (`#batchInputVal`) lacked Enter key handlers despite the Help guide advertising `Enter Evaluate / Calculate`, and pressing Enter while focused on a favorite card's Delete button (`✕`) triggered `card.onkeydown` and unexpectedly loaded the favorite instead of deleting it. Added tabindex, Enter/Space activation, and guarded card keydown.
  - 🔧 FIXED: `switchView('custom')` failed to invoke `renderCustomUnitsTable()`, `addCustomUnit` omitted `convert()` and `renderBatch()`, and `deleteCustomUnit` omitted `renderBatch()`. Added custom view refresh and synchronized calculation updates on custom unit changes.
  - 🔧 FIXED: Smart Parser failed on standard English unit names (e.g. `yards`, `centimeters`, `millimeters`, `foot`, `ton`, `milligram`, `gigabyte`, `quarts`, `milliliters`, `hectares`, `pascals`) and leading verbs (`convert`, `calculate`). Expanded `expressAliasTable` with standard English names, supported command prefixes, added unresolved unit feedback, fixed negative zero formatting in `formatNumber`, and updated `swapUnits()` to swap the output value into the input value for reversible unit conversions.

- **KContacts**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core functionality works (address book and contact manager with dynamic category pills All/Favs/Work/Personal/Family/Friends/Other, dynamic interactive hashtag filtering chips `#all-tags` and per-tag filters, live full-text search with clear button, avatar initials generator with deterministic hue gradients, full contact profile editor with name, company, title, phone, email, tags, notes, and favorite toggle, automated duplicate contact merger with tag & note consolidation, multi-format export Markdown/JSON/vCard 3.0/CSV, multi-format import parser for JSON arrays, RFC 2426 vCard address books, and CSV spreadsheets with quoted fields, localStorage persistence `kcontacts_data_v3`).
  - 🔧 FIXED: Numeric keyboard shortcuts (keys 1–7) for switching categories checked bare `e.key` without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking browser tab switching accelerators (`Ctrl+1` through `Ctrl+7`). Added modifier key guards.
  - 🔧 FIXED: In ArrowDown/ArrowUp keyboard navigation, calling `items[nextIdx].click()` triggered `selectContact()` which re-rendered the contact list and completely rebuilt the DOM, causing the subsequent `items[nextIdx].scrollIntoView()` call to execute on a detached DOM element. Updated navigation to scroll the newly rendered active contact element into view.
  - 🔧 FIXED: Export modal format cards (`.export-card`) and contact list tag chips lacked `tabindex="0"`, `role="button"`, and keyboard event handlers, preventing keyboard navigation and screen reader users from selecting export formats or activating tag filters. Added accessibility attributes and Enter/Space handlers.
  - 🔧 FIXED: In `doExport()`, the download anchor was clicked via `a.click()` without being attached to `document.body`, failing silently in Firefox and sandboxed iframe environments. Attached anchor to DOM before triggering click and cleanly removed it afterwards.
  - 🔧 FIXED: CSV export did not strip or escape newlines in contact `notes`, causing multiline notes to break single CSV records across multiple lines and corrupting subsequent imports into line-by-line CSV parsers. Replaced newlines in notes with spaces during CSV generation.
  - 🔧 FIXED: Quick Action phone and email buttons called `window.open('tel:...', '_self')` and `window.open('mailto:...', '_self')`, which navigated the hosting iframe window and could result in browser security errors or blank error pages (`ERR_UNKNOWN_URL_SCHEME`). Routed actions through standard temporary anchor dispatching.
  - 🔧 FIXED: Creating a new contact while filtering by a specific category (e.g. "Work") or tag caused the new draft (defaulting to "Personal") to immediately vanish from the contact list. Now inherits the active category/tag filter and clears active search text so the draft remains visible and selected. Also added Enter key submission on the Delete confirmation modal and case-insensitive file extension checks on import.

- **KConnect4**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core gameplay works (Classic 7x6 Connect-4, 2-Player, vs AI with 4 personalities Rookie/Aggressive/Trapper/Grandmaster Minimax, 20-Stage Campaign with obstacle hazards and dynamic 7x6 to 10x8 grids, 7-second Speed mode, special discs Bomb/Drill/Magnet and Freeze skill, live positional evaluation bar and threat radar, C4N and FEN notation viewer and custom position loader, match replay engine with step/jump/speed controls, localStorage save/load state, sound FX synthesizers and cybernetic canvas particle engine).
  - 🔧 FIXED: Duplicate `window.addEventListener('keydown')` listener intercepted shortcuts (F1, H, N, T, U, F, B, D, M, F5, F9, Home, End, Arrows) without checking for active input focus, causing Help and Notation modals to instantly flicker open and closed upon pressing 'H', 'F1', or 'N', and hijacking keystrokes while typing custom positions in `#customPositionInput`. Removed duplicate listener so the primary guarded listener manages all keybinds cleanly.
  - 🔧 FIXED: Notation modal (`toggleNotationModal`) opened without displaying `modalBackdrop` and failed to call `closeAllModals()`, leaving the Help modal visible underneath and preventing clicking outside the modal from closing it. Added backdrop toggling and modal coordination.
  - 🔧 FIXED: Freeze skill blocked the column for both players indiscriminately instead of only the targeted opponent (`frozenPlayer`), locking the caster out of their own column. Furthermore, `frozenTurns` was decremented on every move rather than on the frozen player's turn, cutting the freeze duration in half. Fixed to restrict only the targeted opponent and decrement only on the frozen player's turn.
  - 🔧 FIXED: Clicking column number badges or pressing keys 1–7 while Freeze skill was armed bypassed freeze activation and dropped regular discs instead. Extracted `applyFreezeCol(c)` and routed column badge clicks and numeric keypresses to freeze the selected column when Freeze Mode is active. Also synchronized `.hover` state on column number badges when hovering over the board.
  - 🔧 FIXED: In `dropPiece`, the win detection branch `w1 || w2` handled game termination but bypassed `checkWin(r, col)` in an unreachable `else if`. Because `checkWinBoard` only returned a boolean without populating `winningCells`, `winningCells` remained permanently empty on victory, suppressing winning disc highlight animations, golden sparkle emission, and the canvas neon laser win beam. Implemented `findWinningCells(b, p)` to populate `winningCells` on game conclusion.
  - 🔧 FIXED: Stepping backwards in match replay retained final `winningCells`, leaving glowing win highlights on empty board cells; C4N and JSON file downloads lacked DOM anchor attachment; FEN board import hardcoded `COLS = 7`, truncating Campaign boards with 8–10 columns; and `#customPositionInput` lacked Enter key submission. Added replay winningCells clearing, dynamic FEN column calculation, cross-browser download cleanup, and Enter key submission.

- **KColor**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (interactive color picker suite with precision RGB/HSL/HSV/CMYK range sliders, Universal CSS/HEX/RGB/HSL/CMYK parser with live input, 7-format color conversions table with clipboard copy HEX/RGB/HSL/HSV/CMYK/CSS-VAR/Win32-C, 9-step dynamic Tints & Shades scale, 8 color harmonies generator Complementary/Analogous/Triadic/Tetradic, WCAG 2.1 AA/AAA contrast ratios against white & dark text, palette library with localStorage persistence).
  - 🔧 FIXED: Universal color parser canvas fallback set `fillStyle = '#000000'` before attempting to parse input, causing any invalid color string or mid-typing incomplete hex code (e.g. `#12345` or non-color text) to silently evaluate to `#000000` and turn the active color pitch black. Implemented dual-sentinel validation (`#010203` and `#040506`) to accurately reject invalid strings without corrupting color state.
  - 🔧 FIXED: Hex regex `^#?([0-9a-fA-F]{3,8})$` matched 5-digit and 7-digit hex codes which are invalid CSS syntax, falling through to canvas fallback and resetting color to black. Fixed regex to strictly validate 3, 4, 6, and 8 hex digits.
  - 🔧 FIXED: Universal color input lacked an HSV format parser (`hsv(...)`) despite HSV being a core app mode and exported in the formats table, and `hslMatch` rejected degree symbols `°`, causing the app's own preview HSL string (`hsl(210°, 50%, 59%)`) to fail parsing. Added HSV regex parser and degree symbol / CSS unit tolerance.
  - 🔧 FIXED: Global keyboard shortcuts checked bare `e.key` without verifying `!e.ctrlKey && !e.metaKey && !e.altKey`, which hijacked essential browser shortcuts: `Ctrl+C` (copy selection blocked), `Ctrl+R` (page reload blocked, unexpectedly randomizing color), and `Ctrl+1`–`Ctrl+9` (browser tab switching blocked, picking palette swatches). Added modifier key checks to protect browser accelerators.
  - 🔧 FIXED: Eyedropper button showed a dead-end toast "Eyedropper not supported in this browser" on Firefox, Safari, and sandboxed iframes. Added fallback triggering a native `<input type="color">` picker so screen/system color sampling works across all browsers.
  - 🔧 FIXED: Large active color preview box (`#mainPreview`) lacked pointer cursor and click handler to copy HEX, saved swatches could not be deleted individually without wiping the whole library via "Clear Saved", and the app lacked a Help guide. Added click-to-copy on `#mainPreview`, right-click removal on saved swatches, and implemented a Help & Keyboard Shortcuts modal (`helpModal`, bound to `H` / `?`).

- **KColosseum**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core functionality works (Roman gladiatorial ludus management simulator with recruit market, dynamic attribute training STR/AGI/VIT, weapons Gladius/Trident and armor Lorica/Scutum equipment, 5 league tiers Local Pits to Champion of Rome, canvas arena combat engine with dynamic lunge animations and CRT shake, 4 tactical actions Attack/Defend/Showboat/Flee, special boss encounters Ferocious Lion / Armed Chariot / Twin Gladiators, dynamic crowd favor system with Denarii coin drops & medical sponge healing, audio synthesizers).
  - 🔧 FIXED: Crowd favor medical sponge healing did not decrease `currentFighter.damageTaken`, causing all healed health to be instantly wiped out upon exiting combat and when re-entering the arena. Added damageTaken recalculation upon crowd heal.
  - 🔧 FIXED: `exitArena()` bypassed updating `currentFighter.desc`, leaving the fighter's card on the ludus dashboard displaying stale pre-battle HP until modified or reloaded. Added `updateDesc(currentFighter)` on match exit.
  - 🔧 FIXED: Combat action buttons (`btnAttack`, `btnDefend`, `btnShowboat`, `btnFlee`) remained active during enemy turns, allowing button spam, double attacks, and simultaneous queued enemy counterattacks. Added button disabling during action resolution.
  - 🔧 FIXED: Defeating the Ferocious Lion or Armed Chariot bosses left them standing, breathing, and spinning scythe blades because `drawLionVisual` and `drawChariotVisual` lacked `state.stance === 'dead'` handling. Added collapsed/overturned defeat rendering states.
  - 🔧 FIXED: Player death called `owned.splice(-1, 1)` if `currentFighter` index was not found, deleting the last owned gladiator. Added `deadIdx !== -1` bounds check.
  - 🔧 FIXED: Gladiators with damage exceeding treasury funds could not be healed even partially, softlocking players when fighters reached 0 HP; and players who lost all fighters and funds had no recourse. Added partial healing up to available funds and implemented an emergency Patron Relief Grant (+200D) when bankrupt.
  - 🔧 FIXED: Entire app lacked keyboard shortcuts, backdrop modal dismissal, Ludus sound FX, and game persistence. Bound keys 1–4 and A/D/S/F for combat, H for guide, Esc for modal/flee, added audio cues to buy/train/heal/equip/refresh, and implemented localStorage save/load state persistence.

- **KColony**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core gameplay works (planetary colony builder with 7 biomes/scenarios Mars/Cryo/Volcanic/Acid/Sandbox/100-Day Survival/Resource Rush, 19 interactive structure types, 13 tech research projects, dynamic alien assaults with laser/turret defense, 5 planetary expeditions and deep cavern diving, orbital defense umbrella with tactical kinetic orbital strikes [Space], procedural weather/disaster events, Web Audio synthesizers, cybernetic CRT particle and shockwave engine).
  - 🔧 FIXED: "Inspect" mode (`[0/Esc] Inspect`) was a dead stub that returned silently on cell click (`if (currentType === 0) return;`), rendering sector inspection completely non-functional. Implemented interactive scanner inspecting tile coordinates, structure operational status, power/resource output, shielded status, damage assessment, and alien lifeform scan.
  - 🔧 FIXED: Clicking any of the 13 Tech Tree research buttons with insufficient Science failed silently with no user feedback or prompt. Added dynamic cost evaluation and informative toast feedback stating required vs current Science.
  - 🔧 FIXED: Keyboard shortcut guide promised hotkeys for Core Structures (Solar `S`, Farm `F`, Mine `M`, Battery `B`, Lab `L`, Nuclear `N`, Hydroponics `Y`, Laser `D`), but none of these letter keys were wired in `keydown`, and hotkeys were not suppressed when the Administrator's Manual was open. Wired all letter shortcut aliases, suppressed background actions while reading Help, and added guidance toasts when attempting to select locked structures.
  - 🔧 FIXED: Constructing structures or executing repairs failed silently when lacking required Mat, Power, or AdvM, and clicking occupied tiles gave no indication. Added detailed resource shortage warnings, occupied sector alerts, and repair status feedback.
  - 🔧 FIXED: Nanite Swarm anomaly (`activeAnomaly = 2`) providing +20% colony productivity was implemented in the game loop but was completely unreachable across all expeditions and events. Added Nanite Swarm anomaly discovery chance to Scout Recon.
  - 🔧 FIXED: Challenge scenarios (100-Day Survival and Resource Rush) did not display objective progress or deadlines in the topbar, and players had no button or shortcut to return to the scenario select menu once a game started. Added real-time objective/quota tracking to `lbl-planet`, added `[Menu / Scenarios]` topbar button, added victory celebration particle bursts, and added a colony wipeout defeat check.
  - 🔧 FIXED: Grid cells lacked hover tooltips, making it impossible to identify which structure was damaged under the red hazard marker or what was built on each sector. Added dynamic `cell.title` tooltips displaying structure name, shielded state, and alien hostiles.

- **KClock**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core functionality works (8-tab multi-tool suite with local precision clock, multi-city world clock grid with live day offsets and delta badges, timezone offset difference calculator, Unix Epoch timestamp ticker with ISO week / DOY / Julian Day and bidirectional timestamp converter, stopwatch with fastest/slowest lap detection, countdown timer with quick presets and custom minutes, repeating weekly alarms manager with snooze and chime synthesizers, JSON configuration export/import, keyboard shortcuts).
  - 🔧 FIXED: Content area HTML contained a stray comment delimiter (`<!-- Content Area -->-->`) which rendered literal `-->` text in the browser page body. Removed stray delimiter.
  - 🔧 FIXED: Header toggle buttons (`sound-toggle-btn` and `format-toggle-btn`) wiped out their child keyboard shortcut badge elements (`<span class="badge-key">`) on click via `.innerText` assignments and lacked toast feedback; updated handlers to preserve shortcut badges with `.innerHTML` and display state toasts.
  - 🔧 FIXED: World Clock day difference calculation on month boundaries compared raw day numbers (`localDay` vs `targetDay`), displaying `+1 Day` instead of `-1 Day` when the target city was on the 31st of the previous month while local was on the 1st. Refactored to calculate calendar day diffs using midnight date offsets. Also added empty state and toast notifications when adding or removing cities.
  - 🔧 FIXED: Timezone Calculator dropdowns (`#calc-src-city` and `#calc-tgt-city`) only offered 9 cities, omitting Singapore, Honolulu, São Paulo, Cairo, and Auckland available in World Clock. Synchronized all 14 timezones across calculator dropdowns, added a quick "Now" button, and converted target time calculations to UTC wall-clock time so calculations remain immune to local client DST shifts.
  - 🔧 FIXED: Countdown timer minutes input (`#timer-mins`) lacked an `oninput` handler, leaving the big countdown display desynchronized until Start or Reset was clicked. Added `onTimerMinsChange()` live input handler.
  - 🔧 FIXED: Stopwatch button remained labeled "Start" instead of "Resume" when paused with elapsed time, and clicking "Lap" while stopped was completely silent. Updated button label to "Resume" when paused and added informative feedback for lap recording.
  - 🔧 FIXED: Snoozed alarms were saved to `localStorage` as standard weekly repeating alarms with `days: [snoozedDay]`, causing snoozed alarms to ring weekly forever. Added `isOneTime: true` flag and automatic deactivation upon ringing. Also added sandboxed iframe clipboard fallback for live epoch copying, appended download anchor to DOM before `.click()` in JSON export, added empty state for alarms, and optimized `updateEpochTicker` to skip background execution when not on the Epoch tab.

- **KChess**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core gameplay works (standard 8x8 chessboard with 3D ornate lighting and procedural wood grain, Campaign Mode with 20 historical/tactical stages, Free Play, 6 tactical endgame Puzzles, 3-minute Blitz clock, 4-tier AI engine with alpha-beta pruning Minimax, real-time ECO Opening Book classifier, move history undo/redo stack, Quick Save (F5) / Load (F9) state persistence, FEN and PGN import/export).
  - 🔧 FIXED: Puzzle Mode progression was broken because `puzzleIndex` was never incremented upon solving a puzzle in `handleSquareClick` or `handleHotkey('r')`, permanently trapping players on Puzzle #1. Added automatic `puzzleIndex` advancement across solved puzzles #1–6.
  - 🔧 FIXED: 2-player pass-and-play (`vs Player`) was advertised in help guide and top status text, but `aiMode` was permanently locked to `true` with no keybinding or click handler to toggle it. Bound 'T' key to toggle `aiMode` and enabled top mode bar clicks.
  - 🔧 FIXED: `getCapturedPieces()` hardcoded standard 16-piece starting counts, generating up to 20 phantom captured pieces in the tray and distorting net material advantage calculations during Campaign, Puzzle, and FEN games. Refactored to dynamically compute missing pieces relative to the match's starting board snapshot.
  - 🔧 FIXED: Capture tray panel (`capBox`, height 75px) physically collided with and covered the top board file coordinate letters (A–H at y:95) and rank 8 board border. Rescaled and repositioned `capBox` and file labels so coordinates and board tiles render completely unobstructed.
  - 🔧 FIXED: Canvas button labeled `FEN/PGN [E]` only opened FEN modal, with no visual mechanism to access PGN Move History without hotkeys. Added cross-modal navigation buttons (`PGN View [G]` and `FEN View [E]`) across both modals, and enabled Enter key submission on `#fen-input`.
  - 🔧 FIXED: `copyFEN()` and `copyPGN()` threw unhandled promise rejections on sandboxed/iframe clipboard errors. Wrapped clipboard writes with `document.execCommand('copy')` fallbacks.
  - 🔧 FIXED: `getSAN()` omitted `=Q` on pawn promotions, outputting malformed SAN/PGN (e.g. `e8` instead of `e8=Q`), while `loadPGN()` failed on promotions and multiline brace comments. Added `=Q` promotion notation and multiline comment/disambiguation tolerance to PGN parser. Also added touch `onpointerdown` and status bar click-for-hint/restart.

- **KChat**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core functionality works (multi-room channel messaging, interactive polling suite with live vote tallies & percentage bars, channel topics and pinned message banners, AI persona interactions Assistant/Cyberpunk/CodeBot/Sarcastic/Cerberus, message reactions suite, search filter, JSON and TXT export/import, Firebase Realtime Database Global room and custom TCP/WebSocket server connection, comprehensive keyboard shortcuts suite and cybernetic help modal).
  - 🔧 FIXED: Custom channels created via `+ Room` modal or `/join`, or imported from JSON/localStorage, were never rendered into `#channelBar` upon reload, and active channel pill styling was desynchronized on startup. Implemented dynamic `renderChannelBar()` to render all channel pills with shortcut badges and exact active room highlights.
  - 🔧 FIXED: `<select id="aiPersona">` was not synchronized with `activePersona` loaded from `localStorage` on page initialization, causing the UI dropdown to remain stuck on "Assistant" despite active Cyberpunk/CodeBot persona state. Added startup persona value sync.
  - 🔧 FIXED: Clearing chat logs (`clearLog()`) and creating new polls (`createPoll()`) updated in-memory arrays but omitted `safeSaveState()`, causing cleared messages to reappear and newly created polls to vanish on page refresh. Added `safeSaveState()` across both operations and JSON imports.
  - 🔧 FIXED: Modal form inputs across Create Poll, Create/Join Channel, and Edit Topic lacked Enter key handlers, ignoring Enter key presses when typing. Added Enter key navigation and submission across all modal inputs.
  - 🔧 FIXED: Custom server Connect button defaulted to IP `127.0.0.1` and Port `6667`, but virtual node simulation only accepted ports 8080–8082, causing connection attempts with default parameters to immediately error with a socket failure. Added port `6667` to the virtual node simulation suite.
  - 🔧 FIXED: Sending messages while connected to Firebase Global Room duplicated messages locally because `send()` added the message to the log immediately and `onChildAdded` added it again upon receiving the RTDB broadcast. Added outgoing message deduplication to `firebaseListener`.
  - 🔧 FIXED: Slash commands `/poll`, `/topic`, `/join`, and `/ai` without arguments fell through as ordinary chat messages because handlers required trailing spaces. Added modal and AI dispatchers for bare commands, and updated `/join <#room>` to register new channels in `rooms` and re-render the channel bar.

- **KChart**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core functionality works (6 interactive chart rendering engines Bar/Line/Area/Pie/Donut/Radar, trendline overlay suite Linear Fit OLS/3-point Moving Average/Mean baseline, real-time 8-metric statistical analysis suite with OLS slope and R² fit, tabular data point editor with live inline validation and Enter key advancement, 6 color themes, 5 sample domain presets, SVG/PNG/CSV/JSON export, file import & drag-and-drop, full keyboard shortcuts suite, cybernetic help guide modal).
  - 🔧 FIXED: 3-point Moving Average trendline overlay (`trendMode === 'movavg'`) was omitted from vector SVG export (`exportSvg`), causing SVG downloads to exclude active moving average trend paths and labels present on canvas. Added smoothed moving average path calculation and legend badge to SVG export.
  - 🔧 FIXED: Radar mode SVG export lacked minimum point count validation (unlike canvas which requires ≥3 points), causing malformed SVG polygon outputs when exporting datasets with 1 or 2 points. Added radar point validation and warning toast.
  - 🔧 FIXED: Cartesian Y-axis grid labels in canvas and SVG export used `Math.round(maxVal - (maxVal / ticks) * i)`, which generated duplicate integer labels (e.g. `3, 3, 2, 1, 1, 0`) on fractional or small-scale datasets like the Fitness preset. Added dynamic decimal precision formatting (`.toFixed(1)`) for small ranges.
  - 🔧 FIXED: PNG export (`btnExportPng`) rendered the canvas directly onto a transparent background, causing dark gray axes and labels to appear illegible or washed out in standard image viewers. Export now composites onto a solid `#09090b` canvas matching SVG styling and temporarily suppresses active hover tooltips during capture.
  - 🔧 FIXED: Preset dropdown (`presetSelect`) retained its selected value key after loading, preventing users from re-selecting or resetting that same preset after editing points. Reset `presetSelect.value = ''` after import so presets remain immediately re-selectable.
  - 🔧 FIXED: Table row deletion (`del-btn`) relied on `e.target.dataset.index`, which could fail if clicks landed on child nodes. Switched to `btn.dataset.index`, and updated `btnAddRow` to auto-scroll `.table-wrapper` to bottom so new points are immediately visible.
  - 🔧 FIXED: Canvas lacked pointer/touch events (`onpointermove`, `onpointerdown`, `onpointerleave`) and CSS `touch-action: none`, hindering hover inspection on touchscreens, and radar spoke hover angle suffered from top-axis angle wrap discontinuity. Added pointer event listeners, touch-action styling, normalized spoke angle calculations, and enabled Escape key modal dismissal.

- **KCalendar**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core functionality works (interactive Month/Week/Day/Agenda views, custom category & priority color coding, daily/weekly/monthly/yearly recurrence engine, live keyword search and multi-criteria filters, event creation/editing modal, delete confirmation modal, calendar analytics overview modal, iCalendar .ics export/import, CSV export/import, Markdown agenda report generation, JSON backup export/import, keyboard shortcuts).
  - 🔧 FIXED: Week View event card clicks did not stop propagation, and day column clicks restricted `e.target` strictly to the outer container. Consequently, clicking the column header, the date label, or the "No events" placeholder failed to open the event creation modal. Stopped propagation on event cards and enabled column-wide clicking to create events.
  - 🔧 FIXED: Day View event card clicks did not stop propagation, and hour row clicks restricted `e.target` to row or events container. As a result, clicking the hour label (e.g. "09:00") failed to open the event creation modal. Stopped propagation on event cards and enabled row-wide clicking to schedule events directly at that hour slot.
  - 🔧 FIXED: Agenda View event cards were non-interactive and only responded when clicking the tiny "Edit" button. Added pointer cursor and click handler to the entire card so clicking an agenda entry immediately opens the edit modal.
  - 🔧 FIXED: Event Modal form inputs (title, date, time) lacked Enter key handlers, ignoring Enter key presses when typing. Added Enter key submission listeners across all inputs.
  - 🔧 FIXED: Pressing Escape while the Delete Confirmation dialog was open simultaneously closed both the confirmation modal and the underlying event edit modal. Configured Escape to dismiss only the delete confirmation overlay when active.
  - 🔧 FIXED: iCalendar (.ics) import failed to recognize standard formats with parameters/timezones (e.g. `DTSTART;VALUE=DATE:` or `DTSTART;TZID=...`), dropped escaped characters (`\,`, `\;`, `\n`, `\\`), and broke recurring events when RRULE included parameters (e.g. `RRULE:FREQ=WEEKLY;BYDAY=MO`). Added regex-based frequency extraction, unescaping, and parameter-agnostic DTSTART/field parsing, and added DTSTAMP and VALUE=DATE formatting to .ics exports.
  - 🔧 FIXED: CSV import hardcoded column indices assuming column 0 was an ID, which inverted titles and dates on standard CSV imports lacking an ID column. Implemented dynamic header column matching for robust imports, made string escapers null-safe, and enabled case-insensitive file extension detection for imports (.ICS/.CSV/.JSON).

- **KCalc**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core functionality works (scientific calculator with direct formula bar and 49-key keypad, degree/radian mode switching, multi-register memory M/M1/M2/M3 management modal, financial tools suite Loan PMT / Compound FV / Profit Margin / CAGR with result forwarding into calculator, descriptive 1-variable statistics suite with 10 computed metrics, 2-variable linear regression model with correlation/determination and interactive y-prediction, scientific constants library, calculation tape history with live search and CSV/TXT export, help & keyboard shortcuts modal).
  - 🔧 FIXED: Random number generator button `rnd` appended `rnd` which evaluated to the raw function `() => Math.random()` in `evalExpression()`, failing `isNaN()` check and permanently displaying `Error` on `=`. Updated evaluator to rewrite standalone `rnd` token to `rnd()` and added `rnd`, `^2`, and `^3` to backspace keyword tokens.
  - 🔧 FIXED: Modulo operator (`%` / `mod` button) was broken because percentage replacement `(\d+)%` unconditionally replaced `%` with `(val/100)`, turning binary expressions like `10%3` into `(10/100)3` which crashed with a JS SyntaxError. Updated regex to only match percentage `%` when not followed by another operand/variable, and mapped `mod` keyword to `%`.
  - 🔧 FIXED: `switchFinTool(toolId)` called `event.target.classList.add('active')` without receiving `event`, throwing `ReferenceError: event is not defined` in standards-compliant browsers and breaking tab switching. Assigned explicit element IDs (`tab-fin-*`) and decoupled tab activation from global event state.
  - 🔧 FIXED: Typing in the formula input bar desynchronized `expr` because `handleFormulaKey` assigned `expr = e.target.value` on `keydown` before the browser committed the typed character. Added `oninput="expr = this.value"` and streamlined `handleFormulaKey` for Enter execution.
  - 🔧 FIXED: "Clear" buttons in the 1-Variable Statistics and 2-Variable Regression tools emptied input textareas but left all 10 summary metric tiles and regression model tiles displaying stale results. Implemented `clear1VarData()` and `clear2VarData()` to reset input and metrics displays.
  - 🔧 FIXED: Initial financial tool cards displayed `$0.00` with uninitialized `dataset.rawVal` until manually clicking Calculate, causing "Use Result" to do nothing, and Profit Margin tool lacked a "Use Result" button present in PMT/FV/CAGR. Invoked all financial calculations on `DOMContentLoaded` and added "Use Result" to Profit Margin.
  - 🔧 FIXED: User Guide modal promised shortcuts `1`–`5` for calculator mode switching, but pressing `1`–`5` entered digits into the calculator keypad. Configured `1`–`5` to switch modes whenever outside Scientific mode (and with Alt/Ctrl anywhere), and clarified shortcut documentation.

- **KBudget**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (income/expense transaction ledger, summary balance/income/expense cards, interactive pie chart for category breakdown, multi-field search and 4-way sorting, 20-item pagination controls, Add/Edit transaction modal, Settings modal with custom currency symbol persistence, CSV import and export, print stylesheet layout, keyboard shortcuts Ctrl+N/F/S/O/Escape).
  - 🔧 FIXED: Date display shifted backwards by 1 day in western timezones (e.g. Sep 5 showed as Sep 4) due to ISO 8601 UTC midnight parsing in `new Date(dateString).toLocaleDateString()`, and `dateInput.valueAsDate = new Date()` assigned UTC dates. Updated `formatDate()` to parse local date components and set local calendar values.
  - 🔧 FIXED: `exportCSV()` used `data:text/csv;charset=utf-8,` with `encodeURI()`, which failed to escape `#` characters in descriptions (e.g. "Invoice #42"), causing browsers to treat `#` as a URL fragment identifier and corrupt/truncate the downloaded CSV. Switched to `Blob` and `URL.createObjectURL()`.
  - 🔧 FIXED: `importCSV()` split rows only on `\n`, retaining carriage returns (`\r`) on Windows and Excel CSV exports, and lacked positive amount checks. Updated parser to split on `\r?\n`, validate `amount > 0`, and accept flexible income labels.
  - 🔧 FIXED: Negative balances formatted as `$-X.XX` instead of `-$X.XX` because `formatCurrency()` prepended the currency symbol directly to negative numbers. Updated formatter to place negative signs before currency symbols.
  - 🔧 FIXED: Strict equality checks (`x.id === id`, `t.id !== id`) caused Edit and Delete operations to fail if transaction IDs were numeric in localStorage. Converted comparisons to string representations and sanitized IDs in event attributes.
  - 🔧 FIXED: Editing transactions with custom or imported categories not in the hardcoded default arrays wiped the category and reset it to "Food". Updated `openModal()` to dynamically append missing categories to the select dropdown, and added HTML escaping to transaction list rendering.

- **KBreakout**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core gameplay works (Classic/Hard/Multi-Ball Chaos campaigns, 40 stages, 6 active cyber skills Laser/Split/Fire/Barrier/Gravity/Satellite, Cyber-Forge Lab with 7 crafting recipes, dynamic brick types Quantum Resonance/Prism Reflector/Explosive/Hazard/Steel, boss fortress encounters, particle explosion physics, high scores and material persistence).
  - 🔧 FIXED: `initAudio` and `playTone` were called by all skill buttons, Cyber-Forge craft buttons, brick bounces, explosions, and game-over states, but neither function was defined, throwing fatal ReferenceErrors on every interaction. Implemented full Web Audio synthesizer suite with dynamic gain envelopes.
  - 🔧 FIXED: Undefined variable `screenShake` was assigned in `triggerQuantumResonance()` and `triggerExplosion()`, silently breaking screen shake intensity and rotational damping. Routed calls to `triggerScreenShake()`.
  - 🔧 FIXED: Key 'F' was bound to both FIR (Fire skill) and `toggleForge()`, causing the Cyber-Forge lab overlay to unexpectedly open during gameplay whenever activating the Fire skill. Removed 'F' from forge toggle, updated title screen prompt to 'O', and restricted 1–7 crafting keys to when the forge modal is open.
  - 🔧 FIXED: High score was loaded and saved to localStorage, but was never updated when `score > high_score`, leaving the displayed high score permanently stuck. Added high score tracking in `update()` and `saveGameData()`.
  - 🔧 FIXED: Canvas had `cursor: crosshair` but lacked all mouse/touch listeners, preventing mouse, touchpad, and mobile users from moving the paddle, launching stuck balls, or clicking to start/retry. Added `pointermove` and `pointerdown` event listeners.
  - 🔧 FIXED: Boss fortress core was declared with health bar container and laser damage checks, but the boss chassis was never drawn on canvas, boss movement was missing, the health bar fill width was never updated, and balls could not collide with the boss. Added boss canvas rendering, movement update, health bar fill sync, and ball-boss bounce collision.
  - 🔧 FIXED: Opening the Cyber-Forge lab overlay during gameplay did not pause the game loop, allowing active balls to fall and lose player lives while viewing recipes. Paused game update while `forge_open` is active.

- **KBase**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (simultaneous 64-bit live multi-base converter for Bin/Oct/Dec/Hex/Custom 2..36/ASCII, text string encoding suite Base64/URL/Hex/SHA-256 with clipboard copy, 64-bit interactive toggle board with shifts, rotations ROL/ROR, reversal, byte swap endianness, 64-bit bitwise logical matrix calculator AND/OR/XOR/NOT/SHL/SHR/SAR/ROL/ROR, IEEE-754 32-bit single and 64-bit double precision floating point breakdown strips and formulas, variable-length integer suite LEB128/SLEB128/Protobuf ZigZag encoder and decoder with binary layout breakdown, multi-width 8/16/32/64-bit two's complement and sign-magnitude inspector, structured bitfield slicer with bit mask and metrics Popcount/CLZ/CTZ/Parity/Pow2, activity history logging with CSV and JSON export, help and shortcuts modal).
  - 🔧 FIXED: User guide explicitly promised Enter keyboard shortcut to execute/recalculate active conversions or bitwise operations, but Enter was unhandled in keydown listener. Added Enter key handler dispatching to active input/tab calculation.
  - 🔧 FIXED: Bitwise operator suite inputs (`op-a`, `op-b`) did not live-update the matrix display on input and none of the 9 operator buttons indicated active operator state. Added live input listeners, `currentBitwiseOp` tracking, and dynamic active button highlight.
  - 🔧 FIXED: Activity log was spammed on every keystroke in IEEE-754 float input and Varint integer input, and unconditionally recorded 3 entries on initial page load. Added `recordHistory` flag so live typing and page load do not pollute the history log while button clicks and Enter key triggers record operations cleanly.
  - 🔧 FIXED: Varint hex decoder failed when receiving continuous hex byte sequences (e.g. `E58E26`) because `parseHexBytes` assumed space separators and parsed continuous hex as a single integer > 255. Updated parser to split continuous hex strings into 2-character byte pairs.
  - 🔧 FIXED: "Clear All" button in multi-base converter cleared all base text inputs but left `bitboardVal` with stale bits, causing the 64-bit board to desync from inputs. Wired up `bitboardVal = 0n` and `renderBitboard()`.
  - 🔧 FIXED: IEEE-754 single-precision formula generated invalid expressions for special values (Zero, Subnormals, ±Infinity, NaN). Added dedicated formula branches for zero, subnormals, infinity, and NaN. Also added cursor pointer styling and fallback copy handling to `str-output`.

- **KBBS**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (virtual ANSI terminal emulator with canvas rendering, CRT scanline effects, multi-node dialing directory with JSON import/export, interactive door games Legend of the Red Dragon & TradeWars 2015 streaming to terminal, EchoNet FidoNet EchoMail reader with message posting, reply quoting, and .MSG export, binary file transfers XMODEM/ZMODEM, session log capture/export as ANSI/TXT, keyboard shortcuts).
  - 🔧 FIXED: Unescaped single quote (`'''`) and backslash (`'\'`) in the `CP437` character mapping array triggered a fatal `SyntaxError: Unexpected string`, crashing the entire script before execution in browsers. Properly escaped characters to `'\''` and `'\\'`.
  - 🔧 FIXED: `selectMacro()` had an unclosed `if` block, nesting `saveMacro()` inside it and failing to populate `macroString` into the textarea when selecting a macro. Terminated the block correctly and restored script string binding.
  - 🔧 FIXED: "New" button in the Macro editor referenced `onclick="newMacro()"` which was undefined, throwing a ReferenceError. Implemented `newMacro()` to clear inputs and initialize a script template.
  - 🔧 FIXED: Display Settings modal had an empty `<div id="paletteContainer">` with no interactive color inputs, and `setBlinkRate` was neither loaded nor saved. Added `renderPaletteEditor()` with 16 color inputs, persisted custom palettes to `kbbsSettings.palette`, and wired up `setBlinkRate`.
  - 🔧 FIXED: Terminal scrollbar container had `onscroll="handleScroll(event)"` but `handleScroll` was never defined. Added `handleScroll(event)` handler calculating proportional `scrollOffset` and re-rendering the terminal canvas.
  - 🔧 FIXED: ANSI Art Viewer presets "sunset" and "acid" fell back to a generic label without artwork, `artPaletteSelect` did not tint presets, and `updateArtBaud` was an empty no-op. Added retro artwork presets with palette theming (Amber, Green, Cyber) and status toast notifications.

- **KAudio**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (virtual piano keyboard, ADSR envelope, biquad filter, overdrive/distortion, stereo delay/echo, 16-step 4-track sequencer, 8-bit sound FX generator presets, dual-mode visualizer oscilloscope/FFT spectrum, JSON import/export, WAV offline master export, performance recording/playback).
  - 🔧 FIXED: Sequencer notes called `playNote(0, true, ...)` with `freqOverride`, which bypassed storing the voice in `activeOscs`. As a result, `stopNote(0, true)` could not find the voice, causing sequencer oscillators to never stop and continuously leak voices into the Web Audio context. Assigned unique `seq_${trIdx}` voice keys so notes cleanly trigger ADSR release and terminate.
  - 🔧 FIXED: Automated sequencer playback triggered `triggerPreset()` with preset sound effects, spamming dozens of toast notifications per second. Added optional `showNotification = false` parameter when presets are executed via the sequencer.
  - 🔧 FIXED: Stopping recorded performance playback (`stopPlayback()`) cancelled pending `stopNote` timeouts without silencing currently active voices, leaving notes permanently ringing. Added voice cleanup loop on playback stop.
  - 🔧 FIXED: "Z: Rec, X: Play" status indicator and "Octave: 0" badge were non-interactive text elements, preventing mouse and touch users from recording/playing performances or shifting octaves. Added interactive click handlers with visual pointer styling and tooltips.
  - 🔧 FIXED: Virtual piano keys were not playable via keyboard navigation (Tab focus + Enter/Space) despite having `tabIndex="0"` and `role="button"`. Added `keydown`/`keyup` event handlers for Enter and Space.
  - 🔧 FIXED: Exporting WAV with an empty sequencer grid silently generated and downloaded 4 seconds of pure silence. Added active step validation with a user guidance toast before triggering offline rendering.

- **KAsteroids**: ISSUES FOUND ⚠️ (4 issues, 4 fixed inline)
  - ✅ Core gameplay works (ship rotation, thrust physics, lasers/spread/overdrive, UFO aliens, boss encounters, 8 active skills, asteroids fracturing, audio synthesizers, 4 game modes Classic/Time Attack/Hardcore/Campaign, local storage stats/highscores).
  - 🔧 FIXED: Menu items ([1] Classic, [2] Time Attack, [3] Hardcore, [4] Campaign, [S] Statistics, [H] How to Play, [B] Back to Menu) were unclickable `<p>` elements with no mouse click handlers or hover styles. Added `.menu-btn` styling and interactive click handlers.
  - 🔧 FIXED: Numpad keys (1–4) did not trigger game start in mode select menu because only `Digit1`–`Digit4` were handled. Added `Numpad1`–`Numpad4` mappings.
  - 🔧 FIXED: Starting a game from Statistics or How to Play sub-panels left `menuContent` hidden (`display: none`), causing subsequent game over screens to remain stuck showing the help/stats panel instead of the mode select menu. Added `showMenu()` reset in `initGame()`, `killShip()`, and campaign victory.
  - 🔧 FIXED: Mouse-clicking top active skill buttons (`btnEmp`, `btnLaser`, etc.) retained DOM focus on the button, causing subsequent `Space` (shoot) presses to re-trigger the skill button instead of firing cannons. Added `this.blur()` on click.

- **KAlchemy**: ISSUES FOUND ⚠️ (4 issues, 4 fixed inline)
  - ✅ Core gameplay works (element transmutations, apparatuses Retort/Alembic/Anvil, 3 game modes Classic/Blitz/Puzzle, Quests, Workshop upgrades, Potions brewing & effect tester, Daily trials, Magnum Opus Rebirth & Astral perks, Expeditions, Planetary cores, Familiars sanctuary, audio FX, Grandmaster manual modal).
  - 🔧 FIXED: "📖 Codex" equipment tab button called `selectEquipment('codex')` which had no branch in `selectEquipment()`, leaving the Codex container hidden and non-functional.
  - 🔧 FIXED: Selecting tier filter (or clicking tier cards in Codex) only triggered `renderGrimoire()` without updating the active Codex view when Codex was open.
  - 🔧 FIXED: Typing in the element search filter input didn't update the Codex element list when Codex was active.
  - 🔧 FIXED: Keyboard shortcut numbers were limited to 0–5, ignoring Tier 6 (Mythic) element filter on key '6'.

- **K2048**: ISSUES FOUND ⚠️ (5 issues, 5 fixed inline)
  - ✅ Core gameplay works (sliding tiles via arrow keys/WASD/swipe, merging, rulesets Classic/Fibonacci/Threes, active skills Upgrade/Rotate/Hammer/Undo, auto-play, 4 grid sizes, audio beeps, animations/particles).
  - 🔧 FIXED: "New Game" button in Campaign Mode called `initGame()` instead of `restartOrNextStage()`, creating an orphaned campaign state without stage target/obstacles.
  - 🔧 FIXED: Timed Campaign stages (e.g. Stage 4, 10, 16) activated background timer but left `#timer-display` invisible (`display: none`), concealing the countdown from the player.
  - 🔧 FIXED: Toggling Campaign Mode off didn't clear stage hazard flags (`timeAttackEnabled`, `obstaclesEnabled`, etc.), causing subsequent free play sessions to retain campaign hazard rules.
  - 🔧 FIXED: Initial campaign stage start didn't trigger `updateScore()`, showing stale standard game scores until the first move.
  - 🔧 FIXED: Keyboard shortcut Shift+H (intended for Help) was intercepted by the Hammer shortcut `k === 'h'`, executing a hammer smash instead of opening Help modal.

