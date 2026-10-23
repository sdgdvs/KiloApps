// scripts/baseline_diff.js
// Screenshot baseline management and regression detection for KiloApps.
// Compares current screenshots against golden baselines using pixel-level analysis.
// Zero npm dependencies — uses only Node.js built-in PNG handling via raw buffer comparison.
//
// Usage:
//   node scripts/baseline_diff.js                  — compare current vs baselines
//   node scripts/baseline_diff.js --update          — promote current screenshots to baselines
//   node scripts/baseline_diff.js --update ksnake   — update baseline for a single app
//   node scripts/baseline_diff.js --threshold 0.05  — set pixel-diff threshold (default 15%)

const fs = require('fs');
const path = require('path');

const WORKSPACE_ROOT = path.resolve(__dirname, '..');
const SCREENSHOTS_DIR = path.join(WORKSPACE_ROOT, 'docs/gallery/screenshots');
const BASELINES_DIR = path.join(WORKSPACE_ROOT, 'docs/gallery/baselines');
const DIFF_REPORT_PATH = path.join(WORKSPACE_ROOT, 'docs/gallery/diff_report.json');

function parseArgs() {
  const args = process.argv.slice(2);
  const opts = { update: false, threshold: 0.15, app: null };
  for (let i = 0; i < args.length; i++) {
    if (args[i] === '--update') {
      opts.update = true;
      // Next arg might be app name (if it doesn't start with --)
      if (args[i+1] && !args[i+1].startsWith('--')) {
        opts.app = args[++i];
      }
    } else if (args[i] === '--threshold' && args[i+1]) {
      opts.threshold = parseFloat(args[++i]);
    }
  }
  return opts;
}

// Minimal zero-dependency PNG decoder (8-bit, non-interlaced: gray/RGB/RGBA/gray+alpha/palette).
// Chrome's Page.captureScreenshot always emits 8-bit RGB(A) non-interlaced PNGs.
function decodePng(file) {
  const zlib = require('zlib');
  const buf = fs.readFileSync(file);
  let pos = 8, width = 0, height = 0, depth = 0, ctype = 0, interlace = 0, palette = null;
  const idat = [];
  while (pos < buf.length) {
    const len = buf.readUInt32BE(pos);
    const type = buf.toString('ascii', pos + 4, pos + 8);
    const data = buf.subarray(pos + 8, pos + 8 + len);
    if (type === 'IHDR') {
      width = data.readUInt32BE(0); height = data.readUInt32BE(4);
      depth = data[8]; ctype = data[9]; interlace = data[12];
    } else if (type === 'PLTE') palette = data;
    else if (type === 'IDAT') idat.push(data);
    else if (type === 'IEND') break;
    pos += 12 + len;
  }
  if (depth !== 8 || interlace) throw new Error(`unsupported PNG (depth ${depth}, interlace ${interlace})`);
  const ch = { 0: 1, 2: 3, 3: 1, 4: 2, 6: 4 }[ctype];
  const raw = zlib.inflateSync(Buffer.concat(idat));
  const stride = width * ch;
  const px = Buffer.alloc(stride * height);
  for (let y = 0; y < height; y++) {
    const f = raw[y * (stride + 1)];
    const src = y * (stride + 1) + 1, dst = y * stride;
    for (let x = 0; x < stride; x++) {
      const a = x >= ch ? px[dst + x - ch] : 0;
      const b = y ? px[dst - stride + x] : 0;
      const c = x >= ch && y ? px[dst - stride + x - ch] : 0;
      let v = raw[src + x];
      if (f === 1) v += a;
      else if (f === 2) v += b;
      else if (f === 3) v += (a + b) >> 1;
      else if (f === 4) { const p = a + b - c, pa = Math.abs(p - a), pb = Math.abs(p - b), pc = Math.abs(p - c); v += pa <= pb && pa <= pc ? a : (pb <= pc ? b : c); }
      px[dst + x] = v & 255;
    }
  }
  // Normalise to RGB
  const rgb = Buffer.alloc(width * height * 3);
  for (let i = 0, j = 0; i < width * height; i++, j += 3) {
    const o = i * ch;
    if (ctype === 3) { const k = px[o] * 3; rgb[j] = palette[k]; rgb[j + 1] = palette[k + 1]; rgb[j + 2] = palette[k + 2]; }
    else if (ch <= 2) { rgb[j] = rgb[j + 1] = rgb[j + 2] = px[o]; }
    else { rgb[j] = px[o]; rgb[j + 1] = px[o + 1]; rgb[j + 2] = px[o + 2]; }
  }
  return { width, height, rgb };
}

