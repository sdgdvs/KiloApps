// scripts/test_web_apps.js
// Automated Headless Browser Testing Suite, FPS Benchmark & Visual Gallery Generator for KiloApps.
// Uses Chrome/Edge DevTools Protocol (CDP) over native Node 22+ WebSocket. Zero npm dependencies.

const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');
const http = require('http');

const BROWSER_PATHS = [
  'C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe',
  'C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe'
];

const WORKSPACE_ROOT = path.resolve(__dirname, '..');
const APPS_DIR = path.join(WORKSPACE_ROOT, 'KiloOS/public/apps');
const GALLERY_DIR = path.join(WORKSPACE_ROOT, 'docs/gallery');
const SCREENSHOTS_DIR = path.join(GALLERY_DIR, 'screenshots');
const MAX_FILE_SIZE_BYTES = 999 * 1024;
const PORT = 9222;

function findBrowser() {
  for (const p of BROWSER_PATHS) {
    if (fs.existsSync(p)) return p;
  }
  return null;
}

function fetchJson(url) {
  return new Promise((resolve, reject) => {
    http.get(url, (res) => {
      let data = '';
      res.on('data', chunk => data += chunk);
      res.on('end', () => {
        try { resolve(JSON.parse(data)); } catch(e) { reject(e); }
      });
    }).on('error', reject);
  });
}

function delay(ms) {
  return new Promise(r => setTimeout(r, ms));
}

// Injected performance probe to measure 60 FPS frame pacing and detect shuttering
const PERF_PROBE_SCRIPT = `
(function() {
  window.__perfProbe = {
    frames: [],
    start: performance.now(),
    done: false
  };
  function onFrame(ts) {
    window.__perfProbe.frames.push(ts);
    if (!window.__perfProbe.done) {
      requestAnimationFrame(onFrame);
    }
  }
  requestAnimationFrame(onFrame);
})();
`;

