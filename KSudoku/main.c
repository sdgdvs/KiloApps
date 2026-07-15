#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// Colors
#define COLOR_BG RGB(15, 23, 42)
#define COLOR_GRID_THICK RGB(248, 250, 252)
#define COLOR_GRID_THIN RGB(51, 65, 85)
#define COLOR_TEXT RGB(248, 250, 252)
#define COLOR_FIXED_TEXT RGB(248, 250, 252)
#define COLOR_MUTABLE_TEXT RGB(59, 130, 246)
#define COLOR_SELECTED RGB(30, 64, 112)
#define COLOR_HIGHLIGHT RGB(24, 44, 85)
#define COLOR_ERROR RGB(239, 68, 68)

int board[9][9];
int solution[9][9];
int fixed[9][9];
int sel_r = -1, sel_c = -1;
int error_cells[9][9];
int notes[9][9][10];
int notesMode = 0;
int elapsedTime = 0;
int score = 0;
int awarded[9][9];
int timerActive = 0;
HWND hBtnNew, hBtnNotes, hBtnValidate, hBtnHint, hBtnUndo, hBtnRedo;
HFONT hFont, hFontSmall;

typedef struct {
    int board[9][9];
    int notes[9][9][10];
    int score;
} ActionState;

ActionState* undoStack = NULL;
int undoCapacity = 0;
int undoCount = 0;

ActionState* redoStack = NULL;
int redoCapacity = 0;
int redoCount = 0;

void PushState() {
    if (undoCount == undoCapacity) {
        undoCapacity = undoCapacity == 0 ? 16 : undoCapacity * 2;
        undoStack = (ActionState*)realloc(undoStack, undoCapacity * sizeof(ActionState));
    }
    for(int r=0; r<9; r++) {
        for(int c=0; c<9; c++) {
            undoStack[undoCount].board[r][c] = board[r][c];
            for(int i=0; i<10; i++) undoStack[undoCount].notes[r][c][i] = notes[r][c][i];
        }
    }
    undoStack[undoCount].score = score;
    undoCount++;
    redoCount = 0;
}

void Undo() {
    if (undoCount > 0) {
        if (redoCount == redoCapacity) {
            redoCapacity = redoCapacity == 0 ? 16 : redoCapacity * 2;
            redoStack = (ActionState*)realloc(redoStack, redoCapacity * sizeof(ActionState));
        }
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
                redoStack[redoCount].board[r][c] = board[r][c];
                for(int i=0; i<10; i++) redoStack[redoCount].notes[r][c][i] = notes[r][c][i];
            }
        }
        redoStack[redoCount].score = score;
        redoCount++;
        
        undoCount--;
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
                board[r][c] = undoStack[undoCount].board[r][c];
                for(int i=0; i<10; i++) notes[r][c][i] = undoStack[undoCount].notes[r][c][i];
                error_cells[r][c] = 0;
            }
        }
        score = undoStack[undoCount].score;
    }
}

void Redo() {
    if (redoCount > 0) {
        if (undoCount == undoCapacity) {
            undoCapacity = undoCapacity == 0 ? 16 : undoCapacity * 2;
            undoStack = (ActionState*)realloc(undoStack, undoCapacity * sizeof(ActionState));
        }
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
                undoStack[undoCount].board[r][c] = board[r][c];
                for(int i=0; i<10; i++) undoStack[undoCount].notes[r][c][i] = notes[r][c][i];
            }
        }
        undoStack[undoCount].score = score;
        undoCount++;
        
        redoCount--;
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
                board[r][c] = redoStack[redoCount].board[r][c];
                for(int i=0; i<10; i++) notes[r][c][i] = redoStack[redoCount].notes[r][c][i];
                error_cells[r][c] = 0;
            }
        }
        score = redoStack[redoCount].score;
    }
}

typedef struct {
    int played;
    int won;
    int bestTime;
    int bestScore;
} DifficultyStats;

