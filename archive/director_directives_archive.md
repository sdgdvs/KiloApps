# Director Directives Archive

> Archived from next_work.md to reduce per-turn token overhead.
> These directives are now codified in AGENTS.md rules and skill instructions.
> Agents do NOT need to read this file — the rules are already in their system prompt.

## Director Directives

> Human-sourced requests from the KDirector console. See Rule 7 above.
> Agents: implement in good faith, but **reject if counterproductive** to project pillars.

<!-- DIRECTIVE ENTRY FORMAT (paste from KDirector "ðŸ“‹ Copy for next_work.md" button):
- **[TARGET_APP] â€” Category** | Director Directive
  - âš ï¸ AGENT NOTE: This is a human director request, not a machine-generated task. Evaluate whether this directive aligns with the project's core pillars (999KB retro OS, ludonarrative consonance, ARG integrity, fleet stability) before implementing. If counterproductive, skip and log your reasoning.
  - Instructions: <directive text here>
-->

- **[FLEET-WIDE] â€” Pivot to ARG, Multiplayer & Virtual Net Expansion (Freeze Standalone App Creation)** | Director Directive
  - âš ï¸ AGENT NOTE: Human director priority directive.
  - Instructions: Halt creation of new standalone OS apps (frozen at 92 native / 99 web).
    1. **Virtual Net**: Pivot `kilo-creator` 100% to building real, rich Web 1.0 destinations in `KiloOS/public/web/` (`users/~neon_rider`, `asm-temple`, `cybercafe`, Tier 3 hidden nodes) and linking them to `KNet`.
    2. **Multiplayer**: Keep `kilo-expander` dedicated to seamless Firebase RTDB multiplayer retrofits (`KGo`, `KReversi`, `KDarts`, `KTetris`, etc.) so users across different computers can play together without servers.
    3. **ARG Clue-Weaving**: Weave subtle ARG clues (Arc 1 & 2 per `arg_plan.md`) into existing apps: `KHex` (internal IP `10.19.99.4/classified` offset), `KSynth` (1999Hz morse spelling `echo-subsystem.net`), `KTerm` (glitched sysadmin log pointing to `kweb://deep-core`), `KBBS` (sysop server notes), and `KNote` (`system_recovery_1999.log`).

- **[FLEET: kilo-creator & kilo-expander] â€” Seamless Online Multiplayer via Firebase** | Director Directive
  - âš ï¸ AGENT NOTE: Human director request. Priority architectural directive for creator and expander agents.
  - Instructions: Concentrate on adding multiplayer features that work seamlessly through Firebase Realtime Database with different people playing on kiloapps.web.app from different computers that are not otherwise communicating, similar to how KChat allows chat from the global room. Use the shared Firebase RTDB (`https://kiloappschat-default-rtdb.firebaseio.com`) with CDN imports and clean room namespacing (`multiplayer/<app>/...`).

- **[ALL_APPS / FLEET] â€” Visual Quality & Graphics** | Director Directive
  - âš ï¸ AGENT NOTE: Human director request. Priority fleet-wide directive.
  - Instructions: Systematically remove rotating/traveling specular glint comets, perimeter glint dots, and moving border balls across both web (HTML) and native (Win32 C) on every app pass. They are annoying across every app and look like distracting projectiles/balls. Replace with clean, static, or period-accurate borders without traveling dots or orbital glint particles. NEVER add new perimeter traveling glints.

- **[FLEET: kilo-graphics & kilo-planner] â€” Daily App Icon Uniqueness Audit** | Director Directive
  - âš ï¸ AGENT NOTE: Human director request. Priority fleet-wide directive.
  - Instructions: Ensure that every application registered in `KiloOS/src/App.jsx` has a unique, visually distinctive 32x32 `.ico` file in `KiloOS/public/assets/icons/`. Reusing or copying existing icons is strictly prohibited. Run `python scripts/check_icons.py` to audit for missing or duplicate icon hashes across the fleet daily during planner ticks and on graphics passes. If duplicates are found, resolve them immediately using `python scripts/check_icons.py --fix`.

- **[FLEET: kilo-qa, kilo-usability, kilo-tester] â€” Toast Occlusion & Modal Clipping Remediation** | Vision Audit Directive
  - âš ï¸ AGENT NOTE: Secondary state vision audit revealed 26 apps where persistent or timed toasts (`z-index: 150-200`) overlap interactive controls (buttons, inputs, close icons) and 10 apps with clipped dialogs/virtual keyboards.
  - Instructions: During app passes, ensure toasts do not occlude interactive inputs or primary buttons (position toasts safely, dismiss on click/interaction, or use unobtrusive non-overlapping toast bars). Fix double-modal stacking (`kclip` - fixed, `kpomodoro` - fixed) and remove internal loop labels (`kdarts`, `kwords`).

