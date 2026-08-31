#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>

#pragma comment(lib, "msvcrt.lib")

#define W 340
#define H 540
#define COLS 15
#define ROWS 15
#define TS 20

// 20 Unique Campaign Maps
char maps[20][ROWS][COLS] = {
    // Stage 1: Classic Maze
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,2,1,1,1,2,1,1,1,2,1,1,1,2,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,1,1,1,1,1,2,1,2,1,1,1,1,1,1},
        {0,0,0,0,0,1,2,1,2,1,0,0,0,0,0},
        {1,1,1,1,1,1,2,2,2,1,1,1,1,1,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,1,1,2,2,2,1,1,2,2,2,1},
        {1,1,1,3,2,2,2,1,2,2,2,3,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 2: Central Square
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,2,2,2,2,2,2,3,1},
        {1,2,1,1,1,2,1,1,1,2,1,1,1,2,1},
        {1,2,1,1,1,2,1,1,1,2,1,1,1,2,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,1,1,1,1,1,2,1,2,1,1,1,1,1,1},
        {0,0,0,0,0,1,2,1,2,1,0,0,0,0,0},
        {1,1,1,1,1,1,2,2,2,1,1,1,1,1,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,2,1,1,1,2,1,1,1,2,1,1,1,2,1},
        {1,4,2,2,1,2,2,2,2,2,1,2,2,4,1},
        {1,1,1,2,1,2,1,1,1,2,1,2,1,1,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,3,1,1,1,1,2,2,2,1,1,1,1,3,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 3: Diamond Arena
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,4,2,2,2,2,2,1,2,2,2,2,2,4,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,2,1,1,2,1,2,2,2,1,2,1,1,2,1},
        {1,2,2,2,2,1,1,1,1,1,2,2,2,2,1},
        {1,1,1,1,2,2,2,2,2,2,2,1,1,1,1},
        {0,0,0,1,2,1,1,2,1,1,2,1,0,0,0},
        {1,1,1,1,2,1,0,0,0,1,2,1,1,1,1},
        {1,2,2,2,2,1,1,1,1,1,2,2,2,2,1},
        {1,2,1,1,2,2,2,1,2,2,2,1,1,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,2,1,1,1,2,1,1,1,2,1,1,1,2,1},
        {1,3,2,2,2,2,2,3,2,2,2,2,2,3,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 4: Cross Tunnel
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,1,1,2,1,1,1,1,1,1,1,2,1,1,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {0,0,0,0,0,0,2,2,2,0,0,0,0,0,0},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,1,1,2,1,1,1,1,1,1,1,2,1,1,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,4,2,2,2,2,2,1,2,2,2,2,2,4,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 5: Inward Spiral
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,1,2,2,2,1,2,2,2,1,2,3,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,1,1,1,2,1,1,1,1,1,2,1,1,1,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,2,1,1,1,1,1,1,1,1,1,1,1,2,1},
        {0,0,2,2,2,2,2,2,2,2,2,2,2,0,0},
        {1,2,1,1,1,1,1,1,1,1,1,1,1,2,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,1,1,1,2,1,1,1,1,1,2,1,1,1,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,4,2,1,2,2,2,1,2,2,2,1,2,4,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 6: Twin Chamber
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,4,2,2,2,2,2,1,2,2,2,2,2,4,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,1,2,2,2,2,2,2,2,2,2,1,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {0,0,0,0,0,1,2,1,2,1,0,0,0,0,0},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,1,2,2,2,2,2,2,2,2,2,1,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 7: Grid Labyrinth
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,1,2,2,2,1,2,2,2,3,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,2,1,1,2,2,2,1,2,2,2,1,1,2,1},
        {1,2,2,2,2,1,1,1,1,1,2,2,2,2,1},
        {1,2,1,1,2,1,2,2,2,1,2,1,1,2,1},
        {1,2,2,2,2,1,2,1,2,1,2,2,2,2,1},
        {1,1,1,1,2,2,2,1,2,2,2,1,1,1,1},
        {0,0,0,1,2,1,2,1,2,1,2,1,0,0,0},
        {1,1,1,1,2,1,2,2,2,1,2,1,1,1,1},
        {1,2,2,2,2,1,1,1,1,1,2,2,2,2,1},
        {1,2,1,1,2,2,2,1,2,2,2,1,1,2,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,4,2,2,2,1,2,2,2,1,2,2,2,4,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 8: Concentric Rings
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,2,2,2,2,2,2,3,1},
        {1,2,1,1,1,1,1,1,1,1,1,1,1,2,1},
        {1,2,1,2,2,2,2,2,2,2,2,2,1,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,1,2,1,2,2,2,2,2,1,2,1,2,1},
        {1,2,1,2,1,2,1,1,1,2,1,2,1,2,1},
        {1,2,2,2,2,2,1,2,1,2,2,2,2,2,1},
        {1,2,1,2,1,2,1,1,1,2,1,2,1,2,1},
        {1,2,1,2,1,2,2,2,2,2,1,2,1,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,1,2,2,2,2,2,2,2,2,2,1,2,1},
        {1,2,1,1,1,1,1,1,1,1,1,1,1,2,1},
        {1,4,2,2,2,2,2,2,2,2,2,2,2,4,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 9: Checkerboard Fortress
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,1,2,1,2,1,2,1,2,1,2,3,1},
        {1,1,2,1,2,1,2,1,2,1,2,1,2,1,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,1,2,2,2,2,2,2,2,2,2,1,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {0,2,2,2,1,0,0,0,0,0,1,2,2,2,0},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,1,2,2,2,2,2,2,2,2,2,1,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,1,2,1,2,1,2,1,2,1,2,1,2,1,1},
        {1,4,2,1,2,1,2,1,2,1,2,1,2,4,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 10: Central Hub
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,2,2,2,2,2,2,3,1},
        {1,2,1,2,1,1,1,2,1,1,1,2,1,2,1},
        {1,2,1,2,2,2,2,2,2,2,2,2,1,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,1,1,1,1,2,1,1,1,2,1,1,1,1,1},
        {1,0,0,0,1,2,1,0,1,2,1,0,0,0,1},
        {1,1,1,1,1,2,1,1,1,2,1,1,1,1,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,1,2,2,2,2,2,2,2,2,2,1,2,1},
        {1,2,1,2,1,1,1,2,1,1,1,2,1,2,1},
        {1,4,2,2,2,2,2,2,2,2,2,2,2,4,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 11: Frost Vault
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,5,2,2,2,2,1,2,1,2,2,2,2,5,1},
        {1,2,1,1,1,2,1,2,1,2,1,1,1,2,1},
        {1,2,1,3,2,2,2,2,2,2,2,3,1,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,2,2,1,2,2,2,2,2,1,2,2,2,1},
        {1,1,1,2,1,2,1,1,1,2,1,2,1,1,1},
        {0,0,0,2,2,2,1,0,1,2,2,2,0,0,0},
        {1,1,1,2,1,2,1,1,1,2,1,2,1,1,1},
        {1,2,2,2,1,2,2,2,2,2,1,2,2,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,1,4,2,2,2,2,2,2,2,4,1,2,1},
        {1,2,1,1,1,2,1,2,1,2,1,1,1,2,1},
        {1,5,2,2,2,2,1,2,1,2,2,2,2,5,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 12: Dual Warp Arena
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,5,1,2,2,2,1,5,2,2,2,1},
        {1,1,1,1,2,1,1,1,1,1,2,1,1,1,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,2,1,1,1,2,1,1,1,2,1,1,1,2,1},
        {1,2,1,0,1,2,1,0,1,2,1,0,1,2,1},
        {1,2,1,1,1,2,1,1,1,2,1,1,1,2,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,1,1,1,2,1,1,1,1,1,2,1,1,1,1},
        {1,2,2,2,4,1,2,2,2,1,4,2,2,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 13: Lightning Chamber
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,5,2,1,2,2,2,2,2,2,2,1,2,5,1},
        {1,1,2,1,2,1,1,1,1,1,2,1,2,1,1},
        {1,2,2,2,2,1,3,2,3,1,2,2,2,2,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,2,1,2,2,2,2,1,2,2,2,2,1,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,2,2,1,0,0,0,0,0,1,2,2,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,1,2,2,2,2,1,2,2,2,2,1,2,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,2,2,2,2,1,4,2,4,1,2,2,2,2,1},
        {1,1,2,1,2,1,1,1,1,1,2,1,2,1,1},
        {1,5,2,1,2,2,2,2,2,2,2,1,2,5,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 14: Pinwheel Crossroads
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,1,2,5,2,1,2,2,2,3,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,1,1,1,1,2,1,1,1,2,1,1,1,1,1},
        {1,2,2,2,1,2,2,2,2,2,1,2,2,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {0,2,1,2,2,2,2,0,2,2,2,2,1,2,0},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,2,2,1,2,2,2,2,2,1,2,2,2,1},
        {1,1,1,1,1,2,1,1,1,2,1,1,1,1,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,3,2,2,2,1,2,5,2,1,2,2,2,3,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 15: Blockade Maze
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,5,2,2,2,2,2,2,2,2,2,2,2,5,1},
        {1,2,1,1,1,1,1,2,1,1,1,1,1,2,1},
        {1,2,1,3,2,2,1,2,1,2,2,3,1,2,1},
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {1,2,1,2,1,2,2,2,2,2,1,2,1,2,1},
        {1,2,2,2,1,1,1,1,1,1,1,2,2,2,1},
        {1,1,1,2,2,2,2,0,2,2,2,2,1,1,1},
        {1,2,2,2,1,1,1,1,1,1,1,2,2,2,1},
        {1,2,1,2,1,2,2,2,2,2,1,2,1,2,1},
        {1,2,1,2,1,2,1,2,1,2,1,2,1,2,1},
        {1,2,1,4,2,2,1,2,1,2,2,4,1,2,1},
        {1,2,1,1,1,1,1,2,1,1,1,1,1,2,1},
        {1,5,2,2,2,2,2,2,2,2,2,2,2,5,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 16: Warp Tunnel Heavy
    {
        {1,1,1,1,1,1,0,0,0,1,1,1,1,1,1},
        {1,3,2,2,2,1,2,2,2,1,2,2,2,3,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,2,1,4,2,2,2,1,2,2,2,4,1,2,1},
        {1,2,2,2,1,1,2,2,2,1,1,2,2,2,1},
        {0,0,0,2,1,2,2,1,2,2,1,2,0,0,0},
        {1,1,1,2,1,2,1,1,1,2,1,2,1,1,1},
        {0,0,0,2,2,2,1,0,1,2,2,2,0,0,0},
        {1,1,1,2,1,2,1,1,1,2,1,2,1,1,1},
        {0,0,0,2,1,2,2,1,2,2,1,2,0,0,0},
        {1,2,2,2,1,1,2,2,2,1,1,2,2,2,1},
        {1,2,1,5,2,2,2,1,2,2,2,5,1,2,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,3,2,2,2,1,2,2,2,1,2,2,2,3,1},
        {1,1,1,1,1,1,0,0,0,1,1,1,1,1,1}
    },
    // Stage 17: Speed Zone Arena
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,4,4,4,2,2,2,1,2,2,2,4,4,4,1},
        {1,4,1,1,1,2,1,1,1,2,1,1,1,4,1},
        {1,4,1,3,2,2,2,4,2,2,2,3,1,4,1},
        {1,2,1,2,1,1,2,1,2,1,1,2,1,2,1},
        {1,2,2,2,1,4,4,4,4,4,1,2,2,2,1},
        {1,1,1,2,1,4,1,1,1,4,1,2,1,1,1},
        {0,0,0,2,4,4,1,0,1,4,4,2,0,0,0},
        {1,1,1,2,1,4,1,1,1,4,1,2,1,1,1},
        {1,2,2,2,1,4,4,4,4,4,1,2,2,2,1},
        {1,2,1,2,1,1,2,1,2,1,1,2,1,2,1},
        {1,4,1,3,2,2,2,4,2,2,2,3,1,4,1},
        {1,4,1,1,1,2,1,1,1,2,1,1,1,4,1},
        {1,4,4,4,2,2,2,1,2,2,2,4,4,4,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 18: Spiral Shadow Labyrinth
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,2,2,2,2,2,2,3,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,2,1,1},
        {1,5,2,2,2,2,2,2,2,2,2,1,2,5,1},
        {1,2,1,1,1,1,1,1,1,1,2,1,1,2,1},
        {1,2,1,3,2,2,2,2,2,1,2,1,2,2,1},
        {1,2,1,2,1,1,1,1,2,1,2,1,2,1,1},
        {0,2,1,2,1,0,0,0,2,1,2,1,2,2,0},
        {1,1,1,2,1,2,1,1,1,1,2,1,1,2,1},
        {1,2,2,2,1,2,2,2,2,2,2,1,2,2,1},
        {1,2,1,1,1,1,1,1,1,1,1,1,2,1,1},
        {1,5,2,2,2,2,2,2,2,2,2,2,2,5,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,2,2,2,2,2,2,3,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 19: The Gauntlet
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,1,3,2,1,2,1,2,3,1,2,3,1},
        {1,1,2,1,1,2,1,2,1,2,1,1,2,1,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,1,4,2,2,2,5,2,2,2,4,1,2,1},
        {1,2,1,1,1,1,1,1,1,1,1,1,1,2,1},
        {0,2,2,2,2,2,2,0,2,2,2,2,2,2,0},
        {1,2,1,1,1,1,1,1,1,1,1,1,1,2,1},
        {1,2,1,4,2,2,2,5,2,2,2,4,1,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,1,2,1,1,2,1,2,1,2,1,1,2,1,1},
        {1,3,2,1,3,2,1,2,1,2,3,1,2,3,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 20: Ghost King Lair (Boss Chamber)
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,1,5,2,2,2,2,2,2,2,5,1,2,1},
        {1,2,1,2,1,1,1,0,1,1,1,2,1,2,1},
        {1,2,2,2,1,0,0,0,0,0,1,2,2,2,1},
        {1,1,1,2,1,0,0,0,0,0,1,2,1,1,1},
        {0,0,0,2,0,0,0,0,0,0,0,2,0,0,0},
        {1,1,1,2,1,0,0,0,0,0,1,2,1,1,1},
        {1,2,2,2,1,0,0,0,0,0,1,2,2,2,1},
        {1,2,1,2,1,1,1,0,1,1,1,2,1,2,1},
        {1,2,1,4,2,2,2,2,2,2,2,4,1,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    }
};

