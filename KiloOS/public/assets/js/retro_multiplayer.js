/**
 * RetroMultiplayer (RFMS) - Standardized Retro Firebase Multiplayer Service for KiloApps.
 * Provides ephemeral room matchmaking, state synchronization, presence tracking,
 * automatic disconnect pruning, and a 25-second solo AI fallback across KiloApps web games.
 *
 * Zero external bundler dependencies. Compatible with both ES module and global script inclusion.
 */

(function (global, factory) {
  if (typeof exports === 'object' && typeof module !== 'undefined') {
    module.exports = factory();
  } else if (typeof define === 'function' && define.amd) {
    define(factory);
  } else {
    global.RetroMultiplayer = factory();
  }
})(typeof globalThis !== 'undefined' ? globalThis : typeof window !== 'undefined' ? window : this, function () {
  'use strict';

  const DEFAULT_CONFIG = {
    apiKey: "AIzaSyDns9KBDxyd4v-TbAvi5xLrVkbXaUt_9GE",
    authDomain: "kiloappschat.firebaseapp.com",
    databaseURL: "https://kiloappschat-default-rtdb.firebaseio.com",
    projectId: "kiloappschat",
    storageBucket: "kiloappschat.firebasestorage.app",
    messagingSenderId: "290208566057",
    appId: "1:290208566057:web:deacd8c7457d0cc7ec0538"
  };

  function generateRoomCode(prefix = '') {
    const chars = 'ABCDEFGHJKLMNPQRSTUVWXYZ23456789';
    let code = '';
    for (let i = 0; i < 4; i++) {
      code += chars.charAt(Math.floor(Math.random() * chars.length));
    }
    return prefix ? `${prefix}-${code}` : code;
  }

  function getUrlParam(key) {
    if (typeof window === 'undefined') return null;
    const urlParams = new URLSearchParams(window.location.search);
    if (urlParams.has(key)) return urlParams.get(key);
    // Also check hash format: #room=CODE or #CODE
    const hash = window.location.hash.replace(/^#/, '');
    if (hash.startsWith(key + '=')) {
      return hash.split('=')[1];
    }
    if (key === 'room' && /^[A-Za-z0-9_-]{3,10}$/.test(hash)) {
      return hash;
    }
    return null;
  }

  class RetroMultiplayer {
    constructor(options = {}) {
      this.gameId = options.gameId || 'game';
      this.prefix = options.prefix || this.gameId.slice(0, 3).toUpperCase();
      this.playerName = options.playerName || localStorage.getItem(`kilo_${this.gameId}_name`) || ('Player_' + Math.floor(1000 + Math.random() * 9000));
      this.soloTimeoutMs = options.soloTimeoutMs || 25000;
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
    }

    /**
     * Initializes Firebase connection and checks for invite link in URL.
     */
    async init() {
      // 1. If window.fb already available, bind it
      if (this.fb && this.fb.db) {
        this.isOnlineAvailable = true;
      } else if (typeof window !== 'undefined' && window.fb && window.fb.db) {
        this.fb = window.fb;
        this.isOnlineAvailable = true;
      } else {
        // Dynamic import fallback if not already injected
        try {
          const { initializeApp } = await import("https://www.gstatic.com/firebasejs/10.9.0/firebase-app.js");
          const { getDatabase, ref, set, get, push, onValue, onChildAdded, off, serverTimestamp, onDisconnect } =
            await import("https://www.gstatic.com/firebasejs/10.9.0/firebase-database.js");

          const app = initializeApp(DEFAULT_CONFIG);
          const db = getDatabase(app);
          this.fb = { app, db, ref, set, get, push, onValue, onChildAdded, off, serverTimestamp, onDisconnect };
          if (typeof window !== 'undefined') window.fb = this.fb;
          this.isOnlineAvailable = true;
        } catch (err) {
          console.warn(`[RetroMultiplayer:${this.gameId}] Firebase SDK unreachable. Solo mode only.`, err);
          this.isOnlineAvailable = false;
        }
      }

      // 2. Check for URL invite code
      const inviteCode = getUrlParam('room');
      return {
        onlineAvailable: this.isOnlineAvailable,
        inviteCode: inviteCode || null
      };
    }

    /**
     * Checks if current connection is online and ready.
     */
    isOnline() {
      return Boolean(this.isOnlineAvailable && this.fb && this.fb.db);
    }

    /**
     * Start the 25-second solo fallback timer when hosting or waiting in matchmaking.
     */
    startSoloFallbackTimer() {
      this.stopSoloFallbackTimer();
      this.fallbackStartTime = Date.now();
      this.fallbackTimer = setTimeout(() => {
        if (this.status === 'waiting') {
          this.onStatusMsg('No peer connected in 25s. Engaging Subnet AI Cyber-Bot...', 'warning');
          this.leaveRoom(true);
          this.onSoloFallback({ reason: 'timeout', duration: 25 });
        }
      }, this.soloTimeoutMs);
    }

    /**
     * Clears any active solo fallback timer.
     */
    stopSoloFallbackTimer() {
      if (this.fallbackTimer) {
        clearTimeout(this.fallbackTimer);
        this.fallbackTimer = null;
      }
    }

    /**
     * Creates a new multiplayer room.
     */
    async createRoom(customData = {}, isPublic = true) {
      if (!this.isOnline()) {
        this.onStatusMsg('Firebase offline. Switching to local vs AI.', 'warning');
        this.onSoloFallback({ reason: 'offline' });
        return { success: false, reason: 'offline' };
      }

      const code = (customData && customData.roomId) ? customData.roomId.toUpperCase() : generateRoomCode(this.prefix);
      this.roomId = code;
      this.mySlot = 'p1';
      this.isHost = true;
      this.status = 'waiting';
      this.opponent = null;

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

        // OnDisconnect cleanup
        try {
          this.fb.onDisconnect(this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}/p1/connected`)).set(false);
          if (isPublic) {
            this.fb.onDisconnect(this.fb.ref(this.fb.db, `${this.basePath}/lobby/${code}`)).remove();
          }
        } catch (e) {}

        this.attachRoomListener(code);
        this.startSoloFallbackTimer();

        // Sync URL hash
        if (typeof window !== 'undefined' && window.history && window.history.replaceState) {
          window.history.replaceState(null, '', `#room=${code}`);
        }

        this.onStatusMsg(`Room [${code}] open! Waiting for opponent (or AI in 25s)...`, 'info');
        return { success: true, roomId: code, slot: 'p1' };
      } catch (err) {
        console.error(`[RetroMultiplayer:${this.gameId}] createRoom failed:`, err);
        this.onStatusMsg('Failed to open room: ' + err.message, 'error');
        this.status = 'idle';
        return { success: false, error: err.message };
      }
    }

    /**
     * Joins an existing room by code.
     */
    async joinRoom(roomId) {
      if (!this.isOnline()) {
        this.onStatusMsg('Firebase offline. Switching to local vs AI.', 'warning');
        this.onSoloFallback({ reason: 'offline' });
        return { success: false, reason: 'offline' };
      }

      const code = (roomId || '').trim().toUpperCase();
      if (!code) return { success: false, reason: 'invalid_code' };

      try {
        const roomRef = this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}`);
        const snap = await this.fb.get(roomRef);
        const room = snap.val();

        if (!room) {
          this.onStatusMsg(`Room [${code}] not found.`, 'warning');
          return { success: false, reason: 'not_found' };
        }

        this.roomId = code;

        // Slot Assignment
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

          // Remove from public lobby since match has started
          try {
            await this.fb.set(this.fb.ref(this.fb.db, `${this.basePath}/lobby/${code}`), null);
          } catch (e) {}

          // Disconnect presence hook
          try {
            this.fb.onDisconnect(this.fb.ref(this.fb.db, `${this.basePath}/rooms/${code}/p2/connected`)).set(false);
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
          // Room full -> Join as Spectator
          this.mySlot = 'spectator';
          this.isHost = false;
          this.status = room.status;
          this.attachRoomListener(code);
          this.onStatusMsg(`Joined room [${code}] as Spectator.`, 'info');
          return { success: true, roomId: code, slot: 'spectator', room };
        }
      } catch (err) {
        console.error(`[RetroMultiplayer:${this.gameId}] joinRoom failed:`, err);
        this.onStatusMsg('Failed to join room: ' + err.message, 'error');
        return { success: false, error: err.message };
      }
    }

    /**
     * Automatic matchmaking: searches public lobby for open match; creates one if none found.
     */
    async quickMatch(customData = {}) {
      if (!this.isOnline()) {
        this.onStatusMsg('Firebase offline. Starting local vs AI.', 'warning');
        this.onSoloFallback({ reason: 'offline' });
        return { success: false, reason: 'offline' };
      }

      this.onStatusMsg('Scanning public subnet for open games...', 'info');

      try {
        const lobbyRef = this.fb.ref(this.fb.db, `${this.basePath}/lobby`);
        const snap = await this.fb.get(lobbyRef);
        const lobby = snap.val();

        if (lobby && typeof lobby === 'object') {
          const openCodes = Object.keys(lobby);
          // Pick the newest room
          if (openCodes.length > 0) {
            const bestCode = openCodes[openCodes.length - 1];
            this.onStatusMsg(`Found open game [${bestCode}]! Connecting...`, 'info');
            return await this.joinRoom(bestCode);
          }
        }

        // No open rooms -> Host a new public room
        return await this.createRoom(customData, true);
      } catch (err) {
        console.warn(`[RetroMultiplayer:${this.gameId}] quickMatch scan error, falling back to host:`, err);
        return await this.createRoom(customData, true);
      }
    }

    /**
     * Attaches live RTDB value listener to the active room.
     */
    attachRoomListener(code) {
      this.detachRoomListener();

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
          this.onStatusMsg(`Player 2 (${this.opponent}) joined! Match started.`, 'success');
          this.onOpponentJoin({ slot: 'p2', name: this.opponent });
        }

        // Detect opponent disconnect
        const otherSlot = this.mySlot === 'p1' ? 'p2' : (this.mySlot === 'p2' ? 'p1' : null);
        if (otherSlot && room[otherSlot] && room[otherSlot].connected === false && this.status === 'playing') {
          this.onStatusMsg(`Opponent (${room[otherSlot].name || otherSlot}) disconnected.`, 'warning');
          this.onOpponentLeave({ slot: otherSlot, name: room[otherSlot].name });
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

    /**
     * Detaches live RTDB value listener.
     */
    detachRoomListener() {
      if (this.activeRoomRef && this.activeRoomListener && this.fb) {
        try {
          this.fb.off(this.activeRoomRef, 'value', this.activeRoomListener);
        } catch (e) {}
      }
      this.activeRoomRef = null;
      this.activeRoomListener = null;
    }

    /**
     * Broadcasts a move payload to the room.
     */
    async sendMove(moveData, nextTurnSlot = null, additionalState = {}) {
      if (!this.isOnline() || !this.roomId) return false;

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

      try {
        const roomRef = this.fb.ref(this.fb.db, `${this.basePath}/rooms/${this.roomId}`);
        await this.fb.set(roomRef, {
          ...(await this.fb.get(roomRef)).val(),
          ...payload
        });
        return true;
      } catch (err) {
        console.error(`[RetroMultiplayer:${this.gameId}] sendMove failed:`, err);
        return false;
      }
    }

    /**
     * Broadcasts a short quick-chat message or emote.
     */
    async sendChat(text) {
      if (!this.isOnline() || !this.roomId) return false;

      try {
        const chatRef = this.fb.ref(this.fb.db, `${this.basePath}/rooms/${this.roomId}/chat`);
        await this.fb.set(chatRef, {
          by: this.mySlot,
          name: this.playerName,
          text: (text || '').slice(0, 80),
          ts: Date.now()
        });
        return true;
      } catch (e) {
        return false;
      }
    }

    /**
     * Submits a rematch confirmation.
     */
    async requestRematch(resetData = {}) {
      if (!this.isOnline() || !this.roomId || !this.mySlot) return false;

      try {
        const rematchRef = this.fb.ref(this.fb.db, `${this.basePath}/rooms/${this.roomId}/rematch/${this.mySlot}`);
        await this.fb.set(rematchRef, true);

        // Check if both agreed
        const snap = await this.fb.get(this.fb.ref(this.fb.db, `${this.basePath}/rooms/${this.roomId}`));
        const room = snap.val();
        if (room && room.rematch && room.rematch.p1 && room.rematch.p2) {
          // Reset game state
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
    }

    /**
     * Leaves the active room and cleans up resources.
     */
    async leaveRoom(notifyServer = true) {
      this.stopSoloFallbackTimer();
      const code = this.roomId;
      const slot = this.mySlot;

      this.detachRoomListener();

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

      // Clear URL hash
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
  }

  return RetroMultiplayer;
});
