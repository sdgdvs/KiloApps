/**
 * Retro Firebase Multiplayer Service (RFMS v2.0)
 * Standardized real-time multiplayer, ephemeral room matchmaking,
 * automatic presence tracking, stale room cleanup, 25-second Mandate 12 bot fallback,
 * and offline local BroadcastChannel peer-to-peer synchronization across KiloApps.
 *
 * Zero external bundler dependencies. Universal UMD + ES module compatibility.
 */

(function (global, factory) {
  const res = factory();
  if (typeof exports === 'object' && typeof module !== 'undefined') {
    module.exports = res;
    module.exports.default = res;
    module.exports.RetroMultiplayer = res;
  }
  if (typeof define === 'function' && define.amd) {
    define(function () { return res; });
  }
  global.RetroMultiplayer = res;
  if (typeof window !== 'undefined') {
    window.RetroMultiplayer = res;
  }
})(typeof globalThis !== 'undefined' ? globalThis : typeof window !== 'undefined' ? window : this, function () {
  'use strict';

  const RFMS_VERSION = '2.0.0';

  const DEFAULT_CONFIG = {
    apiKey: "AIzaSyDns9KBDxyd4v-TbAvi5xLrVkbXaUt_9GE",
    authDomain: "kiloappschat.firebaseapp.com",
    databaseURL: "https://kiloappschat-default-rtdb.firebaseio.com",
    projectId: "kiloappschat",
    storageBucket: "kiloappschat.firebasestorage.app",
    messagingSenderId: "290208566057",
    appId: "1:290208566057:web:deacd8c7457d0cc7ec0538"
  };

  const STALE_ROOM_THRESHOLD_MS = 10 * 60 * 1000; // 10 minutes (Requirement 3)
  const DEFAULT_SOLO_TIMEOUT_SEC = 25; // 25 seconds (Requirement 2, Mandate 12)

  // =========================================================================
  // 1. IN-MEMORY & BROADCASTCHANNEL OFFLINE SHIM (Requirement 4)
  // =========================================================================

  function createOfflineFbShim() {
    const store = {};
    const listeners = new Map(); // path -> Set of callbacks
    const bus = (typeof BroadcastChannel !== 'undefined') ? new BroadcastChannel('kilo_rfms_fb_sync') : null;

    function getPath(obj, path) {
      if (!path) return obj;
      const parts = String(path).split('/').filter(Boolean);
      let curr = obj;
      for (const p of parts) {
        if (curr === undefined || curr === null) return undefined;
        curr = curr[p];
      }
      return curr;
    }

    function setPath(obj, path, val) {
      const parts = String(path).split('/').filter(Boolean);
      let curr = obj;
      for (let i = 0; i < parts.length - 1; i++) {
        const p = parts[i];
        if (!curr[p] || typeof curr[p] !== 'object') curr[p] = {};
        curr = curr[p];
      }
      const last = parts[parts.length - 1];
      if (last === undefined) return;
      if (val === null || val === undefined) {
        delete curr[last];
      } else {
        try {
          curr[last] = JSON.parse(JSON.stringify(val));
        } catch (e) {
          curr[last] = val;
        }
      }
    }

    function notify(path, val) {
      const normPath = String(path).replace(/^\/+|\/+$/g, '');
      listeners.forEach((callbacks, listenPath) => {
        const normListen = String(listenPath).replace(/^\/+|\/+$/g, '');
        if (normPath.startsWith(normListen) || normListen.startsWith(normPath) || normListen === '') {
          const currentVal = getPath(store, normListen);
          const snap = {
            val: () => (currentVal !== undefined ? JSON.parse(JSON.stringify(currentVal)) : null),
            exists: () => currentVal !== undefined && currentVal !== null,
            key: normListen.split('/').pop() || null
          };
          callbacks.forEach(cb => {
            try { cb(snap); } catch (e) { console.error('[RFMS:OfflineShim] Listener error:', e); }
          });
        }
      });
    }

    if (bus) {
      bus.onmessage = (event) => {
        const msg = event.data;
        if (msg && msg.type === 'RFMS_SHIM_SYNC') {
          setPath(store, msg.path, msg.val);
          notify(msg.path, msg.val);
        }
      };
    }

    return {
      _isOffline: true,
      app: { name: '[KiloApps-Offline-App]' },
      db: { _offline: true },
      ref: (db, path = '') => ({ path: String(path) }),
      set: async (refObj, val) => {
        const p = refObj && refObj.path !== undefined ? refObj.path : String(refObj);
        setPath(store, p, val);
        notify(p, val);
        if (bus) {
          try {
            bus.postMessage({ type: 'RFMS_SHIM_SYNC', path: p, val });
          } catch (e) {}
        }
        return true;
      },
      get: async (refObj) => {
        const p = refObj && refObj.path !== undefined ? refObj.path : String(refObj);
        const val = getPath(store, p);
        return {
          val: () => (val !== undefined ? JSON.parse(JSON.stringify(val)) : null),
          exists: () => val !== undefined && val !== null,
          key: String(p).split('/').pop() || null
        };
      },
      push: (refObj, val) => {
        const p = refObj && refObj.path !== undefined ? refObj.path : String(refObj);
        const key = 'k_' + Date.now().toString(36) + '_' + Math.random().toString(36).substring(2, 7);
        const childPath = p ? `${p}/${key}` : key;
        if (val !== undefined) {
          setPath(store, childPath, val);
          notify(childPath, val);
          if (bus) {
            try {
              bus.postMessage({ type: 'RFMS_SHIM_SYNC', path: childPath, val });
            } catch (e) {}
          }
        }
        return { path: childPath, key };
      },
      onValue: (refObj, callback) => {
        const p = refObj && refObj.path !== undefined ? refObj.path : String(refObj);
        if (!listeners.has(p)) listeners.set(p, new Set());
        listeners.get(p).add(callback);

        // Immediate snapshot dispatch
        const val = getPath(store, p);
        try {
          callback({
            val: () => (val !== undefined ? JSON.parse(JSON.stringify(val)) : null),
            exists: () => val !== undefined && val !== null,
            key: String(p).split('/').pop() || null
          });
        } catch (e) {}

        return callback;
      },
      onChildAdded: (refObj, callback) => {
        return callback;
      },
      off: (refObj, type, callback) => {
        const p = refObj && refObj.path !== undefined ? refObj.path : String(refObj);
        if (listeners.has(p)) {
          if (callback) {
            listeners.get(p).delete(callback);
          } else {
            listeners.delete(p);
          }
        }
      },
      remove: async (refObj) => {
        const p = refObj && refObj.path !== undefined ? refObj.path : String(refObj);
        setPath(store, p, null);
        notify(p, null);
        if (bus) {
          try {
            bus.postMessage({ type: 'RFMS_SHIM_SYNC', path: p, val: null });
          } catch (e) {}
        }
        return true;
      },
      serverTimestamp: () => Date.now(),
      onDisconnect: (refObj) => ({
        set: async () => {},
        remove: async () => {}
      })
    };
  }

  // =========================================================================
  // 2. SELF-CONTAINED FIREBASE DYNAMIC LOADER (Requirement 1)
  // =========================================================================

  let fbLoaderPromise = null;

  async function loadFirebase() {
    if (typeof window === 'undefined') return null;
    if (window.fb && window.fb.db && !window.fb._isOffline) return window.fb;
    if (fbLoaderPromise) return fbLoaderPromise;

    fbLoaderPromise = (async () => {
      try {
        const { initializeApp, getApps, getApp } = await import("https://www.gstatic.com/firebasejs/10.9.0/firebase-app.js");
        const {
          getDatabase,
          ref,
          set,
          get,
          push,
          onValue,
          onChildAdded,
          off,
          remove,
          serverTimestamp,
          onDisconnect
        } = await import("https://www.gstatic.com/firebasejs/10.9.0/firebase-database.js");

        const app = (getApps && getApps().length > 0) ? getApp() : initializeApp(DEFAULT_CONFIG);
        const db = getDatabase(app);

        window.fb = {
          app,
          db,
          ref,
          set,
          get,
          push,
          onValue,
          onChildAdded,
          off,
          remove,
          serverTimestamp,
          onDisconnect,
          _isOffline: false
        };

        // Fire onFirebaseLoaded callback if defined
        if (typeof window.onFirebaseLoaded === 'function') {
          try { window.onFirebaseLoaded(); } catch (e) { console.error('[RFMS] onFirebaseLoaded error:', e); }
        }

        // Dispatch CustomEvent 'rfms-ready'
        window.dispatchEvent(new CustomEvent('rfms-ready', {
          detail: { fb: window.fb, mode: 'firebase', version: RFMS_VERSION }
        }));

        return window.fb;
      } catch (err) {
        console.warn('[RFMS] Firebase dynamic import unavailable / offline. Engaging BroadcastChannel offline shim:', err);
        window.fb = createOfflineFbShim();

        if (typeof window.onFirebaseLoaded === 'function') {
          try { window.onFirebaseLoaded(); } catch (e) { console.error('[RFMS] onFirebaseLoaded error:', e); }
        }

        window.dispatchEvent(new CustomEvent('rfms-ready', {
          detail: { fb: window.fb, mode: 'broadcast', version: RFMS_VERSION, error: err }
        }));

        return window.fb;
      }
    })();

    return fbLoaderPromise;
  }

  // Intercept window.onFirebaseLoaded so late assignments trigger immediately if fb is already initialized
  if (typeof window !== 'undefined') {
    let _lateCallback = window.onFirebaseLoaded;
    try {
      Object.defineProperty(window, 'onFirebaseLoaded', {
        configurable: true,
        enumerable: true,
        get() {
          return _lateCallback;
        },
        set(fn) {
          _lateCallback = fn;
          if (window.fb && typeof fn === 'function') {
            setTimeout(() => {
              try { fn(); } catch (err) { console.error('[RFMS] onFirebaseLoaded error:', err); }
            }, 0);
          }
        }
      });
    } catch (e) {
      // Ignore if Object.defineProperty is restricted
    }

    // Auto-trigger background dynamic import on script evaluation if window.fb is missing
    if (!window.fb) {
      loadFirebase();
    }
  }

  // =========================================================================
  // UTILITY HELPERS
  // =========================================================================

  function generateRoomCode(prefix = '') {
    const chars = 'ABCDEFGHJKLMNPQRSTUVWXYZ23456789';
    let code = '';
    for (let i = 0; i < 4; i++) {
      code += chars.charAt(Math.floor(Math.random() * chars.length));
    }
    return prefix ? `${prefix}-${code}` : code;
  }

  function getUrlParam(key) {
    if (typeof window === 'undefined' || !window.location) return null;
    if (typeof URLSearchParams !== 'undefined') {
      const urlParams = new URLSearchParams(window.location.search || '');
      if (urlParams.has(key)) return urlParams.get(key);
    }
    const hash = (window.location.hash || '').replace(/^#/, '');
    if (hash.startsWith(key + '=')) {
      return hash.split('=')[1];
    }
    if (key === 'room' && /^[A-Za-z0-9_-]{3,12}$/.test(hash)) {
      return hash;
    }
    return null;
  }

  // =========================================================================
  // 3. RETROMULTIPLAYER CLASS DEFINITION
  // =========================================================================

  class RetroMultiplayer {
    constructor(options = {}) {
      this.gameId = options.gameId || 'game';
      this.prefix = options.prefix || this.gameId.slice(0, 3).toUpperCase();
      this.playerName = options.playerName ||
        (typeof localStorage !== 'undefined' ? localStorage.getItem(`kilo_${this.gameId}_name`) : null) ||
        ('Player_' + Math.floor(1000 + Math.random() * 9000));
      this.soloTimeoutMs = options.soloTimeoutMs || (DEFAULT_SOLO_TIMEOUT_SEC * 1000);
      this.basePath = `multiplayer/${this.gameId}`;

      // Callbacks
      this.onStateChange = options.onStateChange || (() => {});
      this.onMove = options.onMove || (() => {});
      this.onOpponentJoin = options.onOpponentJoin || (() => {});
      this.onOpponentLeave = options.onOpponentLeave || (() => {});
      this.onChat = options.onChat || (() => {});
      this.onRematch = options.onRematch || (() => {});
      this.onSoloFallback = options.onSoloFallback || (() => {});
      this.onStatusMsg = options.onStatusMsg || (() => {});

      // Runtime State
      this.fb = options.fb || (typeof window !== 'undefined' ? window.fb : null);
      this.roomId = null;
      this.mySlot = null; // 'p1' | 'p2' | 'spectator'
      this.isHost = false;
      this.status = 'idle'; // 'idle' | 'waiting' | 'playing' | 'finished'
      this.opponent = null;
      this.fallbackTimer = null;
      this.fallbackStartTime = 0;
      this.activeRoomRef = null;
      this.activeRoomListener = null;
      this.isOnlineAvailable = false;
      this.transport = 'firebase'; // 'firebase' | 'broadcast'
      this.lastMoveTs = 0;
      this.customData = null;
      this.createdAt = 0;
      this.pendingJoinCode = null;
      this._opponentLeaveFired = false;
      this.myRematchRequested = false;

      // Local BroadcastChannel instance for offline synchronization (Requirement 4)
      this.clientId = 'client_' + Math.random().toString(36).slice(2, 9) + '_' + Date.now();
      this.channelName = 'kilo_rfms_' + this.gameId;
      this.bc = (typeof BroadcastChannel !== 'undefined') ? new BroadcastChannel(this.channelName) : null;
      this.discoveredBroadcastRooms = {};

      if (this.bc) {
        this.bc.onmessage = (event) => this.handleBroadcastMessage(event.data);
      }

      // Cleanup on window unload
      if (typeof window !== 'undefined' && typeof window.addEventListener === 'function') {
        window.addEventListener('beforeunload', () => {
          if (this.roomId && this.status === 'playing') {
            this.leaveRoom(true);
          }
        });
      }
    }

    /**
     * Initializes Firebase connection or offline BroadcastChannel transport.
     */
    async init() {
      // 1. Resolve Firebase or offline shim
      if (!this.fb || !this.fb.db || this.fb._isOffline) {
        this.fb = await loadFirebase();
      }

      if (this.fb && this.fb.db && !this.fb._isOffline) {
        this.isOnlineAvailable = true;
        this.transport = 'firebase';
      } else {
        // BroadcastChannel fallback
        this.isOnlineAvailable = Boolean(this.bc);
        this.transport = 'broadcast';
      }

      // 2. Check for URL invite code
      const inviteCode = getUrlParam('room');
      return {
        onlineAvailable: this.isOnlineAvailable,
        transport: this.transport,
        inviteCode: inviteCode || null
      };
    }

    isOnline() {
      return Boolean(this.isOnlineAvailable && this.fb && this.fb.db && !this.fb._isOffline);
    }

    /**
     * Start the 25-second solo fallback timer (Mandate 12 compliant).
     */
    startSoloFallbackTimer() {
      this.stopSoloFallbackTimer();
      this.fallbackStartTime = Date.now();
      const timeoutSec = Math.round(this.soloTimeoutMs / 1000);

      this.fallbackTimer = setTimeout(() => {
        if (this.status === 'waiting') {
          this.onStatusMsg(`No peer connected in ${timeoutSec}s. Engaging CyberBot-99...`, 'warning');
          this.leaveRoom(true);
          this.onSoloFallback({
            reason: 'timeout',
            duration: timeoutSec,
            botName: 'CyberBot-99',
            elo: 1200,
            isBot: true
          });
        }
      }, this.soloTimeoutMs);
    }

    stopSoloFallbackTimer() {
      if (this.fallbackTimer) {
        clearTimeout(this.fallbackTimer);
        this.fallbackTimer = null;
      }
    }

    /**
     * Requirement 3: Scans lobby and cleans up stale rooms older than 10 minutes.
     */
    async cleanupStaleRooms(lobbyData = null) {
      if (!this.isOnline() || !this.fb) return;
      const now = Date.now();

      try {
        let lobby = lobbyData;
        if (!lobby) {
          const snap = await this.fb.get(this.fb.ref(this.fb.db, `${this.basePath}/lobby`));
          lobby = snap.val();
        }

        if (lobby && typeof lobby === 'object') {
          const prunePromises = [];
          for (const [code, entry] of Object.entries(lobby)) {
            const createdAt = (entry && (entry.createdAt || entry.updatedAt)) || 0;
            if (createdAt && (now - createdAt > STALE_ROOM_THRESHOLD_MS)) {
              prunePromises.push(this.fb.set(this.fb.ref(this.fb.db, `${this.basePath}/lobby/${code}`), null));
              prunePromises.push(this.fb.set(this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}`), null));
              delete lobby[code];
            }
          }
          await Promise.allSettled(prunePromises);
        }
      } catch (err) {
        console.warn(`[RetroMultiplayer:${this.gameId}] cleanupStaleRooms error:`, err);
      }
    }

    /**
     * Requirement 4: Handles incoming BroadcastChannel events for offline peer play.
     */
    handleBroadcastMessage(msg) {
      if (!msg || typeof msg !== 'object') return;
      if (msg.fromClient === this.clientId) return; // Ignore own messages

      switch (msg.type) {
        case 'RFMS_LOBBY_QUERY':
          if (this.isHost && this.status === 'waiting' && this.roomId) {
            this.broadcastPost({
              type: 'RFMS_LOBBY_ANNOUNCE',
              roomId: this.roomId,
              host: this.playerName,
              customData: this.customData,
              createdAt: this.createdAt
            });
          }
          break;

        case 'RFMS_LOBBY_ANNOUNCE':
          if (msg.roomId) {
            this.discoveredBroadcastRooms[msg.roomId] = msg;
          }
          break;

        case 'RFMS_JOIN_REQUEST':
          if (this.isHost && this.status === 'waiting' && this.roomId === msg.roomId) {
            this.status = 'playing';
            this.opponent = msg.playerName;
            this.stopSoloFallbackTimer();
            this._opponentLeaveFired = false;

            const roomPayload = {
              status: 'playing',
              p1: { name: this.playerName, connected: true },
              p2: { name: msg.playerName, connected: true },
              turn: 'p1',
              customData: this.customData
            };

            this.broadcastPost({
              type: 'RFMS_JOIN_ACCEPT',
              roomId: this.roomId,
              toClient: msg.fromClient,
              room: roomPayload
            });

            this.onStatusMsg(`Player 2 (${this.opponent}) connected via Local Subnet! Match started.`, 'success');
            this.onOpponentJoin({ slot: 'p2', name: this.opponent });
            this.onStateChange(roomPayload);
          }
          break;

        case 'RFMS_JOIN_ACCEPT':
          if (msg.toClient === this.clientId && msg.roomId === this.pendingJoinCode) {
            this.roomId = msg.roomId;
            this.mySlot = 'p2';
            this.isHost = false;
            this.status = 'playing';
            this.opponent = (msg.room && msg.room.p1) ? msg.room.p1.name : 'Host';
            this.stopSoloFallbackTimer();
            this._opponentLeaveFired = false;

            if (typeof window !== 'undefined' && window.history && window.history.replaceState) {
              window.history.replaceState(null, '', `#room=${this.roomId}`);
            }

            this.onStatusMsg(`Connected to ${this.opponent} in [${this.roomId}] via Local Subnet!`, 'success');
            this.onOpponentJoin({ slot: 'p1', name: this.opponent });
            this.onStateChange(msg.room);
          }
          break;

        case 'RFMS_MOVE':
          if (msg.roomId === this.roomId && msg.fromSlot !== this.mySlot) {
            this.lastMoveTs = msg.ts || Date.now();
            this.onMove(msg.data, { by: msg.fromSlot, data: msg.data, ts: msg.ts });
            if (msg.roomState) {
              this.onStateChange(msg.roomState);
            }
          }
          break;

        case 'RFMS_CHAT':
          if (msg.roomId === this.roomId && msg.fromSlot !== this.mySlot) {
            this.onChat({ by: msg.fromSlot, name: msg.name, text: msg.text, ts: msg.ts });
          }
          break;

        case 'RFMS_REMATCH':
          if (msg.roomId === this.roomId && msg.fromSlot !== this.mySlot) {
            if (this.myRematchRequested) {
              this.broadcastPost({ type: 'RFMS_REMATCH_CONFIRM', roomId: this.roomId });
              this.onStatusMsg('Rematch accepted! Starting new round...', 'success');
              this.onStateChange({ status: 'playing', rematch: { p1: false, p2: false } });
            } else {
              this.onRematch({ requestedBy: msg.fromSlot });
            }
          }
          break;

        case 'RFMS_REMATCH_CONFIRM':
          if (msg.roomId === this.roomId) {
            this.myRematchRequested = false;
            this.onStatusMsg('Rematch accepted! Starting new round...', 'success');
            this.onStateChange({ status: 'playing', rematch: { p1: false, p2: false } });
          }
          break;

        case 'RFMS_LEAVE':
          if (msg.roomId === this.roomId && msg.fromSlot !== this.mySlot) {
            if (!this._opponentLeaveFired) {
              this._opponentLeaveFired = true;
              this.onStatusMsg(`Opponent (${msg.name || msg.fromSlot}) disconnected.`, 'warning');
              this.onOpponentLeave({ slot: msg.fromSlot, name: msg.name || msg.fromSlot });
            }
          }
          break;
      }
    }

    broadcastPost(msg) {
      if (this.bc) {
        try {
          this.bc.postMessage({ ...msg, fromClient: this.clientId });
        } catch (e) {}
      }
    }

    /**
     * Creates a new multiplayer room.
     * In online mode: registers onDisconnect().remove() on p1 and lobby (Requirement 3).
     * In offline mode: announces over BroadcastChannel (Requirement 4).
     */
    async createRoom(customData = {}, isPublic = true) {
      const code = (customData && customData.roomId) ? customData.roomId.toUpperCase() : generateRoomCode(this.prefix);
      this.roomId = code;
      this.mySlot = 'p1';
      this.isHost = true;
      this.status = 'waiting';
      this.opponent = null;
      this.customData = customData;
      this.createdAt = Date.now();
      this._opponentLeaveFired = false;
      this.myRematchRequested = false;

      // BroadcastChannel announcement
      this.broadcastPost({
        type: 'RFMS_LOBBY_ANNOUNCE',
        roomId: code,
        host: this.playerName,
        customData,
        createdAt: this.createdAt
      });

      if (!this.isOnline()) {
        this.startSoloFallbackTimer();
        if (typeof window !== 'undefined' && window.history && window.history.replaceState) {
          window.history.replaceState(null, '', `#room=${code}`);
        }
        this.onStatusMsg(`Local Room [${code}] open! Waiting for peer (or AI in 25s)...`, 'info');
        return { success: true, roomId: code, slot: 'p1' };
      }

      const roomPayload = {
        id: code,
        status: 'waiting',
        p1: { name: this.playerName, connected: true, joinedAt: Date.now() },
        p2: null,
        turn: 'p1',
        winner: null,
        rematch: { p1: false, p2: false },
        chat: null,
        lastMove: null,
        isPublic: Boolean(isPublic),
        createdAt: Date.now(),
        updatedAt: Date.now(),
        ...customData
      };

      try {
        const roomRef = this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}`);
        await this.fb.set(roomRef, roomPayload);

        // Public lobby listing
        if (isPublic) {
          const lobbyRef = this.fb.ref(this.fb.db, `${this.basePath}/lobby/${code}`);
          await this.fb.set(lobbyRef, {
            id: code,
            host: this.playerName,
            createdAt: Date.now()
          });
        }

        // Requirement 3: Automatic Presence Cleanup - onDisconnect().remove() on room player node and lobby
        try {
          if (this.fb.onDisconnect) {
            this.fb.onDisconnect(this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}/p1`)).remove();
            if (isPublic) {
              this.fb.onDisconnect(this.fb.ref(this.fb.db, `${this.basePath}/lobby/${code}`)).remove();
            }
          }
        } catch (e) {}

        this.attachRoomListener(code);
        this.startSoloFallbackTimer();

        if (typeof window !== 'undefined' && window.history && window.history.replaceState) {
          window.history.replaceState(null, '', `#room=${code}`);
        }

        this.onStatusMsg(`Room [${code}] open! Waiting for opponent (or AI in 25s)...`, 'info');
        return { success: true, roomId: code, slot: 'p1' };
      } catch (err) {
        console.error(`[RetroMultiplayer:${this.gameId}] createRoom failed:`, err);
        // Fallback to offline mode
        this.startSoloFallbackTimer();
        this.onStatusMsg(`Subnet room [${code}] opened in offline mode. Waiting for local peer...`, 'warning');
        return { success: true, roomId: code, slot: 'p1', offline: true };
      }
    }

    /**
     * Joins an existing room by code.
     * In online mode: assigns p2 and registers onDisconnect().remove() on p2 (Requirement 3).
     * In offline mode: broadcasts join request via BroadcastChannel (Requirement 4).
     */
    async joinRoom(roomId) {
      const code = (roomId || '').trim().toUpperCase();
      if (!code) return { success: false, reason: 'invalid_code' };

      this.pendingJoinCode = code;
      this._opponentLeaveFired = false;
      this.myRematchRequested = false;

      // 1. Try Firebase online join if online
      if (this.isOnline()) {
        try {
          const roomRef = this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}`);
          const snap = await this.fb.get(roomRef);
          const room = snap.val();

          if (room) {
            this.roomId = code;

            if (room.status === 'waiting' || !room.p2) {
              this.mySlot = 'p2';
              this.isHost = false;
              this.status = 'playing';
              this.opponent = room.p1 ? room.p1.name : 'Host';

              await this.fb.set(this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}/p2`), {
                name: this.playerName,
                connected: true,
                joinedAt: Date.now()
              });
              await this.fb.set(this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}/status`), 'playing');
              await this.fb.set(this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}/updatedAt`), Date.now());

              // Remove from lobby
              try {
                await this.fb.set(this.fb.ref(this.fb.db, `${this.basePath}/lobby/${code}`), null);
              } catch (e) {}

              // Requirement 3: Automatic Presence Cleanup - onDisconnect().remove() on p2 node
              try {
                if (this.fb.onDisconnect) {
                  this.fb.onDisconnect(this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}/p2`)).remove();
                }
              } catch (e) {}

              this.stopSoloFallbackTimer();
              this.attachRoomListener(code);

              if (typeof window !== 'undefined' && window.history && window.history.replaceState) {
                window.history.replaceState(null, '', `#room=${code}`);
              }

              this.onStatusMsg(`Connected to ${this.opponent} in [${code}]!`, 'success');
              this.onOpponentJoin({ slot: 'p1', name: this.opponent });
              return { success: true, roomId: code, slot: 'p2', room };
            } else {
              // Spectator
              this.mySlot = 'spectator';
              this.isHost = false;
              this.status = room.status;
              this.attachRoomListener(code);
              this.onStatusMsg(`Joined room [${code}] as Spectator.`, 'info');
              return { success: true, roomId: code, slot: 'spectator', room };
            }
          }
        } catch (err) {
          console.warn(`[RetroMultiplayer:${this.gameId}] joinRoom online probe error, trying BroadcastChannel:`, err);
        }
      }

      // 2. Fallback to Local BroadcastChannel (Requirement 4)
      if (this.bc) {
        this.onStatusMsg(`Searching local subnet for [${code}]...`, 'info');
        this.broadcastPost({
          type: 'RFMS_JOIN_REQUEST',
          roomId: code,
          playerName: this.playerName
        });

        const acceptWait = await new Promise((resolve) => {
          let resolved = false;
          const handler = (event) => {
            const m = event.data;
            if (m && m.type === 'RFMS_JOIN_ACCEPT' && m.toClient === this.clientId && m.roomId === code) {
              if (!resolved) {
                resolved = true;
                if (this.bc) this.bc.removeEventListener('message', handler);
                resolve({ success: true, room: m.room });
              }
            }
          };
          if (this.bc) this.bc.addEventListener('message', handler);
          setTimeout(() => {
            if (!resolved) {
              resolved = true;
              if (this.bc) this.bc.removeEventListener('message', handler);
              resolve({ success: false });
            }
          }, 1500);
        });

        if (acceptWait.success) {
          return { success: true, roomId: code, slot: 'p2', room: acceptWait.room };
        }
      }

      this.onStatusMsg(`Room [${code}] not found.`, 'warning');
      return { success: false, reason: 'not_found' };
    }

    /**
     * Automatic matchmaking: searches public lobby, cleans stale rooms older than 10m (Requirement 3),
     * or queries local BroadcastChannel peers (Requirement 4).
     */
    async quickMatch(customData = {}) {
      this.onStatusMsg('Scanning subnet for open games...', 'info');

      // 1. Online lobby scan if Firebase is available
      if (this.isOnline()) {
        try {
          const lobbyRef = this.fb.ref(this.fb.db, `${this.basePath}/lobby`);
          const snap = await this.fb.get(lobbyRef);
          const lobby = snap.val() || {};

          // Requirement 3: Clean up stale rooms older than 10 minutes
          await this.cleanupStaleRooms(lobby);

          const openCodes = Object.keys(lobby);
          if (openCodes.length > 0) {
            const bestCode = openCodes[openCodes.length - 1];
            this.onStatusMsg(`Found open game [${bestCode}]! Connecting...`, 'info');
            const joinRes = await this.joinRoom(bestCode);
            if (joinRes && joinRes.success) {
              return joinRes;
            }
          }
        } catch (err) {
          console.warn(`[RetroMultiplayer:${this.gameId}] quickMatch online error:`, err);
        }
      }

      // 2. BroadcastChannel probe for open local rooms (Requirement 4)
      if (this.bc) {
        this.discoveredBroadcastRooms = {};
        this.broadcastPost({
          type: 'RFMS_LOBBY_QUERY',
          gameId: this.gameId
        });

        // Wait 350ms for local hosts to respond
        await new Promise((r) => setTimeout(r, 350));

        const localCodes = Object.keys(this.discoveredBroadcastRooms);
        if (localCodes.length > 0) {
          const targetCode = localCodes[localCodes.length - 1];
          this.onStatusMsg(`Found local game [${targetCode}]! Connecting...`, 'info');
          const localJoin = await this.joinRoom(targetCode);
          if (localJoin && localJoin.success) {
            return localJoin;
          }
        }
      }

      // 3. No open room found -> Host a new public room
      return await this.createRoom(customData, true);
    }

    /**
     * Attaches live RTDB value listener to the active room.
     */
    attachRoomListener(code) {
      this.detachRoomListener();
      if (!this.isOnline()) return;

      this.activeRoomRef = this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}`);
      this.activeRoomListener = (snap) => {
        const room = snap.val();
        if (!room) {
          if (this.status !== 'idle') {
            this.onStatusMsg('Room closed by host.', 'warning');
            this.leaveRoom(false);
          }
          return;
        }

        // Detect opponent joining
        if (this.isHost && this.status === 'waiting' && room.status === 'playing' && room.p2) {
          this.status = 'playing';
          this.opponent = room.p2.name;
          this.stopSoloFallbackTimer();
          this._opponentLeaveFired = false;
          this.onStatusMsg(`Player 2 (${this.opponent}) joined! Match started.`, 'success');
          this.onOpponentJoin({ slot: 'p2', name: this.opponent });
        }

        // Detect opponent disconnect / node removal (Requirement 3)
        const otherSlot = this.mySlot === 'p1' ? 'p2' : (this.mySlot === 'p2' ? 'p1' : null);
        if (otherSlot && this.status === 'playing') {
          const otherPlayer = room[otherSlot];
          if (!otherPlayer || otherPlayer.connected === false) {
            if (!this._opponentLeaveFired) {
              this._opponentLeaveFired = true;
              const oppName = (otherPlayer && otherPlayer.name) || this.opponent || otherSlot;
              this.onStatusMsg(`Opponent (${oppName}) disconnected.`, 'warning');
              this.onOpponentLeave({ slot: otherSlot, name: oppName });
            }
          } else {
            this._opponentLeaveFired = false;
          }
        }

        // Notify state update
        this.onStateChange(room);

        // Process incoming move
        if (room.lastMove && room.lastMove.ts && room.lastMove.ts > this.lastMoveTs) {
          this.lastMoveTs = room.lastMove.ts;
          if (room.lastMove.by !== this.mySlot) {
            this.onMove(room.lastMove.data, room.lastMove);
          }
        }

        // Process chat
        if (room.chat && room.chat.ts && room.chat.by !== this.mySlot) {
          this.onChat(room.chat);
        }

        // Process rematch request
        if (room.rematch && otherSlot && room.rematch[otherSlot] && !room.rematch[this.mySlot]) {
          this.onRematch({ requestedBy: otherSlot });
        }
      };

      this.fb.onValue(this.activeRoomRef, this.activeRoomListener);
    }

    detachRoomListener() {
      if (this.activeRoomRef && this.activeRoomListener && this.fb && this.fb.off) {
        try {
          this.fb.off(this.activeRoomRef, 'value', this.activeRoomListener);
        } catch (e) {}
      }
      this.activeRoomRef = null;
      this.activeRoomListener = null;
    }

    /**
     * Broadcasts a move payload to RTDB and local BroadcastChannel.
     */
    async sendMove(moveData, nextTurnSlot = null, additionalState = {}) {
      if (!this.roomId) return false;

      const now = Date.now();
      this.lastMoveTs = now;

      const payload = {
        lastMove: {
          by: this.mySlot,
          data: moveData,
          ts: now
        },
        updatedAt: now,
        ...additionalState
      };

      if (nextTurnSlot) {
        payload.turn = nextTurnSlot;
      }

      // BroadcastChannel transmission (Requirement 4)
      this.broadcastPost({
        type: 'RFMS_MOVE',
        roomId: this.roomId,
        fromSlot: this.mySlot,
        data: moveData,
        roomState: payload,
        ts: now
      });

      if (this.isOnline()) {
        try {
          const roomRef = this.fb.ref(this.fb.db, `${this.basePath}/rooms/${this.roomId}`);
          const currentSnap = await this.fb.get(roomRef);
          await this.fb.set(roomRef, {
            ...(currentSnap.val() || {}),
            ...payload
          });
          return true;
        } catch (err) {
          console.error(`[RetroMultiplayer:${this.gameId}] sendMove online error:`, err);
          return false;
        }
      }

      return true;
    }

    /**
     * Broadcasts a short quick-chat message or emote.
     */
    async sendChat(text) {
      if (!this.roomId) return false;
      const chatPayload = {
        by: this.mySlot,
        name: this.playerName,
        text: (text || '').slice(0, 80),
        ts: Date.now()
      };

      this.broadcastPost({
        type: 'RFMS_CHAT',
        roomId: this.roomId,
        fromSlot: this.mySlot,
        ...chatPayload
      });

      if (this.isOnline()) {
        try {
          const chatRef = this.fb.ref(this.fb.db, `${this.basePath}/rooms/${this.roomId}/chat`);
          await this.fb.set(chatRef, chatPayload);
          return true;
        } catch (e) {
          return false;
        }
      }
      return true;
    }

    /**
     * Submits a rematch confirmation.
     */
    async requestRematch(resetData = {}) {
      if (!this.roomId || !this.mySlot) return false;
      this.myRematchRequested = true;

      this.broadcastPost({
        type: 'RFMS_REMATCH',
        roomId: this.roomId,
        fromSlot: this.mySlot
      });

      if (this.isOnline()) {
        try {
          const rematchRef = this.fb.ref(this.fb.db, `${this.basePath}/rooms/${this.roomId}/rematch/${this.mySlot}`);
          await this.fb.set(rematchRef, true);

          const snap = await this.fb.get(this.fb.ref(this.fb.db, `${this.basePath}/rooms/${this.roomId}`));
          const room = snap.val();
          if (room && room.rematch && room.rematch.p1 && room.rematch.p2) {
            await this.fb.set(this.fb.ref(this.fb.db, `${this.basePath}/rooms/${this.roomId}`), {
              ...room,
              status: 'playing',
              winner: null,
              rematch: { p1: false, p2: false },
              lastMove: null,
              updatedAt: Date.now(),
              ...resetData
            });
            this.onStatusMsg('Rematch accepted! Starting new round...', 'success');
          } else {
            this.onStatusMsg('Rematch requested. Waiting for opponent...', 'info');
          }
          return true;
        } catch (err) {
          return false;
        }
      } else {
        this.onStatusMsg('Rematch requested on local subnet. Waiting for peer...', 'info');
        return true;
      }
    }

    /**
     * Leaves the active room and cleans up resources.
     */
    async leaveRoom(notifyServer = true) {
      this.stopSoloFallbackTimer();
      const code = this.roomId;
      const slot = this.mySlot;

      this.detachRoomListener();

      // BroadcastChannel notification (Requirement 4)
      if (code) {
        this.broadcastPost({
          type: 'RFMS_LEAVE',
          roomId: code,
          fromSlot: slot,
          name: this.playerName
        });
      }

      if (notifyServer && code && this.isOnline()) {
        try {
          if (slot === 'p1' || slot === 'p2') {
            await this.fb.set(this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}/${slot}/connected`), false);
          }
          if (this.isHost && this.status === 'waiting') {
            await this.fb.set(this.fb.ref(this.fb.db, `${this.basePath}/lobby/${code}`), null);
          }
        } catch (e) {}
      }

      this.roomId = null;
      this.mySlot = null;
      this.isHost = false;
      this.status = 'idle';
      this.opponent = null;
      this._opponentLeaveFired = false;
      this.myRematchRequested = false;

      if (typeof window !== 'undefined' && window.history && window.history.replaceState) {
        const cleanUrl = window.location.pathname + window.location.search;
        window.history.replaceState(null, '', cleanUrl);
      }

      this.onStatusMsg('Multiplayer session ended.', 'info');
    }

    /**
     * Copies the current match invite link to clipboard.
     */
    copyInviteLink() {
      if (!this.roomId || typeof window === 'undefined') return false;
      const url = `${window.location.origin}${window.location.pathname}#room=${this.roomId}`;
      if (navigator.clipboard && navigator.clipboard.writeText) {
        navigator.clipboard.writeText(url).then(() => {
          this.onStatusMsg('Invite link copied to clipboard!', 'success');
        }).catch(() => {
          this.fallbackCopy(url);
        });
      } else {
        this.fallbackCopy(url);
      }
      return url;
    }

    fallbackCopy(text) {
      const textarea = document.createElement('textarea');
      textarea.value = text;
      document.body.appendChild(textarea);
      textarea.select();
      try {
        document.execCommand('copy');
        this.onStatusMsg('Invite link copied to clipboard!', 'success');
      } catch (e) {
        this.onStatusMsg(`Match Code: ${this.roomId}`, 'info');
      }
      document.body.removeChild(textarea);
    }

    /**
     * Instance convenience wrapper for startMatchmaking (Requirement 2).
     */
    startMatchmaking(options = {}) {
      return RetroMultiplayer.startMatchmaking({ ...options, instance: this });
    }
  }

  // =========================================================================
  // 4. MANDATE 12 COMPLIANT MATCHMAKING HELPER (Requirement 2)
  // =========================================================================

  /**
   * Static helper: RetroMultiplayer.startMatchmaking({ gameId, playerName, timeoutSec: 25, onPeerJoined, onBotFallback, onTick })
   * Counts down from timeoutSec (default 25) and invokes onBotFallback({ botName: 'CyberBot-99', elo: 1200 })
   * if no live human opponent joins within timeoutSec.
   */
  RetroMultiplayer.startMatchmaking = async function (options = {}) {
    const timeoutSec = (typeof options.timeoutSec === 'number' && options.timeoutSec > 0)
      ? options.timeoutSec
      : DEFAULT_SOLO_TIMEOUT_SEC;

    const rm = options.instance || new RetroMultiplayer({
      gameId: options.gameId || 'game',
      playerName: options.playerName,
      soloTimeoutMs: timeoutSec * 1000,
      onStatusMsg: options.onStatusMsg || (() => {})
    });

    await rm.init();

    let remainingSec = timeoutSec;
    let isTerminated = false;
    let countdownInterval = null;

    const botFallbackData = {
      botName: options.botName || 'CyberBot-99',
      elo: typeof options.elo === 'number' ? options.elo : 1200,
      isBot: true,
      reason: 'timeout',
      timeoutSec
    };

    const triggerBotFallback = () => {
      if (isTerminated) return;
      isTerminated = true;
      if (countdownInterval) {
        clearInterval(countdownInterval);
        countdownInterval = null;
      }
      rm.leaveRoom(true);
      if (typeof options.onBotFallback === 'function') {
        options.onBotFallback(botFallbackData, rm);
      }
    };

    // Wrap opponent join listener
    const origJoin = rm.onOpponentJoin;
    rm.onOpponentJoin = (peer) => {
      if (isTerminated) return;
      isTerminated = true;
      if (countdownInterval) {
        clearInterval(countdownInterval);
        countdownInterval = null;
      }
      if (typeof origJoin === 'function') origJoin(peer);
      if (typeof options.onPeerJoined === 'function') {
        options.onPeerJoined(peer, rm);
      }
    };

    // Initial tick
    if (typeof options.onTick === 'function') {
      try { options.onTick(remainingSec); } catch (e) {}
    }

    // Start 1-second countdown
    countdownInterval = setInterval(() => {
      if (isTerminated) {
        if (countdownInterval) clearInterval(countdownInterval);
        return;
      }
      remainingSec--;
      if (typeof options.onTick === 'function') {
        try { options.onTick(remainingSec); } catch (e) {}
      }
      if (remainingSec <= 0) {
        triggerBotFallback();
      }
    }, 1000);

    // Initiate quickMatch search
    const matchResult = await rm.quickMatch(options.customData || {});

    // If already paired with an opponent immediately (joined room as p2)
    if (matchResult && matchResult.success && matchResult.slot === 'p2' && rm.opponent) {
      if (!isTerminated) {
        isTerminated = true;
        if (countdownInterval) {
          clearInterval(countdownInterval);
          countdownInterval = null;
        }
        if (typeof options.onPeerJoined === 'function') {
          options.onPeerJoined({ slot: 'p1', name: rm.opponent, room: matchResult.room }, rm);
        }
      }
    }

    return {
      instance: rm,
      get roomId() { return rm.roomId; },
      get status() { return rm.status; },
      cancel: () => {
        if (isTerminated) return;
        isTerminated = true;
        if (countdownInterval) {
          clearInterval(countdownInterval);
          countdownInterval = null;
        }
        rm.leaveRoom(true);
      }
    };
  };

  // Expose static members
  RetroMultiplayer.VERSION = RFMS_VERSION;
  RetroMultiplayer.DEFAULT_CONFIG = DEFAULT_CONFIG;
  RetroMultiplayer.loadFirebase = loadFirebase;
  RetroMultiplayer.ensureFirebase = loadFirebase;
  RetroMultiplayer.createOfflineFbShim = createOfflineFbShim;

  return RetroMultiplayer;
});
