#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define W 300
#define H 300
#define COLS 15
#define ROWS 15
#define TS 20

char maps[15][ROWS][COLS] = {
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
    }
};
char map[ROWS][COLS];

int randSeed = 42;
int MyRand() {
    randSeed = randSeed * 1103515245 + 12345;
    return (unsigned int)(randSeed / 65536) % 32768;
}

int px = 7, py = 12;
int pdx = 0, pdy = 0;
int ndx = 0, ndy = 0;

typedef struct { int x; int y; int dx; int dy; COLORREF c; } Ghost;
Ghost ghosts[5];
int freezeTimer = 0;
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
int speedTimer = 0;

int statsGamesPlayed = 0;
int statsGhostsEaten = 0;
int statsMaxScore = 0;

#ifndef abs
#define abs(x) (((x) < 0) ? -(x) : (x))
#endif

void LoadHighScore() {
    HANDLE hFile = CreateFileA("kpac_hi.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes = 0;
        ReadFile(hFile, &highScore, sizeof(int), &readBytes, NULL);
        CloseHandle(hFile);
    }
    hFile = CreateFileA("kpac_stats.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD readBytes = 0;
        ReadFile(hFile, &statsGamesPlayed, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &statsGhostsEaten, sizeof(int), &readBytes, NULL);
        ReadFile(hFile, &statsMaxScore, sizeof(int), &readBytes, NULL);
        CloseHandle(hFile);
    }
}
void SaveHighScore() {
    HANDLE hFile = CreateFileA("kpac_hi.dat", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD writtenBytes = 0;
        WriteFile(hFile, &highScore, sizeof(int), &writtenBytes, NULL);
        CloseHandle(hFile);
    }
    hFile = CreateFileA("kpac_stats.dat", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD writtenBytes = 0;
        WriteFile(hFile, &statsGamesPlayed, sizeof(int), &writtenBytes, NULL);
        WriteFile(hFile, &statsGhostsEaten, sizeof(int), &writtenBytes, NULL);
        WriteFile(hFile, &statsMaxScore, sizeof(int), &writtenBytes, NULL);
        CloseHandle(hFile);
    }
}

void Init(int keepScore) {
    if (!keepScore) { score = 0; level = 1; lives = 3; }
    gameOver = 0;
    paused = 0;
    dotCount = 0;
    int mapIndex = (level - 1) % 15;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            map[r][c] = maps[mapIndex][r][c];
            if (map[r][c] >= 2 && map[r][c] <= 5) dotCount++;
        }
    }
    px = 7; py = 12;
    pdx = 0; pdy = 0;
    ndx = 0; ndy = 0;
    ghosts[0] = (Ghost){7, 6, 0, -1, RGB(255, 23, 68)};
    ghosts[1] = (Ghost){6, 7, -1, 0, RGB(240, 98, 146)};
    ghosts[2] = (Ghost){8, 7, 1, 0, RGB(0, 229, 255)};
    ghosts[3] = (Ghost){7, 7, 0, 1, RGB(255, 145, 0)};
    ghosts[4] = (Ghost){7, 5, 0, -1, RGB(0, 230, 118)};
    frightTimer = 0;
    freezeTimer = 0;
    fruitActive = 0;
    fruitTimer = 0;
    speedTimer = 0;
}

