#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define EMPTY 0
#define BLACK 1
#define WHITE 2

#define IDM_EASY 1001
#define IDM_MEDIUM 1002
#define IDM_HARD 1003
#define IDM_UNDO 1004
#define IDM_SOUND 1005
#define IDM_STATS 1006
#define IDM_SAVE 1007
#define IDM_LOAD 1008
#define IDM_HINT 1009
#define IDM_TIME_UNTIMED 1010
#define IDM_TIME_BLITZ 1011
#define IDM_TIME_RAPID 1012
#define IDM_HANDICAP_STD 1013
#define IDM_HANDICAP_BLACK_CORNER 1014
#define IDM_HANDICAP_WHITE_CORNER 1015
#define IDM_HANDICAP_BLACK_ADV 1016
#define IDM_HANDICAP_4CORNERS 1017
#define IDM_THEME_CLASSIC 1020
#define IDM_THEME_CYBER 1021
#define IDM_THEME_CRIMSON 1022
#define IDM_THEME_TERMINAL 1023

typedef struct {
    COLORREF bg;
    COLORREF boardCell;
    COLORREF cellHover;
    COLORREF gridPen;
    COLORREF disc1;
    COLORREF disc2;
    COLORREF text;
    COLORREF evalPanelBg;
    COLORREF hintPen;
    const char* p1Name;
    const char* p2Name;
} BoardTheme;

static const BoardTheme g_themes[4] = {
    // 0: Classic Emerald
    { RGB(18, 18, 18), RGB(26, 86, 44), RGB(35, 120, 59), RGB(34, 34, 34), RGB(20, 20, 20), RGB(220, 220, 220), RGB(220, 220, 220), RGB(26, 26, 26), RGB(255, 215, 0), "Black", "White" },
    // 1: Midnight Cyber
    { RGB(6, 11, 25), RGB(11, 19, 43), RGB(28, 37, 65), RGB(0, 245, 212), RGB(0, 245, 212), RGB(255, 0, 127), RGB(224, 230, 237), RGB(13, 27, 42), RGB(255, 0, 127), "Cyan", "Pink" },
    // 2: Obsidian Crimson
    { RGB(20, 3, 6), RGB(58, 8, 16), RGB(84, 12, 23), RGB(90, 12, 24), RGB(255, 215, 0), RGB(217, 4, 41), RGB(247, 214, 216), RGB(40, 6, 12), RGB(255, 215, 0), "Gold", "Ruby" },
    // 3: Retro Terminal
    { RGB(2, 11, 2), RGB(7, 28, 7), RGB(17, 54, 17), RGB(0, 255, 102), RGB(255, 176, 0), RGB(51, 255, 51), RGB(51, 255, 51), RGB(9, 32, 9), RGB(255, 176, 0), "Amber", "Green" }
};

int currentTheme = 0; // 0=Classic, 1=Cyber, 2=Crimson, 3=Terminal

const char g_szClassName[] = "KReversiClass";

int board[64];
int soundEnabled = 1;
int showHint = 1;
int stats[3] = {0, 0, 0}; // Wins, Losses, Draws

int moveTimeMode = 0; // 0=Untimed, 5=Blitz 5s, 15=Rapid 15s
int handicapMode = 0; // 0=Std, 1=Black Corner, 2=White Corner, 3=Black Adv, 4=4 Corners
int moveTimeLeftDeci = 0;
int gameEnded = 0;

static const int g_weights[64] = {
    100, -20,  10,   5,   5,  10, -20, 100,
    -20, -50,  -2,  -2,  -2,  -2, -50, -20,
     10,  -2,  -1,  -1,  -1,  -1,  -2,  10,
      5,  -2,  -1,  -1,  -1,  -1,  -2,   5,
      5,  -2,  -1,  -1,  -1,  -1,  -2,   5,
     10,  -2,  -1,  -1,  -1,  -1,  -2,  10,
    -20, -50,  -2,  -2,  -2,  -2, -50, -20,
    100, -20,  10,   5,   5,  10, -20, 100
};

