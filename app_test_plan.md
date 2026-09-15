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

**Target App:** KMystery
**Status:** Next in queue

## Round-Robin Testing Queue (NEVER STOP — loop forever)
Pick the top app, audit it, write a test report, move it to bottom. One app per turn.

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
- KMandel
- KMatch3
- KMaze
- KMech
- KMedia
- KMine
- KMines

## Test Reports

> 📁 **Archived Reports**: Historical test reports have been archived to [archive/app_test_reports_archive.md](archive/app_test_reports_archive.md) to preserve token efficiency.

- **KMines**: ISSUES FOUND ⚠️ (8 issues, 8 fixed inline)
  - ✅ Core gameplay works (arcade/cyber minesweeper with 3 classic difficulty modes Easy 9x9/Medium 16x16/Hard 16x30, 60-second Rush mode with combo multipliers up to 8x, 20-stage tactical Campaign mode with speedrun timers and hidden treasure chests, 3 active power-ups Detector Bot/Sonar Radar Scan/Blast Shield, touch Dig/Flag mode toggle, dual-tier particle and shockwave explosion physics, procedural screen shake, and retro Web Audio sound effects).
  - 🔧 FIXED: In Rush Mode, clicking the restart smiley face button (`#face`) called `setDiff(currentDiff)` via `resetLevel()`, immediately terminating Rush Mode and switching back to Freeplay Easy. Added `else if (rushMode) startRush()` to restart Rush Mode properly.
  - 🔧 FIXED: Global keyboard listener checked bare keys (`1`..`4`, `c`, `r`, `d`, `s`, `h`, `q`, arrows) without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking browser accelerators: `Ctrl+C` (copy text hijacked to toggle Campaign), `Ctrl+R` (browser reload hijacked to fire Sonar scan), `Ctrl+S` (save page hijacked to activate Blast Shield), `Ctrl+D` (bookmark hijacked to deploy Detector), `Ctrl+H` (browser history hijacked to open Help), `Ctrl+1`..`Ctrl+4` / `Alt+1`..`Alt+4` (tab switching hijacked to switch difficulty/modes), and `Alt+ArrowLeft`/`Alt+ArrowRight` (browser navigation hijacked to move cursor). Added modifier guards across all shortcuts.
  - 🔧 FIXED: Keyboard listener intercepted `Space` and `Enter` even when toolbar buttons (`#btn-detector`, `#btn-sonar`, `#btn-help`, etc.) were focused, overriding native button activation and triggering unintended tile reveals/flags on the grid. Added `e.target.tagName` check to preserve standard button activation.
  - 🔧 FIXED: Pressing `Escape` when the Help modal was closed triggered `toggleHelpModal()`, popping open the manual when users pressed Esc to cancel or clear focus. Restricted `Escape` to closing the Help modal when displayed.
  - 🔧 FIXED: In Help modal shortcut manual, the table promised `WASD` for cursor navigation, which directly conflicted with `S` (Blast Shield) and `D` (Detector Bot) and was unhandled for `W` and `A`. Removed conflicting WASD notation and clarified arrow key navigation and Escape dismissal.
  - 🔧 FIXED: Power-up buttons (Detector, Sonar, Blast Shield) and right-click flagging before first tile reveal silently failed without feedback when charges were 0 or before game initialization. Added informative status banner notices and error buzz audio.
  - 🔧 FIXED: In `ontouchend`, lack of `e.preventDefault()` allowed mobile browsers to synthesize a secondary `mousedown` on the newly revealed cell, triggering an unintended chord reveal that could detonate adjacent unflagged mines immediately after tapping. Added `e.preventDefault()`, added middle-click mouse button (`e.button === 1`) chord reveal, and cleared stale question mark bit (`& ~16`) when Detector Bot flags a cell.
  - 🔧 FIXED: Every grid cell was initialized with `tabindex="0"`, creating up to 480 redundant tab stops across the board. Implemented ARIA roving tabindex (`tabindex="0"` on focused cell, `"-1"` on others), and synchronized initial Campaign button text (`Camp (C)` vs `Camp X/20 (C)`).