- **[ARG / FLEET-WIDE] â€” Middle-Game Puzzle Chain & Tier 3 Node Gating** | Director Directive
  - âš ï¸ AGENT NOTE: Human director priority directive. ARG structural improvement.
  - Instructions: The progression from "player discovers the virtual web" to "player is ready for KMatrix endgame" is too loose. Agents (`kilo-creator`, `kilo-expander`) must build a **gated middle-game puzzle chain** across the Tier 3 nodes (`darknet`, `classified`, `echo_subsystem`, `deep_core`). Specifically:
    1. **Sequential key-artifact system**: Each Tier 3 node should yield a specific artifact (a decoded phrase, a hex offset, a frequency value, a file fragment) that is **required as input** to unlock a deeper layer on a *different* Tier 3 node. Example: decoding a Morse transmission on `echo_subsystem` reveals a memory offset that, when entered in `darknet`'s hex inspector, unlocks a classified memo fragment; that memo contains coordinates that unlock a hidden panel on `classified`.
    2. **Puzzle chain must converge on `deep_core`**: The final Tier 3 node before the endgame. All threads from the other 3 nodes should feed information needed to reach `deep_core`'s inner sanctum, which in turn points players toward KMatrix (App #100).
    3. **No walkthrough scaffolding**: Implement via diegetic mechanisms only (input fields that validate specific answers, hex addresses that highlight when correct, frequency lock indicators). Never explain the sequence or label puzzle steps.
    4. **Breadcrumb ordering hint**: Subtly suggest the investigation order via in-universe timestamps, log sequence numbers, or packet IDs â€” e.g., `echo_subsystem` logs are dated earliest, `deep_core` latest.

- **[ARG / VIRTUAL WEB] â€” Surface-Site Breadcrumb Density Pass** | Director Directive
  - âš ï¸ AGENT NOTE: Human director priority directive. ARG discoverability improvement.
  - Instructions: The "surface" virtual web sites (`geocities`, `warez`, `webring`, `portal`, `asm_temple`, `cybercafe`) currently have very few ARG breadcrumbs (geocities: 1 reference, warez: 3). Players who only explore surface sites may never feel the pull toward the mystery. Agents (`kilo-creator`, `kilo-expander`, `kilo-graphics`) must seed **subtle, atmospheric anomalies** into every surface site during their next pass:
    1. **Geocities**: A corrupted guestbook entry from a user whose timestamp reads `1999-12-31 23:59:58` containing garbled text that, when decoded (ROT13 or hex), spells a Tier 3 URL fragment. A "neighborhood watch" bulletin mentioning unusual network traffic from subnet `10.19.99.x`.
    2. **Warez**: A scene NFO file from a fictional group whose release notes contain a suspiciously specific frequency (`1999Hz`) and mention intercepting "echo transmissions." A cracktro that briefly flashes hex addresses matching `deep_core` offsets.
    3. **Webring**: One or two "dead" webring nodes in the topology map that resolve to `???` or show anomalous ping times, hinting at hidden nodes not in the public directory. A traceroute that passes through a gateway named `echo-gw-07.kilonet.internal`.
    4. **Portal**: A classified ad posted by "SysAdmin_NULL" seeking help with "anomalous signal patterns at 1999Hz" with a reply-to address pointing to the echo subsystem. A news ticker item about unexplained network anomalies on the corporate intranet.
    5. **ASM Temple**: An opcode reference entry with a "NOTE" annotation referencing an undocumented instruction behavior observed "only on KiloNet internal nodes." A PE32 dissector sample binary whose embedded strings contain Tier 3 URL fragments.
    6. **Cybercafe**: An IRC channel message from a user warning about "ghost packets from 10.19.99.4" or a BBS thread titled "has anyone else heard the signal?" with timestamps that form a pattern.
    7. **Tone**: All breadcrumbs must be *atmospheric and ambient* â€” things a casual user might dismiss as flavor text but an attentive investigator would collect. Never label, highlight, or explain them.

- **[ARG / FLEET: kilo-creator & kilo-expander] â€” Solo-Completable Collaborative ARG Mechanics** | Director Directive
  - âš ï¸ AGENT NOTE: Human director priority directive. Multiplayer ARG design constraint.
  - Instructions: Build ARG puzzle mechanics that *leverage* the Firebase RTDB multiplayer infrastructure as a clue-delivery or puzzle-solving medium, but with the hard constraint that **every puzzle must be completable by a single person** using either a VM, a phone alongside their computer, or any two browser tabs. Do NOT require strangers or coordinated groups. Specifically:
    1. **Shared signal board**: A Firebase RTDB node (`arg/signals/...`) where certain in-app actions (e.g., tuning KRadio to 1999Hz, entering a specific hex value in KHex, completing a specific KCipher decode) silently write a "signal fragment" to the shared board. When enough fragments accumulate (from the same user across different apps, or across sessions), a hidden panel on one of the Tier 3 web pages reveals new content. The threshold should be reachable by one person using 2-3 apps.
    2. **Dual-presence puzzle**: One Tier 3 node requires two simultaneous "connections" â€” but this should work with two browser tabs on the same computer, or a phone and desktop both visiting `kiloapps.web.app`. Example: `classified` shows a locked signal diagnostic that requires a second user (or the same user in another tab) to be actively running `echo_subsystem`'s Morse transmitter at the correct frequency. The RTDB presence check should use session IDs, not unique users, so one person with two tabs qualifies.
    3. **Dead-drop guestbooks**: Certain Firebase guestbooks (`cybercafe` IRC, `geocities` shoutbox, `neon_rider` guestbook) should have a hidden mechanic where posting a specific passphrase (discoverable from puzzle chain artifacts) triggers a server-side Firebase rule or client-side listener that reveals a hidden response message containing the next clue. Since these are persistent RTDB writes, the player's own post triggers their own reveal â€” no second person needed.
    4. **Explicit design rule**: If a puzzle involves Firebase presence, matchmaking, or multi-session state, it MUST include a "solo path" â€” either via multi-tab, or via a time-delayed fallback (e.g., if no second connection appears within 30 seconds, the puzzle auto-advances with a "signal lock acquired from cached relay" diegetic message).

