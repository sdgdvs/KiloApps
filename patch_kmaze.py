import os
import json

# --- UPDATE C VERSION ---
with open('d:/KiloApps/KMaze/main.c', 'r') as f:
    c_code = f.read()

maps_addition = '''
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
'''
c_code = c_code.replace(
'''int map1[10][10];
int map2[12][12];
int map3[15][15];
int map4[10][10];
int map5[12][12];''', maps_addition)

c_code = c_code.replace('''    memcpy(map5, orig_map5, sizeof(map5));
}''', '''    memcpy(map5, orig_map5, sizeof(map5));
    memcpy(map6, orig_map6, sizeof(map6));
    memcpy(map7, orig_map7, sizeof(map7));
    memcpy(map8, orig_map8, sizeof(map8));
    memcpy(map9, orig_map9, sizeof(map9));
    memcpy(map10, orig_map10, sizeof(map10));
}''')

c_code = c_code.replace('''    for(int i=0; i<w*h/10; i++) {
        int rx = 1 + rand()%(w-2);
        int ry = 1 + rand()%(h-2);
        if (mapRandom[rx][ry] == 0 && (rx != 1 || ry != 1) && (rx != farX || ry != farY)) {
            mapRandom[rx][ry] = 5;
        }
    }
    mapRandom[farX][farY] = 2;
    curRandW = w;
    curRandH = h;
}''', '''    for(int i=0; i<w*h/10; i++) {
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
    mapRandom[farX][farY] = 2;
    curRandW = w;
    curRandH = h;
}''')

c_code = c_code.replace('''int keysHeld = 0;

int gameState = 0;''', '''int keysHeld = 0;
int hasCompass = 0;

int gameState = 0;''')

get_map_old = '''    } else if (currentLevel == 4) {
        if (x >= 12 || y >= 12) return 1;
        return map5[x][y];
    } else {'''
get_map_new = '''    } else if (currentLevel == 4) {
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
    } else {'''
c_code = c_code.replace(get_map_old, get_map_new)

set_map_old = '''    else if (currentLevel == 4 && x < 12 && y < 12) map5[x][y] = v;
    else if (currentLevel >= 5 && x < curRandW && y < curRandH) mapRandom[x][y] = v;'''
set_map_new = '''    else if (currentLevel == 4 && x < 12 && y < 12) map5[x][y] = v;
    else if (currentLevel == 5 && x < 12 && y < 12) map6[x][y] = v;
    else if (currentLevel == 6 && x < 15 && y < 15) map7[x][y] = v;
    else if (currentLevel == 7 && x < 12 && y < 12) map8[x][y] = v;
    else if (currentLevel == 8 && x < 12 && y < 12) map9[x][y] = v;
    else if (currentLevel == 9 && x < 15 && y < 15) map10[x][y] = v;
    else if (currentLevel >= 10 && x < curRandW && y < curRandH) mapRandom[x][y] = v;'''
c_code = c_code.replace(set_map_old, set_map_new)

c_code = c_code.replace('''void NextLevel() {
    keysHeld = 0;
    currentLevel++;
    if (currentLevel > 9) {''', '''void NextLevel() {
    keysHeld = 0;
    currentLevel++;
    hasCompass = (currentLevel < 6) ? 1 : 0;
    if (currentLevel > 14) {''')

c_code = c_code.replace('''    if (currentLevel >= 5) {
        int s = 11 + (currentLevel - 5) * 4;
        if (s > 31) s = 31;
        GenerateMaze(s, s);
    }''', '''    if (currentLevel >= 10) {
        int s = 11 + (currentLevel - 10) * 4;
        if (s > 31) s = 31;
        GenerateMaze(s, s);
    }''')

c_code = c_code.replace('''            int TryMove(int x, int y) {
                int val = GetMapValue(x, y);
                if (val == 0 || val == 2 || val == 3 || val == 5) return 1;''', '''            int TryMove(int x, int y) {
                int val = GetMapValue(x, y);
                if (val == 0 || val == 2 || val == 3 || val == 5 || val == 6 || val == 7 || val == 8) return 1;''')

