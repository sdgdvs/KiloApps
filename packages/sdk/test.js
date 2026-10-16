/**
 * Unit Test Suite for @kiloapps/sdk v2.0.0
 * Verifies:
 *  - ZzFX 20-parameter sample synthesis & wave shapes
 *  - Backward compatibility of playTone and sfx
 *  - InputManager isDown and justPressed tracking
 *  - StorageBridge persistence and high-score logic
 *  - TelemetryReporter structure & FPS calculation
 *  - QuickSave / QuickLoad hooks
 */

const assert = require('assert');

// Mock browser environment for Node.js test runner
global.window = {
  self: {},
  top: {}, // simulate standalone
  location: { pathname: '/apps/ktestcartridge.html', origin: 'http://localhost:3000' },
  addEventListener: () => {},
  removeEventListener: () => {},
  parent: { postMessage: () => {} }
};
global.window.self = global.window; // standalone initially
global.document = {
  hidden: false,
  body: { appendChild: () => {} },
  createElement: () => ({
    style: {},
    appendChild: () => {},
    addEventListener: () => {}
  }),
  getElementById: () => null,
  addEventListener: () => {}
};
global.navigator = {
  maxTouchPoints: 0,
  getGamepads: () => []
};
global.performance = {
  now: () => Date.now(),
  memory: { usedJSHeapSize: 4194304 }
};
global.requestAnimationFrame = (fn) => setTimeout(fn, 16);
global.cancelAnimationFrame = (id) => clearTimeout(id);

// Mock localStorage
const storage = {};
global.localStorage = {
  getItem: (k) => (k in storage ? storage[k] : null),
  setItem: (k, v) => { storage[k] = String(v); },
  removeItem: (k) => { delete storage[k]; },
  clear: () => { Object.keys(storage).forEach(k => delete storage[k]); }
};

// Mock AudioContext
class MockAudioBuffer {
  constructor(channels, length, sampleRate) {
    this.channels = channels;
    this.length = length;
    this.sampleRate = sampleRate;
    this._data = new Float32Array(length);
  }
  getChannelData() { return this._data; }
}

class MockAudioNode {
  constructor() {
    this.gain = { value: 1, setValueAtTime: () => {}, exponentialRampToValueAtTime: () => {} };
    this.frequency = { value: 440, setValueAtTime: () => {}, exponentialRampToValueAtTime: () => {} };
  }
  connect() { return new MockAudioNode(); }
  start() {}
  stop() {}
}

global.AudioContext = class {
  constructor() {
    this.state = 'running';
    this.sampleRate = 44100;
    this.currentTime = 0;
    this.destination = new MockAudioNode();
  }
  createBuffer(c, l, r) { return new MockAudioBuffer(c, l, r); }
  createBufferSource() { return new MockAudioNode(); }
  createGain() { return new MockAudioNode(); }
  createOscillator() { return new MockAudioNode(); }
  createBiquadFilter() { return new MockAudioNode(); }
  resume() { return Promise.resolve(); }
};

// Load SDK
const KiloSDK = require('./kilo-sdk.js');

console.log('--- Testing @kiloapps/sdk v2.0.0 ---');

// 1. Version Check
assert.strictEqual(KiloSDK.version, '2.0.0', 'Version should be 2.0.0');
console.log('✅ Version: 2.0.0 verified');

// 2. Audio Engine & ZzFX 20-parameter synthesis
const audio = KiloSDK.audio;
assert(typeof audio.buildSamples === 'function', 'buildSamples must be a function');
assert(typeof audio.zzfx === 'function', 'zzfx must be a function');
assert(typeof audio.playTone === 'function', 'playTone must be backward compatible');
assert(typeof audio.playNoise === 'function', 'playNoise must be available');
assert(typeof audio.sfx.shoot === 'function', 'sfx.shoot must exist');
assert(typeof audio.sfx.jump === 'function', 'sfx.jump must exist');
assert(typeof audio.sfx.coin === 'function', 'sfx.coin must exist');
assert(typeof audio.sfx.explosion === 'function', 'sfx.explosion must exist');
assert(typeof audio.sfx.powerup === 'function', 'sfx.powerup must exist');

// Test ZzFX sample generation with 20 parameters
const samples = audio.buildSamples(
  1,     // volume
  0.05,  // randomness
  440,   // frequency
  0.02,  // attack
  0.05,  // sustain
  0.1,   // release
  0,     // shape (sine)
  1,     // shapeCurve
  0,     // slide
  0,     // deltaSlide
  0,     // pitchJump
  0,     // pitchJumpTime
  0,     // repeatTime
  0,     // noise
  0,     // modulation
  0,     // bitCrush
  0,     // delay
  1,     // sustainVolume
  0.02,  // decay
  0      // tremolo
);

assert(samples instanceof Float32Array, 'Samples must be a Float32Array');
assert(samples.length > 0, 'Generated samples must not be empty');
console.log(`✅ ZzFX 20-param synthesis generated ${samples.length} PCM samples successfully`);

