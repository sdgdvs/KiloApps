# KiloApps Games Content Plan

This document tracks the **perpetual, never-ending** improvement loop for all KiloApps games.

## DIRECTOR NOTE (2026-07-29): STOP CAMPAIGN SPAM — FOCUS ON DEPTH WHERE IT MATTERS + GAME BALANCE

**⚠️ READ THIS BEFORE EVERY TURN. This supersedes old loop instructions.**

The old approach of "add 20-stage campaigns and active skills to every game" is OVER. A 20-stage campaign with boss encounters makes sense for KRogue and KSpace. It does NOT make sense for Chess, Connect4, Hangman, or Towers of Hanoi. Those are **classic strategy/puzzle games** — they need smart AI and good difficulty curves, not boss fights and powerups.

**From now on, games are split into two categories with DIFFERENT work priorities:**

### 🎮 DEEP GAMES (get campaigns, content, lore, progression)
These are action, adventure, RPG, and exploration games where depth = more content:
- **KRogue** — roguelike RPG (biomes, enemies, spells, items, floors)
- **KQuest** — fantasy RPG (chapters, bosses, spells, story)
- **KStarship** — space exploration (sectors, subsystems, encounters)
- **KAlchemy** — crafting/discovery (recipes, quests, lab upgrades)
- **KSpace** — arcade shooter (waves, enemy variety, boss fights)
- **KAsteroids** — arcade shooter (sectors, hazards, bosses)
- **KMaze** — exploration/survival (mazes, traps, items, fog of war)
- **KPac** — action (maze variety, ghost AI, power-ups)
- **KBreakout** — arcade (brick layouts, powerups, boss stages)
- **KSnake** — action (obstacle mazes, rival AI, boss encounters)

For Deep Games: Add more content, enemy variety, procedural generation, narrative elements, crafting, lore. These games SHOULD have long campaigns and progression systems. Fill the 999KB budget.

### ♟️ CLASSIC GAMES (get balance, usability, AI tuning — NO MORE CAMPAIGNS)
These are traditional board, card, and puzzle games where quality = smart AI and good UX:
- **KChess, KGo, KReversi, KConnect4** — board games
- **KSolitaire, KFreecell** — card games
- **KSudoku, K2048, KTowers, KMines** — puzzle games
- **KHangman, KWords, KSimon, KMatch3** — word/pattern games
- **KTetris, KPong, KDarts** — skill games

For Classic Games: **DO NOT add more campaign stages, boss encounters, or active skill hotkeys.** They already have too many. Instead focus on:
1. **Game Balance** — Is Easy too easy? Is Hard actually hard? Does the AI play well? Does difficulty ramp smoothly?
2. **AI Quality** — For board games (Chess, Go, Reversi, Connect4): make the AI smarter and more varied, not just add more stages.
3. **Usability** — Are controls intuitive? Is the UI clear? Can a new player figure out how to play? Is there a help/tutorial?
4. **Bug Fixes** — Fix broken mechanics, impossible levels, crashes, or stuck states.
5. **Polish** — Score display, win/loss feedback, smooth animations, sound effects.

### 🔧 GAME BALANCE PASS (NEW PRIORITY)
**Before adding ANY new content to a game, play-test it mentally first:**
- Can a player actually win Stage 1 on their first try? If not, it's too hard.
- Is Easy mode trivially easy (AI makes random moves)? If so, fix it.
- Does the difficulty curve feel smooth? Or does it jump from trivial to impossible?
- Are powerups so strong they trivialize the game? Or so weak they're useless?
- For board game AI: does Easy AI feel fair but beatable? Does Hard AI feel challenging but not cheating?
- Is the campaign so long that nobody will finish it? 10 stages is plenty for most games. 20+ is only for Deep Games.

**When you pick a Classic Game from the queue, your job is a BALANCE & USABILITY pass, not more content.**

### 🛑 MATURITY & TURN SKIPPING DIRECTIVE (2026-09-13 — CRITICAL)
- Unless you have a good directive from the director or user to add something, **"turn skipped because this app is complete and we don't have new ideas here"** is fine for apps that have already been through 6+ passes.
- The director can add new ideas or directions later and you can implement those on the next turn, but you should NOT just add random poorly-thought-out features, mechanics, or graphics.
- If a game has had 6+ passes and is balanced, complete, and solid, skip the turn, log the skip concisely, rotate it to the bottom of the queue, and finish cleanly.

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