void Update() {
    if (gameOver || paused) return;
    
    if (freezeTimer > 0) freezeTimer--;
    int numGhosts = (level > 5) ? 5 : 4;
    
    // Ghost basic logic (random move)
    if (frightTimer > 0) frightTimer--;
    int ghostSpeed = 4 - (level / 3);
    if (ghostSpeed < 1) ghostSpeed = 1;
    if (frightTimer > 0) ghostSpeed *= 2;

    if (freezeTimer == 0 && frameCount % ghostSpeed == 0) {
        int dirs[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
        for(int i=0; i<numGhosts; i++) {
            int oldX = ghosts[i].x;
            int oldY = ghosts[i].y;
            if (frightTimer == 0) {
                int tx = px;
                int ty = py;
                if (i == 1) { // Pink ghost anticipates
                    tx = px + pdx * 2;
                    ty = py + pdy * 2;
                } else if (i == 2) { // Cyan ghost tries to flank
                    tx = px - pdx * 2;
                    ty = py - pdy * 2;
                } else if (i == 3) { // Orange ghost scatters if too close
                    int distToPac = abs(ghosts[i].x - px) + abs(ghosts[i].y - py);
                    if (distToPac > 6) {
                        tx = px;
                        ty = py;
                    } else {
                        tx = 0;
                        ty = ROWS - 1;
                    }
                } else if (i == 4) { // Green ghost exactly tracks
                    tx = px;
                    ty = py;
                }
                int best_d = -1;
                int min_dist = 9999;
                if (MyRand() % 100 < 20) {
                    best_d = MyRand() % 4;
                } else {
                    for (int d=0; d<4; d++) {
                        int nx = ghosts[i].x + dirs[d][0];
                        int ny = ghosts[i].y + dirs[d][1];
                        if (nx < 0) nx = COLS - 1;
                        if (nx >= COLS) nx = 0;
                        if (map[ny][nx] != 1) {
                            int dist = abs(nx - tx) + abs(ny - ty);
                            if (dist < min_dist) { min_dist = dist; best_d = d; }
                        }
                    }
                }
                if (best_d != -1) {
                    int nx = ghosts[i].x + dirs[best_d][0];
                    if (nx < 0) nx = COLS - 1;
                    if (nx >= COLS) nx = 0;
                    if (map[ghosts[i].y + dirs[best_d][1]][nx] != 1) {
                        ghosts[i].x = nx;
                        ghosts[i].y += dirs[best_d][1];
                    }
                }
            } else {
                int d = MyRand() % 4;
                int nx = ghosts[i].x + dirs[d][0];
                if (nx < 0) nx = COLS - 1;
                if (nx >= COLS) nx = 0;
                if (map[ghosts[i].y + dirs[d][1]][nx] != 1) {
                    ghosts[i].x = nx;
                    ghosts[i].y += dirs[d][1];
                }
            }
            if (ghosts[i].x < 0) ghosts[i].x = COLS - 1;
            if (ghosts[i].x >= COLS) ghosts[i].x = 0;

            if (ghosts[i].x != oldX || ghosts[i].y != oldY) {
                ghosts[i].dx = (ghosts[i].x > oldX) ? 1 : ((ghosts[i].x < oldX) ? -1 : 0);
                ghosts[i].dy = (ghosts[i].y > oldY) ? 1 : ((ghosts[i].y < oldY) ? -1 : 0);
            }
        }
    }
    
    // Player logic
    if (ndx != 0 || ndy != 0) {
        int nx = px + ndx;
        int ny = py + ndy;
        if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS && map[ny][nx] != 1) {
            pdx = ndx; pdy = ndy;
            ndx = 0; ndy = 0;
        }
    }
    
    if (speedTimer > 0) speedTimer--;
    int playerMoves = (speedTimer > 0) ? 1 : (frameCount % 2 == 0);
    
    if (playerMoves) {
        int nx = px + pdx;
        int ny = py + pdy;
        if (nx < 0) nx = COLS - 1;
        if (nx >= COLS) nx = 0;
        
        if (ny >= 0 && ny < ROWS && map[ny][nx] != 1) {
            px = nx;
            py = ny;
            if (map[py][px] >= 2 && map[py][px] <= 5) {
                if (map[py][px] == 3) { score += 40; frightTimer = 50; MessageBeep(MB_OK); }
                else if (map[py][px] == 4) { score += 20; speedTimer = 80; MessageBeep(MB_ICONEXCLAMATION); }
                else if (map[py][px] == 5) { score += 30; freezeTimer = 100; MessageBeep(MB_ICONINFORMATION); }
                else { score += 10; }
                if (score > highScore) highScore = score;
                map[py][px] = 0;
                dotCount--;
                if (dotCount == 0) {
                    level++;
                    Init(1);
                }
            }
        }
    }
    
    for(int i=0; i<numGhosts; i++) {
        if (px == ghosts[i].x && py == ghosts[i].y) {
            if (frightTimer > 0) {
                score += 200;
                statsGhostsEaten++;
                if (score > highScore) highScore = score;
                if (score > statsMaxScore) statsMaxScore = score;
                MessageBeep(MB_ICONASTERISK);
                ghosts[i].x = 7; ghosts[i].y = 6;
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
                    ghosts[0] = (Ghost){7, 6, 0, -1, RGB(255, 23, 68)};
                    ghosts[1] = (Ghost){6, 7, -1, 0, RGB(240, 98, 146)};
                    ghosts[2] = (Ghost){8, 7, 1, 0, RGB(0, 229, 255)};
                    ghosts[3] = (Ghost){7, 7, 0, 1, RGB(255, 145, 0)};
                    ghosts[4] = (Ghost){7, 5, 0, -1, RGB(0, 230, 118)};
                }
                break;
            }
        }
    }
    
    if (dotCount < 50 && fruitActive == 0 && fruitTimer == 0 && (MyRand() % 200 == 0)) {
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

void DrawArcadeWall(HDC hdc, int r, int c) {
    HBRUSH bgBr = CreateSolidBrush(RGB(6, 10, 23));
    RECT wr = {c * TS, r * TS, c * TS + TS, r * TS + TS};
    FillRect(hdc, &wr, bgBr);
    DeleteObject(bgBr);

    HPEN penBlue = CreatePen(PS_SOLID, 2, RGB(30, 136, 229));
    HPEN oldPen = (HPEN)SelectObject(hdc, penBlue);
    HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, nullBr);

    Rectangle(hdc, c * TS + 2, r * TS + 2, c * TS + TS - 2, r * TS + TS - 2);

    SelectObject(hdc, oldPen); SelectObject(hdc, oldBr);
    DeleteObject(penBlue);
}