void LoadTheme() {
    FILE* f = fopen("kreversi_theme.dat", "rb");
    if (f) {
        fread(&currentTheme, sizeof(int), 1, f);
        fclose(f);
        if (currentTheme < 0 || currentTheme > 3) currentTheme = 0;
    }
}

void SaveTheme() {
    FILE* f = fopen("kreversi_theme.dat", "wb");
    if (f) {
        fwrite(&currentTheme, sizeof(int), 1, f);
        fclose(f);
    }
}

void LoadStats() {
    FILE* f = fopen("kreversi_stats.dat", "rb");
    if (f) {
        fread(stats, sizeof(int), 3, f);
        fclose(f);
    }
}

void SaveStats() {
    FILE* f = fopen("kreversi_stats.dat", "wb");
    if (f) {
        fwrite(stats, sizeof(int), 3, f);
        fclose(f);
    }
}

void UpdateStats(int bCount, int wCount) {
    if (bCount > wCount) stats[0]++;
    else if (wCount > bCount) stats[1]++;
    else stats[2]++;
    SaveStats();
}

void ShowStats(HWND hwnd) {
    char msg[256];
    sprintf(msg, "Games Played: %d\nWins: %d\nLosses: %d\nDraws: %d", stats[0]+stats[1]+stats[2], stats[0], stats[1], stats[2]);
    MessageBox(hwnd, msg, "Game Statistics", MB_OK | MB_ICONINFORMATION);
}

int gameOverSoundPlayed = 0;
int animatingFlips[64];
int numAnimatingFlips = 0;
int flipProgress = 0;

void PlaySoundEffect(int type) {
    if (!soundEnabled) return;
    if (type == 1) { // Place
        Beep(600, 50);
    } else if (type == 2) { // Game Over
        Beep(400, 150);
        Beep(200, 150);
    }
}

