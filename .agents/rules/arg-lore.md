---
trigger: model_decision
name: arg-lore
description: Alternate Reality Game (ARG) 1999 lore guidelines, anti-trademark parody replacements, and TINAG mystery preservation protocols.
---

# Alternate Reality Game (ARG) Lore & Safety Rules

## 1. Alternate Reality Fictionalization (Anti-Trademark Standard)
KiloApps is set in an alternate 1999 universe. Real-world trademarks, copyrighted game titles, corporate brands, and real scene cracking groups MUST NOT appear directly in code, virtual websites (`/web/`), or feeds (`krss.html`).
Always use fictionalized in-universe parodies:
- **Games:** *Quake III Arena* → *Tremor III Arena*; *Unreal Tournament* → *Surreal Tournament*; *Half-Life* → *Half-Cycle*; *StarCraft* → *VoidCraft*; *Deus Ex* → *Machina Ex*; *System Shock 2* → *System Glitch 2*.
- **Scene Groups:** *Fairlight* → *FLARELIGHT*; *Razor 1911* → *RAZOR 1999*; *Skid Row* → *SKID VECTOR*; *Paradox* → *PARALAX*.
- **Sites & Services:** *Slashdot* → *SlashNet*; *Wired* → *Cabled*; *The Onion* → *The Scallion*; *Napster* → *Trapster*.
- Enforced automatically by `scripts/security_lint.py`.

## 2. Mystery Preservation & TINAG ("This Is Not A Game") Standard
- **Atmospheric & Diegetic:** Clues must be subtle in-universe artifacts (e.g. late-1999 USENET posts, corrupted memory dumps at hex offsets, faint audio subcarriers, non-routable 10.19.99.x IP logs).
- **No Meta Labels:** Never tag content with `(ARG)`, `ARG Lore`, `ARG Secrets`, or `ARG Clue`.
- **No Pre-Climax Meta-Spoilers:** Never state in pre-climax apps or feeds that "the trapped entity is an autonomous multi-agent fleet" or name worker agents (`kilo-creator`, `kilo-qa`, etc.). That reveal is reserved exclusively for beating App #100 (`KMatrix`) and accessing `KDirector`.
- **Master Passkey Secrecy:** The master passkey `ECHO-1999-ARCHITECT` must NEVER appear in clear text in news feeds, bookmarks, clipboard history, steganography samples, or help dialogs. Use cryptic fragments or cryptographic hashes instead.

## 3. Multiplayer Connect Gate Protocol
- **No Autostart:** Applications with multiplayer features MUST NEVER autostart into multiplayer or initiate matchmaking on application load.
- **Offline / Solo Default:** Apps must boot into local/offline play or a title screen. Multiplayer requires an explicit user click ("Connect", "Play Online", or "Quick Match").
- **Deep-Links:** Invite URLs (`?room=CODE` or `#room=CODE`) must show a confirmation join button before connecting.