- **KMine**: ISSUES FOUND ⚠️ (2 issues, 2 fixed inline)
  - ✅ Core gameplay works (Minesweeper engine with 3 difficulty modes 10x10/16x16/30x16, guaranteed safe first click, recursive 0-cell flood fill, left-click chord revealing on fulfilled numbers, right-click flagging, safe move hint generator, quicksave and quickload states, frame-by-frame move replay system, personal best times tracker, Canvas particle explosion physics, and interactive help modal).
  - 🔧 FIXED: In `exportStats()`, the created download anchor element was clicked without being attached to `document.body` (`a.click()`), causing statistics JSON exports to fail silently in Firefox and sandboxed iframe environments. Attached anchor to DOM before clicking and cleanly removed it afterwards.
  - 🔧 FIXED: Global keyboard listener checked bare keys (`h`, `?`, `1`..`3`, `e`, `i`, `p`) without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking browser accelerators: `Ctrl+Shift+I` (developer tools hijacked to trigger JSON file import), `Ctrl+E` (browser search bar hijacked to export stats), `Ctrl+F5` (hard reload hijacked to quicksave), and `Alt+1`..`Alt+3` (browser tab switching hijacked to change difficulty). Added modifier key guards across keyboard shortcuts.

- **KMedia**: ISSUES FOUND ⚠️ (8 issues, 8 fixed inline)
  - ✅ Core functionality works (Multi-format HTML5 audio/video player with playlist management, synthetic WAV audio generator for instant in-browser demo playback, real-time Web Audio API frequency/time-domain oscilloscope waveform visualizer, 3-band parametric DSP equalizer Bass/Mid/Treble, GPU video post-processing filters Brightness/Contrast/Saturation, SRT and WebVTT subtitle parser with real-time overlay, fullscreen presentation, and single-frame video snapshot PNG exporter).
  - 🔧 FIXED: Restoring saved volume on startup used `parseFloat(safeGetStorage('kmedia_vol', '1')) || 1`, where a saved 0% volume setting was treated as falsy due to `0 || 1`, blowing out audio to 100% blast on page reload. Replaced with explicit `isNaN` parsing to preserve 0% volume settings.
  - 🔧 FIXED: Global keyboard listener checked bare keys (`h`, `p`, `n`, `s`, `m`, `f`, `u`, `e`, arrows) without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking browser accelerators: `Ctrl+H` (browser history hijacked to toggle Help), `Ctrl+P` (browser print hijacked to switch previous track), `Ctrl+N` (new window hijacked to switch next track), `Ctrl+S` (save page hijacked to stop playback), `Ctrl+F` (find in page hijacked to toggle fullscreen), `Ctrl+U` (view source hijacked to toggle mute), and `Alt+ArrowLeft`/`Alt+ArrowRight` (browser back/forward navigation hijacked for seeking). Added modifier guards across all shortcuts.
  - 🔧 FIXED: Keyboard shortcuts (`Space`, `P`, `N`, `S`, `M`, `Arrows`, `Del`, `F`, `U`) continued to fire beneath an open Help modal (`#helpOverlay`), manipulating playlists and audio while users read documentation. Suppressed background hotkeys while Help modal is displayed.
  - 🔧 FIXED: During active dragging of `#seekSlider`, the 200ms `setInterval` progress polling continually overwrote `UI.seekSlider.value`, causing erratic slider jitter and fighting user scrubbing. Added activeElement check to pause seek slider updates during drag.
  - 🔧 FIXED: In playlist items, pressing `Delete` on a focused item deleted the item via `li.onkeydown` and then bubbled to `window` keydown which executed `removeTrackByIndex(currentIndex)`, deleting two tracks in one keystroke. Added `e.stopPropagation()`.
  - 🔧 FIXED: Clicking `Clear` or removing the last track paused playback and removed `src` but failed to invoke `mediaPlayer.load()`, leaving the frozen video frame visible; and failed to reset `#seekSlider`, `#timeDisplay`, and `#waveform`. Added proper element reset and idle waveform redraw.
  - 🔧 FIXED: In `parseSubtitles()`, cues separated by multiple blank lines or containing cue IDs caused cue lines to be misaligned or dropped. Replaced with dynamic timestamp line matching (`-->`) to robustly support all SRT and WebVTT cue variations.
  - 🔧 FIXED: Changing tracks or reloading media reset `mediaPlayer.playbackRate` to 1.0x due to native HTML5 media element behavior. Added `loadedmetadata` event listener to persist the selected playback speed across tracks, and added audio context resumption on Play button click.

