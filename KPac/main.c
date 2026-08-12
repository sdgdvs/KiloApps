#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>

#pragma comment(lib, "msvcrt.lib")

#define W 300
#define H 350
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

// Player state
int px = 7, py = 12;
int pdx = 0, pdy = 0;
int ndx = 0, ndy = 0;

// Ghost struct (supports 5 standard ghosts + boss ghost + phantom clones)
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
} Ghost;

Ghost ghosts[8];
int numGhosts = 5;

// Active Skills & Timers
int freezeSkillTimer = 0, freezeCooldown = 0;
int speedSkillTimer = 0, speedCooldown = 0;
int magnetSkillTimer = 0, magnetCooldown = 0;
int shieldActive = 0, shieldCooldown = 0;

// Stage 20 Boss State
int bossHp = 8;
int bossMaxHp = 8;
int phantomSpawnTimer = 0;

// Game Loop State
int score = 0;
int highScore = 0;
int gameOver = 0;
int dotCount = 0;
int frameCount = 0;
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
} ShockwaveRing;

ShockwaveRing shockwaves[16];
int numShockwaves = 0;

void AddShockwave(int x, int y, COLORREF color) {
    if (numShockwaves < 16) {
        shockwaves[numShockwaves++] = (ShockwaveRing){x, y, 2, 28, color};
    }
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
    int frightTimer, freezeSkillTimer, speedSkillTimer, magnetSkillTimer, shieldActive;
    int freezeCooldown, speedCooldown, magnetCooldown, shieldCooldown;
    int bossHp;
    int dotCount, frameCount, fruitActive, fruitTimer, gameOver;
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
    st.magnetSkillTimer = magnetSkillTimer; st.shieldActive = shieldActive;
    st.freezeCooldown = freezeCooldown; st.speedCooldown = speedCooldown;
    st.magnetCooldown = magnetCooldown; st.shieldCooldown = shieldCooldown;
    st.bossHp = bossHp;
    st.dotCount = dotCount; st.frameCount = frameCount;
    st.fruitActive = fruitActive; st.fruitTimer = fruitTimer; st.gameOver = gameOver;

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
        magnetSkillTimer = st.magnetSkillTimer; shieldActive = st.shieldActive;
        freezeCooldown = st.freezeCooldown; speedCooldown = st.speedCooldown;
        magnetCooldown = st.magnetCooldown; shieldCooldown = st.shieldCooldown;
        bossHp = st.bossHp;
        dotCount = st.dotCount; frameCount = st.frameCount;
        fruitActive = st.fruitActive; fruitTimer = st.fruitTimer; gameOver = st.gameOver;
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

void Init(int keepScore) {
    if (!keepScore) { score = 0; level = 1; lives = GetInitLives(); }
    gameOver = 0;
    paused = 0;
    dotCount = 0;
    int mapIndex = (level - 1) % 20;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            map[r][c] = maps[mapIndex][r][c];
            if (map[r][c] >= 2 && map[r][c] <= 5) dotCount++;
        }
    }
    px = 7; py = 12;
    pdx = 0; pdy = 0;
    ndx = 0; ndy = 0;

    // Roster of 5 AI Ghosts:
    // Ghost 0: Blinky (Red Chaser)
    // Ghost 1: Pinky (Pink Interceptor)
    // Ghost 2: Inky (Cyan Flanker)
    // Ghost 3: Clyde (Orange Patrol)
    // Ghost 4: Sue (Purple Stalker)
    ghosts[0] = (Ghost){7, 6, RGB(255, 23, 68), 0, 0, 0, 0, 0, -1};
    ghosts[1] = (Ghost){6, 7, RGB(240, 98, 146), 1, 0, 0, 0, -1, 0};
    ghosts[2] = (Ghost){8, 7, RGB(0, 229, 255), 2, 0, 0, 0, 1, 0};
    ghosts[3] = (Ghost){7, 7, RGB(255, 145, 0), 3, 0, 0, 0, 0, 1};
    ghosts[4] = (Ghost){7, 5, RGB(170, 0, 255), 4, 0, 0, 0, 0, -1};

    if (level == 20) {
        // Stage 20 Ghost King Boss
        ghosts[5] = (Ghost){7, 6, RGB(255, 215, 0), 5, 0, 0, 0, 0, -1};
        ghosts[6] = (Ghost){0, 0, RGB(0,0,0), 6, 1, 0, 0, 0, 0}; // Phantom clone slot 1
        ghosts[7] = (Ghost){0, 0, RGB(0,0,0), 6, 1, 0, 0, 0, 0}; // Phantom clone slot 2
        numGhosts = 6;
        bossHp = 8;
        bossMaxHp = 8;
        phantomSpawnTimer = 0;
    } else if (level >= 4) {
        numGhosts = 5;
    } else if (level == 3) {
        numGhosts = 4;
    } else if (level == 2) {
        numGhosts = 3;
    } else {
        numGhosts = 2;
    }

    frightTimer = 0;
    victoryTimer = 0;
    freezeSkillTimer = 0; freezeCooldown = 0;
    speedSkillTimer = 0; speedCooldown = 0;
    magnetSkillTimer = 0; magnetCooldown = 0;
    shieldActive = 0; shieldCooldown = 0;
    fruitActive = 0;
    fruitTimer = 0;
    numShockwaves = 0;
}

