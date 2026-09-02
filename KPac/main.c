#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>

#pragma comment(lib, "msvcrt.lib")

#define W 340
#define H 540
#define COLS 15
#define ROWS 15
#define TS 20

// 20 Unique Campaign Maps (Expanded with Loop 11 Hazard Portals [9] and Phase Gateways [10])
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
    // Stage 4: Cross Tunnel & Void Rifts
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,9,2,2,2,2,2,9,2,2,2,1},
        {1,1,1,2,1,1,1,1,1,1,1,2,1,1,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {0,0,0,0,0,0,2,2,2,0,0,0,0,0,0},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,2,2,2,1,2,2,2,2,2,2,1},
        {1,1,1,2,1,1,1,1,1,1,1,2,1,1,1},
        {1,2,2,2,9,2,2,2,2,2,9,2,2,2,1},
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
    // Stage 8: Concentric Rings & Phase Gateways
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,2,2,2,2,2,2,3,1},
        {1,2,1,1,1,1,1,1,1,1,1,1,1,2,1},
        {1,2,1,2,2,2,10,2,10,2,2,2,1,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,1,2,1,2,2,2,2,2,1,2,1,2,1},
        {1,2,1,2,1,2,1,1,1,2,1,2,1,2,1},
        {1,2,2,2,2,2,1,2,1,2,2,2,2,2,1},
        {1,2,1,2,1,2,1,1,1,2,1,2,1,2,1},
        {1,2,1,2,1,2,2,2,2,2,1,2,1,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,1,2,2,2,10,2,10,2,2,2,1,2,1},
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
    // Stage 12: Dual Warp Arena & Void Rifts
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,2,2,9,1,2,2,2,1,9,2,2,2,1},
        {1,1,1,1,2,1,1,1,1,1,2,1,1,1,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,2,1,1,1,2,1,1,1,2,1,1,1,2,1},
        {1,2,1,0,1,2,1,0,1,2,1,0,1,2,1},
        {1,2,1,1,1,2,1,1,1,2,1,1,1,2,1},
        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
        {1,1,1,1,2,1,1,1,1,1,2,1,1,1,1},
        {1,2,2,2,9,1,2,2,2,1,9,2,2,2,1},
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
        {1,3,2,2,2,1,2,2,2,1,2,2,2,3,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,2,1,2,2,2,2,1,2,2,2,2,1,2,1},
        {1,2,2,2,1,1,2,1,2,1,1,2,2,2,1},
        {1,1,1,2,1,2,2,2,2,2,1,2,1,1,1},
        {1,2,2,2,2,2,1,0,1,2,2,2,2,2,1},
        {0,2,1,1,1,2,0,0,0,2,1,1,1,2,0},
        {1,2,2,2,2,2,1,0,1,2,2,2,2,2,1},
        {1,1,1,2,1,2,2,2,2,2,1,2,1,1,1},
        {1,2,2,2,1,1,2,1,2,1,1,2,2,2,1},
        {1,2,1,2,2,2,2,1,2,2,2,2,1,2,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,4,2,2,2,1,2,2,2,1,2,2,2,4,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 15: Cyber Fortress
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,1,0,0,1,2,1,2,1,0,0,1,2,1},
        {1,2,1,0,0,1,2,2,2,1,0,0,1,2,1},
        {1,2,1,1,1,1,1,0,1,1,1,1,1,2,1},
        {1,2,2,2,2,2,2,0,2,2,2,2,2,2,1},
        {0,0,0,1,1,1,2,0,2,1,1,1,0,0,0},
        {1,2,2,2,2,2,2,0,2,2,2,2,2,2,1},
        {1,2,1,1,1,1,1,0,1,1,1,1,1,2,1},
        {1,2,1,0,0,1,2,2,2,1,0,0,1,2,1},
        {1,2,1,0,0,1,2,1,2,1,0,0,1,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,4,2,2,2,2,2,1,2,2,2,2,2,4,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 16: Hazard Matrix & Void Rifts
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,6,2,2,2,1,2,2,2,6,2,3,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,1,2,2,2,2,2,2,2,2,2,1,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,2,2,2,2,9,0,9,2,2,2,2,2,1},
        {1,1,1,1,2,1,1,0,1,1,2,1,1,1,1},
        {0,2,2,2,2,1,0,0,0,1,2,2,2,2,0},
        {1,1,1,1,2,1,1,0,1,1,2,1,1,1,1},
        {1,2,2,2,2,2,9,0,9,2,2,2,2,2,1},
        {1,2,1,2,1,1,1,1,1,1,1,2,1,2,1},
        {1,2,1,2,2,2,2,2,2,2,2,2,1,2,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,4,2,6,2,2,2,1,2,2,2,6,2,4,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Stage 17: Neon Citadel
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,1,2,2,2,1,2,2,2,3,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,2,1,2,2,2,2,1,2,2,2,2,1,2,1},
        {1,2,2,2,1,1,2,2,2,1,1,2,2,2,1},
        {1,1,1,2,1,0,0,0,0,0,1,2,1,1,1},
        {0,0,1,2,1,0,0,0,0,0,1,2,1,0,0},
        {1,1,1,2,2,0,0,0,0,0,2,2,1,1,1},
        {0,0,1,2,1,0,0,0,0,0,1,2,1,0,0},
        {1,1,1,2,1,0,0,0,0,0,1,2,1,1,1},
        {1,2,2,2,1,1,2,2,2,1,1,2,2,2,1},
        {1,2,1,2,2,2,2,1,2,2,2,2,1,2,1},
        {1,2,1,1,2,1,2,1,2,1,2,1,1,2,1},
        {1,4,2,2,2,1,2,2,2,1,2,2,2,4,1},
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
    // Stage 20: Ghost King Lair (Boss Chamber with Void Rifts)
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,3,2,2,2,2,2,1,2,2,2,2,2,3,1},
        {1,2,1,1,1,1,2,1,2,1,1,1,1,2,1},
        {1,2,1,5,2,9,2,2,2,9,2,5,1,2,1},
        {1,2,1,2,1,1,1,0,1,1,1,2,1,2,1},
        {1,2,2,2,1,0,0,0,0,0,1,2,2,2,1},
        {1,1,1,2,1,0,0,0,0,0,1,2,1,1,1},
        {0,0,0,2,0,0,0,0,0,0,0,2,0,0,0},
        {1,1,1,2,1,0,0,0,0,0,1,2,1,1,1},
        {1,2,2,2,1,0,0,0,0,0,1,2,2,2,1},
        {1,2,1,2,1,1,1,0,1,1,1,2,1,2,1},
        {1,2,1,4,2,9,2,2,2,9,2,4,1,2,1},
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

// Game Modes & Progression
int gameMode = 0; // 0 = Campaign, 1 = Arcade Endless
int endlessWave = 1;
int endlessHighWave = 1;

// Crafting Materials (The Cyber Forge)
int craftEctoplasm = 0;
int craftFruitEssence = 0;
int craftStarDust = 0;
int showCraftMenu = 0;
int craftMenuPage = 0; // 0 = Consumables, 1 = Legendary Relics
int shieldHits = 0;

// Loop 11: Legendary Relics
int relicCrown = 0;      // Recipe 5: Crown of Ghost King (+50% Fright timer, 2x ghost eaten score, essence drop)
int relicHourglass = 0;  // Recipe 6: Chrono-Phase Hourglass (Ghosts 25% slower, turn slow-mo)
int relicOrb = 0;        // Recipe 7: Astral Devourer Orb (Dots = 25 pts, 2-tile vacuum aura)
int relicAegis = 0;      // Recipe 8: Sun Titan Aegis (Regen +1 shield every 25s, sludge immune)
int relicStone = 0;      // Recipe 9: Philosopher's Pac-Stone (Mythic Ambrosia fruit +2500, +1 Max Life, 2x Global Mult)
int relicAegisTimer = 0;