- **KMech**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (Tactical turn-based mech combat simulation with procedural SVG chassis and enemy sprite rendering across 3 enemy classes Scout/Goliath/Titan, hangar diagnostics and equipment cycling weapons/armor/heat sinks/specials, dynamic heat reactor management, multi-limb targeting system Head/Torso/Arms/Legs with critical multipliers, Web Audio synthesizer effects, 3-tier particle canvas physics with debris/smoke/shocks/screen shake, garage repair economy, and pilot XP leveling).
  - 🔧 FIXED: Global keyboard listener checked bare keys (`d`, `r`, `u`, `s`, `1`..`4`, `a`, `t`, `h`) without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking browser accelerators: `Ctrl+D` (bookmark hijacked to deploy/defend), `Ctrl+R` (browser reload hijacked to repair/return), `Ctrl+S` (save page hijacked to sell salvage), `Ctrl+U` (view source hijacked to consume salvage), `Ctrl+A` (select all hijacked to attack), `Ctrl+T` (new tab hijacked to cycle targeting), `Ctrl+H` (browser history hijacked to open manual), and `Ctrl+1`..`Ctrl+4` (tab switching hijacked to cycle gear). Added modifier key guards across all keyboard shortcuts.
  - 🔧 FIXED: In `enemyTurn()`, damage was applied inside a 140ms `setTimeout()`, but `actionDefend()` synchronously set `isDefending = false` immediately after invoking `enemyTurn()`. As a result, `isDefending` was always false when damage was computed, completely breaking Defend and providing 0 damage reduction; kept `isDefending` active through the enemy attack exchange and reset it after cooling.
  - 🔧 FIXED: In `actionAttack()`, line 1661 checked `enemyStats.hp > 0` synchronously before the player's 120ms damage timeout ran, scheduling an `enemyTurn()` at 350ms. If the player's attack destroyed the enemy, the dead enemy still fired a projectile from the grave at 350ms, inflicting damage on the victory screen and overwriting Victory with Defeat if lethal; additionally guarded `enemyTurn()` against running when `enemyStats.hp <= 0` or when post-battle actions are active.
  - 🔧 FIXED: In `cycleArm()`, changing armor assigned `playerStats.hp = playerStats.maxHp`, granting free 100% health restoration by simply cycling armor back and forth, completely bypassing the credit repair cost and salvage parts mechanics. Clamped health with `Math.min(playerStats.hp, playerStats.maxHp)`.
  - 🔧 FIXED: No debounce or busy lock existed during combat actions (`actionAttack()` / `actionDefend()`), allowing rapid keypresses or mouse clicks to launch multiple concurrent attacks, leading to runaway heat generation, duplicate enemy counter-attacks, and desynchronized animations. Added `isProcessingTurn` state lock during combat sequences.
  - 🔧 FIXED: Trailing duplicate HTML and script tags (`</html>    }); </script> </body> </html>`) at the bottom of the file formed corrupted markup outside the root document. Cleaned up trailer.

