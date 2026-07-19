#include <windows.h>
#include <stdbool.h>

const char g_szClassName[] = "KConnect4WindowClass";
#define ROWS 6
#define COLS 7

int board[ROWS][COLS];
int currentPlayer = 1;
bool gameActive = true;
bool isDraw = false;

void ResetGame() {
    for(int r=0; r<ROWS; r++)
        for(int c=0; c<COLS; c++)
            board[r][c] = 0;
    currentPlayer = 1;
    gameActive = true;
    isDraw = false;
}

bool CheckWin(int r, int c, int p) {
    int dirs[4][2] = {{0,1}, {1,0}, {1,1}, {1,-1}};
    for(int d=0; d<4; d++) {
        int count = 1;
        for(int i=1; i<4; i++) {
            int nr = r + dirs[d][0]*i;
            int nc = c + dirs[d][1]*i;
            if(nr>=0 && nr<ROWS && nc>=0 && nc<COLS && board[nr][nc]==p) count++;
            else break;
        }
        for(int i=1; i<4; i++) {
            int nr = r - dirs[d][0]*i;
            int nc = c - dirs[d][1]*i;
            if(nr>=0 && nr<ROWS && nc>=0 && nc<COLS && board[nr][nc]==p) count++;
            else break;
        }
        if(count >= 4) return true;
    }
    return false;
}

bool CheckDraw() {
    for(int c=0; c<COLS; c++) {
        if(board[0][c] == 0) return false;
    }
    return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            ResetGame();
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
            
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    int x = 20 + 5 + c * 45;
                    int y = 50 + 5 + r * 45;
                    
                    if (board[r][c] == 1) SelectObject(hdc, p1Cell);
                    else if (board[r][c] == 2) SelectObject(hdc, p2Cell);
                    else SelectObject(hdc, emptyCell);
                    
                    SelectObject(hdc, GetStockObject(NULL_PEN));
                    Ellipse(hdc, x, y, x + 40, y + 40);
                }
            }
            DeleteObject(emptyCell);
            DeleteObject(p1Cell);
            DeleteObject(p2Cell);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN: {
            if (!gameActive) {
                ResetGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
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
