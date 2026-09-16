# KiloApps UX Ergonomics & Retro Minimalist Standard

Governed by User Alignment Directive (2026-09-15).

## 1. The Core Philosophy
1. **1999 Lightweight Efficiency:** No app exceeds 999 KB. Clean, bloat-free, instant startup.
2. **Visual Restraint:**
   - NO intrusive first-person weapon overlays or HUD reticle animations.
   - NO continuous particle engines, screenshake, traveling specular glints, or blinking border diodes.
   - Authentic retro palette: High-contrast, clean Win32 or classic console / CRT palettes.
3. **Modern Ergonomics:**
   - Unified keyboard shortcuts across all web and native apps:
     - \Esc\ / \q\: Back, cancel, or exit modal / menu.
     - \Ctrl+S\ or \s\: Save current state / document.
     - \Enter\ / \Space\: Confirm or trigger primary action.
     - Arrow keys & \WASD\: Standard directional navigation.
   - **Responsive Scaling:** Web apps must scale cleanly to mobile and tablet screens using CSS aspect-ratio containment (\object-fit: contain\, \	ouch-action: none\).
   - **Touch Controls:** Web games must provide touch/tap/drag interactions or virtual on-screen d-pad fallbacks on touch devices.

## 2. Mature App Governance
Apps that have achieved stability across 6+ passes are listed in \mature_apps_registry.json\.
- These apps are **LOCKED_STABLE**.
- Worker agents are prohibited from altering mechanics, adding redundant features, or modifying visuals on locked apps without an explicit bug report or user directive.
