#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 480
#define H 500
#define TS 50
#define OX 30
#define OY 30

// 0: empty, 1:P, 2:N, 3:B, 4:R, 5:Q, 6:K,  7:p, 8:n, 9:b, 10:r, 11:q, 12:k
int board[8][8] = {
    {10, 8, 9, 11, 12, 9, 8, 10},
    {7,  7, 7,  7,  7, 7, 7,  7},
    {0,  0, 0,  0,  0, 0, 0,  0},
    {0,  0, 0,  0,  0, 0, 0,  0},
    {0,  0, 0,  0,  0, 0, 0,  0},
    {0,  0, 0,  0,  0, 0, 0,  0},
    {1,  1, 1,  1,  1, 1, 1,  1},
    {4,  2, 3,  5,  6, 3, 2,  4}
};

const char* pieceChars[] = {
    "", "P", "N", "B", "R", "Q", "K",
    "p", "n", "b", "r", "q", "k"
};

int selX = -1;
int selY = -1;
int whiteTurn = 1;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HFONT hFont = CreateFontA(32, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            HGDIOBJ oldFont = SelectObject(hdc, hFont);
            SetBkMode(hdc, TRANSPARENT);
            
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    RECT rc = { OX + x * TS, OY + y * TS, OX + (x + 1) * TS, OY + (y + 1) * TS };
                    HBRUSH brush;
                    
                    if (x == selX && y == selY) {
                        brush = CreateSolidBrush(RGB(255, 255, 100)); // selected
                    } else if ((x + y) % 2 == 0) {
                        brush = CreateSolidBrush(RGB(240, 217, 181)); // light
                    } else {
                        brush = CreateSolidBrush(RGB(181, 136, 99)); // dark
                    }
                    
                    FillRect(hdc, &rc, brush);
                    DeleteObject(brush);
                    
                    int p = board[y][x];
                    if (p != 0) {
                        if (p <= 6) SetTextColor(hdc, RGB(255, 255, 255)); // White pieces
                        else SetTextColor(hdc, RGB(0, 0, 0)); // Black pieces
                        
                        DrawTextA(hdc, pieceChars[p], -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                }
            }
            
            SelectObject(hdc, oldFont);
            DeleteObject(hFont);
            
            RECT statusRc = { 10, H - 60, W - 10, H - 30 };
            HFONT sFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            oldFont = SelectObject(hdc, sFont);
            SetTextColor(hdc, RGB(0, 0, 0));
            
            if (whiteTurn) {
                DrawTextA(hdc, "White's Turn", -1, &statusRc, DT_LEFT);
            } else {
                DrawTextA(hdc, "Black's Turn", -1, &statusRc, DT_LEFT);
            }
            SelectObject(hdc, oldFont);
            DeleteObject(sFont);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            
            int tx = (mx - OX) / TS;
            int ty = (my - OY) / TS;
            
            if (tx >= 0 && tx < 8 && ty >= 0 && ty < 8) {
                if (selX == -1) {
                    if (board[ty][tx] != 0) {
                        int isWhite = board[ty][tx] <= 6;
                        if ((whiteTurn && isWhite) || (!whiteTurn && !isWhite)) {
                            selX = tx;
                            selY = ty;
                        }
                    }
                } else {
                    if (selX == tx && selY == ty) {
                        selX = -1;
                        selY = -1;
                    } else {
                        // Very basic move (no validation for a minimal toy)
                        board[ty][tx] = board[selY][selX];
                        board[selY][selX] = 0;
                        selX = -1;
                        selY = -1;
                        whiteTurn = !whiteTurn;
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
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

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KChessApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KChessApp", "KChess", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
