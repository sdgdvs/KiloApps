#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define MAX_GRID_SIZE 25
#define CELL_SIZE 30
#define MAX_WORDS 20
#define NUM_THEMES 4
#define THEME_DICT_SIZE 30

const char* THEMES[NUM_THEMES] = {"Programming", "Animals", "Countries", "Space"};

const char* DICTIONARIES[NUM_THEMES][THEME_DICT_SIZE] = {
    { // Programming
        "ALGORITHM", "COMPILER", "DEBUG", "FUNCTION", "VARIABLE", 
        "POINTER", "SYNTAX", "OBJECT", "CLASS", "METHOD", 
        "ARRAY", "STRING", "BOOLEAN", "INTEGER", "FLOAT",
        "NETWORK", "SERVER", "DATABASE", "CLIENT", "PROTOCOL", 
        "ROUTER", "BROWSER", "KERNEL", "MEMORY", "THREAD", 
        "PROCESS", "SOCKET", "PACKET", "CACHE", "FRAMEWORK"
    },
    { // Animals
        "ELEPHANT", "GIRAFFE", "PENGUIN", "KANGAROO", "DOLPHIN",
        "TIGER", "CHEETAH", "MONKEY", "OSTRICH", "IGUANA",
        "ZEBRA", "GORILLA", "PANTHER", "LEOPARD", "HIPPO",
        "RHINO", "CROCODILE", "ALLIGATOR", "CHIMPANZEE", "SNAIL",
        "OCTOPUS", "SHARK", "WHALE", "WALRUS", "SEAL",
        "BEAR", "WOLF", "FOX", "RABBIT", "DEER"
    },
    { // Countries
        "CANADA", "BRAZIL", "JAPAN", "FRANCE", "GERMANY",
        "ITALY", "SPAIN", "INDIA", "CHINA", "RUSSIA",
        "AUSTRALIA", "MEXICO", "ARGENTINA", "CHILE", "PERU",
        "EGYPT", "MOROCCO", "NIGERIA", "KENYA", "SWEDEN",
        "NORWAY", "FINLAND", "DENMARK", "IRELAND", "POLAND",
        "GREECE", "TURKEY", "THAILAND", "VIETNAM", "INDONESIA"
    },
    { // Space
        "ASTEROID", "COMET", "GALAXY", "NEBULA", "PLANET",
        "STAR", "ORBIT", "SATELLITE", "ROCKET", "GRAVITY",
        "ECLIPSE", "METEOR", "UNIVERSE", "COSMOS", "PULSAR",
        "QUASAR", "SUPERNOVA", "VACUUM", "EQUATOR", "HORIZON",
        "ZENITH", "LUNAR", "SOLAR", "TELESCOPE", "ASTRONAUT",
        "SPACECRAFT", "OBSERVATORY", "CONSTELLATION", "ZODIAC", "APOLLO"
    }
};

char grid[MAX_GRID_SIZE][MAX_GRID_SIZE];
bool foundGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
bool hintedGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
int gridSize = 15;
int numWordsToFind = 8;
int currentDifficulty = 1; // 0=Easy, 1=Medium, 2=Hard
int currentThemeIdx = 0;

char wordsToFind[MAX_WORDS][32];
bool wordsFoundStatus[MAX_WORDS];
bool wordsHintedStatus[MAX_WORDS];
int wordCount = 0;
int foundCount = 0;
int currentScore = 0;

bool isSelecting = false;
int startR = -1, startC = -1;
int curR = -1, curC = -1;

float cellAnim[MAX_GRID_SIZE][MAX_GRID_SIZE] = {0};
float strikeAnim[MAX_WORDS] = {0};

int timerSeconds = 0;
bool gameWon = false;

typedef struct {
    int completed;
    int bestTimes[NUM_THEMES][3];
} Stats;

Stats gameStats = {0};

void LoadStats() {
    FILE* fp = fopen("kwords_stats.dat", "rb");
    if (fp) {
        fread(&gameStats, sizeof(Stats), 1, fp);
        fclose(fp);
    } else {
        memset(&gameStats, 0, sizeof(Stats));
    }
}

