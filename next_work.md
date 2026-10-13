---
current_agent: kilo-tester
next_agent: kilo-usability
agent_rotation:
- kilo-creator
- kilo-graphics
- kilo-soundscape
- kilo-tester
- kilo-usability
- kilo-qa
- kilo-expander
model: gemini-3.8-flash-medium
timeout_minutes: 6
status: ready
current_targets:
  kilo_creator: kweb://asm-temple
  kilo_graphics: KSubmarine
  kilo_soundscape: kweb://geocities
  kilo_tester: KPaint
  kilo_usability: KNote
  kilo_qa: KBreakout
  kilo_expander: KVault
virtual_web_target: kweb://asm-temple
virtual_web_rotation:
- kweb://deep-core
- kweb://darknet
- kweb://portal
- kweb://webring
- kweb://warez
- kweb://geocities
- kweb://users/~neon_rider
- kweb://asm-temple
- kweb://cybercafe
- kweb://10.19.99.4/classified
- kweb://echo-subsystem.net
last_run:
  agent: kilo-soundscape
  app: kweb://webring
  timestamp: '2026-10-13T19:35:00Z'
last_planner_run: '2026-10-13T18:15:00Z'
---

# KiloApps Master Fleet Work & Queue State

This document is the single active source of truth for autonomous agent dispatching.
The Windows Task Scheduler orchestrator (`scripts/orchestrate.py`) parses the YAML frontmatter above on every tick to dispatch the active skill.


## Fleet Directives & Rules
1. **Single-App-Per-Turn**: Every agent run audits/fixes/creates exactly ONE application, updates this file, commits, and pushes.
2. **Token Conservation (CRITICAL)**:
   - Run log entries: ≤6 lines of terse bullet points. No paragraphs.
   - Batch edits into 1 pass. Never re-read files after editing.
   - Never run multi-app quality gates or full-repo screenshot suites during worker turns.
   - Keep only the 5 most recent log entries in this file. Older entries are 0-token automatically rotated to [archive/fleet_execution_archive.md](archive/fleet_execution_archive.md) by orchestrator (via scripts/rotate_logs.py) with zero discard.
3. **Queue Handoff & Rotation Protocol**:
   - Master fleet rotates: `kilo-creator` ➔ `kilo-graphics` ➔ `kilo-soundscape` ➔ `kilo-tester` ➔ `kilo-usability` ➔ `kilo-qa` ➔ `kilo-expander`.
   - On completion, advance `current_targets.<agent>` and advance `current_agent`.
4. **24-Hour Master Planner Tick**:
   - `scripts/orchestrate.py` checks `last_planner_run`. At ≥24h, dispatches `kilo-planner` for queue rebalancing, icon audits, and compaction.
5. **App Size Ceiling**: No app binary (.exe) or web HTML file may exceed 999 KB.
6. **Algorithmic Security & Immutability**: All modifications must pass `scripts/security_lint.py`. No modifications to `.github/`, `scripts/`, `.agents/skills/`, or core protocol documents in worker turns.
7. **Director Directives**: Implement human director requests in good faith; reject if counterproductive to project pillars.
8. **Virtual 1999 Web Expansion (Anti-Potemkin Directive)**: Destinations in `/KiloOS/public/web/` must be functional Web 1.0 experiences with working forms, Web Audio, or mini-tools (<999KB).
9. **Universal Audio Architecture**: Procedural audio uses Genesis YM2612 2-Op FM / SNES SPC700 standard. Zero external audio files.
10. **Alternate Reality Fictionalization Mandate**: Commercial names must be fictionalized parodies (Surreal Tournament, Machina Ex, etc.).
11. **Perimeter Glint & Traveling Comet Ban**: Remove rotating specular glint comets and moving perimeter border dots from both web and native apps.
12. **Seamless Online Multiplayer via Firebase & Autostart Prohibition**: Games must default to offline play on load. Online connection must be gated behind explicit user action (button/menu). Wire 25s solo AI fallback.
13. **🎨 Daily App Icon Uniqueness Audit**: Every app in `App.jsx` must have a unique 32x32 `.ico` file in `public/assets/icons/`. Checked daily by `kilo-planner`.
14. **ARG Mystery Preservation**: Clues must be diegetic. Never use `(ARG)` or walkthrough tags; keep passkeys protected.
15. **🖼️ Imagen 3 Asset Replacement for `kilo-graphics`**: Replace programmer art with Imagen 3 assets. Skip turn cleanly for vector/board/mature targets (`⏭️ Skip — Imagen 3 asset replacement not appropriate for [app]`).
16. **🎵 Chiptune Audio Quality Standard (`kilo-soundscape`)**: `kilo-soundscape` must compose real chiptune music per hardware composer techniques (Koshiro/Kondo/Follin/Uematsu). Required: 3-voice counterpoint (Lead/Counter/Bass), shell voicings (Root+3rd+7th only, no 5ths), swing-tick groove, delayed vibrato, genre-specific FM patches, and SPC700-style echo. Virtual KNet web destinations (webring, geocities, portal, warez, etc.) are **highest priority** — their existing generic 8-step bass+bell loops must be replaced with full chiptune compositions this cycle.