// Active Skill Trigger Functions
void TriggerFreezeSkill() {
    if (freezeCooldown == 0 && !gameOver && !paused) {
        freezeSkillTimer = 60; // 6 seconds freeze
        freezeCooldown = 150;  // 15s cooldown
        lstrcpyA(saveMsgText, "FREEZE SKILL!");
        saveMsgTimer = 20;
        MessageBeep(MB_ICONINFORMATION);
    }
}

void TriggerSpeedSkill() {
    if (speedCooldown == 0 && !gameOver && !paused) {
        speedSkillTimer = 80; // 8 seconds 2x speed
        speedCooldown = 150;  // 15s cooldown
        lstrcpyA(saveMsgText, "SPEED SPRINT!");
        saveMsgTimer = 20;
        MessageBeep(MB_ICONEXCLAMATION);
    }
}

void TriggerMagnetSkill() {
    if (magnetCooldown == 0 && !gameOver && !paused) {
        magnetSkillTimer = 50; // 5 seconds magnet
        magnetCooldown = 150;  // 15s cooldown
        lstrcpyA(saveMsgText, "DOT MAGNET!");
        saveMsgTimer = 20;
        MessageBeep(MB_OK);
    }
}

void TriggerShieldSkill() {
    if (shieldCooldown == 0 && !gameOver && !paused) {
        shieldActive = 1;     // 1-hit invincible barrier
        shieldCooldown = 200; // 20s cooldown
        lstrcpyA(saveMsgText, "GHOST SHIELD!");
        saveMsgTimer = 20;
        MessageBeep(MB_OK);
    }
}

