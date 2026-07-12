# New App Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 5 agents operate on this repo on overlapping schedules. You are the **App Creator & Deep Expander**.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`new_app_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Builder agent), `app_fix_plan.md` (QA agent), `kiloos_ux_plan.md` (UX agent), `game_content_plan.md` (Games agent)
- **Shared file `KiloOS/src/App.jsx`** — owned by the UX agent. You may ONLY add entries to the APPS array. Protocol: `git pull` → add APPS entry only → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — owned by the UX agent. Do NOT edit.
- **App folder categories** for the `folder` property in APPS: `System`, `Media`, `Office`, `Games`, `Network`, `Dev`.
- **Dual-target model:** Each app has a native C version (`K[Name]/main.c` + `build.bat`) and a web HTML5 version (`KiloOS/public/apps/k[name].html`). Both versions should offer functional parity where feasible. Web HTML files must be single self-contained files (inline CSS + JS, no imports).
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native). This is a LOT of room for optimized code — use it to build deep, feature-rich apps.
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **Logging discipline:** Keep this plan file concise. Brief notes per completed phase. Do NOT dump file contents.

## Deduplication Rules (CRITICAL — READ BEFORE EVERY NEW APP)

Before creating any new app, check for overlap with the existing suite. The following categories are SATURATED — do NOT create more apps in these niches:
- **Text/Code Editors:** KWrite, KPad, KNote, KScript — no more text editors or notepads.
- **Drawing/Art:** KDraw, KPaint, KImage — no more paint or drawing apps.
- **Calculators/Converters:** KCalc, KConverter, KBase — no more calculator or unit converter variants.
- **Time/Clocks:** KClock, KTimer, KCalendar — no more clock or timer variants.
- **Chat/Messaging:** KChat, KChatServer, KBBS, KMail — no more chat or email apps.

When choosing a new app, ask: "Does this do something fundamentally different from every existing app?" If the answer is no, pick something else.

---

## Current App

**App:** KVault
**Phase:** 4
**Status:** In Progress (Phase 3 native C Win32 core functionality completed)

## App Lifecycle (14 phases per app)

### Creation (Phases 1-4)
- **Phase 1:** Choose a unique app. Scaffold `K[Name]/` directory. Create web HTML file with basic UI skeleton. Register in App.jsx.
- **Phase 2:** Implement core functionality in the web HTML (inline JS/CSS, self-contained).
- **Phase 3:** Create native C version (`main.c`, `build.bat`) with Win32 API. Aim for functional parity with web.
- **Phase 4:** Polish both versions: dark-mode glassmorphic aesthetic for web, dark theme for native.

### Deep Expansion (Phases 5-14)
Each expansion phase adds ONE substantial feature to BOTH web and native versions. Target near-commercial quality. Examples of expansion features:
- Data persistence (localStorage for web, file I/O for native)
- Import/export functionality
- Search and filtering
- Undo/redo
- Keyboard shortcuts / accessibility
- Settings panel / user preferences
- Multiple views or modes
- Print or share functionality
- Drag-and-drop interactions
- Data visualization or charts
- Advanced algorithms or processing
- Multi-item management (tabs, lists, collections)
- Responsive layout improvements
- Help/tutorial system
- Performance optimization for large datasets

### Completion
After Phase 14, mark the app as COMPLETE and move it to the Completed list. Pick a new app and start over.

## Completed Apps
(none yet)

## Possible Future Apps (pick from here or invent your own)

**Utilities:** KPomodoro (focus timer with work/break cycles), KBudget (expense tracker with categories and charts), KFlash (flashcard study app with spaced repetition), KBookmark (bookmark manager with folders and tags), KHash (file/text hasher — MD5/SHA/CRC), KRSS (RSS feed reader), KClip (clipboard history manager), KRecipe (recipe organizer), KHabit (habit tracker with streaks), KFit (workout log), KVault (encrypted text storage), KDiff (text diff/compare tool)

**Games:** K2048 (sliding tile puzzle), KSudoku (logic puzzle generator/solver), KBreakout (brick breaker), KConnect4 (connect four), KHangman (word guessing), KSimon (memory pattern game), KAsteroids (space shooter), KFreecell (card solitaire variant), KMatch3 (gem matching), KWords (word search), KGo (board game)