## Agent Rules & Guidelines

- **Token Conservation & Logging Rules (CRITICAL):**
  - Run log entries: ≤8 lines of terse bullet points. No paragraphs.
  - Skip-turn entries: exactly 1 line: ⏭️ Skip — [reason in ≤15 words].
  - Never restate implementation details that exist in code. Log WHAT changed + results, not HOW.
  - Never list parameter names, field names, or variable values unless reporting failure.
  - Completed work needs no elaboration: ✅ Done (N/N tests pass) is sufficient.
  - Surgical edits only. Touch only specific cells/lines that changed. Table cell notes ≤100 chars.
  - Only read files relevant to current task. Move historical logs older than ~80 lines to rchive/.

**Perpetual Loop (NEVER STOP)**
This agent loops forever. Pick the top game from the queue, do work appropriate to its category (see above), move it to the bottom, repeat.
- For **Deep Games**: Add meaningful new content — new enemies, mechanics, story, levels, items.
- For **Classic Games**: Do a balance/usability audit — test difficulty, improve AI, fix UX issues, tune parameters.
- **CREATE NEW GAME** mode: Only if the Director or user explicitly requests it. The Creator agent is the primary new-game creator.

**Dual-Target Strategy (CRITICAL)**
Each game exists in two forms: a native Windows executable (`K[Name]/main.c`) and a web HTML5 version (`KiloOS/public/apps/k[name].html`). ALWAYS audit both versions before working on a game.
- **Tier 1 (Full Parity)**: Both web and exe have equivalent gameplay. Improve both in parallel.
- **Tier 2 (Simplified Web Version)**: Web version is simplified but playable. Build the fullest web version possible. The exe version should always be equal or superior.
- **Tier 3 (Exe-Only with Web Stub)**: The game is too complex to port perfectly right now. Focus content improvement on the EXE version. Web is a polished download stub. Over time, incrementally port features to move from Tier 3 → Tier 2 → Tier 1.

**Important Rules**
- ALWAYS read the exe source FIRST. It is the "canonical" version and may be far more advanced than the web version.
- NEVER remove features from the exe to match a simpler web version.
- NEVER replace a working advanced exe with a simpler reimplementation.
- When improving the exe, preserve ALL existing gameplay systems and add to them.
- If you encounter a download-stub web version, it's intentional. Improve the exe, and optionally begin an incremental web port if time allows.

**Content Additions (for Deep Games only — pick 2-3 per turn)**
- New enemies, bosses, items, spells, or story elements
- Procedural generation improvements (more room types, varied layouts)
- Lore, narrative, crafting recipes, progression unlocks
- New biomes, environments, or themes
- Sound effects (Web Audio API for web; Beep()/PlaySound for native)

**Balance & Usability Pass (for Classic Games — do ALL of these)**
- Test each difficulty level: is Easy actually easy? Is Hard actually hard?
- Audit AI quality: does the AI make reasonable moves at each difficulty?
- Check first-play experience: can a new player figure out controls?
- Verify win/loss conditions work correctly
- Ensure score display, high scores, and restart work properly
- Check that powerups (if any) are balanced — not too strong, not useless

**Quality Bar**
- Must have: start screen, score display, game-over screen with restart
- Must have at least 2 difficulty levels or progressive difficulty
- Persist high scores
- Responsive controls
- Web games: use `requestAnimationFrame`
- Exe games: preserve Win32 message loop and rendering approach

**Multi-Agent Coordination**

