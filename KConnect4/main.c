#include <windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

const char g_szClassName[] = "KConnect4WindowClass";
#define ROWS 6
#define COLS 7

int board[ROWS][COLS];
int currentPlayer = 1;
bool gameActive = true;
bool isDraw = false;
bool vsAI = true;
int aiDifficulty = 0;

bool isAnimating = false;
int animPlayer = 0;
int animRow = -1;
int animCol = -1;
int animY = 0;
int animTargetY = 0;

int hoverCol = -1;

int winCells[7][2];
int winCellCount = 0;
HWND hModeBtn, hDiffSelect, hUndoBtn, hResetBtn, hMuteBtn;
bool isMuted = false;

void PlaySoundEffect(int type) {
    if (isMuted) return;
    if (type == 1) { // Drop
        Beep(400, 50);
    } else if (type == 2) { // Win
        Beep(400, 100);
        Beep(500, 100);
        Beep(600, 100);
        Beep(800, 150);
    } else if (type == 3) { // Lose
        Beep(300, 200);
        Beep(200, 250);
    } else if (type == 4) { // Invalid
        Beep(150, 100);
    } else if (type == 5) { // Draw
        Beep(300, 300);
    }
}

typedef struct {
    int redWins;
    int yellowWins;
    int draws;
    int streak;
    int bestStreak;
    int lastWinner;
} GameStats;
GameStats stats = {0,0,0,0,0,0};

typedef struct {
    int r;
    int c;
    int p;
    GameStats oldStats;
} MoveRecord;

MoveRecord moveHistory[ROWS * COLS];
int historyCount = 0;

void LoadStats() {
    FILE *f = fopen("kconnect4_stats.bin", "rb");
    if(f) {
        fread(&stats, sizeof(GameStats), 1, f);
        fclose(f);
    }
}
void SaveStats() {
    FILE *f = fopen("kconnect4_stats.bin", "wb");
    if(f) {
        fwrite(&stats, sizeof(GameStats), 1, f);
        fclose(f);
    }
}

void ResetGame() {
    for(int r=0; r<ROWS; r++)
        for(int c=0; c<COLS; c++)
            board[r][c] = 0;
    currentPlayer = 1;
    gameActive = true;
    isDraw = false;
    isAnimating = false;
    winCellCount = 0;
    historyCount = 0;
}