DifficultyStats stats[3] = {0}; // 0: Easy (30), 1: Medium (40), 2: Hard (50)
int currentDiffIdx = 1;
int gameActive = 0;

void LoadStats() {
    FILE *f = fopen("ksudoku_stats.dat", "rb");
    if(f) {
        fread(stats, sizeof(DifficultyStats), 3, f);
        fclose(f);
    } else {
        for(int i=0; i<3; i++) {
            stats[i].played = 0;
            stats[i].won = 0;
            stats[i].bestTime = -1;
            stats[i].bestScore = 0;
        }
    }
}

void SaveStats() {
    FILE *f = fopen("ksudoku_stats.dat", "wb");
    if(f) {
        fwrite(stats, sizeof(DifficultyStats), 3, f);
        fclose(f);
    }
}

typedef struct {
    int board[9][9];
    int solution[9][9];
    int fixed[9][9];
    int notes[9][9][10];
    int elapsedTime;
    int score;
    int awarded[9][9];
    int currentDiffIdx;
    int gameActive;
} GameState;

int LoadGameState() {
    FILE *f = fopen("ksudoku_save.dat", "rb");
    if(!f) return 0;
    GameState state;
    if(fread(&state, sizeof(GameState), 1, f) == 1 && state.gameActive) {
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
                board[r][c] = state.board[r][c];
                solution[r][c] = state.solution[r][c];
                fixed[r][c] = state.fixed[r][c];
                awarded[r][c] = state.awarded[r][c];
                error_cells[r][c] = 0;
                for(int i=0; i<10; i++) notes[r][c][i] = state.notes[r][c][i];
            }
        }
        elapsedTime = state.elapsedTime;
        score = state.score;
        currentDiffIdx = state.currentDiffIdx;
        gameActive = 1;
        timerActive = 1;
        undoCount = 0;
        redoCount = 0;
        fclose(f);
        return 1;
    }
    fclose(f);
    return 0;
}

void SaveGameState() {
    if (!gameActive) {
        remove("ksudoku_save.dat");
        return;
    }
    FILE *f = fopen("ksudoku_save.dat", "wb");
    if (f) {
        GameState state;
        for(int r=0; r<9; r++) {
            for(int c=0; c<9; c++) {
                state.board[r][c] = board[r][c];
                state.solution[r][c] = solution[r][c];
                state.fixed[r][c] = fixed[r][c];
                state.awarded[r][c] = awarded[r][c];
                for(int i=0; i<10; i++) state.notes[r][c][i] = notes[r][c][i];
            }
        }
        state.elapsedTime = elapsedTime;
        state.score = score;
        state.currentDiffIdx = currentDiffIdx;
        state.gameActive = gameActive;
        fwrite(&state, sizeof(GameState), 1, f);
        fclose(f);
    }
}

