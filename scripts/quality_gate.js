// scripts/quality_gate.js
// Unified quality gate orchestrator for KiloApps.
// Runs all three automation layers in sequence:
//   1. Headless browser testing (errors, FPS, screenshots, interaction testing)
//   2. Baseline screenshot diffing (regression detection)
//   3. Vision model audit (optional, requires GEMINI_API_KEY)
//
// Usage:
//   node scripts/quality_gate.js              — full pipeline
//   node scripts/quality_gate.js --skip-vision — skip vision audit (no API key)
//   node scripts/quality_gate.js --update-baselines — accept current screenshots as baselines
//
// Exit codes:
//   0 = all gates passed
//   1 = one or more gates failed

const { execSync, spawnSync } = require('child_process');
const path = require('path');
const fs = require('fs');

const ROOT = path.resolve(__dirname, '..');
const SCRIPTS = __dirname;

function run(label, command) {
  console.log(`\n${'='.repeat(60)}`);
  console.log(`  GATE: ${label}`);
  console.log(`${'='.repeat(60)}\n`);

  try {
    execSync(command, { cwd: ROOT, stdio: 'inherit', timeout: 600000 }); // 10min timeout
    console.log(`\n  ✅ ${label} — PASSED\n`);
    return true;
  } catch (err) {
    console.log(`\n  ❌ ${label} — FAILED (exit code ${err.status})\n`);
    return false;
  }
}

function main() {
  const args = process.argv.slice(2);
  const skipVision = args.includes('--skip-vision');
  const updateBaselines = args.includes('--update-baselines');

  console.log('╔══════════════════════════════════════════════════════════╗');
  console.log('║          KiloApps Unified Quality Gate Pipeline         ║');
  console.log('╚══════════════════════════════════════════════════════════╝');

  const gates = [];

  // Gate 1: Headless browser testing (errors, FPS, screenshots, interactions)
  gates.push(run(
    'Headless Browser Testing (Errors + FPS + Interactions + Screenshots)',
    `node "${path.join(SCRIPTS, 'test_web_apps.js')}"`
  ));

  // Gate 2: Baseline screenshot diffing
  if (updateBaselines) {
    run('Update Baselines', `node "${path.join(SCRIPTS, 'baseline_diff.js')}" --update`);
    gates.push(true); // Updating baselines always passes
  } else if (fs.existsSync(path.join(ROOT, 'docs/gallery/baselines'))) {
    gates.push(run(
      'Baseline Screenshot Regression Detection',
      `node "${path.join(SCRIPTS, 'baseline_diff.js')}"`
    ));
  } else {
    console.log('\n  ⏭️  Baseline diff skipped — no baselines directory. Run with --update-baselines first.\n');
    gates.push(true);
  }

  // Gate 3: Vision model audit
  // Prefer the agent-native kilo-vision-audit skill (no API key needed).
  // Fall back to the legacy Python script if GEMINI_API_KEY is set and uv is available.
  if (skipVision) {
    console.log('\n  ⏭️  Vision audit skipped (--skip-vision flag).\n');
    gates.push(true);
  } else if (process.env.GEMINI_API_KEY) {
    // Legacy path: use Python script with external Gemini API
    const uvCheck = spawnSync('uv', ['--version'], { stdio: 'pipe' });
    if (uvCheck.status === 0) {
      gates.push(run(
        'AI Vision Quality Audit (Gemini Flash — legacy Python script)',
        `uv run --with google-genai "${path.join(SCRIPTS, 'vision_audit.py')}"`
      ));
    } else {
      console.log('\n  ⏭️  Vision audit skipped — uv not found. Install with: pip install uv\n');
      gates.push(true);
    }
  } else {
    console.log('\n  ⏭️  Vision audit skipped — no GEMINI_API_KEY set.');
    console.log('      Recommended: Use the kilo-vision-audit agent skill instead (no API key needed).');
    console.log('      Or set GEMINI_API_KEY to use the legacy Python script.\n');
    gates.push(true);
  }

  // Final summary
  console.log('\n╔══════════════════════════════════════════════════════════╗');
  console.log('║                    QUALITY GATE SUMMARY                 ║');
  console.log('╠══════════════════════════════════════════════════════════╣');
  
  const labels = [
    'Headless Browser Testing',
    'Baseline Regression Detection',
    'AI Vision Quality Audit'
  ];
  
  for (let i = 0; i < gates.length; i++) {
    const icon = gates[i] ? '✅' : '❌';
    console.log(`║  ${icon} ${labels[i].padEnd(52)}║`);
  }

  const allPassed = gates.every(g => g);
  console.log('╠══════════════════════════════════════════════════════════╣');
  console.log(`║  ${allPassed ? '🟢 ALL GATES PASSED' : '🔴 ONE OR MORE GATES FAILED'}${' '.repeat(allPassed ? 36 : 30)}║`);
  console.log('╚══════════════════════════════════════════════════════════╝');

  // Write JSON summary for programmatic consumption by agents/orchestrator
  const report = {
    timestamp: new Date().toISOString(),
    gates: labels.map((label, i) => ({ gate: label, passed: gates[i] })),
    allPassed
  };
  const reportPath = path.join(ROOT, 'docs/gallery/quality_gate_report.json');
  fs.mkdirSync(path.dirname(reportPath), { recursive: true });
  fs.writeFileSync(reportPath, JSON.stringify(report, null, 2), 'utf8');
  console.log(`\nFull report saved to docs/gallery/quality_gate_report.json`);

  process.exit(allPassed ? 0 : 1);
}

main();
