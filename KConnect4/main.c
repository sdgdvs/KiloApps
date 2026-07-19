#include <windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

const char g_szClassName[] = "KConnect4WindowClass";
#define ROWS 6
#define COLS 7

int board[ROWS][COLS];
int currentPlayer = 1;
bool gameActive = true;
bool isDraw = false;
bool vsAI = true;

int winCells[7][2];
int winCellCount = 0;
HWND hModeBtn, hResetBtn;

void ResetGame() {
    for(int r=0; r<ROWS; r++)
        for(int c=0; c<COLS; c++)
            board[r][c] = 0;
    currentPlayer = 1;
    gameActive = true;
    isDraw = false;
    winCellCount = 0;
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

void AIMove(HWND hwnd) {
    if (!gameActive) return;
    int available[COLS];
    int count = 0;
    for(int c=0; c<COLS; c++) {
        if (board[0][c] == 0) available[count++] = c;
    }
    if (count == 0) return;
    
    int bestCol = -1;
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
    
    for (int r = ROWS - 1; r >= 0; r--) {
        if (board[r][bestCol] == 0) {
            board[r][bestCol] = 2;
            if (CheckWin(r, bestCol, 2)) {
                gameActive = false;
            } else if (CheckDraw()) {
                gameActive = false;
                isDraw = true;
            } else {
                currentPlayer = 1;
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            srand((unsigned int)time(NULL));
            hModeBtn = CreateWindow("BUTTON", "Mode: vs AI", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 20, 340, 150, 30, hwnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hResetBtn = CreateWindow("BUTTON", "Reset", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 180, 340, 100, 30, hwnd, (HMENU)2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            ResetGame();
            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                vsAI = !vsAI;
                SetWindowText(hModeBtn, vsAI ? "Mode: vs AI" : "Mode: 2 Player");
                ResetGame();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 2) {
                ResetGame();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case WM_TIMER:
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                AIMove(hwnd);
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
            
            // Draw board background
            HBRUSH boardBg = CreateSolidBrush(RGB(31, 66, 135));
            RECT boardRect = {20, 50, 20 + COLS * 45 + 5, 50 + ROWS * 45 + 5};
            FillRect(hdc, &boardRect, boardBg);
            DeleteObject(boardBg);
            
            // Draw cells
            HBRUSH emptyCell = CreateSolidBrush(RGB(18, 18, 18));
            HBRUSH p1Cell = CreateSolidBrush(RGB(255, 82, 82));
            HBRUSH p2Cell = CreateSolidBrush(RGB(255, 235, 59));
            HPEN hlPen = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
            HPEN nullPen = GetStockObject(NULL_PEN);
            
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

                    if (board[r][c] == 1) SelectObject(hdc, p1Cell);
                    else if (board[r][c] == 2) SelectObject(hdc, p2Cell);
                    else SelectObject(hdc, emptyCell);
                    
                    if (isWinCell) {
                        SelectObject(hdc, hlPen);
                    } else {
                        SelectObject(hdc, nullPen);
                    }
                    
                    Ellipse(hdc, x, y, x + 40, y + 40);
                }
            }
            SelectObject(hdc, nullPen);
            DeleteObject(emptyCell);
            DeleteObject(p1Cell);
            DeleteObject(p2Cell);
            DeleteObject(hlPen);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN: {
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
                    for (int r = ROWS - 1; r >= 0; r--) {
                        if (board[r][c] == 0) {
                            board[r][c] = currentPlayer;
                            if (CheckWin(r, c, currentPlayer)) {
                                gameActive = false;
                            } else if (CheckDraw()) {
                                gameActive = false;
                                isDraw = true;
                            } else {
                                currentPlayer = currentPlayer == 1 ? 2 : 1;
                                if (vsAI && currentPlayer == 2) {
                                    SetTimer(hwnd, 1, 300, NULL);
                                }
                            }
                            InvalidateRect(hwnd, NULL, TRUE);
                            break;
                        }
                    }
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
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 450,
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