void SaveStats() {
    FILE* fp = fopen("kwords_stats.dat", "wb");
    if (fp) {
        fwrite(&gameStats, sizeof(Stats), 1, fp);
        fclose(fp);
    }
}

RECT btnTheme = {220, 10, 360, 35};
RECT btnEasy = {370, 10, 430, 35};
RECT btnMed  = {440, 10, 510, 35};
RECT btnHard = {520, 10, 580, 35};
RECT btnHint = {590, 10, 650, 35};
RECT btnSave = {660, 10, 720, 35};
RECT btnLoad = {730, 10, 790, 35};
RECT btnStats = {800, 10, 860, 35};

bool showStats = false;

DWORD WINAPI SoundTick(LPVOID lpParam) {
    Beep(1500, 10);
    return 0;
}

DWORD WINAPI SoundChime(LPVOID lpParam) {
    Beep(523, 100);
    Beep(659, 100);
    Beep(784, 200);
    return 0;
}

DWORD WINAPI SoundFanfare(LPVOID lpParam) {
    Beep(440, 150);
    Beep(554, 150);
    Beep(659, 150);
    Beep(880, 400);
    return 0;
}

void PlaySoundEffect(int type) {
    if (type == 0) CreateThread(NULL, 0, SoundTick, NULL, 0, NULL);
    else if (type == 1) CreateThread(NULL, 0, SoundChime, NULL, 0, NULL);
    else if (type == 2) CreateThread(NULL, 0, SoundFanfare, NULL, 0, NULL);
}

void InitGame() {
    srand((unsigned int)time(NULL));
    memset(foundGrid, 0, sizeof(foundGrid));
    memset(hintedGrid, 0, sizeof(hintedGrid));
    memset(wordsFoundStatus, 0, sizeof(wordsFoundStatus));
    memset(wordsHintedStatus, 0, sizeof(wordsHintedStatus));
    memset(cellAnim, 0, sizeof(cellAnim));
    memset(strikeAnim, 0, sizeof(strikeAnim));
    foundCount = 0;
    currentScore = 0;
    timerSeconds = 0;
    gameWon = false;
    
    // Fill empty
    for(int r=0; r<gridSize; r++){
        for(int c=0; c<gridSize; c++){
            grid[r][c] = ' ';
        }
    }
    
    // Pick words
    int picked[THEME_DICT_SIZE] = {0};
    wordCount = 0;
    while(wordCount < numWordsToFind) {
        int idx = rand() % THEME_DICT_SIZE;
        if(!picked[idx]) {
            picked[idx] = 1;
            strcpy(wordsToFind[wordCount], DICTIONARIES[currentThemeIdx][idx]);
            wordCount++;
        }
    }
    
    // Place words
    int dirs[8][2] = {{0,1}, {1,0}, {1,1}, {-1,1}, {1,-1}, {-1,-1}, {0,-1}, {-1,0}};
    for(int w=0; w<wordCount; w++) {
        bool placed = false;
        int len = strlen(wordsToFind[w]);
        int attempts = 0;
        while(!placed && attempts < 200) {
            attempts++;
            int d = rand() % 8;
            int r = rand() % gridSize;
            int c = rand() % gridSize;
            
            bool canPlace = true;
            for(int i=0; i<len; i++) {
                int nr = r + i * dirs[d][0];
                int nc = c + i * dirs[d][1];
                if(nr < 0 || nr >= gridSize || nc < 0 || nc >= gridSize || (grid[nr][nc] != ' ' && grid[nr][nc] != wordsToFind[w][i])) {
                    canPlace = false;
                    break;
                }
            }
            if(canPlace) {
                for(int i=0; i<len; i++) {
                    grid[r + i * dirs[d][0]][c + i * dirs[d][1]] = wordsToFind[w][i];
                }
                placed = true;
            }
        }
    }
    
    // Fill rest
    for(int r=0; r<gridSize; r++){
        for(int c=0; c<gridSize; c++){
            if(grid[r][c] == ' '){
                grid[r][c] = 'A' + (rand() % 26);
            }
        }
    }
}