- **KMaze**: ISSUES FOUND ⚠️ (7 issues, 7 fixed inline)
  - ✅ Core gameplay works (Raycasting 3D labyrinth engine with 45 descent stages across 5 biomes Catacombs/Cyber/Frost/Abyssal/Inferno, procedural maze generator, 4-layer particle physics, Minotaur AI with footstep stealth detection, Boss combat with Overlord on floor 45, 5 relics Pickaxe/Pathfinder/Speed Shoes/Stun Spray/Time Freeze, crouch stealth mechanics, wall torch lighting, frame-by-frame replay viewer, checkpoint save/load, and stats export/import).
  - 🔧 FIXED: In `exportStats()`, the created download anchor was clicked without being attached to the DOM (`a.click()`), causing statistics JSON exports to fail silently in Firefox and sandboxed iframe environments. Attached anchor to `document.body` prior to clicking and cleanly removed it afterwards.
  - 🔧 FIXED: Global keyboard listener checked bare keys (`h`, `k`, `r`, `p`, `c`, `s`, `f`, `t`, `v`, `l`, `w`, `a`, `s`, `d`) without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking browser accelerators: `Ctrl+S` (hijacked to consume Speed Shoes), `Ctrl+P` (hijacked to swing Pickaxe), `Ctrl+F` (find hijacked to fire Stun Spray), `Ctrl+T` (new tab hijacked to fire Time Freeze), `Ctrl+C` (copy text hijacked to activate Pathfinder), `Ctrl+V` (paste hijacked to save checkpoint), `Ctrl+L` (address bar focus hijacked to load checkpoint), `Ctrl+H` (browser history hijacked to toggle Help), `Ctrl+K` (browser search hijacked to toggle Keybinds), `Ctrl+R` (browser reload hijacked to trigger Replay on victory screen), and `Alt+ArrowLeft`/`Alt+ArrowRight` (browser navigation hijacked to turn adventurer). Added modifier key guards across all keyboard hotkeys.
  - 🔧 FIXED: Keyboard shortcuts (`p`, `c`, `s`, `f`, `t`, `x`, `v`, `l`, `w`, `a`, `s`, `d`) continued firing beneath the Help & Codex modal (`#helpModal`), allowing blind movement and relic consumption; and the game loop in `update()` continued ticking, allowing roaming Minotaurs to trample and kill the player while reading instructions. Suppressed background hotkeys and paused `update()` when `#helpModal` is displayed.
  - 🔧 FIXED: Clicking `saveCheckpointGame()` (`btnSave` or `V`) outside of an active game session (`gameState !== 1`, e.g. start screen or win screen) saved invalid state with `elapsed: Date.now()` (56 years) and corrupted the player's saved descent. Added `gameState === 1` guard with warning toast.
  - 🔧 FIXED: In Keybinds mode (`gameState === 4`), clicking the `Keybinds` HUD button (`btnKeys`) overwrote `prevState` with 4, permanently trapping the user in Keybinds mode on Escape. Replaced with `toggleKeybinds()` so clicking the button or pressing `K` toggles in and out safely, handled Escape during active rebinding to cancel without closing, and added a high-contrast semi-transparent backdrop panel over the 3D raycast view for readability.
  - 🔧 FIXED: In `loadCheckpointGame()`, loading a checkpoint while in Keybinds mode left `gameState === 4` instead of restoring the game view. Explicitly set `gameState = 1` upon loading a valid checkpoint.
  - 🔧 FIXED: In `canvas.addEventListener('mousedown')`, clicking the canvas on start screen (`gameState === 0`) or victory screen (`gameState === 2`) did nothing, requiring physical keyboard `Enter` or `Space` to start descent. Added canvas click start support and enabled `ArrowUp`/`ArrowDown`/`W`/`S` in Replay mode to browse across completed stages.

- **KMatch3**: ISSUES FOUND ⚠️ (8 issues, 8 fixed inline)
  - ✅ Core gameplay works (Classic Match-3 gem engine with 4 special gems: Line Blaster, Rainbow Gem, 3x3 Bomb, Cross Blaster, 20-stage Campaign with escalating board sizes 6x6 to 10x10, Ice tiles, multi-hit Stone & Iron obstacles, Stage 20 Jewel King Boss with shield barrier mechanics, Zen infinite relaxation mode, Timed Rush speed challenge, 4 active skills Hammer/Extra Moves/Shuffle/Color Nuke, Web Audio synthesizer sound effects, multi-tier particle spark and canvas shockwave physics).
  - 🔧 FIXED: Global keyboard listener checked bare keys (`1`, `2`, `3`, `h`, `e`, `m`, `s`, `l`, arrow keys) without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, hijacking native browser accelerators: `Ctrl+S` (hijacked to spend 300 points and scramble board via shuffle), `Ctrl+H` (browser history hijacked to activate Hammer), `Ctrl+L` (address bar focus hijacked to activate Color Nuke), `Ctrl+E` (address bar search hijacked to buy extra moves), `Ctrl+1`..`Ctrl+3` / `Alt+1`..`Alt+3` (browser tab switching hijacked to reset the active stage and switch game modes), and `Alt+ArrowLeft`/`Alt+ArrowRight` (browser history navigation hijacked for gem swapping). Added modifier key guards across all shortcuts.
  - 🔧 FIXED: Background keyboard shortcuts (skills, mode switches, arrow swapping) continued to fire underneath an active Help modal (`#helpModal`), manipulating and restarting background games while users read documentation. Suppressed background hotkeys while `#helpModal` is displayed.
  - 🔧 FIXED: In Timed Rush or timed Campaign stages (e.g. Stage 4, 7, 10, 13, 16, 19), the countdown interval continued ticking down while the Help modal was open, causing stages to fail and trigger Game Over alerts while users read instructions. Added modal pause check to interval to pause countdown and hint timer while Help is displayed.
  - 🔧 FIXED: Clicking `[H] Hammer` or `[L] Color Nuke` powerup buttons activated their mode, but clicking the active button a second time did not toggle or cancel the mode, forcing users to click a cell or press Escape. Enabled click toggling to cancel active powerup modes.
  - 🔧 FIXED: In Zen mode (`gameMode === 1`), `moves = '∞'`. Clicking `[E] +Moves` deducted 300 score from the player without granting extra moves because `typeof moves === 'number'` evaluated to false. Added guard to prevent point deductions and display a toast informing the player that moves are infinite in Zen mode.
  - 🔧 FIXED: Clicking any skill button with insufficient score (<300 pts) produced zero audio or visual feedback. Added audio buzz and floating warning popup (`Need 300 pts!`).
  - 🔧 FIXED: `swapAndCheck()` lacked an `isProcessing` guard, permitting rapid duplicate arrow keystrokes during swap animations to launch concurrent swap routines that desynchronized grid state and created ghost cells. Added `if (isProcessing) return;` and cell bounds checks to `swapAndCheck()`, and guarded skill buttons against mid-cascade execution.
  - 🔧 FIXED: `loadGame()` failed to update `document.getElementById('movesLabel')`, leaving timed stages labeled as "Moves" instead of "Time". Synchronized moves label and dynamic `#btnMoves` text (`+15s` vs `+Moves`), added persistent `campaignLevel` in stats to prevent switching to Zen or Timed Rush from wiping campaign progress, ensured Hammer completely shatters multi-hit stone/iron obstacles in one hit per documentation, guarded Color Nuke against invalid non-gem clicks, and replaced `/0` target score with `BOSS` on Stage 20.

