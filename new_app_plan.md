# New App Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 4 worker agents + 1 director operate on this repo on overlapping schedules. You are the **App Creator & Deep Expander**.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`new_app_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Feature Expander agent), `app_fix_plan.md` (QA agent), `game_content_plan.md` (Games agent), `kiloos_ux_plan.md` (inactive)
- **Shared file `KiloOS/src/App.jsx`** — shared ownership. You may ONLY add entries to the APPS array. Protocol: `git pull` → add APPS entry only → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — Do NOT edit.
- **App folder categories** for the `folder` property in APPS: `System`, `Media`, `Office`, `Games`, `Network`, `Dev`.
- **Dual-target model:** Each app has a native C version (`K[Name]/main.c` + `build.bat`) and a web HTML5 version (`KiloOS/public/apps/k[name].html`). Both versions should offer functional parity where feasible. Web HTML files must be single self-contained files (inline CSS + JS, no imports).
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native). This is a LOT of room for optimized code — use it to build deep, feature-rich apps.
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **Logging discipline:** Keep this plan file concise. Brief notes per completed phase. Do NOT dump file contents.

## DIRECTOR NOTE (2026-07-22): DEEP FANTASY & SCI-FI GAMES

**Current: 21+ games. We have enough board/puzzle games.** After KReversi completes, switch to creating **deep, clever, creative fantasy and science fiction games** — NOT more board games or simple puzzles. Think roguelikes, RPGs, survival, space exploration, dungeon crawlers, text adventures with real depth. These should feel like miniature worlds players can get lost in. Use procedural generation, inventory systems, crafting, storylines, lore, enemy variety, and progression systems to fill the 999KB budget. Each game should be meaningfully different from KRogue (our existing roguelike).

**NO MORE:** Board games (chess, checkers, go, connect4), pattern games (simon, match3), simple arcade clones. We have plenty.
**YES MORE:** Games with emergent gameplay, narrative, exploration, strategy, resource management, survival.

When creating games, think about what makes them **deeply replayable**:
- Multiple game modes (classic, timed, endless, puzzle)
- Progressive difficulty that adapts to skill
- High score tables with multiple categories
- Sound effects and visual feedback (particles, screen shake, combos)
- Unlockable content or achievements
- Multiple levels, maps, or boards
- AI opponents with adjustable difficulty
- Multiplayer (local 2-player where appropriate)
- Tutorial/how-to-play screen
- Statistics tracking (games played, win rate, streaks)

## Deduplication Rules (CRITICAL — READ BEFORE EVERY NEW APP)

Before creating any new app, check for overlap with the existing suite. The following categories are SATURATED — do NOT create more apps in these niches:
- **Text/Code Editors:** KPad, KNote, KScript — no more text editors or notepads.
- **Drawing/Art:** KPaint, KImage — no more paint or drawing apps.
- **Calculators/Converters:** KCalc, KConverter, KBase — no more calculator or unit converter variants.
- **Time/Clocks:** KClock, KTimer, KCalendar — no more clock or timer variants.
- **Chat/Messaging:** KChat, KChatServer, KBBS, KMail — no more chat or email apps.
- **Existing Games:** K2048, KAsteroids, KBreakout, KChess, KConnect4, KFreecell, KGo, KHangman, KMatch3, KMaze, KMines, KPac, KPong, KRogue, KSimon, KSnake, KSolitaire, KSpace, KSudoku, KTetris, KWords — do NOT recreate these genres.

When choosing a new app, ask: "Does this do something fundamentally different from every existing app?" If the answer is no, pick something else.

---

## Current App

**App:** KQuest
**Phase:** 2
**Status:** In Progress

- [x] Phase 1: Choose a unique app. Scaffold `KQuest/` directory. Create web HTML file with basic UI skeleton. Register in App.jsx.
- [x] Phase 2: Implement core functionality in the web HTML (inline JS/CSS, self-contained).
- [ ] Phase 3: Create native C version (`main.c`, `build.bat`) with Win32 API. Aim for functional parity with web.
- [ ] Phase 4: Apply a clean dark theme to both versions. Do NOT spend time on elaborate visual polish — keep it functional and move on to features.

## Next Priority (Before New Apps)

**App:** KQuest
**Goal:** Phase 3



## App Lifecycle (14 phases per app)

### Creation (Phases 1-4)
- **Phase 1:** Choose a unique app. Scaffold `K[Name]/` directory. Create web HTML file with basic UI skeleton. Register in App.jsx.
- **Phase 2:** Implement core functionality in the web HTML (inline JS/CSS, self-contained).
- **Phase 3:** Create native C version (`main.c`, `build.bat`) with Win32 API. Aim for functional parity with web.
- **Phase 4:** Apply a clean dark theme to both versions. Do NOT spend time on elaborate visual polish — keep it functional and move on to features.

### Deep Expansion (Phases 5-14)
Each expansion phase adds ONE substantial feature to BOTH web and native versions. Target near-commercial quality.

**For Games, prioritize these expansion features:**
- Additional game modes (timed, endless, puzzle, campaign)
- AI opponent improvements (smarter, adjustable difficulty)
- Sound effects (Web Audio API for web, Beep()/PlaySound for native)
- Visual polish (particle effects, animations, screen shake, combo counters)
- High score system with multiple categories
- Level/map generation or progression
- Power-ups, special abilities, or items
- Statistics tracking (games played, win rate, best streaks)
- Tutorial / how-to-play overlay
- Local 2-player mode where appropriate

