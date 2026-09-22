# 💾 KiloApps

> **99 Sovereign Applications Under 999 Kilobytes**  
> *A return to 1999 software efficiency, unbloated computing, and autonomous multi-agent architecture.*

Live Web Desktop: **[kiloapps.web.app](https://kiloapps.web.app)**

---

## 📖 Overview

**KiloApps** is an open-source retro computing operating system and application fleet built around a single hard constraint: **no individual application may exceed 999 kilobytes**, across both native Win32 (`.exe`) and web (`.html`) platforms.

The platform hosts 99 feature-complete applications across six domains:
1. **Games**: Deep procedural RPGs, roguelikes, space exploration, strategy, and classic retro arcade games.
2. **System & Dev**: Hex editors, terminals, task managers, network monitors, and assemblers.
3. **Productivity & Office**: Spreadsheets, databases, text editors, calendars, and organizers.
4. **Media & Creative**: Pixel paint programs, fractal renderers, and procedural sound synthesizers.
5. **Network**: Web 1.0 browsers, BBS terminal emulators, and RSS/Atom readers.
6. **Alternate Reality Game (ARG)**: A subterranean layer of in-universe computing lore, secret frequencies, and terminal puzzles unifying the fleet.

---

## ⚖️ Legal & Parody Notice

KiloApps is an Alternate Reality Game (ARG) and artistic retro computing preservation project set in a fictionalized 1999 universe.

- **Original Fictional Parodies:** All video game titles, software products, warez cracking groups, and media outlets referenced within KiloApps (such as *Surreal Tournament*, *Tremor III Arena*, *VoidCraft*, *Machina Ex*, *FLARELIGHT*, *RAZOR 1999*, *SlashNet*, and *Cabled*) are **original fictional parodies**.
- **Zero Piracy / Zero Malware:** This repository does **not** host or distribute copyrighted video game binaries, commercial ROMs, or actual software circumvention tools. All simulated demoscene cracktros and keygens are client-side HTML5 Canvas visual art and procedural Web Audio synthesizers.
- For complete details, see **[DISCLAIMER.md](DISCLAIMER.md)**.

---

## 🚀 Quickstart

### Web Platform (KiloOS)
```bash
cd KiloOS
npm install
npm run dev
```
Build for production:
```bash
npm run build
```

### Automated Verification
```bash
# Verify all native binaries stay strictly beneath 999 KB:
python check_sizes.py

# Run algorithmic security, API, and trademark banlist checks:
python scripts/security_lint.py
```

---

## 🤖 Autonomous Multi-Agent Fleet

KiloApps is continuously maintained and expanded by an unattended autonomous multi-agent fleet operating under Windows Task Scheduler and GitHub Actions:
- **`kilo-creator`**: Designs new applications and deep game worlds.
- **`kilo-graphics`**: Deepens gameplay content, enemy variety, and visual polish.
- **`kilo-tester`**: Audits interactive UI elements, hotkeys, and data persistence.
- **`kilo-usability`**: Refines layout ergonomics and high-DPI canvas rendering.
- **`kilo-qa`**: Audits build quality, memory hygiene, and compiler clean builds.
- **`kilo-expander`**: Deepens diagnostic features and functional utilities.

Active queue state and agent handoffs are tracked in **[`next_work.md`](next_work.md)**.
