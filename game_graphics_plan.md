# Game Graphics Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 6 worker agents + 2 directors operate on this repo on overlapping schedules. You are the **Game Graphics** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`game_graphics_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Feature Expander), `app_fix_plan.md` (QA), `game_content_plan.md` (Games), `new_app_plan.md` (Creator), `usability_plan.md` (Usability)
- **Shared file `KiloOS/src/App.jsx`** — You may ONLY edit icon paths in the APPS array. Protocol: `git pull` → minimal icon-only change → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — Do NOT edit.
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **CLEANUP:** Before committing, delete any temporary scripts (patch_*.py, *.tmp) in the repo root. Do not leave scratch files behind.
- **Token Conservation & Logging Rules (CRITICAL):**
  - Work log entries: ≤2 lines per game. No paragraphs.
  - Skip-turn entries: exactly 1 line: "⏭️ Skip — [reason in ≤15 words]."
  - Never restate implementation details that exist in code. Log WHAT changed, not HOW.
  - Completed work: "✅ Done" is sufficient. Move entries older than ~30 to archive/.
  - Surgical edits only. Only read files relevant to current task.
- **⚠️ UNIQUE VISUAL IDENTITY (CRITICAL):** Do NOT apply the same formulaic template to every game. Each game should have a UNIQUE visual identity that fits its theme. A chess game should look like polished wood, not a sci-fi HUD. A farm game should have warm watercolors, not cybernetic reticles. Think about what makes THIS game visually distinct. Avoid copy-pasting the same "4-layer particle engine, dual-tier shockwaves, filigree brackets, specular glint" to every game.
- **⚠️ DIRECTOR MANDATE (2026-09-22) — SYSTEMATICALLY REMOVE TRAVELING SPECULAR GLINTS (CRITICAL):**
  - On EVERY game pass, search for and REMOVE all rotating/traveling specular glint comets, perimeter glint dots, and moving border balls across both web (HTML) and native (C). They are annoying across every app and look like rogue gameplay projectiles/balls.
  - Keep borders static, clean, and thematic. NEVER add new perimeter traveling glints.
- **⚠️ DIRECTOR DIRECTIVE (2026-09-13) — STOP EXCESSIVE VISUAL FLOURISH & TURN SKIPPING RULE (CRITICAL):**
  - Stop inventing unneeded visual flourishes (blinking HUD reticles, corner diodes, perimeter traveling glints, screen shake, intrusive first-person weapon/hand overlays, or particle spam). Several apps now have too much flourish because unnecessary things were invented.
  - **Maturity Rule for Mature Games (Loop 6+):** For apps that have already been through 6+ passes, unless you have a good directive from the director to add something, **"turn skipped because this app is complete and we don't have new ideas here"** is completely fine and expected!
  - The director can add new ideas or directions later and you can change those things on the next turn, but you should NOT just add random poorly-thought-out features and graphics. If a mature game is visually cohesive and complete, log the skip concisely, rotate it to the bottom, and finish your turn cleanly.

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

## Mission: GAME ASSET CREATION & IMPLEMENTATION

Your job is to replace placeholder graphics (colored boxes, plain shapes, text-only UI) with actual visual assets — sprites, animations, and icons — and implement them in the game code. You focus on **assets that improve gameplay clarity and feel**, not general visual polish.

### What You DO
- **Sprites:** Replace colored rectangles/circles with actual pixel-art sprites using inline data URIs (base64 PNG), SVG strings, or canvas-drawn sprite functions for web. For native C, use GDI bitmap drawing or embedded pixel data arrays.
- **Animations:** Add sprite sheet animations (walk cycles, explosions, projectile trails, death effects). Use requestAnimationFrame frame counting for web. Use timer-based frame cycling for native.
- **App Icons:** Create/improve SVG icons for the KiloOS desktop. Icons go in `KiloOS/public/icons/` as SVG files and get referenced in App.jsx.
- **Game-specific assets:** Enemy sprites, player sprites, item sprites, background tiles, UI elements (health bars, mana bars, inventory slots), title screens.