move_logic_old = '''            int curVal = GetMapValue((int)pX, (int)pY);
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
            }'''
move_logic_new = '''            int curVal = GetMapValue((int)pX, (int)pY);
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
            }'''
c_code = c_code.replace(move_logic_old, move_logic_new)

brushes_old = '''            HBRUSH c1 = CreateSolidBrush(RGB(255, 128, 0));
            HBRUSH c2 = CreateSolidBrush(RGB(204, 102, 0));
            
            HBRUSH w1s = CreateSolidBrush(RGB(170, 0, 0));'''
brushes_new = '''            HBRUSH c1 = CreateSolidBrush(RGB(255, 128, 0));
            HBRUSH c2 = CreateSolidBrush(RGB(204, 102, 0));
            
            HBRUSH t1 = CreateSolidBrush(RGB(153, 0, 153));
            HBRUSH t2 = CreateSolidBrush(RGB(102, 0, 102));
            HBRUSH m1 = CreateSolidBrush(RGB(0, 255, 255));
            HBRUSH m2 = CreateSolidBrush(RGB(0, 204, 204));
            
            HBRUSH w1s = CreateSolidBrush(RGB(170, 0, 0));'''
c_code = c_code.replace(brushes_old, brushes_new)

brushes2_old = '''            HBRUSH c1s = CreateSolidBrush(RGB(204, 102, 0));
            HBRUSH c2s = CreateSolidBrush(RGB(153, 76, 0));
            
            for (int x = 0; x < W; x++) {'''
brushes2_new = '''            HBRUSH c1s = CreateSolidBrush(RGB(204, 102, 0));
            HBRUSH c2s = CreateSolidBrush(RGB(153, 76, 0));
            HBRUSH t1s = CreateSolidBrush(RGB(102, 0, 102));
            HBRUSH t2s = CreateSolidBrush(RGB(51, 0, 51));
            HBRUSH m1s = CreateSolidBrush(RGB(0, 204, 204));
            HBRUSH m2s = CreateSolidBrush(RGB(0, 153, 153));
            
            for (int x = 0; x < W; x++) {'''
c_code = c_code.replace(brushes2_old, brushes2_new)

render_old = '''                RECT wallRc = {x, drawStart, x+1, drawEnd};
                if (hit == 2) {
                    FillRect(hdcMem, &wallRc, side == 1 ? (tex ? e2s : e2) : (tex ? e1s : e1));
                } else if (hit == 3) {
                    FillRect(hdcMem, &wallRc, side == 1 ? (tex ? k2s : k2) : (tex ? k1s : k1));
                } else if (hit == 4) {
                    FillRect(hdcMem, &wallRc, side == 1 ? (tex ? d2s : d2) : (tex ? d1s : d1));
                } else if (hit == 5) {
                    FillRect(hdcMem, &wallRc, side == 1 ? (tex ? c2s : c2) : (tex ? c1s : c1));
                } else {
                    FillRect(hdcMem, &wallRc, side == 1 ? (tex ? w2s : w2) : (tex ? w1s : w1));
                }
            }
            DeleteObject(w1); DeleteObject(w2); DeleteObject(e1); DeleteObject(e2); DeleteObject(k1); DeleteObject(k2); DeleteObject(d1); DeleteObject(d2); DeleteObject(c1); DeleteObject(c2);
            DeleteObject(w1s); DeleteObject(w2s); DeleteObject(e1s); DeleteObject(e2s); DeleteObject(k1s); DeleteObject(k2s); DeleteObject(d1s); DeleteObject(d2s); DeleteObject(c1s); DeleteObject(c2s);'''
render_new = '''                RECT wallRc = {x, drawStart, x+1, drawEnd};
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
                } else {
                    FillRect(hdcMem, &wallRc, side == 1 ? (tex ? w2s : w2) : (tex ? w1s : w1));
                }
            }
            DeleteObject(w1); DeleteObject(w2); DeleteObject(e1); DeleteObject(e2); DeleteObject(k1); DeleteObject(k2); DeleteObject(d1); DeleteObject(d2); DeleteObject(c1); DeleteObject(c2);
            DeleteObject(t1); DeleteObject(t2); DeleteObject(m1); DeleteObject(m2);
            DeleteObject(w1s); DeleteObject(w2s); DeleteObject(e1s); DeleteObject(e2s); DeleteObject(k1s); DeleteObject(k2s); DeleteObject(d1s); DeleteObject(d2s); DeleteObject(c1s); DeleteObject(c2s);
            DeleteObject(t1s); DeleteObject(t2s); DeleteObject(m1s); DeleteObject(m2s);'''
