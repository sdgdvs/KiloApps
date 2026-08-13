# App Work Plan

## DIRECTOR NOTE (2026-07-29): STOP BLANKET FEATURE SPAM — FOCUS ON APP-SPECIFIC UTILITY

**⚠️ READ THIS BEFORE EVERY TURN. This supersedes old loop instructions.**

The old approach of adding generic "Search, Save/Load, and Import/Export" to every single app is OVER. A hex editor needs different features than a music synthesizer. Furthermore, the Feature Expander has been adding gameplay features (bosses, power-ups, campaigns) to Games — **STOP DOING THIS**. Gameplay content belongs to the Game Content agent.

**From now on, apps are split into categories with DIFFERENT work priorities:**

### 🛠️ SYSTEM & DEV TOOLS
*Apps: KTerm, KSys, KTask, KNet, KPing, KHex, KBase, KConverter, KCalc, KScript, KZip, KFont*
- **Focus:** Diagnostic depth, granular controls, advanced parsing, system accuracy.
- **Do:** Add packet sniffing logic, advanced regex search, macro scripting, deep memory inspection, hardware benching, large file handling.
- **Don't:** Add visual fluff. 

### 📝 PRODUCTIVITY & DATA
*Apps: KPad, KNote, KDB, KTodo, KJournal, KCalendar, KContacts, KMail, KRead, KPass*
- **Focus:** Data interoperability, workflow efficiency, robust search, data security.
- **Do:** Add CSV/JSON/Markdown import/export, AES encryption/password protection, multi-tab sessions, tagging, advanced filtering, auto-save.
- **Don't:** Add basic features that already exist. Deepen the data structures.

### 🎨 MEDIA & CREATIVE
*Apps: KPaint, KImage, KAudio, KSynth, KMedia, KChart, KGraph, KMandel, KType*
- **Focus:** Format support, processing algorithms, rendering performance, complex transformations.
- **Do:** Add new export formats (WAV, PNG, WEBP), audio/image filters (FFT, convolution matrices, ADSR), layer support, waveform visualization.
- **Don't:** Make them simple toys. Add professional-grade utility features.

### 🎮 GAMES (UTILITY ONLY)
*Apps: KConnect4, KMaze, KMine, KPac, KQuest, KSnake, KTetris, KSpace, KSolitaire, KRogue, KChess, KPong, KBBS*
- **Focus:** Meta-features and Engine Utility.
- **Do:** Add Save/Load states (F5/F9), High Score JSON Export/Import, Replay Viewers, PGN/FEN parsers, custom keybinding config.
- **Don't:** DO NOT add campaigns, power-ups, new enemies, or bosses. Leave gameplay content to the Game Content agent!

---

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 6 worker agents + 2 directors operate on this repo on overlapping schedules. You are the **Feature Expander**.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`app_work_plan.md`).** Read but NEVER edit:
  - `app_fix_plan.md` (QA agent), `game_content_plan.md` (Games agent), `new_app_plan.md` (Creator agent), `usability_plan.md`
- **Shared file `KiloOS/src/App.jsx`** — shared ownership. You may ONLY add entries to the APPS array. Protocol: `git pull` → make minimal APPS-only change → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — Do NOT edit.
- **Dual-target model:** Each app has a native C version (`K[Name]/main.c` + `build.bat`) and a web HTML5 version (`KiloOS/public/apps/k[name].html`). Both versions should offer functional parity where feasible. Web HTML files must be single self-contained files (inline CSS + JS, no imports).
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.
- **Version bumping:** If you modify KiloOS shell files or update versioning/changelog, bump the patch version in `KiloOS/package.json` AND update `MICROS_VERSION` in `KiloOS/src/App.jsx` so the opening screen displays the current version.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **Logging discipline:** Keep this plan file concise. A few lines per completed item. Do NOT dump file contents or create verbose logs.

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
- **Model Selection:** Always spawn worker subagents using the `flash` model (`Model: "flash"`) to prevent 503 server capacity bottlenecks.

**Graceful Termination Checklist (do this EVERY turn before stopping):**
1. Processed one item
2. Updated plan file
3. Committed and pushed
4. All subagents terminated (killed or completed)
5. STOP — call no more tools

---

**Target App:** KContacts
**Status:** In Queue
**Current Phase:** In Queue

## Round-Robin Continuous Improvement Queue (NEVER STOP — loop forever)
Pick the top app from this list, add a meaningful new feature based on its **CATEGORY PRIORITY**, and move it to the bottom. Update BOTH web and native versions. You have up to **999KB per app**.