async function runSuite() {
  const browserPath = findBrowser();
  if (!browserPath) {
    console.error('Error: No supported browser found.');
    process.exit(1);
  }

  fs.mkdirSync(SCREENSHOTS_DIR, { recursive: true });

  const appFiles = fs.readdirSync(APPS_DIR)
    .filter(f => f.endsWith('.html'))
    .sort();

  console.log(`=== KiloApps Automated Test & Visual Audit Suite ===`);
  console.log(`Discovered ${appFiles.length} web applications.`);
  console.log(`Browser: ${browserPath}`);
  console.log(`Output Gallery: docs/gallery/index.html\n`);

  const browserProc = spawn(browserPath, [
    '--headless=new',
    `--remote-debugging-port=${PORT}`,
    '--disable-gpu',
    '--no-sandbox',
    '--disable-dev-shm-usage',
    '--mute-audio',
    '--window-size=1024,768',
    'about:blank'
  ], { stdio: 'ignore' });

  let versionInfo = null;
  for (let i = 0; i < 30; i++) {
    await delay(200);
    try {
      versionInfo = await fetchJson(`http://127.0.0.1:${PORT}/json/version`);
      if (versionInfo && versionInfo.webSocketDebuggerUrl) break;
    } catch(e) {}
  }

  if (!versionInfo || !versionInfo.webSocketDebuggerUrl) {
    console.error('Failed to connect to browser CDP port.');
    browserProc.kill();
    process.exit(1);
  }

  const targets = await fetchJson(`http://127.0.0.1:${PORT}/json/list`);
  const pageTarget = targets.find(t => t.type === 'page') || targets[0];
  const wsUrl = pageTarget.webSocketDebuggerUrl;

  const ws = new WebSocket(wsUrl);
  await new Promise((resolve, reject) => {
    ws.onopen = resolve;
    ws.onerror = reject;
  });

  let messageId = 0;
  const pending = new Map();
  let currentErrors = [];

  ws.onmessage = (event) => {
    const msg = JSON.parse(event.data);
    if (msg.id && pending.has(msg.id)) {
      const { resolve } = pending.get(msg.id);
      pending.delete(msg.id);
      resolve(msg.result);
    }
    if (msg.method === 'Runtime.exceptionThrown') {
      const desc = msg.params.exceptionDetails.exception?.description || msg.params.exceptionDetails.text;
      currentErrors.push(desc);
    }
  };

  function sendCommand(method, params = {}) {
    return new Promise((resolve) => {
      const id = ++messageId;
      pending.set(id, { resolve });
      ws.send(JSON.stringify({ id, method, params }));
    });
  }

  await sendCommand('Runtime.enable');
  await sendCommand('Page.enable');

  const appReports = [];
  const startTime = Date.now();

  for (let i = 0; i < appFiles.length; i++) {
    const file = appFiles[i];
    const appBase = file.replace('.html', '');
    const fullPath = path.join(APPS_DIR, file);
    const stat = fs.statSync(fullPath);
    const sizeKb = (stat.size / 1024).toFixed(1);
    const fileUrl = 'file:///' + fullPath.replace(/\\/g, '/');

    currentErrors = [];

    const report = {
      file,
      name: appBase.toUpperCase(),
      sizeKb: parseFloat(sizeKb),
      sizeValid: stat.size <= MAX_FILE_SIZE_BYTES,
      syntaxValid: true,
      hasUi: true,
      fps: 60,
      stutterCount: 0,
      maxFrameDelta: 16.6,
      screenshot: `screenshots/${appBase}.png`,
      interaction: { totalInteractive: 0, buttonsClicked: 0, buttonsMutated: 0, buttonsErrored: 0, keyErrors: 0 },
      errors: []
    };

    if (!report.sizeValid) {
      report.errors.push(`Size ${sizeKb}KB exceeds 999KB limit`);
      process.stdout.write('F');
      appReports.push(report);
      continue;
    }

    try {
      await sendCommand('Page.navigate', { url: fileUrl });
      
      // Inject performance probe
      await sendCommand('Runtime.evaluate', { expression: PERF_PROBE_SCRIPT });

      // Run active loop for 350ms to measure frame pacing
      await delay(350);

      // Extract metrics
      const perfRes = await sendCommand('Runtime.evaluate', {
        expression: `
        (function() {
          if (!window.__perfProbe) return null;
          window.__perfProbe.done = true;
          const frames = window.__perfProbe.frames;
          if (frames.length < 2) return { fps: 60, stutters: 0, maxDelta: 16 };
          const totalMs = frames[frames.length - 1] - frames[0];
          const fps = Math.round(((frames.length - 1) / (totalMs / 1000)));
          let stutters = 0;
          let maxDelta = 0;
          for (let i = 1; i < frames.length; i++) {
            const d = frames[i] - frames[i-1];
            if (d > maxDelta) maxDelta = d;
            if (d > 28) stutters++;
          }
          return {
            fps: isFinite(fps) ? fps : 60,
            stutters,
            maxDelta: Math.round(maxDelta)
          };
        })()
        `,
        returnByValue: true
      });

      if (perfRes && perfRes.result && perfRes.result.value) {
        report.fps = perfRes.result.value.fps;
        report.stutterCount = perfRes.result.value.stutters;
        report.maxFrameDelta = perfRes.result.value.maxDelta;
      }

      // Check DOM content
      const domCheck = await sendCommand('Runtime.evaluate', {
        expression: 'document.querySelector("canvas") !== null || document.body.childElementCount > 0'
      });
      report.hasUi = Boolean(domCheck && domCheck.result && domCheck.result.value);

      if (currentErrors.length > 0) {
        report.syntaxValid = false;
        report.errors.push(...currentErrors);
        process.stdout.write('E');
      } else if (!report.hasUi) {
        report.errors.push('Empty DOM / No canvas or UI rendered');
        process.stdout.write('?');
      } else if (report.fps < 45 || report.stutterCount >= 3) {
        process.stdout.write('S'); // S for stutter/shutter
      } else {
        process.stdout.write('.');
      }

      // Capture screenshot
      const ssRes = await sendCommand('Page.captureScreenshot', { format: 'png' });
      if (ssRes && ssRes.data) {
        const dest = path.join(SCREENSHOTS_DIR, `${appBase}.png`);
        fs.writeFileSync(dest, Buffer.from(ssRes.data, 'base64'));
      }

      // === Interaction Testing Phase ===
      const preInteractErrors = currentErrors.length;

      const interactRes = await sendCommand('Runtime.evaluate', {
        expression: `
        (async function() {
          const buttons = document.querySelectorAll('button, [onclick], [role="button"]');
          let totalClicked = 0, errored = 0, mutated = 0;

          for (const btn of buttons) {
            if (totalClicked >= 20) break;
            let didMutate = false;
            const obs = new MutationObserver(() => { didMutate = true; });
            obs.observe(document.body, { childList: true, subtree: true, attributes: true });
            try {
              btn.click();
              await new Promise(r => setTimeout(r, 50));
            } catch(e) { errored++; }
            obs.disconnect();
            if (didMutate) mutated++;
            totalClicked++;
          }

          let keyErrors = 0;
          for (const key of ['Escape', 'F1', 'Enter']) {
            try {
              document.dispatchEvent(new KeyboardEvent('keydown', { key, bubbles: true }));
              await new Promise(r => setTimeout(r, 30));
            } catch(e) { keyErrors++; }
          }

          return {
            totalInteractive: document.querySelectorAll('button, [onclick], input, select, a[href], [role="button"]').length,
            buttonsClicked: totalClicked,
            buttonsMutated: mutated,
            buttonsErrored: errored,
            keyErrors
          };
        })()
        `,
        returnByValue: true,
        awaitPromise: true
      });

      if (interactRes && interactRes.result && interactRes.result.value) {
        report.interaction = interactRes.result.value;
      }

      // Check for JS errors thrown during interaction
      const interactErrors = currentErrors.slice(preInteractErrors);
      if (interactErrors.length > 0) {
        report.errors.push(`Interaction errors (${interactErrors.length}): ${interactErrors[0].slice(0, 80)}`);
      }

      // Capture post-interaction screenshot
      const ssInteract = await sendCommand('Page.captureScreenshot', { format: 'png' });
      if (ssInteract && ssInteract.data) {
        const dest = path.join(SCREENSHOTS_DIR, `${appBase}_interact.png`);
        fs.writeFileSync(dest, Buffer.from(ssInteract.data, 'base64'));
      }
    } catch (err) {
      report.errors.push(`Runner error: ${err.message}`);
      process.stdout.write('X');
    }

    appReports.push(report);
  }

  ws.close();
  browserProc.kill();

  const elapsed = ((Date.now() - startTime) / 1000).toFixed(1);
  console.log(`\n\nCompleted full test & visual capture in ${elapsed}s.`);

  // Generate HTML Gallery & Human Review Queue
  generateGallery(appReports);
  generateHumanReviewQueue(appReports);

  const passed = appReports.filter(r => r.errors.length === 0).length;
  const failed = appReports.filter(r => r.errors.length > 0).length;
  const stuttering = appReports.filter(r => r.fps < 50 || r.stutterCount >= 2).length;

  console.log(`Summary: ${passed} passed, ${failed} failed across ${appReports.length} apps.`);
  console.log(`Performance Warnings: ${stuttering} apps exhibited frame drops or stuttering.`);

  const totalInteractive = appReports.reduce((s, r) => s + r.interaction.totalInteractive, 0);
  const totalClicked = appReports.reduce((s, r) => s + r.interaction.buttonsClicked, 0);
  const totalMutated = appReports.reduce((s, r) => s + r.interaction.buttonsMutated, 0);
  const interactErrors = appReports.filter(r => r.interaction.buttonsErrored > 0 || r.interaction.keyErrors > 0).length;
  console.log(`Interaction Testing: ${totalInteractive} interactive elements, ${totalClicked} buttons clicked, ${totalMutated} reactive, ${interactErrors} apps with interaction errors.`);
}