char map[ROWS][COLS];

int randSeed = 42;
int MyRand() {
    randSeed = randSeed * 1103515245 + 12345;
    return (unsigned int)(randSeed / 65536) % 32768;
}

int Abs(int x) {
    return (x < 0) ? -x : x;
}

static double MySin(double x) {
    while (x > 3.1415926535) x -= 6.283185307;
    while (x < -3.1415926535) x += 6.283185307;
    double x3 = x * x * x;
    double x5 = x3 * x * x;
    return x - (x3 / 6.0) + (x5 / 120.0);
}

static double MyCos(double x) {
    return MySin(x + 1.57079632679);
}

// Game Modes & Loop 10 Progression
int gameMode = 0; // 0 = Campaign, 1 = Arcade Endless
int endlessWave = 1;
int endlessHighWave = 1;

// Crafting System (The Cyber Forge)
int craftEctoplasm = 0;
int craftFruitEssence = 0;
int craftStarDust = 0;
int showCraftMenu = 0;
int shieldHits = 0;

// Player state
int px = 7, py = 12;
int pdx = 0, pdy = 0;
int ndx = 0, ndy = 0;
int playerSlowTimer = 0;

// Ghost struct (supports 5 standard ghosts + boss ghost + phantom clones + procedural personalities)
typedef struct {
    int x;
    int y;
    COLORREF c;
    int type; // 0=Blinky(Red), 1=Pinky(Pink), 2=Inky(Cyan), 3=Clyde(Orange), 4=Sue(Purple Stalker), 5=GhostKing, 6=Phantom
    int isPhantom;
    int phantomTimer;
    int isDead;
    int dirX;
    int dirY;
    int trait; // 0=None, 1=Vortex Magnet, 2=Glitch Shifter, 3=Trapper, 4=Mirage, 5=Hyper Chaser
    int traitTimer;
    int glitchOffsetX;
    int glitchOffsetY;
} Ghost;

Ghost ghosts[8];
int numGhosts = 5;

// Sludge Traps (placed by Trapper ghosts)
typedef struct {
    int x;
    int y;
    int life;
} SludgeTrap;
SludgeTrap sludgeTraps[16];
int numSludgeTraps = 0;

// Active Skills & Timers
int freezeSkillTimer = 0, freezeCooldown = 0;
int speedSkillTimer = 0, speedCooldown = 0;
int magnetSkillTimer = 0, magnetCooldown = 0;
int shieldActive = 0, shieldCooldown = 0;

// Stage 20 Boss State
int bossHp = 8;
int bossMaxHp = 8;
int phantomSpawnTimer = 0;

int vipX = 7, vipY = 7, vipActive = 0;

// Game Loop State
int score = 0;
int highScore = 0;
int gameOver = 0;
int dotCount = 0;
int frameCount = 0;
int deathTimer = 0;
int screenShake = 0;
int level = 1;
int frightTimer = 0;
int victoryTimer = 0;
int lives = 3;
int paused = 0;
int fruitActive = 0;
int fruitTimer = 0;

typedef struct {
    int x;
    int y;
    int r;
    int maxR;
    COLORREF color;
    int is3D;
} ShockwaveRing;

ShockwaveRing shockwaves[16];
int numShockwaves = 0;

void AddShockwave(int x, int y, COLORREF color) {
    if (numShockwaves < 16) {
        shockwaves[numShockwaves++] = (ShockwaveRing){x, y, 2, 28, color, 0};
    }
}

void AddShockwave3D(int x, int y, COLORREF color) {
    if (numShockwaves < 16) {
        shockwaves[numShockwaves++] = (ShockwaveRing){x, y, 2, 28, color, 1};
    }
}

typedef struct {
    double x, y, vx, vy;
    int life, maxLife;
    COLORREF color;
} Particle;
Particle particles[256];
int numParticles = 0;

void AddSparks(int cx, int cy, COLORREF color, int count) {
    for (int i = 0; i < count && numParticles < 256; i++) {
        double ang = (MyRand() % 360) * 3.14159 / 180.0;
        double spd = 1.0 + (MyRand() % 40) / 10.0;
        particles[numParticles].x = cx;
        particles[numParticles].y = cy;
        particles[numParticles].vx = MyCos(ang) * spd;
        particles[numParticles].vy = MySin(ang) * spd;
        particles[numParticles].life = 20 + (MyRand() % 10);
        particles[numParticles].maxLife = particles[numParticles].life;
        particles[numParticles].color = color;
        numParticles++;
    }
}

void AddExplosion(int cx, int cy, COLORREF color) {
    AddShockwave3D(cx, cy, color);
    AddShockwave(cx, cy, RGB(255, 255, 255));
    AddSparks(cx, cy, color, 15);
    AddSparks(cx, cy, RGB(255, 255, 255), 10);
    screenShake = 15;
}

int diffMode = 1; // 0 = Easy, 1 = Normal, 2 = Hard
char saveMsgText[64] = "";
int saveMsgTimer = 0;
int showHelp = 1;

int bindState = 0;
int bindUp = VK_UP, bindDown = VK_DOWN, bindLeft = VK_LEFT, bindRight = VK_RIGHT;
int bindSkill1 = 'F', bindSkill2 = 'Z', bindSkill3 = 'M', bindSkill4 = 'B';

typedef struct {
    int frame;
    int action;
} ReplayEvent;
ReplayEvent replays[10000];
int replayCount = 0;
int replayPlaybackIndex = 0;
int replayMode = 0; // 0=off, 1=record, 2=playback
int replaySeed = 0;
int replayDiffMode = 1;

int statsGamesPlayed = 0;
int statsGhostsEaten = 0;
int statsMaxScore = 0;

typedef struct {
    int px, py, pdx, pdy, ndx, ndy;
    Ghost ghosts[8];
    char map[ROWS][COLS];
    int score, level, lives, diffMode;
    int frightTimer, freezeSkillTimer, speedSkillTimer, magnetSkillTimer, shieldActive, shieldHits;
    int freezeCooldown, speedCooldown, magnetCooldown, shieldCooldown;
    int bossHp;
    int dotCount, frameCount, fruitActive, fruitTimer, gameOver;
    int vipX, vipY, vipActive;
    int gameMode, endlessWave, endlessHighWave;
    int craftEctoplasm, craftFruitEssence, craftStarDust;
} SaveState;

void SaveGame() {
    SaveState st;
    st.px = px; st.py = py; st.pdx = pdx; st.pdy = pdy; st.ndx = ndx; st.ndy = ndy;
    for (int i = 0; i < 8; i++) st.ghosts[i] = ghosts[i];
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            st.map[r][c] = map[r][c];
        }
    }
    st.score = score; st.level = level; st.lives = lives; st.diffMode = diffMode;
    st.frightTimer = frightTimer; st.freezeSkillTimer = freezeSkillTimer; st.speedSkillTimer = speedSkillTimer;
    st.magnetSkillTimer = magnetSkillTimer; st.shieldActive = shieldActive; st.shieldHits = shieldHits;
    st.freezeCooldown = freezeCooldown; st.speedCooldown = speedCooldown;
    st.magnetCooldown = magnetCooldown; st.shieldCooldown = shieldCooldown;
    st.bossHp = bossHp;
    st.dotCount = dotCount; st.frameCount = frameCount;
    st.fruitActive = fruitActive; st.fruitTimer = fruitTimer; st.gameOver = gameOver;
    st.vipX = vipX; st.vipY = vipY; st.vipActive = vipActive;
    st.gameMode = gameMode; st.endlessWave = endlessWave; st.endlessHighWave = endlessHighWave;
    st.craftEctoplasm = craftEctoplasm; st.craftFruitEssence = craftFruitEssence; st.craftStarDust = craftStarDust;

    HANDLE hFile = CreateFileA("kpac_save.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, &st, sizeof(SaveState), &written, NULL);
        CloseHandle(hFile);
        lstrcpyA(saveMsgText, "GAME SAVED");
        saveMsgTimer = 20;
        MessageBeep(MB_OK);
    }
}

void LoadGame() {
    HANDLE hFile = CreateFileA("kpac_save.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        lstrcpyA(saveMsgText, "NO SAVE FOUND");
        saveMsgTimer = 20;
        MessageBeep(MB_ICONHAND);
        return;
    }
    SaveState st;
    DWORD readBytes = 0;
    if (ReadFile(hFile, &st, sizeof(SaveState), &readBytes, NULL) && readBytes == sizeof(SaveState)) {
        px = st.px; py = st.py; pdx = st.pdx; pdy = st.pdy; ndx = st.ndx; ndy = st.ndy;
        for (int i = 0; i < 8; i++) ghosts[i] = st.ghosts[i];
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                map[r][c] = st.map[r][c];
            }
        }
        score = st.score; level = st.level; lives = st.lives; diffMode = st.diffMode;
        frightTimer = st.frightTimer; freezeSkillTimer = st.freezeSkillTimer; speedSkillTimer = st.speedSkillTimer;
        magnetSkillTimer = st.magnetSkillTimer; shieldActive = st.shieldActive; shieldHits = st.shieldHits;
        freezeCooldown = st.freezeCooldown; speedCooldown = st.speedCooldown;
        magnetCooldown = st.magnetCooldown; shieldCooldown = st.shieldCooldown;
        bossHp = st.bossHp;
        dotCount = st.dotCount; frameCount = st.frameCount;
        fruitActive = st.fruitActive; fruitTimer = st.fruitTimer; gameOver = st.gameOver;
        vipX = st.vipX; vipY = st.vipY; vipActive = st.vipActive;
        gameMode = st.gameMode; endlessWave = st.endlessWave; endlessHighWave = st.endlessHighWave;
        craftEctoplasm = st.craftEctoplasm; craftFruitEssence = st.craftFruitEssence; craftStarDust = st.craftStarDust;
        paused = 0;
        lstrcpyA(saveMsgText, "GAME LOADED");
        saveMsgTimer = 20;
        MessageBeep(MB_OK);
    }
    CloseHandle(hFile);
}