- **KMandel**: ISSUES FOUND ⚠️ (6 issues, 6 fixed inline)
  - ✅ Core functionality works (interactive fractal explorer supporting 5 formulas Mandelbrot/Burning Ship/Tricorn/Celtic/Buffalo, 12 landmark presets, Julia set explorer with interactive coordinate sampling, real-time zoom/pan navigation with 256-level undo/redo history, 7 color themes with custom dual-color gradient picker, and 4K Ultra-HD PNG renderer).
  - 🔧 FIXED: In `#btn-save`, download anchor element was clicked without being attached to `document.body` (`link.click()`), causing 4K PNG file downloads to fail silently in Firefox and sandboxed iframe environments. Attached anchor to DOM before clicking and cleanly removed it afterwards.
  - 🔧 FIXED: In `#btn-save`, button lacked a disabled-state lock during long-running 4K rendering, permitting rapid duplicate clicks that triggered concurrent multi-gigapixel canvas allocations and worker message race conditions. Added busy lock state guard on `#btn-save`.
  - 🔧 FIXED: Global keyboard listener checked bare keys (`s`, `r`, `0`, `f`, `l`, `p`, `t`, `j`, `h`, `+`, `-`) without verifying `!e.ctrlKey && !e.altKey && !e.metaKey`, intercepting standard browser accelerators: `Ctrl+S` (hijacked to export 4K PNG), `Ctrl+R` (hijacked to reset view), `Ctrl+0` (reset browser zoom hijacked to reset view), `Ctrl+F` (find in page hijacked to cycle formulas), `Ctrl+P` (print hijacked to cycle landmarks), `Ctrl+L` (address bar focus hijacked to cycle landmarks), `Ctrl+T` (new tab hijacked to cycle themes), `Ctrl+J` (browser downloads hijacked to toggle Julia mode), `Ctrl+H` (history hijacked to toggle help), `Alt+Z`/`Alt+Y` (hijacked for undo/redo), and `Alt+ArrowLeft`/`Alt+ArrowRight` (browser history navigation hijacked for viewport panning). Added modifier guards across all shortcuts.
  - 🔧 FIXED: Keyboard shortcuts continued to fire in the background underneath an open Help overlay (`#help-overlay`), mutating formulas, jumping landmarks, or triggering exports while users read the guide. Suppressed background hotkeys while the modal is open.
  - 🔧 FIXED: In `loadState()`, custom color picker inputs (`#color1`, `#color2`) were never updated to reflect the restored custom palette, leaving the pickers displaying stale colors upon undo/redo; additionally added 'change' listeners to save history when custom colors are picked.
  - 🔧 FIXED: In `updateDisplays()`, `iterDisp.textContent` was initially formatted before dynamic iteration depth recalculation, causing the iteration HUD to display stale values during deep zooms until subsequent frame updates; moved display update after recalculation and added `onerror` fallback to Web Workers instantiation to degrade to synchronous rendering instead of hanging the loading spinner.

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

