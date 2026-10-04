# Retro Firebase Multiplayer Service (RFMS) Specification

**Standardized Ephemeral Matchmaking & State Sync for KiloApps**  
*Location: [`KiloOS/public/assets/js/retro_multiplayer.js`](../KiloOS/public/assets/js/retro_multiplayer.js)*  
*Payload Size: < 20 KB uncompressed (< 3 KB gzip / minified), 0 external bundler dependencies*

---

## 1. Overview & Architectural Goals

The **Retro Firebase Multiplayer Service (RFMS)** provides all KiloApps web games and collaborative tools with a standardized, lightweight protocol for cross-computer real-time gameplay via Firebase Realtime Database.

### Core Capabilities
- **Ephemeral Room Generation:** Standardized 4-character codes (e.g. `KGO-4892`, `REV-8193`).
- **Presence & Auto-Pruning:** Uses RTDB `.info/connected` and `onDisconnect().set(false)` to handle dropped peers without orphan ghost rooms.
- **Mandatory 25-Second Solo Fallback (Mandate Rule 12):** Lone players who host a room or wait in matchmaking are automatically transitioned to an active local AI opponent after 25 seconds so no user is ever trapped waiting indefinitely.
- **No Autostart / Explicit Connect Gate Mandate (CRITICAL):** Apps MUST NOT autostart into multiplayer, launch matchmaking, or connect to the online lobby on application load/boot. Players must never be thrown into multiplayer without hitting a "Connect", "Play Online", or "Quick Match" button or choosing Multiplayer from a menu or start screen first. The default startup state must always be local/offline play or a clean title/mode selection screen.
- **Dual URL Scheme & Iframe Support:** Supports both `?room=CODE` query parameters and `#room=CODE` URL hash fragments, ensuring direct browser links and embedded KiloOS desktop iframes work out-of-the-box.
- **Zero Bundler Overhead:** Loads via `<script src="../assets/js/retro_multiplayer.js"></script>` or ES import without swelling the sacred `< 999 KB` app size ceiling.

---

## 2. Quick Integration Guide

### Step 1: Include the Script in Your App HTML
```html
<script src="../assets/js/retro_multiplayer.js"></script>
```

### Step 2: Instantiate `RetroMultiplayer`
```javascript
const mp = new RetroMultiplayer({
  gameId: 'kreversi',       // RTDB path: multiplayer/kreversi/
  prefix: 'REV',           // Room prefix: REV-XXXX
  soloTimeoutMs: 25000,    // 25s auto-fallback (Rule 12)
  onMove: (moveData) => {
    applyRemoteMove(moveData);
  },
  onOpponentJoin: ({ slot, name }) => {
    showToast(`Player 2 (${name}) joined! Game starting.`, 'success');
  },
  onOpponentLeave: ({ slot, name }) => {
    showToast(`Opponent ${name} disconnected.`, 'warning');
  },
  onSoloFallback: ({ reason }) => {
    // Mandate Rule 12: engage local AI
    startAiGame();
  },
  onStatusMsg: (msg, type) => {
    showToast(msg, type);
  }
});
```

### Step 3: Initialize on Page Load
```javascript
window.addEventListener('DOMContentLoaded', async () => {
  const { onlineAvailable, inviteCode } = await mp.init();
  if (inviteCode) {
    await mp.joinRoom(inviteCode);
  }
});
```

### Step 4: Hook Match Actions
```javascript
// Quick Matchmaking
document.getElementById('quickMatchBtn').onclick = () => mp.quickMatch();

// Host Room
document.getElementById('createRoomBtn').onclick = () => mp.createRoom();

// Join Specific Code
document.getElementById('joinRoomBtn').onclick = () => mp.joinRoom(inputCode.value);

// Send Move (when local player takes turn)
mp.sendMove({ x, y, boardState }, nextPlayerSlot);

// Send In-Game Chat
mp.sendChat("Well played!");

// Leave or Reset
mp.leaveRoom();
```

---

## 3. Database Schema (`multiplayer/<gameId>/`)

```
multiplayer/
  └── <gameId>/
        ├── lobby/
        │     └── <ROOM_CODE>: { id, host, createdAt }
        └── rooms/
              └── <ROOM_CODE>:
                    ├── id: "<ROOM_CODE>"
                    ├── status: "waiting" | "playing" | "finished"
                    ├── p1: { name, connected: true, joinedAt }
                    ├── p2: { name, connected: true, joinedAt }
                    ├── turn: "p1" | "p2"
                    ├── winner: null | "p1" | "p2" | 0
                    ├── rematch: { p1: false, p2: false }
                    ├── chat: { by, name, text, ts }
                    ├── lastMove: { by, data, ts }
                    ├── isPublic: true | false
                    ├── createdAt: <timestamp>
                    └── updatedAt: <timestamp>
```

---

## 4. Director Mandate Rule 12 Compliance Checklist

When adding or updating multiplayer in any KiloApp:
1. [ ] **No Autostart on Boot (Connect Gate)**: NEVER autostart into multiplayer, initiate matchmaking, or listen to the lobby on initial app load. Default to offline play (vs AI/solo) or a start screen with mode choices.
2. [ ] **Start / Connect Screen**: Require players to hit an explicit "Connect", "Play Online", or "Quick Match" button (or select Multiplayer from a menu) before establishing any online session. People must never be thrown into multiplayer without prior warning.
3. [ ] **Invite Link Handling**: When launched with an invite link (`?room=CODE` or `#room=CODE`), show a clean connect prompt or join confirmation instead of an unprompted silent takeover.
4. [ ] **25s Solo Path**: If waiting for a peer and none joins within 25 seconds, app automatically switches to local AI cyber-bot.
5. [ ] **Address Bar Hash**: Sync `#room=CODE` upon hosting/joining and strip clean upon leaving.
6. [ ] **Disconnect Hook**: Hook `onDisconnect()` so closing a tab marks the player disconnected and removes open lobby entries.
7. [ ] **Offline Fallback**: App remains 100% playable offline if Firebase is unreachable or user is disconnected.
8. [ ] **File Size**: App + scripts stay strictly `< 999 KB`.