c_code = c_code.replace(render_old, render_new)

minimap_old = '''            // Minimap
            if (gameState == 1) {
                int mmW = 0, mmH = 0;
                if (currentLevel >= 5) { mmW = curRandW; mmH = curRandH; }
                else if (currentLevel == 0 || currentLevel == 3) { mmW = 10; mmH = 10; }
                else if (currentLevel == 1 || currentLevel == 4) { mmW = 12; mmH = 12; }
                else if (currentLevel == 2) { mmW = 15; mmH = 15; }'''
minimap_new = '''            // Minimap
            if (gameState == 1 && hasCompass) {
                int mmW = 0, mmH = 0;
                if (currentLevel >= 10) { mmW = curRandW; mmH = curRandH; }
                else if (currentLevel == 0 || currentLevel == 3) { mmW = 10; mmH = 10; }
                else if (currentLevel == 1 || currentLevel == 4 || currentLevel == 5 || currentLevel == 7 || currentLevel == 8) { mmW = 12; mmH = 12; }
                else if (currentLevel == 2 || currentLevel == 6 || currentLevel == 9) { mmW = 15; mmH = 15; }'''
c_code = c_code.replace(minimap_old, minimap_new)

minimap_colors_old = '''                    HBRUSH mPlayer = CreateSolidBrush(RGB(255, 0, 0));
                    HBRUSH mCoin = CreateSolidBrush(RGB(255, 128, 0));
                    
                    for (int i = 0; i < mmW; i++) {
                        for (int j = 0; j < mmH; j++) {
                            int v = GetMapValue(i, j);
                            HBRUSH b = mFloor;
                            if (v == 1) b = mWall;
                            else if (v == 2) b = mExit;
                            else if (v == 3) b = mKey;
                            else if (v == 4) b = mDoor;
                            else if (v == 5) b = mCoin;
                            
                            RECT mr = {mmX + i*mmS, mmY + j*mmS, mmX + i*mmS + mmS, mmY + j*mmS + mmS};
                            FillRect(hdcMem, &mr, b);
                        }
                    }
                    RECT mr = {mmX + (int)pX*mmS, mmY + (int)pY*mmS, mmX + (int)pX*mmS + mmS, mmY + (int)pY*mmS + mmS};
                    FillRect(hdcMem, &mr, mPlayer);
                    
                    DeleteObject(mWall); DeleteObject(mExit); DeleteObject(mKey); DeleteObject(mDoor); DeleteObject(mFloor); DeleteObject(mPlayer); DeleteObject(mCoin);'''
minimap_colors_new = '''                    HBRUSH mPlayer = CreateSolidBrush(RGB(255, 0, 0));
                    HBRUSH mCoin = CreateSolidBrush(RGB(255, 128, 0));
                    HBRUSH mTrap = CreateSolidBrush(RGB(153, 0, 153));
                    HBRUSH mFake = CreateSolidBrush(RGB(153, 153, 153));
                    HBRUSH mComp = CreateSolidBrush(RGB(0, 255, 255));
                    
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
                            
                            RECT mr = {mmX + i*mmS, mmY + j*mmS, mmX + i*mmS + mmS, mmY + j*mmS + mmS};
                            FillRect(hdcMem, &mr, b);
                        }
                    }
                    RECT mr = {mmX + (int)pX*mmS, mmY + (int)pY*mmS, mmX + (int)pX*mmS + mmS, mmY + (int)pY*mmS + mmS};
                    FillRect(hdcMem, &mr, mPlayer);
                    
                    DeleteObject(mWall); DeleteObject(mExit); DeleteObject(mKey); DeleteObject(mDoor); DeleteObject(mFloor); DeleteObject(mPlayer); DeleteObject(mCoin);
                    DeleteObject(mTrap); DeleteObject(mFake); DeleteObject(mComp);'''
