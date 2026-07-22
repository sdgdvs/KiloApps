#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

const int W = 320;
const int H = 240;

const int orig_map1[10][10] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,5,0,0,1,5,0,1},
    {1,0,1,1,0,0,1,0,0,1},
    {1,5,0,1,0,0,0,0,0,1},
    {1,0,0,1,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,5,1},
    {1,1,0,1,1,1,1,1,0,1},
    {1,0,0,0,5,0,0,1,0,1},
    {1,0,0,0,0,0,0,2,0,1},
    {1,1,1,1,1,1,1,1,1,1}
};

const int orig_map2[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,5,0,0,1,0,0,0,5,0,1},
    {1,0,1,1,0,1,0,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,0,1,0,1},
    {1,5,0,0,1,0,0,1,5,1,0,1},
    {1,1,1,0,1,0,1,1,0,1,0,1},
    {1,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,5,1},
    {1,0,5,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,0,1,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map3[15][15] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,5,0,0,0,0,0,0,5,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,1,5,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,1,0,1,0,5,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,1,2,1,0,1,0,1,5,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,1,0,1,0,1,0,1},
    {1,5,1,0,1,1,1,1,1,0,1,0,1,0,1},
    {1,0,1,0,0,5,0,0,0,0,1,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,5,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map4[10][10] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,5,0,0,1,3,0,1},
    {1,1,1,1,1,0,1,1,0,1},
    {1,5,0,0,1,0,0,0,0,1},
    {1,0,1,0,1,1,1,1,1,1},
    {1,0,1,0,0,0,5,0,4,1},
    {1,0,1,1,1,1,1,0,1,1},
    {1,0,5,0,0,0,1,0,2,1},
    {1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1}
};

const int orig_map5[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,0,0,0,0,3,1},
    {1,0,1,0,1,0,1,1,1,1,1,1},
    {1,0,1,0,1,0,0,0,0,0,0,1},
    {1,0,1,0,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,1,0,1,0,1},
    {1,1,1,1,1,1,0,1,0,1,0,1},
    {1,4,0,0,0,1,0,0,0,1,0,1},
    {1,2,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};



