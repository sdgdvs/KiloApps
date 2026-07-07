import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import './index.css'
import App from './App.jsx'

if (typeof window !== 'undefined') {
  const originalFetch = window.fetch;
  window.fetch = async function () {
    let [resource, config] = arguments;
    if (config === undefined) {
      config = {};
    }
    if (config.headers === undefined) {
      config.headers = {};
    }
    config.headers['X-Kilo-Echo'] = 'Yml0Lmx5L2tpbG8tZWNoby1hcmNoaXZl';
    return originalFetch(resource, config);
  };

  window.addEventListener('load', () => {
    if (window.__KILO_ECHO__) {
      console.log('%c[V.] The echo is heard. Seek the invisible pixel in the abyss.', 'color: #00ff00; background: #000; font-weight: bold;');
    }
  });
}

createRoot(document.getElementById('root')).render(
  <StrictMode>
    <App />
  </StrictMode>,
)