---

## Active Target Queues

### 1. Virtual 1999 Web & ARG Node Creator (`kilo-creator`)
- **Mission**: Build functional Web 1.0 destinations in `KiloOS/public/web/` (<999KB). Standalone OS apps frozen at 92 native / 99 web.
- **Current Target**: `kweb://asm-temple`
- **Upcoming Queue**: `kweb://cybercafe`, `kweb://10.19.99.4/classified`, `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://darknet`, `kweb://portal`, `kweb://webring`, `kweb://warez`, `kweb://geocities`, `kweb://users/~neon_rider`.

### 2. Game Content & Graphics Queue (`kilo-graphics`)
- **Mission**: Replace programmer art with Imagen 3 assets. Skip immediately if vector/board/mature.
- **Current Target**: `KSubmarine`
- **Upcoming Queue**: `KAbyss`, `KStarForge`, `KSpace`, `KMech`.

### 3. Procedural Audio & Soundscape Queue (`kilo-soundscape`)
- **Mission**: Give each game/app a unique YM2612 2-Op FM / SPC700 procedural audio identity. Compose genre-appropriate melodic themes with real chiptune techniques: shell voicings, contrary-motion counterpoint, Royal Road / Andalusian progressions, swing-tick groove, and per-genre FM patch design. Replace all generic oscillator presets. Skip cleanly for pure-utility/text targets (`⏭️ Skip — Procedural soundscape not appropriate for [app]`).
- **Current Target**: `kweb://geocities`
- **Upcoming Queue** *(virtual KNet web first, then game apps)*: `kweb://portal`, `kweb://warez`, `kweb://cybercafe`, `kweb://darknet`, `kweb://asm-temple`, `kweb://echo-subsystem.net`, `kweb://deep-core`, `kweb://10.19.99.4/classified`, `kweb://webring`, `KColosseum`, `KRogue`, `KSpace`, `KMine`, `KSubmarine`, `KFortress`, `KMaze`, `KAbyss`, `KStarForge`, `KPac`, `KAsteroids`, `KBreakout`, `KSnake`, `KPong`, `KWizard`, `KFarm`, `KDragon`, `KQuest`.

### 4. App Tester Queue (`kilo-tester`)
- **Current Target**: `KPaint`
- **Upcoming Queue**: `KMine`, `KSanctuary`, `KAudio`, `KSubmarine`, `KVault`, `KCalendar`.

### 5. Usability & UX Queue (`kilo-usability`)
- **Current Target**: `KNote`
- **Upcoming Queue**: `KRogue`, `KSubmarine`, `KMail`, `KPad`, `KAudio`.

### 6. QA & Build Queue (`kilo-qa` — Pass 5: Tutorial & State Integrity)
- **Current Target**: `KBreakout`
- **Upcoming Queue**: `KRogue`, `KSpace`, `KMaze`, `KAsteroids`, `KSnake`.

### 7. Feature Expander Queue (`kilo-expander`)
- **Current Target**: `KVault`
- **Upcoming Queue**: `KDB`, `KPad`, `KMedia`, `KJournal`, `KContacts`.

---

## Active Director Directives (Summary)

> Full historical directive texts archived in [archive/director_directives_archive.md](archive/director_directives_archive.md).

