#include <windows.h>
#include <string.h>
#include <stdio.h>

#define ID_BTN_PASS 101
#define ID_BTN_RESIGN 102
#define ID_BTN_NEW 103
#define ID_CB_SIZE 104
#define ID_BTN_SCORE 105

int boardSize = 9;
char board[19][19] = {0}; // 0 = empty, 1 = black, 2 = white
char prevBoard[19][19] = {0};
int currentPlayer = 1;
int captures[3] = {0}; // 1 = black, 2 = white
int hoverX = -1, hoverY = -1;
int animX = -1, animY = -1;
int animRadius = 13;

void CopyBoard(char dst[19][19], char src[19][19]) {
    memcpy(dst, src, sizeof(char) * 19 * 19);
}

int GetLiberties(int x, int y, int color, char visited[19][19]) {
    if (x < 0 || x >= boardSize || y < 0 || y >= boardSize) return 0;
    if (visited[y][x]) return 0;
    visited[y][x] = 1;
    
    if (board[y][x] == 0) return 1;
    if (board[y][x] != color) return 0;
    
    return GetLiberties(x-1, y, color, visited) +
           GetLiberties(x+1, y, color, visited) +
           GetLiberties(x, y-1, color, visited) +
           GetLiberties(x, y+1, color, visited);
}

void GetGroup(int x, int y, int color, char visited[19][19], POINT group[], int *groupSize) {
    if (x < 0 || x >= boardSize || y < 0 || y >= boardSize) return;
    if (visited[y][x]) return;
    if (board[y][x] != color) return;
    
    visited[y][x] = 1;
    group[*groupSize].x = x;
    group[*groupSize].y = y;
    (*groupSize)++;
    
    GetGroup(x-1, y, color, visited, group, groupSize);
    GetGroup(x+1, y, color, visited, group, groupSize);
    GetGroup(x, y-1, color, visited, group, groupSize);
    GetGroup(x, y+1, color, visited, group, groupSize);
}

int CheckCaptures(int color) {
    int totalCaptured = 0;
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            if (board[y][x] == color) {
                char visited[19][19] = {0};
                if (GetLiberties(x, y, color, visited) == 0) {
                    char groupVisited[19][19] = {0};
                    POINT group[19*19];
                    int groupSize = 0;
                    GetGroup(x, y, color, groupVisited, group, &groupSize);
                    for (int i = 0; i < groupSize; i++) {
                        board[group[i].y][group[i].x] = 0;
                        totalCaptured++;
                    }
                }
            }
        }
    }
    if (totalCaptured > 0) {
        if (color == 1) captures[2] += totalCaptured;
        else captures[1] += totalCaptured;
    }
    return totalCaptured;
}

void PlaceStone(HWND hwnd, int x, int y) {
    if (board[y][x] != 0) return;
    
    char boardBackup[19][19];
    CopyBoard(boardBackup, board);
    
    board[y][x] = (char)currentPlayer;
    
    int opp = currentPlayer == 1 ? 2 : 1;
    CheckCaptures(opp);
    
    char visited[19][19] = {0};
    if (GetLiberties(x, y, currentPlayer, visited) == 0) {
        CopyBoard(board, boardBackup);
        MessageBox(hwnd, "Suicide move is not allowed.", "Invalid Move", MB_OK);
        return;
    }
    
    if (memcmp(board, prevBoard, sizeof(char) * 19 * 19) == 0) {
        CopyBoard(board, boardBackup);
        MessageBox(hwnd, "Ko rule violation.", "Invalid Move", MB_OK);
        return;
    }
    
    CopyBoard(prevBoard, boardBackup);
    currentPlayer = opp;
    animX = x;
    animY = y;
    animRadius = 0;
    SetTimer(hwnd, 1, 16, NULL);
    InvalidateRect(hwnd, NULL, TRUE);
}

void InitBoard() {
    memset(board, 0, sizeof(board));
    memset(prevBoard, 0, sizeof(prevBoard));
    currentPlayer = 1;
    captures[1] = 0;
    captures[2] = 0;
    animX = -1;
    animY = -1;
}