c_code = c_code.replace(minimap_colors_old, minimap_colors_new)

with open('d:/KiloApps/KMaze/main.c', 'w') as f:
    f.write(c_code)

# --- UPDATE HTML VERSION ---
with open('d:/KiloApps/KiloOS/public/apps/kmaze.html', 'r') as f:
    h_code = f.read()

new_maps_js = '''    [
      [1,1,1,1,1,1,1,1,1,1,1,1],
      [1,0,0,6,0,0,0,1,5,0,0,1],
      [1,0,1,1,1,1,0,1,1,1,0,1],
      [1,0,1,5,0,1,0,6,0,1,0,1],
      [1,0,1,0,0,1,0,1,0,1,0,1],
      [1,0,1,1,4,1,1,1,0,1,0,1],
      [1,0,0,0,0,0,0,0,0,1,0,1],
      [1,1,1,1,1,1,1,1,0,1,0,1],
      [1,3,6,0,5,0,0,1,0,1,0,1],
      [1,0,1,1,1,1,0,1,6,1,0,1],
      [1,0,0,0,0,1,0,0,0,1,2,1],
      [1,1,1,1,1,1,1,1,1,1,1,1]
    ],
    [
      [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
      [1,0,1,0,0,0,1,0,0,0,1,0,0,8,1],
      [1,0,1,0,1,0,1,0,1,0,1,0,1,1,1],
      [1,0,1,0,1,0,1,0,1,0,1,0,0,0,1],
      [1,0,1,0,1,0,1,0,1,0,1,1,1,0,1],
      [1,0,1,0,1,0,1,0,1,0,1,5,0,0,1],
      [1,0,1,0,1,0,1,0,1,0,1,0,1,1,1],
      [1,0,1,0,1,0,1,0,1,0,1,0,1,3,1],
      [1,0,1,0,1,0,1,0,1,0,1,0,1,1,1],
      [1,0,1,0,1,0,1,0,1,0,1,0,0,0,1],
      [1,0,1,0,1,0,1,0,1,0,1,1,1,0,1],
      [1,0,0,0,1,0,0,0,1,0,0,0,4,0,1],
      [1,1,1,1,1,1,1,1,1,1,1,1,1,0,1],
      [1,2,0,0,0,0,0,0,0,0,0,0,0,0,1],
      [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
    ],
    [
      [1,1,1,1,1,1,1,1,1,1,1,1],
      [1,0,0,1,2,1,0,0,0,5,0,1],
      [1,1,0,1,4,1,0,1,1,1,1,1],
      [1,0,0,1,0,1,0,1,3,0,0,1],
      [1,0,1,1,7,1,1,1,1,1,0,1],
      [1,0,0,0,0,0,0,1,5,0,0,1],
      [1,1,1,1,1,1,0,1,1,1,1,1],
      [1,5,0,0,0,1,0,1,0,0,0,1],
      [1,1,1,1,0,1,0,1,0,1,0,1],
      [1,8,0,1,0,7,0,7,0,1,0,1],
      [1,1,0,1,1,1,1,1,1,1,0,1],
      [1,1,1,1,1,1,1,1,1,1,1,1]
    ],
    [
      [1,1,1,1,1,1,1,1,1,1,1,1],
      [1,0,0,6,0,0,1,2,1,0,3,1],
      [1,0,1,1,1,0,1,4,1,0,1,1],
      [1,0,1,8,1,0,1,0,1,0,0,1],
      [1,0,1,1,1,0,1,0,1,1,0,1],
      [1,0,0,0,6,0,7,0,6,0,0,1],
      [1,1,1,1,1,1,1,0,1,1,1,1],
      [1,5,0,0,0,0,1,0,1,5,0,1],
      [1,0,1,1,1,0,1,0,1,0,1,1],
      [1,0,1,6,1,0,1,0,7,0,0,1],
      [1,0,0,0,1,5,1,0,1,1,0,1],
      [1,1,1,1,1,1,1,1,1,1,1,1]
    ],
    [
      [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
      [1,0,0,0,1,5,0,6,0,0,0,1,8,0,1],
      [1,0,1,0,1,1,1,1,1,1,0,1,1,0,1],
      [1,0,1,0,7,0,0,5,0,1,0,7,0,0,1],
      [1,0,1,1,1,1,1,1,0,1,1,1,1,1,1],
      [1,0,1,3,0,0,0,1,0,0,0,6,0,0,1],
      [1,0,1,1,1,1,0,1,1,1,1,1,1,0,1],
      [1,0,0,6,0,1,0,4,0,0,0,1,0,0,1],
      [1,1,1,1,0,1,1,1,1,1,0,1,0,1,1],
      [1,5,0,1,0,0,0,0,0,1,0,1,0,0,1],
      [1,0,1,1,1,1,1,1,0,1,0,1,1,0,1],
      [1,0,1,2,1,0,0,1,0,1,0,1,5,0,1],
      [1,0,1,4,1,1,0,1,0,1,0,1,1,1,1],
      [1,0,0,0,6,0,0,1,5,1,0,0,0,0,1],
      [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
    ]
  ];'''