void DrawPellet(HDC hdc, int c, int r) {
    HBRUSH dotBr = CreateSolidBrush(RGB(255, 200, 150));
    HPEN nullPen = CreatePen(PS_NULL, 0, 0);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, dotBr);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    Ellipse(hdc, c * TS + 8, r * TS + 8, c * TS + 12, r * TS + 12);

    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
    DeleteObject(dotBr); DeleteObject(nullPen);
}

void DrawPowerPellet(HDC hdc, int c, int r, int frame) {
    int rad = 4 + (frame % 3);
    HBRUSH powerBr = CreateSolidBrush(RGB(255, 235, 170));
    HPEN nullPen = CreatePen(PS_NULL, 0, 0);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, powerBr);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    int cx = c * TS + TS / 2;
    int cy = r * TS + TS / 2;
    Ellipse(hdc, cx - rad, cy - rad, cx + rad, cy + rad);

    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
    DeleteObject(powerBr); DeleteObject(nullPen);
}

void DrawSpeedItem(HDC hdc, int c, int r) {
    int cx = c * TS + TS / 2;
    int cy = r * TS + TS / 2;
    POINT pts[6] = {
        {cx + 2, cy - 6}, {cx - 4, cy + 1}, {cx, cy + 1},
        {cx - 2, cy + 6}, {cx + 4, cy - 1}, {cx, cy - 1}
    };
    HBRUSH cyanBr = CreateSolidBrush(RGB(0, 229, 255));
    HPEN nullPen = CreatePen(PS_NULL, 0, 0);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, cyanBr);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    Polygon(hdc, pts, 6);

    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
    DeleteObject(cyanBr); DeleteObject(nullPen);
}

void DrawFreezeItem(HDC hdc, int c, int r) {
    int cx = c * TS + TS / 2;
    int cy = r * TS + TS / 2;
    HPEN icePen = CreatePen(PS_SOLID, 2, RGB(128, 222, 234));
    HPEN oldPen = (HPEN)SelectObject(hdc, icePen);

    MoveToEx(hdc, cx - 5, cy, NULL); LineTo(hdc, cx + 5, cy);
    MoveToEx(hdc, cx, cy - 5, NULL); LineTo(hdc, cx, cy + 5);
    MoveToEx(hdc, cx - 3, cy - 3, NULL); LineTo(hdc, cx + 3, cy + 3);
    MoveToEx(hdc, cx + 3, cy - 3, NULL); LineTo(hdc, cx - 3, cy + 3);

    SelectObject(hdc, oldPen);
    DeleteObject(icePen);
}