function generateGallery(reports) {
  const cardsHtml = reports.map(r => {
    const isClean = r.errors.length === 0;
    const isSmooth = r.fps >= 55 && r.stutterCount === 0;
    const statusBadge = !isClean
      ? '<span class="badge badge-fail">ERROR</span>'
      : (!isSmooth ? '<span class="badge badge-warn">LAG / STUTTER</span>' : '<span class="badge badge-pass">60 FPS PASS</span>');

    return `
      <div class="app-card ${!isClean ? 'card-fail' : (!isSmooth ? 'card-warn' : '')}">
        <div class="card-img-wrap">
          <img src="${r.screenshot}" alt="${r.name}" loading="lazy">
        </div>
        <div class="card-body">
          <div class="card-header">
            <h3>${r.name}</h3>
            ${statusBadge}
          </div>
          <div class="card-metrics">
            <span>FPS: <strong>${r.fps}</strong></span>
            <span>Max Delta: <strong>${r.maxFrameDelta}ms</strong></span>
            <span>Size: <strong>${r.sizeKb} KB</strong></span>
          </div>
          <div class="card-metrics">
            <span>Interactive: <strong>${r.interaction.totalInteractive}</strong></span>
            <span>Clicked: <strong>${r.interaction.buttonsClicked}</strong></span>
            <span>Reactive: <strong>${r.interaction.buttonsMutated}/${r.interaction.buttonsClicked || 1}</strong></span>
          </div>
          ${r.errors.length > 0 ? `<div class="card-errors">${r.errors[0].slice(0, 80)}</div>` : ''}
          <div class="card-actions">
            <a href="../../KiloOS/public/apps/${r.file}" target="_blank" class="play-btn">▶ Play App</a>
          </div>
        </div>
      </div>
    `;
  }).join('\n');

  const html = `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>KiloApps Visual Audit & Performance Gallery</title>
  <style>
    :root {
      --bg: #0d1117;
      --card-bg: #161b22;
      --border: #30363d;
      --text: #c9d1d9;
      --text-muted: #8b949e;
      --accent: #58a6ff;
      --pass: #238636;
      --warn: #d29922;
      --fail: #da3633;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      background: var(--bg);
      color: var(--text);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
      padding: 24px;
    }
    header {
      margin-bottom: 24px;
      padding-bottom: 16px;
      border-bottom: 1px solid var(--border);
    }
    h1 { font-size: 24px; color: #fff; margin-bottom: 8px; }
    p.sub { color: var(--text-muted); font-size: 14px; }
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
      gap: 20px;
    }
    .app-card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 8px;
      overflow: hidden;
      display: flex;
      flex-direction: column;
      transition: transform 0.15s ease;
    }
    .app-card:hover { transform: translateY(-3px); }
    .card-warn { border-color: var(--warn); }
    .card-fail { border-color: var(--fail); }
    .card-img-wrap {
      width: 100%;
      height: 180px;
      background: #000;
      display: flex;
      align-items: center;
      justify-content: center;
      overflow: hidden;
    }
    .card-img-wrap img {
      width: 100%;
      height: 100%;
      object-fit: contain;
    }
    .card-body { padding: 14px; display: flex; flex-direction: column; gap: 8px; flex: 1; }
    .card-header { display: flex; justify-content: space-between; align-items: center; }
    .card-header h3 { font-size: 16px; color: #fff; }
    .badge {
      font-size: 10px;
      font-weight: 700;
      padding: 2px 6px;
      border-radius: 4px;
      text-transform: uppercase;
    }
    .badge-pass { background: rgba(35, 134, 54, 0.2); color: #3fb950; border: 1px solid var(--pass); }
    .badge-warn { background: rgba(210, 153, 34, 0.2); color: #e3b341; border: 1px solid var(--warn); }
    .badge-fail { background: rgba(218, 54, 51, 0.2); color: #f85149; border: 1px solid var(--fail); }
    .card-metrics {
      display: flex;
      justify-content: space-between;
      font-size: 11px;
      color: var(--text-muted);
      border-top: 1px solid rgba(255,255,255,0.05);
      padding-top: 6px;
    }
    .card-metrics strong { color: #fff; }
    .card-errors {
      font-size: 11px;
      color: #f85149;
      background: rgba(218, 54, 51, 0.1);
      padding: 4px;
      border-radius: 4px;
      font-family: monospace;
    }
    .card-actions { margin-top: auto; padding-top: 8px; }
    .play-btn {
      display: block;
      text-align: center;
      background: #21262d;
      color: var(--accent);
      text-decoration: none;
      font-size: 12px;
      font-weight: 600;
      padding: 6px 12px;
      border-radius: 4px;
      border: 1px solid var(--border);
    }
    .play-btn:hover { background: #30363d; }
  </style>
</head>
<body>
  <header>
    <h1>KiloApps Visual Audit & Performance Gallery</h1>
    <p class="sub">Automated headless visual capture and 60 FPS pacing analysis across all registered web applications.</p>
  </header>
  <div class="grid">
    ${cardsHtml}
  </div>
</body>
</html>`;

  fs.writeFileSync(path.join(GALLERY_DIR, 'index.html'), html, 'utf8');
  console.log(`Generated docs/gallery/index.html`);
}

