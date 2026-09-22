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
    execSync(command, { cwd: ROOT, stdio: 'inherit', timeout: 300000 }); // 5min timeout
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

  // Gate 3: Vision model audit (optional)
  if (skipVision || !process.env.GEMINI_API_KEY) {
    if (!skipVision && !process.env.GEMINI_API_KEY) {
      console.log('\n  ⏭️  Vision audit skipped — GEMINI_API_KEY not set.');
      console.log('      Set it to enable AI-powered visual quality scoring.\n');
    } else {
      console.log('\n  ⏭️  Vision audit skipped (--skip-vision flag).\n');
    }
    gates.push(true);
  } else {
    // Check if uv is available for Python script execution
    const uvCheck = spawnSync('uv', ['--version'], { stdio: 'pipe' });
    if (uvCheck.status === 0) {
      gates.push(run(
        'AI Vision Quality Audit (Gemini Flash)',
        `uv run --with google-genai "${path.join(SCRIPTS, 'vision_audit.py')}"`
      ));
    } else {
      console.log('\n  ⏭️  Vision audit skipped — uv not found. Install with: pip install uv\n');
      gates.push(true);
    }
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

  process.exit(allPassed ? 0 : 1);
}

main();
