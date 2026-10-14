# KiloApps Cartridge Contract Specification (v1.0.0)

The **Cartridge Contract** formalizes the architectural boundary between the **KiloOS Web Desktop** and independent **Sub-999KB Minigame Cartridges**.

## 1. Core Principles
1. **Absolute Byte Ceiling**: Every cartridge must weigh strictly under **999,000 bytes** total (uncompressed web build + assets).
2. **Dual-Execution Mode**: Every cartridge can execute:
   - **Inside KiloOS**: Sandboxed within an `<iframe sandbox="allow-scripts allow-same-origin allow-downloads allow-popups">` inside the retro desktop window manager.
   - **Standalone URL**: Hosted at `/play/<game-slug>` or loaded directly in mobile/desktop browsers without needing the OS shell.
3. **Zero Asset Dependencies**: Audio and complex textures must be generated procedurally via `@kiloapps/sdk` (ZzFX sound synthesis + 2D canvas procedural drawing), avoiding large static asset bloat.

## 2. Cartridge Directory Structure
```
cartridges/<game-slug>/
  manifest.json          # Cartridge Contract metadata (validates against cartridge.schema.json)
  index.html             # Standalone web entrypoint
  icon.ico / icon.svg    # 32x32 retro desktop icon
  src/                   # Optional modular source
```

## 3. Host Communication Protocol (PostMessage Bridge)
When running inside KiloOS, the parent window provides transparent VFS services:

| Message Type | Direction | Payload | Description |
| :--- | :--- | :--- | :--- |
| `VFS_WRITE` | Child → Host | `{ path, data }` | Persist save state or high score to KiloOS VFS |
| `VFS_READ` | Child → Host | `{ path }` | Retrieve saved data |
| `OS_LAUNCH_APP` | Child → Host | `{ appId }` | Request KiloOS to open another app/cartridge |

When running outside KiloOS, `@kiloapps/sdk` automatically falls back to `window.localStorage` and native Web Audio.