void CalculateScore(HWND hwnd) {
    int terrB = 0;
    int terrW = 0;
    char visited[19][19] = {0};
    
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            if (board[y][x] == 0 && !visited[y][x]) {
                int touchesB = 0;
                int touchesW = 0;
                
                POINT queue[19*19];
                int head = 0, tail = 0;
                queue[tail].x = x;
                queue[tail].y = y;
                tail++;
                visited[y][x] = 1;
                
                int emptyCount = 0;
                
                while (head < tail) {
                    POINT p = queue[head++];
                    emptyCount++;
                    
                    int dx[] = {0, 1, 0, -1};
                    int dy[] = {1, 0, -1, 0};
                    for (int i = 0; i < 4; i++) {
                        int nx = p.x + dx[i];
                        int ny = p.y + dy[i];
                        if (nx >= 0 && nx < boardSize && ny >= 0 && ny < boardSize) {
                            if (board[ny][nx] == 1) touchesB = 1;
                            else if (board[ny][nx] == 2) touchesW = 1;
                            else if (!visited[ny][nx]) {
                                visited[ny][nx] = 1;
                                queue[tail].x = nx;
                                queue[tail].y = ny;
                                tail++;
                            }
                        }
                    }
                }
                
                if (touchesB && !touchesW) terrB += emptyCount;
                if (touchesW && !touchesB) terrW += emptyCount;
            }
        }
    }
    
    float totalB = captures[1] + terrB;
    float totalW = captures[2] + terrW + 6.5f;
    
    char msg[256];
    sprintf(msg, "Black: %d territory + %d captures = %g\n"
                 "White: %d territory + %d captures + 6.5 komi = %g\n\n"
                 "%s wins!", 
                 terrB, captures[1], totalB,
                 terrW, captures[2], totalW,
                 (totalB > totalW) ? "Black" : "White");
                 
    MessageBox(hwnd, msg, "Game Score", MB_OK);
    InitBoard();
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hBtnPass, hBtnResign, hBtnNew, hCbSize;

    switch (uMsg) {
        case WM_CREATE:
            hBtnPass = CreateWindow("BUTTON", "Pass", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                20, 600, 60, 30, hwnd, (HMENU)ID_BTN_PASS, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hBtnResign = CreateWindow("BUTTON", "Resign", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                90, 600, 60, 30, hwnd, (HMENU)ID_BTN_RESIGN, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Score", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                160, 600, 60, 30, hwnd, (HMENU)ID_BTN_SCORE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hBtnNew = CreateWindow("BUTTON", "New Game", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                230, 600, 80, 30, hwnd, (HMENU)ID_BTN_NEW, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hCbSize = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
                320, 600, 80, 100, hwnd, (HMENU)ID_CB_SIZE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            SendMessage(hCbSize, CB_ADDSTRING, 0, (LPARAM)"9x9");
            SendMessage(hCbSize, CB_ADDSTRING, 0, (LPARAM)"13x13");
            SendMessage(hCbSize, CB_ADDSTRING, 0, (LPARAM)"19x19");
            SendMessage(hCbSize, CB_SETCURSEL, 0, 0);
            return 0;

        case WM_TIMER:
            if (wParam == 1) {
                animRadius += 2;
                if (animRadius >= 13) {
                    animRadius = 13;
                    KillTimer(hwnd, 1);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;

        case WM_MOUSEMOVE: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            int padding = 40;
            int cellSize = 30;
            
            if (x >= padding - cellSize/2 && x <= padding + (boardSize-1)*cellSize + cellSize/2 &&
                y >= padding - cellSize/2 && y <= padding + (boardSize-1)*cellSize + cellSize/2) {
                int col = (x - padding + cellSize / 2) / cellSize;
                int row = (y - padding + cellSize / 2) / cellSize;
                if (col >= 0 && col < boardSize && row >= 0 && row < boardSize) {
                    if (hoverX != col || hoverY != row) {
                        hoverX = col;
                        hoverY = row;
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            } else {
                if (hoverX != -1 || hoverY != -1) {
                    hoverX = -1; hoverY = -1;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            int padding = 40;
            int cellSize = 30;
            
            // Check if within bounds roughly
            int col = (x - padding + cellSize / 2) / cellSize;
            int row = (y - padding + cellSize / 2) / cellSize;
            
            if (col >= 0 && col < boardSize && row >= 0 && row < boardSize) {
                PlaceStone(hwnd, col, row);
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH hBrush = CreateSolidBrush(RGB(18, 18, 18));
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);

            int padding = 40;
            int cellSize = 30;
            
            HBRUSH boardBrush = CreateSolidBrush(RGB(220, 179, 92)); // #dcb35c
            RECT boardRect = { padding, padding, padding + (boardSize-1)*cellSize, padding + (boardSize-1)*cellSize };
            boardRect.left -= 15; boardRect.top -= 15;
            boardRect.right += 15; boardRect.bottom += 15;
            FillRect(hdc, &boardRect, boardBrush);
            DeleteObject(boardBrush);

            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            SelectObject(hdc, hPen);
            for (int i = 0; i < boardSize; i++) {
                MoveToEx(hdc, padding, padding + i * cellSize, NULL);
                LineTo(hdc, padding + (boardSize - 1) * cellSize, padding + i * cellSize);
                MoveToEx(hdc, padding + i * cellSize, padding, NULL);
                LineTo(hdc, padding + i * cellSize, padding + (boardSize - 1) * cellSize);
            }
            DeleteObject(hPen);
            
            for (int r = 0; r < boardSize; r++) {
                for (int c = 0; c < boardSize; c++) {
                    if (board[r][c] != 0) {
                        int cx = padding + c * cellSize;
                        int cy = padding + r * cellSize;
                        HBRUSH stoneBrush = CreateSolidBrush(board[r][c] == 1 ? RGB(17, 17, 17) : RGB(238, 238, 238));
                        HPEN stonePen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                        HBRUSH oldBrush = SelectObject(hdc, stoneBrush);
                        HPEN oldPen = SelectObject(hdc, stonePen);
                        
                        int radius = (r == animY && c == animX) ? animRadius : 13;
                        Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);
                        
                        SelectObject(hdc, oldBrush);
                        SelectObject(hdc, oldPen);
                        DeleteObject(stoneBrush);
                        DeleteObject(stonePen);
                    }
                }
            }

            if (hoverX != -1 && hoverY != -1 && board[hoverY][hoverX] == 0) {
                int cx = padding + hoverX * cellSize;
                int cy = padding + hoverY * cellSize;
                HPEN ghostPen = CreatePen(PS_SOLID, 2, currentPlayer == 1 ? RGB(100, 100, 100) : RGB(200, 200, 200));
                HBRUSH ghostBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
                HPEN oldPen = SelectObject(hdc, ghostPen);
                HBRUSH oldBrush = SelectObject(hdc, ghostBrush);
                Ellipse(hdc, cx - 13, cy - 13, cx + 13, cy + 13);
                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);
                DeleteObject(ghostPen);
            }
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(224, 224, 224));
            char status[64];
            sprintf(status, "KGo - Current Turn: %s", currentPlayer == 1 ? "Black" : "White");
            TextOut(hdc, 20, 10, status, strlen(status));

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BTN_PASS) {
                currentPlayer = currentPlayer == 1 ? 2 : 1;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == ID_BTN_RESIGN) {
                char msg[64];
                sprintf(msg, "%s wins by resignation!", currentPlayer == 1 ? "White" : "Black");
                MessageBox(hwnd, msg, "Game Over", MB_OK);
                InitBoard();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == ID_BTN_SCORE) {
                CalculateScore(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_NEW) {
                InitBoard();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == ID_CB_SIZE && HIWORD(wParam) == CBN_SELCHANGE) {
                int sel = SendMessage(hCbSize, CB_GETCURSEL, 0, 0);
                if (sel == 0) boardSize = 9;
                else if (sel == 1) boardSize = 13;
                else if (sel == 2) boardSize = 19;
                InitBoard();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KGoWindowClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(18, 18, 18));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KGo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 620, 700,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