void ShuffleArray(int* arr, int len) {
    for (int i = len - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

void GenerateBoard(int removal) {
    int base[9][9] = {
        {1,2,3, 4,5,6, 7,8,9},
        {4,5,6, 7,8,9, 1,2,3},
        {7,8,9, 1,2,3, 4,5,6},
        {2,3,1, 5,6,4, 8,9,7},
        {5,6,4, 8,9,7, 2,3,1},
        {8,9,7, 2,3,1, 5,6,4},
        {3,1,2, 6,4,5, 9,7,8},
        {6,4,5, 9,7,8, 3,1,2},
        {9,7,8, 3,1,2, 6,4,5}
    };
    int nums[9] = {1,2,3,4,5,6,7,8,9};
    ShuffleArray(nums, 9);
    for(int r=0; r<9; r++)
        for(int c=0; c<9; c++)
            solution[r][c] = nums[base[r][c]-1];
            
    for(int band=0; band<3; band++) {
        int rows[3] = {0,1,2};
        ShuffleArray(rows, 3);
        int temp[3][9];
        for(int i=0; i<3; i++)
            for(int c=0; c<9; c++) temp[i][c] = solution[band*3 + rows[i]][c];
        for(int i=0; i<3; i++)
            for(int c=0; c<9; c++) solution[band*3 + i][c] = temp[i][c];
    }
    
    for(int band=0; band<3; band++) {
        int cols[3] = {0,1,2};
        ShuffleArray(cols, 3);
        int temp[9][3];
        for(int i=0; i<3; i++)
            for(int r=0; r<9; r++) temp[r][i] = solution[r][band*3 + cols[i]];
        for(int i=0; i<3; i++)
            for(int r=0; r<9; r++) solution[r][band*3 + i] = temp[r][i];
    }
    
    for(int r=0; r<9; r++) {
        for(int c=0; c<9; c++) {
            board[r][c] = solution[r][c];
            fixed[r][c] = 1;
            error_cells[r][c] = 0;
        }
    }
    
    while(removal > 0) {
        int r = rand() % 9;
        int c = rand() % 9;
        if(board[r][c] != 0) {
            board[r][c] = 0;
            fixed[r][c] = 0;
            removal--;
        }
    }
    
    for(int r=0; r<9; r++) {
        for(int c=0; c<9; c++) {
            for(int i=0; i<10; i++) {
                notes[r][c][i] = 0;
            }
            awarded[r][c] = 0;
        }
    }
    elapsedTime = 0;
    score = 0;
    timerActive = 1;
    gameActive = 1;
    SaveGameState();
}

void CheckWin(HWND hwnd) {
    for(int r=0; r<9; r++)
        for(int c=0; c<9; c++)
            if(board[r][c] != solution[r][c]) return;
            
    timerActive = 0;
    int timeBonus = 3000 - elapsedTime * 2;
    if(timeBonus < 0) timeBonus = 0;
    score += timeBonus;
    
    if (gameActive) {
        gameActive = 0;
        SaveGameState(); // clear save
        stats[currentDiffIdx].won++;
        if (stats[currentDiffIdx].bestTime == -1 || elapsedTime < stats[currentDiffIdx].bestTime) 
            stats[currentDiffIdx].bestTime = elapsedTime;
        if (score > stats[currentDiffIdx].bestScore) 
            stats[currentDiffIdx].bestScore = score;
        SaveStats();
    }
    
    char msg[256];
    sprintf(msg, "Congratulations! You solved the puzzle!\nTime: %02d:%02d\nScore: %d", elapsedTime/60, elapsedTime%60, score);
    MessageBoxA(hwnd, msg, "KSudoku", MB_OK);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            srand((unsigned int)time(NULL));
            LoadStats();
            if(!LoadGameState()) {
                currentDiffIdx = 1;
                stats[currentDiffIdx].played++;
                SaveStats();
                GenerateBoard(40);
            }
            HWND hComboDifficulty = CreateWindowA("COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 10, 10, 90, 100, hwnd, (HMENU)4, NULL, NULL);
            SendMessageA(hComboDifficulty, CB_ADDSTRING, 0, (LPARAM)"Easy");
            SendMessageA(hComboDifficulty, CB_ADDSTRING, 0, (LPARAM)"Medium");
            SendMessageA(hComboDifficulty, CB_ADDSTRING, 0, (LPARAM)"Hard");
            SendMessageA(hComboDifficulty, CB_SETCURSEL, currentDiffIdx, 0);
            hBtnNew = CreateWindowA("BUTTON", "New", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 110, 10, 50, 30, hwnd, (HMENU)1, NULL, NULL);
            HWND hBtnStats = CreateWindowA("BUTTON", "Stats", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 165, 10, 50, 30, hwnd, (HMENU)6, NULL, NULL);
            hBtnNotes = CreateWindowA("BUTTON", "Notes: OFF", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 220, 10, 85, 30, hwnd, (HMENU)3, NULL, NULL);
            hBtnValidate = CreateWindowA("BUTTON", "Validate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 310, 10, 60, 30, hwnd, (HMENU)2, NULL, NULL);
            hBtnHint = CreateWindowA("BUTTON", "Hint", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 375, 10, 45, 30, hwnd, (HMENU)5, NULL, NULL);
            hBtnUndo = CreateWindowA("BUTTON", "Undo", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 45, 50, 30, hwnd, (HMENU)7, NULL, NULL);
            hBtnRedo = CreateWindowA("BUTTON", "Redo", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 65, 45, 50, 30, hwnd, (HMENU)8, NULL, NULL);
            hFont = CreateFontA(24, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            hFontSmall = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            SetTimer(hwnd, 1, 1000, NULL);
            break;
        }
        case WM_TIMER: {
            if(timerActive) {
                elapsedTime++;
                SaveGameState();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) { // New Game
                HWND hCombo = GetDlgItem(hwnd, 4);
                int sel = SendMessageA(hCombo, CB_GETCURSEL, 0, 0);
                int removal = 40;
                currentDiffIdx = 1;
                if(sel == 0) { removal = 30; currentDiffIdx = 0; }
                else if(sel == 2) { removal = 50; currentDiffIdx = 2; }
                stats[currentDiffIdx].played++;
                SaveStats();
                undoCount = 0;
                redoCount = 0;
                GenerateBoard(removal);
                sel_r = -1; sel_c = -1;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 6) { // Stats
                char msg[512] = "";
                const char* diffs[] = {"Easy", "Medium", "Hard"};
                for(int i=0; i<3; i++) {
                    int rate = stats[i].played > 0 ? (stats[i].won * 100) / stats[i].played : 0;
                    char bt[32];
                    if(stats[i].bestTime == -1) sprintf(bt, "--:--");
                    else sprintf(bt, "%02d:%02d", stats[i].bestTime/60, stats[i].bestTime%60);
                    char buf[128];
                    sprintf(buf, "[%s]\nPlayed: %d | Won: %d | Win Rate: %d%%\nBest Time: %s | Best Score: %d\n\n", 
                            diffs[i], stats[i].played, stats[i].won, rate, bt, stats[i].bestScore);
                    strcat(msg, buf);
                }
                MessageBoxA(hwnd, msg, "Statistics", MB_OK | MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == 3) { // Toggle Notes
                notesMode = !notesMode;
                SetWindowTextA(hBtnNotes, notesMode ? "Notes: ON" : "Notes: OFF");
            } else if (LOWORD(wParam) == 2) { // Validate
                int errorCount = 0;
                for(int r=0; r<9; r++) {
                    for(int c=0; c<9; c++) {
                        if (!fixed[r][c] && board[r][c] != 0 && board[r][c] != solution[r][c]) {
                            error_cells[r][c] = 1;
                            errorCount++;
                        } else {
                            error_cells[r][c] = 0;
                        }
                    }
                }
                if (errorCount > 0) {
                    score -= errorCount * 20;
                    if (score < 0) score = 0;
                }
                SaveGameState();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 5) { // Hint
                if (sel_r >= 0 && sel_c >= 0 && !fixed[sel_r][sel_c]) {
                    if (board[sel_r][sel_c] != solution[sel_r][sel_c]) {
                        PushState();
                        score -= 150;
                        if (score < 0) score = 0;
                        board[sel_r][sel_c] = solution[sel_r][sel_c];
                        awarded[sel_r][sel_c] = 1;
                        error_cells[sel_r][sel_c] = 0;
                        CheckWin(hwnd);
                    }
                }
                SaveGameState();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 7) { // Undo
                Undo();
                SaveGameState();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 8) { // Redo
                Redo();
                SaveGameState();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Set focus back to main window so keyboard works
            SetFocus(hwnd);
            break;
        }
        case WM_KEYDOWN: {
            if(sel_r == -1) sel_r = 0, sel_c = 0;
            else {
                if(wParam == VK_UP) sel_r = max(0, sel_r - 1);
                if(wParam == VK_DOWN) sel_r = min(8, sel_r + 1);
                if(wParam == VK_LEFT) sel_c = max(0, sel_c - 1);
                if(wParam == VK_RIGHT) sel_c = min(8, sel_c + 1);
            }
            if(wParam >= '1' && wParam <= '9') {
                if(!fixed[sel_r][sel_c]) {
                    int num = wParam - '0';
                    if (notesMode) {
                        if (board[sel_r][sel_c] == 0) {
                            PushState();
                            notes[sel_r][sel_c][num] = !notes[sel_r][sel_c][num];
                        }
                    } else {
                        if (board[sel_r][sel_c] != num) {
                            PushState();
                            int invalid = 0;
                            for(int i=0; i<9; i++) {
                                if(i != sel_c && board[sel_r][i] == num) invalid = 1;
                                if(i != sel_r && board[i][sel_c] == num) invalid = 1;
                            }
                            int br = (sel_r/3)*3, bc = (sel_c/3)*3;
                            for(int r=0; r<3; r++) {
                                for(int c=0; c<3; c++) {
                                    if((br+r != sel_r || bc+c != sel_c) && board[br+r][bc+c] == num) invalid = 1;
                                }
                            }
                            
                            if(invalid) {
                                MessageBeep(MB_ICONWARNING);
                                score -= 50;
                                if (score < 0) score = 0;
                            }
                            board[sel_r][sel_c] = num;
                            error_cells[sel_r][sel_c] = 0;
                            if (num == solution[sel_r][sel_c]) {
                                if (!awarded[sel_r][sel_c]) {
                                    awarded[sel_r][sel_c] = 1;
                                    score += 100;
                                }
                            }
                            CheckWin(hwnd);
                        }
                    }
                }
            } else if(wParam == VK_BACK || wParam == VK_DELETE || wParam == '0') {
                if(!fixed[sel_r][sel_c]) {
                    if (board[sel_r][sel_c] != 0) {
                        PushState();
                        board[sel_r][sel_c] = 0;
                        error_cells[sel_r][sel_c] = 0;
                        for(int i=0; i<10; i++) notes[sel_r][sel_c][i] = 0;
                    } else {
                        int hasNotes = 0;
                        for(int i=0; i<10; i++) if (notes[sel_r][sel_c][i]) hasNotes = 1;
                        if (hasNotes) {
                            PushState();
                            for(int i=0; i<10; i++) notes[sel_r][sel_c][i] = 0;
                        }
                    }
                }
            } else if(wParam == 'Z' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                Undo();
            } else if(wParam == 'Y' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                Redo();
            } else if(wParam == 'N') {
                SendMessageA(hwnd, WM_COMMAND, 3, 0);
            }
            SaveGameState();
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int start_x = 40, start_y = 80, cell_sz = 40;
            if(x >= start_x && x < start_x + 9*cell_sz && y >= start_y && y < start_y + 9*cell_sz) {
                sel_c = (x - start_x) / cell_sz;
                sel_r = (y - start_y) / cell_sz;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            // Ensure focus is on main window so keyboard works
            SetFocus(hwnd);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Background
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            HBRUSH hbg = CreateSolidBrush(COLOR_BG);
            FillRect(hdc, &clientRect, hbg);
            DeleteObject(hbg);
            
            HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, COLOR_MUTABLE_TEXT);
            char info[64];
            sprintf(info, "Time: %02d:%02d   Score: %d", elapsedTime/60, elapsedTime%60, score);
            TextOutA(hdc, 40, 50, info, strlen(info));
            
            int start_x = 40, start_y = 80, cell_sz = 40;
            
            // Determine highlight value
            int highlight_val = 0;
            if(sel_r >= 0 && sel_c >= 0 && board[sel_r][sel_c] != 0) {
                highlight_val = board[sel_r][sel_c];
            }
            
            // Draw cells
            for(int r=0; r<9; r++) {
                for(int c=0; c<9; c++) {
                    RECT rc = {start_x + c*cell_sz, start_y + r*cell_sz, start_x + (c+1)*cell_sz, start_y + (r+1)*cell_sz};
                    
                    HBRUSH cellBg = NULL;
                    if(r == sel_r && c == sel_c) {
                        cellBg = CreateSolidBrush(COLOR_SELECTED);
                    } else if(sel_r >= 0 && (r == sel_r || c == sel_c || (r/3 == sel_r/3 && c/3 == sel_c/3))) {
                        cellBg = CreateSolidBrush(COLOR_HIGHLIGHT);
                    } else if(highlight_val && board[r][c] == highlight_val) {
                        cellBg = CreateSolidBrush(COLOR_HIGHLIGHT);
                    }
                    
                    if(cellBg) {
                        FillRect(hdc, &rc, cellBg);
                        DeleteObject(cellBg);
                    }
                    
                    if(board[r][c] != 0) {
                        char buf[2];
                        sprintf(buf, "%d", board[r][c]);
                        if(error_cells[r][c]) SetTextColor(hdc, COLOR_ERROR);
                        else if(fixed[r][c]) SetTextColor(hdc, COLOR_FIXED_TEXT); 
                        else SetTextColor(hdc, COLOR_MUTABLE_TEXT);
                        
                        RECT textRc = rc;
                        textRc.top += 2; // Visually center the Arial font
                        DrawTextA(hdc, buf, -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    } else {
                        int hasNotes = 0;
                        for(int i=1; i<=9; i++) if(notes[r][c][i]) hasNotes = 1;
                        if(hasNotes) {
                            HFONT oldFnt = (HFONT)SelectObject(hdc, hFontSmall);
                            SetTextColor(hdc, RGB(148, 163, 184));
                            for(int i=1; i<=9; i++) {
                                if(notes[r][c][i]) {
                                    int sub_r = (i-1) / 3;
                                    int sub_c = (i-1) % 3;
                                    RECT textRc = rc;
                                    textRc.left += sub_c * (cell_sz / 3);
                                    textRc.right = textRc.left + (cell_sz / 3);
                                    textRc.top += sub_r * (cell_sz / 3);
                                    textRc.bottom = textRc.top + (cell_sz / 3);
                                    
                                    char buf[2];
                                    sprintf(buf, "%d", i);
                                    DrawTextA(hdc, buf, -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                                }
                            }
                            SelectObject(hdc, oldFnt);
                        }
                    }
                }
            }
            
            // Draw grid lines
            for(int i=0; i<=9; i++) {
                int thick = (i % 3 == 0);
                HPEN hPen = CreatePen(PS_SOLID, thick ? 3 : 1, thick ? COLOR_GRID_THICK : COLOR_GRID_THIN);
                HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
                
                MoveToEx(hdc, start_x + i*cell_sz, start_y, NULL);
                LineTo(hdc, start_x + i*cell_sz, start_y + 9*cell_sz);
                
                MoveToEx(hdc, start_x, start_y + i*cell_sz, NULL);
                LineTo(hdc, start_x + 9*cell_sz, start_y + i*cell_sz);
                
                SelectObject(hdc, oldPen);
                DeleteObject(hPen);
            }
            
            SelectObject(hdc, oldFont);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            DeleteObject(hFont);
            DeleteObject(hFontSmall);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "KSudokuClass";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClassA(&wc);
    
    // Calculate required window size
    RECT rc = {0, 0, 440, 480}; 
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX), FALSE);
    
    HWND hwnd = CreateWindowA("KSudokuClass", "KSudoku", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
        
    ShowWindow(hwnd, nCmdShow);
    
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