void LoadHighScore() {
    HANDLE hFile = CreateFileA("kpac_hi.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes;
        ReadFile(hFile, &highScore, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &endlessHighWave, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &craftEctoplasm, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &craftFruitEssence, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &craftStarDust, sizeof(int), &readBytes, NULL);
        CloseHandle(hFile);
    }
    hFile = CreateFileA("kpac_stats.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes;
        ReadFile(hFile, &statsGamesPlayed, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &statsGhostsEaten, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &statsMaxScore, sizeof(int), &readBytes, NULL);
        CloseHandle(hFile);
    }
}

void SaveHighScore() {
    HANDLE hFile = CreateFileA("kpac_hi.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, &highScore, sizeof(int), &written, NULL);
        WriteFile(hFile, &endlessHighWave, sizeof(int), &written, NULL);
        WriteFile(hFile, &craftEctoplasm, sizeof(int), &written, NULL);
        WriteFile(hFile, &craftFruitEssence, sizeof(int), &written, NULL);
        WriteFile(hFile, &craftStarDust, sizeof(int), &written, NULL);
        CloseHandle(hFile);
    }
    hFile = CreateFileA("kpac_stats.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, &statsGamesPlayed, sizeof(int), &written, NULL);
        WriteFile(hFile, &statsGhostsEaten, sizeof(int), &written, NULL);
        WriteFile(hFile, &statsMaxScore, sizeof(int), &written, NULL);
        CloseHandle(hFile);
    }
}

int GetInitLives() {
    if (diffMode == 0) return 5;
    if (diffMode == 2) return 2;
    return 3;
}

// Procedural Maze Generator for Arcade Endless Mode
void GenerateProceduralMaze(int wave) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (r == 0 || r == ROWS - 1 || c == 0 || c == COLS - 1) {
                map[r][c] = 1;
            } else {
                map[r][c] = 2; // standard dot
            }
        }
    }
    // Symmetrical obstacle layouts
    int seed = wave * 179 + 31;
    for (int r = 2; r < ROWS - 2; r += 2) {
        for (int c = 2; c <= COLS / 2; c += 2) {
            int opp = COLS - 1 - c;
            int blk = (seed + r * 5 + c * 7) % 5;
            if (blk == 0) {
                map[r][c] = 1; map[r][opp] = 1;
                map[r+1][c] = 1; map[r+1][opp] = 1;
            } else if (blk == 1) {
                map[r][c] = 1; map[r][opp] = 1;
                map[r][c+1] = 1; map[r][opp-1] = 1;
            } else if (blk == 2) {
                map[r][c] = 1; map[r][opp] = 1;
            }
        }
    }
    // Central Ghost House
    for (int r = 6; r <= 8; r++) {
        for (int c = 5; c <= 9; c++) {
            map[r][c] = 0;
        }
    }
    map[6][5] = 1; map[7][5] = 1; map[8][5] = 1;
    map[6][9] = 1; map[7][9] = 1; map[8][9] = 1;
    map[8][6] = 1; map[8][7] = 1; map[8][8] = 1;
    map[6][6] = 1; map[6][7] = 1; map[6][8] = 1; // Door at (6,7) handled specially
    
    // Left and right warp tunnels
    map[7][0] = 0; map[7][1] = 2;
    map[7][COLS-1] = 0; map[7][COLS-2] = 2;
    
    // Player spawn (7, 12)
    map[12][7] = 0; map[11][7] = 2; map[12][6] = 2; map[12][8] = 2;
    
    // 4 Corner Power Pellets
    map[1][1] = 3; map[1][COLS-2] = 3;
    map[ROWS-2][1] = 3; map[ROWS-2][COLS-2] = 3;
    
    // Speed / Freeze powerups
    map[3][3] = 4; map[3][COLS-4] = 5;
    map[ROWS-4][3] = 5; map[ROWS-4][COLS-4] = 4;

    // Hazards for higher waves
    if (wave >= 3) {
        if (map[4][7] == 2) map[4][7] = 6;
        if (map[10][7] == 2) map[10][7] = 6;
    }
}

void Init(int keepScore) {
    if (!keepScore) {
        score = 0;
        if (gameMode == 0) level = 1;
        else endlessWave = 1;
        lives = GetInitLives();
    }
    gameOver = 0;
    paused = 0;
    dotCount = 0;
    deathTimer = 0;
    numSludgeTraps = 0;
    playerSlowTimer = 0;

    if (gameMode == 1) {
        GenerateProceduralMaze(endlessWave);
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (map[r][c] >= 2 && map[r][c] <= 5) dotCount++;
            }
        }
    } else {
        int mapIndex = (level - 1) % 20;
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                map[r][c] = maps[mapIndex][r][c];
                if (map[r][c] >= 2 && map[r][c] <= 5) dotCount++;
            }
        }
    }

    px = 7; py = 12;
    pdx = 0; pdy = 0;
    ndx = 0; ndy = 0;

    // Roster of AI Ghosts with Procedural Personalities & Traits
    int effWave = (gameMode == 1) ? endlessWave : level;
    int t0 = (effWave >= 2) ? ((effWave % 5) + 1) : 0;
    int t1 = (effWave >= 3) ? (((effWave + 2) % 5) + 1) : 0;
    int t2 = (effWave >= 4) ? (((effWave + 3) % 5) + 1) : 0;
    int t3 = (effWave >= 5) ? (((effWave + 4) % 5) + 1) : 0;
    int t4 = (effWave >= 6) ? (((effWave + 1) % 5) + 1) : 0;

    ghosts[0] = (Ghost){7, 6, RGB(255, 23, 68), 0, 0, 0, 0, 0, -1, t0, 0, 0, 0};
    ghosts[1] = (Ghost){6, 7, RGB(240, 98, 146), 1, 0, 0, 0, -1, 0, t1, 0, 0, 0};
    ghosts[2] = (Ghost){8, 7, RGB(0, 229, 255), 2, 0, 0, 0, 1, 0, t2, 0, 0, 0};
    ghosts[3] = (Ghost){7, 7, RGB(255, 145, 0), 3, 0, 0, 0, 0, 1, t3, 0, 0, 0};
    ghosts[4] = (Ghost){7, 5, RGB(170, 0, 255), 4, 0, 0, 0, 0, -1, t4, 0, 0, 0};

    if (gameMode == 0 && level == 20) {
        // Stage 20 Ghost King Boss
        ghosts[5] = (Ghost){7, 6, RGB(255, 215, 0), 5, 0, 0, 0, 0, -1, 5, 0, 0, 0};
        ghosts[6] = (Ghost){0, 0, RGB(0,0,0), 6, 1, 0, 0, 0, 0, 0, 0, 0, 0};
        ghosts[7] = (Ghost){0, 0, RGB(0,0,0), 6, 1, 0, 0, 0, 0, 0, 0, 0, 0};
        numGhosts = 6;
        bossHp = 8;
        bossMaxHp = 8;
        phantomSpawnTimer = 0;
    } else if (effWave >= 4) {
        numGhosts = 5;
    } else if (effWave == 3) {
        numGhosts = 4;
    } else if (effWave == 2) {
        numGhosts = 3;
    } else {
        numGhosts = 2;
    }

    vipActive = (gameMode == 0 && level % 5 == 0 && level != 20) ? 1 : 0;
    vipX = 7; vipY = 7;
    if (gameMode == 0 && level % 3 == 0 && level < 18 && map[ROWS/2][COLS/2] != 1) {
        map[ROWS/2][COLS/2] = 8; // Branch Warp
    }

    frightTimer = 0;
    victoryTimer = 0;
    freezeSkillTimer = 0; freezeCooldown = 0;
    speedSkillTimer = 0; speedCooldown = 0;
    magnetSkillTimer = 0; magnetCooldown = 0;
    shieldActive = 0; shieldCooldown = 0; shieldHits = 0;
    fruitActive = 0;
    fruitTimer = 0;
    numShockwaves = 0;
    numParticles = 0;
    showCraftMenu = 0;
}

// Crafting System Logic
void CraftItem(int recipe) {
    if (recipe == 1) { // Super Pellet: 2 Ectoplasm + 1 Star Dust
        if (craftEctoplasm >= 2 && craftStarDust >= 1) {
            craftEctoplasm -= 2;
            craftStarDust -= 1;
            frightTimer = 80;
            score += 500;
            lstrcpyA(saveMsgText, "CRAFTED: SUPER PELLET!");
            saveMsgTimer = 25;
            AddExplosion(px * TS + TS/2, py * TS + TS/2, RGB(255, 215, 0));
            MessageBeep(MB_ICONASTERISK);
        } else {
            lstrcpyA(saveMsgText, "NEED: 2 ECTO + 1 DUST");
            saveMsgTimer = 25;
            MessageBeep(MB_ICONHAND);
        }
    } else if (recipe == 2) { // Chrono Warp: 2 Ectoplasm + 2 Fruit Essence
        if (craftEctoplasm >= 2 && craftFruitEssence >= 2) {
            craftEctoplasm -= 2;
            craftFruitEssence -= 2;
            freezeSkillTimer = 80;
            speedSkillTimer = 80;
            lstrcpyA(saveMsgText, "CRAFTED: CHRONO WARP!");
            saveMsgTimer = 25;
            AddShockwave(px * TS + TS/2, py * TS + TS/2, RGB(0, 255, 255));
            MessageBeep(MB_ICONASTERISK);
        } else {
            lstrcpyA(saveMsgText, "NEED: 2 ECTO + 2 ESSENCE");
            saveMsgTimer = 25;
            MessageBeep(MB_ICONHAND);
        }
    } else if (recipe == 3) { // Aegis Shield: 2 Star Dust + 2 Fruit Essence
        if (craftStarDust >= 2 && craftFruitEssence >= 2) {
            craftStarDust -= 2;
            craftFruitEssence -= 2;
            shieldActive = 1;
            shieldHits = 2; // 2-hit barrier
            lstrcpyA(saveMsgText, "CRAFTED: AEGIS SHIELD (2X)!");
            saveMsgTimer = 25;
            AddShockwave3D(px * TS + TS/2, py * TS + TS/2, RGB(0, 229, 255));
            MessageBeep(MB_ICONASTERISK);
        } else {
            lstrcpyA(saveMsgText, "NEED: 2 DUST + 2 ESSENCE");
            saveMsgTimer = 25;
            MessageBeep(MB_ICONHAND);
        }
    } else if (recipe == 4) { // Void Pulse Bomb: 3 Ectoplasm + 3 Star Dust + 1 Fruit Essence
        if (craftEctoplasm >= 3 && craftStarDust >= 3 && craftFruitEssence >= 1) {
            craftEctoplasm -= 3;
            craftStarDust -= 3;
            craftFruitEssence -= 1;
            for (int i = 0; i < numGhosts; i++) {
                if (!ghosts[i].isPhantom) {
                    ghosts[i].x = 7;
                    ghosts[i].y = 6;
                    ghosts[i].isDead = 0;
                }
            }
            for (int r = py - 5; r <= py + 5; r++) {
                for (int c = px - 5; c <= px + 5; c++) {
                    if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                        if (map[r][c] >= 2 && map[r][c] <= 5) {
                            score += 20;
                            map[r][c] = 0;
                            dotCount--;
                        }
                    }
                }
            }
            lstrcpyA(saveMsgText, "CRAFTED: VOID BOMB PULSE!");
            saveMsgTimer = 25;
            AddExplosion(px * TS + TS/2, py * TS + TS/2, RGB(200, 50, 255));
            MessageBeep(MB_ICONASTERISK);
        } else {
            lstrcpyA(saveMsgText, "NEED: 3 ECTO + 3 DUST + 1 ESS");
            saveMsgTimer = 25;
            MessageBeep(MB_ICONHAND);
        }
    }
}

