/**
 * @kiloapps/sdk - Ultra-lightweight Sub-15KB Micro-SDK for Sub-999KB Web Minigames
 * Provides procedural ZzFX audio synthesis, unified mobile/desktop input,
 * and transparent KiloOS VFS / standalone localStorage persistence.
 */
(function(global) {
  'use strict';

  // --- 1. Procedural Audio Synthesizer (ZzFX Micro-Synth) ---
  const AudioEngine = {
    ctx: null,
    muted: false,
    _getCtx() {
      if (!this.ctx) {
        const AudioCtx = window.AudioContext || window.webkitAudioContext;
        if (AudioCtx) this.ctx = new AudioCtx();
      }
      if (this.ctx && this.ctx.state === 'suspended') {
        this.ctx.resume().catch(() => {});
      }
      return this.ctx;
    },
    // Micro-synthesizer: frequency, attack, sustain, decay, type, modulation
    playTone(freq = 440, duration = 0.1, type = 'square', decay = 0.08) {
      if (this.muted) return;
      try {
        const ctx = this._getCtx();
        if (!ctx) return;
        const t = ctx.currentTime;
        const osc = ctx.createOscillator();
        const gain = ctx.createGain();
        osc.type = type;
        osc.frequency.setValueAtTime(freq, t);
        osc.frequency.exponentialRampToValueAtTime(Math.max(20, freq * 0.5), t + duration);
        gain.gain.setValueAtTime(0.12, t);
        gain.gain.exponentialRampToValueAtTime(0.001, t + duration + decay);
        osc.connect(gain);
        gain.connect(ctx.destination);
        osc.start(t);
        osc.stop(t + duration + decay);
      } catch (e) {}
    },
    sfx: {
      shoot() { AudioEngine.playTone(880, 0.06, 'square', 0.04); },
      jump() { AudioEngine.playTone(320, 0.12, 'sine', 0.08); },
      coin() { AudioEngine.playTone(987.77, 0.08, 'sine', 0.08); setTimeout(() => AudioEngine.playTone(1318.51, 0.15, 'sine', 0.1), 60); },
      explosion() { AudioEngine.playTone(120, 0.35, 'sawtooth', 0.2); },
      hit() { AudioEngine.playTone(200, 0.08, 'sawtooth', 0.05); },
      powerup() {
        [440, 554, 659, 880].forEach((f, i) => {
          setTimeout(() => AudioEngine.playTone(f, 0.08, 'triangle', 0.05), i * 50);
        });
      },
      click() { AudioEngine.playTone(1200, 0.02, 'sine', 0.02); }
    }
  };

  // --- 2. Unified Input Controller (Keyboard + Touch D-Pad + Gamepad) ---
  const InputManager = {
    keys: {},
    touchActive: false,
    virtualPadEl: null,
    init() {
      window.addEventListener('keydown', (e) => {
        this.keys[e.key] = true;
        this.keys[e.code] = true;
      });
      window.addEventListener('keyup', (e) => {
        this.keys[e.key] = false;
        this.keys[e.code] = false;
      });
      if ('ontouchstart' in window || navigator.maxTouchPoints > 0) {
        this.enableTouchControls();
      }
    },
    isDown(action) {
      const a = action.toUpperCase();
      if (a === 'LEFT') return !!(this.keys['ArrowLeft'] || this.keys['KeyA'] || this.keys['left']);
      if (a === 'RIGHT') return !!(this.keys['ArrowRight'] || this.keys['KeyD'] || this.keys['right']);
      if (a === 'UP') return !!(this.keys['ArrowUp'] || this.keys['KeyW'] || this.keys['up']);
      if (a === 'DOWN') return !!(this.keys['ArrowDown'] || this.keys['KeyS'] || this.keys['down']);
      if (a === 'ACTION' || a === 'FIRE' || a === 'JUMP') {
        return !!(this.keys['Space'] || this.keys['KeyZ'] || this.keys['KeyJ'] || this.keys['action']);
      }
      return !!this.keys[action];
    },
    enableTouchControls() {
      if (this.virtualPadEl || !document.body) return;
      const pad = document.createElement('div');
      pad.id = 'kilo-virtual-pad';
      pad.innerHTML = `
        <div style="position:fixed;bottom:16px;left:16px;z-index:9999;display:grid;grid-template-columns:repeat(3,44px);grid-gap:6px;opacity:0.65;user-select:none;">
          <div></div><button id="kvp-up" style="height:44px;background:#222;color:#0f6;border:1px solid #0f6;border-radius:6px;font-size:16px;">▲</button><div></div>
          <button id="kvp-left" style="height:44px;background:#222;color:#0f6;border:1px solid #0f6;border-radius:6px;font-size:16px;">◀</button>
          <div></div>
          <button id="kvp-right" style="height:44px;background:#222;color:#0f6;border:1px solid #0f6;border-radius:6px;font-size:16px;">▶</button>
          <div></div><button id="kvp-down" style="height:44px;background:#222;color:#0f6;border:1px solid #0f6;border-radius:6px;font-size:16px;">▼</button><div></div>
        </div>
        <div style="position:fixed;bottom:20px;right:20px;z-index:9999;display:flex;gap:12px;opacity:0.65;user-select:none;">
          <button id="kvp-act" style="width:58px;height:58px;border-radius:50%;background:#e11d48;color:#fff;font-weight:bold;border:2px solid #fff;font-size:16px;">A</button>
        </div>
      `;
      document.body.appendChild(pad);
      this.virtualPadEl = pad;

      const bind = (id, key) => {
        const btn = document.getElementById(id);
        if (!btn) return;
        const set = (val) => { this.keys[key] = val; };
        btn.addEventListener('touchstart', (e) => { e.preventDefault(); set(true); }, { passive: false });
        btn.addEventListener('touchend', (e) => { e.preventDefault(); set(false); }, { passive: false });
      };
      bind('kvp-up', 'ArrowUp');
      bind('kvp-down', 'ArrowDown');
      bind('kvp-left', 'ArrowLeft');
      bind('kvp-right', 'ArrowRight');
      bind('kvp-act', 'Space');
    }
  };

  // --- 3. Storage & VFS Bridge (KiloOS Iframe or Standalone localStorage) ---
  const StorageBridge = {
    cartridgeId: 'kilo-app',
    isEmbedded: window.self !== window.top,
    init(id) {
      if (id) this.cartridgeId = id;
    },
    get(key, defaultValue = null) {
      try {
        const val = localStorage.getItem(`${this.cartridgeId}_${key}`);
        return val !== null ? JSON.parse(val) : defaultValue;
      } catch (e) {
        return defaultValue;
      }
    },
    set(key, value) {
      try {
        localStorage.setItem(`${this.cartridgeId}_${key}`, JSON.stringify(value));
        // If embedded in KiloOS, inform host of save state
        if (this.isEmbedded) {
          window.parent.postMessage({
            type: 'VFS_WRITE',
            requestId: Date.now(),
            path: `/saves/${this.cartridgeId}_${key}.json`,
            data: JSON.stringify(value)
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

  // --- 4. Main KiloSDK Interface ---
  const KiloSDK = {
    version: '1.0.0',
    audio: AudioEngine,
    input: InputManager,
    storage: StorageBridge,
    isEmbedded: window.self !== window.top,
    manifest: null,

    init(manifest = {}) {
      this.manifest = manifest;
      if (manifest.id) StorageBridge.init(manifest.id);
      InputManager.init();
      // Unlock audio on first user touch / click
      const unlockAudio = () => {
        AudioEngine._getCtx();
        window.removeEventListener('pointerdown', unlockAudio);
        window.removeEventListener('keydown', unlockAudio);
      };
      window.addEventListener('pointerdown', unlockAudio);
      window.addEventListener('keydown', unlockAudio);
      console.log(`[KiloSDK v${this.version}] Cartridge initialized: ${manifest.title || manifest.id || 'unnamed'}`);
    }
  };

  global.KiloSDK = KiloSDK;
  if (typeof module !== 'undefined' && module.exports) {
    module.exports = KiloSDK;
  }
})(typeof window !== 'undefined' ? window : this);