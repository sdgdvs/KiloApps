# KiloOS UX Improvements Plan

## Coordination & Design Rules (DO NOT DELETE — required for subagent context)

**Multi-Agent System:** 4 agents operate on this repo on overlapping schedules. You are the **Shell & UX** agent.
- **Always `git pull`** before reading or editing files. Other agents push changes between your turns.
- **Plan file ownership — only edit YOUR file (`kiloos_ux_plan.md`).** Read but NEVER edit:
  - `app_work_plan.md` (Builder agent), `app_fix_plan.md` (QA agent), `game_content_plan.md` (Games agent)
- **Files you OWN (exclusive edit rights):**
  - `KiloOS/src/App.jsx` — all code EXCEPT the APPS array entries, which other agents add to
  - `KiloOS/src/index.css`, `KiloOS/src/main.jsx`, `KiloOS/index.html`
- **When merging conflicts in App.jsx:** other agents may have added APPS array entries since your last turn. Always prefer the remote version for APPS entries, then re-apply your shell/UX changes on top.
- **Do NOT** create or modify individual app files (`K*/` directories, `KiloOS/public/apps/`).
- **Do NOT** add new apps to the APPS array — that's the Builder/Games agents' job.
- **Do NOT** add hidden behaviors, easter eggs, or ARG elements.
- **Version bumping:** After functional changes, bump the patch version in `KiloOS/package.json`.

**Design System (maintain strictly):**
- **Palette:** Background `hsl(210, 15%, 12%)`, Card `hsl(210, 15%, 20%)`, Primary `hsl(200, 85%, 60%)`, Secondary `hsl(30, 85%, 60%)`
- **Typography:** Google Font "Inter" (weights 400, 600). `font-family: 'Inter', system-ui, sans-serif;`
- **Animations:** CSS transitions `0.15s ease`. `@keyframes fadeIn` for entry. Keep animation code under 300 bytes. Glassmorphism (`backdrop-filter`) only behind `@media (prefers-reduced-motion: no-preference)`.
- **Layout:** Flexbox/Grid, max-width 800px for desktop. No external CSS frameworks allowed.
- **Start Menu:** Two-column glassmorphic layout with user avatar and categorized app folders.
- **Taskbar:** Horizontal scroll with entrance animations and active-state indicators.

**KiloOS Build Targets:**
- HTML bundle ≤ 50KB gzipped, EXE ≤ 150KB, Load time < 500ms, First paint < 300ms on 3G.
- **Always verify:** `cd KiloOS && npm run build` after any changes. Fix failures before committing.
- Only `react` and `react-dom` are allowed dependencies. No external CSS frameworks.

- **CI/CD:** Every push to `main` triggers GitHub Actions → Firebase deploy to `kiloapps.web.app`.
- **Conflict resolution:** If `git push` fails → `git pull --rebase` → resolve conservatively (prefer remote APPS entries) → push again.
- **Logging discipline:** Keep this plan file concise. Brief notes per completed item. Do NOT dump file contents.

---

## Current Status
- Glassmorphism isolated to prefers-reduced-motion media query.
- Improved window minimize animation.
- Fixed desktop icon selected state styling and added window drag visual feedback.

## Priorities

### 1. Window Management Polish
- Consider adding snap-to-edge highlights or transparent overlays when dragging windows to the edges.

## Completed Items
- Theme Consistency: Fixed broken desktop icon selected state styling by unifying `.icon-img` with `.desktop-icon img` across `index.css`.
- Window Management Polish: Added visual feedback when dragging windows by lowering opacity to `0.85` via `.xp-window.dragging` class to improve responsiveness feel.
- Window Management Polish: Added smooth entrance animations for windows and updated Taskbar clock interactions. Added active interaction states to the Start Menu button.
- Theme Consistency: Centralized desktop icon styling in `index.css`, removed duplicate/inline hover states, adjusted sizing, and added subtle drop-shadows to ensure all app icons and labels fit well.
- Theme Consistency: Improved desktop icon labels to use standard `Inter` weight 400 with a better text-shadow and background on hover/select. Removed inline styles from `folder-content` icons in `App.jsx` to inherit the robust `index.css` classes.
- Window Management Polish: Updated window control buttons (minimize, maximize, close) to a macOS-style stoplight design with standard hover states, replacing the raw text symbols with more consistent styling.
- Window Management Polish: Improved minimize animations to scale down smoother towards the taskbar.
- Theme Consistency: Ensured glassmorphism (backdrop-filter) is only applied inside the `prefers-reduced-motion: no-preference` media query for performance and accessibility.
- Start Menu Polish: Implemented dynamic folder navigation within the start menu to prevent the app list from becoming an unreadable, infinitely scrolling dump. Added 'Back' button and folder drilling.
- Taskbar Enhancements: Added horizontal scrolling support on wheel and improved the active app indicator with a glowing bottom border and `flex-shrink: 0` to prevent unreadable tabs.
- Start Menu Polish: Improved start menu open animation by using `@keyframes` on mount, and replaced inline spacing/alignment styles with proper CSS classes.
- Window Management: Added a subtle dimming effect (`filter: brightness(0.85)`) to inactive windows to better highlight the active window.
- Start Menu: Enhanced search bar styling with glassmorphism and improved hover states for items.
- Taskbar: Added missing system tray icons next to the clock.
- Window Management: Transition easing and timing updated to 0.15s ease to make maximizing and snapping windows feel more fluid.
- Responsive Design: Added media queries to resize desktop icons and start menu gracefully on smaller screens.