h_code = h_code.replace('''    [
      [1,1,1,1,1,1,1,1,1,1,1,1],
      [1,0,0,0,1,0,0,0,0,0,3,1],
      [1,0,1,0,1,0,1,1,1,1,1,1],
      [1,0,1,0,1,0,0,0,0,0,0,1],
      [1,0,1,0,1,1,1,1,1,1,0,1],
      [1,0,1,0,0,0,0,0,0,1,0,1],
      [1,0,1,1,1,1,1,1,0,1,0,1],
      [1,0,0,0,0,0,0,1,0,1,0,1],
      [1,1,1,1,1,1,0,1,0,1,0,1],
      [1,4,0,0,0,1,0,0,0,1,0,1],
      [1,2,1,1,1,1,1,1,1,1,1,1],
      [1,1,1,1,1,1,1,1,1,1,1,1]
    ]
  ];''', '''    [
      [1,1,1,1,1,1,1,1,1,1,1,1],
      [1,0,0,0,1,0,0,0,0,0,3,1],
      [1,0,1,0,1,0,1,1,1,1,1,1],
      [1,0,1,0,1,0,0,0,0,0,0,1],
      [1,0,1,0,1,1,1,1,1,1,0,1],
      [1,0,1,0,0,0,0,0,0,1,0,1],
      [1,0,1,1,1,1,1,1,0,1,0,1],
      [1,0,0,0,0,0,0,1,0,1,0,1],
      [1,1,1,1,1,1,0,1,0,1,0,1],
      [1,4,0,0,0,1,0,0,0,1,0,1],
      [1,2,1,1,1,1,1,1,1,1,1,1],
      [1,1,1,1,1,1,1,1,1,1,1,1]
    ],
''' + new_maps_js)

h_code = h_code.replace('''  let currentLevel = 0;
  let keysHeld = 0;''', '''  let currentLevel = 0;
  let keysHeld = 0;
  let hasCompass = 0;''')

h_code = h_code.replace('''  function generateMaze(w, h) {
      let m = Array.from({length: w}, () => Array(h).fill(1));''', '''  function generateMaze(w, h) {
      let m = Array.from({length: w}, () => Array(h).fill(1));''')

gen_maze_old = '''      for(let i=0; i < (w*h)/10; i++) {
          let rx = 1 + Math.floor(Math.random()*(w-2));
          let ry = 1 + Math.floor(Math.random()*(h-2));
          if (m[rx][ry] === 0 && (rx !== 1 || ry !== 1) && (rx !== farX || ry !== farY)) {
              m[rx][ry] = 5;
          }
      }
      m[farX][farY] = 2;
      return m;
  }'''