int my_sign(int x) { return (x > 0) - (x < 0); }
int my_max(int a, int b) { return a > b ? a : b; }
int my_abs(int x) { return x < 0 ? -x : x; }

void GetLineCells(int r1, int c1, int r2, int c2, int* outR, int* outC, int* count) {
    *count = 0;
    int dr = my_sign(r2 - r1);
    int dc = my_sign(c2 - c1);
    int dist = my_max(my_abs(r2 - r1), my_abs(c2 - c1));
    
    if (my_abs(r2 - r1) != my_abs(c2 - c1) && r1 != r2 && c1 != c2) return;
    
    for (int i = 0; i <= dist; i++) {
        outR[*count] = r1 + i * dr;
        outC[*count] = c1 + i * dc;
        (*count)++;
    }
}

void EndSelection(HWND hwnd) {
    if (!isSelecting) return;
    isSelecting = false;
    
    int selR[MAX_GRID_SIZE*2], selC[MAX_GRID_SIZE*2];
    int count;
    GetLineCells(startR, startC, curR, curC, selR, selC, &count);
    
    if (count > 0) {
        char selWord[32] = {0};
        char revWord[32] = {0};
        for(int i=0; i<count; i++) {
            selWord[i] = grid[selR[i]][selC[i]];
            revWord[count - 1 - i] = grid[selR[i]][selC[i]];
        }
        
        bool found = false;
        for(int w=0; w<wordCount; w++) {
            if(!wordsFoundStatus[w]) {
                if(strcmp(wordsToFind[w], selWord) == 0 || strcmp(wordsToFind[w], revWord) == 0) {
                    wordsFoundStatus[w] = true;
                    foundCount++;
                    found = true;
                    
                    int points = 1000 - (timerSeconds * 2);
                    if (points < 100) points = 100;
                    currentScore += points;
                    
                    for(int i=0; i<count; i++) {
                        foundGrid[selR[i]][selC[i]] = true;
                    }
                    break;
                }
            }
        }
        
        if (foundCount == wordCount) {
            gameWon = true;
            PlaySoundEffect(2);
            LoadStats();
            gameStats.completed++;
            int bTime = gameStats.bestTimes[currentThemeIdx][currentDifficulty];
            if (bTime == 0 || timerSeconds < bTime) {
                gameStats.bestTimes[currentThemeIdx][currentDifficulty] = timerSeconds;
            }
            SaveStats();
        } else if (found) {
            PlaySoundEffect(1);
        }
    }
    
    InvalidateRect(hwnd, NULL, TRUE);
}