bool CheckWin(int r, int c, int p) {
    int dirs[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
    for(int d=0; d<4; d++) {
        int count = 1;
        int tempCells[7][2];
        tempCells[0][0] = r;
        tempCells[0][1] = c;
        for(int i=1; i<4; i++) {
            int nr = r + dirs[d][0]*i;
            int nc = c + dirs[d][1]*i;
            if(nr>=0 && nr<ROWS && nc>=0 && nc<COLS && board[nr][nc]==p) {
                tempCells[count][0] = nr;
                tempCells[count][1] = nc;
                count++;
            }
            else break;
        }
        for(int i=1; i<4; i++) {
            int nr = r - dirs[d][0]*i;
            int nc = c - dirs[d][1]*i;
            if(nr>=0 && nr<ROWS && nc>=0 && nc<COLS && board[nr][nc]==p) {
                tempCells[count][0] = nr;
                tempCells[count][1] = nc;
                count++;
            }
            else break;
        }
        if(count >= 4) {
            for(int k=0; k<count; k++) {
                winCells[k][0] = tempCells[k][0];
                winCells[k][1] = tempCells[k][1];
            }
            winCellCount = count;
            return true;
        }
    }
    return false;
}

bool CheckDraw() {
    for(int c=0; c<COLS; c++) {
        if(board[0][c] == 0) return false;
    }
    return true;
}

bool SimDrop(int col, int p) {
    for (int r = ROWS - 1; r >= 0; r--) {
        if (board[r][col] == 0) {
            board[r][col] = p;
            int tempWin[7][2];
            int tempCount = winCellCount;
            for(int i=0; i<7; i++) { tempWin[i][0]=winCells[i][0]; tempWin[i][1]=winCells[i][1]; }
            bool win = CheckWin(r, col, p);
            for(int i=0; i<7; i++) { winCells[i][0]=tempWin[i][0]; winCells[i][1]=tempWin[i][1]; }
            winCellCount = tempCount;
            board[r][col] = 0;
            return win;
        }
    }
    return false;
}

int evaluateWindow(int w[4], int piece) {
    int score = 0;
    int oppPiece = (piece == 1) ? 2 : 1;
    int pc = 0, ec = 0, oc = 0;
    for(int i=0; i<4; i++) {
        if(w[i] == piece) pc++;
        else if(w[i] == 0) ec++;
        else if(w[i] == oppPiece) oc++;
    }
    if (pc == 4) score += 100;
    else if (pc == 3 && ec == 1) score += 5;
    else if (pc == 2 && ec == 2) score += 2;
    if (oc == 3 && ec == 1) score -= 4;
    return score;
}

int scoreBoard(int b[ROWS][COLS], int piece) {
    int score = 0;
    int centerCount = 0;
    for (int r=0; r<ROWS; r++) if (b[r][3] == piece) centerCount++;
    score += centerCount * 3;

    for (int r=0; r<ROWS; r++) {
        for (int c=0; c<COLS-3; c++) {
            int w[4] = {b[r][c], b[r][c+1], b[r][c+2], b[r][c+3]};
            score += evaluateWindow(w, piece);
        }
    }
    for (int c=0; c<COLS; c++) {
        for (int r=0; r<ROWS-3; r++) {
            int w[4] = {b[r][c], b[r+1][c], b[r+2][c], b[r+3][c]};
            score += evaluateWindow(w, piece);
        }
    }
    for (int r=0; r<ROWS-3; r++) {
        for (int c=0; c<COLS-3; c++) {
            int w[4] = {b[r][c], b[r+1][c+1], b[r+2][c+2], b[r+3][c+3]};
            score += evaluateWindow(w, piece);
        }
    }
    for (int r=3; r<ROWS; r++) {
        for (int c=0; c<COLS-3; c++) {
            int w[4] = {b[r][c], b[r-1][c+1], b[r-2][c+2], b[r-3][c+3]};
            score += evaluateWindow(w, piece);
        }
    }
    return score;
}

bool checkWinBoard(int b[ROWS][COLS], int p) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS - 3; c++) {
            if (b[r][c] == p && b[r][c+1] == p && b[r][c+2] == p && b[r][c+3] == p) return true;
        }
    }
    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r < ROWS - 3; r++) {
            if (b[r][c] == p && b[r+1][c] == p && b[r+2][c] == p && b[r+3][c] == p) return true;
        }
    }
    for (int r = 0; r < ROWS - 3; r++) {
        for (int c = 0; c < COLS - 3; c++) {
            if (b[r][c] == p && b[r+1][c+1] == p && b[r+2][c+2] == p && b[r+3][c+3] == p) return true;
        }
    }
    for (int r = 3; r < ROWS; r++) {
        for (int c = 0; c < COLS - 3; c++) {
            if (b[r][c] == p && b[r-1][c+1] == p && b[r-2][c+2] == p && b[r-3][c+3] == p) return true;
        }
    }
    return false;
}

int getValidLocations(int b[ROWS][COLS], int locs[COLS]) {
    int count = 0;
    int pref[COLS] = {3, 2, 4, 1, 5, 0, 6};
    for(int i=0; i<COLS; i++) {
        if(b[0][pref[i]] == 0) locs[count++] = pref[i];
    }
    return count;
}

bool isTerminalNode(int b[ROWS][COLS]) {
    if(checkWinBoard(b, 1) || checkWinBoard(b, 2)) return true;
    for(int c=0; c<COLS; c++) if(b[0][c] == 0) return false;
    return true;
}

int getNextOpenRow(int b[ROWS][COLS], int c) {
    for (int r = ROWS - 1; r >= 0; r--) {
        if (b[r][c] == 0) return r;
    }
    return -1;
}

typedef struct {
    int col;
    int score;
} MMResult;