// Loop 11: Ghost Companion Pet System ("Ghost Whisperer")
int petActive = 1;
int petType = 0;       // 0=Blinky Jr (Pyre Wisp), 1=Inky Spark (Cyan Orbit), 2=Pinky Heart (Rose Guardian), 3=Gold Kinglet (Aurum Pixie)
int petLevel = 1;      // 1..5
int petExp = 0;
int petX = 7, petY = 12;
double petFloatAngle = 0;
int petCooldown = 0;
int petBurstTimer = 0;
int petStunTarget = -1;

// Player state
int px = 7, py = 12;
int pdx = 0, pdy = 0;
int ndx = 0, ndy = 0;
int playerSlowTimer = 0;

// Ghost struct
typedef struct {
    int x;
    int y;
    COLORREF c;
    int type; // 0=Blinky, 1=Pinky, 2=Inky, 3=Clyde, 4=Sue, 5=GhostKing, 6=Phantom
    int isPhantom;
    int phantomTimer;
    int isDead;
    int dirX;
    int dirY;
    int trait; // 0=None, 1=Vortex Magnet, 2=Glitch Shifter, 3=Trapper, 4=Mirage, 5=Hyper Chaser
    int traitTimer;
    int stunTimer;
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
    int r1;
    int r2;
    int maxR;
    COLORREF color;
    int is3D;
} ShockwaveRing;

ShockwaveRing shockwaves[16];
int numShockwaves = 0;

void AddShockwave(int x, int y, COLORREF color) {
    if (numShockwaves < 16) {
        shockwaves[numShockwaves++] = (ShockwaveRing){x, y, 2, 1, 32, color, 0};
    }
}

void AddShockwave3D(int x, int y, COLORREF color) {
    if (numShockwaves < 16) {
        shockwaves[numShockwaves++] = (ShockwaveRing){x, y, 2, 1, 32, color, 1};
    }
}

typedef struct {
    double x, y, vx, vy;
    int life, maxLife;
    COLORREF color;
    int type;      // 0=Needle spark, 1=Smoke puff, 2=Heavy shard, 3=Energy star
    int size;
    double rot, vRot;
} Particle;
#define MAX_PARTICLES 384
Particle particles[MAX_PARTICLES];
int numParticles = 0;

void AddSparks(int cx, int cy, COLORREF color, int count) {
    for (int i = 0; i < count && numParticles < MAX_PARTICLES; i++) {
        double ang = (MyRand() % 360) * 3.14159 / 180.0;
        double spd = 2.0 + (MyRand() % 40) / 10.0;
        particles[numParticles].x = cx;
        particles[numParticles].y = cy;
        particles[numParticles].vx = MyCos(ang) * spd;
        particles[numParticles].vy = MySin(ang) * spd;
        particles[numParticles].life = 20 + (MyRand() % 8);
        particles[numParticles].maxLife = particles[numParticles].life;
        particles[numParticles].color = color;
        particles[numParticles].type = 0;
        particles[numParticles].size = 2;
        particles[numParticles].rot = 0;
        particles[numParticles].vRot = 0;
        numParticles++;
    }
}

void AddExplosion(int cx, int cy, COLORREF color) {
    AddShockwave3D(cx, cy, color);
    AddShockwave(cx, cy, RGB(255, 255, 255));
    // Layer 0: Incandescent needle core sparks
    AddSparks(cx, cy, color, 14);
    AddSparks(cx, cy, RGB(255, 255, 255), 8);
    // Layer 1: Expanding buoyant smoke puffs with negative gravity
    for (int i = 0; i < 10 && numParticles < MAX_PARTICLES; i++) {
        double ang = (MyRand() % 360) * 3.14159 / 180.0;
        double spd = 0.5 + (MyRand() % 15) / 10.0;
        particles[numParticles].x = cx;
        particles[numParticles].y = cy;
        particles[numParticles].vx = MyCos(ang) * spd;
        particles[numParticles].vy = MySin(ang) * spd - 0.4;
        particles[numParticles].life = 22 + (MyRand() % 8);
        particles[numParticles].maxLife = particles[numParticles].life;
        particles[numParticles].color = color;
        particles[numParticles].type = 1;
        particles[numParticles].size = 3 + (MyRand() % 3);
        particles[numParticles].rot = 0;
        particles[numParticles].vRot = 0;
        numParticles++;
    }
    // Layer 2: Heavy kinematic cyber debris shards with tumbling rotation
    for (int i = 0; i < 12 && numParticles < MAX_PARTICLES; i++) {
        double ang = (MyRand() % 360) * 3.14159 / 180.0;
        double spd = 1.0 + (MyRand() % 30) / 10.0;
        particles[numParticles].x = cx;
        particles[numParticles].y = cy;
        particles[numParticles].vx = MyCos(ang) * spd;
        particles[numParticles].vy = MySin(ang) * spd - 1.0;
        particles[numParticles].life = 24 + (MyRand() % 8);
        particles[numParticles].maxLife = particles[numParticles].life;
        particles[numParticles].color = RGB(0, 229, 255);
        particles[numParticles].type = 2;
        particles[numParticles].size = 3 + (MyRand() % 2);
        particles[numParticles].rot = (MyRand() % 360) * 3.14159 / 180.0;
        particles[numParticles].vRot = ((MyRand() % 40) - 20) / 100.0;
        numParticles++;
    }
    // Layer 3: Radiant golden/cyan celebration energy stars
    for (int i = 0; i < 8 && numParticles < MAX_PARTICLES; i++) {
        double ang = (MyRand() % 360) * 3.14159 / 180.0;
        double spd = 1.0 + (MyRand() % 25) / 10.0;
        particles[numParticles].x = cx;
        particles[numParticles].y = cy;
        particles[numParticles].vx = MyCos(ang) * spd;
        particles[numParticles].vy = MySin(ang) * spd;
        particles[numParticles].life = 26 + (MyRand() % 8);
        particles[numParticles].maxLife = particles[numParticles].life;
        particles[numParticles].color = (i % 2 == 0) ? RGB(255, 215, 0) : RGB(0, 229, 255);
        particles[numParticles].type = 3;
        particles[numParticles].size = 4 + (MyRand() % 2);
        particles[numParticles].rot = 0;
        particles[numParticles].vRot = 0.15;
        numParticles++;
    }
    screenShake = 16;
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
int statsRelicsForged = 0;

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
    int relicCrown, relicHourglass, relicOrb, relicAegis, relicStone;
    int petActive, petType, petLevel, petExp;
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
    st.relicCrown = relicCrown; st.relicHourglass = relicHourglass; st.relicOrb = relicOrb;
    st.relicAegis = relicAegis; st.relicStone = relicStone;
    st.petActive = petActive; st.petType = petType; st.petLevel = petLevel; st.petExp = petExp;

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
        freezeCooldown = st.freezeCooldown; speedCooldown = speedCooldown;
        magnetCooldown = st.magnetCooldown; shieldCooldown = shieldCooldown;
        bossHp = st.bossHp;
        dotCount = st.dotCount; frameCount = st.frameCount;
        fruitActive = st.fruitActive; fruitTimer = st.fruitTimer; gameOver = st.gameOver;
        vipX = st.vipX; vipY = st.vipY; vipActive = st.vipActive;
        gameMode = st.gameMode; endlessWave = st.endlessWave; endlessHighWave = st.endlessHighWave;
        craftEctoplasm = st.craftEctoplasm; craftFruitEssence = st.craftFruitEssence; craftStarDust = st.craftStarDust;
        relicCrown = st.relicCrown; relicHourglass = st.relicHourglass; relicOrb = st.relicOrb;
        relicAegis = st.relicAegis; relicStone = st.relicStone;
        petActive = st.petActive; petType = st.petType; petLevel = st.petLevel; petExp = st.petExp;
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
        ReadFile(hFile, &relicCrown, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &relicHourglass, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &relicOrb, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &relicAegis, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &relicStone, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &petType, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &petLevel, sizeof(int), &readBytes, NULL);
        CloseHandle(hFile);
    }
    hFile = CreateFileA("kpac_stats.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes;
        ReadFile(hFile, &statsGamesPlayed, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &statsGhostsEaten, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &statsMaxScore, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &statsRelicsForged, sizeof(int), &readBytes, NULL);
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
        WriteFile(hFile, &relicCrown, sizeof(int), &written, NULL);
        WriteFile(hFile, &relicHourglass, sizeof(int), &written, NULL);
        WriteFile(hFile, &relicOrb, sizeof(int), &written, NULL);
        WriteFile(hFile, &relicAegis, sizeof(int), &written, NULL);
        WriteFile(hFile, &relicStone, sizeof(int), &written, NULL);
        WriteFile(hFile, &petType, sizeof(int), &written, NULL);
        WriteFile(hFile, &petLevel, sizeof(int), &written, NULL);
        CloseHandle(hFile);
    }
    hFile = CreateFileA("kpac_stats.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, &statsGamesPlayed, sizeof(int), &written, NULL);
        WriteFile(hFile, &statsGhostsEaten, sizeof(int), &written, NULL);
        WriteFile(hFile, &statsMaxScore, sizeof(int), &written, NULL);
        WriteFile(hFile, &statsRelicsForged, sizeof(int), &written, NULL);
        CloseHandle(hFile);
    }
}