void Update() {
    if (saveMsgTimer > 0) saveMsgTimer--;
    if (showHelp || gameOver || paused) return;

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
            if (level == 20) {
                gameOver = 2; // Victory!
                statsGamesPlayed++;
                SaveHighScore();
            } else {
                level++;
                Init(1);
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

    // Stage 20 Ghost King Phantom Clones Spawner
    if (level == 20 && bossHp > 0) {
        phantomSpawnTimer++;
        if (phantomSpawnTimer >= 50) {
            phantomSpawnTimer = 0;
            for (int k = 6; k <= 7; k++) {
                if (ghosts[k].phantomTimer <= 0) {
                    ghosts[k] = (Ghost){7, 6, RGB(200, 100, 255), 6, 1, 80, 0, 0, 0};
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

    // Dead Ghost Returning Eyes Step (Move float eyes back to Ghost House)
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
    int ghostSpeed = 4 - (level / 4);
    if (diffMode == 0) ghostSpeed += 1;
    else if (diffMode == 2) ghostSpeed = (ghostSpeed > 1) ? (ghostSpeed - 1) : 1;
    if (ghostSpeed < 1) ghostSpeed = 1;
    if (frightTimer > 0) ghostSpeed *= 2;

    // Ghost Movement AI
    if (freezeSkillTimer == 0 && frameCount % ghostSpeed == 0) {
        int dirs[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
        for (int i = 0; i < numGhosts; i++) {
            if (ghosts[i].isDead) continue;
            if (ghosts[i].isPhantom && ghosts[i].phantomTimer <= 0) continue;

            int oldX = ghosts[i].x, oldY = ghosts[i].y;

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
                    tx = px;
                    ty = py;
                } else if (ghosts[i].type == 6) { // Phantom Clone: Random Pursuit
                    tx = px + (MyRand() % 5 - 2);
                    ty = py + (MyRand() % 5 - 2);
                }

                int loopNum = (level - 1) / 20;
                if (loopNum >= 7 && ghosts[i].type == 0 && (MyRand() % 100 < 5)) {
                    int nx = px + (MyRand() % 5 - 2);
                    int ny = py + (MyRand() % 5 - 2);
                    if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS && map[ny][nx] != 1) {
                        ghosts[i].x = nx;
                        ghosts[i].y = ny;
                    }
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

    // Speed Sprint Active Skill
    if (speedSkillTimer > 0) speedSkillTimer--;
    int playerMoves = (speedSkillTimer > 0) ? 1 : (frameCount % 2 == 0);

    if (playerMoves) {
        int nx = px + pdx;
        int ny = py + pdy;
        if (nx < 0) nx = COLS - 1;
        if (nx >= COLS) nx = 0;

        if (ny >= 0 && ny < ROWS && map[ny][nx] != 1) {
            px = nx;
            py = ny;
            if (map[py][px] == 6) {
                if (shieldActive) {
                    shieldActive = 0;
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
                        ghosts[0] = (Ghost){7, 6, RGB(255, 23, 68), 0, 0, 0, 0, 0, -1};
                        ghosts[1] = (Ghost){6, 7, RGB(240, 98, 146), 1, 0, 0, 0, -1, 0};
                        ghosts[2] = (Ghost){8, 7, RGB(0, 229, 255), 2, 0, 0, 0, 1, 0};
                        ghosts[3] = (Ghost){7, 7, RGB(255, 145, 0), 3, 0, 0, 0, 0, 1};
                        ghosts[4] = (Ghost){7, 5, RGB(170, 0, 255), 4, 0, 0, 0, 0, -1};
                        if (level % 20 == 0) ghosts[5] = (Ghost){7, 6, RGB(255, 215, 0), 5, 0, 0, 0, 0, -1};
                    }
                }
            } else if (map[py][px] >= 2 && map[py][px] <= 5) {
                int loopNum = (level - 1) / 20;
                int mult = (loopNum >= 7) ? 8 : 1;
                if (map[py][px] == 3) {
                    score += 40 * mult;
                    frightTimer = (loopNum >= 7) ? 0 : ((diffMode == 0) ? 75 : ((diffMode == 2) ? 35 : 50));
                    AddShockwave(px * TS + TS/2, py * TS + TS/2, RGB(255, 184, 82));
                    AddShockwave(px * TS + TS/2, py * TS + TS/2, RGB(255, 255, 255));
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
                }

                if (score > highScore) highScore = score;
                if (score > statsMaxScore) statsMaxScore = score;

                map[py][px] = 0;
                dotCount--;

                if (dotCount == 0) {
                    victoryTimer = 30;
                    MessageBeep(MB_ICONASTERISK);
                }
            }
        }
    }

    // Dot Magnet Active Skill Logic (Attract dots in 4-tile radius)
    if (magnetSkillTimer > 0) {
        magnetSkillTimer--;
        for (int r = py - 3; r <= py + 3; r++) {
            for (int c = px - 3; c <= px + 3; c++) {
                if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                    if (map[r][c] >= 2 && map[r][c] <= 5) {
                        int loopNum = (level - 1) / 20;
                        int mult = (loopNum >= 7) ? 8 : 1;
                        if (map[r][c] == 3) { score += 40 * mult; frightTimer = (loopNum >= 7) ? 0 : 50; AddShockwave(c * TS + TS/2, r * TS + TS/2, RGB(255, 184, 82)); }
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
            int loopNum = (level - 1) / 20;
            int mult = (loopNum >= 7) ? 8 : 1;
            score += 500 * mult;
            fruitActive = 0;
        }
    }

    // Ghost Collisions
    for (int i = 0; i < numGhosts; i++) {
        if (ghosts[i].isDead) continue; // Floating dead eyes don't collide with Pac-Man
        if (ghosts[i].isPhantom && ghosts[i].phantomTimer <= 0) continue;

        if (px == ghosts[i].x && py == ghosts[i].y) {
            if (frightTimer > 0) {
                int loopNum = (level - 1) / 20;
                int mult = (loopNum >= 7) ? 8 : 1;
                if (ghosts[i].type == 5) { // Ghost King Boss
                    bossHp--;
                    score += 500 * mult;
                    ghosts[i].x = 7; ghosts[i].y = 6;
                    AddShockwave(px * TS + TS/2, py * TS + TS/2, RGB(255, 215, 0));
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
                    AddShockwave(px * TS + TS/2, py * TS + TS/2, RGB(206, 147, 216));
                } else {
                    score += 200 * mult;
                    statsGhostsEaten++;
                    if (score > highScore) highScore = score;
                    if (score > statsMaxScore) statsMaxScore = score;
                    AddShockwave(px * TS + TS/2, py * TS + TS/2, RGB(0, 255, 255));
                    MessageBeep(MB_ICONASTERISK);
                    ghosts[i].isDead = 1; // Float eyes return to house!
                }
            } else if (shieldActive) {
                // Ghost Shield absorbs hit!
                shieldActive = 0;
                ghosts[i].x = 7; ghosts[i].y = 6;
                AddShockwave(px * TS + TS/2, py * TS + TS/2, RGB(0, 229, 255));
                lstrcpyA(saveMsgText, "SHIELD ABSORBED!");
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
                    px = 7; py = 12;
                    pdx = 0; pdy = 0;
                    ndx = 0; ndy = 0;
                    ghosts[0] = (Ghost){7, 6, RGB(255, 23, 68), 0, 0, 0, 0, 0, -1};
                    ghosts[1] = (Ghost){6, 7, RGB(240, 98, 146), 1, 0, 0, 0, -1, 0};
                    ghosts[2] = (Ghost){8, 7, RGB(0, 229, 255), 2, 0, 0, 0, 1, 0};
                    ghosts[3] = (Ghost){7, 7, RGB(255, 145, 0), 3, 0, 0, 0, 0, 1};
                    ghosts[4] = (Ghost){7, 5, RGB(170, 0, 255), 4, 0, 0, 0, 0, -1};
                    if (level == 20) {
                        ghosts[5] = (Ghost){7, 6, RGB(255, 215, 0), 5, 0, 0, 0, 0, -1};
                    }
                }
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
            int loopNum = (level - 1) / 20;
            int mult = (loopNum >= 7) ? 8 : 1;
            score += 500 * mult;
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

    int loopNum = (level - 1) / 20;
    if (loopNum >= 7 && (frameCount % 60 == 0)) {
        int hx = MyRand() % COLS;
        int hy = MyRand() % ROWS;
        if (map[hy][hx] == 0 && (hx != px || hy != py) && (hx != 7 || hy != 6)) {
            map[hy][hx] = 6;
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
            if (key == 'H') showHelp = !showHelp;
            if (showHelp) break;

            if (key == 'E') {
                HANDLE hFile = CreateFileA("kpac_data.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    char buf[256];
                    wsprintfA(buf, "{\"highScore\":%d,\"statsGamesPlayed\":%d,\"statsGhostsEaten\":%d,\"statsMaxScore\":%d}", highScore, statsGamesPlayed, statsGhostsEaten, statsMaxScore);
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
                        char *p = buf;
                        while (*p && (*p < '0' || *p > '9')) p++;
                        if (*p) {
                            int v1 = 0; while (*p >= '0' && *p <= '9') { v1 = v1 * 10 + (*p - '0'); p++; }
                            while (*p && (*p < '0' || *p > '9')) p++;
                            if (*p) {
                                int v2 = 0; while (*p >= '0' && *p <= '9') { v2 = v2 * 10 + (*p - '0'); p++; }
                                while (*p && (*p < '0' || *p > '9')) p++;
                                if (*p) {
                                    int v3 = 0; while (*p >= '0' && *p <= '9') { v3 = v3 * 10 + (*p - '0'); p++; }
                                    while (*p && (*p < '0' || *p > '9')) p++;
                                    if (*p) {
                                        int v4 = 0; while (*p >= '0' && *p <= '9') { v4 = v4 * 10 + (*p - '0'); p++; }
                                        highScore = v1; statsGamesPlayed = v2; statsGhostsEaten = v3; statsMaxScore = v4;
                                        SaveHighScore();
                                        lstrcpyA(saveMsgText, "IMPORTED JSON"); saveMsgTimer = 20; MessageBeep(MB_OK);
                                    }
                                }
                            }
                        }
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
            if (!hFont) hFont = CreateFontA(-14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
            SelectObject(memDC, hFont);
            HBRUSH bg = CreateSolidBrush(RGB(3, 6, 17));
            RECT rc = {0, 0, W, H};
            FillRect(memDC, &rc, bg);
            DeleteObject(bg);

            // Cyber-grid background environmental art
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(10, 40, 80));
            SelectObject(memDC, gridPen);
            for (int i = 0; i <= W; i += 20) {
                MoveToEx(memDC, i, 0, NULL);
                LineTo(memDC, i, H);
            }
            for (int j = 0; j <= H; j += 20) {
                MoveToEx(memDC, 0, j, NULL);
                LineTo(memDC, W, j);
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

            // Draw Maze Map with Neon Glow and Junction Caps
            COLORREF wallCol = RGB(30, 136, 229);
            COLORREF wallHi = RGB(100, 181, 246);
            if (victoryTimer > 0) {
                COLORREF flashCols[] = { RGB(255,255,255), RGB(0,255,255), RGB(255,215,0), RGB(30,136,229) };
                wallCol = flashCols[victoryTimer % 4];
                wallHi = RGB(255, 255, 255);
            }
            HBRUSH wallBr = CreateSolidBrush(RGB(8, 14, 30));
            HPEN wallPen = CreatePen(PS_SOLID, 2, wallCol);
            HPEN hiPen = CreatePen(PS_SOLID, 1, wallHi);
            HBRUSH capBr = CreateSolidBrush(wallHi);
            HBRUSH dotBr = CreateSolidBrush(RGB(255, 200, 150));

            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    if (map[r][c] == 1) {
                        RECT wr = {c * TS, r * TS, c * TS + TS, r * TS + TS};
                        FillRect(memDC, &wr, wallBr);
                        
                        SelectObject(memDC, wallPen);
                        RECT innerWr = {c * TS + 2, r * TS + 2, c * TS + TS - 2, r * TS + TS - 2};
                        FrameRect(memDC, &innerWr, wallBr);

                        SelectObject(memDC, hiPen);
                        int nU = r > 0 && map[r-1][c] == 1;
                        int nD = r < ROWS-1 && map[r+1][c] == 1;
                        int nL = c > 0 && map[r][c-1] == 1;
                        int nR = c < COLS-1 && map[r][c+1] == 1;

                        if (!nU) { MoveToEx(memDC, c*TS, r*TS+1, NULL); LineTo(memDC, c*TS+TS, r*TS+1); }
                        if (!nD) { MoveToEx(memDC, c*TS, r*TS+TS-1, NULL); LineTo(memDC, c*TS+TS, r*TS+TS-1); }
                        if (!nL) { MoveToEx(memDC, c*TS+1, r*TS, NULL); LineTo(memDC, c*TS+1, r*TS+TS); }
                        if (!nR) { MoveToEx(memDC, c*TS+TS-1, r*TS, NULL); LineTo(memDC, c*TS+TS-1, r*TS+TS); }

                        // Junction Caps
                        if (nU && nR) { RECT cr = {c*TS+TS-3, r*TS, c*TS+TS, r*TS+3}; FillRect(memDC, &cr, capBr); }
                        if (nU && nL) { RECT cr = {c*TS, r*TS, c*TS+3, r*TS+3}; FillRect(memDC, &cr, capBr); }
                        if (nD && nR) { RECT cr = {c*TS+TS-3, r*TS+TS-3, c*TS+TS, r*TS+TS}; FillRect(memDC, &cr, capBr); }
                        if (nD && nL) { RECT cr = {c*TS, r*TS+TS-3, c*TS+3, r*TS+TS}; FillRect(memDC, &cr, capBr); }
                    } else if (map[r][c] == 2) {
                        RECT dr = {c * TS + 8, r * TS + 8, c * TS + 12, r * TS + 12};
                        FillRect(memDC, &dr, dotBr);
                    } else if (map[r][c] == 3) {
                        // Pulsing Power Pellet
                        int pulse = (int)(MySin(frameCount * 0.3) * 2.0);
                        HBRUSH ppBr = CreateSolidBrush(RGB(255, 184, 82));
                        SelectObject(memDC, ppBr);
                        Ellipse(memDC, c * TS + 4 - pulse, r * TS + 4 - pulse, c * TS + 16 + pulse, r * TS + 16 + pulse);
                        DeleteObject(ppBr);
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
                    }
                }
            }
            DeleteObject(wallBr); DeleteObject(wallPen); DeleteObject(hiPen); DeleteObject(capBr); DeleteObject(dotBr);

            // Draw Shockwave Rings
            for (int i = 0; i < numShockwaves; i++) {
                HPEN sPen = CreatePen(PS_SOLID, 2, shockwaves[i].color);
                SelectObject(memDC, sPen);
                SelectObject(memDC, GetStockObject(HOLLOW_BRUSH));
                Ellipse(memDC, shockwaves[i].x - shockwaves[i].r, shockwaves[i].y - shockwaves[i].r,
                               shockwaves[i].x + shockwaves[i].r, shockwaves[i].y + shockwaves[i].r);
                DeleteObject(sPen);
            }

            // Draw Pac-Man with 4-Frame Chomp & Direction Mouth Angle
            int cx = px * TS + TS/2, cy = py * TS + TS/2;
            int radius = TS/2 - 1;
            
            COLORREF pacColor = RGB(255, 235, 59);
            if (shieldActive) pacColor = RGB(0, 229, 255);
            else if (speedSkillTimer > 0) pacColor = RGB(255, 152, 0);

            // Speed Sprint Aura Ring
            if (speedSkillTimer > 0 || pdx != 0 || pdy != 0) {
                HPEN auraPen = CreatePen(PS_SOLID, 2, speedSkillTimer > 0 ? RGB(0, 255, 255) : RGB(255, 235, 59));
                SelectObject(memDC, auraPen);
                SelectObject(memDC, GetStockObject(HOLLOW_BRUSH));
                Ellipse(memDC, cx - radius - 3, cy - radius - 3, cx + radius + 4, cy + radius + 4);
                DeleteObject(auraPen);
            }

            HBRUSH pacBr = CreateSolidBrush(pacColor);
            SelectObject(memDC, pacBr);
            SelectObject(memDC, GetStockObject(NULL_PEN));

            // Direction Angle
            double baseAngle = 0;
            if (pdx == 1) baseAngle = 0;
            else if (pdx == -1) baseAngle = 3.14159;
            else if (pdy == 1) baseAngle = 3.14159 / 2.0;
            else if (pdy == -1) baseAngle = 3.14159 * 1.5;

            double chompAngles[] = { 0.45 * 3.14159, 0.28 * 3.14159, 0.05 * 3.14159, 0.28 * 3.14159 };
            double mouth = chompAngles[frameCount % 4];

            int xStart = cx + (int)(MyCos(baseAngle + mouth) * radius * 2);
            int yStart = cy + (int)(MySin(baseAngle + mouth) * radius * 2);
            int xEnd   = cx + (int)(MyCos(baseAngle - mouth) * radius * 2);
            int yEnd   = cy + (int)(MySin(baseAngle - mouth) * radius * 2);

            Pie(memDC, cx - radius, cy - radius, cx + radius + 1, cy + radius + 1, xStart, yStart, xEnd, yEnd);
            DeleteObject(pacBr);

            // Pac-Man Eye
            double eyeAng = baseAngle - 3.14159 / 3.0;
            int ex = cx + (int)(MyCos(eyeAng) * 4);
            int ey = cy + (int)(MySin(eyeAng) * 4);
            HBRUSH eyeBr = CreateSolidBrush(RGB(0, 0, 0));
            RECT eyeR = {ex - 1, ey - 1, ex + 2, ey + 2};
            FillRect(memDC, &eyeR, eyeBr);
            DeleteObject(eyeBr);

            // Draw Ghosts (Normal, Scared Warning Flash, Floating Return Eyes)
            for (int i = 0; i < numGhosts; i++) {
                if (ghosts[i].isPhantom && ghosts[i].phantomTimer <= 0) continue;

                int gcx = ghosts[i].x * TS + TS/2, gcy = ghosts[i].y * TS + TS/2;
                int gw = TS - 4;
                int gx = gcx - gw/2, gy = gcy - gw/2;

                if (ghosts[i].isDead) {
                    // Floating Return Eyes
                    int eyeDx = (7 - ghosts[i].x > 0) ? 2 : ((7 - ghosts[i].x < 0) ? -2 : 0);
                    int eyeDy = (6 - ghosts[i].y > 0) ? 2 : ((6 - ghosts[i].y < 0) ? -2 : 0);
                    HBRUSH wEyeBr = CreateSolidBrush(RGB(255, 255, 255));
                    HBRUSH bPupBr = CreateSolidBrush(RGB(21, 101, 192));

                    SelectObject(memDC, wEyeBr);
                    Ellipse(memDC, gcx - 6, gcy - 4, gcx - 1, gcy + 4);
                    Ellipse(memDC, gcx + 1, gcy - 4, gcx + 6, gcy + 4);

                    RECT p1 = {gcx - 5 + eyeDx, gcy - 2 + eyeDy, gcx - 2 + eyeDx, gcy + 1 + eyeDy};
                    RECT p2 = {gcx + 2 + eyeDx, gcy - 2 + eyeDy, gcx + 5 + eyeDx, gcy + 1 + eyeDy};
                    FillRect(memDC, &p1, bPupBr);
                    FillRect(memDC, &p2, bPupBr);

                    DeleteObject(wEyeBr); DeleteObject(bPupBr);
                    continue;
                }

                int isScared = frightTimer > 0;
                int isFlashing = isScared && frightTimer < 15 && ((frightTimer / 2) % 2 == 0);
                COLORREF c = isScared ? (isFlashing ? RGB(255, 255, 255) : RGB(30, 136, 229)) : ghosts[i].c;

                HBRUSH gBr = CreateSolidBrush(c);
                SelectObject(memDC, gBr);
                SelectObject(memDC, GetStockObject(NULL_PEN));

                // Dome head
                Ellipse(memDC, gx, gy, gx + gw + 1, gy + gw + 1);
                // Body skirt base
                int skirtOff = (frameCount % 2 == 0) ? 2 : -2;
                RECT gBodyR = {gx, gy + gw/2, gx + gw + 1, gy + gw - 1 + skirtOff};
                FillRect(memDC, &gBodyR, gBr);
                DeleteObject(gBr);

                if (ghosts[i].type == 5) { // Ghost King Crown
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
                    HBRUSH bPupBr = CreateSolidBrush(RGB(13, 71, 161));
                    SelectObject(memDC, wEyeBr);
                    Ellipse(memDC, gcx - 6, gcy - 4, gcx - 1, gcy + 3);
                    Ellipse(memDC, gcx + 1, gcy - 4, gcx + 6, gcy + 3);
                    RECT p1 = {gcx - 5 + eyeDx, gcy - 2 + eyeDy, gcx - 2 + eyeDx, gcy + 1 + eyeDy};
                    RECT p2 = {gcx + 2 + eyeDx, gcy - 2 + eyeDy, gcx + 5 + eyeDx, gcy + 1 + eyeDy};
                    FillRect(memDC, &p1, bPupBr);
                    FillRect(memDC, &p2, bPupBr);
                    DeleteObject(wEyeBr); DeleteObject(bPupBr);
                } else {
                    HBRUSH scEyeBr = CreateSolidBrush(isFlashing ? RGB(213, 0, 0) : RGB(255, 255, 255));
                    RECT e1 = {gcx - 4, gcy - 3, gcx - 2, gcy - 1};
                    RECT e2 = {gcx + 2, gcy - 3, gcx + 4, gcy - 1};
                    FillRect(memDC, &e1, scEyeBr);
                    FillRect(memDC, &e2, scEyeBr);
                    DeleteObject(scEyeBr);
                }
            }

            // Draw Fruit with Float Bounce & Level Variety
            if (fruitActive) {
                int bounceY = (int)(MySin(frameCount * 0.3) * 3.0);
                int fcx = 7 * TS + TS/2, fcy = 12 * TS + TS/2 + bounceY;
                int fruitType = ((level - 1) / 2) % 5; // 0=Cherry, 1=Strawberry, 2=Peach, 3=Apple, 4=Melon

                if (fruitType == 0) { // Cherry
                    HBRUSH cBr = CreateSolidBrush(RGB(213, 0, 0));
                    SelectObject(memDC, cBr);
                    Ellipse(memDC, fcx - 6, fcy, fcx + 1, fcy + 7);
                    Ellipse(memDC, fcx, fcy + 1, fcx + 7, fcy + 8);
                    DeleteObject(cBr);
                    HPEN stPen = CreatePen(PS_SOLID, 1, RGB(76, 175, 80));
                    SelectObject(memDC, stPen);
                    MoveToEx(memDC, fcx - 3, fcy + 1, NULL); LineTo(memDC, fcx + 2, fcy - 5);
                    MoveToEx(memDC, fcx + 3, fcy + 2, NULL); LineTo(memDC, fcx + 2, fcy - 5);
                    DeleteObject(stPen);
                } else if (fruitType == 1) { // Strawberry
                    HBRUSH sBr = CreateSolidBrush(RGB(229, 57, 53));
                    POINT sPts[3] = {{fcx, fcy + 6}, {fcx - 5, fcy - 2}, {fcx + 5, fcy - 2}};
                    SelectObject(memDC, sBr); Polygon(memDC, sPts, 3); DeleteObject(sBr);
                    HBRUSH lBr = CreateSolidBrush(RGB(76, 175, 80));
                    RECT lr = {fcx - 4, fcy - 4, fcx + 4, fcy - 1}; FillRect(memDC, &lr, lBr); DeleteObject(lBr);
                } else if (fruitType == 2) { // Peach/Orange
                    HBRUSH oBr = CreateSolidBrush(RGB(255, 152, 0));
                    SelectObject(memDC, oBr); Ellipse(memDC, fcx - 5, fcy - 4, fcx + 6, fcy + 7); DeleteObject(oBr);
                } else if (fruitType == 3) { // Apple
                    HBRUSH aBr = CreateSolidBrush(RGB(244, 67, 54));
                    SelectObject(memDC, aBr); Ellipse(memDC, fcx - 5, fcy - 4, fcx + 6, fcy + 7); DeleteObject(aBr);
                    HBRUSH stBr = CreateSolidBrush(RGB(121, 85, 72));
                    RECT str = {fcx - 1, fcy - 6, fcx + 1, fcy - 3}; FillRect(memDC, &str, stBr); DeleteObject(stBr);
                } else { // Melon Slice
                    HBRUSH mBr = CreateSolidBrush(RGB(46, 125, 50));
                    SelectObject(memDC, mBr); Chord(memDC, fcx - 6, fcy - 6, fcx + 6, fcy + 6, fcx + 6, fcy, fcx - 6, fcy); DeleteObject(mBr);
                    HBRUSH rBr = CreateSolidBrush(RGB(229, 57, 53));
                    SelectObject(memDC, rBr); Chord(memDC, fcx - 4, fcy - 4, fcx + 4, fcy + 4, fcx + 4, fcy, fcx - 4, fcy); DeleteObject(rBr);
                }
            }

            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            char sstr[128];
            wsprintfA(sstr, "Lv:%d/20 Sc:%d HI:%d Lvs:%d", level, score, highScore, lives);
            TextOutA(memDC, 2, 305, sstr, lstrlenA(sstr));

            // Skill HUD Line
            char rText[16] = "";
            if (replayMode == 1) lstrcpyA(rText, " [REC]");
            else if (replayMode == 2) lstrcpyA(rText, " [PLAY]");

            wsprintfA(sstr, "F:%s Z:%s M:%s B:%s%s",
                freezeCooldown > 0 ? "CD" : "OK",
                speedCooldown > 0 ? "CD" : "OK",
                magnetCooldown > 0 ? "CD" : "OK",
                shieldCooldown > 0 ? "CD" : (shieldActive ? "ON" : "OK"), rText);
            SetTextColor(memDC, RGB(255, 235, 59));
            TextOutA(memDC, 2, 320, sstr, lstrlenA(sstr));
            
            SetTextColor(memDC, RGB(0, 255, 200));
            TextOutA(memDC, 2, 335, "[Press H for Help] [K]Bind [E]Exp", 33);

            if (level == 20 && bossHp > 0) {
                char bossStr[64];
                wsprintfA(bossStr, "BOSS KING HP: %d/%d", bossHp, bossMaxHp);
                SetTextColor(memDC, RGB(255, 215, 0));
                TextOutA(memDC, W - 130, 305, bossStr, lstrlenA(bossStr));
            }
            
            if (bindState > 0) {
                HBRUSH overlay = CreateSolidBrush(RGB(0, 0, 0));
                RECT overlayRect = {0, 0, W, H};
                FillRect(memDC, &overlayRect, overlay);
                DeleteObject(overlay);
                SetTextColor(memDC, RGB(255, 100, 100));
                char bindMsg[64];
                char* bindNames[] = {"", "UP", "DOWN", "LEFT", "RIGHT", "SKILL1(FREEZE)", "SKILL2(SPRINT)", "SKILL3(MAGNET)", "SKILL4(SHIELD)"};
                wsprintfA(bindMsg, "PRESS KEY FOR %s", bindNames[bindState]);
                TextOutA(memDC, W/2 - 90, H/2, bindMsg, lstrlenA(bindMsg));
            } else if (showHelp) {
                HBRUSH overlay = CreateSolidBrush(RGB(0, 0, 0));
                RECT overlayRect = {0, 0, W, H};
                FillRect(memDC, &overlayRect, overlay);
                DeleteObject(overlay);
                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, 70, 40, "KPac - Help & Controls", 22);
                TextOutA(memDC, 70, 70, "Move: Arrows or WASD", 20);
                TextOutA(memDC, 70, 90, "Skills: F, Z, M, B", 18);
                TextOutA(memDC, 70, 110, "Diff: 1(Easy) 2(Norm) 3(Hard)", 29);
                TextOutA(memDC, 70, 130, "Save/Load: V / L | Pause: P", 27);
                TextOutA(memDC, 70, 150, "Replay: R(Rec) T(Play)", 22);
                TextOutA(memDC, 50, 180, "Avoid ghosts, eat all dots.", 27);
                TextOutA(memDC, 30, 200, "Power pellets let you eat ghosts!", 33);
                SetTextColor(memDC, RGB(0, 230, 118));
                TextOutA(memDC, 60, 250, "Press H to start/resume", 23);
            }

            if (saveMsgTimer > 0) {
                SetTextColor(memDC, RGB(255, 255, 0));
                TextOutA(memDC, W/2 - 45, H/2 + 30, saveMsgText, lstrlenA(saveMsgText));
            }

            if (victoryTimer > 0) {
                SetTextColor(memDC, RGB(76, 175, 80));
                TextOutA(memDC, W/2 - 55, H/2 - 20, "MAZE CLEARED!", 13);
            } else if (gameOver) {
                if (gameOver == 2) {
                    SetTextColor(memDC, RGB(76, 175, 80));
                    TextOutA(memDC, W/2 - 70, H/2 - 20, "CAMPAIGN VICTORY!", 17);
                } else {
                    SetTextColor(memDC, RGB(244, 67, 54));
                    TextOutA(memDC, W/2 - 45, H/2 - 20, "GAME OVER", 9);
                }
                char statStr[128];
                wsprintfA(statStr, "Gms: %d Ghsts: %d Max: %d", statsGamesPlayed, statsGhostsEaten, statsMaxScore);
                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, 10, H/2 + 10, statStr, lstrlenA(statStr));
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
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    int winW = rect.right - rect.left;
    int winH = rect.bottom - rect.top;
    HWND hwnd = CreateWindowEx(0, "KPacApp", "KPac", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