- KContacts (Added Real-time Search & Group Filters, vCard [.vcf] & CSV Import/Export, Duplicates Merger, Contact Avatar Initial/Color Generator, and Quick Action buttons for Email/Phone)
- KRead (Added Real-Time Search & Highlight Navigation, Bookmarks Drawer & Persistence, Reading Statistics Engine [WPM & Time Remaining], Reader Themes & Font Controls, Highlights/Notes & JSON/TXT Export)
- KBase (Added Simultaneous Multi-Base Converter [BIN/OCT/DEC/HEX/Base 2..36], 64-Bit Interactive Toggle Board & Bitwise Operator Suite [AND/OR/XOR/NOT/SHL/SHR/ROL/ROR], IEEE-754 Floating Point Breakdown, and Conversion History Log & CSV/JSON Export)
- KJournal (Added Calendar Entry Navigator, Mood Tracker & Writing Streak Analytics, Entry Search & Hashtag Filtering, PIN Lock Security, and Data Import/Export [JSON/MD/TXT])
- KPad (Added Multi-Tab Document Sessions, Syntax Highlighting & Language Switcher, Find & Replace Panel with Regex Support, Line Numbers & Gutter Stats, and Document Export/Import)
- KImage (Added Image Adjustments & Filters [Grayscale, Sepia, Invert, Brightness/Contrast, Blur], Transformations [Rotate 90°, Flips, Crop, Resize], RGB Color Histogram & EXIF Metadata Inspector, Auto-Slideshow Mode with Timer, and Image Export/Format Conversion [PNG/JPEG/WEBP/BMP])
- KHex (Added Hex & ASCII Pattern Search/Replace, Data Inspector Panel [Int8..Int32, Float, Double, Endianness], Checksum & Cryptographic Hash Suite [CRC32, MD5, SHA-256], Byte Manipulation Operations [Fill, Invert, XOR Mask, Endian Swap], and Binary Data Export [Hex Dump, C Array, RAW])
- KCalc (Added Scientific & Financial Calculators, Interactive History Tape with Recall, Memory Storage Banks [M+, M-, MR, MC, MS], Scientific Constants Library, Expression Formula Evaluator, and History Tape CSV/TXT Export)
- KCalendar (Added Multi-view [Month, Week, Day, Agenda], Event Categories & Color Tagging, Real-time Search & Category Filters, Recurring Events Engine, and iCalendar [.ics]/CSV Import/Export)
- KChart (Added Multi-Type Charting Engine [Bar, Line, Area, Pie, Doughnut, Radar], Interactive Data Table Editor, Statistical Analysis Suite [Mean, Median, Std Dev, Min/Max], Palette Themes, PNG Image Export, and CSV/JSON Import/Export)
- KChess (Added PGN Move History Export & Import, FEN Board Position Loader & Exporter, AI Engine Difficulty Levels [Easy, Medium, Hard, Master], Interactive Undo/Redo Move Stack, Best Move Hint arrow/system, and Material Advantage & Captured Pieces Counter)
- KChat (Added Multi-Room/Channel Support, AI Persona Selector [5 Personalities], Real-time Search & Filtering, Message Pinning & Reactions, and Chat History Export/Import [JSON/TXT])
- KBBS (Added Door Games Mini-Suite [LORD/TradeWars text adventure], ANSI Color & Art Viewer Mode, Multi-Node Dialing Directory Manager, Session Log Capture & Export [ANSI/TXT], and Custom BBS Script Macro Auto-Login)
- KAudio (Added Sound FX Generator Presets, ADSR Envelope & Filter Controls, 16-Step Multi-Track Sequencer, Waveform Visualizer, and WAV/JSON Export & Import)
- KClock (Added Multi-City World Clock Selector, Repeating Alarms Manager with Snooze, Lap Split Stopwatch with Fastest/Slowest badges, Timezone Converter, and Settings Export/Import [JSON])
- KPong (Added 2-Player Local PvP Mode, 5 Game Modes, Power-Up System [Paddle Extend, Speed Boost, Multi-Ball, Shield, Freeze], Leaderboard & Win-Rate Stats Tracker, Theme Switcher, and Save/Resume State)
- KDB (Added CSV/JSON Import/Export engine, robust real-time search and advanced column filtering, and simple password protection/encryption)
- KFont (Added deep TTF/OTF Font Metadata & Metrics inspection, a Unicode Range & Glyph Viewer with hexadecimal character mapping, and a visual Glyph Kerning & Hinting diagnostics panel)
- KMail (Added Multi-Tab Email Sessions, AES Encryption/Password Protection, Advanced Tagging & Filtering, and JSON Import/Export)
- KMandel (Added Multi-threading, Custom Palettes, 4K Export, Julia Sets)
- KPing (Added Packet Logging, Jitter Analytics, Payload Config, Log Export)
- KConnect4 (Added Save/Load States, Match Replay Viewer, Stats Tracker, JSON Export)
- KScript (Added Advanced Regex Search & Replace, Macro Scripting, Deep Memory Inspector, and Step-Through Debugger)
- KMaze (Added Save/Load States, Match Replay Viewer, High Score JSON Export/Import, Custom Keybindings)
- KMine (Added Safe First-Click, Chording, Hint System, Save/Load Game States, High Score JSON Export/Import, Replay Viewer)
- KPac (Added Difficulty Levels, Save/Load Game State, High Score JSON Export/Import, Match Replay Viewer, Custom Keybindings)
- KQuest (Added Save/Load States, High Score JSON Export/Import, Match Replay Viewer, Custom Keybindings)
- KNote (Added Real-Time Search, Note Pinning, TXT Export, JSON/MD Import/Export, AES Encryption, Multi-Tab, Tagging, Auto-save)
- KPass (Added Password Strength & Entropy Assessment and Searchable Vault System, CSV/JSON I/O, AES Crypto, Tagging, Auto-lock)
- KMedia (Added Playback Modes, Speed Controls, Playlist Search, Waveform Visualizer, Filter Suite, Subtitles, Frame Export)
- KNet (Added Traffic Logging, Ping Stats, Port Inspector, Packet Sniffing, Regex Search, DNS/WHOIS, Traceroute, Ifconfig)
- KZip (Added Archive Search, Checksum Verification, Compression, Encryption, Hex Preview, Chunk Vis, Batch Extract, Regex Search, Deep Metadata)
- KPaint (Added Image Filters, Canvas Transforms, Undo/Redo, Brush Shapes, Color Palettes, Layers, Blend Modes, Convolution, Select Tools, Export)
- KSnake (Added Leaderboard, Game Modes, Power-ups, Save States, High Score I/O, Match Replay, Keybindings, Stats Export)
- KTetris (Added Hold Piece, 3-Next Queue, Ghost Landing Shadow, Hard Drop, Leaderboard, Game Modes, Save States, Match Replay, Keybindings, Stats Export, High Score I/O)
- KSpace (Added Game Modes, Weapon Upgrades, Power-ups, Smart Bomb, Boss Waves, Persistent Leaderboard, Save/Resume State, Match Replay Viewer, Custom Keybindings, Match Stats JSON/CSV Export, High Score JSON I/O)
- KType (Added Timed Test Modes, Custom Lesson Import/Export, Real-time WPM/Accuracy Canvas Graphs, Error Heatmap Analysis, Persistent Leaderboard, and PNG/BMP Certificate Export)
- KSolitaire (Added Draw 1/3 Rules, Unlimited Undo/Redo, Smart Hint System, Auto-Finish Solver, Win-Rate & Stats Tracker, Deck & Felt Themes, Save/Resume State, and JSON Export/Import for Stats)
- KTerm (Added Multi-Tab Terminals, Reverse History Search Ctrl+R, Custom Aliases, Environment Variables, Tab Autocomplete, Session Log Export, and Macro Scripting)
- KSynth (Added Preset Save/Load System [JSON import/export & presets menu], Programmable Arpeggiator [Up, Down, Up-Down, Random], Dual Oscillators & Waveforms, Real-Time Oscilloscope / Spectrum Visualizer, ADSR Envelope Controls, 16-Step Pattern Sequencer Export/Import, and Delay/Echo Effect)
- KTask (Added Process Priority Adjuster, CPU/RAM Rolling Sparkline Charts, Process Snapshot Exports [CSV/JSON], Kill Confirmation Modal, System Summary, and Deep Process & Memory Inspector [Threads, Modules, Memory Map, Hex Peek])
- KSys (Added Hardware Component Inspector, Diagnostic Stress & Memory Benchmarks, Event History Log Viewer, Report Export [TXT/JSON/HTML], and Real-Time System Telemetry & Windows Services Manager)
- KRogue (Added Character Classes & Races, Custom Dungeon Seed Generator, Equipment & Inventory Slots, Magic Spellbook, Run History Leaderboard, Save/Resume State, Custom Keybinding Config, and High Score / Run Stats JSON Export & Import)
- KConverter (Added Single & Batch Conversion Modes, Favorite Pairs Pinning, Custom Unit Formula Creator, Searchable Conversion History Log with CSV/JSON Export, Precision/Scientific Notation Controls, and Smart Natural Expression & Dimension Parser)
- KTodo (Added Task Search & Multi-Filter System, Category Tagging, Due Dates & Priorities, Subtask Checklists, JSON/CSV Data Import/Export, Productivity Statistics, Markdown Task List Import/Export, and Kanban Board / Project Category Tabs)
- KGraph (Added Multi-Function Plotting y1-y5, Derivative & Simpson Integral Calculators, Roots & Intersections Finders, Interactive Value Tracing Cursor, Preset Library, CSV/JSON/PNG Export, and Polar r(θ) & Parametric (x(t),y(t)) Plotting Engines with Polar Grids)
- KTimer (Added Multi-Timer Engine [concurrent named timers], Pomodoro Work/Break Cycle Manager with Stats, Lap Split Analysis & CSV/TXT Export, Alarm Presets Library, Sound Alerts & Visual Progress Ring, and Interval / HIIT Circuit Timer Engine [Work/Rest/Prep cycles, Tabata/HIIT/Boxing presets])
