/**
 * @kiloapps/sdk v2.0.0 - Micro-SDK for Sub-999KB Web Minigames & Utilities
 *
 * Upgrades:
 *  1. Full ZzFX 20-parameter procedural synth + noise generator
 *     - 100% backward compatible with KiloSDK.audio.playTone() and KiloSDK.audio.sfx
 *  2. Real HTML5 Gamepad API polling (navigator.getGamepads())
 *     - Multi-button Virtual Touch Pad (D-Pad + A / B buttons)
 *     - Keyboard & controller state tracking (isDown, justPressed)
 *  3. Bidirectional VFS Sync
 *     - Hydrates localStorage from KiloOS VFS_READ on boot
 *     - Out-of-the-box F5 Quicksave / F9 Quickload persistence hooks
 *     - Streams live 60FPS telemetry to KDirector, KTaskMgr, and KiloOS
 */
(function (global) {
  'use strict';

  // --- 1. Procedural Audio Synthesizer (Singleton WebAudio + Full ZzFX 20-Parameter Engine) ---
  const AudioEngine = {
    ctx: null,
    muted: false,
    masterVolume: 0.8,
    _noiseBuffer: null,

    _getCtx() {
      if (!this.ctx) {
        const AudioCtx =
          (typeof window !== 'undefined' && (window.AudioContext || window.webkitAudioContext)) ||
          (typeof AudioContext !== 'undefined' ? AudioContext : null);
        if (AudioCtx) this.ctx = new AudioCtx();
      }
      if (this.ctx && this.ctx.state === 'suspended') {
        this.ctx.resume().catch(() => {});
      }
      return this.ctx;
    },

    _getNoiseBuffer(ctx) {
      if (this._noiseBuffer) return this._noiseBuffer;
      const sampleRate = ctx.sampleRate || 44100;
      const bufferSize = Math.floor(sampleRate * 1.0);
      const buffer = ctx.createBuffer(1, bufferSize, sampleRate);
      const output = buffer.getChannelData(0);
      for (let i = 0; i < bufferSize; i++) {
        output[i] = Math.random() * 2 - 1;
      }
      this._noiseBuffer = buffer;
      return buffer;
    },

    /**
     * Backward-compatible tone generator using Web Audio oscillators.
     * @param {number} freq - Base frequency in Hz
     * @param {number} duration - Tone duration in seconds
     * @param {OscillatorType} type - 'square' | 'sine' | 'sawtooth' | 'triangle'
     * @param {number} decay - Exponential decay tail in seconds
     * @param {number} slide - Pitch slide multiplier
     */
    playTone(freq = 440, duration = 0.1, type = 'square', decay = 0.08, slide = 0.5) {
      if (this.muted) return null;
      try {
        const ctx = this._getCtx();
        if (!ctx) return null;
        const t = ctx.currentTime;
        const osc = ctx.createOscillator();
        const gain = ctx.createGain();
        osc.type = type;
        osc.frequency.setValueAtTime(freq, t);
        osc.frequency.exponentialRampToValueAtTime(Math.max(20, freq * slide), t + duration);
        gain.gain.setValueAtTime(0.12 * this.masterVolume, t);
        gain.gain.exponentialRampToValueAtTime(0.001, t + duration + decay);
        osc.connect(gain);
        gain.connect(ctx.destination);
        osc.start(t);
        osc.stop(t + duration + decay);
        return osc;
      } catch (e) {
        return null;
      }
    },

    /**
     * Procedural bandpass-filtered noise generator.
     * @param {number} duration - Noise duration in seconds
     * @param {number} filterFreq - Bandpass center frequency in Hz
     * @param {number} volume - Volume multiplier (0 to 1)
     */
    playNoise(duration = 0.2, filterFreq = 800, volume = 0.15) {
      if (this.muted) return null;
      try {
        const ctx = this._getCtx();
        if (!ctx) return null;
        const t = ctx.currentTime;
        const noise = ctx.createBufferSource();
        noise.buffer = this._getNoiseBuffer(ctx);
        const filter = ctx.createBiquadFilter();
        filter.type = 'bandpass';
        filter.frequency.setValueAtTime(filterFreq, t);
        const gain = ctx.createGain();
        gain.gain.setValueAtTime(volume * this.masterVolume, t);
        gain.gain.exponentialRampToValueAtTime(0.001, t + duration);
        noise.connect(filter);
        filter.connect(gain);
        gain.connect(ctx.destination);
        noise.start(t);
        noise.stop(t + duration);
        return noise;
      } catch (e) {
        return null;
      }
    },

    /**
     * Full ZzFX 20-Parameter PCM Sample Builder.
     * Generates a Float32Array containing pure synthesized PCM audio data.
     *
     * Parameters:
     *  0: volume = 1               - Volume scale (0 to 1+)
     *  1: randomness = .05         - Randomness factor added to start frequency
     *  2: frequency = 220          - Starting frequency (Hz)
     *  3: attack = 0               - Attack duration in seconds
     *  4: sustain = 0              - Sustain duration in seconds
     *  5: release = .1             - Release duration in seconds
     *  6: shape = 0                - Waveform shape (0: sin, 1: triangle, 2: saw, 3: tan, 4: noise, 5: square)
     *  7: shapeCurve = 1           - Exponent power curve applied to waveform
     *  8: slide = 0                - Frequency slide rate
     *  9: deltaSlide = 0           - Frequency slide acceleration
     * 10: pitchJump = 0            - Frequency jump amount
     * 11: pitchJumpTime = 0        - Time in seconds before pitch jump occurs
     * 12: repeatTime = 0           - Period in seconds after which sound repeats
     * 13: noise = 0                - Noise mix amount
     * 14: modulation = 0           - Frequency modulation depth / vibrato
     * 15: bitCrush = 0             - Bit-crush resolution divisor
     * 16: delay = 0                - Echo delay time in seconds
     * 17: sustainVolume = 1        - Volume level during sustain phase
     * 18: decay = 0                - Decay duration in seconds
     * 19: tremolo = 0              - Tremolo amplitude modulation depth
     * 20: filter = 0               - Optional biquad filter frequency
     */
    buildSamples(
      volume = 1,
      randomness = 0.05,
      frequency = 220,
      attack = 0,
      sustain = 0,
      release = 0.1,
      shape = 0,
      shapeCurve = 1,
      slide = 0,
      deltaSlide = 0,
      pitchJump = 0,
      pitchJumpTime = 0,
      repeatTime = 0,
      noise = 0,
      modulation = 0,
      bitCrush = 0,
      delay = 0,
      sustainVolume = 1,
      decay = 0,
      tremolo = 0,
      filter = 0
    ) {
      const sampleRate = (this.ctx && this.ctx.sampleRate) ? this.ctx.sampleRate : 44100;
      const PI2 = Math.PI * 2;
      const abs = Math.abs;
      const sign = (v) => (v < 0 ? -1 : 1);

      // Support string shape aliases
      if (typeof shape === 'string') {
        const shapeMap = { sin: 0, sine: 0, tri: 1, triangle: 1, saw: 2, sawtooth: 2, tan: 3, noise: 4, square: 5 };
        shape = shapeMap[shape.toLowerCase()] ?? 0;
      }

      let startSlide = (slide *= (500 * PI2) / sampleRate / sampleRate);
      let startFrequency = (frequency *= ((1 + randomness * 2 * Math.random() - randomness) * PI2) / sampleRate);
      let modOffset = 0;
      let repeat = 0;
      let crush = 0;
      let jump = 1;
      let t = 0;
      let i = 0;
      let s = 0;
      let f = 0;

      // Filter coefficients (LP/HP)
      let quality = 2;
      let w = (PI2 * abs(filter) * 2) / sampleRate;
      let cos = Math.cos(w);
      let alpha = Math.sin(w) / 2 / quality;
      let a0 = 1 + alpha;
      let a1 = (-2 * cos) / a0;
      let a2 = (1 - alpha) / a0;
      let b0 = (1 + sign(filter) * cos) / 2 / a0;
      let b1 = -(sign(filter) + cos) / a0;
      let b2 = b0;
      let x2 = 0;
      let x1 = 0;
      let y2 = 0;
      let y1 = 0;

      // Scale times by sample rate
      const minAttack = 9;
      attack = attack * sampleRate || minAttack;
      decay *= sampleRate;
      sustain *= sampleRate;
      release *= sampleRate;
      delay *= sampleRate;
      deltaSlide *= (500 * PI2) / Math.pow(sampleRate, 3);
      modulation *= PI2 / sampleRate;
      pitchJump *= PI2 / sampleRate;
      pitchJumpTime *= sampleRate;
      repeatTime = Math.floor(repeatTime * sampleRate);

      const length = Math.floor(attack + decay + sustain + release + delay);
      const b = new Float32Array(length > 0 ? length : 0);

      // Synthesize waveform
      for (; i < length; b[i++] = s * volume) {
        if (!(++crush % (Math.floor(bitCrush * 100) || 1))) {
          s = shape
            ? shape > 1
              ? shape > 2
                ? shape > 3
                  ? shape > 4
                    ? (t / PI2) % 1 < shapeCurve / 2 ? 1 : -1 // 5: Square duty
                    : Math.sin(Math.pow(t, 3))                 // 4: Noise
                  : Math.max(Math.min(Math.tan(t), 1), -1)    // 3: Tan
                : 1 - ((2 * t / PI2) % 2 + 2) % 2              // 2: Sawtooth
              : 1 - 4 * abs(Math.round(t / PI2) - t / PI2)     // 1: Triangle
            : Math.sin(t);                                      // 0: Sine

          s =
            (repeatTime ? 1 - tremolo + tremolo * Math.sin((PI2 * i) / repeatTime) : 1) *
            (shape > 4 ? s : sign(s) * Math.pow(abs(s), shapeCurve)) *
            (i < attack
              ? i / attack
              : i < attack + decay
              ? 1 - ((i - attack) / decay) * (1 - sustainVolume)
              : i < attack + decay + sustain
              ? sustainVolume
              : i < length - delay
              ? ((length - i - delay) / release) * sustainVolume
              : 0);

          s = delay
            ? s / 2 +
              (delay > i
                ? 0
                : (i < length - delay ? 1 : (length - i) / delay) *
                  (b[Math.floor(i - delay)] || 0) / 2 / (volume || 1))
            : s;

          if (filter) {
            s = y1 = b2 * x2 + b1 * (x2 = x1) + b0 * (x1 = s) - a2 * y2 - a1 * (y2 = y1);
          }
        }

        f = (frequency += slide += deltaSlide) * Math.cos(modulation * modOffset++);
        t += f + f * noise * (((i * i * PI2) % 2) - 1);

        if (jump && ++jump > pitchJumpTime) {
          frequency += pitchJump;
          startFrequency += pitchJump;
          jump = 0;
        }

        if (repeatTime && !(++repeat % repeatTime)) {
          frequency = startFrequency;
          slide = startSlide;
          jump = jump || 1;
        }
      }

      return b;
    },

    /**
     * Plays a procedural sound using the full 20-parameter ZzFX engine.
     * Supports parameter array: zzfx([volume, randomness, freq, ...])
     * or spread arguments: zzfx(volume, randomness, freq, ...)
     */
    zzfx(...parameters) {
      if (this.muted) return null;
      try {
        const ctx = this._getCtx();
        if (!ctx) return null;

        // Unwrap array if passed as single array argument
        const params = (parameters.length === 1 && Array.isArray(parameters[0]))
          ? parameters[0]
          : parameters;

        const samples = this.buildSamples(...params);
        if (!samples || samples.length === 0) return null;

        const buffer = ctx.createBuffer(1, samples.length, ctx.sampleRate || 44100);
        buffer.getChannelData(0).set(samples);

        const source = ctx.createBufferSource();
        source.buffer = buffer;

        const gainNode = ctx.createGain();
        gainNode.gain.value = this.masterVolume;
        source.connect(gainNode);
        gainNode.connect(ctx.destination);

        source.start();
        return source;
      } catch (e) {
        return null;
      }
    },

    /**
     * Calculates note frequency on standard 12-TET diatonic scale.
     */
    getNote(semitoneOffset = 0, rootNoteFrequency = 440) {
      return rootNoteFrequency * Math.pow(2, semitoneOffset / 12);
    },

    /**
     * Standardized retro 1999 sound effects library.
     */
    sfx: {
      shoot() { AudioEngine.playTone(880, 0.06, 'square', 0.04, 0.35); },
      jump() { AudioEngine.playTone(320, 0.12, 'sine', 0.08, 1.8); },
      coin() {
        AudioEngine.playTone(987.77, 0.08, 'sine', 0.08, 1.0);
        setTimeout(() => AudioEngine.playTone(1318.51, 0.15, 'sine', 0.1, 1.0), 60);
      },
      explosion() {
        AudioEngine.playNoise(0.38, 320, 0.25);
        AudioEngine.playTone(110, 0.35, 'sawtooth', 0.2, 0.25);
      },
      hit() {
        AudioEngine.playNoise(0.09, 1200, 0.15);
        AudioEngine.playTone(200, 0.08, 'sawtooth', 0.05, 0.4);
      },
      powerup() {
        [440, 554.37, 659.25, 880].forEach((f, i) => {
          setTimeout(() => AudioEngine.playTone(f, 0.08, 'triangle', 0.05, 1.05), i * 50);
        });
      },
      click() { AudioEngine.playTone(1200, 0.02, 'sine', 0.02, 0.5); }
    }
  };

  // --- 2. Unified Input Controller (Keyboard + Gamepad API + Multi-button Virtual Touch Pad) ---
  const InputManager = {
    keys: {},
    prevKeys: {},
    _justPressed: {},
    gamepadState: {},
    prevGamepadState: {},
    gamepadJustPressed: {},
    touchActive: false,
    virtualPadEl: null,
    deadzone: 0.25,
    _rafId: null,
    _running: false,

    init(enableVirtualPad = false) {
      window.addEventListener('keydown', (e) => {
        if (!this.keys[e.key] && !this.keys[e.code]) {
          this._justPressed[e.key] = true;
          this._justPressed[e.code] = true;
        }
        this.keys[e.key] = true;
        this.keys[e.code] = true;
      });

      window.addEventListener('keyup', (e) => {
        this.keys[e.key] = false;
        this.keys[e.code] = false;
        this._justPressed[e.key] = false;
        this._justPressed[e.code] = false;
      });

      // Clear stuck keys when window loses visibility
      document.addEventListener('visibilitychange', () => {
        if (document.hidden) {
          this.reset();
          this._stopGamepadLoop();
        } else {
          this._startGamepadLoop();
        }
      });

      if (enableVirtualPad || 'ontouchstart' in window || (navigator.maxTouchPoints && navigator.maxTouchPoints > 0)) {
        this.enableTouchControls();
      }

      this._startGamepadLoop();
    },

    reset() {
      this.keys = {};
      this.prevKeys = {};
      this._justPressed = {};
      this.gamepadState = {};
      this.prevGamepadState = {};
      this.gamepadJustPressed = {};
    },

    _startGamepadLoop() {
      if (this._running) return;
      this._running = true;

      const poll = () => {
        if (!this._running) return;

        // Advance keyboard tracking
        this.prevKeys = { ...this.keys };
        // Reset justPressed accumulator for next frame
        this._justPressed = {};

        // Poll HTML5 Gamepads
        const pads = navigator.getGamepads ? navigator.getGamepads() : [];
        let gp = null;
        for (let i = 0; i < pads.length; i++) {
          if (pads[i] && pads[i].connected) {
            gp = pads[i];
            break;
          }
        }

        const nextGamepad = {};
        if (gp) {
          const ax0 = gp.axes[0] || 0;
          const ax1 = gp.axes[1] || 0;
          const btn = (idx) => !!(gp.buttons[idx] && gp.buttons[idx].pressed);

          nextGamepad['GP_LEFT'] = ax0 < -this.deadzone || btn(14);
          nextGamepad['GP_RIGHT'] = ax0 > this.deadzone || btn(15);
          nextGamepad['GP_UP'] = ax1 < -this.deadzone || btn(12);
          nextGamepad['GP_DOWN'] = ax1 > this.deadzone || btn(13);
          nextGamepad['GP_ACTION'] = btn(0);
          nextGamepad['GP_A'] = btn(0);
          nextGamepad['GP_ALT'] = btn(1);
          nextGamepad['GP_B'] = btn(1);
          nextGamepad['GP_X'] = btn(2);
          nextGamepad['GP_Y'] = btn(3);
          nextGamepad['GP_L1'] = btn(4);
          nextGamepad['GP_R1'] = btn(5);
          nextGamepad['GP_SELECT'] = btn(8);
          nextGamepad['GP_START'] = btn(9);
        }

        // Calculate single-frame gamepad transitions
        this.gamepadJustPressed = {};
        for (const k in nextGamepad) {
          if (nextGamepad[k] && !this.prevGamepadState[k]) {
            this.gamepadJustPressed[k] = true;
          }
        }

        this.prevGamepadState = { ...this.gamepadState };
        this.gamepadState = nextGamepad;

        this._rafId = requestAnimationFrame(poll);
      };

      this._rafId = requestAnimationFrame(poll);
    },

    _stopGamepadLoop() {
      this._running = false;
      if (this._rafId) {
        cancelAnimationFrame(this._rafId);
        this._rafId = null;
      }
    },

    /**
     * Checks if a key, semantic action, or gamepad button is currently held down.
     * @param {string} action - 'LEFT' | 'RIGHT' | 'UP' | 'DOWN' | 'ACTION' | 'ALT' | key name
     */
    isDown(action) {
      if (!action) return false;
      const a = String(action).toUpperCase();
      if (a === 'LEFT') {
        return !!(this.keys['ArrowLeft'] || this.keys['KeyA'] || this.keys['a'] || this.keys['left'] || this.gamepadState['GP_LEFT']);
      }
      if (a === 'RIGHT') {
        return !!(this.keys['ArrowRight'] || this.keys['KeyD'] || this.keys['d'] || this.keys['right'] || this.gamepadState['GP_RIGHT']);
      }
      if (a === 'UP') {
        return !!(this.keys['ArrowUp'] || this.keys['KeyW'] || this.keys['w'] || this.keys['up'] || this.gamepadState['GP_UP']);
      }
      if (a === 'DOWN') {
        return !!(this.keys['ArrowDown'] || this.keys['KeyS'] || this.keys['s'] || this.keys['down'] || this.gamepadState['GP_DOWN']);
      }
      if (a === 'ACTION' || a === 'FIRE' || a === 'JUMP' || a === 'A') {
        return !!(this.keys['Space'] || this.keys['KeyZ'] || this.keys['KeyJ'] || this.keys['Enter'] || this.keys['action'] || this.gamepadState['GP_ACTION']);
      }
      if (a === 'ALT' || a === 'B') {
        return !!(this.keys['KeyX'] || this.keys['KeyK'] || this.keys['ShiftLeft'] || this.keys['ShiftRight'] || this.keys['alt'] || this.gamepadState['GP_ALT']);
      }
      if (a === 'SELECT') {
        return !!(this.keys['KeyC'] || this.keys['Backspace'] || this.gamepadState['GP_SELECT']);
      }
      if (a === 'START') {
        return !!(this.keys['Enter'] || this.keys['Escape'] || this.gamepadState['GP_START']);
      }
      return !!(this.keys[action] || this.keys[action.toLowerCase()] || this.gamepadState[action]);
    },

    /**
     * Checks if a key, semantic action, or gamepad button was pressed down this frame.
     * Guaranteed to trigger only on the rising-edge transition.
     * @param {string} action - 'LEFT' | 'RIGHT' | 'UP' | 'DOWN' | 'ACTION' | 'ALT' | key name
     */
    justPressed(action) {
      if (!action) return false;
      const a = String(action).toUpperCase();
      if (a === 'LEFT') {
        return !!(this._justPressed['ArrowLeft'] || this._justPressed['KeyA'] || this._justPressed['a'] || this.gamepadJustPressed['GP_LEFT']);
      }
      if (a === 'RIGHT') {
        return !!(this._justPressed['ArrowRight'] || this._justPressed['KeyD'] || this._justPressed['d'] || this.gamepadJustPressed['GP_RIGHT']);
      }
      if (a === 'UP') {
        return !!(this._justPressed['ArrowUp'] || this._justPressed['KeyW'] || this._justPressed['w'] || this.gamepadJustPressed['GP_UP']);
      }
      if (a === 'DOWN') {
        return !!(this._justPressed['ArrowDown'] || this._justPressed['KeyS'] || this._justPressed['s'] || this.gamepadJustPressed['GP_DOWN']);
      }
      if (a === 'ACTION' || a === 'FIRE' || a === 'JUMP' || a === 'A') {
        return !!(this._justPressed['Space'] || this._justPressed['KeyZ'] || this._justPressed['KeyJ'] || this._justPressed['Enter'] || this.gamepadJustPressed['GP_ACTION']);
      }
      if (a === 'ALT' || a === 'B') {
        return !!(this._justPressed['KeyX'] || this._justPressed['KeyK'] || this._justPressed['ShiftLeft'] || this._justPressed['ShiftRight'] || this.gamepadJustPressed['GP_ALT']);
      }
      if (a === 'SELECT') {
        return !!(this._justPressed['KeyC'] || this._justPressed['Backspace'] || this.gamepadJustPressed['GP_SELECT']);
      }
      if (a === 'START') {
        return !!(this._justPressed['Enter'] || this._justPressed['Escape'] || this.gamepadJustPressed['GP_START']);
      }
      return !!(this._justPressed[action] || this._justPressed[action.toLowerCase()] || this.gamepadJustPressed[action]);
    },

    enableTouchControls() {
      if (this.virtualPadEl || typeof document === 'undefined' || !document.body) return;
      const pad = document.createElement('div');
      pad.id = 'kilo-virtual-pad';
      pad.innerHTML = `
        <div style="position:fixed;bottom:16px;left:16px;z-index:99999;display:grid;grid-template-columns:repeat(3,44px);grid-gap:6px;opacity:0.75;user-select:none;-webkit-user-select:none;touch-action:none;">
          <div></div>
          <button id="kvp-up" aria-label="Up" style="height:44px;background:#111827;color:#00f0ff;border:1px solid #00f0ff;border-radius:6px;font-size:16px;cursor:pointer;touch-action:none;">▲</button>
          <div></div>
          <button id="kvp-left" aria-label="Left" style="height:44px;background:#111827;color:#00f0ff;border:1px solid #00f0ff;border-radius:6px;font-size:16px;cursor:pointer;touch-action:none;">◀</button>
          <div></div>
          <button id="kvp-right" aria-label="Right" style="height:44px;background:#111827;color:#00f0ff;border:1px solid #00f0ff;border-radius:6px;font-size:16px;cursor:pointer;touch-action:none;">▶</button>
          <div></div>
          <button id="kvp-down" aria-label="Down" style="height:44px;background:#111827;color:#00f0ff;border:1px solid #00f0ff;border-radius:6px;font-size:16px;cursor:pointer;touch-action:none;">▼</button>
          <div></div>
        </div>
        <div style="position:fixed;bottom:20px;right:20px;z-index:99999;display:flex;gap:12px;opacity:0.75;user-select:none;-webkit-user-select:none;touch-action:none;">
          <button id="kvp-alt" aria-label="B Button" style="width:52px;height:52px;border-radius:50%;background:#f59e0b;color:#000;font-weight:bold;border:2px solid #fff;font-size:16px;cursor:pointer;touch-action:none;">B</button>
          <button id="kvp-act" aria-label="A Button" style="width:58px;height:58px;border-radius:50%;background:#e11d48;color:#fff;font-weight:bold;border:2px solid #fff;font-size:18px;cursor:pointer;touch-action:none;">A</button>
        </div>
      `;
      document.body.appendChild(pad);
      this.virtualPadEl = pad;

      const bind = (id, key) => {
        const btn = document.getElementById(id);
        if (!btn) return;
        const press = (e) => {
          e.preventDefault();
          this.keys[key] = true;
          this._justPressed[key] = true;
        };
        const release = (e) => {
          if (e) e.preventDefault();
          this.keys[key] = false;
        };
        btn.addEventListener('pointerdown', press);
        btn.addEventListener('pointerup', release);
        btn.addEventListener('pointercancel', release);
        btn.addEventListener('pointerleave', release);
      };

      bind('kvp-up', 'ArrowUp');
      bind('kvp-down', 'ArrowDown');
      bind('kvp-left', 'ArrowLeft');
      bind('kvp-right', 'ArrowRight');
      bind('kvp-act', 'Space');
      bind('kvp-alt', 'KeyX');
    },

    showTouchControls() {
      if (this.virtualPadEl) this.virtualPadEl.style.display = 'block';
    },

    hideTouchControls() {
      if (this.virtualPadEl) this.virtualPadEl.style.display = 'none';
    }
  };

  // --- 3. Bidirectional Storage & VFS Bridge ---
  const StorageBridge = {
    cartridgeId: 'kilo-app',
    isEmbedded: typeof window !== 'undefined' && window.self !== window.top,
    _pendingRequests: new Map(),

    init(id) {
      if (id) this.cartridgeId = id;

      if (this.isEmbedded && typeof window !== 'undefined') {
        window.addEventListener('message', (e) => {
          if (window.location.origin && window.location.origin !== 'null' && e.origin && e.origin !== window.location.origin) {
            return;
          }

          const { type, requestId, data, files, path } = e.data || {};
          if (requestId && this._pendingRequests.has(requestId)) {
            const resolve = this._pendingRequests.get(requestId);
            this._pendingRequests.delete(requestId);
            resolve(files !== undefined ? files : data);
          }

          // Live VFS sync from host OS
          if (type === 'VFS_CHANGED' && path && path.startsWith(`/saves/${this.cartridgeId}_`)) {
            const key = path.replace(`/saves/${this.cartridgeId}_`, '').replace('.json', '');
            try {
              localStorage.setItem(`${this.cartridgeId}_${key}`, typeof data === 'string' ? data : JSON.stringify(data));
            } catch (_) {}
          }
        });

        // Hydrate localStorage from KiloOS VFS on startup
        this.hydrate();
      }
    },

    /**
     * Hydrates local state from KiloOS VFS on boot.
     */
    async hydrate() {
      if (!this.isEmbedded) return;
      try {
        // Query VFS catalog for all saves belonging to this cartridge
        const files = await this._requestHost('VFS_LIST');
        const prefix = `/saves/${this.cartridgeId}_`;
        if (Array.isArray(files)) {
          for (const path of files) {
            if (path.startsWith(prefix) && path.endsWith('.json')) {
              const key = path.slice(prefix.length, -5);
              const fileData = await this._requestHost('VFS_READ', { path });
              if (fileData !== null && fileData !== undefined) {
                try {
                  localStorage.setItem(`${this.cartridgeId}_${key}`, typeof fileData === 'string' ? fileData : JSON.stringify(fileData));
                } catch (_) {}
              }
            }
          }
        }

        // Specifically ensure quicksave is hydrated
        const quicksaveData = await this._requestHost('VFS_READ', { path: `/saves/${this.cartridgeId}_quicksave.json` });
        if (quicksaveData) {
          try {
            localStorage.setItem(`${this.cartridgeId}_quicksave`, typeof quicksaveData === 'string' ? quicksaveData : JSON.stringify(quicksaveData));
          } catch (_) {}
        }
      } catch (e) {
        console.warn('[KiloSDK] StorageBridge hydration warning:', e);
      }
    },

    _requestHost(type, payload = {}, timeoutMs = 1500) {
      if (!this.isEmbedded || typeof window === 'undefined') return Promise.resolve(null);
      return new Promise((resolve) => {
        const requestId = `${Date.now()}_${Math.random().toString(36).slice(2, 7)}`;
        this._pendingRequests.set(requestId, resolve);
        window.parent.postMessage({ type, requestId, ...payload }, '*');
        setTimeout(() => {
          if (this._pendingRequests.has(requestId)) {
            this._pendingRequests.delete(requestId);
            resolve(null);
          }
        }, timeoutMs);
      });
    },

    get(key, defaultValue = null) {
      try {
        const val = localStorage.getItem(`${this.cartridgeId}_${key}`);
        return val !== null ? JSON.parse(val) : defaultValue;
      } catch (e) {
        return defaultValue;
      }
    },

    async getAsync(key, defaultValue = null) {
      if (this.isEmbedded) {
        const vfsData = await this._requestHost('VFS_READ', {
          path: `/saves/${this.cartridgeId}_${key}.json`
        });
        if (vfsData !== null && vfsData !== undefined && vfsData !== '') {
          try {
            const raw = typeof vfsData === 'string' ? vfsData : JSON.stringify(vfsData);
            localStorage.setItem(`${this.cartridgeId}_${key}`, raw);
            return typeof vfsData === 'string' ? JSON.parse(vfsData) : vfsData;
          } catch (_) {}
        }
      }
      return this.get(key, defaultValue);
    },

    set(key, value) {
      try {
        const serialized = JSON.stringify(value);
        localStorage.setItem(`${this.cartridgeId}_${key}`, serialized);
        if (this.isEmbedded && typeof window !== 'undefined') {
          window.parent.postMessage({
            type: 'VFS_WRITE',
            requestId: Date.now(),
            path: `/saves/${this.cartridgeId}_${key}.json`,
            data: serialized
          }, '*');
        }
        return true;
      } catch (e) {
        console.warn('[KiloSDK] Storage write error:', e);
        return false;
      }
    },

    delete(key) {
      try {
        localStorage.removeItem(`${this.cartridgeId}_${key}`);
        if (this.isEmbedded && typeof window !== 'undefined') {
          window.parent.postMessage({
            type: 'VFS_DELETE',
            requestId: Date.now(),
            path: `/saves/${this.cartridgeId}_${key}.json`
          }, '*');
        }
        return true;
      } catch (e) {
        return false;
      }
    },

    getHighScore() {
      return this.get('high_score', 0);
    },

    setHighScore(score) {
      const current = this.getHighScore();
      if (score > current) {
        this.set('high_score', score);
        return true;
      }
      return false;
    }
  };

  // --- 4. IPC & Live 60FPS Telemetry Reporter (Feeds KDirector & KTaskMgr) ---
  const TelemetryReporter = {
    fps: 60,
    _frames: 0,
    _lastTime: (typeof performance !== 'undefined' ? performance.now() : Date.now()),
    _rafId: null,
    _running: false,
    cartridgeId: 'kilo-app',

    init(cartridgeId) {
      if (cartridgeId) this.cartridgeId = cartridgeId;
      this.start();

      document.addEventListener('visibilitychange', () => {
        if (document.hidden) {
          this.stop();
        } else {
          this.start();
        }
      });
    },

    start() {
      if (this._running || typeof requestAnimationFrame === 'undefined') return;
      this._running = true;
      this._lastTime = performance.now();
      this._frames = 0;

      const loop = (now) => {
        if (!this._running) return;
        this._frames++;

        if (now - this._lastTime >= 1000) {
          this.fps = Math.round((this._frames * 1000) / (now - this._lastTime));
          this._frames = 0;
          this._lastTime = now;

          const mem = (typeof performance !== 'undefined' && performance.memory)
            ? Math.round(performance.memory.usedJSHeapSize / 1024)
            : 2048;

          const telemetryData = {
            appId: this.cartridgeId,
            fps: this.fps,
            memoryKB: mem,
            timestamp: Date.now()
          };

          // 1. Post to host KiloOS
          if (window.self !== window.top) {
            window.parent.postMessage({
              type: 'OS_TELEMETRY_BEAT',
              ...telemetryData
            }, '*');
          }

          // 2. Direct origin localStorage write for sibling apps (KDirector & KTaskMgr)
          try {
            localStorage.setItem(`kilo_telemetry_${this.cartridgeId}`, JSON.stringify(telemetryData));
          } catch (_) {}
        }

        this._rafId = requestAnimationFrame(loop);
      };

      this._rafId = requestAnimationFrame(loop);
    },

    stop() {
      this._running = false;
      if (this._rafId) {
        cancelAnimationFrame(this._rafId);
        this._rafId = null;
      }
    }
  };

  // --- 5. Main KiloSDK Interface ---
  const KiloSDK = {
    version: '2.0.0',
    audio: AudioEngine,
    input: InputManager,
    storage: StorageBridge,
    telemetry: TelemetryReporter,
    isEmbedded: typeof window !== 'undefined' && window.self !== window.top,
    manifest: null,
    onQuicksave: null,
    onQuickload: null,

    init(manifest = {}) {
      this.manifest = manifest;
      const derivedId = (typeof window !== 'undefined' && window.location.pathname)
        ? window.location.pathname.split('/').pop().replace('.html', '')
        : 'kilo-app';
      const id = manifest.id || derivedId;

      StorageBridge.init(id);
      InputManager.init(manifest.capabilities?.includes('virtual-dpad'));
      TelemetryReporter.init(id);

      // Global F5 Quicksave / F9 Quickload hooks
      if (typeof window !== 'undefined') {
        window.addEventListener('keydown', (e) => {
          if (e.key === 'F5' && typeof this.onQuicksave === 'function') {
            e.preventDefault();
            const state = this.onQuicksave();
            if (state !== undefined) {
              StorageBridge.set('quicksave', state);
              AudioEngine.sfx.coin();
              this.notify('Quicksave [F5]', `State saved for ${manifest.title || id}`);
            }
          } else if (e.key === 'F9' && typeof this.onQuickload === 'function') {
            e.preventDefault();
            StorageBridge.getAsync('quicksave').then((state) => {
              if (state !== null && state !== undefined) {
                this.onQuickload(state);
                AudioEngine.sfx.powerup();
                this.notify('Quickload [F9]', `Restored state for ${manifest.title || id}`);
              }
            });
          }
        });

        // Unlock audio on first user touch / pointer gesture
        const unlockAudio = () => {
          AudioEngine._getCtx();
          window.removeEventListener('pointerdown', unlockAudio);
          window.removeEventListener('keydown', unlockAudio);
        };
        window.addEventListener('pointerdown', unlockAudio);
        window.addEventListener('keydown', unlockAudio);
      }
    },

    saveQuick(state) {
      if (state !== undefined) {
        StorageBridge.set('quicksave', state);
        AudioEngine.sfx.coin();
        this.notify('Quicksave', 'Snapshot saved');
        return true;
      }
      return false;
    },

    async loadQuick() {
      const state = await StorageBridge.getAsync('quicksave');
      if (state !== null && state !== undefined) {
        if (typeof this.onQuickload === 'function') {
          this.onQuickload(state);
        }
        AudioEngine.sfx.powerup();
        this.notify('Quickload', 'Snapshot restored');
        return state;
      }
      return null;
    },

    launchApp(appId, path = '') {
      if (this.isEmbedded && typeof window !== 'undefined') {
        window.parent.postMessage({ type: 'OS_LAUNCH_APP', appId, path }, '*');
      } else if (typeof window !== 'undefined') {
        window.location.href = `/apps/${appId}.html${path ? '#' + path : ''}`;
      }
    },

    notify(title, message) {
      if (this.isEmbedded && typeof window !== 'undefined') {
        window.parent.postMessage({ type: 'OS_NOTIFY', title, message }, '*');
      }
    }
  };

  global.KiloSDK = KiloSDK;
  if (typeof module !== 'undefined' && module.exports) {
    module.exports = KiloSDK;
  }
})(typeof window !== 'undefined' ? window : this);