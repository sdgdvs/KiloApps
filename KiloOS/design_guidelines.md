# Design Guidelines for Minimalist KiloOS Apps

## Core Principle
**Minimalism in size and compute efficiency is more important than any particular style.** Every visual decision must be weighed against its cost in bytes and rendering time. While aesthetics matter, a lightweight, highly responsive application is the ultimate goal.

## Visual Aesthetics (Premium, Token‑Efficient)
- **Palette** – Use a dark‑mode base with accent colors in HSL ranges: `hsl(210, 15%, 12%)` (background), `hsl(210, 15%, 20%)` (card), `hsl(200, 85%, 60%)` (primary accent), `hsl(30, 85%, 60%)` (secondary accent).
- **Typography** – Import Google Font **"Inter"** (weight 400/600) via `<link>` in `index.html`. Set `font-family: 'Inter', system-ui, sans-serif;`.
- **Micro‑Animations** – Use CSS `transition` for hover/focus (0.15s ease) and `@keyframes fadeIn` for entry. Keep animation code under 300 bytes.
- **Glassmorphism** – Apply lightweight backdrop‑filter only at `@media (prefers-reduced-motion: no-preference)` to respect low‑power devices.
- **Responsive Layout** – Use CSS Flexbox/Grid with max‑width 800px for desktop, full width for mobile. No external CSS frameworks.

## Code Style (Token‑Efficient)
- **File Names** – Keep under 12 characters, e.g., `app.jsx`, `ui.css`.
- **Component Naming** – PascalCase for React components (`<Btn/>`, `<Card/>`).
- **Avoid Redundancy** – Centralize theme constants in `src/theme.js`.
- **Tree‑Shaking** – Import only needed React hooks (`import {useState} from 'react';`).
- **Minimize Dependencies** – Only `react` and `react-dom` are required.

## Performance Targets
- Bundle size ≤ 50 KB (gzip).
- First paint ≤ 300 ms on 3G.
- Use native HTML5 APIs, avoid heavy polyfills.

---
*Token‑Efficiency Tips for Future Agents*
- Re‑use existing utilities (`src/utils.js`).
- Reference documentation via relative paths (`../design_guidelines.md`).
- Keep folder depth ≤ 2 for new apps.
