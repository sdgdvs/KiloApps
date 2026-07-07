# KTetris

A lightweight, pure Win32 API implementation of Tetris for the KiloOS ecosystem.

## Features
- **Classic Tetris Gameplay**: Move, rotate, and clear lines.
- **Dynamic Difficulty**: The game gradually speeds up as your score increases.
- **Hard Drop**: Press `Spacebar` to instantly drop the current piece to the bottom.
- **Scoring System**: View your score in real-time on the side panel.

## Controls
- `Left Arrow`: Move piece left
- `Right Arrow`: Move piece right
- `Down Arrow`: Move piece down faster
- `Up Arrow`: Rotate piece
- `Spacebar`: Hard drop

## Building
To build KTetris, simply run the included `build.bat` script from a Visual Studio Developer Command Prompt. This will compile `main.c` and produce `KTetris.exe`.
