---
trigger: always_on
name: quality-standards
description: >-
  Explicit typing, error-boundary, storage-safety, and network-resilience
  standards that ALL KiloApps agents must enforce on every file they touch.
  Non-compliance is a blocking failure — do not commit or push without fixing.
---

# KiloApps Quality Standards (always_on)

## 1. Explicit Typing via JSDoc

All public functions, state shapes, and message-payload objects in
`App.jsx` and agent-facing utilities MUST be annotated with JSDoc `@typedef`
or `@param`/`@returns` tags. Implicit `any`-equivalent loose objects are
banned when the shape is knowable.

```js
/** @typedef {{ id: string, title: string, url: string, icon: string,
 *              w: number, h: number, folder: string,
 *              exeUrl: string|null, zIndex?: number }} AppDef */

/** @param {AppDef} app */
function openApp(app) { ... }
```

## 2. Storage Safety — Mandatory try/catch

Every `localStorage.getItem` and `localStorage.setItem` call MUST be
wrapped. Use the standard helper pattern:

```js
function safeGet(key, fallback) {
  try { return JSON.parse(localStorage.getItem(key)) ?? fallback; }
  catch { return fallback; }
}
function safeSet(key, value) {
  try { localStorage.setItem(key, JSON.stringify(value)); }
  catch (e) { console.warn('[KiloApp] Storage write failed:', e.message); }
}
```

Bare `localStorage.getItem(key)` without null/parse guard is a type-decay
violation. Bare `localStorage.setItem` without try/catch is an
offline-resilience violation.

## 3. Network Safety — Mandatory Error Handling

All `fetch()` calls must be wrapped to handle offline, CORS, and HTTP errors:

```js
async function safeFetch(url, opts = {}) {
  try {
    const r = await fetch(url, opts);
    if (!r.ok) throw new Error(`HTTP ${r.status}`);
    return r;
  } catch (e) {
    showToast(`Network error: ${e.message}`, 'error');
    return null;
  }
}
```

Callers must null-check the return: `if (!r) return;`

Unhandled promise rejections from `fetch` are a P2 bug — treat as blocking.

## 4. Loading & Empty States

Every data-fetching or async operation in a UI app MUST render a visible
loading indicator and a graceful empty/error state:

- Loading: show a spinner, skeleton, or `Loading…` message.
- Error: show a toast or inline error with a retry affordance.
- Empty: show a "No items yet" placeholder — never a blank pane.

Happy-path-only renders (data assumed non-null) are a P2 bug.

## 5. Null / Undefined Defensive Checks

All `APPS.find()`, `querySelector()`, and JSON-parsed object accesses MUST
include a null guard before property access:

```js
// ❌ BANNED:
const app = APPS.find(a => a.id === id);
openWindow(app.url);   // crashes if app is undefined

// ✅ REQUIRED:
const app = APPS.find(a => a.id === id);
if (!app) { console.warn(`[KiloOS] Unknown app: ${id}`); return; }
openWindow(app.url);
```

## 6. Lifecycle Cleanup Checklist

Before committing any file that touches timers or event listeners, verify:

- [ ] Every `setInterval` has a paired `clearInterval` in cleanup.
- [ ] Every `setTimeout` used for recurring work has been replaced with `setInterval` (or rAF) + cleanup.
- [ ] Every `requestAnimationFrame` loop has a `cancelAnimationFrame` guard.
- [ ] Every `addEventListener` on `window` or `document` has a paired `removeEventListener` in cleanup or on `visibilitychange`/`pagehide`.
- [ ] No `WebSocket` or `EventSource` connection is opened without a `.close()` on component unmount or page hide.

## 7. ARG Integrity Standard

Any code path that is part of the ARG narrative layer must include the
comment `// ARG: intentional` on the relevant line. This protects it from
being removed by cleanup passes and signals to reviewers that the
"anomalous" behaviour is by design.
