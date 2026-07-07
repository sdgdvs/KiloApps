# App Integration Guide for KiloOS Minimal Apps

## Folder Layout (Token‑Efficient)
```
micros/
├─ src/               # React source (ES modules)
│   ├─ main.jsx       # Entry point (creates root component)
│   ├─ App.jsx        # Minimal UI component
│   └─ theme.js       # Shared theme constants (colors, fonts)
├─ public/            # Static assets copied as‑is
│   └─ index.html     # Basic HTML shell (loads built JS)
├─ vite.config.js     # Vite config – keep unchanged for new apps
└─ package.json       # Project metadata (private, dependencies)
```
* Keep folder depth ≤ 2 to minimise token count for agents.
* Use **kebab‑case** for directory names (e.g., `micro‑calc`).
* All source files should be under `src/`.

## Adding a New Minimal App
1. **Create a new directory** under the KiloOS root, e.g. `micro‑calc`.
2. Copy the *template* files:
   - `src/main.jsx`
   - `src/App.jsx`
   - `src/theme.js`
   - `public/index.html`
3. Update `package.json` if you need a unique name (optional, keep private).
4. Run `npm run dev` inside the new directory to develop.
5. When ready, run `npm run build` – Vite outputs to `dist/`.
6. Package the output with Crinkler (see **build_process.md**).

## Code Template Overview
### `src/main.jsx`
```jsx
import React from 'react';
import ReactDOM from 'react-dom/client';
import App from './App';
import './theme.js'; // inject CSS variables

const root = ReactDOM.createRoot(document.getElementById('root'));
root.render(<App />);
```
### `src/App.jsx`
```jsx
import React from 'react';

export default function App() {
  return (
    <div className="app-card">
      <h1>Welcome to KiloOS</h1>
      <p>Minimalist app scaffold.</p>
    </div>
  );
}
```
### `src/theme.js`
```js
import './App.css'; // basic styles shared across apps
```
### `public/index.html`
```html
<!DOCTYPE html>
<html lang="en" data-theme="dark">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width,initial-scale=1" />
    <title>KiloOS – Minimal App</title>
    <link rel="preconnect" href="https://fonts.googleapis.com" />
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600&display=swap" rel="stylesheet" />
  </head>
  <body>
    <div id="root"></div>
    <script type="module" src="/src/main.jsx"></script>
  </body>
</html>
```
---
*All CSS utilities are defined in `src/App.css` (see **design_guidelines.md** for palette & micro‑animations).*

## App Organization (Folders)
To keep the KiloOS desktop and start menu organized, new apps must be categorized into one of the designated folders.

When adding a new app to `APPS` in `App.jsx`, ensure you assign the correct folder string to the `folder` property:

```js
const APPS = [
  ...
  { 
    id: 'mynewapp', 
    title: 'My New App', 
    url: '/apps/mynewapp.html', 
    icon: '/assets/icons/mynewapp.ico', 
    w: 400, 
    h: 300, 
    folder: 'System' // <-- Must be one of the defined categories
  }
];
```

**Available Folders / Categories:**
- `'System'` (System Tools)
- `'Media'` (Media & Arts)
- `'Office'` (Office)
- `'Games'` (Games)
- `'Network'` (Network)
- `'Dev'` (Development)

If an app does not fit neatly into one of these, default it to `'System'` or the most relevant alternative, but **do not** create a new folder unless absolutely necessary.
