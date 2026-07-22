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

const char g_szClassName[] = "KReversiClass";

int board[64];
int soundEnabled = 1;
int stats[3] = {0, 0, 0}; // Wins, Losses, Draws

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


void InitGame() {
    for(int i = 0; i < 64; i++) board[i] = EMPTY;
    board[27] = WHITE; board[28] = BLACK;
    board[35] = BLACK; board[36] = WHITE;
    currentPlayer = BLACK;
    historyCount = 0;
    gameOverSoundPlayed = 0;
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
    KillTimer(hwnd, 1); // Cancel AI timer if running
    KillTimer(hwnd, 2); // Cancel animation timer
    numAnimatingFlips = 0;
    if (historyCount == 0) return;
    
    HistoryState lastState = history[--historyCount];
    while(lastState.currentPlayer != BLACK && historyCount > 0) {
        lastState = history[--historyCount];
    }
    
    for(int i = 0; i < 64; i++) board[i] = lastState.board[i];
    currentPlayer = lastState.currentPlayer;
    
    gameOverSoundPlayed = 0;
    InvalidateRect(hwnd, NULL, TRUE);
}

void SaveGame(HWND hwnd) {
    FILE* f = fopen("kreversi_save.dat", "wb");
    if (f) {
        fwrite(board, sizeof(int), 64, f);
        fwrite(&currentPlayer, sizeof(int), 1, f);
        fwrite(&historyCount, sizeof(int), 1, f);
        fwrite(history, sizeof(HistoryState), historyCount, f);
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
        fclose(f);
        KillTimer(hwnd, 1);
        KillTimer(hwnd, 2);
        numAnimatingFlips = 0;
        gameOverSoundPlayed = 0;
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
    if (!HasValidMoves(WHITE)) {
        currentPlayer = BLACK;
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
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            srand((unsigned int)time(NULL));
            HMENU hMenu = CreateMenu();
            HMENU hSubMenu = CreatePopupMenu();
            AppendMenu(hSubMenu, MF_STRING, IDM_EASY, "Easy");
            AppendMenu(hSubMenu, MF_STRING | MF_CHECKED, IDM_MEDIUM, "Medium");
            AppendMenu(hSubMenu, MF_STRING, IDM_HARD, "Hard");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "Difficulty");
            
            HMENU hActionMenu = CreatePopupMenu();
            AppendMenu(hActionMenu, MF_STRING, IDM_SAVE, "Save Game");
            AppendMenu(hActionMenu, MF_STRING, IDM_LOAD, "Load Game");
            AppendMenu(hActionMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hActionMenu, MF_STRING, IDM_UNDO, "Undo Move");
            AppendMenu(hActionMenu, MF_STRING, IDM_STATS, "Statistics");
            AppendMenu(hActionMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hActionMenu, MF_STRING | MF_CHECKED, IDM_SOUND, "Sound Effects");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hActionMenu, "Actions");
            
            SetMenu(hwnd, hMenu);
            LoadStats();
            InitGame();
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
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (currentPlayer != BLACK) break;
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
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            SetBkMode(hdc, OPAQUE);
            SetBkColor(hdc, RGB(18, 18, 18));
            SetTextColor(hdc, RGB(220, 220, 220));
            
            int bCount = 0, wCount = 0;
            for(int i=0; i<64; i++) {
                if(board[i] == BLACK) bCount++;
                if(board[i] == WHITE) wCount++;
            }
            
            char scoreStr[64];
            sprintf(scoreStr, "Black: %d   White: %d", bCount, wCount);
            TextOut(hdc, 10, 10, scoreStr, strlen(scoreStr));
            
            if (!HasValidMoves(BLACK) && !HasValidMoves(WHITE)) {
                if (bCount > wCount) TextOut(hdc, 200, 10, "Black Wins!", 11);
                else if (wCount > bCount) TextOut(hdc, 200, 10, "White Wins!", 11);
                else TextOut(hdc, 200, 10, "Draw!", 5);
                
                if (!gameOverSoundPlayed) {
                    gameOverSoundPlayed = 1;
                    PlaySoundEffect(2);
                    UpdateStats(bCount, wCount);
                }
            } else {
                if (currentPlayer == BLACK) TextOut(hdc, 200, 10, "Turn: Black", 11);
                else TextOut(hdc, 200, 10, "Turn: White", 11);
            }

            HBRUSH boardBrush = CreateSolidBrush(RGB(26, 86, 44));
            HBRUSH blackBrush = CreateSolidBrush(RGB(20, 20, 20));
            HBRUSH whiteBrush = CreateSolidBrush(RGB(220, 220, 220));
            HPEN gridPen = CreatePen(PS_SOLID, 2, RGB(34, 34, 34));
            
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
                            SelectObject(hdc, displayColor == BLACK ? blackBrush : whiteBrush);
                            
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
                            SelectObject(hdc, board[idx] == BLACK ? blackBrush : whiteBrush);
                            Ellipse(hdc, rect.left + 5, rect.top + 5, rect.right - 5, rect.bottom - 5);
                        }
                    } else if (currentPlayer == BLACK) {
                        int dummy[64];
                        if (GetFlippable(idx, BLACK, dummy) > 0) {
                            SelectObject(hdc, GetStockObject(NULL_BRUSH));
                            HPEN hintPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
                            SelectObject(hdc, hintPen);
                            Ellipse(hdc, rect.left + 20, rect.top + 20, rect.right - 20, rect.bottom - 20);
                            DeleteObject(hintPen);
                        }
                    }
                }
            }
            
            DeleteObject(boardBrush);
            DeleteObject(blackBrush);
            DeleteObject(whiteBrush);
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