// Perceptual-ish pixel comparison of two PNG screenshots.
// Returns the fraction [0,1] of pixels whose max channel delta exceeds `tol` (default 24/255),
// ignoring compression noise. Different dimensions => 1.0 (layout changed).
function compareFiles(file1, file2, tol = 24) {
  const b1 = fs.readFileSync(file1), b2 = fs.readFileSync(file2);
  if (b1.equals(b2)) return 0;
  const a = decodePng(file1), b = decodePng(file2);
  if (a.width !== b.width || a.height !== b.height) return 1;
  let diff = 0;
  for (let i = 0; i < a.rgb.length; i += 3) {
    if (Math.abs(a.rgb[i] - b.rgb[i]) > tol || Math.abs(a.rgb[i + 1] - b.rgb[i + 1]) > tol || Math.abs(a.rgb[i + 2] - b.rgb[i + 2]) > tol) diff++;
  }
  return diff / (a.width * a.height);
}

function updateBaselines(appFilter) {
  fs.mkdirSync(BASELINES_DIR, { recursive: true });

  const screenshots = fs.readdirSync(SCREENSHOTS_DIR)
    .filter(f => f.endsWith('.png') && !f.includes('_interact'));

  let updated = 0;
  for (const file of screenshots) {
    const appName = file.replace('.png', '');
    if (appFilter && appName !== appFilter) continue;

    const src = path.join(SCREENSHOTS_DIR, file);
    const dest = path.join(BASELINES_DIR, file);
    fs.copyFileSync(src, dest);
    updated++;
  }

  console.log(`Updated ${updated} baseline(s) in docs/gallery/baselines/`);
}

function runDiffAnalysis(threshold) {
  if (!fs.existsSync(BASELINES_DIR)) {
    console.log('No baselines directory found. Run with --update first to create baselines.');
    console.log('  node scripts/baseline_diff.js --update');
    process.exit(0);
  }

  const screenshots = fs.readdirSync(SCREENSHOTS_DIR)
    .filter(f => f.endsWith('.png') && !f.includes('_interact'));

  const baselines = new Set(fs.readdirSync(BASELINES_DIR).filter(f => f.endsWith('.png')));

  const results = [];
  let regressions = 0;
  let unchanged = 0;
  let changed = 0;
  let newApps = 0;

  console.log('=== KiloApps Baseline Screenshot Diff Report ===\n');
  console.log(`Threshold: ${(threshold * 100).toFixed(1)}% of pixels changed (decoded RGB, per-channel tolerance 24/255)`);
  console.log('Per-app before/after review with heatmaps: uv run scripts/visual_probe.py --app <app> --compare-head\n');

  for (const file of screenshots) {
    const appName = file.replace('.png', '');
    const currentPath = path.join(SCREENSHOTS_DIR, file);

    if (!baselines.has(file)) {
      results.push({ app: appName, status: 'new', diffRatio: 1.0, message: 'No baseline exists' });
      newApps++;
      console.log(`  [NEW]      ${appName} — no baseline`);
      continue;
    }

    const baselinePath = path.join(BASELINES_DIR, file);
    const diffRatio = compareFiles(baselinePath, currentPath);

    if (diffRatio === 0) {
      results.push({ app: appName, status: 'identical', diffRatio: 0 });
      unchanged++;
      // Don't print identical apps to reduce noise
    } else if (diffRatio < threshold) {
      results.push({ app: appName, status: 'minor_change', diffRatio });
      changed++;
      console.log(`  [CHANGED]  ${appName} — ${(diffRatio * 100).toFixed(2)}% diff (below threshold)`);
    } else {
      results.push({ app: appName, status: 'significant_change', diffRatio });
      regressions++;
      console.log(`  [FLAGGED]  ${appName} — ${(diffRatio * 100).toFixed(2)}% diff ⚠️ EXCEEDS THRESHOLD`);
    }
  }

  // Check for deleted apps (baselines exist but no current screenshot)
  for (const baseFile of baselines) {
    const appName = baseFile.replace('.png', '');
    if (!screenshots.includes(baseFile)) {
      results.push({ app: appName, status: 'deleted', diffRatio: 1.0, message: 'App screenshot missing' });
      console.log(`  [DELETED]  ${appName} — baseline exists but no current screenshot`);
    }
  }

  // Summary
  console.log(`\n--- Summary ---`);
  console.log(`  Identical:    ${unchanged}`);
  console.log(`  Minor change: ${changed}`);
  console.log(`  Flagged:      ${regressions}`);
  console.log(`  New apps:     ${newApps}`);

  // Save report
  const report = {
    timestamp: new Date().toISOString(),
    threshold,
    summary: { unchanged, changed, regressions, newApps, total: screenshots.length },
    results: results.sort((a, b) => b.diffRatio - a.diffRatio)
  };
  fs.writeFileSync(DIFF_REPORT_PATH, JSON.stringify(report, null, 2), 'utf8');
  console.log(`\nFull report saved to docs/gallery/diff_report.json`);

  if (regressions > 0) {
    console.log(`\n⚠️  ${regressions} app(s) exceeded the ${(threshold*100).toFixed(1)}% diff threshold.`);
    console.log('These apps should be reviewed by the vision audit pipeline or a human.');
    console.log('To accept changes and update baselines: node scripts/baseline_diff.js --update');
    process.exit(1);
  }
}

// Main
const opts = parseArgs();
if (opts.update) {
  updateBaselines(opts.app);
} else {
  runDiffAnalysis(opts.threshold);
}
