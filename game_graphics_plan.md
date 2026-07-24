# Game Graphics Plan

## Coordination Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 5 worker agents + 2 directors operate on this repo on overlapping schedules. You are the **Game Graphics** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`game_graphics_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Feature Expander), `app_fix_plan.md` (QA), `game_content_plan.md` (Games), `new_app_plan.md` (Creator), `kiloos_ux_plan.md` (inactive)
- **Shared file `KiloOS/src/App.jsx`** — You may ONLY edit icon paths in the APPS array. Protocol: `git pull` → minimal icon-only change → commit and push IMMEDIATELY before doing other work.
- **`KiloOS/src/index.css`** — Do NOT edit.
- **Size limit:** No individual KiloApp may exceed 999 kilobytes (web or native).
- **Testing:** After editing HTML → verify in browser if possible. After editing App.jsx → `cd KiloOS && npm run build`. After editing `.c` files → run the app's `build.bat`.
- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote for code you didn't write) → push again.
- **CLEANUP:** Before committing, delete any temporary scripts (patch_*.py, *.tmp) in the repo root. Do not leave scratch files behind.
- **Logging discipline:** Keep this plan file concise. Brief notes per completed item. Do NOT dump file contents or create verbose logs.

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

- KSimon
- KMatch3
- KFreecell
- KSudoku
- KGo
- KDarts
- KTowers
- KWords
- KReversi
- KQuest
- KSpace
- KPac
- KSnake
- KTetris
- KBreakout
- KMaze
- KRogue
- KChess
- KPong
- KMines
- KAsteroids
- K2048
- KStarship
- KSolitaire
- KConnect4
- KHangman

## Completed Work Log