MMResult minimax(int b[ROWS][COLS], int depth, int alpha, int beta, bool maximizingPlayer) {
    int validLocations[COLS];
    int count = getValidLocations(b, validLocations);
    bool isTerminal = isTerminalNode(b);
    
    if (depth == 0 || isTerminal) {
        MMResult res;
        res.col = -1;
        if (isTerminal) {
            if (checkWinBoard(b, 2)) res.score = 10000000;
            else if (checkWinBoard(b, 1)) res.score = -10000000;
            else res.score = 0;
        } else {
            res.score = scoreBoard(b, 2);
        }
        return res;
    }
    
    if (maximizingPlayer) {
        int value = -20000000;
        int column = validLocations[rand() % count];
        for (int i=0; i<count; i++) {
            int c = validLocations[i];
            int r = getNextOpenRow(b, c);
            b[r][c] = 2;
            MMResult newRes = minimax(b, depth - 1, alpha, beta, false);
            b[r][c] = 0;
            if (newRes.score > value) {
                value = newRes.score;
                column = c;
            }
            if (value > alpha) alpha = value;
            if (alpha >= beta) break;
        }
        MMResult res = {column, value};
        return res;
    } else {
        int value = 20000000;
        int column = validLocations[rand() % count];
        for (int i=0; i<count; i++) {
            int c = validLocations[i];
            int r = getNextOpenRow(b, c);
            b[r][c] = 1;
            MMResult newRes = minimax(b, depth - 1, alpha, beta, true);
            b[r][c] = 0;
            if (newRes.score < value) {
                value = newRes.score;
                column = c;
            }
            if (value < beta) beta = value;
            if (alpha >= beta) break;
        }
        MMResult res = {column, value};
        return res;
    }
}