void DrawFruit(HDC hdc, int c, int r) {
    int cx = c * TS + TS / 2;
    int cy = r * TS + TS / 2;
    
    HPEN stemPen = CreatePen(PS_SOLID, 1, RGB(76, 175, 80));
    HPEN oldPen = (HPEN)SelectObject(hdc, stemPen);
    MoveToEx(hdc, cx - 3, cy + 1, NULL); LineTo(hdc, cx, cy - 4);
    MoveToEx(hdc, cx + 3, cy + 2, NULL); LineTo(hdc, cx, cy - 4);
    SelectObject(hdc, oldPen); DeleteObject(stemPen);

    HBRUSH redBr = CreateSolidBrush(RGB(213, 0, 0));
    HPEN nullPen = CreatePen(PS_NULL, 0, 0);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, redBr);
    oldPen = (HPEN)SelectObject(hdc, nullPen);

    Ellipse(hdc, cx - 6, cy - 1, cx, cy + 5);
    Ellipse(hdc, cx, cy, cx + 6, cy + 6);

    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
    DeleteObject(redBr); DeleteObject(nullPen);
}

void DrawPacman(HDC hdc, int c, int r, int dx, int dy, int frame, int speedTimer) {
    int cx = c * TS + TS / 2;
    int cy = r * TS + TS / 2;

    HBRUSH pacBr = CreateSolidBrush(RGB(255, 235, 59));
    HPEN nullPen = CreatePen(PS_NULL, 0, 0);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, pacBr);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    int mouthPhase = frame % 4; // 0: closed, 1: medium, 2: wide, 3: medium
    int offset = (mouthPhase == 0) ? 0 : ((mouthPhase == 2) ? 6 : 3);

    int rad1X = cx, rad1Y = cy;
    int rad2X = cx, rad2Y = cy;

    if (dx == 1) { // Right
        rad1X = cx + 8; rad1Y = cy - offset;
        rad2X = cx + 8; rad2Y = cy + offset;
    } else if (dx == -1) { // Left
        rad1X = cx - 8; rad1Y = cy + offset;
        rad2X = cx - 8; rad2Y = cy - offset;
    } else if (dy == 1) { // Down
        rad1X = cx + offset; rad1Y = cy + 8;
        rad2X = cx - offset; rad2Y = cy + 8;
    } else if (dy == -1) { // Up
        rad1X = cx - offset; rad1Y = cy - 8;
        rad2X = cx + offset; rad2Y = cy - 8;
    } else { // Stopped (facing Right)
        rad1X = cx + 8; rad1Y = cy - offset;
        rad2X = cx + 8; rad2Y = cy + offset;
    }

    if (offset == 0) {
        Ellipse(hdc, cx - 8, cy - 8, cx + 8, cy + 8);
    } else {
        Pie(hdc, cx - 8, cy - 8, cx + 8, cy + 8, rad1X, rad1Y, rad2X, rad2Y);
    }

    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
    DeleteObject(pacBr); DeleteObject(nullPen);

    SetPixel(hdc, cx + ((dx == -1) ? 2 : -2), cy - 4, RGB(0, 0, 0));
    SetPixel(hdc, cx + ((dx == -1) ? 3 : -1), cy - 4, RGB(0, 0, 0));

    if (speedTimer > 0) {
        HPEN auraPen = CreatePen(PS_SOLID, 1, RGB(0, 229, 255));
        HBRUSH nullB = (HBRUSH)GetStockObject(NULL_BRUSH);
        oldPen = (HPEN)SelectObject(hdc, auraPen);
        oldBr = (HBRUSH)SelectObject(hdc, nullB);
        Ellipse(hdc, cx - 10, cy - 10, cx + 10, cy + 10);
        SelectObject(hdc, oldPen); SelectObject(hdc, oldBr);
        DeleteObject(auraPen);
    }
}

