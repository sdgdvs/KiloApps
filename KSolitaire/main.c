#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 400
#define H 400
#define ROWS 4
#define COLS 4

int cards[16];
int flipped[16] = {0};
int matched[16] = {0};
int firstFlip = -1;
int secondFlip = -1;
int matches = 0;

// simple pseudo-random generator to avoid CRT
unsigned int seed = 12345;
unsigned int rnd() {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

void Shuffle() {
    for (int i = 0; i < 16; i++) {
        cards[i] = i / 2;
        flipped[i] = 0;
        matched[i] = 0;
    }
    for (int i = 15; i > 0; i--) {
        int j = rnd() % (i + 1);
        int temp = cards[i];
        cards[i] = cards[j];
        cards[j] = temp;
    }
    firstFlip = -1;
    secondFlip = -1;
    matches = 0;
}

void DrawCard(HDC hdc, int idx, int x, int y, int w, int h) {
    RECT r = {x, y, x + w, y + h};
    if (matched[idx]) {
        HBRUSH bg = CreateSolidBrush(RGB(240, 240, 240));
        FillRect(hdc, &r, bg);
        DeleteObject(bg);
        return;
    }
    
    if (!flipped[idx]) {
        HBRUSH bg = CreateSolidBrush(RGB(50, 100, 200));
        FillRect(hdc, &r, bg);
        DeleteObject(bg);
        DrawEdge(hdc, &r, BDR_RAISEDINNER, BF_RECT);
    } else {
        HBRUSH bg = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &r, bg);
        DeleteObject(bg);
        DrawEdge(hdc, &r, BDR_SUNKENOUTER, BF_RECT);
        
        char text[2] = {'A' + cards[idx], 0};
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 0, 0));
        TextOutA(hdc, x + w / 2 - 4, y + h / 2 - 8, text, 1);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            seed = GetTickCount();
            Shuffle();
            break;
        case WM_LBUTTONDOWN: {
            if (secondFlip != -1) return 0; // wait for timer
            
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            int cw = W / COLS;
            int ch = H / ROWS;
            int col = x / cw;
            int row = y / ch;
            
            if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {
                int idx = row * COLS + col;
                if (!matched[idx] && !flipped[idx]) {
                    flipped[idx] = 1;
                    if (firstFlip == -1) {
                        firstFlip = idx;
                    } else {
                        secondFlip = idx;
                        if (cards[firstFlip] == cards[secondFlip]) {
                            matched[firstFlip] = 1;
                            matched[secondFlip] = 1;
                            firstFlip = -1;
                            secondFlip = -1;
                            matches++;
                            if (matches == 8) {
                                MessageBoxA(hwnd, "You won!", "KMemory", MB_OK);
                                Shuffle();
                            }
                        } else {
                            SetTimer(hwnd, 1, 1000, NULL);
                        }
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }
        case WM_TIMER:
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                flipped[firstFlip] = 0;
                flipped[secondFlip] = 0;
                firstFlip = -1;
                secondFlip = -1;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            SelectObject(memDC, hbm);
            
            RECT rc = {0, 0, W, H};
            HBRUSH bg = CreateSolidBrush(RGB(30, 150, 60)); // Green table
            FillRect(memDC, &rc, bg);
            DeleteObject(bg);
            
            int cw = W / COLS;
            int ch = H / ROWS;
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    DrawCard(memDC, r * COLS + c, c * cw + 5, r * ch + 5, cw - 10, ch - 10);
                }
            }
            
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            DeleteObject(hbm);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
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

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KSolitaireApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KSolitaireApp", "KMemory", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W + 16, H + 39, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
