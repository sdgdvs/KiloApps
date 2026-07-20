#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define ROWS 8
#define COLS 8
#define CELL_SIZE 50
#define BOARD_X 50
#define BOARD_Y 50

int grid[ROWS][COLS];
int score = 0;
int moves = 0;
int selR = -1, selC = -1;
int isProcessing = 0;

void PlaySwapSound() { Beep(300, 100); }
void PlayMatchSound(int combo) { Beep(400 + combo * 100, 150); }
void PlayBadSwapSound() { Beep(150, 150); }

COLORREF colors[] = {
    RGB(255, 64, 64),
    RGB(64, 255, 64),
    RGB(64, 64, 255),
    RGB(255, 255, 64),
    RGB(255, 64, 255),
    RGB(64, 255, 255)
};

void DrawBoard(HDC hdc) {
    char buf[64];
    sprintf(buf, "Score: %d   Moves: %d", score, moves);
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    TextOut(hdc, BOARD_X, 15, buf, strlen(buf));

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            RECT rect = { BOARD_X + c * CELL_SIZE, BOARD_Y + r * CELL_SIZE, 
                          BOARD_X + (c + 1) * CELL_SIZE, BOARD_Y + (r + 1) * CELL_SIZE };
            
            HBRUSH brush;
            if (grid[r][c] == -1) {
                brush = CreateSolidBrush(RGB(45, 45, 45));
            } else {
                brush = CreateSolidBrush(colors[grid[r][c]]);
            }
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);

            if (r == selR && c == selC) {
                HBRUSH border = CreateSolidBrush(RGB(255, 255, 255));
                FrameRect(hdc, &rect, border);
                DeleteObject(border);
                
                rect.left += 1; rect.top += 1; rect.right -= 1; rect.bottom -= 1;
                border = CreateSolidBrush(RGB(255, 255, 255));
                FrameRect(hdc, &rect, border);
                DeleteObject(border);
            } else {
                HBRUSH border = CreateSolidBrush(RGB(85, 85, 85));
                FrameRect(hdc, &rect, border);
                DeleteObject(border);
            }
        }
    }
}

int FindMatches(int matchGrid[ROWS][COLS]) {
    int hasMatch = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            matchGrid[r][c] = 0;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS - 2; c++) {
            int color = grid[r][c];
            if (color != -1 && grid[r][c+1] == color && grid[r][c+2] == color) {
                matchGrid[r][c] = 1; matchGrid[r][c+1] = 1; matchGrid[r][c+2] = 1;
                int k = c + 3;
                while(k < COLS && grid[r][k] == color) { matchGrid[r][k] = 1; k++; }
                hasMatch = 1;
            }
        }
    }
    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r < ROWS - 2; r++) {
            int color = grid[r][c];
            if (color != -1 && grid[r+1][c] == color && grid[r+2][c] == color) {
                matchGrid[r][c] = 1; matchGrid[r+1][c] = 1; matchGrid[r+2][c] = 1;
                int k = r + 3;
                while(k < ROWS && grid[k][c] == color) { matchGrid[k][c] = 1; k++; }
                hasMatch = 1;
            }
        }
    }
    return hasMatch;
}

void ProcessMatches(HWND hwnd) {
    int matchGrid[ROWS][COLS];
    int comboMultiplier = 1;
    while (FindMatches(matchGrid)) {
        int matchCount = 0;
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (matchGrid[r][c]) {
                    grid[r][c] = -1;
                    matchCount++;
                }
            }
        }
        score += matchCount * 10 * comboMultiplier;
        InvalidateRect(hwnd, NULL, FALSE); UpdateWindow(hwnd);
        PlayMatchSound(comboMultiplier);
        Sleep(50);
        comboMultiplier++;

        for (int c = 0; c < COLS; c++) {
            int emptyR = ROWS - 1;
            for (int r = ROWS - 1; r >= 0; r--) {
                if (grid[r][c] != -1) {
                    if (emptyR != r) {
                        grid[emptyR][c] = grid[r][c];
                        grid[r][c] = -1;
                    }
                    emptyR--;
                }
            }
            for (int r = emptyR; r >= 0; r--) {
                grid[r][c] = rand() % 6;
            }
        }
        InvalidateRect(hwnd, NULL, FALSE); UpdateWindow(hwnd);
        Sleep(200);
    }
}

void InitGame() {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int color;
            do {
                color = rand() % 6;
            } while (
                (r >= 2 && grid[r-1][c] == color && grid[r-2][c] == color) ||
                (c >= 2 && grid[r][c-1] == color && grid[r][c-2] == color)
            );
            grid[r][c] = color;
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            srand((unsigned)time(NULL));
            InitGame();
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
            
            HBRUSH bgBrush = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(memDC, &rect, bgBrush);
            DeleteObject(bgBrush);
            
            DrawBoard(memDC);
            
            BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);
            
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN: {
            if (isProcessing) break;
            int x = LOWORD(lParam) - BOARD_X;
            int y = HIWORD(lParam) - BOARD_Y;
            if (x >= 0 && x < COLS * CELL_SIZE && y >= 0 && y < ROWS * CELL_SIZE) {
                int c = x / CELL_SIZE;
                int r = y / CELL_SIZE;
                if (selR == -1) {
                    selR = r; selC = c;
                    InvalidateRect(hwnd, NULL, FALSE);
                } else {
                    if (selR == r && selC == c) {
                        selR = -1; selC = -1;
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else if (abs(selR - r) + abs(selC - c) == 1) {
                        moves++;
                        isProcessing = 1;
                        int temp = grid[selR][selC];
                        grid[selR][selC] = grid[r][c];
                        grid[r][c] = temp;
                        int origR = selR, origC = selC;
                        selR = -1; selC = -1;
                        InvalidateRect(hwnd, NULL, FALSE); UpdateWindow(hwnd);
                        PlaySwapSound();
                        Sleep(100);

                        int matchGrid[ROWS][COLS];
                        if (FindMatches(matchGrid)) {
                            ProcessMatches(hwnd);
                        } else {
                            PlayBadSwapSound();
                            temp = grid[origR][origC];
                            grid[origR][origC] = grid[r][c];
                            grid[r][c] = temp;
                            InvalidateRect(hwnd, NULL, FALSE); UpdateWindow(hwnd);
                        }
                        isProcessing = 0;
                    } else {
                        selR = r; selC = c;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
    wc.lpszClassName = "KMatch3Class";

    if(!RegisterClassEx(&wc)) return 0;

    HWND hwnd = CreateWindowEx(
        0, "KMatch3Class", "KMatch3",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 550,
        NULL, NULL, hInstance, NULL
    );

    if(!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