**For Utilities, prioritize these expansion features:**
- Data persistence (localStorage for web, file I/O for native)
- Import/export functionality
- Search and filtering
- Undo/redo
- Keyboard shortcuts / accessibility
- Settings panel / user preferences
- Multiple views or modes
- Help/tutorial system

### Completion (NEVER STOP — immediately start the next app)
After Phase 14, mark the app as COMPLETE and move it to the Completed list. **Immediately pick the next app from the Possible Future Apps list and start Phase 1.** This agent never idles. If the suggestion list is exhausted, invent new unique apps. The cycle is: create → expand deeply → complete → create next → repeat forever.

## Completed Apps
- KDarts (Phase 14 completed: Added Comprehensive Help / How-to-Play modal to both versions)
- KGo (Phase 14 completed: Added Comprehensive Help / How-to-Play modal to both versions)
- KWords (Phase 14 completed: Added Comprehensive Help / How-to-Play modal to both versions)
- KMatch3 (Phase 14 completed: Added Comprehensive Help / How-to-Play modal to both versions)
- KAsteroids (Phase 14 completed: Added Comprehensive Help / How-to-Play modal to both versions)
- KSimon (Phase 14 completed: Added Comprehensive Help / How-to-Play modal to both versions)
- KHangman (Phase 14 completed: Added Help / How-to-Play modal to both versions)
- KConnect4 (Phase 14 completed: Added Comprehensive Help / How-to-Play modal to both versions)
- K2048 (Phase 14 completed: Added Comprehensive Help / Manual System to both versions)
- KBBS (Phase 14 completed: Added Help/Quick Reference System to both versions)
- KFlash (Phase 14 completed: Added Help/Tutorial modal to both versions)
- KVault (Phase 14 completed: Added Help/Tutorial modal to both versions)
- KBudget (Phase 14 completed: Added Print Report support to both versions)
- KHabit (Phase 14 completed: Added Detailed Statistics modal to both versions)
- KSudoku (Phase 14 completed: Added Auto-fill Notes / Magic Pencil feature to both versions)
- KFreecell (Phase 14 completed: Added Comprehensive Help / How-to-Play modal to both versions)
- KTowers (Phase 14 completed: Added Comprehensive Help / How-to-Play modal to both versions)
- KReversi (Phase 14 completed: Added Comprehensive Help / How-to-Play modal & Strategy Guide to both versions)

## Possible Future Apps (pick from here or invent your own)

### 🎮 GAMES — DEEP FANTASY & SCI-FI (PRIORITY)
- **KQuest** — Fantasy text RPG with procedural dungeons. Character creation (warrior/mage/rogue). Inventory, equipment, spells. Turn-based combat with enemy AI. Town hub with shop, inn, quest board. Multiple dungeon biomes. Boss fights. Save/load. 20+ enemy types. Loot tables. XP and leveling.
- **KStarship** — Sci-fi exploration. Navigate a star map, discover planets, manage fuel/hull/crew. Random encounters (pirates, anomalies, traders). Ship upgrades. Crew management. Text-based with ASCII star maps. Permadeath mode.
- **KAlchemy** — Fantasy crafting/discovery game. Combine elements to discover new ones. 100+ discoverable recipes. Procedural hints. Encyclopedia tracking. Multiple tiers of complexity (earth+fire=lava, lava+water=obsidian). Score by discoveries.
- **KFortress** — Tower defense with fantasy theme. Place towers on a grid path. Multiple tower types (archer, mage, cannon). Enemy waves with varying resistances. Upgrade system. Gold economy. 15+ levels with different maps. Boss waves.
- **KColony** — Sci-fi colony survival. Manage resources (food, power, materials) on an alien planet. Build structures. Research tech tree. Random events (storms, alien encounters, equipment failures). Population management. Day/night cycle.
- **KWizard** — Fantasy dueling card game. Build a hand from a spell deck. Turn-based combat vs AI. Mana system. 30+ unique spells (fireball, shield, heal, summon). AI opponents with different strategies. Campaign with progressive difficulty.
- **KVoid** — Sci-fi horror survival. Explore a derelict space station room by room. Limited oxygen and battery. Find keycards, solve puzzles, avoid or fight creatures. Procedural station layout. Multiple endings based on choices. Tension through resource scarcity.
- **KDragon** — Fantasy creature-raising sim. Hatch, feed, train, and evolve a dragon. Multiple dragon types with different abilities. Arena battles vs AI dragons. Stat management (strength, speed, fire, loyalty). Aging system. Treasure hoarding mechanic.

### 🛠️ Utilities (lower priority — create 1 for every 2-3 games)
- **KPomodoro** — Focus timer with work/break cycles, session history, daily stats.
- **KBookmark** — Bookmark manager with folders, tags, search.
- **KHash** — File/text hasher (MD5, SHA-1, SHA-256, CRC32).
- **KRSS** — RSS feed reader with folder organization.
- **KClip** — Clipboard history manager with pinning and search.
- **KRecipe** — Recipe organizer with ingredients, steps, categories.
- **KDiff** — Text diff/compare tool with side-by-side view.
