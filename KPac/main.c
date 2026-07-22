#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "msvcrt.lib")

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

int Abs(int x) {
    return (x < 0) ? -x : x;
}

int px = 7, py = 12;
int pdx = 0, pdy = 0;
int ndx = 0, ndy = 0;

typedef struct { int x; int y; COLORREF c; } Ghost;
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

int diffMode = 1; // 0 = Easy, 1 = Normal, 2 = Hard
char saveMsgText[64] = "";
int saveMsgTimer = 0;
int numGhosts = 4;

int statsGamesPlayed = 0;
int statsGhostsEaten = 0;
int statsMaxScore = 0;

typedef struct {
    int px, py, pdx, pdy, ndx, ndy;
    Ghost ghosts[5];
    char map[ROWS][COLS];
    int score, level, lives, diffMode;
    int frightTimer, freezeTimer, speedTimer;
    int dotCount, frameCount, fruitActive, fruitTimer, gameOver;
} SaveState;

void SaveGame() {
    SaveState st;
    st.px = px; st.py = py; st.pdx = pdx; st.pdy = pdy; st.ndx = ndx; st.ndy = ndy;
    for (int i = 0; i < 5; i++) st.ghosts[i] = ghosts[i];
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            st.map[r][c] = map[r][c];
        }
    }
    st.score = score; st.level = level; st.lives = lives; st.diffMode = diffMode;
    st.frightTimer = frightTimer; st.freezeTimer = freezeTimer; st.speedTimer = speedTimer;
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
        for (int i = 0; i < 5; i++) ghosts[i] = st.ghosts[i];
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                map[r][c] = st.map[r][c];
            }
        }
        score = st.score; level = st.level; lives = st.lives; diffMode = st.diffMode;
        frightTimer = st.frightTimer; freezeTimer = st.freezeTimer; speedTimer = st.speedTimer;
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
    ghosts[0] = (Ghost){7, 6, RGB(255, 0, 0)};
    ghosts[1] = (Ghost){6, 7, RGB(255, 184, 255)};
    ghosts[2] = (Ghost){8, 7, RGB(0, 255, 255)};
    ghosts[3] = (Ghost){7, 7, RGB(255, 184, 82)};
    ghosts[4] = (Ghost){7, 5, RGB(0, 255, 0)};
    frightTimer = 0;
    freezeTimer = 0;
    fruitActive = 0;
    fruitTimer = 0;
    speedTimer = 0;
}

void Update() {
    if (saveMsgTimer > 0) saveMsgTimer--;
    if (gameOver || paused) return;
    
    if (freezeTimer > 0) freezeTimer--;
    numGhosts = (level > 5) ? 5 : 4;
    
    // Ghost basic logic (random move)
    if (frightTimer > 0) frightTimer--;
    int ghostSpeed = 4 - (level / 3);
    if (diffMode == 0) ghostSpeed += 1;
    else if (diffMode == 2) ghostSpeed = (ghostSpeed > 1) ? (ghostSpeed - 1) : 1;
    if (ghostSpeed < 1) ghostSpeed = 1;
    if (frightTimer > 0) ghostSpeed *= 2;

    if (freezeTimer == 0 && frameCount % ghostSpeed == 0) {
        int dirs[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
        for(int i=0; i<numGhosts; i++) {
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
                    int distToPac = Abs(ghosts[i].x - px) + Abs(ghosts[i].y - py);
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
                            int dist = Abs(nx - tx) + Abs(ny - ty);
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
                if (map[py][px] == 3) {
                    score += 40;
                    frightTimer = (diffMode == 0) ? 75 : ((diffMode == 2) ? 30 : 50);
                    MessageBeep(MB_OK);
                }
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
                    ghosts[0].x = 7; ghosts[0].y = 6;
                    ghosts[1].x = 6; ghosts[1].y = 7;
                    ghosts[2].x = 8; ghosts[2].y = 7;
                    ghosts[3].x = 7; ghosts[3].y = 7;
                    ghosts[4].x = 7; ghosts[4].y = 5;
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
            if (wParam == '1') { diffMode = 0; lstrcpyA(saveMsgText, "DIFF: EASY"); saveMsgTimer = 20; MessageBeep(MB_OK); }
            if (wParam == '2') { diffMode = 1; lstrcpyA(saveMsgText, "DIFF: NORMAL"); saveMsgTimer = 20; MessageBeep(MB_OK); }
            if (wParam == '3') { diffMode = 2; lstrcpyA(saveMsgText, "DIFF: HARD"); saveMsgTimer = 20; MessageBeep(MB_OK); }
            if ((wParam == 'S' || wParam == 's') && !gameOver) SaveGame();
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
            
            HBRUSH bg = CreateSolidBrush(RGB(0, 0, 0));
            RECT rc = {0, 0, W, H};
            FillRect(memDC, &rc, bg);
            DeleteObject(bg);
            
            HBRUSH wallBr = CreateSolidBrush(RGB(20, 20, 200));
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
            
            HBRUSH pacBr = CreateSolidBrush(RGB(255, 255, 0));
            RECT pr = {px * TS + 2, py * TS + 2, px * TS + TS - 2, py * TS + TS - 2};
            FillRect(memDC, &pr, pacBr);
            DeleteObject(pacBr);
            
            for(int i=0; i<numGhosts; i++) {
                COLORREF c = ghosts[i].c;
                if (frightTimer > 0) {
                    c = ((frightTimer / 2) % 2 == 0) ? RGB(0,0,255) : RGB(255,255,255);
                }
                HBRUSH gBr = CreateSolidBrush(c);
                RECT gr = {ghosts[i].x * TS + 2, ghosts[i].y * TS + 2, ghosts[i].x * TS + TS - 2, ghosts[i].y * TS + TS - 2};
                FillRect(memDC, &gr, gBr);
                DeleteObject(gBr);
            }
            
            if (fruitActive) {
                HBRUSH fBr = CreateSolidBrush(RGB(0, 255, 0));
                RECT fr = {7 * TS + 4, 12 * TS + 4, 7 * TS + TS - 4, 12 * TS + TS - 4};
                FillRect(memDC, &fr, fBr);
                DeleteObject(fBr);
            }
            
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            char sstr[64];
            const char* diffNames[] = {"EASY", "NORM", "HARD"};
            wsprintfA(sstr, "Lv:%d Sc:%d HI:%d Lvs:%d", level, score, highScore, lives);
            TextOutA(memDC, 2, 0, sstr, lstrlenA(sstr));
            wsprintfA(sstr, "Diff:%s [1-3:Diff S:Save L:Load]", diffNames[diffMode]);
            TextOutA(memDC, 2, 10, sstr, lstrlenA(sstr));
            
            if (saveMsgTimer > 0) {
                SetTextColor(memDC, RGB(255, 255, 0));
                TextOutA(memDC, W/2 - 40, H/2 + 25, saveMsgText, lstrlenA(saveMsgText));
            }
            
            if (gameOver) {
                SetTextColor(memDC, gameOver == 1 ? RGB(255,0,0) : RGB(0,255,0));
                TextOutA(memDC, W/2 - 50, H/2 - 20, gameOver == 1 ? "GAME OVER" : "YOU WIN!", 9);
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
