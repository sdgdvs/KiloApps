// scripts/baseline_diff.js
// Screenshot baseline management and regression detection for KiloApps.
// Compares current screenshots against golden baselines using pixel-level analysis.
// Zero npm dependencies — uses only Node.js built-in PNG handling via raw buffer comparison.
//
// Usage:
//   node scripts/baseline_diff.js                  — compare current vs baselines
//   node scripts/baseline_diff.js --update          — promote current screenshots to baselines
//   node scripts/baseline_diff.js --update ksnake   — update baseline for a single app
//   node scripts/baseline_diff.js --threshold 0.05  — set pixel-diff threshold (default 5%)

const fs = require('fs');
const path = require('path');

const WORKSPACE_ROOT = path.resolve(__dirname, '..');
const SCREENSHOTS_DIR = path.join(WORKSPACE_ROOT, 'docs/gallery/screenshots');
const BASELINES_DIR = path.join(WORKSPACE_ROOT, 'docs/gallery/baselines');
const DIFF_REPORT_PATH = path.join(WORKSPACE_ROOT, 'docs/gallery/diff_report.json');

function parseArgs() {
  const args = process.argv.slice(2);
  const opts = { update: false, threshold: 0.05, app: null };
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

// Simple byte-level comparison of two PNG files.
// Returns a diff ratio [0,1] where 0 = identical, 1 = completely different.
// This is a raw byte comparison — not perceptual, but effective for detecting
// whether anything changed at all. For perceptual analysis, use vision_audit.py.
function compareFiles(file1, file2) {
  const buf1 = fs.readFileSync(file1);
  const buf2 = fs.readFileSync(file2);

  // If files are identical, fast path
  if (buf1.equals(buf2)) return 0;

  // Compare byte-by-byte up to the shorter length
  const len = Math.min(buf1.length, buf2.length);
  let diffBytes = 0;

  for (let i = 0; i < len; i++) {
    if (buf1[i] !== buf2[i]) diffBytes++;
  }

  // Account for size difference
  diffBytes += Math.abs(buf1.length - buf2.length);
  const totalBytes = Math.max(buf1.length, buf2.length);

  return diffBytes / totalBytes;
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
  console.log(`Threshold: ${(threshold * 100).toFixed(1)}% pixel difference\n`);

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