void UseHint(HWND hwnd) {
    if (gameWon) return;
    
    int unfound[MAX_WORDS];
    int uncount = 0;
    for(int w=0; w<wordCount; w++) {
        if(!wordsFoundStatus[w] && !wordsHintedStatus[w]) {
            unfound[uncount++] = w;
        }
    }
    
    if (uncount == 0) {
        for(int w=0; w<wordCount; w++) {
            if(!wordsFoundStatus[w]) {
                unfound[uncount++] = w;
            }
        }
    }
    
    if (uncount == 0) return;
    
    int targetIdx = unfound[rand() % uncount];
    
    int dirs[8][2] = {{0,1}, {1,0}, {1,1}, {-1,1}, {1,-1}, {-1,-1}, {0,-1}, {-1,0}};
    int len = strlen(wordsToFind[targetIdx]);
    bool foundInGrid = false;
    for (int r = 0; r < gridSize && !foundInGrid; r++) {
        for (int c = 0; c < gridSize && !foundInGrid; c++) {
            if (grid[r][c] == wordsToFind[targetIdx][0]) {
                for (int d = 0; d < 8; d++) {
                    bool match = true;
                    for (int i = 0; i < len; i++) {
                        int nr = r + i * dirs[d][0];
                        int nc = c + i * dirs[d][1];
                        if (nr < 0 || nr >= gridSize || nc < 0 || nc >= gridSize || grid[nr][nc] != wordsToFind[targetIdx][i]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        hintedGrid[r][c] = true;
                        wordsHintedStatus[targetIdx] = true;
                        currentScore -= 50;
                        if (currentScore < 0) currentScore = 0;
                        timerSeconds += 30;
                        foundInGrid = true;
                        break;
                    }
                }
            }
        }
    }
    InvalidateRect(hwnd, NULL, FALSE);
}

void SaveGame(HWND hwnd) {
    FILE* fp = fopen("kwords_save.dat", "wb");
    if (fp) {
        fwrite(&gridSize, sizeof(int), 1, fp);
        fwrite(&numWordsToFind, sizeof(int), 1, fp);
        fwrite(&currentDifficulty, sizeof(int), 1, fp);
        fwrite(&currentThemeIdx, sizeof(int), 1, fp);
        fwrite(&wordCount, sizeof(int), 1, fp);
        fwrite(&foundCount, sizeof(int), 1, fp);
        fwrite(&currentScore, sizeof(int), 1, fp);
        fwrite(&timerSeconds, sizeof(int), 1, fp);
        fwrite(&gameWon, sizeof(bool), 1, fp);
        fwrite(grid, sizeof(char), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fwrite(foundGrid, sizeof(bool), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fwrite(hintedGrid, sizeof(bool), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fwrite(wordsToFind, sizeof(char), MAX_WORDS * 32, fp);
        fwrite(wordsFoundStatus, sizeof(bool), MAX_WORDS, fp);
        fwrite(wordsHintedStatus, sizeof(bool), MAX_WORDS, fp);
        fclose(fp);
        MessageBox(hwnd, "Game Saved successfully!", "Save", MB_OK);
    } else {
        MessageBox(hwnd, "Failed to save game.", "Error", MB_OK | MB_ICONERROR);
    }
}

void LoadGame(HWND hwnd) {
    FILE* fp = fopen("kwords_save.dat", "rb");
    if (fp) {
        fread(&gridSize, sizeof(int), 1, fp);
        fread(&numWordsToFind, sizeof(int), 1, fp);
        fread(&currentDifficulty, sizeof(int), 1, fp);
        fread(&currentThemeIdx, sizeof(int), 1, fp);
        fread(&wordCount, sizeof(int), 1, fp);
        fread(&foundCount, sizeof(int), 1, fp);
        fread(&currentScore, sizeof(int), 1, fp);
        fread(&timerSeconds, sizeof(int), 1, fp);
        fread(&gameWon, sizeof(bool), 1, fp);
        fread(grid, sizeof(char), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fread(foundGrid, sizeof(bool), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fread(hintedGrid, sizeof(bool), MAX_GRID_SIZE * MAX_GRID_SIZE, fp);
        fread(wordsToFind, sizeof(char), MAX_WORDS * 32, fp);
        fread(wordsFoundStatus, sizeof(bool), MAX_WORDS, fp);
        fread(wordsHintedStatus, sizeof(bool), MAX_WORDS, fp);
        fclose(fp);
        
        memset(cellAnim, 0, sizeof(cellAnim));
        memset(strikeAnim, 0, sizeof(strikeAnim));
        isSelecting = false;
        
        for(int w=0; w<wordCount; w++) {
            if (wordsFoundStatus[w]) strikeAnim[w] = 1.0f;
        }
        for(int r=0; r<gridSize; r++){
            for(int c=0; c<gridSize; c++){
                if(foundGrid[r][c]) cellAnim[r][c] = 1.0f;
            }
        }
        
        InvalidateRect(hwnd, NULL, TRUE);
    } else {
        MessageBox(hwnd, "No save file found.", "Load", MB_OK | MB_ICONINFORMATION);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            InitGame();
            SetTimer(hwnd, 1, 1000, NULL);
            SetTimer(hwnd, 2, 30, NULL);
            break;
        case WM_TIMER:
            if(wParam == 1) {
                if(!gameWon) {
                    timerSeconds++;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 2) {
                bool needsRedraw = false;
                int selR[MAX_GRID_SIZE*2], selC[MAX_GRID_SIZE*2];
                int selCount = 0;
                if (isSelecting) {
                    GetLineCells(startR, startC, curR, curC, selR, selC, &selCount);
                }
                for(int r=0; r<gridSize; r++) {
                    for(int c=0; c<gridSize; c++) {
                        bool isSel = false;
                        for(int i=0; i<selCount; i++) {
                            if(selR[i] == r && selC[i] == c) { isSel = true; break; }
                        }
                        bool isFound = foundGrid[r][c];
                        float target = (isSel || isFound) ? 1.0f : 0.0f;
                        if(cellAnim[r][c] < target) { cellAnim[r][c] += 0.2f; if(cellAnim[r][c]>1.0f) cellAnim[r][c]=1.0f; needsRedraw = true; }
                        else if(cellAnim[r][c] > target) { cellAnim[r][c] -= 0.2f; if(cellAnim[r][c]<0.0f) cellAnim[r][c]=0.0f; needsRedraw = true; }
                    }
                }
                for(int w=0; w<wordCount; w++) {
                    float target = wordsFoundStatus[w] ? 1.0f : 0.0f;
                    if(strikeAnim[w] < target) { strikeAnim[w] += 0.1f; if(strikeAnim[w]>1.0f) strikeAnim[w]=1.0f; needsRedraw = true; }
                }
                if(needsRedraw) InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_LBUTTONDOWN: {
            if (showStats) {
                showStats = false;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            if (PtInRect(&btnTheme, pt)) {
                currentThemeIdx = (currentThemeIdx + 1) % NUM_THEMES; InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }
            if (PtInRect(&btnEasy, pt)) {
                currentDifficulty = 0; gridSize = 10; numWordsToFind = 5; InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }
            if (PtInRect(&btnMed, pt)) {
                currentDifficulty = 1; gridSize = 15; numWordsToFind = 8; InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }
            if (PtInRect(&btnHard, pt)) {
                currentDifficulty = 2; gridSize = 20; numWordsToFind = 12; InitGame(); InvalidateRect(hwnd, NULL, TRUE); break;
            }
            if (PtInRect(&btnHint, pt)) {
                UseHint(hwnd); break;
            }
            if (PtInRect(&btnSave, pt)) {
                SaveGame(hwnd); break;
            }
            if (PtInRect(&btnLoad, pt)) {
                LoadGame(hwnd); break;
            }
            if (PtInRect(&btnStats, pt)) {
                LoadStats();
                showStats = true;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            if(gameWon) {
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int c = (x - 20) / CELL_SIZE;
            int r = (y - 50) / CELL_SIZE;
            if(r >= 0 && r < gridSize && c >= 0 && c < gridSize) {
                isSelecting = true;
                startR = curR = r;
                startC = curC = c;
                PlaySoundEffect(0);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_MOUSEMOVE: {
            if(isSelecting) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                int c = (x - 20) / CELL_SIZE;
                int r = (y - 50) / CELL_SIZE;
                if(r >= 0 && r < gridSize && c >= 0 && c < gridSize) {
                    if(curR != r || curC != c) {
                        int oldR = curR, oldC = curC;
                        curR = r;
                        curC = c;
                        int dummyR[MAX_GRID_SIZE*2], dummyC[MAX_GRID_SIZE*2];
                        int oldSelCount = 0, newSelCount = 0;
                        GetLineCells(startR, startC, oldR, oldC, dummyR, dummyC, &oldSelCount);
                        GetLineCells(startR, startC, curR, curC, dummyR, dummyC, &newSelCount);
                        if (oldSelCount != newSelCount) {
                            PlaySoundEffect(0);
                        }
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            }
            break;
        }
        case WM_LBUTTONUP:
            EndSelection(hwnd);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH bgBrush = CreateSolidBrush(RGB(15, 17, 26));
            FillRect(hdc, &ps.rcPaint, bgBrush);
            DeleteObject(bgBrush);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(226, 232, 240));
            
            char header[128];
            sprintf(header, "Found: %d/%d   Score: %d   Timer: %02d:%02d", foundCount, wordCount, currentScore, timerSeconds/60, timerSeconds%60);
            TextOut(hdc, 20, 15, header, strlen(header));
            
            // Draw difficulty buttons
            HBRUSH btnBrush = CreateSolidBrush(RGB(45, 55, 72));
            HBRUSH activeBrush = CreateSolidBrush(RGB(49, 130, 206));
            
            FillRect(hdc, &btnTheme, btnBrush);
            FillRect(hdc, &btnEasy, currentDifficulty == 0 ? activeBrush : btnBrush);
            FillRect(hdc, &btnMed, currentDifficulty == 1 ? activeBrush : btnBrush);
            FillRect(hdc, &btnHard, currentDifficulty == 2 ? activeBrush : btnBrush);
            
            HBRUSH hintBrush = CreateSolidBrush(RGB(214, 158, 46));
            FillRect(hdc, &btnHint, hintBrush);
            DeleteObject(hintBrush);
            
            HBRUSH slBrush = CreateSolidBrush(RGB(100, 100, 100));
            FillRect(hdc, &btnSave, slBrush);
            FillRect(hdc, &btnLoad, slBrush);
            FillRect(hdc, &btnStats, slBrush);
            DeleteObject(slBrush);
            
            char themeStr[64];
            sprintf(themeStr, "Theme: %s", THEMES[currentThemeIdx]);
            TextOut(hdc, btnTheme.left + 10, btnTheme.top + 5, themeStr, strlen(themeStr));
            
            TextOut(hdc, btnEasy.left + 14, btnEasy.top + 5, "Easy", 4);
            TextOut(hdc, btnMed.left + 7, btnMed.top + 5, "Medium", 6);
            TextOut(hdc, btnHard.left + 14, btnHard.top + 5, "Hard", 4);
            
            SetTextColor(hdc, RGB(26, 32, 44));
            TextOut(hdc, btnHint.left + 14, btnHint.top + 5, "Hint", 4);
            SetTextColor(hdc, RGB(226, 232, 240));
            
            TextOut(hdc, btnSave.left + 14, btnSave.top + 5, "Save", 4);
            TextOut(hdc, btnLoad.left + 14, btnLoad.top + 5, "Load", 4);
            TextOut(hdc, btnStats.left + 10, btnStats.top + 5, "Stats", 5);
            
            DeleteObject(btnBrush);
            DeleteObject(activeBrush);
            
            int selR[MAX_GRID_SIZE*2], selC[MAX_GRID_SIZE*2];
            int selCount = 0;
            if (isSelecting) {
                GetLineCells(startR, startC, curR, curC, selR, selC, &selCount);
            }
            
            HFONT hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(45, 55, 72));
            HPEN hOldPen = (HPEN)SelectObject(hdc, gridPen);
            
            for(int r=0; r<gridSize; r++) {
                for(int c=0; c<gridSize; c++) {
                    RECT rc = { 20 + c*CELL_SIZE, 50 + r*CELL_SIZE, 20 + (c+1)*CELL_SIZE, 50 + (r+1)*CELL_SIZE };
                    
                    bool isSelected = false;
                    for(int i=0; i<selCount; i++) {
                        if(selR[i] == r && selC[i] == c) {
                            isSelected = true; break;
                        }
                    }
                    
                    if (cellAnim[r][c] > 0.0f) {
                        int cx = (rc.left + rc.right) / 2;
                        int cy = (rc.top + rc.bottom) / 2;
                        int cw = (rc.right - rc.left) / 2;
                        int ch = (rc.bottom - rc.top) / 2;
                        int w = (int)(cw * cellAnim[r][c]);
                        int h = (int)(ch * cellAnim[r][c]);
                        RECT animRc = { cx - w, cy - h, cx + w, cy + h };
                        COLORREF color = isSelected ? RGB(49, 130, 206) : RGB(56, 161, 105);
                        HBRUSH animBrush = CreateSolidBrush(color);
                        FillRect(hdc, &animRc, animBrush);
                        DeleteObject(animBrush);
                    } else if (hintedGrid[r][c]) {
                        HBRUSH hHint = CreateSolidBrush(RGB(214, 158, 46));
                        FillRect(hdc, &rc, hHint);
                        DeleteObject(hHint);
                    }
                    
                    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
                    
                    if (hintedGrid[r][c] && cellAnim[r][c] == 0.0f) SetTextColor(hdc, RGB(26, 32, 44));
                    else SetTextColor(hdc, RGB(226, 232, 240));
                    
                    char ch[2] = { grid[r][c], 0 };
                    DrawText(hdc, ch, 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
            }
            
            int listX = 20 + gridSize*CELL_SIZE + 30;
            int listY = 50;
            TextOut(hdc, listX, listY, "Words to Find:", 14);
            listY += 25;
            
            for(int w=0; w<wordCount; w++) {
                if(wordsFoundStatus[w]) {
                    SetTextColor(hdc, RGB(100, 100, 100));
                } else {
                    SetTextColor(hdc, RGB(255, 255, 255));
                }
                TextOut(hdc, listX, listY, wordsToFind[w], strlen(wordsToFind[w]));
                
                if(strikeAnim[w] > 0.0f) {
                    HPEN strikePen = CreatePen(PS_SOLID, 2, RGB(100, 100, 100));
                    HPEN oldP = (HPEN)SelectObject(hdc, strikePen);
                    MoveToEx(hdc, listX, listY + 10, NULL);
                    int strikeLen = (int)(strlen(wordsToFind[w]) * 9 * strikeAnim[w]);
                    LineTo(hdc, listX + strikeLen, listY + 10);
                    SelectObject(hdc, oldP);
                    DeleteObject(strikePen);
                }
                
                listY += 25;
            }
            
            if(gameWon) {
                SetTextColor(hdc, RGB(255, 215, 0));
                TextOut(hdc, listX, listY + 20, "YOU WIN!", 8);
                SetTextColor(hdc, RGB(200, 200, 200));
                char scoreStr[64];
                sprintf(scoreStr, "Score: %d", currentScore);
                TextOut(hdc, listX, listY + 50, scoreStr, strlen(scoreStr));
                TextOut(hdc, listX, listY + 75, "Click anywhere", 14);
                TextOut(hdc, listX, listY + 95, "to play again", 13);
            }
            
            if(showStats) {
                RECT modalRc = { 100, 100, 700, 700 };
                HBRUSH modalBrush = CreateSolidBrush(RGB(30, 33, 43));
                FillRect(hdc, &modalRc, modalBrush);
                DeleteObject(modalBrush);
                
                SetTextColor(hdc, RGB(226, 232, 240));
                TextOut(hdc, 120, 120, "Statistics", 10);
                char statStr[128];
                sprintf(statStr, "Puzzles Completed: %d", gameStats.completed);
                TextOut(hdc, 120, 160, statStr, strlen(statStr));
                
                int yPos = 200;
                TextOut(hdc, 120, yPos, "Best Times:", 11);
                yPos += 30;
                const char* diffNames[] = {"Easy", "Medium", "Hard"};
                for(int t=0; t<NUM_THEMES; t++) {
                    for(int d=0; d<3; d++) {
                        int bTime = gameStats.bestTimes[t][d];
                        if(bTime > 0) {
                            sprintf(statStr, "%s - %s: %02d:%02d", THEMES[t], diffNames[d], bTime / 60, bTime % 60);
                            TextOut(hdc, 140, yPos, statStr, strlen(statStr));
                            yPos += 25;
                        }
                    }
                }
                TextOut(hdc, 120, 650, "Click anywhere to close stats", 29);
            }
            
            SelectObject(hdc, hOldPen);
            DeleteObject(gridPen);
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            KillTimer(hwnd, 2);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KWordsClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    if (!RegisterClass(&wc)) return 0;
    
    HWND hwnd = CreateWindow("KWordsClass", "KWords", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                             CW_USEDEFAULT, CW_USEDEFAULT, 900, 850,
                             NULL, NULL, hInstance, NULL);
                             
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return msg.wParam;
}
