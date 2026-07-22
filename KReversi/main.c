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

const char g_szClassName[] = "KReversiClass";

int board[64];
int currentPlayer = BLACK;
int dirs[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
int aiDifficulty = 1; // 0=Easy, 1=Medium, 2=Hard


void InitGame() {
    for(int i = 0; i < 64; i++) board[i] = EMPTY;
    board[27] = WHITE; board[28] = BLACK;
    board[35] = BLACK; board[36] = WHITE;
    currentPlayer = BLACK;
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

void DoMove(int index, int player) {
    int flips[64];
    int count = GetFlippable(index, player, flips);
    if (count > 0) {
        board[index] = player;
        for(int i=0; i<count; i++) {
            board[flips[i]] = player;
        }
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
        DoMove(bestMove, WHITE);
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
            SetMenu(hwnd, hMenu);
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
                    DoMove(idx, BLACK);
                    currentPlayer = WHITE;
                    if (!HasValidMoves(WHITE)) {
                        currentPlayer = BLACK;
                    } else {
                        InvalidateRect(hwnd, NULL, TRUE);
                        SetTimer(hwnd, 1, 300, NULL); // small delay for AI
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
                        SelectObject(hdc, board[idx] == BLACK ? blackBrush : whiteBrush);
                        Ellipse(hdc, rect.left + 5, rect.top + 5, rect.right - 5, rect.bottom - 5);
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