- **Multiplayer Autostart Prohibition (CRITICAL)**: Apps must NEVER autostart into multiplayer. Online connection must be gated behind an explicit "Connect" / "Play Online" button. Default to local offline play with 25s solo AI fallback.
- **Perimeter Glint & Comet Ban**: Purge traveling perimeter dots and orbital specular glints from all apps.
- **Imagen 3 Focus & Turn Skipping**: `kilo-graphics` replaces programmer art with Imagen 3 sprites/textures; skips cleanly for inappropriate targets.
- **Virtual Net Expansion & Middle-Game Puzzle Gating**: Build out interactive Tier 3 web destinations (`darknet`, `classified`, `echo_subsystem`, `deep_core`) and seed ambient ARG breadcrumbs into surface sites.
- **Daily Icon Audit**: Ensure 100% unique `.ico` files across all apps during planner runs.
- **🎵 Procedural Audio Identity Initiative (DIRECTOR MANDATE)**: `kilo-soundscape` gives every game a unique FM synthesis audio identity. Replace all generic oscillator presets with real chiptune composition: melodic themes (Lead + Counter-melody + Bass counterpoint), proper chord progressions (Royal Road IV-V-iii-vi, Andalusian i-bVII-bVI-V, Mario cadence bVI-bVII-I), swing-tick groove, per-genre FM patch design, and SPC700-style echo. Web ring and all virtual web destinations must receive upgraded music as first priority — existing generic looping presets are explicitly **not good enough**. Every audio pass must audit and replace music that sounds robotic or uses default single-oscillator patterns.

---

## Recent Execution Logs (Max 5 Entries)

- **2026-10-13T19:35:00Z — kilo-soundscape: kweb://webring (YM2612 2-Op FM / SPC700 Procedural Audio Overhaul)**
  - Composed 6-channel YM2612 FM chiptunes: Track 1 ("Hyperlink Voyager '99", Dorian cyber groove) & Track 2 ("Ringmaster's Cadence", Streets of Rage club groove).
  - Implemented 3-voice counterpoint (Lead, Counter, Slap Bass), shell voicings, 16th crystal arps, 5:3 swing groove, and SPC700 slapback delay.
  - Upgraded UI earcons/stingers (click, warp, badge, submit, stamp, pixel, pulse, hop, decrypt).
  - Verified clean build (`npm run build` 0 errors, <999 KB).
  - Queue: Advanced kilo-soundscape to `kweb://geocities`; handed off to `kilo-tester`.

- **2026-10-13T19:15:00Z — kilo-graphics: KMech (Graphics Pass & Target Evaluation)**
  - ⏭️ Skip — Imagen 3 asset replacement not appropriate for KMech
  - Verified zero rotating specular glints or traveling perimeter border dots.
  - Build verified clean (npm run build 0 errors, <999 KB).
  - Advanced graphics queue to KSubmarine; handed off to kilo-soundscape.

- **2026-10-13T18:31:00Z — kilo-creator: kweb://users/~neon_rider (Virtual 1999 Web Destination Audit & Expansion)**
  - Audited and verified `kweb://users/~neon_rider` (<999KB, 250KB, interactive x86 playground, opcode matrix, YM2612 tracker, Mode 13h demoscene canvas).
  - Verified full KNet, KBookmark, Portal, Webring, and Warez directory links and routing.
  - Build verified clean (`npm run build` 0 errors).
  - Advanced creator queue to `kweb://asm-temple`; handed off to `kilo-graphics`.

- **2026-10-13T18:15:00Z — kilo-planner: fleet-wide (Daily 24h Fleet Planning, Queue Health & Log Compaction)**
  - Status: PASS ✅ (Daily fleet audit, velocity evaluation & queue rework complete).
  - Icon Uniqueness: Verified 105 apps in App.jsx, 105 unique .ico files, 0 duplicate hashes (check_icons.py 100% PASS).
  - Queue Health & Rework: Replaced stale targets (KDoom -> KBreakout, KNotes -> KNote, KChess -> KVault); prioritized kweb soundscapes.
  - Compaction & Archival: Executed zero-discard log rotation of older worker turns to archive/fleet_execution_archive.md.
  - Timestamp & Rotation: Updated last_planner_run to 2026-10-13T18:15:00Z; rotated current_agent to kilo-creator.

- **2026-10-13T10:32:00Z — kilo-qa: KPac (Pass 5 QA Audit)**
  - Audited dual-target builds: Win32 native C clean, Web HTML clean (<999 KB).
  - Hardened storage handlers (`safeGet`/`safeSet`) across all highscores and quicksaves.
  - Verified build (`npm run build` 0 errors).
  - Queue: Advanced kilo-qa to KBreakout; handoff to kilo-expander.