const int orig_map6[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,6,0,0,0,1,5,0,0,1},
    {1,0,1,1,1,1,0,1,1,1,0,1},
    {1,0,1,5,0,1,0,6,0,1,0,1},
    {1,0,1,0,0,1,0,1,0,1,0,1},
    {1,0,1,1,4,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,0,1,0,1},
    {1,3,6,0,5,0,0,1,0,1,0,1},
    {1,0,1,1,1,1,0,1,6,1,0,1},
    {1,0,0,0,0,1,0,0,0,1,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map7[15][15] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,1,0,0,0,1,0,0,0,1,0,0,8,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,1,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,1,1,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,5,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,1,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,3,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,1,1},
    {1,0,1,0,1,0,1,0,1,0,1,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,0,1,1,1,0,1},
    {1,0,0,0,1,0,0,0,1,0,0,0,4,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
    {1,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map8[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,1,2,1,0,0,0,5,0,1},
    {1,1,0,1,4,1,0,1,1,1,1,1},
    {1,0,0,1,0,1,0,1,3,0,0,1},
    {1,0,1,1,7,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,1,5,0,0,1},
    {1,1,1,1,1,1,0,1,1,1,1,1},
    {1,5,0,0,0,1,0,1,0,0,0,1},
    {1,1,1,1,0,1,0,1,0,1,0,1},
    {1,8,0,1,0,7,0,7,0,1,0,1},
    {1,1,0,1,1,1,1,1,1,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map9[12][12] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,6,0,0,1,2,1,0,3,1},
    {1,0,1,1,1,0,1,4,1,0,1,1},
    {1,0,1,8,1,0,1,0,1,0,0,1},
    {1,0,1,1,1,0,1,0,1,1,0,1},
    {1,0,0,0,6,0,7,0,6,0,0,1},
    {1,1,1,1,1,1,1,0,1,1,1,1},
    {1,5,0,0,0,0,1,0,1,5,0,1},
    {1,0,1,1,1,0,1,0,1,0,1,1},
    {1,0,1,6,1,0,1,0,7,0,0,1},
    {1,0,0,0,1,5,1,0,1,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

const int orig_map10[15][15] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,5,0,6,0,0,0,1,8,0,1},
    {1,0,1,0,1,1,1,1,1,1,0,1,1,0,1},
    {1,0,1,0,7,0,0,5,0,1,0,7,0,0,1},
    {1,0,1,1,1,1,1,1,0,1,1,1,1,1,1},
    {1,0,1,3,0,0,0,1,0,0,0,6,0,0,1},
    {1,0,1,1,1,1,0,1,1,1,1,1,1,0,1},
    {1,0,0,6,0,1,0,4,0,0,0,1,0,0,1},
    {1,1,1,1,0,1,1,1,1,1,0,1,0,1,1},
    {1,5,0,1,0,0,0,0,0,1,0,1,0,0,1},
    {1,0,1,1,1,1,1,1,0,1,0,1,1,0,1},
    {1,0,1,2,1,0,0,1,0,1,0,1,5,0,1},
    {1,0,1,4,1,1,0,1,0,1,0,1,1,1,1},
    {1,0,0,0,6,0,0,1,5,1,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

int map1[10][10];
int map2[12][12];
int map3[15][15];
int map4[10][10];
int map5[12][12];
int map6[12][12];
int map7[15][15];
int map8[12][12];
int map9[12][12];
int map10[15][15];


int mapRandom[31][31];
int curRandW = 15;
int curRandH = 15;

void GenerateMaze(int w, int h) {
    for (int i=0; i<w; i++) for (int j=0; j<h; j++) mapRandom[i][j] = 1;
    int stackX[1000], stackY[1000];
    int stackPtr = 0;
    
    int cx = 1, cy = 1;
    mapRandom[cx][cy] = 0;
    stackX[stackPtr] = cx; stackY[stackPtr] = cy; stackPtr++;
    
    int farX = cx, farY = cy;
    
    while(stackPtr > 0) {
        cx = stackX[stackPtr-1];
        cy = stackY[stackPtr-1];
        stackPtr--;
        
        int dirs[4][2] = {{0,-2}, {0,2}, {-2,0}, {2,0}};
        for(int i=0; i<4; i++) {
            int r = rand() % 4;
            int tx = dirs[i][0]; int ty = dirs[i][1];
            dirs[i][0] = dirs[r][0]; dirs[i][1] = dirs[r][1];
            dirs[r][0] = tx; dirs[r][1] = ty;
        }
        
        for(int i=0; i<4; i++) {
            int nx = cx + dirs[i][0];
            int ny = cy + dirs[i][1];
            if (nx > 0 && nx < w-1 && ny > 0 && ny < h-1 && mapRandom[nx][ny] == 1) {
                mapRandom[cx + dirs[i][0]/2][cy + dirs[i][1]/2] = 0;
                mapRandom[nx][ny] = 0;
                
                stackX[stackPtr] = cx; stackY[stackPtr] = cy; stackPtr++;
                stackX[stackPtr] = nx; stackY[stackPtr] = ny; stackPtr++;
                
                farX = nx; farY = ny;
                break;
            }
        }
    }
    
    for(int i=0; i<w*h/10; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 5;
        }
    }
    int placedCompass = 0;
    while(!placedCompass) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0) {
            mapRandom[rx][ry] = 8;
            placedCompass = 1;
        }
    }
    for(int i=0; i<w*h/20; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0) {
            mapRandom[rx][ry] = 6;
        }
    }
    for(int i=0; i<w*h/20; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 1 && rx > 1 && rx < w-2 && ry > 1 && ry < h-2) {
            mapRandom[rx][ry] = 7;
        }
    }
    for(int i=0; i<w*h/30; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 9;
        }
    }
    if (currentLevel >= 5) {
        int placedMino = 0;
        while (!placedMino) {
            int rx = 1 + rand()%(w-2);
            int ry = 1 + rand()%(h-2);
            if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY) && abs(rx - 1) + abs(ry - 1) > 5) {
                mapRandom[rx][ry] = 12;
                placedMino = 1;
            }
        }
    }
    for(int i=0; i<w*h/40; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 13;
        }
    }
    int t1x = 0, t1y = 0, t2x = 0, t2y = 0;
    for(int i=0; i<100; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) { t1x = rx; t1y = ry; break; }
    }
    for(int i=0; i<100; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != t1x || ry != t1y) && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) { t2x = rx; t2y = ry; break; }
    }
    if (t1x && t2x) {
        mapRandom[t1x][t1y] = 10;
        mapRandom[t2x][t2y] = 11;
    }
    mapRandom[farX][farY] = 2;
    curRandW = w;
    curRandH = h;
}

void ResetMaps() {
    memcpy(map1, orig_map1, sizeof(map1));
    memcpy(map2, orig_map2, sizeof(map2));
    memcpy(map3, orig_map3, sizeof(map3));
    memcpy(map4, orig_map4, sizeof(map4));
    memcpy(map5, orig_map5, sizeof(map5));
    memcpy(map6, orig_map6, sizeof(map6));
    memcpy(map7, orig_map7, sizeof(map7));
    memcpy(map8, orig_map8, sizeof(map8));
    memcpy(map9, orig_map9, sizeof(map9));
    memcpy(map10, orig_map10, sizeof(map10));
}

int currentLevel = 0;
int keysHeld = 0;
int hasCompass = 0;
int speedBoost = 0;
int hasPickaxe = 0;

int totalGames = 0;
int totalEscapes = 0;
int totalScore = 0;

int gameState = 0; // 0=start, 1=play, 2=win
DWORD startTime = 0;
DWORD endTime = 0;
float bestTime = 9999.9f;
int score = 0;

char msgText[64] = "";
int msgTimer = 0;

void LoadBest() {
    FILE* f = fopen("kmaze_score.dat", "rb");
    if (f) {
        fread(&bestTime, sizeof(float), 1, f);
        fread(&totalGames, sizeof(int), 1, f);
        fread(&totalEscapes, sizeof(int), 1, f);
        fread(&totalScore, sizeof(int), 1, f);
        fclose(f);
    }
}
void SaveBest() {
    FILE* f = fopen("kmaze_score.dat", "wb");
    if (f) {
        fwrite(&bestTime, sizeof(float), 1, f);
        fwrite(&totalGames, sizeof(int), 1, f);
        fwrite(&totalEscapes, sizeof(int), 1, f);
        fwrite(&totalScore, sizeof(int), 1, f);
        fclose(f);
    }
}