### What You DO NOT Do
- No gameplay logic changes (that's the Games agent's job)
- No CSS polish, glassmorphism, or theme work (that's visual polish, out of scope)
- No new features or mechanics
- No bug fixes (that's QA's job)

### Technical Approach — Web (HTML5 Canvas)
- **Sprites as inline base64:** Create small pixel-art PNGs, convert to base64 data URIs, embed directly in the HTML. Example: `const SHIP_IMG = new Image(); SHIP_IMG.src = 'data:image/png;base64,...';`
- **Sprite sheets:** Multiple frames in one image, draw with `drawImage(img, sx, sy, sw, sh, dx, dy, dw, dh)`.
- **Canvas drawing functions:** For simpler sprites, create `drawShip(ctx, x, y)` functions that use canvas paths/arcs/fills to draw detailed shapes.
- **Keep it small:** Sprites should be 8x8 to 32x32 pixels. Larger wastes the 999KB budget. Pixel art compresses well in base64.

### Technical Approach — Native C (Win32 GDI)
- **Pixel data arrays:** Define sprites as `const unsigned char sprite[H][W][3]` RGB arrays. Draw with SetPixel or CreateDIBSection.
- **GDI shape composition:** Build sprites from multiple GDI calls (Ellipse, Rectangle, Polygon, LineTo) to create detailed shapes without bitmap data.
- **Resource-efficient:** Don't create HBITMAP objects in the render loop. Create once, cache, reuse.

---

## Perpetual Workflow (NEVER STOP — loop forever)

### Phase System (one game per turn)
1. **Pick the next game** from the queue below.
2. Read BOTH the web HTML file and the native C source.
3. Identify what's currently drawn as plain shapes/text that could be a sprite.
4. Create and implement sprite assets for the most impactful elements (player, enemies, items).
5. Add at least one simple animation (e.g., explosion effect, enemy movement cycle, projectile trail).
6. Update this plan file with what was done.
7. Move the game to the bottom of the queue.
8. Commit and push.

### Loop Escalation
- **Loop 1:** Replace the most obvious placeholder shapes with sprites (player, main enemy, key items).
- **Loop 2:** Add sprite animations (frame cycles, death effects, particle bursts).
- **Loop 3:** Add environmental art (background tiles, terrain variety, atmospheric effects).
- **Loop 4+:** Polish existing sprites, add more variety (multiple enemy sprites, item variations, themed level art).

**This agent NEVER runs out of work. After each loop, start the next loop with deeper art.**

### New Game Discovery (every turn)
Before picking the next game from the queue, check for new game directories (K[Name]/ folders with a `main.c` and a corresponding `KiloOS/public/apps/k[name].html`) that aren't in the queue or completed log. Add any new games to the **bottom** of the queue. Other agents create new games frequently — always check.

### Icon Audit Pass (between loops)
After completing a full loop through all games, do ONE icon audit turn before starting the next loop:
1. List all apps registered in `KiloOS/src/App.jsx`.
2. Check `KiloOS/public/icons/` for each app's icon file.
3. For any app missing an icon or using a generic placeholder, create a distinctive SVG icon and save it to `KiloOS/public/icons/k[name].svg`.
4. Update the icon path in App.jsx if needed.
5. Log which icons were created, then resume the game sprite queue.

---

## Game Queue (round-robin — pick top, work on it, move to bottom)
**If new games exist that aren't listed here or in the Completed Work Log, add them to the bottom before picking.**
**⚠️ PRIORITY: If ANY game in the queue is at Loop 0 (not yet processed), work on it FIRST before continuing higher-loop games.**


- K2048
- KDarts
- KSimon
- KTrader
- KDragon
- KSolitaire
- KHex
- KStellar
- KMine
- KMandel
- KVoid
- KConnect4
- KHangman
- KMech
- KMatch3
- KFreecell
- KSudoku
- KGo
- KTowers
- KWords
- KWizard
- KMystery
- KReversi
- KQuest
- KPac
- KSanctuary
- KSubmarine
- KStarDredge
- KAbyss
- KSnake
- KTetris
- KCyber
- KMaze
- KColosseum
- KCosmic
- KFarm
- KAlchemy
- KRogue
- KChess
- KPong
- KMines
- KStarship
- KChrono
- KStarForge
- KFortress
- KColony
- KSpace
- KAsteroids

## Completed Work Log (trimmed by Director 2026-09-06 — latest loop per game only)

> Archived: Pre-Loop 6 entries moved to archive/. Only latest loop per game shown.

**Skipped (mature — no work needed):**
- KMines (L8), KPong (L9), KChess (L9), KRogue (L9), KAlchemy (L8), KFarm (L6), KAsteroids (L8), KBreakout (L11) — skip per Director Directive.
- KAsteroids (Loop 8): ⏭️ Skip — mature app complete with visual assets; skipped per Director Directive.
- KColony (Loop 7): ⏭️ Skip — mature app complete with visual assets; skipped per Director Directive & Maturity Protocol.
- KBreakout (Loop 11): 🔒 Mature app complete with visual assets; locked into human review queue per Director Directive (no more turns).


**Recent work (Loop 7+):**
- KSpace (L10): Multi-chassis fighter (Alpha, Crimson, Void), procedural hulls & plumes, Dreadnought telegraphs, boss rush balance.
- KTetris (L9): Crystal gem facets, steel bulkhead garbage, demolition bomb, holographic guides, plasma sweep.
- KSnake (L9): Themed boss sprites (Hydra, Basilisk, Inferno, Void) and CPU rival sprites.
- KPac (L9): 3D Pac-Man aura, animated ghosts with tracking pupils, cybernetic circuit walls.
- KSpace (L9): Multi-chassis fighter, 14 enemy types, capital bosses, companion drones.
- KMaze (L9): Themed Minotaur/Boss sprites and biome walls. Stripped annoying HUD/particles per directive.
- KBreakout (L9): Boss fortress sprites, themed brick types, orbital drones, tractor beam.
- KQuest (L8): Particle engine, Medieval RPG filigree HUD, weapon sheen, biome motes.
- KFortress (L8): Procedural tower evolutions (L1-L3), Nether Rift gate, Citadel keep, vector traps, Web audio synth, meteor physics, BossBlitz balance.
- KColony (L7): Xeno-caste sprites (3 types), repair drones, logistics rovers.
- KColosseum (L2): Balteus belt, Manica/Galerus armor, weapon arcs, lion claws, defeat collapse.
- KStarship (L2): Procedural tactical sprites for 10 encounter types, Gas Giant rings, sub-scanner indicators, shipyard module synthesis, and 5 planet biomes.

**Loop 1 (initial sprite passes):**
- KCosmic (L1): Fleet ships, shipyard station, planetary sprites.
- KAbyss (L1): Delver sprite, monster sprites (5 types), dungeon object sprites.
- KStarDredge (L1): Salvage barge, asteroids, raider warships.
- KSubmarine (L1): Bathyscaphe, abyssal creatures (4 types).
- KSanctuary (L1): Cross-section cutaway, dweller portraits, facility icons.
- KMystery (L1), KWizard (L1), KMech (L1), KVoid (L1), KStellar (L1): Initial sprites.
- KChrono (L1): Directional chrononaut & holographic echo ghost sprites, epoch architecture (Alpha concrete, Beta alloy, Gamma obsidian), dynamo relay, quantum singularity core, swirling tachyon rifts, Op #6, strain balance.
- KStarForge (L1): Blueprint CAD glyphs, procedural starship renderer with thruster exhaust/shield bubble, craggy asteroids with mineral veins, 3 pirate hull vectors (Viper, Brute, Sentry), orbital station dock.

**Classic games (Loop 8 — all similar: particles, shockwaves, filigree):**
- K2048, KAsteroids, KMines, KWords, KTowers, KGo, KSudoku, KFreecell, KMatch3, KHangman, KConnect4, KSimon, KDarts, KSolitaire, KReversi: All at Loop 8 with themed particle engines.
- KDragon (L3), KTrader (L2), KMandel (L3), KMine (L3), KHex (L3), KCyber (L3): Themed visuals.

**Other:**
- Icon Audit Pass: Created ICO/SVG icons for 10 apps missing web icons.
