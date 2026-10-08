---
trigger: always_on
name: architecture
description: >-
  Structural invariants, directory layout, and state-isolation rules
  that ALL KiloApps agents must obey on every run. Violations are
  blocking — do not commit or push without compliance.
---

# KiloApps Architecture Rules (always_on)

## 1. State Isolation

- **No direct DOM mutations from App.jsx business logic.** All window state
  (`openApps`, `vfs`, `zIndexCounter`) must be read and written exclusively
  through `useState` / `useRef` inside `App.jsx`.  Web app iframes (in
  `public/apps/`) must communicate with the shell ONLY via `postMessage`.
- **postMessage handlers must validate origin first:**
  ```js
  if (event.origin !== window.location.origin) return;
  ```
- **zIndexCounter mutations must use the functional updater form:**
  ```js
  setZIndexCounter(prev => prev + 1);
  ```
  Direct reference (`zIndexCounter + 1`) is banned because stale-closure
  race conditions collapse the focus stack on rapid multi-open.

## 2. Lifecycle Cleanup — Hard Requirements

Every `setInterval`, `setTimeout`, `requestAnimationFrame` loop,
`addEventListener`, `WebSocket`, `MutationObserver`, or `IntersectionObserver`
created inside a React `useEffect` MUST return a cleanup function:

```js
useEffect(() => {
  const id = setInterval(fn, ms);
  return () => clearInterval(id);     // REQUIRED
}, [deps]);
```

Individual HTML app files MUST guard long-running loops with `visibilitychange`:
```js
document.addEventListener('visibilitychange', () =>
  document.hidden ? stopLoop() : startLoop());
```

## 3. Repository Layout Invariants

| Path | Rule |
|---|---|
| `KiloOS/src/App.jsx` | KiloOS shell only. No app-specific business logic. |
| `KiloOS/public/apps/k<name>.html` | One self-contained file per app. No external script imports allowed. |
| `.agents/skills/` | Skill SKILL.md files only. Do NOT add application code here. |
| `.agents/rules/` | This directory. Rules files only. |
| `archive/` | Immutable. Do NOT modify archived files. |

## 4. APPS Registry Integrity

- Every entry in the `APPS` array in `App.jsx` must include all required
  fields: `id`, `title`, `url`, `icon`, `w`, `h`, `folder`.
- `exeUrl` may be `null` but must be explicitly present.
- The validation warning block (`REQUIRED_FIELDS.forEach`) must remain in
  `App.jsx` and must NOT be removed.

## 5. Boundary Violations — Banned Patterns

The following patterns are categorically banned:

```js
// ❌ BANNED: iframe app directly mutating shell localStorage keys
localStorage.setItem('kiloOS_openApps', ...)

// ❌ BANNED: App.jsx reaching into app-specific storage
localStorage.getItem('kcalc_history')

// ❌ BANNED: postMessage without origin check
window.addEventListener('message', e => { dispatch(e.data); });

// ❌ BANNED: un-guarded setInterval with no cleanup return
useEffect(() => { setInterval(tick, 1000); }, []);
```

## 6. Size Ceiling

No individual `k<name>.html` file may exceed **999 KB** (uncompressed).
The check is enforced by `python scripts/check_sizes.py` — run it before
every commit that modifies app HTML files.

## 7. ARG Layer Protection

ARG narrative elements (`.kilo-echo.json`, `window.__KILO_ECHO__`,
`kquarantine` app, `argAppHistory` ref) are **intentional** and must NOT
be removed, refactored away, or altered in logic. Mark any concern with a
`// ARG: intentional` comment and leave it.
