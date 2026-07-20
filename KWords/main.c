#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define GRID_SIZE 15
#define CELL_SIZE 30
#define MAX_WORDS 8
#define DICTIONARY_SIZE 15

const char* WORD_LIST[DICTIONARY_SIZE] = {
    "ALGORITHM", "COMPILER", "DEBUG", "FUNCTION", "VARIABLE", 
    "POINTER", "SYNTAX", "OBJECT", "CLASS", "METHOD", 
    "ARRAY", "STRING", "BOOLEAN", "INTEGER", "FLOAT"
};

char grid[GRID_SIZE][GRID_SIZE];
bool foundGrid[GRID_SIZE][GRID_SIZE];

char wordsToFind[MAX_WORDS][32];
bool wordsFoundStatus[MAX_WORDS];
int wordCount = 0;
int foundCount = 0;

bool isSelecting = false;
int startR = -1, startC = -1;
int curR = -1, curC = -1;

int timerSeconds = 0;
bool gameWon = false;

void InitGame() {
    srand((unsigned int)time(NULL));
    memset(foundGrid, 0, sizeof(foundGrid));
    memset(wordsFoundStatus, 0, sizeof(wordsFoundStatus));
    foundCount = 0;
    timerSeconds = 0;
    gameWon = false;
    
    // Fill empty
    for(int r=0; r<GRID_SIZE; r++){
        for(int c=0; c<GRID_SIZE; c++){
            grid[r][c] = ' ';
        }
    }
    
    // Pick words
    int picked[DICTIONARY_SIZE] = {0};
    wordCount = 0;
    while(wordCount < MAX_WORDS) {
        int idx = rand() % DICTIONARY_SIZE;
        if(!picked[idx]) {
            picked[idx] = 1;
            strcpy(wordsToFind[wordCount], WORD_LIST[idx]);
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
            int r = rand() % GRID_SIZE;
            int c = rand() % GRID_SIZE;
            
            bool canPlace = true;
            for(int i=0; i<len; i++) {
                int nr = r + i * dirs[d][0];
                int nc = c + i * dirs[d][1];
                if(nr < 0 || nr >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE || (grid[nr][nc] != ' ' && grid[nr][nc] != wordsToFind[w][i])) {
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
    for(int r=0; r<GRID_SIZE; r++){
        for(int c=0; c<GRID_SIZE; c++){
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
    
    int selR[GRID_SIZE*2], selC[GRID_SIZE*2];
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
                    for(int i=0; i<count; i++) {
                        foundGrid[selR[i]][selC[i]] = true;
                    }
                    break;
                }
            }
        }
        
        if (foundCount == wordCount) {
            gameWon = true;
        }
    }
    
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            InitGame();
            SetTimer(hwnd, 1, 1000, NULL);
            break;
        case WM_TIMER:
            if(!gameWon) {
                timerSeconds++;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int c = (x - 20) / CELL_SIZE;
            int r = (y - 50) / CELL_SIZE;
            if(r >= 0 && r < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
                isSelecting = true;
                startR = curR = r;
                startC = curC = c;
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
                if(r >= 0 && r < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
                    if(curR != r || curC != c) {
                        curR = r;
                        curC = c;
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
            
            HBRUSH bgBrush = CreateSolidBrush(RGB(18, 18, 18));
            FillRect(hdc, &ps.rcPaint, bgBrush);
            DeleteObject(bgBrush);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));
            
            char header[128];
            sprintf(header, "KWords   Found: %d/%d   Timer: %02d:%02d", foundCount, wordCount, timerSeconds/60, timerSeconds%60);
            TextOut(hdc, 20, 15, header, strlen(header));
            
            int selR[GRID_SIZE*2], selC[GRID_SIZE*2];
            int selCount = 0;
            if (isSelecting) {
                GetLineCells(startR, startC, curR, curC, selR, selC, &selCount);
            }
            
            HFONT hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));
            HPEN hOldPen = (HPEN)SelectObject(hdc, gridPen);
            
            for(int r=0; r<GRID_SIZE; r++) {
                for(int c=0; c<GRID_SIZE; c++) {
                    RECT rc = { 20 + c*CELL_SIZE, 50 + r*CELL_SIZE, 20 + (c+1)*CELL_SIZE, 50 + (r+1)*CELL_SIZE };
                    
                    bool isSelected = false;
                    for(int i=0; i<selCount; i++) {
                        if(selR[i] == r && selC[i] == c) {
                            isSelected = true; break;
                        }
                    }
                    
                    if (isSelected) {
                        HBRUSH selBrush = CreateSolidBrush(RGB(0, 120, 215));
                        FillRect(hdc, &rc, selBrush);
                        DeleteObject(selBrush);
                    } else if (foundGrid[r][c]) {
                        HBRUSH foundBrush = CreateSolidBrush(RGB(16, 124, 16));
                        FillRect(hdc, &rc, foundBrush);
                        DeleteObject(foundBrush);
                    }
                    
                    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
                    
                    char ch[2] = { grid[r][c], 0 };
                    DrawText(hdc, ch, 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
            }
            
            int listX = 20 + GRID_SIZE*CELL_SIZE + 30;
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
                
                if(wordsFoundStatus[w]) {
                    HPEN strikePen = CreatePen(PS_SOLID, 2, RGB(100, 100, 100));
                    HPEN oldP = (HPEN)SelectObject(hdc, strikePen);
                    MoveToEx(hdc, listX, listY + 10, NULL);
                    LineTo(hdc, listX + strlen(wordsToFind[w])*9, listY + 10);
                    SelectObject(hdc, oldP);
                    DeleteObject(strikePen);
                }
                
                listY += 25;
            }
            
            if(gameWon) {
                SetTextColor(hdc, RGB(255, 215, 0));
                TextOut(hdc, listX, listY + 20, "YOU WIN!", 8);
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
                             CW_USEDEFAULT, CW_USEDEFAULT, 700, 600,
                             NULL, NULL, hInstance, NULL);
                             
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return msg.wParam;
}
