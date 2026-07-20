#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ID_BTN_PASS 101
#define ID_BTN_RESIGN 102
#define ID_BTN_NEW 103
#define ID_CB_SIZE 104
#define ID_BTN_SCORE 105
#define ID_CB_AI 106
#define ID_BTN_SAVE 107
#define ID_BTN_LOAD 108

int boardSize = 9;
char board[19][19] = {0}; // 0 = empty, 1 = black, 2 = white
char prevBoard[19][19] = {0};
int currentPlayer = 1;
int captures[3] = {0}; // 1 = black, 2 = white
int hoverX = -1, hoverY = -1;
int animX = -1, animY = -1;
int animRadius = 13;
POINT capturedAnimStones[19*19];
int capturedAnimColor[19*19];
int capturedAnimCount = 0;
int captureAnimRadius = 0;

DWORD WINAPI PlaySoundThread(LPVOID lpParam) {
    int type = (int)(intptr_t)lpParam;
    if (type == 1) {
        Beep(300, 80);
    } else if (type == 2) {
        Beep(150, 100);
        Beep(400, 100);
    } else if (type == 3) {
        Beep(440, 150);
        Beep(554, 150);
        Beep(659, 400);
    }
    return 0;
}
void PlayGameSound(int type) {
    CreateThread(NULL, 0, PlaySoundThread, (LPVOID)(intptr_t)type, 0, NULL);
}

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

int CheckCaptures(int color, int realMove) {
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
                        if (realMove) {
                            capturedAnimStones[capturedAnimCount].x = group[i].x;
                            capturedAnimStones[capturedAnimCount].y = group[i].y;
                            capturedAnimColor[capturedAnimCount] = color;
                            capturedAnimCount++;
                        }
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
    capturedAnimCount = 0;
    int caps = CheckCaptures(opp, 1);
    
    char visited[19][19] = {0};
    if (GetLiberties(x, y, currentPlayer, visited) == 0) {
        CopyBoard(board, boardBackup);
        if (currentPlayer == 1) MessageBox(hwnd, "Suicide move is not allowed.", "Invalid Move", MB_OK);
        return;
    }
    
    if (memcmp(board, prevBoard, sizeof(char) * 19 * 19) == 0) {
        CopyBoard(board, boardBackup);
        if (currentPlayer == 1) MessageBox(hwnd, "Ko rule violation.", "Invalid Move", MB_OK);
        return;
    }
    
    CopyBoard(prevBoard, boardBackup);
    currentPlayer = opp;
    
    if (caps > 0) PlayGameSound(2);
    else PlayGameSound(1);
    
    animX = x;
    animY = y;
    animRadius = 0;
    SetTimer(hwnd, 1, 16, NULL);
    
    if (caps > 0) {
        captureAnimRadius = 13;
        SetTimer(hwnd, 3, 16, NULL);
    }
    
    InvalidateRect(hwnd, NULL, TRUE);
    
    if (currentPlayer == 2 && SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_GETCHECK, 0, 0) == BST_CHECKED) {
        SetTimer(hwnd, 2, 500, NULL);
    }
}

int IsValidMove(int x, int y, int color, int *outCaptures) {
    if (board[y][x] != 0) return 0;
    
    char boardBackup[19][19];
    CopyBoard(boardBackup, board);
    int capBackup[3] = { captures[0], captures[1], captures[2] };
    
    board[y][x] = (char)color;
    int opp = color == 1 ? 2 : 1;
    int capCount = CheckCaptures(opp, 0);
    if (outCaptures) *outCaptures = capCount;
    
    char visited[19][19] = {0};
    if (GetLiberties(x, y, color, visited) == 0) {
        CopyBoard(board, boardBackup);
        captures[1] = capBackup[1];
        captures[2] = capBackup[2];
        return 0; // suicide
    }
    
    if (memcmp(board, prevBoard, sizeof(char) * 19 * 19) == 0) {
        CopyBoard(board, boardBackup);
        captures[1] = capBackup[1];
        captures[2] = capBackup[2];
        return 0; // ko
    }
    
    CopyBoard(board, boardBackup);
    captures[1] = capBackup[1];
    captures[2] = capBackup[2];
    return 1;
}

void MakeAIMove(HWND hwnd) {
    if (currentPlayer != 2) return;
    
    POINT captureMoves[19*19];
    int captureCount = 0;
    POINT validMoves[19*19];
    int validCount = 0;
    
    for (int y = 0; y < boardSize; y++) {
        for (int x = 0; x < boardSize; x++) {
            int caps = 0;
            if (IsValidMove(x, y, 2, &caps)) {
                if (caps > 0) {
                    captureMoves[captureCount].x = x;
                    captureMoves[captureCount].y = y;
                    captureCount++;
                } else {
                    validMoves[validCount].x = x;
                    validMoves[validCount].y = y;
                    validCount++;
                }
            }
        }
    }
    
    if (captureCount > 0) {
        int r = rand() % captureCount;
        PlaceStone(hwnd, captureMoves[r].x, captureMoves[r].y);
    } else if (validCount > 0) {
        int r = rand() % validCount;
        PlaceStone(hwnd, validMoves[r].x, validMoves[r].y);
    } else {
        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_PASS, 0), 0);
    }
}