int GetInitLives() {
    int base = 3;
    if (diffMode == 0) base = 5;
    if (diffMode == 2) base = 2;
    if (relicStone) base += 1; // Philosopher's Pac-Stone +1 Max Life
    return base;
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
    map[6][6] = 1; map[6][7] = 1; map[6][8] = 1; // Door at (6,7)
    
    // Warp tunnels
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

    // Hazards and Portals for higher waves
    if (wave >= 2) {
        if (map[3][7] == 2) map[3][7] = 9;  // Void Rift A
        if (map[11][7] == 2) map[11][7] = 9; // Void Rift B
    }
    if (wave >= 4) {
        if (map[5][3] == 2) map[5][3] = 10;
        if (map[5][COLS-4] == 2) map[5][COLS-4] = 10;
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
    petBurstTimer = 0;
    petCooldown = 0;
    petStunTarget = -1;

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
    petX = px; petY = py;

    // Roster of AI Ghosts with Procedural Personalities & Traits
    int effWave = (gameMode == 1) ? endlessWave : level;
    int t0 = (effWave >= 2) ? ((effWave % 5) + 1) : 0;
    int t1 = (effWave >= 3) ? (((effWave + 2) % 5) + 1) : 0;
    int t2 = (effWave >= 4) ? (((effWave + 3) % 5) + 1) : 0;
    int t3 = (effWave >= 5) ? (((effWave + 4) % 5) + 1) : 0;
    int t4 = (effWave >= 6) ? (((effWave + 1) % 5) + 1) : 0;

    ghosts[0] = (Ghost){7, 6, RGB(255, 23, 68), 0, 0, 0, 0, 0, -1, t0, 0, 0, 0, 0};
    ghosts[1] = (Ghost){6, 7, RGB(240, 98, 146), 1, 0, 0, 0, -1, 0, t1, 0, 0, 0, 0};
    ghosts[2] = (Ghost){8, 7, RGB(0, 229, 255), 2, 0, 0, 0, 1, 0, t2, 0, 0, 0, 0};
    ghosts[3] = (Ghost){7, 7, RGB(255, 145, 0), 3, 0, 0, 0, 0, 1, t3, 0, 0, 0, 0};
    ghosts[4] = (Ghost){7, 5, RGB(170, 0, 255), 4, 0, 0, 0, 0, -1, t4, 0, 0, 0, 0};

    if (gameMode == 0 && level == 20) {
        // Stage 20 Ghost King Boss
        ghosts[5] = (Ghost){7, 6, RGB(255, 215, 0), 5, 0, 0, 0, 0, -1, 5, 0, 0, 0, 0};
        ghosts[6] = (Ghost){0, 0, RGB(0,0,0), 6, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        ghosts[7] = (Ghost){0, 0, RGB(0,0,0), 6, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
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
    craftMenuPage = 0;
}

// Crafting System Logic (Consumables & Legendary Relics)
void CraftItem(int recipe) {
    if (recipe == 1) { // Super Pellet: 2 Ectoplasm + 1 Star Dust
        if (craftEctoplasm >= 2 && craftStarDust >= 1) {
            craftEctoplasm -= 2;
            craftStarDust -= 1;
            frightTimer = relicCrown ? 120 : 80;
            score += 500;
            lstrcpyA(saveMsgText, "CRAFTED: SUPER PELLET!");
            saveMsgTimer = 25;
            AddExplosion(px * TS + TS/2, py * TS + TS/2, RGB(255, 215, 0));
            MessageBeep(MB_ICONASTERISK);
        } else {
            lstrcpyA(saveMsgText, "NEED: 2 ECTO + 1 DUST");
            saveMsgTimer = 25; MessageBeep(MB_ICONHAND);
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
            saveMsgTimer = 25; MessageBeep(MB_ICONHAND);
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
            saveMsgTimer = 25; MessageBeep(MB_ICONHAND);
        }
    } else if (recipe == 4) { // Void Pulse Bomb: 3 Ectoplasm + 3 Star Dust + 1 Fruit Essence
        if (craftEctoplasm >= 3 && craftStarDust >= 3 && craftFruitEssence >= 1) {
            craftEctoplasm -= 3;
            craftStarDust -= 3;
            craftFruitEssence -= 1;
            for (int i = 0; i < numGhosts; i++) {
                if (!ghosts[i].isPhantom) {
                    ghosts[i].x = 7; ghosts[i].y = 6; ghosts[i].isDead = 0;
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
            saveMsgTimer = 25; MessageBeep(MB_ICONHAND);
        }
    } else if (recipe == 5) { // Relic: Crown of the Ghost King (4 Ecto + 3 Dust)
        if (relicCrown) {
            lstrcpyA(saveMsgText, "RELIC ALREADY FORGED!");
            saveMsgTimer = 25;
        } else if (craftEctoplasm >= 4 && craftStarDust >= 3) {
            craftEctoplasm -= 4;
            craftStarDust -= 3;
            relicCrown = 1;
            statsRelicsForged++;
            SaveHighScore();
            lstrcpyA(saveMsgText, "FORGED: GHOST KING CROWN!");
            saveMsgTimer = 30;
            AddExplosion(px * TS + TS/2, py * TS + TS/2, RGB(255, 215, 0));
            MessageBeep(MB_ICONASTERISK);
        } else {
            lstrcpyA(saveMsgText, "NEED: 4 ECTO + 3 DUST");
            saveMsgTimer = 25; MessageBeep(MB_ICONHAND);
        }
    } else if (recipe == 6) { // Relic: Chrono-Phase Hourglass (4 Essence + 3 Dust)
        if (relicHourglass) {
            lstrcpyA(saveMsgText, "RELIC ALREADY FORGED!");
            saveMsgTimer = 25;
        } else if (craftFruitEssence >= 4 && craftStarDust >= 3) {
            craftFruitEssence -= 4;
            craftStarDust -= 3;
            relicHourglass = 1;
            statsRelicsForged++;
            SaveHighScore();
            lstrcpyA(saveMsgText, "FORGED: CHRONO HOURGLASS!");
            saveMsgTimer = 30;
            AddShockwave3D(px * TS + TS/2, py * TS + TS/2, RGB(0, 229, 255));
            MessageBeep(MB_ICONASTERISK);
        } else {
            lstrcpyA(saveMsgText, "NEED: 4 ESSENCE + 3 DUST");
            saveMsgTimer = 25; MessageBeep(MB_ICONHAND);
        }
    } else if (recipe == 7) { // Relic: Astral Devourer Orb (4 Ecto + 4 Dust)
        if (relicOrb) {
            lstrcpyA(saveMsgText, "RELIC ALREADY FORGED!");
            saveMsgTimer = 25;
        } else if (craftEctoplasm >= 4 && craftStarDust >= 4) {
            craftEctoplasm -= 4;
            craftStarDust -= 4;
            relicOrb = 1;
            statsRelicsForged++;
            SaveHighScore();
            lstrcpyA(saveMsgText, "FORGED: ASTRAL ORB!");
            saveMsgTimer = 30;
            AddExplosion(px * TS + TS/2, py * TS + TS/2, RGB(186, 104, 200));
            MessageBeep(MB_ICONASTERISK);
        } else {
            lstrcpyA(saveMsgText, "NEED: 4 ECTO + 4 DUST");
            saveMsgTimer = 25; MessageBeep(MB_ICONHAND);
        }
    } else if (recipe == 8) { // Relic: Sun Titan Aegis (4 Essence + 4 Dust)
        if (relicAegis) {
            lstrcpyA(saveMsgText, "RELIC ALREADY FORGED!");
            saveMsgTimer = 25;
        } else if (craftFruitEssence >= 4 && craftStarDust >= 4) {
            craftFruitEssence -= 4;
            craftStarDust -= 4;
            relicAegis = 1;
            shieldActive = 1;
            shieldHits = 3;
            statsRelicsForged++;
            SaveHighScore();
            lstrcpyA(saveMsgText, "FORGED: SUN TITAN AEGIS!");
            saveMsgTimer = 30;
            AddShockwave3D(px * TS + TS/2, py * TS + TS/2, RGB(255, 152, 0));
            MessageBeep(MB_ICONASTERISK);
        } else {
            lstrcpyA(saveMsgText, "NEED: 4 ESSENCE + 4 DUST");
            saveMsgTimer = 25; MessageBeep(MB_ICONHAND);
        }
    } else if (recipe == 9) { // Relic: Philosopher's Pac-Stone (5 Ecto + 5 Ess + 5 Dust)
        if (relicStone) {
            lstrcpyA(saveMsgText, "RELIC ALREADY FORGED!");
            saveMsgTimer = 25;
        } else if (craftEctoplasm >= 5 && craftFruitEssence >= 5 && craftStarDust >= 5) {
            craftEctoplasm -= 5;
            craftFruitEssence -= 5;
            craftStarDust -= 5;
            relicStone = 1;
            lives += 1;
            statsRelicsForged++;
            SaveHighScore();
            lstrcpyA(saveMsgText, "FORGED: PHILOSOPHER STONE!");
            saveMsgTimer = 30;
            AddExplosion(px * TS + TS/2, py * TS + TS/2, RGB(255, 215, 0));
            MessageBeep(MB_ICONASTERISK);
        } else {
            lstrcpyA(saveMsgText, "NEED: 5 ECTO + 5 ESS + 5 DUST");
            saveMsgTimer = 25; MessageBeep(MB_ICONHAND);
        }
    }
}

// Active Skill Trigger Functions
void TriggerFreezeSkill() {
    if (freezeCooldown == 0 && !gameOver && !paused) {
        freezeSkillTimer = 60; // 6s freeze
        freezeCooldown = 150;  // 15s cooldown
        lstrcpyA(saveMsgText, "FREEZE SKILL!");
        saveMsgTimer = 20;
        AddSparks(px * TS + TS/2, py * TS + TS/2, RGB(255, 255, 255), 15);
        MessageBeep(MB_ICONINFORMATION);
    }
}

void TriggerSpeedSkill() {
    if (speedCooldown == 0 && !gameOver && !paused) {
        speedSkillTimer = 80; // 8s sprint
        speedCooldown = 150;  // 15s cooldown
        lstrcpyA(saveMsgText, "SPEED SPRINT!");
        saveMsgTimer = 20;
        AddSparks(px * TS + TS/2, py * TS + TS/2, RGB(0, 255, 255), 15);
        MessageBeep(MB_ICONEXCLAMATION);
    }
}

void TriggerMagnetSkill() {
    if (magnetCooldown == 0 && !gameOver && !paused) {
        magnetSkillTimer = 50; // 5s magnet
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
        shieldHits = (shieldHits < 1) ? 1 : (shieldHits + 1);
        shieldCooldown = 200; // 20s cooldown
        lstrcpyA(saveMsgText, "GHOST SHIELD!");
        saveMsgTimer = 20;
        AddSparks(px * TS + TS/2, py * TS + TS/2, RGB(0, 229, 255), 15);
        MessageBeep(MB_OK);
    }
}

// Loop 11: Companion Pet Command / Ultimate Ability Trigger
void TriggerPetUltimate() {
    if (petCooldown == 0 && !gameOver && !paused && petActive) {
        petCooldown = 180; // 18s cooldown
        petBurstTimer = 40;
        if (petType == 0) { // Blinky Jr: Firestorm stun on all ghosts + 1 boss damage
            for (int i = 0; i < numGhosts; i++) {
                ghosts[i].stunTimer = 60;
                AddSparks(ghosts[i].x * TS + TS/2, ghosts[i].y * TS + TS/2, RGB(255, 23, 68), 8);
            }
            if (gameMode == 0 && level == 20 && bossHp > 0) {
                bossHp--;
                if (bossHp <= 0) { victoryTimer = 30; }
            }
            lstrcpyA(saveMsgText, "PET: PYRE FIRESTORM!");
            AddExplosion(petX * TS + TS/2, petY * TS + TS/2, RGB(255, 23, 68));
        } else if (petType == 1) { // Inky Spark: Magnet EMP storm vacuuming dots in 5-tile radius
            for (int r = py - 5; r <= py + 5; r++) {
                for (int c = px - 5; c <= px + 5; c++) {
                    if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                        if (map[r][c] >= 2 && map[r][c] <= 5) {
                            score += 15;
                            map[r][c] = 0;
                            dotCount--;
                        }
                    }
                }
            }
            freezeSkillTimer = 50;
            lstrcpyA(saveMsgText, "PET: EMP VACUUM STORM!");
            AddShockwave3D(petX * TS + TS/2, petY * TS + TS/2, RGB(0, 229, 255));
        } else if (petType == 2) { // Pinky Heart: Divine Aegis
            shieldActive = 1;
            shieldHits += 2;
            frightTimer = relicCrown ? 90 : 60;
            lstrcpyA(saveMsgText, "PET: DIVINE AEGIS + FRIGHT!");
            AddShockwave(petX * TS + TS/2, petY * TS + TS/2, RGB(240, 98, 146));
        } else if (petType == 3) { // Gold Kinglet: Midas Golden Rain
            score += 1000;
            craftStarDust += 1;
            craftFruitEssence += 1;
            craftEctoplasm += 1;
            lstrcpyA(saveMsgText, "PET: MIDAS GOLDEN RAIN!");
            AddExplosion(petX * TS + TS/2, petY * TS + TS/2, RGB(255, 215, 0));
        }
        saveMsgTimer = 25;
        MessageBeep(MB_ICONASTERISK);
    }
}

void CyclePetCompanion() {
    petType = (petType + 1) % 4;
    char names[4][20] = {"Blinky Jr (Pyre)", "Inky (Cyan Spark)", "Pinky (Guardian)", "Kinglet (Gold)"};
    wsprintfA(saveMsgText, "PET: %s", names[petType]);
    saveMsgTimer = 25;
    AddSparks(px * TS + TS/2, py * TS + TS/2, RGB(255, 215, 0), 10);
    SaveHighScore();
    MessageBeep(MB_OK);
}

void Update() {
    screenShake = (int)(screenShake * 0.88);
    if (saveMsgTimer > 0) saveMsgTimer--;
    if (playerSlowTimer > 0 && !relicAegis) playerSlowTimer--;
    if (showHelp || gameOver || paused) return;

    // Loop 11: Sun Titan Aegis passive shield regeneration
    if (relicAegis) {
        relicAegisTimer++;
        if (relicAegisTimer >= 250) { // every 25 seconds
            relicAegisTimer = 0;
            if (shieldHits < 3) {
                shieldActive = 1;
                shieldHits++;
                AddShockwave(px * TS + TS/2, py * TS + TS/2, RGB(255, 152, 0));
                lstrcpyA(saveMsgText, "AEGIS: SHIELD RECHARGED!");
                saveMsgTimer = 20;
            }
        }
    }

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
                petX = px; petY = py;
                int effWave = (gameMode == 1) ? endlessWave : level;
                ghosts[0] = (Ghost){7, 6, RGB(255, 23, 68), 0, 0, 0, 0, 0, -1, (effWave >= 2) ? ((effWave % 5) + 1) : 0, 0, 0, 0, 0};
                ghosts[1] = (Ghost){6, 7, RGB(240, 98, 146), 1, 0, 0, 0, -1, 0, (effWave >= 3) ? (((effWave + 2) % 5) + 1) : 0, 0, 0, 0, 0};
                ghosts[2] = (Ghost){8, 7, RGB(0, 229, 255), 2, 0, 0, 0, 1, 0, (effWave >= 4) ? (((effWave + 3) % 5) + 1) : 0, 0, 0, 0, 0};
                ghosts[3] = (Ghost){7, 7, RGB(255, 145, 0), 3, 0, 0, 0, 0, 1, (effWave >= 5) ? (((effWave + 4) % 5) + 1) : 0, 0, 0, 0, 0};
                ghosts[4] = (Ghost){7, 5, RGB(170, 0, 255), 4, 0, 0, 0, 0, -1, (effWave >= 6) ? (((effWave + 1) % 5) + 1) : 0, 0, 0, 0, 0};
                if (gameMode == 0 && level == 20) {
                    ghosts[5] = (Ghost){7, 6, RGB(255, 215, 0), 5, 0, 0, 0, 0, -1, 5, 0, 0, 0, 0};
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
                int mult = relicStone ? 2 : 1;
                score += (1000 + endlessWave * 200) * mult;
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
    if (petCooldown > 0) petCooldown--;
    if (petBurstTimer > 0) petBurstTimer--;

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

    // Update Ghost Stun Timers
    for (int i = 0; i < numGhosts; i++) {
        if (ghosts[i].stunTimer > 0) ghosts[i].stunTimer--;
    }

    // Stage 20 Ghost King Phantom Clones Spawner
    if (gameMode == 0 && level == 20 && bossHp > 0) {
        phantomSpawnTimer += (bossHp <= bossMaxHp / 2) ? 2 : 1;
        if (phantomSpawnTimer >= 50) {
            phantomSpawnTimer = 0;
            for (int k = 6; k <= 7; k++) {
                if (ghosts[k].phantomTimer <= 0) {
                    ghosts[k] = (Ghost){7, 6, RGB(200, 100, 255), 6, 1, 80, 0, 0, 0, 0, 0, 0, 0, 0};
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

    // Ghost Speed Logic (Chrono Hourglass makes ghosts 25% slower)
    int effLevel = (gameMode == 1) ? endlessWave : level;
    int ghostSpeed = 4 - (effLevel / 4);
    if (diffMode == 0) ghostSpeed += 1;
    else if (diffMode == 2) ghostSpeed = (ghostSpeed > 1) ? (ghostSpeed - 1) : 1;
    if (relicHourglass) ghostSpeed += 1; // 25% slower
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
            if (ghosts[i].stunTimer > 0) continue;
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
                        tx = px; ty = py;
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
    if (playerSlowTimer > 0 && !relicAegis) moveTick = 3;

    int playerMoves = (moveTick == 1) ? 1 : (frameCount % moveTick == 0);

    if (playerMoves) {
        int nx = px + pdx;
        int ny = py + pdy;
        if (nx < 0) nx = COLS - 1;
        if (nx >= COLS) nx = 0;

        if (ny >= 0 && ny < ROWS && map[ny][nx] != 1) {
            px = nx;
            py = ny;

            // Check Sludge Trap collision (immune if Sun Titan Aegis equipped)
            if (!relicAegis) {
                for (int s = 0; s < numSludgeTraps; s++) {
                    if (sludgeTraps[s].x == px && sludgeTraps[s].y == py) {
                        if (!shieldActive) {
                            playerSlowTimer = 30;
                            AddSparks(px * TS + TS/2, py * TS + TS/2, RGB(0, 255, 100), 5);
                        }
                    }
                }
            }

            // Loop 11: Hazard Portal / Void Rift [Tile 9] Interaction
            if (map[py][px] == 9) {
                // Find matching/mirror portal or opposite quadrant
                int targetX = COLS - 1 - px;
                int targetY = ROWS - 1 - py;
                if (targetX >= 0 && targetX < COLS && targetY >= 0 && targetY < ROWS && map[targetY][targetX] != 1) {
                    px = targetX;
                    py = targetY;
                }
                AddExplosion(px * TS + TS/2, py * TS + TS/2, RGB(186, 104, 200));
                if (shieldActive || frightTimer > 0) {
                    score += 300;
                    craftStarDust += 1;
                    lstrcpyA(saveMsgText, "VOID RIFT STABILIZED! +300");
                    saveMsgTimer = 25;
                } else {
                    lstrcpyA(saveMsgText, "DIMENSIONAL WARP!");
                    saveMsgTimer = 20;
                }
                MessageBeep(MB_ICONASTERISK);
            }

            if (map[py][px] == 6) { // Lava / Hazard
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
                if (relicStone) mult *= 2; // Philosopher's Pac-Stone 2x Global Mult

                // Pet Leveling progression
                petExp++;
                if (petExp >= 25 * petLevel && petLevel < 5) {
                    petLevel++;
                    petExp = 0;
                    lstrcpyA(saveMsgText, "PET LEVEL UP!");
                    saveMsgTimer = 25;
                    AddExplosion(petX * TS + TS/2, petY * TS + TS/2, RGB(255, 215, 0));
                    SaveHighScore();
                }

                if (map[py][px] == 3) {
                    score += 40 * mult;
                    craftStarDust += 1;
                    int baseFright = (diffMode == 0) ? 75 : ((diffMode == 2) ? 35 : 50);
                    if (relicCrown) baseFright = (int)(baseFright * 1.5);
                    frightTimer = (loopNum >= 7) ? 0 : baseFright;
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
                    int dotPts = relicOrb ? 25 : 10; // Astral Devourer Orb gives +25
                    if (petType == 3) dotPts *= 3;    // Gold Kinglet triples dot score
                    score += dotPts * mult;
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

    // Loop 11: Companion Pet AI Updates (Patrol, Assist, Vacuum, Stun)
    if (petActive) {
        petFloatAngle += 0.15;
        // Smoothly follow Pac-Man trail
        if (frameCount % 2 == 0) {
            if (petX < px) petX++;
            else if (petX > px) petX--;
            if (petY < py) petY++;
            else if (petY > py) petY--;
        }

        // Pet Type 0 (Blinky Jr): Attacks closest hostile ghost
        if (petType == 0 && frameCount % 15 == 0) {
            for (int i = 0; i < numGhosts; i++) {
                if (!ghosts[i].isDead && ghosts[i].stunTimer == 0) {
                    int dist = Abs(ghosts[i].x - petX) + Abs(ghosts[i].y - petY);
                    if (dist <= 2 + petLevel) {
                        ghosts[i].stunTimer = 30 + petLevel * 5;
                        AddSparks(ghosts[i].x * TS + TS/2, ghosts[i].y * TS + TS/2, RGB(255, 23, 68), 5);
                        break;
                    }
                }
            }
        }

        // Pet Type 1 (Inky Spark) or Astral Orb: Vacuum dots in radius
        int vacRadius = (relicOrb ? 2 : 0) + (petType == 1 ? (1 + petLevel / 2) : 0);
        if (vacRadius > 0 && frameCount % 3 == 0) {
            for (int r = petY - vacRadius; r <= petY + vacRadius; r++) {
                for (int c = petX - vacRadius; c <= petX + vacRadius; c++) {
                    if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                        if (map[r][c] == 2) {
                            int dotPts = relicOrb ? 25 : 10;
                            if (petType == 3) dotPts *= 3;
                            int mult = relicStone ? 2 : 1;
                            score += dotPts * mult;
                            map[r][c] = 0;
                            dotCount--;
                            if (dotCount == 0) { victoryTimer = 30; MessageBeep(MB_ICONASTERISK); }
                        }
                    }
                }
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
                        if (relicStone) mult *= 2;
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
            int fruitPts = relicStone ? 2500 : 500;
            score += fruitPts * mult;
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
                if (relicStone) mult *= 2;
                craftEctoplasm += (ghosts[i].trait > 0 ? 2 : 1);
                if (relicCrown) craftFruitEssence += 1; // Crown drops essence on eat

                int eatScore = relicCrown ? 400 : 200;

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
                    score += eatScore * mult;
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

    // Fruit Spawning (Mythic Ambrosia if Philosopher's Pac-Stone is forged)
    if (dotCount < 40 && fruitActive == 0 && fruitTimer == 0 && (MyRand() % 150 == 0)) {
        fruitActive = 1;
        fruitTimer = 100;
    }
    if (fruitActive) {
        fruitTimer--;
        if (fruitTimer <= 0) fruitActive = 0;
        else if (px == 7 && py == 12) {
            int mult = (gameMode == 1) ? (1 + endlessWave / 2) : 1;
            if (relicStone) mult *= 2;
            int fruitPts = relicStone ? 2500 : 500;
            score += fruitPts * mult;
            craftFruitEssence += 2;
            AddShockwave(7 * TS + TS/2, 12 * TS + TS/2, relicStone ? RGB(255, 215, 0) : RGB(0, 230, 118));
            if (score > highScore) highScore = score;
            if (score > statsMaxScore) statsMaxScore = score;
            fruitActive = 0;
            MessageBeep(MB_ICONASTERISK);
        }
    }

    // Update Shockwaves (Dual-tier concentric)
    for (int i = 0; i < numShockwaves; i++) {
        shockwaves[i].r1 += 2;
        shockwaves[i].r2 += 1;
        if (shockwaves[i].r1 >= shockwaves[i].maxR) {
            shockwaves[i] = shockwaves[--numShockwaves];
            i--;
        }
    }

    // Update 4-Layer Kinematic Particles
    for (int i = 0; i < numParticles; i++) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        if (particles[i].type == 0) { // Needle spark
            particles[i].vx *= 0.92;
            particles[i].vy *= 0.92;
        } else if (particles[i].type == 1) { // Smoke puff
            particles[i].vx *= 0.88;
            particles[i].vy = particles[i].vy * 0.88 - 0.08;
        } else if (particles[i].type == 2) { // Heavy debris shard
            particles[i].vx *= 0.95;
            particles[i].vy += 0.22;
            particles[i].rot += particles[i].vRot;
        } else if (particles[i].type == 3) { // Energy star
            particles[i].vx *= 0.90;
            particles[i].vy *= 0.90;
            particles[i].rot += particles[i].vRot;
        }
        particles[i].life--;
        if (particles[i].life <= 0) {
            particles[i] = particles[--numParticles];
            i--;
        }
    }

    // Trailing speed dust motes when Pac-Man moves
    if ((pdx != 0 || pdy != 0) && (frameCount % 2 == 0) && numParticles < MAX_PARTICLES) {
        particles[numParticles].x = px * TS + TS/2 + ((MyRand() % 6) - 3);
        particles[numParticles].y = py * TS + TS/2 + ((MyRand() % 6) - 3);
        particles[numParticles].vx = -pdx * 0.6;
        particles[numParticles].vy = -pdy * 0.6;
        particles[numParticles].life = 8;
        particles[numParticles].maxLife = 8;
        particles[numParticles].color = RGB(255, 235, 59);
        particles[numParticles].type = 1;
        particles[numParticles].size = 2;
        particles[numParticles].rot = 0;
        particles[numParticles].vRot = 0;
        numParticles++;
    }
    if (petActive && (frameCount % 3 == 0) && numParticles < MAX_PARTICLES) {
        COLORREF pCols[4] = { RGB(255, 82, 82), RGB(0, 229, 255), RGB(240, 98, 146), RGB(255, 215, 0) };
        particles[numParticles].x = petX * TS + TS/2 + ((MyRand() % 4) - 2);
        particles[numParticles].y = petY * TS + TS/2 + ((MyRand() % 4) - 2);
        particles[numParticles].vx = 0;
        particles[numParticles].vy = -0.5;
        particles[numParticles].life = 10;
        particles[numParticles].maxLife = 10;
        particles[numParticles].color = pCols[petType % 4];
        particles[numParticles].type = 1;
        particles[numParticles].size = 2;
        particles[numParticles].rot = 0;
        particles[numParticles].vRot = 0;
        numParticles++;
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
            if (key == VK_TAB && showCraftMenu) { craftMenuPage = !craftMenuPage; return 0; }
            if (key == 'O') {
                gameMode = !gameMode;
                Init(0);
                wsprintfA(saveMsgText, "MODE: %s", gameMode ? "ARCADE ENDLESS" : "CAMPAIGN");
                saveMsgTimer = 25;
                MessageBeep(MB_OK);
                return 0;
            }

            // Loop 11: Pet Companion Hotkeys: U = Pet Ultimate, P = Pet Type Cycle
            if (key == 'U') { TriggerPetUltimate(); return 0; }
            if (key == 'P') { CyclePetCompanion(); return 0; }

            // Quick craft hotkeys
            if (showCraftMenu && craftMenuPage == 0) {
                if (key == '1' || key == '7') { CraftItem(1); return 0; }
                if (key == '2' || key == '8') { CraftItem(2); return 0; }
                if (key == '3' || key == '9') { CraftItem(3); return 0; }
                if (key == '4' || key == '0') { CraftItem(4); return 0; }
            } else if (showCraftMenu && craftMenuPage == 1) {
                if (key == '1' || key == '5') { CraftItem(5); return 0; }
                if (key == '2' || key == '6') { CraftItem(6); return 0; }
                if (key == '3' || key == '7') { CraftItem(7); return 0; }
                if (key == '4' || key == '8') { CraftItem(8); return 0; }
                if (key == '5' || key == '9') { CraftItem(9); return 0; }
            } else {
                if (key == '7') { CraftItem(1); return 0; }
                if (key == '8') { CraftItem(2); return 0; }
                if (key == '9') { CraftItem(3); return 0; }
                if (key == '0') { CraftItem(4); return 0; }
            }

            if (showHelp || showCraftMenu) break;

            if (key == 'E') {
                HANDLE hFile = CreateFileA("kpac_data.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    char buf[512];
                    wsprintfA(buf, "{\"highScore\":%d,\"endlessHighWave\":%d,\"ecto\":%d,\"ess\":%d,\"dust\":%d,\"crown\":%d,\"hourglass\":%d,\"orb\":%d,\"aegis\":%d,\"stone\":%d,\"petType\":%d,\"petLevel\":%d}",
                        highScore, endlessHighWave, craftEctoplasm, craftFruitEssence, craftStarDust, relicCrown, relicHourglass, relicOrb, relicAegis, relicStone, petType, petLevel);
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
                    char buf[512]; buf[0] = 0;
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
            if (key == VK_SPACE) paused = !paused;
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
                int sx = (int)(MySin(frameCount * 2.3) * screenShake);
                int sy = (int)(MyCos(frameCount * 2.9) * screenShake);
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

            // Pulsating Energy Perimeter Inlay Border
            int periPulse = 100 + (int)(55.0 * MySin(frameCount * 0.15));
            HPEN periPen = CreatePen(PS_SOLID, 1, RGB(10, periPulse / 2, periPulse));
            SelectObject(memDC, periPen);
            SelectObject(memDC, GetStockObject(HOLLOW_BRUSH));
            Rectangle(memDC, 3, 3, 297, 297);
            DeleteObject(periPen);

            // Traveling Specular Glint along perimeter (perimeter = 294 * 4 = 1176)
            int periTotal = 294 * 4;
            int gDist = (frameCount * 6) % periTotal;
            int gx = 3, gy = 3;
            if (gDist < 294) { gx = 3 + gDist; gy = 3; }
            else if (gDist < 588) { gx = 297; gy = 3 + (gDist - 294); }
            else if (gDist < 882) { gx = 297 - (gDist - 588); gy = 297; }
            else { gx = 3; gy = 297 - (gDist - 882); }
            HBRUSH glintBr = CreateSolidBrush(RGB(255, 255, 255));
            SelectObject(memDC, glintBr);
            SelectObject(memDC, GetStockObject(NULL_PEN));
            Ellipse(memDC, gx - 2, gy - 2, gx + 3, gy + 3);
            DeleteObject(glintBr);

            // Ornate Cybernetic Arcade HUD Corner Reticle L-Brackets with Tech Notches
            HPEN reticlePen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
            SelectObject(memDC, reticlePen);
            int armLen = 14;
            // Top-Left (4, 4)
            MoveToEx(memDC, 4, 4 + armLen, NULL); LineTo(memDC, 4, 4); LineTo(memDC, 4 + armLen, 4);
            MoveToEx(memDC, 7, 7, NULL); LineTo(memDC, 10, 7);
            // Top-Right (296, 4)
            MoveToEx(memDC, 296 - armLen, 4, NULL); LineTo(memDC, 296, 4); LineTo(memDC, 296, 4 + armLen);
            MoveToEx(memDC, 293, 7, NULL); LineTo(memDC, 290, 7);
            // Bottom-Left (4, 296)
            MoveToEx(memDC, 4, 296 - armLen, NULL); LineTo(memDC, 4, 296); LineTo(memDC, 4 + armLen, 296);
            MoveToEx(memDC, 7, 293, NULL); LineTo(memDC, 10, 293);
            // Bottom-Right (296, 296)
            MoveToEx(memDC, 296 - armLen, 296, NULL); LineTo(memDC, 296, 296); LineTo(memDC, 296, 296 - armLen);
            MoveToEx(memDC, 293, 293, NULL); LineTo(memDC, 290, 293);
            DeleteObject(reticlePen);

            // Glowing Status Diodes
            COLORREF diodeColors[4] = { RGB(0, 255, 200), RGB(255, 215, 0), RGB(255, 64, 129), RGB(0, 229, 255) };
            COLORREF curDiode = diodeColors[(frameCount / 10) % 4];
            HBRUSH diodeBr = CreateSolidBrush(curDiode);
            SelectObject(memDC, diodeBr);
            SelectObject(memDC, GetStockObject(NULL_PEN));
            Ellipse(memDC, 4, 4, 7, 7);
            Ellipse(memDC, 293, 4, 296, 7);
            Ellipse(memDC, 4, 293, 7, 296);
            Ellipse(memDC, 293, 293, 296, 296);
            DeleteObject(diodeBr);

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
                        
                        HBRUSH baseBr = CreateSolidBrush(relicOrb ? RGB(186, 104, 200) : (petType == 3 ? RGB(255, 215, 0) : RGB(216, 134, 59)));
                        HBRUSH midBr = CreateSolidBrush(relicOrb ? RGB(225, 190, 231) : (petType == 3 ? RGB(255, 235, 59) : RGB(255, 200, 150)));
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
                    } else if (map[r][c] == 9) { // Void Rift / Hazard Portal
                        int cx = c * TS + TS/2, cy = r * TS + TS/2;
                        int pulse = (int)(MySin(frameCount * 0.4) * 3);
                        HPEN rPen = CreatePen(PS_SOLID, 2, RGB(186, 104, 200));
                        SelectObject(memDC, rPen);
                        SelectObject(memDC, GetStockObject(HOLLOW_BRUSH));
                        Ellipse(memDC, cx - 7 - pulse, cy - 7 - pulse, cx + 7 + pulse, cy + 7 + pulse);
                        DeleteObject(rPen);
                        HBRUSH coreBr = CreateSolidBrush(RGB(74, 20, 140));
                        SelectObject(memDC, coreBr);
                        SelectObject(memDC, GetStockObject(NULL_PEN));
                        Ellipse(memDC, cx - 4, cy - 4, cx + 4, cy + 4);
                        DeleteObject(coreBr);
                    } else if (map[r][c] == 10) { // Phase Gateway
                        int cx = c * TS + TS/2, cy = r * TS + TS/2;
                        int open = (frameCount % 40 < 20);
                        HPEN gPen = CreatePen(PS_SOLID, 2, open ? RGB(0, 230, 118) : RGB(255, 23, 68));
                        SelectObject(memDC, gPen);
                        MoveToEx(memDC, cx - 6, cy, NULL); LineTo(memDC, cx + 6, cy);
                        DeleteObject(gPen);
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

            // Draw Shockwave Rings (Dual-Tier Concentric)
            for (int i = 0; i < numShockwaves; i++) {
                // Outer dispersion halo
                HPEN sPen1 = CreatePen(PS_SOLID, 1, shockwaves[i].color);
                SelectObject(memDC, sPen1);
                SelectObject(memDC, GetStockObject(HOLLOW_BRUSH));
                if (shockwaves[i].is3D) {
                    Ellipse(memDC, shockwaves[i].x - shockwaves[i].r1, shockwaves[i].y - shockwaves[i].r1 / 2, shockwaves[i].x + shockwaves[i].r1, shockwaves[i].y + shockwaves[i].r1 / 2);
                } else {
                    Ellipse(memDC, shockwaves[i].x - shockwaves[i].r1, shockwaves[i].y - shockwaves[i].r1,
                                   shockwaves[i].x + shockwaves[i].r1, shockwaves[i].y + shockwaves[i].r1);
                }
                DeleteObject(sPen1);
                // Inner compression wave
                if (shockwaves[i].r2 < shockwaves[i].maxR) {
                    HPEN sPen2 = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                    SelectObject(memDC, sPen2);
                    if (shockwaves[i].is3D) {
                        Ellipse(memDC, shockwaves[i].x - shockwaves[i].r2, shockwaves[i].y - shockwaves[i].r2 / 2, shockwaves[i].x + shockwaves[i].r2, shockwaves[i].y + shockwaves[i].r2 / 2);
                    } else {
                        Ellipse(memDC, shockwaves[i].x - shockwaves[i].r2, shockwaves[i].y - shockwaves[i].r2,
                                       shockwaves[i].x + shockwaves[i].r2, shockwaves[i].y + shockwaves[i].r2);
                    }
                    DeleteObject(sPen2);
                }
            }

            // Draw 4-Layer Kinematic Particles
            for (int i = 0; i < numParticles; i++) {
                int ppx = (int)particles[i].x;
                int ppy = (int)particles[i].y;
                if (particles[i].type == 0) { // Layer 0: Needle spark with velocity trail
                    HPEN spkPen = CreatePen(PS_SOLID, 1, particles[i].color);
                    SelectObject(memDC, spkPen);
                    MoveToEx(memDC, ppx - (int)(particles[i].vx * 1.5), ppy - (int)(particles[i].vy * 1.5), NULL);
                    LineTo(memDC, ppx, ppy);
                    DeleteObject(spkPen);
                } else if (particles[i].type == 1) { // Layer 1: Expanding buoyant smoke puff
                    int r = particles[i].size;
                    HBRUSH smkBr = CreateSolidBrush(particles[i].color);
                    SelectObject(memDC, smkBr);
                    SelectObject(memDC, GetStockObject(NULL_PEN));
                    Ellipse(memDC, ppx - r, ppy - r, ppx + r + 1, ppy + r + 1);
                    DeleteObject(smkBr);
                } else if (particles[i].type == 2) { // Layer 2: Tumbling heavy shard (diamond)
                    int sz = particles[i].size;
                    POINT pts[4] = {
                        { ppx, ppy - sz },
                        { ppx + sz, ppy },
                        { ppx, ppy + sz },
                        { ppx - sz, ppy }
                    };
                    HBRUSH shdBr = CreateSolidBrush(particles[i].color);
                    SelectObject(memDC, shdBr);
                    SelectObject(memDC, GetStockObject(NULL_PEN));
                    Polygon(memDC, pts, 4);
                    DeleteObject(shdBr);
                } else if (particles[i].type == 3) { // Layer 3: Energy Star (4-pointed cross star)
                    HPEN starPen = CreatePen(PS_SOLID, 1, particles[i].color);
                    SelectObject(memDC, starPen);
                    int sz = particles[i].size;
                    MoveToEx(memDC, ppx - sz, ppy, NULL); LineTo(memDC, ppx + sz + 1, ppy);
                    MoveToEx(memDC, ppx, ppy - sz, NULL); LineTo(memDC, ppx, ppy + sz + 1);
                    DeleteObject(starPen);
                }
            }

            // Draw Loop 11: Companion Pet
            if (petActive) {
                int petCx = petX * TS + TS/2;
                int petCy = petY * TS + TS/2 + (int)(MySin(petFloatAngle) * 3);
                COLORREF petCol = RGB(255, 215, 0);
                if (petType == 0) petCol = RGB(255, 82, 82);       // Blinky Jr Pyre Wisp
                else if (petType == 1) petCol = RGB(0, 229, 255);   // Inky Spark
                else if (petType == 2) petCol = RGB(240, 98, 146);  // Pinky Heart
                else if (petType == 3) petCol = RGB(255, 215, 0);   // Gold Kinglet

                HPEN petAura = CreatePen(PS_SOLID, 1, petCol);
                SelectObject(memDC, petAura);
                SelectObject(memDC, GetStockObject(HOLLOW_BRUSH));
                Ellipse(memDC, petCx - 8, petCy - 8, petCx + 9, petCy + 9);
                DeleteObject(petAura);

                HBRUSH petBr = CreateSolidBrush(petCol);
                SelectObject(memDC, petBr);
                SelectObject(memDC, GetStockObject(NULL_PEN));
                Ellipse(memDC, petCx - 5, petCy - 5, petCx + 6, petCy + 6);
                DeleteObject(petBr);

                // Mini Eyes
                HBRUSH pEye = CreateSolidBrush(RGB(255, 255, 255));
                SelectObject(memDC, pEye);
                Ellipse(memDC, petCx - 3, petCy - 2, petCx, petCy + 1);
                Ellipse(memDC, petCx + 1, petCy - 2, petCx + 4, petCy + 1);
                DeleteObject(pEye);
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

            // Glossy Specular Sheen on Pac-Man
            HBRUSH pacSheen = CreateSolidBrush(RGB(255, 255, 255));
            SelectObject(memDC, pacSheen);
            SelectObject(memDC, GetStockObject(NULL_PEN));
            Ellipse(memDC, cx - 4, cy - 5, cx - 1, cy - 2);
            DeleteObject(pacSheen);

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

                // Shiny 3D Dome Highlight on Ghost Head
                HBRUSH gDome = CreateSolidBrush(RGB(255, 255, 255));
                SelectObject(memDC, gDome);
                SelectObject(memDC, GetStockObject(NULL_PEN));
                Ellipse(memDC, gcx - 3, gy + 1, gcx, gy + 4);
                DeleteObject(gDome);

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
                HBRUSH cBr = CreateSolidBrush(relicStone ? RGB(255, 215, 0) : RGB(213, 0, 0));
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

            // Crafting Materials & HUD Bar (With Relic Badges)
            char relicIcons[16] = "";
            if (relicCrown) lstrcatA(relicIcons, "C");
            if (relicHourglass) lstrcatA(relicIcons, "H");
            if (relicOrb) lstrcatA(relicIcons, "O");
            if (relicAegis) lstrcatA(relicIcons, "A");
            if (relicStone) lstrcatA(relicIcons, "S");

            wsprintfA(sstr, "Ect:%d Ess:%d Dst:%d [%s] | Pet:Lv%d", craftEctoplasm, craftFruitEssence, craftStarDust, relicIcons[0] ? relicIcons : "-", petLevel);
            SetTextColor(memDC, RGB(0, 255, 200));
            TextOutA(memDC, 2, H - 55, sstr, lstrlenA(sstr));

            char rText[16] = "";
            if (replayMode == 1) lstrcpyA(rText, " [REC]");
            else if (replayMode == 2) lstrcpyA(rText, " [PLAY]");

            wsprintfA(sstr, "F:%s Z:%s M:%s B:%s U:%s%s",
                freezeCooldown > 0 ? "CD" : "OK",
                speedCooldown > 0 ? "CD" : "OK",
                magnetCooldown > 0 ? "CD" : "OK",
                shieldActive ? "ON" : (shieldCooldown > 0 ? "CD" : "OK"),
                petCooldown > 0 ? "CD" : "OK", rText);
            SetTextColor(memDC, RGB(255, 235, 59));
            TextOutA(memDC, 2, H - 35, sstr, lstrlenA(sstr));
            
            SetTextColor(memDC, RGB(180, 180, 180));
            TextOutA(memDC, 2, H - 18, "[H]Help [U]PetUlt [P]PetType [C]Forge [O]Mode", 45);

            if (showCraftMenu) {
                HBRUSH overlay = CreateSolidBrush(RGB(5, 12, 25));
                RECT overlayRect = {10, 20, W - 10, H - 85};
                FillRect(memDC, &overlayRect, overlay);
                DeleteObject(overlay);
                HPEN bordPen = CreatePen(PS_SOLID, 2, RGB(0, 229, 255));
                SelectObject(memDC, bordPen);
                SelectObject(memDC, GetStockObject(NULL_BRUSH));
                Rectangle(memDC, 10, 20, W - 10, H - 85);
                DeleteObject(bordPen);

                SetTextColor(memDC, RGB(0, 255, 255));
                if (craftMenuPage == 0) {
                    TextOutA(memDC, 25, 30, "=== CYBER FORGE: CONSUMABLES ===", 32);
                    SetTextColor(memDC, RGB(255, 255, 255));
                    TextOutA(memDC, 20, 55, "1/7: Super Pellet (2 Ecto + 1 Dust)", 35);
                    TextOutA(memDC, 20, 80, "2/8: Chrono Warp (2 Ecto + 2 Ess)", 33);
                    TextOutA(memDC, 20, 105, "3/9: Aegis Shield (2 Dust + 2 Ess)", 34);
                    TextOutA(memDC, 20, 130, "4/0: Void Pulse Bomb (3E+3D+1Ess)", 33);
                } else {
                    TextOutA(memDC, 25, 30, "=== CYBER FORGE: MYTHIC RELICS ===", 34);
                    SetTextColor(memDC, RGB(255, 255, 255));
                    TextOutA(memDC, 20, 52, "1/5: Crown of Ghost King (4E+3D)", 32);
                    TextOutA(memDC, 20, 74, "2/6: Chrono Hourglass (4Ess+3D)", 31);
                    TextOutA(memDC, 20, 96, "3/7: Astral Devourer Orb (4E+4D)", 32);
                    TextOutA(memDC, 20, 118, "4/8: Sun Titan Aegis (4Ess+4D)", 30);
                    TextOutA(memDC, 20, 140, "5/9: Philosopher Stone (5E+5S+5D)", 33);
                }
                SetTextColor(memDC, RGB(255, 215, 0));
                char invStr[64];
                wsprintfA(invStr, "Current: %d Ecto, %d Ess, %d Dust", craftEctoplasm, craftFruitEssence, craftStarDust);
                TextOutA(memDC, 20, 165, invStr, lstrlenA(invStr));
                SetTextColor(memDC, RGB(0, 230, 118));
                TextOutA(memDC, 20, 190, "[Tab] Toggle Relics | [C] Close", 31);
            } else if (showHelp) {
                HBRUSH overlay = CreateSolidBrush(RGB(0, 0, 0));
                RECT overlayRect = {0, 0, W, H};
                FillRect(memDC, &overlayRect, overlay);
                DeleteObject(overlay);
                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, 70, 30, "KPac Loop 11 - Help", 19);
                TextOutA(memDC, 25, 55, "Move: Arrows / WASD | Space: Pause", 34);
                TextOutA(memDC, 25, 75, "Skills: F(Frz) Z(Spr) M(Mag) B(Shd)", 35);
                TextOutA(memDC, 25, 95, "Pet: [U] Pet Ultimate | [P] Cycle Pet", 37);
                TextOutA(memDC, 25, 115, "Mode: [O] Toggle Campaign / Endless", 35);
                TextOutA(memDC, 25, 135, "Cyber-Forge: [C] Menu [Tab] Relics", 34);
                TextOutA(memDC, 25, 155, "Portals: Blue/Purple Void Rifts", 31);
                TextOutA(memDC, 25, 175, "Diff: 1(Easy) 2(Norm) 3(Hard)", 29);
                TextOutA(memDC, 25, 195, "Save/Load: V / L | Replay: R / T", 32);
                SetTextColor(memDC, RGB(0, 230, 118));
                TextOutA(memDC, 35, 230, "Press H or F1 to Start/Resume", 29);
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