void AIMove(HWND hwnd) {
    if (!gameActive) return;
    
    int bestCol = -1;
    if (aiDifficulty == 0) {
        int available[COLS];
        int count = 0;
        for(int c=0; c<COLS; c++) {
            if (board[0][c] == 0) available[count++] = c;
        }
        if (count == 0) return;
        
        for(int i=0; i<count; i++) {
            if(SimDrop(available[i], 2)) { bestCol = available[i]; break; }
        }
        if(bestCol == -1) {
            for(int i=0; i<count; i++) {
                if(SimDrop(available[i], 1)) { bestCol = available[i]; break; }
            }
        }
        if(bestCol == -1) {
            bestCol = available[rand() % count];
        }
    } else {
        int depth = (aiDifficulty == 1) ? 3 : 6;
        MMResult res = minimax(board, depth, -20000000, 20000000, true);
        bestCol = res.col;
        if (bestCol == -1) {
            int validLocations[COLS];
            int count = getValidLocations(board, validLocations);
            if (count > 0) bestCol = validLocations[rand() % count];
        }
    }
    
    if (bestCol != -1) {
        for (int r = ROWS - 1; r >= 0; r--) {
            if (board[r][bestCol] == 0) {
                PlaySoundEffect(1);
                moveHistory[historyCount].r = r;
                moveHistory[historyCount].c = bestCol;
                moveHistory[historyCount].p = 2;
                moveHistory[historyCount].oldStats = stats;
                historyCount++;

                board[r][bestCol] = 2;
                animPlayer = 2;
                animRow = r;
                animCol = bestCol;
                animY = 50;
                animTargetY = 50 + 5 + r * 45;
                isAnimating = true;
                SetTimer(hwnd, 2, 16, NULL);
                break;
            }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            srand((unsigned int)time(NULL));
            LoadStats();
            hModeBtn = CreateWindow("BUTTON", "Mode: vs AI", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 340, 100, 30, hwnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hDiffSelect = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 120, 342, 80, 100, hwnd, (HMENU)4, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(hDiffSelect, CB_ADDSTRING, 0, (LPARAM)"Easy");
            SendMessage(hDiffSelect, CB_ADDSTRING, 0, (LPARAM)"Medium");
            SendMessage(hDiffSelect, CB_ADDSTRING, 0, (LPARAM)"Hard");
            SendMessage(hDiffSelect, CB_SETCURSEL, 0, 0);
            hUndoBtn = CreateWindow("BUTTON", "Undo", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 210, 340, 70, 30, hwnd, (HMENU)3, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hResetBtn = CreateWindow("BUTTON", "Reset", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 290, 340, 70, 30, hwnd, (HMENU)2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hMuteBtn = CreateWindow("BUTTON", "Mute", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 380, 70, 30, hwnd, (HMENU)5, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            ResetGame();
            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                vsAI = !vsAI;
                SetWindowText(hModeBtn, vsAI ? "Mode: vs AI" : "Mode: 2 Player");
                ShowWindow(hDiffSelect, vsAI ? SW_SHOW : SW_HIDE);
                ResetGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 4 && HIWORD(wParam) == CBN_SELCHANGE) {
                aiDifficulty = SendMessage(hDiffSelect, CB_GETCURSEL, 0, 0);
            } else if (LOWORD(wParam) == 2) {
                ResetGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 5) {
                isMuted = !isMuted;
                SetWindowText(hMuteBtn, isMuted ? "Unmute" : "Mute");
            } else if (LOWORD(wParam) == 3) {
                if (historyCount == 0) break;
                if (isAnimating) break; // Don't undo during animation to prevent glitches
                
                // If AI is scheduled to move, kill the timer
                KillTimer(hwnd, 1);
                
                int toPop = 1;
                if (vsAI) {
                    if (gameActive && currentPlayer == 1 && historyCount >= 2) toPop = 2;
                    if (!gameActive && historyCount >= 2 && moveHistory[historyCount - 1].p == 2) toPop = 2;
                }
                
                for (int i = 0; i < toPop; i++) {
                    if (historyCount > 0) {
                        historyCount--;
                        MoveRecord m = moveHistory[historyCount];
                        board[m.r][m.c] = 0;
                        stats = m.oldStats;
                        currentPlayer = m.p;
                    }
                }
                
                SaveStats();
                gameActive = true;
                isDraw = false;
                winCellCount = 0;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case WM_TIMER:
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                AIMove(hwnd);
            } else if (wParam == 2) {
                animY += 25;
                if (animY >= animTargetY) {
                    animY = animTargetY;
                    isAnimating = false;
                    KillTimer(hwnd, 2);
                    
                    if (CheckWin(animRow, animCol, animPlayer)) {
                        if (vsAI && animPlayer == 2) PlaySoundEffect(3);
                        else PlaySoundEffect(2);
                        gameActive = false;
                        if(animPlayer == 1) stats.redWins++;
                        else stats.yellowWins++;
                        if(stats.lastWinner == animPlayer) stats.streak++;
                        else stats.streak = 1;
                        stats.lastWinner = animPlayer;
                        if(stats.streak > stats.bestStreak) stats.bestStreak = stats.streak;
                        SaveStats();
                    } else if (CheckDraw()) {
                        PlaySoundEffect(5);
                        gameActive = false;
                        isDraw = true;
                        stats.draws++;
                        stats.streak = 0;
                        stats.lastWinner = 0;
                        SaveStats();
                    } else {
                        currentPlayer = animPlayer == 1 ? 2 : 1;
                        if (vsAI && currentPlayer == 2 && gameActive) {
                            SetTimer(hwnd, 1, 300, NULL);
                        }
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            HBRUSH bg = CreateSolidBrush(RGB(18, 18, 18));
            FillRect(hdc, &rect, bg);
            DeleteObject(bg);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(224, 224, 224));
            char statusText[64];
            if (!gameActive) {
                if (isDraw) wsprintf(statusText, "It's a Draw! Click to reset.");
                else wsprintf(statusText, "Player %d Wins! Click to reset.", currentPlayer);
            } else {
                wsprintf(statusText, "Player %d's turn", currentPlayer);
            }
            TextOut(hdc, 20, 20, statusText, lstrlen(statusText));
            
            char statsStr[128];
            wsprintf(statsStr, "Wins: Red %d, Yellow %d | Draws: %d | Streak: %d (Best: %d)",
                     stats.redWins, stats.yellowWins, stats.draws, stats.streak, stats.bestStreak);
            TextOut(hdc, 20, 420, statsStr, lstrlen(statsStr));
            
            // Draw board background
            HBRUSH boardBg = CreateSolidBrush(RGB(31, 66, 135));
            RECT boardRect = {20, 50, 20 + COLS * 45 + 5, 50 + ROWS * 45 + 5};
            FillRect(hdc, &boardRect, boardBg);
            DeleteObject(boardBg);
            
            HRGN hRgn = CreateRectRgn(20, 50, 20 + COLS * 45 + 5, 50 + ROWS * 45 + 5);
            SelectClipRgn(hdc, hRgn);

            // Draw cells
            HBRUSH emptyCell = CreateSolidBrush(RGB(18, 18, 18));
            HBRUSH p1Cell = CreateSolidBrush(RGB(255, 82, 82));
            HBRUSH p2Cell = CreateSolidBrush(RGB(255, 235, 59));
            HBRUSH hoverP1 = CreateSolidBrush(RGB(120, 40, 40));
            HBRUSH hoverP2 = CreateSolidBrush(RGB(120, 110, 20));
            HPEN hlPen = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
            HPEN nullPen = GetStockObject(NULL_PEN);

            int hoverRow = -1;
            if (hoverCol != -1 && !isAnimating && gameActive && !(vsAI && currentPlayer == 2)) {
                for (int r = ROWS - 1; r >= 0; r--) {
                    if (board[r][hoverCol] == 0) {
                        hoverRow = r;
                        break;
                    }
                }
            }
            
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    int x = 20 + 5 + c * 45;
                    int y = 50 + 5 + r * 45;
                    
                    bool isWinCell = false;
                    for(int i=0; i<winCellCount; i++) {
                        if(winCells[i][0] == r && winCells[i][1] == c) {
                            isWinCell = true;
                            break;
                        }
                    }

                    if (isAnimating && r == animRow && c == animCol) {
                        SelectObject(hdc, emptyCell);
                    } else if (board[r][c] == 1) SelectObject(hdc, p1Cell);
                    else if (board[r][c] == 2) SelectObject(hdc, p2Cell);
                    else if (r == hoverRow && c == hoverCol) {
                        SelectObject(hdc, currentPlayer == 1 ? hoverP1 : hoverP2);
                    } else SelectObject(hdc, emptyCell);
                    
                    if (isWinCell) {
                        SelectObject(hdc, hlPen);
                    } else {
                        SelectObject(hdc, nullPen);
                    }
                    
                    Ellipse(hdc, x, y, x + 40, y + 40);
                }
            }
            
            if (isAnimating) {
                int x = 20 + 5 + animCol * 45;
                if (animPlayer == 1) SelectObject(hdc, p1Cell);
                else SelectObject(hdc, p2Cell);
                SelectObject(hdc, nullPen);
                Ellipse(hdc, x, animY, x + 40, animY + 40);
            }
            
            SelectClipRgn(hdc, NULL);
            DeleteObject(hRgn);

            SelectObject(hdc, nullPen);
            DeleteObject(emptyCell);
            DeleteObject(p1Cell);
            DeleteObject(p2Cell);
            DeleteObject(hoverP1);
            DeleteObject(hoverP2);
            DeleteObject(hlPen);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_MOUSEMOVE: {
            if (isAnimating || !gameActive || (vsAI && currentPlayer == 2)) {
                if (hoverCol != -1) {
                    hoverCol = -1;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                break;
            }
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);
            
            if (xPos >= 25 && xPos <= 25 + COLS * 45 && yPos >= 55 && yPos <= 55 + ROWS * 45) {
                int c = (xPos - 25) / 45;
                if (c >= 0 && c < COLS) {
                    if (hoverCol != c) {
                        hoverCol = c;
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                } else if (hoverCol != -1) {
                    hoverCol = -1;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (hoverCol != -1) {
                hoverCol = -1;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
            break;
        }
        case WM_MOUSELEAVE: {
            if (hoverCol != -1) {
                hoverCol = -1;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (isAnimating) break;
            if (!gameActive) {
                ResetGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            if (vsAI && currentPlayer == 2) break;
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);
            
            if (xPos >= 25 && xPos <= 25 + COLS * 45 && yPos >= 55 && yPos <= 55 + ROWS * 45) {
                int c = (xPos - 25) / 45;
                if (c >= 0 && c < COLS) {
                    bool dropped = false;
                    for (int r = ROWS - 1; r >= 0; r--) {
                        if (board[r][c] == 0) {
                            dropped = true;
                            PlaySoundEffect(1);
                            moveHistory[historyCount].r = r;
                            moveHistory[historyCount].c = c;
                            moveHistory[historyCount].p = currentPlayer;
                            moveHistory[historyCount].oldStats = stats;
                            historyCount++;

                            board[r][c] = currentPlayer;
                            animPlayer = currentPlayer;
                            animRow = r;
                            animCol = c;
                            animY = 50;
                            animTargetY = 50 + 5 + r * 45;
                            isAnimating = true;
                            SetTimer(hwnd, 2, 16, NULL);
                            break;
                        }
                    }
                    if (!dropped) PlaySoundEffect(4);
                }
            }
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
        0, g_szClassName, "KConnect4",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 480,
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