void SaveGame(HWND hwnd) {
    FILE *f = fopen("kgo_save.dat", "wb");
    if (f) {
        fwrite(&boardSize, sizeof(int), 1, f);
        fwrite(board, sizeof(char), 19*19, f);
        fwrite(prevBoard, sizeof(char), 19*19, f);
        fwrite(&currentPlayer, sizeof(int), 1, f);
        fwrite(captures, sizeof(int), 3, f);
        fclose(f);
        MessageBox(hwnd, "Game saved.", "Save", MB_OK);
    } else {
        MessageBox(hwnd, "Failed to save game.", "Error", MB_OK);
    }
}

void LoadGame(HWND hwnd) {
    FILE *f = fopen("kgo_save.dat", "rb");
    if (f) {
        fread(&boardSize, sizeof(int), 1, f);
        fread(board, sizeof(char), 19*19, f);
        fread(prevBoard, sizeof(char), 19*19, f);
        fread(&currentPlayer, sizeof(int), 1, f);
        fread(captures, sizeof(int), 3, f);
        fclose(f);
        
        int sel = 0;
        if (boardSize == 13) sel = 1;
        else if (boardSize == 19) sel = 2;
        SendMessage(GetDlgItem(hwnd, ID_CB_SIZE), CB_SETCURSEL, sel, 0);
        
        InvalidateRect(hwnd, NULL, TRUE);
        MessageBox(hwnd, "Game loaded.", "Load", MB_OK);
    } else {
        MessageBox(hwnd, "No saved game found.", "Error", MB_OK);
    }
}

void InitBoard() {
    memset(board, 0, sizeof(board));
    memset(prevBoard, 0, sizeof(prevBoard));
    currentPlayer = 1;
    captures[1] = 0;
    captures[2] = 0;
    animX = -1;
    animY = -1;
    capturedAnimCount = 0;
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
                 
    PlayGameSound(3);
    MessageBox(hwnd, msg, "Game Score", MB_OK);
    InitBoard();
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hBtnPass, hBtnResign, hBtnNew, hCbSize, hCbAi;

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
            hCbAi = CreateWindow("BUTTON", "Play vs AI (White)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                420, 600, 150, 30, hwnd, (HMENU)ID_CB_AI, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            SendMessage(hCbAi, BM_SETCHECK, BST_CHECKED, 0);
            CreateWindow("BUTTON", "Save", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                20, 640, 60, 30, hwnd, (HMENU)ID_BTN_SAVE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            CreateWindow("BUTTON", "Load", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                90, 640, 60, 30, hwnd, (HMENU)ID_BTN_LOAD, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            return 0;

        case WM_TIMER:
            if (wParam == 1) {
                animRadius += 2;
                if (animRadius >= 13) {
                    animRadius = 13;
                    KillTimer(hwnd, 1);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 3) {
                captureAnimRadius -= 2;
                if (captureAnimRadius <= 0) {
                    captureAnimRadius = 0;
                    capturedAnimCount = 0;
                    KillTimer(hwnd, 3);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == 2) {
                KillTimer(hwnd, 2);
                MakeAIMove(hwnd);
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

            for (int i = 0; i < capturedAnimCount; i++) {
                int cx = padding + capturedAnimStones[i].x * cellSize;
                int cy = padding + capturedAnimStones[i].y * cellSize;
                HBRUSH stoneBrush = CreateSolidBrush(capturedAnimColor[i] == 1 ? RGB(17, 17, 17) : RGB(238, 238, 238));
                HPEN stonePen = CreatePen(PS_SOLID, 2, RGB(255, 68, 68)); // Red border for capture highlight
                HBRUSH oldBrush = SelectObject(hdc, stoneBrush);
                HPEN oldPen = SelectObject(hdc, stonePen);
                
                Ellipse(hdc, cx - captureAnimRadius, cy - captureAnimRadius, cx + captureAnimRadius, cy + captureAnimRadius);
                
                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);
                DeleteObject(stoneBrush);
                DeleteObject(stonePen);
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
                if (currentPlayer == 2 && SendMessage(GetDlgItem(hwnd, ID_CB_AI), BM_GETCHECK, 0, 0) == BST_CHECKED) {
                    SetTimer(hwnd, 2, 500, NULL);
                }
            } else if (LOWORD(wParam) == ID_BTN_RESIGN) {
                char msg[64];
                sprintf(msg, "%s wins by resignation!", currentPlayer == 1 ? "White" : "Black");
                PlayGameSound(3);
                MessageBox(hwnd, msg, "Game Over", MB_OK);
                InitBoard();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == ID_BTN_SCORE) {
                CalculateScore(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_NEW) {
                InitBoard();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == ID_BTN_SAVE) {
                SaveGame(hwnd);
            } else if (LOWORD(wParam) == ID_BTN_LOAD) {
                LoadGame(hwnd);
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
    srand(GetTickCount());

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