int currentPlayer = BLACK;
int dirs[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
int aiDifficulty = 1; // 0=Easy, 1=Medium, 2=Hard

typedef struct {
    int board[64];
    int currentPlayer;
} HistoryState;

HistoryState history[200];
int historyCount = 0;

void ApplyHandicap() {
    for(int i = 0; i < 64; i++) board[i] = EMPTY;
    board[27] = WHITE; board[28] = BLACK;
    board[35] = BLACK; board[36] = WHITE;

    if (handicapMode == 1) { // Black Corner A1
        board[0] = BLACK;
    } else if (handicapMode == 2) { // White Corner H8
        board[63] = WHITE;
    } else if (handicapMode == 3) { // Black Adv (+2)
        board[19] = BLACK; board[44] = BLACK;
    } else if (handicapMode == 4) { // 4 Corners
        board[0] = BLACK; board[7] = WHITE;
        board[56] = WHITE; board[63] = BLACK;
    }
}

void ResetMoveTimer(HWND hwnd) {
    if (hwnd) KillTimer(hwnd, 3);
    if (moveTimeMode > 0 && !gameEnded && hwnd) {
        moveTimeLeftDeci = moveTimeMode * 10;
        SetTimer(hwnd, 3, 100, NULL);
    }
}

void InitGame(HWND hwnd) {
    ApplyHandicap();
    currentPlayer = BLACK;
    historyCount = 0;
    gameOverSoundPlayed = 0;
    gameEnded = 0;
    if (hwnd) {
        KillTimer(hwnd, 1);
        KillTimer(hwnd, 2);
        ResetMoveTimer(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void PushHistory() {
    if (historyCount < 200) {
        for (int i = 0; i < 64; i++) history[historyCount].board[i] = board[i];
        history[historyCount].currentPlayer = currentPlayer;
        historyCount++;
    } else {
        for(int i = 1; i < 200; i++) history[i-1] = history[i];
        for(int i = 0; i < 64; i++) history[199].board[i] = board[i];
        history[199].currentPlayer = currentPlayer;
    }
}

void UndoMove(HWND hwnd) {
    KillTimer(hwnd, 1);
    KillTimer(hwnd, 2);
    numAnimatingFlips = 0;
    if (historyCount == 0) return;
    
    HistoryState lastState = history[--historyCount];
    while(lastState.currentPlayer != BLACK && historyCount > 0) {
        lastState = history[--historyCount];
    }
    
    for(int i = 0; i < 64; i++) board[i] = lastState.board[i];
    currentPlayer = lastState.currentPlayer;
    
    gameOverSoundPlayed = 0;
    gameEnded = 0;
    ResetMoveTimer(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

void SaveGame(HWND hwnd) {
    FILE* f = fopen("kreversi_save.dat", "wb");
    if (f) {
        fwrite(board, sizeof(int), 64, f);
        fwrite(&currentPlayer, sizeof(int), 1, f);
        fwrite(&historyCount, sizeof(int), 1, f);
        fwrite(history, sizeof(HistoryState), historyCount, f);
        fwrite(&currentTheme, sizeof(int), 1, f);
        fclose(f);
        MessageBox(hwnd, "Game saved successfully.", "Save Game", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBox(hwnd, "Failed to save game.", "Error", MB_OK | MB_ICONERROR);
    }
}

void LoadGame(HWND hwnd) {
    FILE* f = fopen("kreversi_save.dat", "rb");
    if (f) {
        fread(board, sizeof(int), 64, f);
        fread(&currentPlayer, sizeof(int), 1, f);
        fread(&historyCount, sizeof(int), 1, f);
        fread(history, sizeof(HistoryState), historyCount, f);
        if (fread(&currentTheme, sizeof(int), 1, f) == 1) {
            if (currentTheme < 0 || currentTheme > 3) currentTheme = 0;
            HMENU hMenu = GetMenu(hwnd);
            CheckMenuItem(hMenu, IDM_THEME_CLASSIC, MF_UNCHECKED);
            CheckMenuItem(hMenu, IDM_THEME_CYBER, MF_UNCHECKED);
            CheckMenuItem(hMenu, IDM_THEME_CRIMSON, MF_UNCHECKED);
            CheckMenuItem(hMenu, IDM_THEME_TERMINAL, MF_UNCHECKED);
            CheckMenuItem(hMenu, IDM_THEME_CLASSIC + currentTheme, MF_CHECKED);
        }
        fclose(f);
        KillTimer(hwnd, 1);
        KillTimer(hwnd, 2);
        numAnimatingFlips = 0;
        gameOverSoundPlayed = 0;
        gameEnded = 0;
        ResetMoveTimer(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        MessageBox(hwnd, "Game loaded successfully.", "Load Game", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBox(hwnd, "No saved game found.", "Load Game", MB_OK | MB_ICONWARNING);
    }
}

int GetFlippable(int index, int player, int* flippable) {
    if (board[index] != EMPTY) return 0;
    int r = index / 8;
    int c = index % 8;
    int opponent = (player == BLACK) ? WHITE : BLACK;
    int count = 0;

    for (int d = 0; d < 8; d++) {
        int nr = r + dirs[d][0];
        int nc = c + dirs[d][1];
        int curDirFlippable[8];
        int curCount = 0;

        while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8 && board[nr * 8 + nc] == opponent) {
            curDirFlippable[curCount++] = nr * 8 + nc;
            nr += dirs[d][0];
            nc += dirs[d][1];
        }
        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8 && board[nr * 8 + nc] == player) {
            for (int i = 0; i < curCount; i++) {
                flippable[count++] = curDirFlippable[i];
            }
        }
    }
    return count;
}

int HasValidMoves(int player) {
    int dummy[64];
    for(int i = 0; i < 64; i++) {
        if(GetFlippable(i, player, dummy) > 0) return 1;
    }
    return 0;
}

int EvaluatePosition() {
    int score = 0;
    for (int i = 0; i < 64; i++) {
        if (board[i] == BLACK) score += g_weights[i] + 1;
        else if (board[i] == WHITE) score -= (g_weights[i] + 1);
    }
    return score;
}

int GetBestMoveForPlayer(int player) {
    int maxVal = -999999;
    int bestMove = -1;
    for (int i = 0; i < 64; i++) {
        int flips[64];
        int count = GetFlippable(i, player, flips);
        if (count > 0) {
            int val = count * 2 + g_weights[i];
            if (val > maxVal) {
                maxVal = val;
                bestMove = i;
            }
        }
    }
    return bestMove;
}

void DoMove(int index, int player, HWND hwnd) {
    int count = GetFlippable(index, player, animatingFlips);
    if (count > 0) {
        board[index] = player;
        numAnimatingFlips = count;
        flipProgress = 0;
        SetTimer(hwnd, 2, 30, NULL);
        
        for(int i=0; i<count; i++) {
            board[animatingFlips[i]] = player;
        }
        PlaySoundEffect(1);
    }
}

void AIMove(HWND hwnd) {
    if (gameEnded) return;
    if (!HasValidMoves(WHITE)) {
        currentPlayer = BLACK;
        ResetMoveTimer(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }
    
    int bestMove = -1;
    int moves[64];
    int numMoves = 0;
    
    for(int i = 0; i < 64; i++) {
        int dummy[64];
        if (GetFlippable(i, WHITE, dummy) > 0) {
            moves[numMoves++] = i;
        }
    }
    
    if (numMoves > 0) {
        PushHistory();
        if (aiDifficulty == 0) { // Easy
            bestMove = moves[rand() % numMoves];
        } else if (aiDifficulty == 1) { // Medium
            int maxFlips = -1;
            for(int k = 0; k < numMoves; k++) {
                int m = moves[k];
                int flips[64];
                int count = GetFlippable(m, WHITE, flips);
                if (count > maxFlips) {
                    maxFlips = count;
                    bestMove = m;
                }
            }
        } else { // Hard
            int weights[64] = {
                100, -20,  10,   5,   5,  10, -20, 100,
                -20, -50,  -2,  -2,  -2,  -2, -50, -20,
                 10,  -2,  -1,  -1,  -1,  -1,  -2,  10,
                  5,  -2,  -1,  -1,  -1,  -1,  -2,   5,
                  5,  -2,  -1,  -1,  -1,  -1,  -2,   5,
                 10,  -2,  -1,  -1,  -1,  -1,  -2,  10,
                -20, -50,  -2,  -2,  -2,  -2, -50, -20,
                100, -20,  10,   5,   5,  10, -20, 100
            };
            int maxScore = -999999;
            for(int k = 0; k < numMoves; k++) {
                int m = moves[k];
                int flips[64];
                int count = GetFlippable(m, WHITE, flips);
                int score = count + weights[m];
                if (score > maxScore) {
                    maxScore = score;
                    bestMove = m;
                }
            }
        }
    }
    
    if (bestMove != -1) {
        DoMove(bestMove, WHITE, hwnd);
    }
    currentPlayer = BLACK;
    if (!HasValidMoves(BLACK)) {
        currentPlayer = WHITE;
    }
    ResetMoveTimer(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            srand((unsigned int)time(NULL));
            LoadTheme();
            HMENU hMenu = CreateMenu();
            HMENU hSubMenu = CreatePopupMenu();
            AppendMenu(hSubMenu, MF_STRING, IDM_EASY, "Easy");
            AppendMenu(hSubMenu, MF_STRING | MF_CHECKED, IDM_MEDIUM, "Medium");
            AppendMenu(hSubMenu, MF_STRING, IDM_HARD, "Hard");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "Difficulty");

            HMENU hThemeMenu = CreatePopupMenu();
            AppendMenu(hThemeMenu, MF_STRING | (currentTheme == 0 ? MF_CHECKED : 0), IDM_THEME_CLASSIC, "Classic Emerald");
            AppendMenu(hThemeMenu, MF_STRING | (currentTheme == 1 ? MF_CHECKED : 0), IDM_THEME_CYBER, "Midnight Cyber");
            AppendMenu(hThemeMenu, MF_STRING | (currentTheme == 2 ? MF_CHECKED : 0), IDM_THEME_CRIMSON, "Obsidian Crimson");
            AppendMenu(hThemeMenu, MF_STRING | (currentTheme == 3 ? MF_CHECKED : 0), IDM_THEME_TERMINAL, "Retro Terminal");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hThemeMenu, "Theme");
            
            HMENU hTimeMenu = CreatePopupMenu();
            AppendMenu(hTimeMenu, MF_STRING | MF_CHECKED, IDM_TIME_UNTIMED, "Untimed");
            AppendMenu(hTimeMenu, MF_STRING, IDM_TIME_BLITZ, "Blitz (5s)");
            AppendMenu(hTimeMenu, MF_STRING, IDM_TIME_RAPID, "Rapid (15s)");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hTimeMenu, "Timer");

            HMENU hHandicapMenu = CreatePopupMenu();
            AppendMenu(hHandicapMenu, MF_STRING | MF_CHECKED, IDM_HANDICAP_STD, "Standard");
            AppendMenu(hHandicapMenu, MF_STRING, IDM_HANDICAP_BLACK_CORNER, "Black Corner");
            AppendMenu(hHandicapMenu, MF_STRING, IDM_HANDICAP_WHITE_CORNER, "White Corner");
            AppendMenu(hHandicapMenu, MF_STRING, IDM_HANDICAP_BLACK_ADV, "Black Advantage");
            AppendMenu(hHandicapMenu, MF_STRING, IDM_HANDICAP_4CORNERS, "4 Corners");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHandicapMenu, "Handicap");

            HMENU hActionMenu = CreatePopupMenu();
            AppendMenu(hActionMenu, MF_STRING, IDM_SAVE, "Save Game");
            AppendMenu(hActionMenu, MF_STRING, IDM_LOAD, "Load Game");
            AppendMenu(hActionMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hActionMenu, MF_STRING, IDM_UNDO, "Undo Move");
            AppendMenu(hActionMenu, MF_STRING, IDM_STATS, "Statistics");
            AppendMenu(hActionMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hActionMenu, MF_STRING | MF_CHECKED, IDM_SOUND, "Sound Effects");
            AppendMenu(hActionMenu, MF_STRING | MF_CHECKED, IDM_HINT, "Show Hints");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hActionMenu, "Actions");
            
            SetMenu(hwnd, hMenu);
            LoadStats();
            InitGame(hwnd);
            break;
        }
        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case IDM_EASY:
                case IDM_MEDIUM:
                case IDM_HARD: {
                    aiDifficulty = LOWORD(wParam) - IDM_EASY;
                    HMENU hMenu = GetMenu(hwnd);
                    CheckMenuItem(hMenu, IDM_EASY, MF_UNCHECKED);
                    CheckMenuItem(hMenu, IDM_MEDIUM, MF_UNCHECKED);
                    CheckMenuItem(hMenu, IDM_HARD, MF_UNCHECKED);
                    CheckMenuItem(hMenu, LOWORD(wParam), MF_CHECKED);
                    break;
                }
                case IDM_THEME_CLASSIC:
                case IDM_THEME_CYBER:
                case IDM_THEME_CRIMSON:
                case IDM_THEME_TERMINAL: {
                    currentTheme = LOWORD(wParam) - IDM_THEME_CLASSIC;
                    HMENU hMenu = GetMenu(hwnd);
                    CheckMenuItem(hMenu, IDM_THEME_CLASSIC, MF_UNCHECKED);
                    CheckMenuItem(hMenu, IDM_THEME_CYBER, MF_UNCHECKED);
                    CheckMenuItem(hMenu, IDM_THEME_CRIMSON, MF_UNCHECKED);
                    CheckMenuItem(hMenu, IDM_THEME_TERMINAL, MF_UNCHECKED);
                    CheckMenuItem(hMenu, LOWORD(wParam), MF_CHECKED);
                    SaveTheme();
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                }
                case IDM_TIME_UNTIMED:
                case IDM_TIME_BLITZ:
                case IDM_TIME_RAPID: {
                    if (LOWORD(wParam) == IDM_TIME_UNTIMED) moveTimeMode = 0;
                    else if (LOWORD(wParam) == IDM_TIME_BLITZ) moveTimeMode = 5;
                    else if (LOWORD(wParam) == IDM_TIME_RAPID) moveTimeMode = 15;
                    
                    HMENU hMenu = GetMenu(hwnd);
                    CheckMenuItem(hMenu, IDM_TIME_UNTIMED, MF_UNCHECKED);
                    CheckMenuItem(hMenu, IDM_TIME_BLITZ, MF_UNCHECKED);
                    CheckMenuItem(hMenu, IDM_TIME_RAPID, MF_UNCHECKED);
                    CheckMenuItem(hMenu, LOWORD(wParam), MF_CHECKED);
                    
                    ResetMoveTimer(hwnd);
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                }
                case IDM_HANDICAP_STD:
                case IDM_HANDICAP_BLACK_CORNER:
                case IDM_HANDICAP_WHITE_CORNER:
                case IDM_HANDICAP_BLACK_ADV:
                case IDM_HANDICAP_4CORNERS: {
                    handicapMode = LOWORD(wParam) - IDM_HANDICAP_STD;
                    HMENU hMenu = GetMenu(hwnd);
                    CheckMenuItem(hMenu, IDM_HANDICAP_STD, MF_UNCHECKED);
                    CheckMenuItem(hMenu, IDM_HANDICAP_BLACK_CORNER, MF_UNCHECKED);
                    CheckMenuItem(hMenu, IDM_HANDICAP_WHITE_CORNER, MF_UNCHECKED);
                    CheckMenuItem(hMenu, IDM_HANDICAP_BLACK_ADV, MF_UNCHECKED);
                    CheckMenuItem(hMenu, IDM_HANDICAP_4CORNERS, MF_UNCHECKED);
                    CheckMenuItem(hMenu, LOWORD(wParam), MF_CHECKED);
                    
                    InitGame(hwnd);
                    break;
                }
                case IDM_UNDO: {
                    UndoMove(hwnd);
                    break;
                }
                case IDM_STATS: {
                    ShowStats(hwnd);
                    break;
                }
                case IDM_SAVE: {
                    SaveGame(hwnd);
                    break;
                }
                case IDM_LOAD: {
                    LoadGame(hwnd);
                    break;
                }
                case IDM_SOUND: {
                    soundEnabled = !soundEnabled;
                    CheckMenuItem(GetMenu(hwnd), IDM_SOUND, soundEnabled ? MF_CHECKED : MF_UNCHECKED);
                    break;
                }
                case IDM_HINT: {
                    showHint = !showHint;
                    CheckMenuItem(GetMenu(hwnd), IDM_HINT, showHint ? MF_CHECKED : MF_UNCHECKED);
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                }
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (currentPlayer != BLACK || gameEnded) break;
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            int c = x / 50;
            int r = (y - 40) / 50;
            
            if (r >= 0 && r < 8 && c >= 0 && c < 8) {
                int idx = r * 8 + c;
                int flips[64];
                int count = GetFlippable(idx, BLACK, flips);
                if (count > 0) {
                    PushHistory();
                    DoMove(idx, BLACK, hwnd);
                    currentPlayer = WHITE;
                    if (!HasValidMoves(WHITE)) {
                        currentPlayer = BLACK;
                    } else {
                        InvalidateRect(hwnd, NULL, TRUE);
                        SetTimer(hwnd, 1, 400, NULL); // small delay for AI
                    }
                    ResetMoveTimer(hwnd);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                AIMove(hwnd);
            } else if (wParam == 2) {
                flipProgress++;
                InvalidateRect(hwnd, NULL, FALSE);
                if (flipProgress >= 10) {
                    KillTimer(hwnd, 2);
                    numAnimatingFlips = 0;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (wParam == 3) {
                if (moveTimeMode > 0 && !gameEnded) {
                    moveTimeLeftDeci--;
                    if (moveTimeLeftDeci <= 0) {
                        KillTimer(hwnd, 3);
                        gameEnded = 1;
                        int bCount = 0, wCount = 0;
                        for(int i=0; i<64; i++) {
                            if(board[i] == BLACK) bCount++;
                            if(board[i] == WHITE) wCount++;
                        }
                        if (currentPlayer == BLACK) stats[1]++; else stats[0]++;
                        SaveStats();
                        PlaySoundEffect(2);
                        InvalidateRect(hwnd, NULL, TRUE);
                    } else {
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            const BoardTheme* theme = &g_themes[currentTheme];

            HBRUSH bgBrush = CreateSolidBrush(theme->bg);
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            FillRect(hdc, &clientRect, bgBrush);
            DeleteObject(bgBrush);

            SetBkMode(hdc, OPAQUE);
            SetBkColor(hdc, theme->bg);
            SetTextColor(hdc, theme->text);
            
            int bCount = 0, wCount = 0;
            for(int i=0; i<64; i++) {
                if(board[i] == BLACK) bCount++;
                if(board[i] == WHITE) wCount++;
            }
            
            char scoreStr[64];
            sprintf(scoreStr, "%s: %d   %s: %d", theme->p1Name, bCount, theme->p2Name, wCount);
            TextOut(hdc, 10, 8, scoreStr, strlen(scoreStr));
            
            if (gameEnded) {
                if (moveTimeMode > 0 && moveTimeLeftDeci <= 0) {
                    char timeoutStr[64];
                    sprintf(timeoutStr, "%s Timeout!", currentPlayer == BLACK ? theme->p1Name : theme->p2Name);
                    TextOut(hdc, 220, 8, timeoutStr, strlen(timeoutStr));
                } else if (bCount > wCount) {
                    char winStr[64];
                    sprintf(winStr, "%s Wins!", theme->p1Name);
                    TextOut(hdc, 220, 8, winStr, strlen(winStr));
                } else if (wCount > bCount) {
                    char winStr[64];
                    sprintf(winStr, "%s Wins!", theme->p2Name);
                    TextOut(hdc, 220, 8, winStr, strlen(winStr));
                } else TextOut(hdc, 220, 8, "Draw!", 5);
            } else if (!HasValidMoves(BLACK) && !HasValidMoves(WHITE)) {
                gameEnded = 1;
                KillTimer(hwnd, 3);
                if (bCount > wCount) {
                    char winStr[64];
                    sprintf(winStr, "%s Wins!", theme->p1Name);
                    TextOut(hdc, 220, 8, winStr, strlen(winStr));
                } else if (wCount > bCount) {
                    char winStr[64];
                    sprintf(winStr, "%s Wins!", theme->p2Name);
                    TextOut(hdc, 220, 8, winStr, strlen(winStr));
                } else TextOut(hdc, 220, 8, "Draw!", 5);
                
                if (!gameOverSoundPlayed) {
                    gameOverSoundPlayed = 1;
                    PlaySoundEffect(2);
                    UpdateStats(bCount, wCount);
                }
            } else {
                char turnStr[64];
                sprintf(turnStr, "Turn: %s", currentPlayer == BLACK ? theme->p1Name : theme->p2Name);
                TextOut(hdc, 220, 8, turnStr, strlen(turnStr));
            }

            int evalScore = EvaluatePosition();
            int bestMoveIdx = (currentPlayer == BLACK && HasValidMoves(BLACK) && !gameEnded) ? GetBestMoveForPlayer(BLACK) : -1;
            
            char evalStr[160];
            char clockStr[32];
            if (moveTimeMode > 0) {
                sprintf(clockStr, "Clock: %.1fs", moveTimeLeftDeci / 10.0);
            } else {
                sprintf(clockStr, "Clock: Off");
            }

            if (bestMoveIdx != -1 && showHint) {
                int col = bestMoveIdx % 8;
                int row = bestMoveIdx / 8;
                sprintf(evalStr, "Eval: %+d | Best: %c%d | %s", 
                        evalScore, 'A' + col, row + 1, clockStr);
            } else {
                sprintf(evalStr, "Eval: %+d | Best: - | %s", 
                        evalScore, clockStr);
            }
            TextOut(hdc, 10, 24, evalStr, strlen(evalStr));

            HBRUSH boardBrush = CreateSolidBrush(theme->boardCell);
            HBRUSH disc1Brush = CreateSolidBrush(theme->disc1);
            HBRUSH disc2Brush = CreateSolidBrush(theme->disc2);
            HPEN gridPen = CreatePen(PS_SOLID, 2, theme->gridPen);
            
            SelectObject(hdc, gridPen);
            
            for(int r = 0; r < 8; r++) {
                for(int c = 0; c < 8; c++) {
                    RECT rect = { c * 50, 40 + r * 50, (c + 1) * 50, 40 + (r + 1) * 50 };
                    SelectObject(hdc, boardBrush);
                    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
                    
                    int idx = r * 8 + c;
                    if (board[idx] != EMPTY) {
                        int isAnimating = 0;
                        if (numAnimatingFlips > 0 && flipProgress < 10) {
                            for(int k=0; k<numAnimatingFlips; k++) {
                                if (animatingFlips[k] == idx) { isAnimating = 1; break; }
                            }
                        }
                        
                        if (isAnimating) {
                            int oldColor = (board[idx] == BLACK) ? WHITE : BLACK;
                            int newColor = board[idx];
                            int displayColor = (flipProgress < 5) ? oldColor : newColor;
                            SelectObject(hdc, displayColor == BLACK ? disc1Brush : disc2Brush);
                            
                            int currentWidth = 40;
                            if (flipProgress < 5) {
                                currentWidth = 40 - (flipProgress * 8); 
                            } else {
                                currentWidth = (flipProgress - 5) * 8; 
                            }
                            if (currentWidth < 2) currentWidth = 2;
                            int margin = (40 - currentWidth) / 2;
                            Ellipse(hdc, rect.left + 5 + margin, rect.top + 5, rect.right - 5 - margin, rect.bottom - 5);
                        } else {
                            SelectObject(hdc, board[idx] == BLACK ? disc1Brush : disc2Brush);
                            Ellipse(hdc, rect.left + 5, rect.top + 5, rect.right - 5, rect.bottom - 5);
                        }
                    } else if (currentPlayer == BLACK && !gameEnded) {
                        int dummy[64];
                        if (GetFlippable(idx, BLACK, dummy) > 0) {
                            if (showHint && idx == bestMoveIdx) {
                                HBRUSH hintBrush = CreateSolidBrush(theme->hintPen);
                                HPEN hintPen = CreatePen(PS_SOLID, 2, theme->hintPen);
                                SelectObject(hdc, hintBrush);
                                SelectObject(hdc, hintPen);
                                Ellipse(hdc, rect.left + 14, rect.top + 14, rect.right - 14, rect.bottom - 14);
                                DeleteObject(hintBrush);
                                DeleteObject(hintPen);
                            } else {
                                SelectObject(hdc, GetStockObject(NULL_BRUSH));
                                HPEN hintPen = CreatePen(PS_SOLID, 1, theme->hintPen);
                                SelectObject(hdc, hintPen);
                                Ellipse(hdc, rect.left + 20, rect.top + 20, rect.right - 20, rect.bottom - 20);
                                DeleteObject(hintPen);
                            }
                        }
                    }
                }
            }
            
            DeleteObject(boardBrush);
            DeleteObject(disc1Brush);
            DeleteObject(disc2Brush);
            DeleteObject(gridPen);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(18, 18, 18));
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "KReversi",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 480,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
