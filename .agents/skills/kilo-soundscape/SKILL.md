---
name: kilo-soundscape
description: >-
  Composes unique YM2612 2-Op FM / SPC700-style procedural audio identities for KiloApps games and
  virtual web destinations using 6-channel counterpoint, shell voicings, swing-tick grooves, and
  1-command verification/handoff.
---

# KiloApps Procedural Audio & Soundscape Skill (`kilo-soundscape`)

Executes one procedural chiptune audio overhaul per turn modelled on 16-bit hardware composers (Yuzo Koshiro, Koji Kondo, Tim Follin, Nobuo Uematsu). Zero external audio files — 100% Web Audio API.

## ⚡ Flash Execution & Tool-Budget Protocol (CRITICAL)
- **Tool Call Budget (<= 32 tool calls)** & **Commit Checkpoint (~16 tool calls)**:
  - Read the audio section of the target file (`KiloOS/public/apps/k<name>.html` or `KiloOS/public/web/<name>.html`), design the 6-channel FM tracker & SFX earcons, apply the edit cleanly, and verify!
- **Fast Skip Path (<= 3 tool calls)**:
  - If the target is a pure text/system utility or already has a complete 6-channel YM2612/SPC700 tracker suite, run `python scripts/advance_turn.py --agent kilo-soundscape --app <Target> --title "Skip — Audio Complete/Inappropriate" --bullet "⏭️ Skip — Procedural soundscape already complete or not appropriate for <Target>."`, commit, push, and STOP immediately!
- **1-Command Verification**:
  ```powershell
  python scripts/verify_turn.py --app <Target>
  ```
- **1-Command Queue Handoff & Logging**:
  ```powershell
  python scripts/advance_turn.py --agent kilo-soundscape --app "<Target>" `
    --title "<Track Title> — YM2612/SPC700 Chiptune Suite" `
    --bullet "Status: PASS ✅ (<copy summary from verify_turn.py>)." `
    --bullet "Composition: <mode/progression>, 6-channel counterpoint, 3rd/7th shell voicings, swing groove." `
    --bullet "Patches & DSP: <named FM patches + selective 180ms SPC700 stereo delay + earcons>."
  ```
- **Commit, Push & STOP**:
  ```powershell
  git add -A
  git commit -m "audio(<Target>): <Track Title> — chiptune soundscape pass"
  git push origin main
  ```

---

## Mandatory Hardware Composer Architecture
1. **6 Logical Channels**:
   - **Ch1 Lead Melody**: Expressive vocal phrasing with rests and delayed vibrato (ramp LFO after 180ms).
   - **Ch2 Counter-Melody**: Contrary motion to Lead; takes spotlight when Lead rests.
   - **Ch3 Harmony / Arpeggio**: Shell voicings (3rd + 7th only — never double the 5th).
   - **Ch4 FM Bass**: Punchy 2-Op FM slap/sub bass (`modRatio: 2.0`, fast mod decay) strictly below `C3` (`130.81Hz`).
   - **Ch5 Kick + Snare**: FM pitch-drop kick + high-MUL noise snare.
   - **Ch6 Inharmonic Hi-Hat / Earcons**: Metallic FM ratios (`11.0`, `13.5`) + context-specific UI/combat stingers.
2. **Swing-Tick Groove**: Use alternating `setTimeout` step durations (e.g. `175ms` even / `105ms` odd for 5:3 groove) instead of rigid robotic `setInterval`. Clean up timer on `visibilitychange` when hidden!
3. **Selective SPC700 Stereo Delay**: Route Lead and Counter through a low-pass filtered feedback delay (`180ms–240ms`, `2200Hz` LP, `0.28` feedback); keep Bass and Drums strictly dry.
4. **Controls**: Ensure `[M]` keyboard shortcut and visible Mute/Audio toggle button work cleanly.