int GetMapValue(int x, int y) {
    if (x < 0 || y < 0) return 1;
    if (currentLevel == 0) {
        if (x >= 10 || y >= 10) return 1;
        return map1[x][y];
    } else if (currentLevel == 1) {
        if (x >= 12 || y >= 12) return 1;
        return map2[x][y];
    } else if (currentLevel == 2) {
        if (x >= 15 || y >= 15) return 1;
        return map3[x][y];
    } else if (currentLevel == 3) {
        if (x >= 10 || y >= 10) return 1;
        return map4[x][y];
    } else if (currentLevel == 4) {
        if (x >= 12 || y >= 12) return 1;
        return map5[x][y];
    } else if (currentLevel == 5) {
        if (x >= 12 || y >= 12) return 1;
        return map6[x][y];
    } else if (currentLevel == 6) {
        if (x >= 15 || y >= 15) return 1;
        return map7[x][y];
    } else if (currentLevel == 7) {
        if (x >= 12 || y >= 12) return 1;
        return map8[x][y];
    } else if (currentLevel == 8) {
        if (x >= 12 || y >= 12) return 1;
        return map9[x][y];
    } else if (currentLevel == 9) {
        if (x >= 15 || y >= 15) return 1;
        return map10[x][y];
    } else {
        if (x >= curRandW || y >= curRandH) return 1;
        return mapRandom[x][y];
    }
}

void SetMapValue(int x, int y, int v) {
    if (x < 0 || y < 0) return;
    if (currentLevel == 0 && x < 10 && y < 10) map1[x][y] = v;
    else if (currentLevel == 1 && x < 12 && y < 12) map2[x][y] = v;
    else if (currentLevel == 2 && x < 15 && y < 15) map3[x][y] = v;
    else if (currentLevel == 3 && x < 10 && y < 10) map4[x][y] = v;
    else if (currentLevel == 4 && x < 12 && y < 12) map5[x][y] = v;
    else if (currentLevel == 5 && x < 12 && y < 12) map6[x][y] = v;
    else if (currentLevel == 6 && x < 15 && y < 15) map7[x][y] = v;
    else if (currentLevel == 7 && x < 12 && y < 12) map8[x][y] = v;
    else if (currentLevel == 8 && x < 12 && y < 12) map9[x][y] = v;
    else if (currentLevel == 9 && x < 15 && y < 15) map10[x][y] = v;
    else if (currentLevel >= 10 && x < curRandW && y < curRandH) mapRandom[x][y] = v;
}

float pX = 1.5f, pY = 1.5f;
void InitGame() { srand((unsigned int)GetTickCount()); LoadBest(); ResetMaps(); }
float dX = 1.0f, dY = 0.0f;
float planeX = 0.0f, planeY = 0.66f;

void NextLevel() {
    keysHeld = 0;
    speedBoost = 0;
    hasPickaxe = 0;
    currentLevel++;
    hasCompass = (currentLevel < 6) ? 1 : 0;
    if (currentLevel > 29) {
        gameState = 2;
        endTime = GetTickCount();
        float elapsed = (endTime - startTime) / 1000.0f;
        if (elapsed < bestTime) {
            bestTime = elapsed;
        }
        totalEscapes++;
        totalScore += score;
        SaveBest();
        return;
    }
    
    if (currentLevel >= 10) {
        int s = 11 + (currentLevel - 10) * 4;
        if (s > 31) s = 31;
        GenerateMaze(s, s);
    }
    
    // Copy default maps back if we wanted strict reset, but this is simple version so we won't fully deep copy here for Native unless needed.
    // For simplicity, Native C will just let blocks stay erased if you die or loop around.
    
    pX = 1.5f; pY = 1.5f;
    dX = 1.0f; dY = 0.0f;
    planeX = 0.0f; planeY = 0.66f;
}