// Active Skill Trigger Functions
void TriggerFreezeSkill() {
    if (freezeCooldown == 0 && !gameOver && !paused) {
        freezeSkillTimer = 60; // 6 seconds freeze
        freezeCooldown = 150;  // 15s cooldown
        lstrcpyA(saveMsgText, "FREEZE SKILL!");
        saveMsgTimer = 20;
        AddSparks(px * TS + TS/2, py * TS + TS/2, RGB(255, 255, 255), 15);
        MessageBeep(MB_ICONINFORMATION);
    }
}

void TriggerSpeedSkill() {
    if (speedCooldown == 0 && !gameOver && !paused) {
        speedSkillTimer = 80; // 8 seconds 2x speed
        speedCooldown = 150;  // 15s cooldown
        lstrcpyA(saveMsgText, "SPEED SPRINT!");
        saveMsgTimer = 20;
        AddSparks(px * TS + TS/2, py * TS + TS/2, RGB(0, 255, 255), 15);
        MessageBeep(MB_ICONEXCLAMATION);
    }
}

void TriggerMagnetSkill() {
    if (magnetCooldown == 0 && !gameOver && !paused) {
        magnetSkillTimer = 50; // 5 seconds magnet
        magnetCooldown = 150;  // 15s cooldown
        lstrcpyA(saveMsgText, "DOT MAGNET!");
        saveMsgTimer = 20;
        AddSparks(px * TS + TS/2, py * TS + TS/2, RGB(255, 235, 59), 15);
        MessageBeep(MB_OK);
    }
}

void TriggerShieldSkill() {
    if (shieldCooldown == 0 && !gameOver && !paused) {
        shieldActive = 1;     // 1-hit barrier
        shieldHits = 1;
        shieldCooldown = 200; // 20s cooldown
        lstrcpyA(saveMsgText, "GHOST SHIELD!");
        saveMsgTimer = 20;
        AddSparks(px * TS + TS/2, py * TS + TS/2, RGB(0, 229, 255), 15);
        MessageBeep(MB_OK);
    }
}

