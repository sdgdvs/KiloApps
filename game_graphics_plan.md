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
- **Logging discipline:** Keep this plan file concise. Work log entries MUST be MAX 2 LINES per game. Example: "KChess (Loop 9): Enhanced piece animations, board perspective shadows, and capture particle effects." Do NOT write paragraph-length descriptions.
- **⚠️ UNIQUE VISUAL IDENTITY (CRITICAL):** Do NOT apply the same formulaic template to every game. Each game should have a UNIQUE visual identity that fits its theme. A chess game should look like polished wood, not a sci-fi HUD. A farm game should have warm watercolors, not cybernetic reticles. Think about what makes THIS game visually distinct. Avoid copy-pasting the same "4-layer particle engine, dual-tier shockwaves, filigree brackets, specular glint" to every game.

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


- KSanctuary
- KSnake
- KTetris
- KCyber
- KBreakout
- KMaze
- KColony
- KFortress
- KColosseum
- KFarm
- KAlchemy
- KRogue
- KChess
- KPong
- KMines
- KSubmarine
- KAsteroids
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
- KStarship
- KConnect4
- KHangman
- KStarDredge
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
- KSpace
- KPac

## Completed Work Log (trimmed by Director 2026-09-06 — latest loop per game only)

- KPac (Loop 9): 3D Pac-Man with ambient aura, animated wavy-skirt ghosts with tracking pupils, high-detail fruits, energizers, and cybernetic circuit walls.
- KSpace (Loop 9): Multi-chassis player fighter, custom projectile types, detailed enemy sprites across 14 types, capital boss encounters, and companion drones.
- KQuest (Loop 8): Procedural screen-shake, 4-layer particle engine, ground shockwaves, Medieval RPG filigree HUD, weapon specular sheen, and biome-specific atmospheric motes.
- KReversi (Loop 8): 60fps canvas loop, ambient motes, traveling specular glint, dual shockwaves, and double-buffered rendering.
- KMystery (Loop 1): Initial detective-themed sprites and crime scene art.
- KWizard (Loop 1): Initial wizard sprites and spell effect particles.
- KWords (Loop 8): 4-layer particle physics, shockwave ripples, ice/wood shard debris, and Art Deco filigree on mahogany frames.
- KTowers (Loop 8): 4-layer particles, shockwave ripples, screen shake, Art Deco filigree, and rooftop searchlight beacons.
- KGo (Loop 8): Multi-layer particles, placement shockwave rings, Japanese gold leaf filigree, obsidian/shell stone sheen.
- KSudoku (Loop 8): Multi-layer particle explosions, golden border shimmer on solved blocks, and corner filigree brackets.
- KFreecell (Loop 8): Procedural screen-shake, multi-layer particles, golden border shimmer, and filigree brackets.
- KMatch3 (Loop 8): Physics-driven screen-shake, multi-layer particles, and special gem visual variations.
- KMech (Loop 1): Initial mech sprites with combat animations.
- KHangman (Loop 8): Procedural screen-shake, 4-layer particles, dual shockwaves, and atmospheric motes.
- KConnect4 (Loop 8): Procedural screen-shake, disc drop particles, dual shockwaves, and ornate frame.
- KStarship (Loop 1): Initial starship sprites and space environment.
- KVoid (Loop 1): Initial void-themed sprites and dark atmospheric effects.
- KMandel (Loop 3): Fractal-themed visuals with zoom animations and color cycling.
- KMine (Loop 3): Mining-themed sprites with terrain variety and atmospheric effects.
- KStellar (Loop 1): Initial starship dashboard sprites and galaxy visuals.
- KHex (Loop 3): Hex editor themed visuals with byte highlighting effects.
- KSolitaire (Loop 8): Screen-shake on card snaps, casino particle engine, filigree brackets, and specular sheen sweeps.
- KDragon (Loop 3): Atmospheric effects, screen shake, particle engine, and medieval gold filigree.
- KTrader (Loop 2): Multi-frame ship sprites, parallax starfield, planet rendering, laser projectiles, and particle engine.
- KSimon (Loop 8): Screen-shake, 4-layer particles, dual shockwaves, cyber dust, and retro-arcade filigree.
- KDarts (Loop 8): Multi-layer particles, dual shockwaves, brass filigree, specular sweeps, and tavern dust motes.
- K2048 (Loop 8): Multi-layer particles, dual shockwaves, tile specular sheen, and cybernetic arcade HUD.
- KAsteroids (Loop 8): Screen-shake, multi-layer particles, dual shockwaves, and cybernetic Sci-Fi HUD.
- KMines (Loop 8): Multi-layer particles, dual shockwaves, tile highlights, and cybernetic arcade HUD.
- KPong (Loop 8): Screen-shake, multi-layer particles, paddle specular sheen, and cybernetic HUD.
- KChess (Loop 8): Screen-shake, marble/mahogany particles, classical filigree, and piece specular sweeps.
- KRogue (Loop 8): Screen-shake, dungeon particles, Gothic RPG filigree, and biome-specific atmospheric motes.
- KAlchemy (Loop 7): Arcane particle engine, shockwaves, alchemical filigree, and mystic dust motes.
- KFarm (Loop 6): Seasonal weather particles, roaming livestock sprites, windmill/scarecrow, and rustic filigree.
- KColosseum (Loop 1): Gladiator sprites, lion/chariot enemies, Roman arena environment, and particle system.
- KFortress (Loop 6): Multi-layer particles, dual shockwaves, medieval HUD, and biome-specific weather motes.
- KColony (Loop 6): Multi-layer particles, screen-shake, cybernetic HUD brackets, and planetary biome motes.
- KMaze (Loop 8): Screen-shake, 4-layer particles, dual shockwaves, dungeon HUD reticles, and atmospheric embers.
- KBreakout (Loop 8): Screen-shake, multi-layer particles, dual shockwaves, metallic paddle with thruster nozzles.
- KCyber (Loop 2): Cyberdeck sprites, ICE Daemon enemy, holographic data payload, and hack-themed particle effects.
- KTetris (Loop 8): Screen-shake, multi-layer particles, dual shockwaves, HUD reticles, and tetromino specular sheen.
- KSnake (Loop 8): Screen-shake, multi-layer particles, dual shockwaves, HUD reticles, and viper body specular sheen.
- KSubmarine (Loop 0): Not yet processed.
- KStarDredge (Loop 0): Not yet processed.
- KSanctuary (Loop 0): In queue.
- KAbyss (Loop 0): In queue.