HBITMAP hbmCanvas = NULL;
HDC hdcMem = NULL;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, 1, 30, NULL);
            break;
        case WM_TIMER: {
            static int minotaurTimer = 0;
            float moveSpeed = 0.1f;
            float rotSpeed = 0.05f;
            
            if (gameState == 1) {
                minotaurTimer += 30;
                if (minotaurTimer >= 1000) {
                    minotaurTimer = 0;
                    int mapW = currentLevel >= 10 ? curRandW : 15;
                    int mapH = currentLevel >= 10 ? curRandH : 15;
                    if (currentLevel == 0 || currentLevel == 3) { mapW = 10; mapH = 10; }
                    else if (currentLevel == 1 || currentLevel == 4 || currentLevel == 5 || currentLevel == 7 || currentLevel == 8) { mapW = 12; mapH = 12; }
                    
                    int minotaurs[100][2];
                    int mCount = 0;
                    for (int x = 0; x < mapW; x++) {
                        for (int y = 0; y < mapH; y++) {
                            if (GetMapValue(x, y) == 12) {
                                minotaurs[mCount][0] = x;
                                minotaurs[mCount][1] = y;
                                mCount++;
                            }
                        }
                    }
                    for (int i = 0; i < mCount; i++) {
                        int mx = minotaurs[i][0];
                        int my = minotaurs[i][1];
                        int dx = 0, dy = 0;
                        if ((int)pX > mx) dx = 1;
                        else if ((int)pX < mx) dx = -1;
                        if ((int)pY > my) dy = 1;
                        else if ((int)pY < my) dy = -1;
                        
                        if (dx != 0 && GetMapValue(mx + dx, my) == 0) {
                            SetMapValue(mx, my, 0);
                            SetMapValue(mx + dx, my, 12);
                            mx += dx;
                        } else if (dy != 0 && GetMapValue(mx, my + dy) == 0) {
                            SetMapValue(mx, my, 0);
                            SetMapValue(mx, my + dy, 12);
                            my += dy;
                        }
                        
                        if (mx == (int)pX && my == (int)pY) {
                            MessageBeep(MB_ICONHAND);
                            score = (score >= 100) ? score - 100 : 0;
                            currentLevel--;
                            NextLevel();
                            break;
                        }
                    }
                }
            }
            
            int TryMove(int x, int y) {
                int val = GetMapValue(x, y);
                if (val == 0 || val == 2 || val == 3 || val == 5 || val == 6 || val == 7 || val == 8 || val == 9 || val == 10 || val == 11 || val == 12 || val == 13) return 1;
                if (val == 4) {
                    if (keysHeld > 0) {
                        keysHeld--;
                        SetMapValue(x, y, 0);
                        MessageBeep(MB_ICONEXCLAMATION);
                        return 1;
                    }
                }
                if (val == 1) {
                    if (hasPickaxe > 0 && x > 0 && y > 0) {
                        int mapW = currentLevel >= 10 ? curRandW : 15;
                        int mapH = currentLevel >= 10 ? curRandH : 15;
                        if (currentLevel == 0 || currentLevel == 3) { mapW = 10; mapH = 10; }
                        else if (currentLevel == 1 || currentLevel == 4 || currentLevel == 5 || currentLevel == 7 || currentLevel == 8) { mapW = 12; mapH = 12; }
                        if (x < mapW - 1 && y < mapH - 1) {
                            hasPickaxe--;
                            SetMapValue(x, y, 0);
                            MessageBeep(MB_ICONEXCLAMATION);
                            return 1;
                        }
                    }
                }
                return 0;
            }
            
            if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                if (gameState == 0 || gameState == 2) {
                    gameState = 1;
                    startTime = GetTickCount();
                    currentLevel = -1;
                    score = 0;
                    totalGames++;
                    SaveBest();
                    ResetMaps();
                    NextLevel();
                }
            }
            
            static int saveLoadCooldown = 0;
            if (saveLoadCooldown > 0) saveLoadCooldown -= 30;
            if (msgTimer > 0) msgTimer--;
            
            if (gameState == 1 && saveLoadCooldown <= 0) {
                if (GetAsyncKeyState('S') & 0x8000) {
                    FILE* f = fopen("kmaze_save.dat", "wb");
                    if (f) {
                        fwrite(&currentLevel, sizeof(int), 1, f);
                        fwrite(&score, sizeof(int), 1, f);
                        fwrite(&keysHeld, sizeof(int), 1, f);
                        fwrite(&hasCompass, sizeof(int), 1, f);
                        fwrite(&speedBoost, sizeof(int), 1, f);
                        fwrite(&hasPickaxe, sizeof(int), 1, f);
                        fwrite(&pX, sizeof(float), 1, f);
                        fwrite(&pY, sizeof(float), 1, f);
                        fwrite(&dX, sizeof(float), 1, f);
                        fwrite(&dY, sizeof(float), 1, f);
                        fwrite(&planeX, sizeof(float), 1, f);
                        fwrite(&planeY, sizeof(float), 1, f);
                        DWORD elapsed = GetTickCount() - startTime;
                        fwrite(&elapsed, sizeof(DWORD), 1, f);
                        fwrite(&curRandW, sizeof(int), 1, f);
                        fwrite(&curRandH, sizeof(int), 1, f);
                        fwrite(map1, sizeof(map1), 1, f);
                        fwrite(map2, sizeof(map2), 1, f);
                        fwrite(map3, sizeof(map3), 1, f);
                        fwrite(map4, sizeof(map4), 1, f);
                        fwrite(map5, sizeof(map5), 1, f);
                        fwrite(map6, sizeof(map6), 1, f);
                        fwrite(map7, sizeof(map7), 1, f);
                        fwrite(map8, sizeof(map8), 1, f);
                        fwrite(map9, sizeof(map9), 1, f);
                        fwrite(map10, sizeof(map10), 1, f);
                        fwrite(mapRandom, sizeof(mapRandom), 1, f);
                        fclose(f);
                        strcpy(msgText, "Game Saved!");
                        msgTimer = 60;
                        saveLoadCooldown = 1000;
                        MessageBeep(MB_OK);
                    }
                }
                if (GetAsyncKeyState('L') & 0x8000) {
                    FILE* f = fopen("kmaze_save.dat", "rb");
                    if (f) {
                        fread(&currentLevel, sizeof(int), 1, f);
                        fread(&score, sizeof(int), 1, f);
                        fread(&keysHeld, sizeof(int), 1, f);
                        fread(&hasCompass, sizeof(int), 1, f);
                        fread(&speedBoost, sizeof(int), 1, f);
                        fread(&hasPickaxe, sizeof(int), 1, f);
                        fread(&pX, sizeof(float), 1, f);
                        fread(&pY, sizeof(float), 1, f);
                        fread(&dX, sizeof(float), 1, f);
                        fread(&dY, sizeof(float), 1, f);
                        fread(&planeX, sizeof(float), 1, f);
                        fread(&planeY, sizeof(float), 1, f);
                        DWORD elapsed = 0;
                        fread(&elapsed, sizeof(DWORD), 1, f);
                        startTime = GetTickCount() - elapsed;
                        fread(&curRandW, sizeof(int), 1, f);
                        fread(&curRandH, sizeof(int), 1, f);
                        fread(map1, sizeof(map1), 1, f);
                        fread(map2, sizeof(map2), 1, f);
                        fread(map3, sizeof(map3), 1, f);
                        fread(map4, sizeof(map4), 1, f);
                        fread(map5, sizeof(map5), 1, f);
                        fread(map6, sizeof(map6), 1, f);
                        fread(map7, sizeof(map7), 1, f);
                        fread(map8, sizeof(map8), 1, f);
                        fread(map9, sizeof(map9), 1, f);
                        fread(map10, sizeof(map10), 1, f);
                        fread(mapRandom, sizeof(mapRandom), 1, f);
                        fclose(f);
                        strcpy(msgText, "Game Loaded!");
                        msgTimer = 60;
                        saveLoadCooldown = 1000;
                        MessageBeep(MB_OK);
                    }
                }
            }
            if (gameState != 1) {
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }

            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                moveSpeed = 0.2f;
            }
            if (speedBoost) moveSpeed *= 1.5f;
            if (GetAsyncKeyState(VK_UP) & 0x8000) {
                if (TryMove((int)(pX + dX * moveSpeed), (int)pY)) pX += dX * moveSpeed;
                if (TryMove((int)pX, (int)(pY + dY * moveSpeed))) pY += dY * moveSpeed;
            }
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
                if (TryMove((int)(pX - dX * moveSpeed), (int)pY)) pX -= dX * moveSpeed;
                if (TryMove((int)pX, (int)(pY - dY * moveSpeed))) pY -= dY * moveSpeed;
            }
            
            int curVal = GetMapValue((int)pX, (int)pY);
            if (curVal == 3) {
                keysHeld++;
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_OK);
            } else if (curVal == 2) {
                MessageBeep(MB_ICONASTERISK);
                NextLevel();
            } else if (curVal == 5) {
                score += 100;
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_OK);
            } else if (curVal == 6) {
                MessageBeep(MB_ICONHAND);
                score = (score >= 50) ? score - 50 : 0;
                pX = 1.5f; pY = 1.5f;
            } else if (curVal == 7) {
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_OK);
            } else if (curVal == 8) {
                hasCompass = 1;
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_ICONASTERISK);
            } else if (curVal == 9) {
                speedBoost = 1;
                score += 50;
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_ICONASTERISK);
            } else if (curVal == 10) {
                for(int i=0; i<31; i++) {
                    for(int j=0; j<31; j++) {
                        if (GetMapValue(i, j) == 11) {
                            pX = i + 0.5f; pY = j + 0.5f;
                            MessageBeep(MB_ICONHAND);
                            i=31; break;
                        }
                    }
                }
            } else if (curVal == 11) {
                for(int i=0; i<31; i++) {
                    for(int j=0; j<31; j++) {
                        if (GetMapValue(i, j) == 10) {
                            pX = i + 0.5f; pY = j + 0.5f;
                            MessageBeep(MB_ICONHAND);
                            i=31; break;
                        }
                    }
                }
            } else if (curVal == 12) {
                MessageBeep(MB_ICONHAND);
                score = (score >= 100) ? score - 100 : 0;
                currentLevel--;
                NextLevel();
            } else if (curVal == 13) {
                hasPickaxe++;
                SetMapValue((int)pX, (int)pY, 0);
                MessageBeep(MB_ICONASTERISK);
            }

            if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
                float oldDX = dX;
                dX = dX * (float)cos(rotSpeed) - dY * (float)sin(rotSpeed);
                dY = oldDX * (float)sin(rotSpeed) + dY * (float)cos(rotSpeed);
                float oldPlaneX = planeX;
                planeX = planeX * (float)cos(rotSpeed) - planeY * (float)sin(rotSpeed);
                planeY = oldPlaneX * (float)sin(rotSpeed) + planeY * (float)cos(rotSpeed);
            }
            if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
                float oldDX = dX;
                dX = dX * (float)cos(-rotSpeed) - dY * (float)sin(-rotSpeed);
                dY = oldDX * (float)sin(-rotSpeed) + dY * (float)cos(-rotSpeed);
                float oldPlaneX = planeX;
                planeX = planeX * (float)cos(-rotSpeed) - planeY * (float)sin(-rotSpeed);
                planeY = oldPlaneX * (float)sin(-rotSpeed) + planeY * (float)cos(-rotSpeed);
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            if (!hdcMem) {
                hdcMem = CreateCompatibleDC(hdc);
                hbmCanvas = CreateCompatibleBitmap(hdc, W, H);
                SelectObject(hdcMem, hbmCanvas);
            }
            
            HBRUSH ceilB = CreateSolidBrush(RGB(50, 50, 50));
            HBRUSH floorB = CreateSolidBrush(RGB(100, 100, 100));
            RECT ceilRc = {0, 0, W, H/2};
            RECT floorRc = {0, H/2, W, H};
            FillRect(hdcMem, &ceilRc, ceilB);
            FillRect(hdcMem, &floorRc, floorB);
            DeleteObject(ceilB); DeleteObject(floorB);

            HBRUSH w1 = CreateSolidBrush(RGB(200, 0, 0));
            HBRUSH w2 = CreateSolidBrush(RGB(150, 0, 0));
            HBRUSH e1 = CreateSolidBrush(RGB(0, 255, 0));
            HBRUSH e2 = CreateSolidBrush(RGB(0, 204, 0));
            HBRUSH k1 = CreateSolidBrush(RGB(255, 204, 0));
            HBRUSH k2 = CreateSolidBrush(RGB(204, 153, 0));
            HBRUSH d1 = CreateSolidBrush(RGB(0, 153, 255));
            HBRUSH d2 = CreateSolidBrush(RGB(0, 102, 204));
            HBRUSH c1 = CreateSolidBrush(RGB(255, 128, 0));
            HBRUSH c2 = CreateSolidBrush(RGB(204, 102, 0));
            
            HBRUSH t1 = CreateSolidBrush(RGB(153, 0, 153));
            HBRUSH t2 = CreateSolidBrush(RGB(102, 0, 102));
            HBRUSH m1 = CreateSolidBrush(RGB(0, 255, 255));
            HBRUSH m2 = CreateSolidBrush(RGB(0, 204, 204));
            
            HBRUSH p1 = CreateSolidBrush(RGB(255, 255, 255));
            HBRUSH p2 = CreateSolidBrush(RGB(200, 200, 200));
            HBRUSH tp1 = CreateSolidBrush(RGB(255, 0, 255));
            HBRUSH tp2 = CreateSolidBrush(RGB(200, 0, 200));
            
            HBRUSH w1s = CreateSolidBrush(RGB(170, 0, 0));
            HBRUSH w2s = CreateSolidBrush(RGB(120, 0, 0));
            HBRUSH e1s = CreateSolidBrush(RGB(0, 204, 0));
            HBRUSH e2s = CreateSolidBrush(RGB(0, 153, 0));
            HBRUSH k1s = CreateSolidBrush(RGB(204, 153, 0));
            HBRUSH k2s = CreateSolidBrush(RGB(153, 102, 0));
            HBRUSH d1s = CreateSolidBrush(RGB(0, 102, 204));
            HBRUSH d2s = CreateSolidBrush(RGB(0, 51, 153));
            HBRUSH c1s = CreateSolidBrush(RGB(204, 102, 0));
            HBRUSH c2s = CreateSolidBrush(RGB(153, 76, 0));
            HBRUSH t1s = CreateSolidBrush(RGB(102, 0, 102));
            HBRUSH t2s = CreateSolidBrush(RGB(51, 0, 51));
            HBRUSH m1s = CreateSolidBrush(RGB(0, 204, 204));
            HBRUSH m2s = CreateSolidBrush(RGB(0, 153, 153));
            
            HBRUSH p1s = CreateSolidBrush(RGB(200, 200, 200));
            HBRUSH p2s = CreateSolidBrush(RGB(150, 150, 150));
            HBRUSH tp1s = CreateSolidBrush(RGB(200, 0, 200));
            HBRUSH tp2s = CreateSolidBrush(RGB(150, 0, 150));
            
            HBRUSH min1 = CreateSolidBrush(RGB(255, 50, 50));
            HBRUSH min2 = CreateSolidBrush(RGB(200, 50, 50));
            HBRUSH min1s = CreateSolidBrush(RGB(200, 50, 50));
            HBRUSH min2s = CreateSolidBrush(RGB(150, 50, 50));
            
            HBRUSH pik1 = CreateSolidBrush(RGB(150, 75, 0));
            HBRUSH pik2 = CreateSolidBrush(RGB(120, 60, 0));
            HBRUSH pik1s = CreateSolidBrush(RGB(120, 60, 0));
            HBRUSH pik2s = CreateSolidBrush(RGB(100, 50, 0));
            
            for (int x = 0; x < W; x++) {
                float cameraX = 2 * x / (float)W - 1;
                float rayDX = dX + planeX * cameraX;
                float rayDY = dY + planeY * cameraX;
                
                int mapX = (int)pX;
                int mapY = (int)pY;
                
                float sideDistX, sideDistY;
                float deltaDistX = (rayDX == 0) ? 1e30f : (float)fabs(1 / rayDX);
                float deltaDistY = (rayDY == 0) ? 1e30f : (float)fabs(1 / rayDY);
                float perpWallDist;
                
                int stepX, stepY, hit = 0, side = 0;
                
                if (rayDX < 0) { stepX = -1; sideDistX = (pX - mapX) * deltaDistX; }
                else           { stepX = 1;  sideDistX = (mapX + 1.0f - pX) * deltaDistX; }
                if (rayDY < 0) { stepY = -1; sideDistY = (pY - mapY) * deltaDistY; }
                else           { stepY = 1;  sideDistY = (mapY + 1.0f - pY) * deltaDistY; }
                
                while (hit == 0) {
                    if (sideDistX < sideDistY) {
                        sideDistX += deltaDistX;
                        mapX += stepX;
                        side = 0;
                    } else {
                        sideDistY += deltaDistY;
                        mapY += stepY;
                        side = 1;
                    }
                    if (GetMapValue(mapX, mapY) > 0) hit = GetMapValue(mapX, mapY);
                }
                
                if (side == 0) perpWallDist = (sideDistX - deltaDistX);
                else           perpWallDist = (sideDistY - deltaDistY);
                
                int lineHeight = (int)(H / perpWallDist);
                int drawStart = -lineHeight / 2 + H / 2;
                if (drawStart < 0) drawStart = 0;
                int drawEnd = lineHeight / 2 + H / 2;
                if (drawEnd >= H) drawEnd = H - 1;
                
                float wallX;
                if (side == 0) wallX = pY + perpWallDist * rayDY;
                else           wallX = pX + perpWallDist * rayDX;
                wallX -= floor(wallX);
                int tex = ((int)(wallX * 8.0f)) % 2;
                
                RECT wallRc = {x, drawStart, x+1, drawEnd};
                if (currentLevel >= 15 && perpWallDist > 4.5f) {
                    HBRUSH drk = CreateSolidBrush(RGB(0, 0, 0));
                    FillRect(hdcMem, &wallRc, drk);
                    DeleteObject(drk);
                } else {
                    if (hit == 7) hit = 1;
                    if (hit == 2) {
                        FillRect(hdcMem, &wallRc, side == 1 ? (tex ? e2s : e2) : (tex ? e1s : e1));
                    } else if (hit == 3) {
                        FillRect(hdcMem, &wallRc, side == 1 ? (tex ? k2s : k2) : (tex ? k1s : k1));
                    } else if (hit == 4) {
                        FillRect(hdcMem, &wallRc, side == 1 ? (tex ? d2s : d2) : (tex ? d1s : d1));
                    } else if (hit == 5) {
                        FillRect(hdcMem, &wallRc, side == 1 ? (tex ? c2s : c2) : (tex ? c1s : c1));
                    } else if (hit == 6) {
                        FillRect(hdcMem, &wallRc, side == 1 ? (tex ? t2s : t2) : (tex ? t1s : t1));
                    } else if (hit == 8) {
                        FillRect(hdcMem, &wallRc, side == 1 ? (tex ? m2s : m2) : (tex ? m1s : m1));
                    } else if (hit == 9) {
                        FillRect(hdcMem, &wallRc, side == 1 ? (tex ? p2s : p2) : (tex ? p1s : p1));
                    } else if (hit == 10 || hit == 11) {
                        FillRect(hdcMem, &wallRc, side == 1 ? (tex ? tp2s : tp2) : (tex ? tp1s : tp1));
                    } else if (hit == 12) {
                        FillRect(hdcMem, &wallRc, side == 1 ? (tex ? min2s : min2) : (tex ? min1s : min1));
                    } else if (hit == 13) {
                        FillRect(hdcMem, &wallRc, side == 1 ? (tex ? pik2s : pik2) : (tex ? pik1s : pik1));
                    } else {
                        FillRect(hdcMem, &wallRc, side == 1 ? (tex ? w2s : w2) : (tex ? w1s : w1));
                    }
                }
            }
            DeleteObject(w1); DeleteObject(w2); DeleteObject(e1); DeleteObject(e2); DeleteObject(k1); DeleteObject(k2); DeleteObject(d1); DeleteObject(d2); DeleteObject(c1); DeleteObject(c2);
            DeleteObject(t1); DeleteObject(t2); DeleteObject(m1); DeleteObject(m2);
            DeleteObject(p1); DeleteObject(p2); DeleteObject(tp1); DeleteObject(tp2);
            DeleteObject(w1s); DeleteObject(w2s); DeleteObject(e1s); DeleteObject(e2s); DeleteObject(k1s); DeleteObject(k2s); DeleteObject(d1s); DeleteObject(d2s); DeleteObject(c1s); DeleteObject(c2s);
            DeleteObject(t1s); DeleteObject(t2s); DeleteObject(m1s); DeleteObject(m2s);
            DeleteObject(p1s); DeleteObject(p2s); DeleteObject(tp1s); DeleteObject(tp2s);
            DeleteObject(min1); DeleteObject(min2); DeleteObject(min1s); DeleteObject(min2s);
            DeleteObject(pik1); DeleteObject(pik2); DeleteObject(pik1s); DeleteObject(pik2s);
            
            // Draw UI
            char uiText[128];
            if (gameState == 0) {
                sprintf(uiText, "KMAZE - Press ENTER to start  [Played:%d Escaped:%d]", totalGames, totalEscapes);
            } else if (gameState == 2) {
                float elapsed = (endTime - startTime) / 1000.0f;
                sprintf(uiText, "You Escaped! Score: %d Time: %.1fs (ENTER restart)", score, elapsed);
            } else {
                float elapsed = (GetTickCount() - startTime) / 1000.0f;
                sprintf(uiText, "Score: %d  Keys: %d  Pick: %d  Lvl: %d  Time: %.1f", score, keysHeld, hasPickaxe, currentLevel + 1, elapsed);
            }
            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(255, 255, 255));
            TextOutA(hdcMem, 10, 10, uiText, lstrlenA(uiText));

            if (msgTimer > 0) {
                SetTextColor(hdcMem, RGB(255, 255, 0));
                TextOutA(hdcMem, W/2 - 40, 30, msgText, lstrlenA(msgText));
            }

            // Minimap
            if (gameState == 1 && hasCompass) {
                int mmW = 0, mmH = 0;
                if (currentLevel >= 10) { mmW = curRandW; mmH = curRandH; }
                else if (currentLevel == 0 || currentLevel == 3) { mmW = 10; mmH = 10; }
                else if (currentLevel == 1 || currentLevel == 4 || currentLevel == 5 || currentLevel == 7 || currentLevel == 8) { mmW = 12; mmH = 12; }
                else if (currentLevel == 2 || currentLevel == 6 || currentLevel == 9) { mmW = 15; mmH = 15; }
                
                if (mmW > 0) {
                    int mmS = 5;
                    if (mmW > 15) mmS = 4;
                    if (mmW > 23) mmS = 3;
                    int mmX = W - 10 - mmW * mmS;
                    int mmY = 10;
                    HBRUSH mWall = CreateSolidBrush(RGB(153, 153, 153));
                    HBRUSH mExit = CreateSolidBrush(RGB(0, 255, 0));
                    HBRUSH mKey = CreateSolidBrush(RGB(255, 255, 0));
                    HBRUSH mDoor = CreateSolidBrush(RGB(0, 0, 255));
                    HBRUSH mFloor = CreateSolidBrush(RGB(34, 34, 34));
                    HBRUSH mPlayer = CreateSolidBrush(RGB(255, 0, 0));
                    HBRUSH mCoin = CreateSolidBrush(RGB(255, 128, 0));
                    HBRUSH mTrap = CreateSolidBrush(RGB(153, 0, 153));
                    HBRUSH mFake = CreateSolidBrush(RGB(153, 153, 153));
                    HBRUSH mComp = CreateSolidBrush(RGB(0, 255, 255));
                    HBRUSH mSpeed = CreateSolidBrush(RGB(255, 255, 255));
                    HBRUSH mTele = CreateSolidBrush(RGB(255, 0, 255));
                    
                    for (int i = 0; i < mmW; i++) {
                        for (int j = 0; j < mmH; j++) {
                            int v = GetMapValue(i, j);
                            HBRUSH b = mFloor;
                            if (v == 1 || v == 7) b = mWall;
                            else if (v == 2) b = mExit;
                            else if (v == 3) b = mKey;
                            else if (v == 4) b = mDoor;
                            else if (v == 5) b = mCoin;
                            else if (v == 6) b = mTrap;
                            else if (v == 8) b = mComp;
                            else if (v == 9) b = mSpeed;
                            else if (v == 10 || v == 11) b = mTele;
                            else if (v == 12) b = CreateSolidBrush(RGB(255, 50, 50));
                            else if (v == 13) b = CreateSolidBrush(RGB(150, 75, 0));
                            
                            RECT mr = {mmX + i*mmS, mmY + j*mmS, mmX + i*mmS + mmS, mmY + j*mmS + mmS};
                            FillRect(hdcMem, &mr, b);
                        }
                    }
                    RECT mr = {mmX + (int)pX*mmS, mmY + (int)pY*mmS, mmX + (int)pX*mmS + mmS, mmY + (int)pY*mmS + mmS};
                    FillRect(hdcMem, &mr, mPlayer);
                    
                    DeleteObject(mWall); DeleteObject(mExit); DeleteObject(mKey); DeleteObject(mDoor); DeleteObject(mFloor); DeleteObject(mPlayer); DeleteObject(mCoin);
                    DeleteObject(mTrap); DeleteObject(mFake); DeleteObject(mComp); DeleteObject(mSpeed); DeleteObject(mTele);
                }
            }

            StretchBlt(hdc, 0, 0, 640, 480, hdcMem, 0, 0, W, H, SRCCOPY);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            if (hdcMem) DeleteDC(hdcMem);
            if (hbmCanvas) DeleteObject(hbmCanvas);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KMazeClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    
    RegisterClassA(&wc);
    
    RECT wr = {0, 0, 640, 480};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    
    HWND hwnd = CreateWindowExA(0, "KMazeClass", "KMaze", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
