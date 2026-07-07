# App Integration Guide for MicrOS Minimal Apps

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
1. **Create a new directory** under the MicrOS root, e.g. `micro‑calc`.
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
      <h1>Welcome to MicrOS</h1>
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
    <title>MicrOS – Minimal App</title>
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