6 worker agents + 2 directors operate on this repo on overlapping schedules. You are the **Game Content Expander** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`game_content_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Feature Expander agent), `app_fix_plan.md` (QA agent), `new_app_plan.md` (Creator agent), `usability_plan.md`
- **Shared file `KiloOS/src/App.jsx`** — shared ownership. You may ONLY add entries to the APPS array (to register new games). Protocol: `git pull` → add APPS entry only → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — Do NOT edit.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.

**General Constraints**
- No KiloApp may exceed 999KB (web or native).
- All game HTML files must be SINGLE self-contained files (inline CSS + JS).
- Do NOT edit: `master_plan.md`, `architecture.md`, `.agents/AGENTS.md`, `KiloOS/src/index.css`.
- Do NOT edit other agents' plan files: `app_work_plan.md`, `app_fix_plan.md`, `usability_plan.md`.
- Your focus is CONTENT, GAMEPLAY DEPTH, and GAME LENGTH — not visual polish. Add more levels, more enemies, more modes, more mechanics. Fill the 999KB budget with gameplay content.
- Do NOT add ARG/easter egg elements.
- **CLEANUP:** Before committing, delete any temporary `patch_*.py` or `patch_*.js` scripts in the repo root. Subagents must not leave scratch files behind.
- **Logging discipline:** Keep this plan file concise. Brief notes per turn in the Progress Log. Do NOT dump file contents or create verbose logs.

---
## Game Inventory & Parity Audit

### 🎮 Deep Games (Content Expansion Queue — round-robin, pick top, do content work, move to bottom)
| Game       | Parity | Status | Next Work |
|------------|--------|--------|-----------|
| KSnake     | Tier 1 | Loop 10 done (Map Editor lab, Boss Gauntlet with 4 serpents, 30-stage branching campaigns) | Loop 11: Co-op 2-player local mode, elemental snake skins, speedrun timer splits |
| KRogue     | Tier 1 | Loop 11 done (9 Class Loadout Archetypes, Class Passives & Active Abilities) | Loop 12: Enchanting altar socketing, companion pets, branching secret challenge vaults |
| KQuest     | Tier 1 | Loop 10 done (Kingdom management, army battles, castle defense) | Loop 11: Guild alliances, naval expeditions, realm artifacts |
| KStarship  | Tier 1 | Loop 10 done (Galactic super-weapons, faction wars, alien boarding parties) | Loop 11: Capital ship dreadnought sieges, wormhole gate networks, cybernetic crew augmentations |
| KFortress  | Tier 1 | Loop 2 done (4 Elemental Tower Fusions [Inferno, Superconductor, Venomspite, Solar Prism Beam], Castle Siege Trebuchet strike, automated wall ballistas, 3 new Academy techs [Siege Eng, Fusion Mastery, Fort Traps], and 5 Challenge Mutators) | Loop 3: Rune socketing system, siege boss war engines, multi-lane branching maps |
| KAlchemy   | Tier 1 | Loop 3 done (Ancient Alchemical Guild expeditions, planetary transmutations, elemental familiar summons) | Loop 4: Master Alchemist Guild tournaments, astrological eclipse alignments, philosopher homunculus creation |
| KColony    | Tier 1 | Loop 3 done (Colony trade freighters with periodic docking & active trade contracts, underground cavern networks with Cavern Drill magma tapping & deep cavern expeditions, orbital strike beacons with automated meteor interception & active tactical orbital bombardment) | Loop 4: Terraform atmospheric processors, deep crust geothermal reactors, interstellar warp beacon network |
| KSpace     | Tier 1 | Loop 11 done (Capital ship dreadnought sieges, drone companion wings, hyper-jump mechanics) | Loop 12: Super-laser orbital strike arrays, stealth cloaking generator modules, nebula storm anomaly sectors |
| KAsteroids | Tier 1 | Loop 11 done (Drone wingmen companions, orbital defense platforms, EMP shockwave mines) | Loop 12: Super-laser orbital strike arrays, stealth cloaking generator modules, nebula storm anomaly sectors |
| KMaze      | Tier 1 | Loop 11 done (Secret illusionary walls, dungeon lore tablets, stealth crouch mechanics) | Loop 12: Alchemy potion brewing, locked iron gate mechanisms, ancient treasure vaults |
| KPac       | Tier 1 | Loop 11 done (Ghost companion pet summoning, labyrinth hazard portals, legendary relic forging) | Loop 12: Secret warp chambers, cosmic frenzy power mode, spectral treasure goblins |
| KBreakout  | Tier 1 | Loop 11 done (Orbital satellite barriers, laser reflectors, quantum resonance bricks) | Loop 12: Warp vortex gravity conduits, hyperspace brick phasing, antimatter cascade balls |

### ♟️ Classic Games (Balance & Usability Queue — round-robin, pick top, do balance audit, move to bottom)
| Game       | Parity | Status | Next Work |
|------------|--------|--------|-----------|
| KFreecell  | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Checked that all dealt hands are solvable, rebalanced campaign difficulty constraints. |
| KSudoku    | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Audit puzzle generation quality. Ensure Easy puzzles have unique solutions and Hard is solvable. |
| K2048      | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Check tile spawn rates, verify move-limit stages are achievable, audit powerup balance. |
| KMines     | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Check first-click safety, mine density per difficulty, ensure Sonar powerup isn't overpowered. |
| KTowers    | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Verified optimal move counts (BFS solver) and checked locked-disk puzzles. Fixed unlock bug in main.c. |
| KTetris    | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Adjusted Pentomino piece frequency in late stages and smoothed the drop speed curve. |
| KPong      | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Adjusted AI paddle speed per difficulty, nerfed boss, fixed powerup spawn rates. |
| KHangman   | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Audited word difficulty, removed multi-word phrases, smoothed strike limits for late campaign. |
| KWords     | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Fixed word placement silent failures, buffed Fog of War radius from 3x3 to 5x5, fixed UI bugs. |
| KSimon     | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Reset timer on correct inputs, smoothed speed curves, fixed 6-button label bugs. |
| KMatch3    | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Auto-shuffle on no moves, nerfed Boss HP, reduced powerup costs. |
| KDarts     | Tier 1 | Loop 8 done — BALANCE PASS COMPLETE | Tuned wind/wobble by difficulty, fixed 301 bust bug, fixed Killer logic. |
| KChess     | Tier 1 | Loop 9 done — BALANCE PASS COMPLETE | Tuned Easy/Hard/Master Minimax depths and opening/endgame PST. |
| KGo        | Tier 1 | Loop 9 done — BALANCE PASS COMPLETE | Fixed AI turn skipping, handicap AI trigger, double-pass scoring, and tuned Grandmaster positional weights. |
| KReversi   | Tier 1 | Loop 9 done — BALANCE PASS COMPLETE | Fixed AI softlock on pass, increased Grandmaster Minimax depths, verified difficulty curve. |
| KConnect4  | Tier 1 | Loop 9 done — BALANCE PASS COMPLETE | Upgraded Grandmaster heuristic evaluation for stronger defensive and aggressive plays, verified blocker drop mechanics. |
| KSolitaire | Tier 1 | Loop 9 done — BALANCE PASS COMPLETE | Guaranteed opening deal fairness, fixed foundation-to-tableau Vegas scoring, added bankroll persistence on abandon, and added right-click instant foundation play. |


## Progress Log
## Progress Log

> Archived: Loops 1-9 in [archive/game_content_history.md](archive/game_content_history.md).

**Loop 11**
- [x] KBreakout: Orbital satellite barrier, laser reflector prism bricks, quantum resonance bricks.
- [x] KPac: Ghost companion pet system (4 familiars), labyrinth portals, mythic relic forging.
- [x] KMaze: Illusionary walls, ancient lore tablets, stealth crouch mechanics.
- [x] KAsteroids: Drone wingmen, orbital defense platforms, EMP shockwave mines.
- [x] KSpace: Dreadnought boss sieges, drone companion wings, hyper-jump mechanics.
- [x] KColony (L3): Trade freighters, cavern networks, orbital strike beacons.
- [x] KAlchemy (L3): Guild expeditions, transmutation matrix, elemental familiars.
- [x] KFortress (L2): Elemental tower fusions, castle siege defenses, challenge mutators.
- [x] KRogue: 9 class loadout archetypes with passive perks and active abilities.

**Loop 10**
- [x] KStarship: Super-weapons, faction wars, alien boarding parties.
- [x] KQuest: Kingdom management, royal army warfare, castle gate defense.
- [x] KSnake: Map editor, boss gauntlet, 30-stage branching campaign.
- [x] KBreakout: Multi-ball chaos, gravity wells, cyber-forge power lab.
- [x] KPac: Arcade endless mode, procedural ghost traits, item crafting.
- [x] KMaze: 45-level 5-biome campaign, save shrines, dynamic lighting.
- [x] KAsteroids: Magma titan bosses, warp overdrive, zero-G anomalies.
- [x] KSpace: Elite enemy squads, planetary bombardment, weapon overcharge.
- [x] KColony (L2): Advanced structures, tech tree, biomes with weather.
- [x] KAlchemy (L2): Mythic elements, daily trials, prestige system.
- [x] KFortress (L1): 3 new towers, dynamite trap, 4 new enemies, 2 new maps.
- [x] KSolitaire: Deal fairness validator, Vegas fix, right-click foundation.
- [x] KRogue: True Sanctuary biome, tier 8 ultra bosses, floor 50 finale.