gen_maze_new = '''      for(let i=0; i < (w*h)/10; i++) {
          let rx = 1 + Math.floor(Math.random()*(w-2));
          let ry = 1 + Math.floor(Math.random()*(h-2));
          if (m[rx][ry] === 0 && (rx !== 1 || ry !== 1) && (rx !== farX || ry !== farY)) {
              m[rx][ry] = 5;
          }
      }
      let placedCompass = false;
      while(!placedCompass) {
          let rx = 1 + Math.floor(Math.random()*(w-2));
          let ry = 1 + Math.floor(Math.random()*(h-2));
          if (m[rx][ry] === 0) {
              m[rx][ry] = 8;
              placedCompass = true;
          }
      }
      for(let i=0; i < (w*h)/20; i++) {
          let rx = 1 + Math.floor(Math.random()*(w-2));
          let ry = 1 + Math.floor(Math.random()*(h-2));
          if (m[rx][ry] === 0) m[rx][ry] = 6;
      }
      for(let i=0; i < (w*h)/20; i++) {
          let rx = 1 + Math.floor(Math.random()*(w-2));
          let ry = 1 + Math.floor(Math.random()*(h-2));
          if (m[rx][ry] === 1 && rx > 1 && rx < w-2 && ry > 1 && ry < h-2) m[rx][ry] = 7;
      }
      m[farX][farY] = 2;
      return m;
  }'''
h_code = h_code.replace(gen_maze_old, gen_maze_new)

h_code = h_code.replace('''  function nextLevel() {
      currentLevel++;
      if (currentLevel > 9) {''', '''  function nextLevel() {
      currentLevel++;
      hasCompass = currentLevel < 6 ? 1 : 0;
      if (currentLevel > 14) {''')

h_code = h_code.replace('''      let tryMove = (mx, my) => {
          let val = map[Math.floor(mx)] && map[Math.floor(mx)][Math.floor(my)];
          if (val === 0 || val === 2 || val === 3 || val === 5) return true;''', '''      let tryMove = (mx, my) => {
          let val = map[Math.floor(mx)] && map[Math.floor(mx)][Math.floor(my)];
          if ([0,2,3,5,6,7,8].includes(val)) return true;''')

move_old = '''      let curVal = map[Math.floor(pX)] && map[Math.floor(pX)][Math.floor(pY)];
      if (curVal === 3) {
          keysHeld++;
          map[Math.floor(pX)][Math.floor(pY)] = 0;
          playTone(600, 'square', 0.1);
      } else if (curVal === 2) {
          playTone(800, 'square', 0.2);
          nextLevel();
      } else if (curVal === 5) {
          score += 100;
          map[Math.floor(pX)][Math.floor(pY)] = 0;
          playTone(500, 'square', 0.1);
      }'''
move_new = '''      let curVal = map[Math.floor(pX)] && map[Math.floor(pX)][Math.floor(pY)];
      if (curVal === 3) {
          keysHeld++;
          map[Math.floor(pX)][Math.floor(pY)] = 0;
          playTone(600, 'square', 0.1);
      } else if (curVal === 2) {
          playTone(800, 'square', 0.2);
          nextLevel();
      } else if (curVal === 5) {
          score += 100;
          map[Math.floor(pX)][Math.floor(pY)] = 0;
          playTone(500, 'square', 0.1);
      } else if (curVal === 6) {
          playTone(200, 'sawtooth', 0.2);
          score = Math.max(0, score - 50);
          pX = 1.5; pY = 1.5;
      } else if (curVal === 7) {
          map[Math.floor(pX)][Math.floor(pY)] = 0;
          playTone(400, 'square', 0.1);
      } else if (curVal === 8) {
          hasCompass = 1;
          map[Math.floor(pX)][Math.floor(pY)] = 0;
          playTone(700, 'square', 0.2);
      }'''
h_code = h_code.replace(move_old, move_new)

draw_old = '''          if (hit === 2) {
              ctx.fillStyle = side === 1 ? (tex ? '#009900' : '#00cc00') : (tex ? '#00cc00' : '#00ff00');
          } else if (hit === 3) {
              ctx.fillStyle = side === 1 ? (tex ? '#996600' : '#cc9900') : (tex ? '#cc9900' : '#ffcc00');
          } else if (hit === 4) {
              ctx.fillStyle = side === 1 ? (tex ? '#003399' : '#0066cc') : (tex ? '#0066cc' : '#0099ff');
          } else if (hit === 5) {
              ctx.fillStyle = side === 1 ? (tex ? '#994c00' : '#cc6600') : (tex ? '#cc6600' : '#ff8000');
          } else {
              ctx.fillStyle = side === 1 ? (tex ? '#770000' : '#960000') : (tex ? '#aa0000' : '#c80000');
          }'''