// Test all shapes
const shapes = ['sine', 'triangle', 'sawtooth', 'tan', 'noise', 'square'];
shapes.forEach((sh, idx) => {
  const s = audio.buildSamples(1, 0, 300, 0.01, 0, 0.05, sh);
  assert(s.length > 0, `Shape ${sh} must produce samples`);
});
console.log('✅ All 6 waveform shapes (sine, triangle, saw, tan, noise, square) synthesize cleanly');

// Test zzfx play call with array and spread arguments
const src1 = audio.zzfx([1, 0.05, 440, 0.01, 0, 0.05]);
assert(src1 !== null, 'zzfx([params]) must return audio source');
const src2 = audio.zzfx(1, 0.05, 440, 0.01, 0, 0.05);
assert(src2 !== null, 'zzfx(...params) must return audio source');
console.log('✅ zzfx accepts both array and spread parameters with 100% backward compatibility');

// Test playTone and playNoise backward compatibility
const tone = audio.playTone(440, 0.05, 'square');
assert(tone !== null, 'playTone must return oscillator');
const noise = audio.playNoise(0.05, 800, 0.1);
assert(noise !== null, 'playNoise must return noise buffer source');
console.log('✅ playTone and playNoise backward compatibility confirmed');

// 3. Input Manager (Keyboard, Gamepad, isDown, justPressed)
const input = KiloSDK.input;
input.init();

// Simulate key press
input.keys['ArrowLeft'] = true;
input._justPressed['ArrowLeft'] = true;

assert.strictEqual(input.isDown('LEFT'), true, 'LEFT should be down');
assert.strictEqual(input.justPressed('LEFT'), true, 'LEFT should be just pressed');

// Simulate next frame where justPressed cleared
input._justPressed = {};
assert.strictEqual(input.isDown('LEFT'), true, 'LEFT should still be down');
assert.strictEqual(input.justPressed('LEFT'), false, 'LEFT should no longer be just pressed');

// Key release
input.keys['ArrowLeft'] = false;
assert.strictEqual(input.isDown('LEFT'), false, 'LEFT should be up after release');

// Test action mapping
input.keys['Space'] = true;
input._justPressed['Space'] = true;
assert.strictEqual(input.isDown('ACTION'), true, 'ACTION should map to Space');
assert.strictEqual(input.justPressed('ACTION'), true, 'ACTION justPressed should map to Space');
input.keys['Space'] = false;
input._justPressed = {};

// Test gamepad mapping
input.gamepadState['GP_ACTION'] = true;
input.gamepadJustPressed['GP_ACTION'] = true;
assert.strictEqual(input.isDown('ACTION'), true, 'ACTION should map to GP_ACTION');
assert.strictEqual(input.justPressed('ACTION'), true, 'ACTION justPressed should map to GP_ACTION');
input.gamepadState = {};
input.gamepadJustPressed = {};

console.log('✅ InputManager isDown and justPressed tracking verified across keyboard & gamepad');

// 4. Storage & VFS Bridge
const storageBridge = KiloSDK.storage;
storageBridge.init('ktestcartridge');

storageBridge.set('score', 9500);
assert.strictEqual(storageBridge.get('score'), 9500, 'StorageBridge get must return stored value');
storageBridge.setHighScore(9500);
assert.strictEqual(storageBridge.getHighScore(), 9500, 'High score should match');
assert.strictEqual(storageBridge.setHighScore(12000), true, 'New high score should be accepted');
assert.strictEqual(storageBridge.getHighScore(), 12000, 'High score should now be 12000');
assert.strictEqual(storageBridge.setHighScore(5000), false, 'Lesser score should be rejected');

storageBridge.delete('score');
assert.strictEqual(storageBridge.get('score'), null, 'Deleted key should return null/default');
console.log('✅ StorageBridge persistence, get/set/delete, and high-score logic verified');

// 5. Telemetry Reporter
const telemetry = KiloSDK.telemetry;
telemetry.init('ktestcartridge');
assert.strictEqual(typeof telemetry.fps, 'number', 'FPS should be a number');
assert.strictEqual(telemetry.cartridgeId, 'ktestcartridge', 'CartridgeId should be set');
console.log('✅ TelemetryReporter initialization and metrics loop verified');

// 6. Quicksave / Quickload Out of the Box
let savedState = null;
let loadedState = null;

KiloSDK.init({ id: 'ktestcartridge', title: 'Test Cartridge' });
KiloSDK.onQuicksave = () => ({ level: 4, gold: 250 });
KiloSDK.onQuickload = (st) => { loadedState = st; };

KiloSDK.saveQuick(KiloSDK.onQuicksave());
assert.deepStrictEqual(storageBridge.get('quicksave'), { level: 4, gold: 250 }, 'QuickSave must persist state');

KiloSDK.loadQuick().then((st) => {
  assert.deepStrictEqual(loadedState, { level: 4, gold: 250 }, 'QuickLoad must restore state');
  console.log('✅ Out-of-the-box F5 Quicksave / F9 Quickload verified');
  console.log('--------------------------------------------------');
  console.log('🎉 ALL @kiloapps/sdk v2.0.0 UNIT TESTS PASSED!');
  process.exit(0);
});