- **KSpace (Loop 1):** Implemented custom player fighter sprite, 7 distinct enemy craft sprites (scout, chaser predator, purple saucer, heavy armored, cyan swift, rotating asteroid, red boss dreadnought), thruster flame frame animation, rotating energy shield, glowing plasma bullets, animated powerups, and a multi-color particle explosion system for both Web (kspace.html) and Native C GDI (KSpace/main.c).
- **KPac (Loop 1):** Implemented animated mouth chomp cycle for Pac-Man (with direction-based angles and speed boost aura), distinct ghost sprites (Blinky, Pinky, Inky, Clyde, Green) with movement-direction eye tracking and scared/flashing states, arcade-style double neon wall tiles, glowing pellets, pulsing power pellets, lightning speed item, frost freeze item, cherry fruit sprite, particle sparks, and floating score popups for both Web Canvas (kpac.html) and Native C Win32 GDI (KPac/main.c).
- **KSnake (Loop 1):** Implemented direction-aware snake head sprite with eyes and flickering red tongue animation, rounded multi-shade scale body segments, pulsing red apple with leaf & stem, golden star special food, floating cyan ghost food, ice crystal food, stone brick wall blocks with cracks & highlights, 8-legged creeping spider sprite with animated leg wiggles, swirling energy portals, dark metal tracker drone with pulsing red eye, and colorful eating particle bursts for both Web Canvas (ksnake.html) and Native C Win32 GDI (KSnake/main.c).
- **KTetris (Loop 1):** Implemented 3D beveled gem & metallic tetromino blocks for all piece types (I, J, L, O, S, T, Z, Garbage, Bomb), ghost piece outline with pulsing energy grid, hard drop particle trails & impact sparks, glowing white line clear flash animation, and floating score text popups for both Web Canvas (ktetris.html) and Native C Win32 GDI (KTetris/main.c).
- **KBreakout (Loop 1):** Implemented futuristic metallic paddle with side lights and animated energy core, energy ball with motion trail, 3D beveled bricks (silver metal, reinforced armor, row-tiered colors), powerup capsules with letter badges (Expand, Extra life, Pierce, Sticky, Laser), flying saucer UFO sprite, and brick destruction particle bursts for both Web Canvas (kbreakout.html) and Native C Win32 GDI (KBreakout/main.c).
- **KMaze (Loop 1):** Implemented custom 16x16 procedural wall textures (stone bricks, glowing exit portal, gold key block, steel door, coin chest, spike trap, cyan compass plate, speed boost bolt, teleporter vortex, red minotaur beast, crossed pickaxe block), distance fog & side shading, held equipment 3D HUD (swinging pickaxe and brass compass with target direction needle), particle bursts, and polished minimap with player direction arrow for both Web Canvas (kmaze.html) and Native C (KMaze/main.c).
- **KRogue (Loop 1):** Implemented custom knight player sprite (with helmet, visor slit, cape, sword, shield), 12 distinct animated monster sprites (giant rat, bat, orc, zombie, cave troll, ghost, gelatinous cube with inner skull, hydra, balrog with horns, titan, beholder with eyestalks, mind flayer with tentacles), 3D stone brick wall tiles with biome color palettes (Mossy green, Volcanic red, Void purple, Abyss gold-dark), stairs, doors, shrines, food, gold coins, floating damage numbers, hit spark particles, attack stroke animations, and responsive HUD bars for both Web Canvas (krogue.html) and Native C Win32 GDI (KRogue/main.c).
- **KChess (Loop 1):** Implemented custom 2D vector chess piece sprites for all 6 piece types (Pawn, Knight, Bishop, Rook, Queen, King) for White & Black, 3D wood grain & mahogany tiles with rank/file coordinates, smooth piece sliding animation, capture particle sparks, glowing move highlights, and check warning auras for both Web Canvas (kchess.html) and Native C Win32 GDI (KChess/main.c).
- **KPong (Loop 1):** Implemented futuristic neon metallic paddles with glowing center core lines and directional thruster flame effects, glowing energy ball with color-coded motion trails, powerup capsules with distinct badges (Golden Expand, Red Shrink, Ice Freeze), laser hazard obstacles, wall & goal impact particle bursts with expanding shockwave rings, and retro arcade CRT scanlines for both Web Canvas (kpong.html) and Native C Win32 GDI (KPong/main.c).
- **KMines (Loop 1):** Implemented 3D raised/beveled cell tiles with sunken revealed state, detailed iron mine bomb sprite with flickering fuse spark animation, animated crimson silk flag sprite on silver pole, cyan glowing question mark badge tile, interactive smiley status button with 4 animated expression states (normal happy, shocked click, wounded loss, cool sunglasses win), colored number tile texturing, and multi-color particle explosion debris & smoke burst FX for both Web Canvas/DOM (kmines.html) and Native C Win32 GDI (KMines/main.c).
- **KAsteroids (Loop 1):** Implemented detailed space fighter spaceship sprite with animated twin thruster flames & glowing shield bubble aura, 3 distinct rotating asteroid rock texture designs (Basalt Slate, Iron Rust, Cryo Ice) with inner craters & surface facets, alien saucer UFO enemy sprites with pulsing glass domes & blinking rim LEDs, Boss Mother UFO with satellite energy orbs & health bar, glowing plasma laser bullets, shockwave explosion FX, and twinkling parallax space starfield background for both Web Canvas (kasteroids.html) and Native C Win32 GDI (KAsteroids/main.c).
- **K2048 (Loop 1):** Implemented 3D metallic & gem tile texturing with distinct color themes for values 2 through 4096+, custom SVG and GDI badge icons (Crown for 2048+, Diamond for 1024, Stars for 256/512, Bomb & Wildcard badges), smooth merge pop animations with micro-wobble, 3D wooden/brushed metal board frame styling, milestone glowing aura rings, and interactive merge particle spark explosions for both Web Canvas/HTML (k2048.html) and Native C Win32 GDI (K2048/main.c).
- **KSolitaire (Loop 1):** Implemented detailed vector & GDI playing card suit sprites (Hearts ♥, Diamonds ♦, Clubs ♣, Spades ♠), detailed King, Queen, Jack court card portraits (with crowns, robes, tiaras, scepters, roses, halberds), Ace centerpiece emblems, pip matrix layouts (2-10), custom card back designs with centerpiece golden "K" crown shield logo, casino felt table micro-texture, gold/brass card slot outlines, and a winning cascade card waterfall bounce animation with trail FX for both Web Canvas/HTML (ksolitaire.html) and Native C Win32 GDI (KSolitaire/main.c).
- **KConnect4 (Loop 1):** Implemented 3D blue plastic grid board frame with cylindrical cutouts and inner depth shadows, 3D glossy red and yellow metallic checker discs with outer ridged rims and gold star / blue crown centerpiece emblems, gravity disc drop physics animation with bottom impact bounce, glowing neon beam line connecting the winning 4 discs with pulsing aura, and victory confetti particle spark bursts on win for both Web (kconnect4.html) and Native C Win32 GDI (KConnect4/main.c).
- **KHangman (Loop 1):** Implemented high-quality 3D wooden gallows (beveled oak/mahogany beams, brass brackets & rivets, pulley wheel, twisted hemp rope, animated swinging noose), detailed stick figure/character sprite added limb-by-limb with animated idle sway/breathe, expressive eyes (blinking, happy, scared pupil, dead X_X), blushing cheeks, cap, vest, boots, 3D tactile slate keycaps with hover glow and right/wrong state badging (✓/✗), and victory confetti streams / loss dark rain particle FX for both Web Canvas (khangman.html) and Native C Win32 GDI (KHangman/main.c).