void DrawGhost(HDC hdc, Ghost *g, int frightTimer, int frame) {
    int gx = g->x * TS + 2;
    int gy = g->y * TS + 2;
    int gw = TS - 4;
    int cx = gx + gw / 2;
    int cy = gy + gw / 2;

    int isScared = frightTimer > 0;
    int isFlashing = isScared && frightTimer < 15 && ((frightTimer / 2) % 2 == 0);
    COLORREF c = isScared ? (isFlashing ? RGB(255, 255, 255) : RGB(30, 136, 229)) : g->c;

    HBRUSH gBr = CreateSolidBrush(c);
    HPEN nullPen = CreatePen(PS_NULL, 0, 0);
    HBRUSH oldBr = (HBRUSH)SelectObject(hdc, gBr);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);

    Ellipse(hdc, gx, gy, gx + gw, gy + gw);
    RECT bodyR = {gx, gy + gw / 2, gx + gw, gy + gw - 2};
    FillRect(hdc, &bodyR, gBr);

    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
    DeleteObject(gBr); DeleteObject(nullPen);

    if (!isScared) {
        int eyeDx = g->dx * 2;
        int eyeDy = g->dy * 2;

        HBRUSH whiteBr = CreateSolidBrush(RGB(255, 255, 255));
        oldBr = (HBRUSH)SelectObject(hdc, whiteBr);
        oldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));

        Ellipse(hdc, cx - 6, cy - 4, cx, cy + 2);
        Ellipse(hdc, cx, cy - 4, cx + 6, cy + 2);

        DeleteObject(whiteBr);

        HBRUSH blueBr = CreateSolidBrush(RGB(13, 71, 161));
        SelectObject(hdc, blueBr);
        Ellipse(hdc, cx - 5 + eyeDx, cy - 3 + eyeDy, cx - 2 + eyeDx, cy + eyeDy);
        Ellipse(hdc, cx + 1 + eyeDx, cy - 3 + eyeDy, cx + 4 + eyeDx, cy + eyeDy);

        SelectObject(hdc, oldBr); SelectObject(hdc, oldPen);
        DeleteObject(blueBr);
    } else {
        COLORREF mColor = isFlashing ? RGB(213, 0, 0) : RGB(255, 255, 255);
        SetPixel(hdc, cx - 3, cy - 2, mColor);
        SetPixel(hdc, cx + 3, cy - 2, mColor);
        HPEN wPen = CreatePen(PS_SOLID, 1, mColor);
        oldPen = (HPEN)SelectObject(hdc, wPen);
        MoveToEx(hdc, cx - 4, cy + 2, NULL); LineTo(hdc, cx - 2, cy + 4);
        LineTo(hdc, cx, cy + 2); LineTo(hdc, cx + 2, cy + 4); LineTo(hdc, cx + 4, cy + 2);
        SelectObject(hdc, oldPen); DeleteObject(wPen);
    }
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
            if (wParam == VK_LEFT) { ndx = -1; ndy = 0; }
            if (wParam == VK_RIGHT) { ndx = 1; ndy = 0; }
            if (wParam == VK_UP) { ndx = 0; ndy = -1; }
            if (wParam == VK_DOWN) { ndx = 0; ndy = 1; }
            if (wParam == VK_RETURN && gameOver) Init(0);
            if (wParam == 'P' && !gameOver) paused = !paused;
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
            
            HBRUSH bg = CreateSolidBrush(RGB(3, 6, 17));
            RECT rc = {0, 0, W, H};
            FillRect(memDC, &rc, bg);
            DeleteObject(bg);
            
            int numGhosts = (level > 5) ? 5 : 4;

            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    if (map[r][c] == 1) {
                        DrawArcadeWall(memDC, r, c);
                    } else if (map[r][c] == 2) {
                        DrawPellet(memDC, c, r);
                    } else if (map[r][c] == 3) {
                        DrawPowerPellet(memDC, c, r, frameCount);
                    } else if (map[r][c] == 4) {
                        DrawSpeedItem(memDC, c, r);
                    } else if (map[r][c] == 5) {
                        DrawFreezeItem(memDC, c, r);
                    }
                }
            }

            if (fruitActive) {
                DrawFruit(memDC, 7, 12);
            }
            
            DrawPacman(memDC, px, py, pdx, pdy, frameCount, speedTimer);
            
            for(int i=0; i<numGhosts; i++) {
                DrawGhost(memDC, &ghosts[i], frightTimer, frameCount);
            }
            
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            char sstr[64];
            wsprintfA(sstr, "Lv:%d Sc:%d HI:%d Lvs:%d", level, score, highScore, lives);
            TextOutA(memDC, 5, 5, sstr, lstrlenA(sstr));
            
            if (gameOver) {
                SetTextColor(memDC, gameOver == 1 ? RGB(255,23,68) : RGB(0,230,118));
                TextOutA(memDC, W/2 - 50, H/2 - 20, gameOver == 1 ? "GAME OVER" : "YOU WIN!", 9);
                char statStr[128];
                wsprintfA(statStr, "Gms: %d Ghsts: %d Max: %d", statsGamesPlayed, statsGhostsEaten, statsMaxScore);
                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, 10, H/2 + 10, statStr, lstrlenA(statStr));
            } else if (paused) {
                SetTextColor(memDC, RGB(255, 235, 59));
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