draw_new = '''          if (hit === 7) hit = 1;
          if (hit === 2) {
              ctx.fillStyle = side === 1 ? (tex ? '#009900' : '#00cc00') : (tex ? '#00cc00' : '#00ff00');
          } else if (hit === 3) {
              ctx.fillStyle = side === 1 ? (tex ? '#996600' : '#cc9900') : (tex ? '#cc9900' : '#ffcc00');
          } else if (hit === 4) {
              ctx.fillStyle = side === 1 ? (tex ? '#003399' : '#0066cc') : (tex ? '#0066cc' : '#0099ff');
          } else if (hit === 5) {
              ctx.fillStyle = side === 1 ? (tex ? '#994c00' : '#cc6600') : (tex ? '#cc6600' : '#ff8000');
          } else if (hit === 6) {
              ctx.fillStyle = side === 1 ? (tex ? '#660066' : '#990099') : (tex ? '#990099' : '#cc00cc');
          } else if (hit === 8) {
              ctx.fillStyle = side === 1 ? (tex ? '#009999' : '#00cccc') : (tex ? '#00cccc' : '#00ffff');
          } else {
              ctx.fillStyle = side === 1 ? (tex ? '#770000' : '#960000') : (tex ? '#aa0000' : '#c80000');
          }'''
h_code = h_code.replace(draw_old, draw_new)

mm_old = '''          // Minimap
          let mmW = map.length;
          let mmH = map[0].length;
          let mmS = 5;
          if (mmW > 15) mmS = 4;
          if (mmW > 23) mmS = 3;
          const mmX = W - 10 - mmW * mmS;
          const mmY = 10;
          for (let i = 0; i < mmW; i++) {
              for (let j = 0; j < mmH; j++) {
                  let v = map[i][j];
                  if (v === 1) ctx.fillStyle = '#999';
                  else if (v === 2) ctx.fillStyle = '#0f0';
                  else if (v === 3) ctx.fillStyle = '#ff0';
                  else if (v === 4) ctx.fillStyle = '#00f';
                  else if (v === 5) ctx.fillStyle = '#f80';
                  else ctx.fillStyle = '#222';
                  ctx.fillRect(mmX + i * mmS, mmY + j * mmS, mmS, mmS);
              }
          }
          ctx.fillStyle = '#f00';
          ctx.fillRect(mmX + Math.floor(pX) * mmS, mmY + Math.floor(pY) * mmS, mmS, mmS);'''
mm_new = '''          // Minimap
          if (hasCompass) {
              let mmW = map.length;
              let mmH = map[0].length;
              let mmS = 5;
              if (mmW > 15) mmS = 4;
              if (mmW > 23) mmS = 3;
              const mmX = W - 10 - mmW * mmS;
              const mmY = 10;
              for (let i = 0; i < mmW; i++) {
                  for (let j = 0; j < mmH; j++) {
                      let v = map[i][j];
                      if (v === 1 || v === 7) ctx.fillStyle = '#999';
                      else if (v === 2) ctx.fillStyle = '#0f0';
                      else if (v === 3) ctx.fillStyle = '#ff0';
                      else if (v === 4) ctx.fillStyle = '#00f';
                      else if (v === 5) ctx.fillStyle = '#f80';
                      else if (v === 6) ctx.fillStyle = '#909';
                      else if (v === 8) ctx.fillStyle = '#0ff';
                      else ctx.fillStyle = '#222';
                      ctx.fillRect(mmX + i * mmS, mmY + j * mmS, mmS, mmS);
                  }
              }
              ctx.fillStyle = '#f00';
              ctx.fillRect(mmX + Math.floor(pX) * mmS, mmY + Math.floor(pY) * mmS, mmS, mmS);
          }'''
h_code = h_code.replace(mm_old, mm_new)

with open('d:/KiloApps/KiloOS/public/apps/kmaze.html', 'w') as f:
    f.write(h_code)