void Update() {
    if (screenShake > 0) screenShake--;
    if (saveMsgTimer > 0) saveMsgTimer--;
    if (playerSlowTimer > 0) playerSlowTimer--;
    if (showHelp || gameOver || paused) return;

    if (deathTimer > 0) {
        deathTimer--;
        if (deathTimer % 2 == 0) {
            COLORREF colors[] = {RGB(255,0,0), RGB(0,255,0), RGB(0,0,255), RGB(255,255,0), RGB(0,255,255), RGB(255,0,255)};
            COLORREF rc = colors[MyRand() % 6];
            AddShockwave(px * TS + TS/2, py * TS + TS/2, rc);
            AddSparks(px * TS + TS/2, py * TS + TS/2, rc, 5);
        }
        if (deathTimer == 0) {
            lives--;
            MessageBeep(MB_ICONHAND);
            if (lives <= 0) {
                gameOver = 1;
                statsGamesPlayed++;
                if (score > statsMaxScore) statsMaxScore = score;
                SaveHighScore();
            } else {
                px = 7; py = 12;
                pdx = 0; pdy = 0; ndx = 0; ndy = 0;
                int effWave = (gameMode == 1) ? endlessWave : level;
                ghosts[0] = (Ghost){7, 6, RGB(255, 23, 68), 0, 0, 0, 0, 0, -1, (effWave >= 2) ? ((effWave % 5) + 1) : 0, 0, 0, 0};
                ghosts[1] = (Ghost){6, 7, RGB(240, 98, 146), 1, 0, 0, 0, -1, 0, (effWave >= 3) ? (((effWave + 2) % 5) + 1) : 0, 0, 0, 0};
                ghosts[2] = (Ghost){8, 7, RGB(0, 229, 255), 2, 0, 0, 0, 1, 0, (effWave >= 4) ? (((effWave + 3) % 5) + 1) : 0, 0, 0, 0};
                ghosts[3] = (Ghost){7, 7, RGB(255, 145, 0), 3, 0, 0, 0, 0, 1, (effWave >= 5) ? (((effWave + 4) % 5) + 1) : 0, 0, 0, 0};
                ghosts[4] = (Ghost){7, 5, RGB(170, 0, 255), 4, 0, 0, 0, 0, -1, (effWave >= 6) ? (((effWave + 1) % 5) + 1) : 0, 0, 0, 0};
                if (gameMode == 0 && level == 20) {
                    ghosts[5] = (Ghost){7, 6, RGB(255, 215, 0), 5, 0, 0, 0, 0, -1, 5, 0, 0, 0};
                }
            }
        }
        return;
    }

    if (replayMode == 2) {
        while (replayPlaybackIndex < replayCount && replays[replayPlaybackIndex].frame == frameCount) {
            int act = replays[replayPlaybackIndex].action;
            if (act == 1) { ndx = 0; ndy = -1; }
            else if (act == 2) { ndx = 0; ndy = 1; }
            else if (act == 3) { ndx = -1; ndy = 0; }
            else if (act == 4) { ndx = 1; ndy = 0; }
            else if (act == 5) { TriggerFreezeSkill(); }
            else if (act == 6) { TriggerSpeedSkill(); }
            else if (act == 7) { TriggerMagnetSkill(); }
            else if (act == 8) { TriggerShieldSkill(); }
            replayPlaybackIndex++;
        }
    }

    if (victoryTimer > 0) {
        victoryTimer--;
        if (victoryTimer == 0) {
            if (gameMode == 1) {
                endlessWave++;
                if (endlessWave > endlessHighWave) endlessHighWave = endlessWave;
                score += 1000 + endlessWave * 200;
                craftStarDust += 2;
                craftFruitEssence += 1;
                SaveHighScore();
                Init(1);
            } else {
                if (level == 20) {
                    gameOver = 2; // Victory!
                    statsGamesPlayed++;
                    SaveHighScore();
                } else {
                    level++;
                    Init(1);
                }
            }
        }
        return;
    }

    // Cooldown ticks
    if (freezeCooldown > 0) freezeCooldown--;
    if (speedCooldown > 0) speedCooldown--;
    if (magnetCooldown > 0) magnetCooldown--;
    if (shieldCooldown > 0) shieldCooldown--;

    if (freezeSkillTimer > 0) freezeSkillTimer--;
    if (frightTimer > 0) frightTimer--;

    // Update Sludge Traps
    for (int i = 0; i < numSludgeTraps; i++) {
        sludgeTraps[i].life--;
        if (sludgeTraps[i].life <= 0) {
            sludgeTraps[i] = sludgeTraps[--numSludgeTraps];
            i--;
        }
    }

    // Stage 20 Ghost King Phantom Clones Spawner
    if (gameMode == 0 && level == 20 && bossHp > 0) {
        phantomSpawnTimer += (bossHp <= bossMaxHp / 2) ? 2 : 1;
        if (phantomSpawnTimer >= 50) {
            phantomSpawnTimer = 0;
            for (int k = 6; k <= 7; k++) {
                if (ghosts[k].phantomTimer <= 0) {
                    ghosts[k] = (Ghost){7, 6, RGB(200, 100, 255), 6, 1, 80, 0, 0, 0, 0, 0, 0, 0};
                    if (numGhosts < 8) numGhosts = 8;
                    break;
                }
            }
        }
    }
    for (int k = 6; k <= 7; k++) {
        if (ghosts[k].isPhantom && ghosts[k].phantomTimer > 0) {
            ghosts[k].phantomTimer--;
        }
    }

    // Dead Ghost Returning Eyes Step
    for (int i = 0; i < numGhosts; i++) {
        if (ghosts[i].isDead) {
            if (frameCount % 2 == 0) {
                if (ghosts[i].x < 7) ghosts[i].x++;
                else if (ghosts[i].x > 7) ghosts[i].x--;
                if (ghosts[i].y < 6) ghosts[i].y++;
                else if (ghosts[i].y > 6) ghosts[i].y--;
                if (ghosts[i].x == 7 && ghosts[i].y == 6) ghosts[i].isDead = 0;
            }
        }
    }

    // Ghost Speed Logic
    int effLevel = (gameMode == 1) ? endlessWave : level;
    int ghostSpeed = 4 - (effLevel / 4);
    if (diffMode == 0) ghostSpeed += 1;
    else if (diffMode == 2) ghostSpeed = (ghostSpeed > 1) ? (ghostSpeed - 1) : 1;
    if (ghostSpeed < 1) ghostSpeed = 1;
    if (frightTimer > 0) ghostSpeed *= 2;

    // VIP Escort
    if (vipActive && freezeSkillTimer == 0) {
        if (frameCount % 3 == 0) {
            int dirs[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
            int d = MyRand() % 4;
            int nx = vipX + dirs[d][0];
            int ny = vipY + dirs[d][1];
            if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS && map[ny][nx] != 1) {
                vipX = nx; vipY = ny;
            }
        }
        for (int i = 0; i < numGhosts; i++) {
            if (!ghosts[i].isDead && ghosts[i].type != 6 && ghosts[i].x == vipX && ghosts[i].y == vipY && frightTimer == 0) {
                lives--;
                MessageBeep(MB_ICONHAND);
                vipX = 7; vipY = 7; px = 7; py = 12;
                if (lives <= 0) { gameOver = 1; SaveHighScore(); }
            }
        }
    }

    // Ghost Movement AI & Procedural Personalities
    if (freezeSkillTimer == 0 && frameCount % ghostSpeed == 0) {
        int dirs[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
        for (int i = 0; i < numGhosts; i++) {
            if (ghosts[i].isDead) continue;
            if (ghosts[i].isPhantom && ghosts[i].phantomTimer <= 0) continue;

            int oldX = ghosts[i].x, oldY = ghosts[i].y;

            // Trait 3: Trapper drops sludge
            if (ghosts[i].trait == 3 && (frameCount % 30 == 0) && numSludgeTraps < 16) {
                sludgeTraps[numSludgeTraps++] = (SludgeTrap){ghosts[i].x, ghosts[i].y, 90};
            }

            // Trait 1: Vortex Magnet pulls player slightly
            if (ghosts[i].trait == 1 && (frameCount % 10 == 0)) {
                int distToP = Abs(ghosts[i].x - px) + Abs(ghosts[i].y - py);
                if (distToP <= 3 && distToP > 1) {
                    if (px < ghosts[i].x && map[py][px+1] != 1) px++;
                    else if (px > ghosts[i].x && map[py][px-1] != 1) px--;
                }
            }

            // Trait 2: Glitch Shifter random teleport
            if (ghosts[i].trait == 2 && (MyRand() % 100 < 4)) {
                int rx = px + (MyRand() % 5 - 2);
                int ry = py + (MyRand() % 5 - 2);
                if (rx >= 0 && rx < COLS && ry >= 0 && ry < ROWS && map[ry][rx] != 1) {
                    ghosts[i].x = rx;
                    ghosts[i].y = ry;
                    AddSparks(rx * TS + TS/2, ry * TS + TS/2, RGB(255, 0, 255), 6);
                }
            }

            if (frightTimer == 0) {
                int tx = px, ty = py;

                if (ghosts[i].type == 1) { // Pinky (Interceptor): 3 tiles ahead
                    tx = px + pdx * 3;
                    ty = py + pdy * 3;
                } else if (ghosts[i].type == 2) { // Inky (Flanker): Behind Pac-Man
                    tx = px - pdx * 2;
                    ty = py - pdy * 2;
                } else if (ghosts[i].type == 3) { // Clyde (Patrol): Retreat if close
                    int distToPac = Abs(ghosts[i].x - px) + Abs(ghosts[i].y - py);
                    if (distToPac > 6) { tx = px; ty = py; }
                    else { tx = 0; ty = ROWS - 1; }
                } else if (ghosts[i].type == 4) { // Sue (Purple Stalker): Cutoff Stalker
                    tx = px - pdx * 3;
                    ty = py - pdy * 3;
                } else if (ghosts[i].type == 5) { // Ghost King Boss: Direct Aggressive Chase
                    if (bossHp <= bossMaxHp / 2) {
                        tx = px + pdx * 2;
                        ty = py + pdy * 2;
                    } else {
                        tx = px;
                        ty = py;
                    }
                } else if (ghosts[i].type == 6) { // Phantom Clone: Random Pursuit
                    tx = px + (MyRand() % 5 - 2);
                    ty = py + (MyRand() % 5 - 2);
                }

                // Trait 5: Hyper Chaser
                if (ghosts[i].trait == 5 && (ghosts[i].x == px || ghosts[i].y == py)) {
                    tx = px; ty = py;
                }

                int best_d = -1;
                int min_dist = 99999;
                int randChance = (diffMode == 0) ? 35 : ((diffMode == 2) ? 10 : 20);

                if (MyRand() % 100 < randChance) {
                    best_d = MyRand() % 4;
                } else {
                    for (int d = 0; d < 4; d++) {
                        int nx = ghosts[i].x + dirs[d][0];
                        int ny = ghosts[i].y + dirs[d][1];
                        if (nx < 0) nx = COLS - 1;
                        if (nx >= COLS) nx = 0;
                        if (ny >= 0 && ny < ROWS && map[ny][nx] != 1) {
                            int dist = Abs(nx - tx) + Abs(ny - ty);
                            if (dist < min_dist) { min_dist = dist; best_d = d; }
                        }
                    }
                }
                if (best_d != -1) {
                    int nx = ghosts[i].x + dirs[best_d][0];
                    int ny = ghosts[i].y + dirs[best_d][1];
                    if (nx < 0) nx = COLS - 1;
                    if (nx >= COLS) nx = 0;
                    if (ny >= 0 && ny < ROWS && map[ny][nx] != 1) {
                        ghosts[i].x = nx;
                        ghosts[i].y = ny;
                    }
                }
            } else {
                int d = MyRand() % 4;
                int nx = ghosts[i].x + dirs[d][0];
                int ny = ghosts[i].y + dirs[d][1];
                if (nx < 0) nx = COLS - 1;
                if (nx >= COLS) nx = 0;
                if (ny >= 0 && ny < ROWS && map[ny][nx] != 1) {
                    ghosts[i].x = nx;
                    ghosts[i].y = ny;
                }
            }
            ghosts[i].dirX = ghosts[i].x - oldX;
            ghosts[i].dirY = ghosts[i].y - oldY;
        }
    }

    // Player Direction Queue
    if (ndx != 0 || ndy != 0) {
        int nx = px + ndx;
        int ny = py + ndy;
        if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS && map[ny][nx] != 1) {
            pdx = ndx; pdy = ndy;
            ndx = 0; ndy = 0;
        }
    }

    // Speed Sprint Active Skill & Sludge Slowdown
    if (speedSkillTimer > 0) speedSkillTimer--;
    int moveTick = 2;
    if (speedSkillTimer > 0) moveTick = 1;
    if (playerSlowTimer > 0) moveTick = 3;

    int playerMoves = (moveTick == 1) ? 1 : (frameCount % moveTick == 0);

    if (playerMoves) {
        int nx = px + pdx;
        int ny = py + pdy;
        if (nx < 0) nx = COLS - 1;
        if (nx >= COLS) nx = 0;

        if (ny >= 0 && ny < ROWS && map[ny][nx] != 1) {
            px = nx;
            py = ny;

            // Check Sludge Trap collision
            for (int s = 0; s < numSludgeTraps; s++) {
                if (sludgeTraps[s].x == px && sludgeTraps[s].y == py) {
                    if (!shieldActive) {
                        playerSlowTimer = 30;
                        AddSparks(px * TS + TS/2, py * TS + TS/2, RGB(0, 255, 100), 5);
                    }
                }
            }

            if (map[py][px] == 6) {
                if (shieldActive) {
                    shieldHits--;
                    if (shieldHits <= 0) shieldActive = 0;
                    map[py][px] = 0;
                    AddShockwave(px * TS + TS/2, py * TS + TS/2, RGB(255, 69, 0));
                    lstrcpyA(saveMsgText, "HAZARD ABSORBED!");
                    saveMsgTimer = 20;
                    MessageBeep(MB_OK);
                } else {
                    lives--;
                    MessageBeep(MB_ICONHAND);
                    if (lives <= 0) {
                        gameOver = 1;
                        statsGamesPlayed++;
                        if (score > statsMaxScore) statsMaxScore = score;
                        SaveHighScore();
                    } else {
                        px = 7; py = 12; pdx = 0; pdy = 0; ndx = 0; ndy = 0;
                    }
                }
            } else if (map[py][px] >= 2 && map[py][px] <= 5) {
                int loopNum = (level - 1) / 20;
                int mult = (gameMode == 1) ? (1 + endlessWave / 2) : ((loopNum >= 7) ? 8 : 1);

                if (map[py][px] == 3) {
                    score += 40 * mult;
                    craftStarDust += 1;
                    frightTimer = (loopNum >= 7) ? 0 : ((diffMode == 0) ? 75 : ((diffMode == 2) ? 35 : 50));
                    AddExplosion(px * TS + TS/2, py * TS + TS/2, RGB(255, 184, 82));
                    MessageBeep(MB_OK);
                } else if (map[py][px] == 4) {
                    score += 20 * mult;
                    speedSkillTimer = 80;
                    AddShockwave(px * TS + TS/2, py * TS + TS/2, RGB(0, 255, 255));
                    MessageBeep(MB_ICONEXCLAMATION);
                } else if (map[py][px] == 5) {
                    score += 30 * mult;
                    freezeSkillTimer = 60;
                    AddShockwave(px * TS + TS/2, py * TS + TS/2, RGB(128, 222, 234));
                    MessageBeep(MB_ICONINFORMATION);
                } else {
                    score += 10 * mult;
                    if (dotCount % 25 == 0) craftStarDust++;
                }

                if (score > highScore) highScore = score;
                if (score > statsMaxScore) statsMaxScore = score;

                map[py][px] = 0;
                dotCount--;

                if (dotCount == 0) {
                    victoryTimer = 30;
                    MessageBeep(MB_ICONASTERISK);
                }
            } else if (map[py][px] == 8) {
                level += (MyRand() % 3) + 1;
                score += 1000;
                craftFruitEssence += 2;
                MessageBeep(MB_ICONASTERISK);
                Init(1);
                return;
            }
        }
    }

    // Dot Magnet Active Skill Logic
    if (magnetSkillTimer > 0) {
        magnetSkillTimer--;
        for (int r = py - 3; r <= py + 3; r++) {
            for (int c = px - 3; c <= px + 3; c++) {
                if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                    if (map[r][c] >= 2 && map[r][c] <= 5) {
                        int loopNum = (level - 1) / 20;
                        int mult = (gameMode == 1) ? (1 + endlessWave / 2) : ((loopNum >= 7) ? 8 : 1);
                        if (map[r][c] == 3) { score += 40 * mult; craftStarDust += 1; frightTimer = (loopNum >= 7) ? 0 : 50; AddExplosion(c * TS + TS/2, r * TS + TS/2, RGB(255, 184, 82)); }
                        else if (map[r][c] == 4) { score += 20 * mult; speedSkillTimer = 80; }
                        else if (map[r][c] == 5) { score += 30 * mult; freezeSkillTimer = 60; }
                        else { score += 10 * mult; }
                        map[r][c] = 0;
                        dotCount--;
                        if (score > highScore) highScore = score;
                        if (score > statsMaxScore) statsMaxScore = score;
                    }
                }
            }
        }
        if (fruitActive) {
            int mult = (gameMode == 1) ? (1 + endlessWave / 2) : 1;
            score += 500 * mult;
            craftFruitEssence += 2;
            fruitActive = 0;
        }
    }

    // Ghost Collisions
    for (int i = 0; i < numGhosts; i++) {
        if (ghosts[i].isDead) continue;
        if (ghosts[i].isPhantom && ghosts[i].phantomTimer <= 0) continue;

        if (px == ghosts[i].x && py == ghosts[i].y) {
            if (frightTimer > 0) {
                int loopNum = (level - 1) / 20;
                int mult = (gameMode == 1) ? (1 + endlessWave / 2) : ((loopNum >= 7) ? 8 : 1);
                craftEctoplasm += (ghosts[i].trait > 0 ? 2 : 1);

                if (ghosts[i].type == 5) { // Ghost King Boss
                    bossHp--;
                    score += 500 * mult;
                    craftEctoplasm += 3;
                    ghosts[i].x = 7; ghosts[i].y = 6;
                    AddExplosion(px * TS + TS/2, py * TS + TS/2, RGB(255, 215, 0));
                    MessageBeep(MB_ICONASTERISK);
                    if (bossHp <= 0) {
                        score += 2000 * mult;
                        victoryTimer = 30;
                        statsGamesPlayed++;
                        SaveHighScore();
                    }
                } else if (ghosts[i].type == 6) { // Phantom Clone
                    score += 100 * mult;
                    ghosts[i].phantomTimer = 0;
                    AddExplosion(px * TS + TS/2, py * TS + TS/2, RGB(206, 147, 216));
                } else {
                    score += 200 * mult;
                    statsGhostsEaten++;
                    if (score > highScore) highScore = score;
                    if (score > statsMaxScore) statsMaxScore = score;
                    AddExplosion(px * TS + TS/2, py * TS + TS/2, RGB(0, 255, 255));
                    MessageBeep(MB_ICONASTERISK);
                    ghosts[i].isDead = 1;
                }
            } else if (shieldActive) {
                shieldHits--;
                if (shieldHits <= 0) shieldActive = 0;
                ghosts[i].x = 7; ghosts[i].y = 6;
                AddShockwave(px * TS + TS/2, py * TS + TS/2, RGB(0, 229, 255));
                lstrcpyA(saveMsgText, "SHIELD ABSORBED!");
                saveMsgTimer = 20;
                MessageBeep(MB_OK);
            } else {
                deathTimer = 40;
                MessageBeep(MB_ICONHAND);
                break;
            }
        }
    }

    // Fruit Spawning
    if (dotCount < 40 && fruitActive == 0 && fruitTimer == 0 && (MyRand() % 150 == 0)) {
        fruitActive = 1;
        fruitTimer = 100;
    }
    if (fruitActive) {
        fruitTimer--;
        if (fruitTimer <= 0) fruitActive = 0;
        else if (px == 7 && py == 12) {
            int mult = (gameMode == 1) ? (1 + endlessWave / 2) : 1;
            score += 500 * mult;
            craftFruitEssence += 2;
            AddShockwave(7 * TS + TS/2, 12 * TS + TS/2, RGB(0, 230, 118));
            if (score > highScore) highScore = score;
            if (score > statsMaxScore) statsMaxScore = score;
            fruitActive = 0;
            MessageBeep(MB_ICONASTERISK);
        }
    }

    // Update Shockwaves
    for (int i = 0; i < numShockwaves; i++) {
        shockwaves[i].r += 2;
        if (shockwaves[i].r >= shockwaves[i].maxR) {
            shockwaves[i] = shockwaves[--numShockwaves];
            i--;
        }
    }

    for (int i = 0; i < numParticles; i++) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].vx *= 0.95;
        particles[i].vy += 0.15;
        particles[i].life--;
        if (particles[i].life <= 0) {
            particles[i] = particles[--numParticles];
            i--;
        }
    }

    frameCount++;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            LoadHighScore();
            Init(0);
            randSeed = GetTickCount();
            SetTimer(hwnd, 1, 100, NULL);
            break;
        case WM_KEYDOWN: {
            int key = wParam;
            if (key >= 'a' && key <= 'z') key -= 32;

            if (bindState > 0) {
                if (bindState == 1) bindUp = key;
                else if (bindState == 2) bindDown = key;
                else if (bindState == 3) bindLeft = key;
                else if (bindState == 4) bindRight = key;
                else if (bindState == 5) bindSkill1 = key;
                else if (bindState == 6) bindSkill2 = key;
                else if (bindState == 7) bindSkill3 = key;
                else if (bindState == 8) { bindSkill4 = key; bindState = 0; lstrcpyA(saveMsgText, "KEYS BOUND!"); saveMsgTimer = 30; }
                if (bindState > 0) bindState++;
                return 0;
            }

            if (key == 'K') { bindState = 1; return 0; }
            if (key == 'H' || key == VK_F1) { showHelp = !showHelp; return 0; }
            if (key == 'C') { showCraftMenu = !showCraftMenu; return 0; }
            if (key == 'O') {
                gameMode = !gameMode;
                Init(0);
                wsprintfA(saveMsgText, "MODE: %s", gameMode ? "ARCADE ENDLESS" : "CAMPAIGN");
                saveMsgTimer = 25;
                MessageBeep(MB_OK);
                return 0;
            }

            // Quick craft hotkeys
            if (key == '7' || (showCraftMenu && key == '1')) { CraftItem(1); return 0; }
            if (key == '8' || (showCraftMenu && key == '2')) { CraftItem(2); return 0; }
            if (key == '9' || (showCraftMenu && key == '3')) { CraftItem(3); return 0; }
            if (key == '0' || (showCraftMenu && key == '4')) { CraftItem(4); return 0; }

            if (showHelp || showCraftMenu) break;

            if (key == 'E') {
                HANDLE hFile = CreateFileA("kpac_data.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    char buf[256];
                    wsprintfA(buf, "{\"highScore\":%d,\"endlessHighWave\":%d,\"ecto\":%d,\"ess\":%d,\"dust\":%d}", highScore, endlessHighWave, craftEctoplasm, craftFruitEssence, craftStarDust);
                    DWORD written;
                    WriteFile(hFile, buf, lstrlenA(buf), &written, NULL);
                    CloseHandle(hFile);
                    lstrcpyA(saveMsgText, "EXPORTED JSON"); saveMsgTimer = 20; MessageBeep(MB_OK);
                }
                return 0;
            }
            if (key == 'I') {
                HANDLE hFile = CreateFileA("kpac_data.json", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    char buf[256]; buf[0] = 0;
                    DWORD readBytes = 0;
                    if (ReadFile(hFile, buf, sizeof(buf)-1, &readBytes, NULL) && readBytes > 0) {
                        lstrcpyA(saveMsgText, "IMPORTED JSON"); saveMsgTimer = 20; MessageBeep(MB_OK);
                    }
                    CloseHandle(hFile);
                }
                return 0;
            }

            if (key == 'R') {
                if (replayMode == 1) { replayMode = 0; lstrcpyA(saveMsgText, "RECORD STOPPED"); saveMsgTimer = 20; }
                else {
                    replayMode = 1; replayCount = 0;
                    replaySeed = GetTickCount(); randSeed = replaySeed;
                    replayDiffMode = diffMode; Init(0);
                    lstrcpyA(saveMsgText, "RECORDING..."); saveMsgTimer = 20;
                }
                return 0;
            }
            if (key == 'T') {
                if (replayMode == 2) { replayMode = 0; lstrcpyA(saveMsgText, "PLAYBACK STOPPED"); saveMsgTimer = 20; }
                else if (replayCount > 0) {
                    replayMode = 2; replayPlaybackIndex = 0;
                    randSeed = replaySeed; diffMode = replayDiffMode; Init(0);
                    lstrcpyA(saveMsgText, "PLAYING REPLAY"); saveMsgTimer = 20;
                }
                return 0;
            }

            if (replayMode == 2) break;

            if (key == bindLeft || key == 'A') {
                if (replayMode == 1 && replayCount < 9999 && (ndx != -1 || ndy != 0)) { replays[replayCount++] = (ReplayEvent){frameCount, 3}; }
                ndx = -1; ndy = 0;
            }
            if (key == bindRight || key == 'D') {
                if (replayMode == 1 && replayCount < 9999 && (ndx != 1 || ndy != 0)) { replays[replayCount++] = (ReplayEvent){frameCount, 4}; }
                ndx = 1; ndy = 0;
            }
            if (key == bindUp || key == 'W') {
                if (replayMode == 1 && replayCount < 9999 && (ndx != 0 || ndy != -1)) { replays[replayCount++] = (ReplayEvent){frameCount, 1}; }
                ndx = 0; ndy = -1;
            }
            if (key == bindDown || key == 'S') {
                if (replayMode == 1 && replayCount < 9999 && (ndx != 0 || ndy != 1)) { replays[replayCount++] = (ReplayEvent){frameCount, 2}; }
                ndx = 0; ndy = 1;
            }

            if (key == bindSkill1) { if (replayMode == 1 && replayCount < 9999) { replays[replayCount++] = (ReplayEvent){frameCount, 5}; } TriggerFreezeSkill(); }
            if (key == bindSkill2) { if (replayMode == 1 && replayCount < 9999) { replays[replayCount++] = (ReplayEvent){frameCount, 6}; } TriggerSpeedSkill(); }
            if (key == bindSkill3) { if (replayMode == 1 && replayCount < 9999) { replays[replayCount++] = (ReplayEvent){frameCount, 7}; } TriggerMagnetSkill(); }
            if (key == bindSkill4) { if (replayMode == 1 && replayCount < 9999) { replays[replayCount++] = (ReplayEvent){frameCount, 8}; } TriggerShieldSkill(); }

            if (key == VK_RETURN && gameOver) Init(0);
            if (key == 'P') paused = !paused;
            if (key == '1') { diffMode = 0; lstrcpyA(saveMsgText, "DIFF: EASY"); saveMsgTimer = 20; MessageBeep(MB_OK); }
            if (key == '2') { diffMode = 1; lstrcpyA(saveMsgText, "DIFF: NORMAL"); saveMsgTimer = 20; MessageBeep(MB_OK); }
            if (key == '3') { diffMode = 2; lstrcpyA(saveMsgText, "DIFF: HARD"); saveMsgTimer = 20; MessageBeep(MB_OK); }
            if (key == 'V') SaveGame();
            if (key == 'L') LoadGame();
            break;
        }
        case WM_TIMER:
            Update();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            SelectObject(memDC, hbm);
            static HFONT hFont = NULL;
            if (!hFont) {
                HDC screenDC = GetDC(NULL);
                int dpi = GetDeviceCaps(screenDC, LOGPIXELSY);
                ReleaseDC(NULL, screenDC);
                int fontHeight = -MulDiv(11, dpi, 72);
                hFont = CreateFontA(fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
            }
            SelectObject(memDC, hFont);
            HBRUSH bg = CreateSolidBrush(RGB(3, 6, 17));
            RECT rc = {0, 0, W, H};
            FillRect(memDC, &rc, bg);
            DeleteObject(bg);

            int offsetX = (W - 300) / 2;
            int offsetY = (H - 350) / 2 - 20;
            if (screenShake > 0) {
                int sx = (MyRand() % screenShake) - (screenShake / 2);
                int sy = (MyRand() % screenShake) - (screenShake / 2);
                SetWindowOrgEx(memDC, sx - offsetX, sy - offsetY, NULL);
            } else {
                SetWindowOrgEx(memDC, -offsetX, -offsetY, NULL);
            }

            // Cyber-grid background environmental art
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(10, 40, 80));
            SelectObject(memDC, gridPen);
            for (int i = 0; i <= W; i += 20) {
                MoveToEx(memDC, i, 0, NULL); LineTo(memDC, i, H);
            }
            for (int j = 0; j <= H; j += 20) {
                MoveToEx(memDC, 0, j, NULL); LineTo(memDC, W, j);
            }
            DeleteObject(gridPen);
            
            // Twinkling background stars
            HBRUSH starBr = CreateSolidBrush(RGB(150, 150, 150));
            for(int i = 0; i < 10; i++) {
                int sx = (int)(frameCount * 0.5 + i * 37) % W;
                int sy = (i * 53) % H;
                RECT starR = {sx, sy, sx+2, sy+2};
                FillRect(memDC, &starR, starBr);
            }
            DeleteObject(starBr);

            COLORREF themeCols[] = {
                RGB(30, 136, 229), RGB(76, 175, 80), RGB(156, 39, 176),
                RGB(244, 67, 54), RGB(255, 152, 0), RGB(0, 150, 136)
            };
            COLORREF themeHis[] = {
                RGB(100, 181, 246), RGB(129, 199, 132), RGB(186, 104, 200),
                RGB(229, 115, 115), RGB(255, 183, 77), RGB(77, 208, 225)
            };
            int themeIdx = ((gameMode == 1 ? endlessWave : level) - 1) % 6;
            COLORREF wallCol = themeCols[themeIdx];
            COLORREF wallHi = themeHis[themeIdx];
            if (victoryTimer > 0) {
                COLORREF flashCols[] = { RGB(255,255,255), RGB(0,255,255), RGB(255,215,0), RGB(30,136,229) };
                wallCol = flashCols[victoryTimer % 4];
                wallHi = RGB(255, 255, 255);
            }
            HBRUSH wallBr = CreateSolidBrush(RGB(8, 14, 30));
            HPEN wallPen = CreatePen(PS_SOLID, 2, wallCol);
            HPEN hiPen = CreatePen(PS_SOLID, 1, wallHi);
            HBRUSH capBr = CreateSolidBrush(wallHi);

            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    if (map[r][c] == 1) {
                        if (r == 6 && c == 7) {
                            int dist = 999;
                            for (int i = 0; i < numGhosts; i++) {
                                if (!ghosts[i].isDead && !(ghosts[i].isPhantom && ghosts[i].phantomTimer <= 0)) {
                                    int d = Abs(ghosts[i].x - c) + Abs(ghosts[i].y - r);
                                    if (d < dist) dist = d;
                                }
                            }
                            int ripple = dist < 3 ? (int)(MySin(frameCount * 0.5) * 4) : (int)(MySin(frameCount * 0.1) * 1);
                            HPEN doorPen = CreatePen(PS_SOLID, 2 + Abs(ripple), RGB(0, 255, 255));
                            SelectObject(memDC, doorPen);
                            MoveToEx(memDC, c * TS, r * TS + TS/2 + ripple, NULL);
                            LineTo(memDC, c * TS + TS, r * TS + TS/2 - ripple);
                            DeleteObject(doorPen);
                            continue;
                        }

                        RECT wr = {c * TS, r * TS, c * TS + TS, r * TS + TS};
                        FillRect(memDC, &wr, wallBr);
                        
                        SelectObject(memDC, wallPen);
                        RECT innerWr = {c * TS + 2, r * TS + 2, c * TS + TS - 2, r * TS + TS - 2};
                        FrameRect(memDC, &innerWr, wallBr);

                        int pulseInt = (int)(128 + 127 * MySin(frameCount * 0.4 + r + c));
                        HPEN pulsePen = CreatePen(PS_SOLID, 1, RGB(pulseInt, pulseInt, pulseInt));
                        SelectObject(memDC, pulsePen);
                        SelectObject(memDC, GetStockObject(NULL_BRUSH));
                        RECT pulseWr = {c * TS + 3, r * TS + 3, c * TS + TS - 3, r * TS + TS - 3};
                        Rectangle(memDC, pulseWr.left, pulseWr.top, pulseWr.right, pulseWr.bottom);
                        DeleteObject(pulsePen);

                        SelectObject(memDC, hiPen);
                        int nU = r > 0 && map[r-1][c] == 1;
                        int nD = r < ROWS-1 && map[r+1][c] == 1;
                        int nL = c > 0 && map[r][c-1] == 1;
                        int nR = c < COLS-1 && map[r][c+1] == 1;

                        if (!nU) { MoveToEx(memDC, c*TS, r*TS+1, NULL); LineTo(memDC, c*TS+TS, r*TS+1); }
                        if (!nD) { MoveToEx(memDC, c*TS, r*TS+TS-1, NULL); LineTo(memDC, c*TS+TS, r*TS+TS-1); }
                        if (!nL) { MoveToEx(memDC, c*TS+1, r*TS, NULL); LineTo(memDC, c*TS+1, r*TS+TS); }
                        if (!nR) { MoveToEx(memDC, c*TS+TS-1, r*TS, NULL); LineTo(memDC, c*TS+TS-1, r*TS+TS); }

                        if (nU && nR) { RECT cr = {c*TS+TS-3, r*TS, c*TS+TS, r*TS+3}; FillRect(memDC, &cr, capBr); }
                        if (nU && nL) { RECT cr = {c*TS, r*TS, c*TS+3, r*TS+3}; FillRect(memDC, &cr, capBr); }
                        if (nD && nR) { RECT cr = {c*TS+TS-3, r*TS+TS-3, c*TS+TS, r*TS+TS}; FillRect(memDC, &cr, capBr); }
                        if (nD && nL) { RECT cr = {c*TS, r*TS+TS-3, c*TS+3, r*TS+TS}; FillRect(memDC, &cr, capBr); }
                    } else if (map[r][c] == 2) {
                        int cx = c * TS + TS/2;
                        int cy = r * TS + TS/2;
                        
                        HBRUSH baseBr = CreateSolidBrush(RGB(216, 134, 59));
                        HBRUSH midBr = CreateSolidBrush(RGB(255, 200, 150));
                        HBRUSH hiBr = CreateSolidBrush(RGB(255, 255, 255));
                        
                        SelectObject(memDC, GetStockObject(NULL_PEN));
                        SelectObject(memDC, baseBr);
                        Ellipse(memDC, cx - 3, cy - 3, cx + 4, cy + 4);
                        SelectObject(memDC, midBr);
                        Ellipse(memDC, cx - 2, cy - 2, cx + 2, cy + 2);
                        SelectObject(memDC, hiBr);
                        Ellipse(memDC, cx - 1, cy - 1, cx + 1, cy + 1);
                        DeleteObject(baseBr); DeleteObject(midBr); DeleteObject(hiBr);
                    } else if (map[r][c] == 3) {
                        int pulse = (int)(MySin(frameCount * 0.3) * 2.0);
                        int haloPulse = (frameCount % 15);
                        
                        HPEN haloPen = CreatePen(PS_SOLID, 1, RGB(255, 215, 0));
                        SelectObject(memDC, haloPen);
                        SelectObject(memDC, GetStockObject(HOLLOW_BRUSH));
                        Ellipse(memDC, c * TS + 6 - haloPulse, r * TS + 6 - haloPulse, c * TS + 14 + haloPulse, r * TS + 14 + haloPulse);
                        DeleteObject(haloPen);
                        
                        HBRUSH ppBr = CreateSolidBrush(RGB(255, 184, 82));
                        SelectObject(memDC, ppBr);
                        SelectObject(memDC, GetStockObject(NULL_PEN));
                        Ellipse(memDC, c * TS + 4 - pulse, r * TS + 4 - pulse, c * TS + 16 + pulse, r * TS + 16 + pulse);
                        DeleteObject(ppBr);
                        
                        HBRUSH wBr = CreateSolidBrush(RGB(255, 255, 255));
                        SelectObject(memDC, wBr);
                        Ellipse(memDC, c * TS + 7 - pulse/2, r * TS + 7 - pulse/2, c * TS + 13 + pulse/2, r * TS + 13 + pulse/2);
                        DeleteObject(wBr);
                    } else if (map[r][c] == 4) {
                        HBRUSH spBr = CreateSolidBrush(RGB(0, 255, 255));
                        RECT dr = {c * TS + 7, r * TS + 7, c * TS + 13, r * TS + 13};
                        FillRect(memDC, &dr, spBr);
                        DeleteObject(spBr);
                    } else if (map[r][c] == 5) {
                        HBRUSH frBr = CreateSolidBrush(RGB(128, 222, 234));
                        RECT dr = {c * TS + 6, r * TS + 6, c * TS + 14, r * TS + 14};
                        FillRect(memDC, &dr, frBr);
                        DeleteObject(frBr);
                    } else if (map[r][c] == 6) {
                        HBRUSH hazBr = CreateSolidBrush(RGB(255, 69, 0));
                        int pulse = (frameCount % 4 == 0) ? 1 : 0;
                        RECT dr = {c * TS + 6 - pulse, r * TS + 6 - pulse, c * TS + 14 + pulse, r * TS + 14 + pulse};
                        FillRect(memDC, &dr, hazBr);
                        DeleteObject(hazBr);
                    } else if (map[r][c] == 8) {
                        HBRUSH brBr = CreateSolidBrush(RGB(200, 50, 255));
                        RECT dr = {c * TS + 4, r * TS + 4, c * TS + 16, r * TS + 16};
                        FillRect(memDC, &dr, brBr);
                        DeleteObject(brBr);
                    }
                }
            }
            DeleteObject(wallBr); DeleteObject(wallPen); DeleteObject(hiPen); DeleteObject(capBr);

            // Draw Sludge Traps
            for (int s = 0; s < numSludgeTraps; s++) {
                HBRUSH slBr = CreateSolidBrush(RGB(0, 230, 118));
                RECT sr = {sludgeTraps[s].x * TS + 5, sludgeTraps[s].y * TS + 5, sludgeTraps[s].x * TS + 15, sludgeTraps[s].y * TS + 15};
                FillRect(memDC, &sr, slBr);
                DeleteObject(slBr);
            }

            // Draw Shockwave Rings
            for (int i = 0; i < numShockwaves; i++) {
                HPEN sPen = CreatePen(PS_SOLID, 2, shockwaves[i].color);
                SelectObject(memDC, sPen);
                SelectObject(memDC, GetStockObject(HOLLOW_BRUSH));
                if (shockwaves[i].is3D) {
                    Ellipse(memDC, shockwaves[i].x - shockwaves[i].r, shockwaves[i].y - shockwaves[i].r / 2, shockwaves[i].x + shockwaves[i].r, shockwaves[i].y + shockwaves[i].r / 2);
                } else {
                    Ellipse(memDC, shockwaves[i].x - shockwaves[i].r, shockwaves[i].y - shockwaves[i].r,
                                   shockwaves[i].x + shockwaves[i].r, shockwaves[i].y + shockwaves[i].r);
                }
                DeleteObject(sPen);
            }

            for (int i = 0; i < numParticles; i++) {
                HBRUSH pBr = CreateSolidBrush(particles[i].color);
                RECT pR = {(int)particles[i].x - 1, (int)particles[i].y - 1, (int)particles[i].x + 2, (int)particles[i].y + 2};
                FillRect(memDC, &pR, pBr);
                DeleteObject(pBr);
            }

            // Draw Pac-Man
            int cx = px * TS + TS/2, cy = py * TS + TS/2;
            int radius = TS/2 - 1;
            
            COLORREF pacColor = RGB(255, 235, 59);
            if (shieldActive) pacColor = (shieldHits >= 2) ? RGB(0, 255, 255) : RGB(0, 229, 255);
            else if (speedSkillTimer > 0) pacColor = RGB(255, 152, 0);

            if (shieldActive || speedSkillTimer > 0 || pdx != 0 || pdy != 0) {
                HPEN auraPen = CreatePen(PS_SOLID, 2, shieldActive ? RGB(0, 255, 255) : (speedSkillTimer > 0 ? RGB(0, 255, 255) : RGB(255, 235, 59)));
                SelectObject(memDC, auraPen);
                SelectObject(memDC, GetStockObject(HOLLOW_BRUSH));
                Ellipse(memDC, cx - radius - 3, cy - radius - 3, cx + radius + 4, cy + radius + 4);
                DeleteObject(auraPen);
            }

            HBRUSH pacBr = CreateSolidBrush(pacColor);
            SelectObject(memDC, pacBr);
            SelectObject(memDC, GetStockObject(NULL_PEN));

            double baseAngle = 0;
            if (pdx == 1) baseAngle = 0;
            else if (pdx == -1) baseAngle = 3.14159;
            else if (pdy == 1) baseAngle = 3.14159 / 2.0;
            else if (pdy == -1) baseAngle = 3.14159 * 1.5;

            if (deathTimer > 0) {
                double foldProgress = (40.0 - deathTimer) / 40.0;
                radius = (int)((TS/2 - 1) * (1.0 - foldProgress));
                if (radius < 0) radius = 0;
                double foldMouth = 3.14159 * foldProgress;
                int xStart = cx + (int)(MyCos(baseAngle + foldMouth) * radius * 2);
                int yStart = cy + (int)(MySin(baseAngle + foldMouth) * radius * 2);
                int xEnd   = cx + (int)(MyCos(baseAngle - foldMouth) * radius * 2);
                int yEnd   = cy + (int)(MySin(baseAngle - foldMouth) * radius * 2);
                Pie(memDC, cx - radius, cy - radius, cx + radius + 1, cy + radius + 1, xStart, yStart, xEnd, yEnd);
                DeleteObject(pacBr);
            } else {
                double chompAngles[] = { 0.45 * 3.14159, 0.28 * 3.14159, 0.05 * 3.14159, 0.28 * 3.14159 };
                double mouth = chompAngles[frameCount % 4];
                int xStart = cx + (int)(MyCos(baseAngle + mouth) * radius * 2);
                int yStart = cy + (int)(MySin(baseAngle + mouth) * radius * 2);
                int xEnd   = cx + (int)(MyCos(baseAngle - mouth) * radius * 2);
                int yEnd   = cy + (int)(MySin(baseAngle - mouth) * radius * 2);
                Pie(memDC, cx - radius, cy - radius, cx + radius + 1, cy + radius + 1, xStart, yStart, xEnd, yEnd);
                DeleteObject(pacBr);
            }

            if (vipActive) {
                HBRUSH vipBr = CreateSolidBrush(RGB(76, 175, 80));
                SelectObject(memDC, vipBr);
                Ellipse(memDC, vipX * TS + 2, vipY * TS + 2, vipX * TS + TS - 2, vipY * TS + TS - 2);
                DeleteObject(vipBr);
            }

            // Draw Ghosts with Procedural Personalities & Trait Auras
            for (int i = 0; i < numGhosts; i++) {
                if (ghosts[i].isPhantom && ghosts[i].phantomTimer <= 0) continue;

                int gcx = ghosts[i].x * TS + TS/2, gcy = ghosts[i].y * TS + TS/2;
                int gw = TS - 4;
                int gx = gcx - gw/2, gy = gcy - gw/2;

                if (ghosts[i].isDead) {
                    int eyeDx = (7 - ghosts[i].x > 0) ? 2 : ((7 - ghosts[i].x < 0) ? -2 : 0);
                    int eyeDy = (6 - ghosts[i].y > 0) ? 2 : ((6 - ghosts[i].y < 0) ? -2 : 0);
                    HBRUSH wEyeBr = CreateSolidBrush(RGB(255, 255, 255));
                    HBRUSH bPupBr = CreateSolidBrush(RGB(21, 101, 192));
                    SelectObject(memDC, wEyeBr);
                    Ellipse(memDC, gcx - 6, gcy - 4, gcx - 1, gcy + 4);
                    Ellipse(memDC, gcx + 1, gcy - 4, gcx + 6, gcy + 4);
                    RECT p1 = {gcx - 5 + eyeDx, gcy - 2 + eyeDy, gcx - 2 + eyeDx, gcy + 1 + eyeDy};
                    RECT p2 = {gcx + 2 + eyeDx, gcy - 2 + eyeDy, gcx + 5 + eyeDx, gcy + 1 + eyeDy};
                    FillRect(memDC, &p1, bPupBr); FillRect(memDC, &p2, bPupBr);
                    DeleteObject(wEyeBr); DeleteObject(bPupBr);
                    continue;
                }

                int isScared = frightTimer > 0;
                int isFlashing = isScared && frightTimer < 15 && ((frightTimer / 2) % 2 == 0);
                COLORREF c = isScared ? (isFlashing ? RGB(255, 255, 255) : RGB(30, 136, 229)) : ghosts[i].c;

                // Trait Auras
                if (!isScared && ghosts[i].trait > 0) {
                    COLORREF traitCol = RGB(255,255,255);
                    if (ghosts[i].trait == 1) traitCol = RGB(0, 229, 255); // Vortex
                    else if (ghosts[i].trait == 2) traitCol = RGB(255, 0, 255); // Glitch
                    else if (ghosts[i].trait == 3) traitCol = RGB(0, 230, 118); // Trapper
                    else if (ghosts[i].trait == 4) traitCol = RGB(255, 215, 0); // Mirage
                    else if (ghosts[i].trait == 5) traitCol = RGB(255, 69, 0);  // Hyper Chaser
                    HPEN trPen = CreatePen(PS_SOLID, 1, traitCol);
                    SelectObject(memDC, trPen);
                    SelectObject(memDC, GetStockObject(HOLLOW_BRUSH));
                    Ellipse(memDC, gcx - gw/2 - 2, gcy - gw/2 - 2, gcx + gw/2 + 2, gcy + gw/2 + 2);
                    DeleteObject(trPen);
                }

                HBRUSH gBr = CreateSolidBrush(c);
                SelectObject(memDC, gBr);
                SelectObject(memDC, GetStockObject(NULL_PEN));
                Ellipse(memDC, gx, gy, gx + gw + 1, gy + gw + 1);
                int skirtOff = (frameCount % 2 == 0) ? 2 : -2;
                RECT gBodyR = {gx, gy + gw/2, gx + gw + 1, gy + gw - 1 + skirtOff};
                FillRect(memDC, &gBodyR, gBr);
                DeleteObject(gBr);

                if (ghosts[i].type == 5) {
                    HBRUSH crownBr = CreateSolidBrush(RGB(255, 215, 0));
                    POINT crownPts[5] = {{gcx - 5, gy - 2}, {gcx - 3, gy - 6}, {gcx, gy - 3}, {gcx + 3, gy - 6}, {gcx + 5, gy - 2}};
                    SelectObject(memDC, crownBr);
                    Polygon(memDC, crownPts, 5);
                    DeleteObject(crownBr);
                }

                if (!isScared) {
                    int eyeDx = (ghosts[i].dirX > 0) ? 2 : ((ghosts[i].dirX < 0) ? -2 : 0);
                    int eyeDy = (ghosts[i].dirY > 0) ? 2 : ((ghosts[i].dirY < 0) ? -2 : 0);
                    HBRUSH wEyeBr = CreateSolidBrush(RGB(255, 255, 255));
                    SelectObject(memDC, wEyeBr);
                    Ellipse(memDC, gcx - 6, gcy - 4, gcx - 1, gcy + 3);
                    Ellipse(memDC, gcx + 1, gcy - 4, gcx + 6, gcy + 3);
                    DeleteObject(wEyeBr);
                    HBRUSH bPupBr = CreateSolidBrush(RGB(13, 71, 161));
                    RECT p1 = {gcx - 5 + eyeDx, gcy - 2 + eyeDy, gcx - 2 + eyeDx, gcy + 1 + eyeDy};
                    RECT p2 = {gcx + 2 + eyeDx, gcy - 2 + eyeDy, gcx + 5 + eyeDx, gcy + 1 + eyeDy};
                    FillRect(memDC, &p1, bPupBr); FillRect(memDC, &p2, bPupBr);
                    DeleteObject(bPupBr);
                } else {
                    int scaredOffset = (frameCount % 4 < 2) ? 1 : -1;
                    HBRUSH scEyeBr = CreateSolidBrush(isFlashing ? RGB(213, 0, 0) : RGB(255, 255, 255));
                    SelectObject(memDC, scEyeBr);
                    Ellipse(memDC, gcx - 5, gcy - 4, gcx - 1, gcy);
                    Ellipse(memDC, gcx + 1, gcy - 4, gcx + 5, gcy);
                    DeleteObject(scEyeBr);
                }
            }

            // Draw Fruit
            if (fruitActive) {
                int bounceY = (int)(MySin(frameCount * 0.3) * 3.0);
                int fcx = 7 * TS + TS/2, fcy = 12 * TS + TS/2 + bounceY;
                HBRUSH cBr = CreateSolidBrush(RGB(213, 0, 0));
                SelectObject(memDC, cBr);
                Ellipse(memDC, fcx - 6, fcy, fcx + 1, fcy + 7);
                Ellipse(memDC, fcx, fcy + 1, fcx + 7, fcy + 8);
                DeleteObject(cBr);
            }

            SetWindowOrgEx(memDC, 0, 0, NULL);

            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            char sstr[128];
            if (gameMode == 1) {
                wsprintfA(sstr, "[ENDLESS W:%d/HI:%d] Sc:%d Lvs:%d", endlessWave, endlessHighWave, score, lives);
            } else {
                wsprintfA(sstr, "[CAMP Lv:%d/20] Sc:%d HI:%d Lvs:%d", level, score, highScore, lives);
            }
            TextOutA(memDC, 2, H - 75, sstr, lstrlenA(sstr));

            // Crafting Materials & HUD Bar
            wsprintfA(sstr, "Ecto:%d Ess:%d Dust:%d | [C]Forge [O]Mode", craftEctoplasm, craftFruitEssence, craftStarDust);
            SetTextColor(memDC, RGB(0, 255, 200));
            TextOutA(memDC, 2, H - 55, sstr, lstrlenA(sstr));

            char rText[16] = "";
            if (replayMode == 1) lstrcpyA(rText, " [REC]");
            else if (replayMode == 2) lstrcpyA(rText, " [PLAY]");

            wsprintfA(sstr, "F:%s Z:%s M:%s B:%s%s",
                freezeCooldown > 0 ? "CD" : "OK",
                speedCooldown > 0 ? "CD" : "OK",
                magnetCooldown > 0 ? "CD" : "OK",
                shieldActive ? "ON" : (shieldCooldown > 0 ? "CD" : "OK"), rText);
            SetTextColor(memDC, RGB(255, 235, 59));
            TextOutA(memDC, 2, H - 35, sstr, lstrlenA(sstr));
            
            SetTextColor(memDC, RGB(180, 180, 180));
            TextOutA(memDC, 2, H - 18, "[H]Help [K]Bind [7-0]Craft [V]Save [L]Load", 42);

            if (showCraftMenu) {
                HBRUSH overlay = CreateSolidBrush(RGB(5, 12, 25));
                RECT overlayRect = {10, 30, W - 10, H - 90};
                FillRect(memDC, &overlayRect, overlay);
                DeleteObject(overlay);
                HPEN bordPen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
                SelectObject(memDC, bordPen);
                SelectObject(memDC, GetStockObject(NULL_BRUSH));
                Rectangle(memDC, 10, 30, W - 10, H - 90);
                DeleteObject(bordPen);

                SetTextColor(memDC, RGB(0, 255, 255));
                TextOutA(memDC, 30, 45, "=== THE CYBER FORGE ===", 23);
                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, 25, 75, "1/7: Super Pellet (2 Ecto + 1 Dust)", 35);
                TextOutA(memDC, 25, 105, "2/8: Chrono Warp (2 Ecto + 2 Ess)", 33);
                TextOutA(memDC, 25, 135, "3/9: Aegis Shield (2 Dust + 2 Ess)", 34);
                TextOutA(memDC, 25, 165, "4/0: Void Pulse Bomb (3E+3D+1Ess)", 33);
                SetTextColor(memDC, RGB(255, 215, 0));
                char invStr[64];
                wsprintfA(invStr, "Current: %d Ecto, %d Ess, %d Dust", craftEctoplasm, craftFruitEssence, craftStarDust);
                TextOutA(memDC, 25, 205, invStr, lstrlenA(invStr));
                SetTextColor(memDC, RGB(0, 230, 118));
                TextOutA(memDC, 35, 235, "Press 1-4 or 7-0 to Craft | [C] Close", 37);
            } else if (showHelp) {
                HBRUSH overlay = CreateSolidBrush(RGB(0, 0, 0));
                RECT overlayRect = {0, 0, W, H};
                FillRect(memDC, &overlayRect, overlay);
                DeleteObject(overlay);
                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, 70, 40, "KPac Loop 10 - Help", 19);
                TextOutA(memDC, 30, 70, "Move: Arrows or WASD", 20);
                TextOutA(memDC, 30, 90, "Skills: F(Frz) Z(Spr) M(Mag) B(Shd)", 35);
                TextOutA(memDC, 30, 110, "Mode: [O] Toggle Campaign / Endless", 35);
                TextOutA(memDC, 30, 130, "Crafting: [C] Cyber-Forge Menu", 30);
                TextOutA(memDC, 30, 150, "Quick-Craft: 7(Pellet) 8(Warp) 9(Shield) 0(Bomb)", 48);
                TextOutA(memDC, 30, 170, "Diff: 1(Easy) 2(Norm) 3(Hard)", 29);
                TextOutA(memDC, 30, 190, "Save/Load: V / L | Pause: P", 27);
                SetTextColor(memDC, RGB(0, 230, 118));
                TextOutA(memDC, 40, 230, "Press H or F1 to Start/Resume", 29);
            }

            if (saveMsgTimer > 0) {
                SetTextColor(memDC, RGB(255, 255, 0));
                TextOutA(memDC, W/2 - 60, H/2 + 20, saveMsgText, lstrlenA(saveMsgText));
            }

            if (victoryTimer > 0) {
                SetTextColor(memDC, RGB(76, 175, 80));
                TextOutA(memDC, W/2 - 55, H/2 - 20, (gameMode == 1) ? "WAVE CLEARED!" : "MAZE CLEARED!", 13);
            } else if (gameOver) {
                if (gameOver == 2) {
                    SetTextColor(memDC, RGB(76, 175, 80));
                    TextOutA(memDC, W/2 - 70, H/2 - 20, "CAMPAIGN VICTORY!", 17);
                } else {
                    SetTextColor(memDC, RGB(244, 67, 54));
                    TextOutA(memDC, W/2 - 45, H/2 - 20, "GAME OVER", 9);
                }
            } else if (paused) {
                SetTextColor(memDC, RGB(255, 255, 0));
                TextOutA(memDC, W/2 - 30, H/2 - 10, "PAUSED", 6);
            }

            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            DeleteObject(hbm); DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KPacApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    RECT rect = {0, 0, W, H};
    AdjustWindowRect(&rect, (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN, FALSE);
    int winW = rect.right - rect.left;
    int winH = rect.bottom - rect.top;
    HWND hwnd = CreateWindowEx(0, "KPacApp", "KPac", (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, winW, winH, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