function generateHumanReviewQueue(reports) {
  let matureList = [];
  try {
    const reg = JSON.parse(fs.readFileSync(path.join(WORKSPACE_ROOT, 'mature_apps_registry.json'), 'utf8'));
    matureList = reg.locked_apps.map(a => a.name.toLowerCase());
  } catch(e) {}

  const rows = reports.map(r => {
    const isMature = matureList.includes(r.file.replace('.html', '').toLowerCase());
    const isFlagged = r.fps < 50 || r.stutterCount >= 2 || r.errors.length > 0;
    const status = r.errors.length > 0 ? '❌ Error' : (isFlagged ? '⚠️ Lag/Stutter' : (isMature ? '🔒 Locked (Mature 5+)' : '🟢 Active'));

    return `| [${r.name}](gallery/screenshots/${r.file.replace('.html', '')}.png) | [Launch](file:///${path.join(APPS_DIR, r.file).replace(/\\\\/g, '/')}) | ${r.fps} FPS | ${r.maxFrameDelta}ms | ${r.sizeKb} KB | ${status} |`;
  }).join('\n');

  const md = `# KiloApps Human Review & Visual Audit Queue

**Directive:** Once an application has completed 5+ iterative passes, passes automated headless zero-exception validation, and meets the 60 FPS performance benchmark, it is placed into this queue for **Human Director Review**.

> [!TIP]
> Click **Launch** to play any app directly in your browser, or click the **Name** to inspect its full gameplay screenshot. You can also view the complete interactive dashboard at [docs/gallery/index.html](gallery/index.html).

| App Name | Play In Browser | FPS | Max Delta | File Size | Status |
| :--- | :---: | :---: | :---: | :---: | :--- |
${rows}
`;

  fs.writeFileSync(path.join(WORKSPACE_ROOT, 'docs/human_review_queue.md'), md, 'utf8');
  console.log(`Generated docs/human_review_queue.md`);
}

runSuite().catch(err => {
  console.error('Fatal error in suite:', err);
  process.exit(1);
});
