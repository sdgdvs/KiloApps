#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <math.h>

#pragma comment(lib, "msvcrt.lib")

#define W 300
#define H 340
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
int lives = 3;
int paused = 0;
int fruitActive = 0;
int fruitTimer = 0;

int diffMode = 1; // 0 = Easy, 1 = Normal, 2 = Hard
char saveMsgText[64] = "";
int saveMsgTimer = 0;

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
    ghosts[0] = (Ghost){7, 6, RGB(255, 23, 68), 0, 0, 0};
    ghosts[1] = (Ghost){6, 7, RGB(240, 98, 146), 1, 0, 0};
    ghosts[2] = (Ghost){8, 7, RGB(0, 229, 255), 2, 0, 0};
    ghosts[3] = (Ghost){7, 7, RGB(255, 145, 0), 3, 0, 0};
    ghosts[4] = (Ghost){7, 5, RGB(170, 0, 255), 4, 0, 0};

    if (level == 20) {
        // Stage 20 Ghost King Boss
        ghosts[5] = (Ghost){7, 6, RGB(255, 215, 0), 5, 0, 0};
        ghosts[6] = (Ghost){0, 0, RGB(0,0,0), 6, 1, 0}; // Phantom clone slot 1
        ghosts[7] = (Ghost){0, 0, RGB(0,0,0), 6, 1, 0}; // Phantom clone slot 2
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
    freezeSkillTimer = 0; freezeCooldown = 0;
    speedSkillTimer = 0; speedCooldown = 0;
    magnetSkillTimer = 0; magnetCooldown = 0;
    shieldActive = 0; shieldCooldown = 0;
    fruitActive = 0;
    fruitTimer = 0;
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
    if (gameOver || paused) return;

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
                    ghosts[k] = (Ghost){7, 6, RGB(200, 100, 255), 6, 1, 80};
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
            if (ghosts[i].isPhantom && ghosts[i].phantomTimer <= 0) continue;

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
            if (map[py][px] >= 2 && map[py][px] <= 5) {
                if (map[py][px] == 3) {
                    score += 40;
                    frightTimer = (diffMode == 0) ? 75 : ((diffMode == 2) ? 35 : 50);
                    MessageBeep(MB_OK);
                } else if (map[py][px] == 4) {
                    score += 20;
                    speedSkillTimer = 80;
                    MessageBeep(MB_ICONEXCLAMATION);
                } else if (map[py][px] == 5) {
                    score += 30;
                    freezeSkillTimer = 60;
                    MessageBeep(MB_ICONINFORMATION);
                } else {
                    score += 10;
                }

                if (score > highScore) highScore = score;
                if (score > statsMaxScore) statsMaxScore = score;

                map[py][px] = 0;
                dotCount--;

                if (dotCount == 0) {
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
        }
    }

    // Dot Magnet Active Skill Logic (Attract dots in 4-tile radius)
    if (magnetSkillTimer > 0) {
        magnetSkillTimer--;
        for (int r = py - 3; r <= py + 3; r++) {
            for (int c = px - 3; c <= px + 3; c++) {
                if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                    if (map[r][c] >= 2 && map[r][c] <= 5) {
                        if (map[r][c] == 3) { score += 40; frightTimer = 50; }
                        else if (map[r][c] == 4) { score += 20; speedSkillTimer = 80; }
                        else if (map[r][c] == 5) { score += 30; freezeSkillTimer = 60; }
                        else { score += 10; }
                        map[r][c] = 0;
                        dotCount--;
                        if (score > highScore) highScore = score;
                        if (score > statsMaxScore) statsMaxScore = score;
                    }
                }
            }
        }
        if (fruitActive) {
            score += 500;
            fruitActive = 0;
        }
    }

    // Ghost Collisions
    for (int i = 0; i < numGhosts; i++) {
        if (ghosts[i].isPhantom && ghosts[i].phantomTimer <= 0) continue;

        if (px == ghosts[i].x && py == ghosts[i].y) {
            if (frightTimer > 0) {
                if (ghosts[i].type == 5) { // Ghost King Boss
                    bossHp--;
                    score += 500;
                    ghosts[i].x = 7; ghosts[i].y = 6;
                    MessageBeep(MB_ICONASTERISK);
                    if (bossHp <= 0) {
                        score += 2000;
                        gameOver = 2; // Campaign Win!
                        statsGamesPlayed++;
                        SaveHighScore();
                    }
                } else if (ghosts[i].type == 6) { // Phantom Clone
                    score += 100;
                    ghosts[i].phantomTimer = 0;
                } else {
                    score += 200;
                    statsGhostsEaten++;
                    if (score > highScore) highScore = score;
                    if (score > statsMaxScore) statsMaxScore = score;
                    MessageBeep(MB_ICONASTERISK);
                    ghosts[i].x = 7; ghosts[i].y = 6;
                }
            } else if (shieldActive) {
                // Ghost Shield absorbs hit!
                shieldActive = 0;
                ghosts[i].x = 7; ghosts[i].y = 6;
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
                    ghosts[0].x = 7; ghosts[0].y = 6;
                    ghosts[1].x = 6; ghosts[1].y = 7;
                    ghosts[2].x = 8; ghosts[2].y = 7;
                    ghosts[3].x = 7; ghosts[3].y = 7;
                    ghosts[4].x = 7; ghosts[4].y = 5;
                    if (level == 20) {
                        ghosts[5].x = 7; ghosts[5].y = 6;
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
            score += 500;
            if (score > highScore) highScore = score;
            if (score > statsMaxScore) statsMaxScore = score;
            fruitActive = 0;
            MessageBeep(MB_ICONASTERISK);
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
        case WM_KEYDOWN:
            if (wParam == VK_LEFT || wParam == 'A' || wParam == 'a') { ndx = -1; ndy = 0; }
            if (wParam == VK_RIGHT || wParam == 'D' || wParam == 'd') { ndx = 1; ndy = 0; }
            if (wParam == VK_UP || wParam == 'W' || wParam == 'w') { ndx = 0; ndy = -1; }
            if (wParam == VK_DOWN) { ndx = 0; ndy = 1; }

            // Active Skills hotkeys
            if (wParam == 'F' || wParam == 'f') TriggerFreezeSkill();
            if (wParam == 'S' || wParam == 's') TriggerSpeedSkill();
            if (wParam == 'M' || wParam == 'm') TriggerMagnetSkill();
            if (wParam == 'B' || wParam == 'b') TriggerShieldSkill();

            if (wParam == VK_RETURN && gameOver) Init(0);
            if (wParam == 'P' || wParam == 'p') paused = !paused;
            if (wParam == '1') { diffMode = 0; lstrcpyA(saveMsgText, "DIFF: EASY"); saveMsgTimer = 20; MessageBeep(MB_OK); }
            if (wParam == '2') { diffMode = 1; lstrcpyA(saveMsgText, "DIFF: NORMAL"); saveMsgTimer = 20; MessageBeep(MB_OK); }
            if (wParam == '3') { diffMode = 2; lstrcpyA(saveMsgText, "DIFF: HARD"); saveMsgTimer = 20; MessageBeep(MB_OK); }
            if (wParam == 'V' || wParam == 'v') SaveGame();
            if (wParam == 'L' || wParam == 'l') LoadGame();
            break;
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

            HBRUSH bg = CreateSolidBrush(RGB(5, 8, 20));
            RECT rc = {0, 0, W, H};
            FillRect(memDC, &rc, bg);
            DeleteObject(bg);

            // Draw Maze Map
            HBRUSH wallBr = CreateSolidBrush(RGB(30, 136, 229));
            HBRUSH dotBr = CreateSolidBrush(RGB(255, 200, 150));
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    if (map[r][c] == 1) {
                        RECT wr = {c * TS, r * TS, c * TS + TS, r * TS + TS};
                        FillRect(memDC, &wr, wallBr);
                    } else if (map[r][c] == 2) {
                        RECT dr = {c * TS + 8, r * TS + 8, c * TS + 12, r * TS + 12};
                        FillRect(memDC, &dr, dotBr);
                    } else if (map[r][c] == 3) {
                        RECT dr = {c * TS + 6, r * TS + 6, c * TS + 14, r * TS + 14};
                        FillRect(memDC, &dr, dotBr);
                    } else if (map[r][c] == 4) {
                        HBRUSH spBr = CreateSolidBrush(RGB(0, 255, 255));
                        RECT dr = {c * TS + 7, r * TS + 7, c * TS + 13, r * TS + 13};
                        FillRect(memDC, &dr, spBr);
                        DeleteObject(spBr);
                    } else if (map[r][c] == 5) {
                        HBRUSH frBr = CreateSolidBrush(RGB(255, 255, 255));
                        RECT dr = {c * TS + 6, r * TS + 6, c * TS + 14, r * TS + 14};
                        FillRect(memDC, &dr, frBr);
                        DeleteObject(frBr);
                    }
                }
            }
            DeleteObject(wallBr); DeleteObject(dotBr);

            // Draw Pac-Man with active skill visual effects
            COLORREF pacColor = RGB(255, 235, 59);
            if (shieldActive) pacColor = RGB(0, 229, 255);
            else if (speedSkillTimer > 0) pacColor = RGB(255, 152, 0);

            HBRUSH pacBr = CreateSolidBrush(pacColor);
            RECT pr = {px * TS + 2, py * TS + 2, px * TS + TS - 2, py * TS + TS - 2};
            FillRect(memDC, &pr, pacBr);
            DeleteObject(pacBr);

            // Draw Ghosts
            for (int i = 0; i < numGhosts; i++) {
                if (ghosts[i].isPhantom && ghosts[i].phantomTimer <= 0) continue;

                COLORREF c = ghosts[i].c;
                if (frightTimer > 0) {
                    c = ((frightTimer / 2) % 2 == 0) ? RGB(30,136,229) : RGB(255,255,255);
                }
                HBRUSH gBr = CreateSolidBrush(c);
                RECT gr = {ghosts[i].x * TS + 2, ghosts[i].y * TS + 2, ghosts[i].x * TS + TS - 2, ghosts[i].y * TS + TS - 2};
                FillRect(memDC, &gr, gBr);
                DeleteObject(gBr);
            }

            // Draw Fruit
            if (fruitActive) {
                HBRUSH fBr = CreateSolidBrush(RGB(76, 175, 80));
                RECT fr = {7 * TS + 4, 12 * TS + 4, 7 * TS + TS - 4, 12 * TS + TS - 4};
                FillRect(memDC, &fr, fBr);
                DeleteObject(fBr);
            }

            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            char sstr[128];
            const char* diffNames[] = {"EASY", "NORM", "HARD"};
            wsprintfA(sstr, "Lv:%d/20 Sc:%d HI:%d Lvs:%d", level, score, highScore, lives);
            TextOutA(memDC, 2, 0, sstr, lstrlenA(sstr));

            // Skill HUD Line
            wsprintfA(sstr, "F:%s S:%s M:%s B:%s",
                freezeCooldown > 0 ? "CD" : "OK",
                speedCooldown > 0 ? "CD" : "OK",
                magnetCooldown > 0 ? "CD" : "OK",
                shieldCooldown > 0 ? "CD" : (shieldActive ? "ON" : "OK"));
            SetTextColor(memDC, RGB(255, 235, 59));
            TextOutA(memDC, 2, 10, sstr, lstrlenA(sstr));

            if (level == 20 && bossHp > 0) {
                char bossStr[64];
                wsprintfA(bossStr, "BOSS KING HP: %d/%d", bossHp, bossMaxHp);
                SetTextColor(memDC, RGB(255, 215, 0));
                TextOutA(memDC, W - 130, 0, bossStr, lstrlenA(bossStr));
            }

            if (saveMsgTimer > 0) {
                SetTextColor(memDC, RGB(255, 255, 0));
                TextOutA(memDC, W/2 - 45, H/2 + 30, saveMsgText, lstrlenA(saveMsgText));
            }

            if (gameOver) {
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
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KPacApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KPacApp", "KPac", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W + 16, H + 39, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
