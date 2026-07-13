#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define ID_EASY 1001
#define ID_HARD 1002
#define ID_EXPERT 1003
#define ID_RESTART 1004

int W = 400;
int H = 400;
int ROWS = 4;
int COLS = 4;

int cards[36];
int flipped[36] = {0};
int matched[36] = {0};
int firstFlip = -1;
int secondFlip = -1;
int matches = 0;
int moves = 0;

int best_easy = -1;
int best_hard = -1;
int best_expert = -1;
int is_hard = 0; // 0=easy, 1=hard, 2=expert
int start_time = 0;
int elapsed_time = 0;
int timer_running = 0;
int is_previewing = 0;

// simple pseudo-random generator to avoid CRT
unsigned int seed = 12345;
unsigned int rnd() {
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

void LoadScores() {
    FILE *f = fopen("kmemory.dat", "rb");
    if (f) {
        fread(&best_easy, sizeof(int), 1, f);
        fread(&best_hard, sizeof(int), 1, f);
        fread(&best_expert, sizeof(int), 1, f);
        fclose(f);
    }
}

void SaveScores() {
    FILE *f = fopen("kmemory.dat", "wb");
    if (f) {
        fwrite(&best_easy, sizeof(int), 1, f);
        fwrite(&best_hard, sizeof(int), 1, f);
        fwrite(&best_expert, sizeof(int), 1, f);
        fclose(f);
    }
}

void Shuffle(HWND hwnd) {
    int numCards = ROWS * COLS;
    for (int i = 0; i < numCards; i++) {
        cards[i] = i / 2;
        flipped[i] = 0;
        matched[i] = 0;
    }
    for (int i = numCards - 1; i > 0; i--) {
        int j = rnd() % (i + 1);
        int temp = cards[i];
        cards[i] = cards[j];
        cards[j] = temp;
    }
    firstFlip = -1;
    secondFlip = -1;
    matches = 0;
    moves = 0;
    elapsed_time = 0;
    timer_running = 0;
    
    if (hwnd) {
        is_previewing = 1;
        for (int i = 0; i < numCards; i++) flipped[i] = 1;
        SetTimer(hwnd, 3, (is_hard == 2 ? 1000 : (is_hard == 1 ? 2000 : 3000)), NULL);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void SetDifficulty(HWND hwnd, int hard) {
    is_hard = hard;
    if (hard == 2) {
        ROWS = 4; COLS = 8; W = 800; H = 400;
    } else if (hard == 1) {
        ROWS = 4; COLS = 6; W = 600; H = 400;
    } else {
        ROWS = 4; COLS = 4; W = 400; H = 400;
    }
    RECT rc = {0, 0, W, H + 40}; // extra for status bar text
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, TRUE);
    SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
    if (timer_running) KillTimer(hwnd, 2);
    Shuffle(hwnd);
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
        case WM_CREATE: {
            HMENU hMenu = CreateMenu();
            HMENU hSubMenu = CreatePopupMenu();
            AppendMenu(hSubMenu, MF_STRING, ID_EASY, "Easy (4x4)");
            AppendMenu(hSubMenu, MF_STRING, ID_HARD, "Hard (6x4)");
            AppendMenu(hSubMenu, MF_STRING, ID_EXPERT, "Expert (8x4)");
            AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hSubMenu, MF_STRING, ID_RESTART, "Restart");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "Game");
            SetMenu(hwnd, hMenu);

            LoadScores();
            seed = GetTickCount();
            SetDifficulty(hwnd, 0); // start easy
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_EASY) {
                SetDifficulty(hwnd, 0);
            } else if (LOWORD(wParam) == ID_HARD) {
                SetDifficulty(hwnd, 1);
            } else if (LOWORD(wParam) == ID_EXPERT) {
                SetDifficulty(hwnd, 2);
            } else if (LOWORD(wParam) == ID_RESTART) {
                SetDifficulty(hwnd, is_hard);
            }
            break;
        case WM_LBUTTONDOWN: {
            if (is_previewing) return 0;
            if (secondFlip != -1) return 0; // wait for timer
            
            int x = LOWORD(lParam);
            int y = HIWORD(lParam) - 40; // Adjust for status area
            
            if (y < 0) return 0;

            int cw = W / COLS;
            int ch = H / ROWS;
            int col = x / cw;
            int row = y / ch;
            
            if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {
                int idx = row * COLS + col;
                if (!matched[idx] && !flipped[idx]) {
                    if (!timer_running) {
                        start_time = GetTickCount();
                        timer_running = 1;
                        SetTimer(hwnd, 2, 1000, NULL);
                    }
                    flipped[idx] = 1;
                    MessageBeep(MB_OK);
                    if (firstFlip == -1) {
                        firstFlip = idx;
                    } else {
                        secondFlip = idx;
                        moves++;
                        if (cards[firstFlip] == cards[secondFlip]) {
                            matched[firstFlip] = 1;
                            matched[secondFlip] = 1;
                            firstFlip = -1;
                            secondFlip = -1;
                            matches++;
                            MessageBeep(MB_ICONASTERISK);
                            if (matches == (ROWS * COLS) / 2) {
                                if (timer_running) {
                                    KillTimer(hwnd, 2);
                                    timer_running = 0;
                                }
                                InvalidateRect(hwnd, NULL, FALSE);
                                UpdateWindow(hwnd);
                                
                                int is_new_best = 0;
                                if (is_hard == 2) {
                                    if (best_expert == -1 || moves < best_expert) { best_expert = moves; is_new_best = 1; }
                                } else if (is_hard == 1) {
                                    if (best_hard == -1 || moves < best_hard) { best_hard = moves; is_new_best = 1; }
                                } else {
                                    if (best_easy == -1 || moves < best_easy) { best_easy = moves; is_new_best = 1; }
                                }
                                if (is_new_best) SaveScores();
                                
                                char msgBuf[256];
                                sprintf(msgBuf, "You won in %d moves and %d seconds!", moves, elapsed_time);
                                MessageBoxA(hwnd, msgBuf, "KMemory", MB_OK);
                                Shuffle(hwnd);
                            }
                        } else {
                            SetTimer(hwnd, 1, 1000, NULL);
                            MessageBeep(MB_ICONHAND);
                        }
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }
        case WM_TIMER:
            if (wParam == 3) {
                KillTimer(hwnd, 3);
                is_previewing = 0;
                int numCards = ROWS * COLS;
                for (int i = 0; i < numCards; i++) flipped[i] = 0;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 1) {
                KillTimer(hwnd, 1);
                flipped[firstFlip] = 0;
                flipped[secondFlip] = 0;
                firstFlip = -1;
                secondFlip = -1;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 2) {
                elapsed_time = (GetTickCount() - start_time) / 1000;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H + 40);
            SelectObject(memDC, hbm);
            
            // Draw status area
            RECT statusRc = {0, 0, W, 40};
            HBRUSH statusBg = CreateSolidBrush(RGB(20, 20, 20));
            FillRect(memDC, &statusRc, statusBg);
            DeleteObject(statusBg);
            
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            char statusText[256];
            int best = (is_hard == 2) ? best_expert : (is_hard == 1 ? best_hard : best_easy);
            if (best == -1)
                sprintf(statusText, "Time: %ds   Moves: %d   Best: -", elapsed_time, moves);
            else
                sprintf(statusText, "Time: %ds   Moves: %d   Best: %d", elapsed_time, moves, best);
            TextOutA(memDC, 10, 10, statusText, strlen(statusText));

            // Draw table
            RECT rc = {0, 40, W, H + 40};
            HBRUSH bg = CreateSolidBrush(RGB(30, 150, 60)); // Green table
            FillRect(memDC, &rc, bg);
            DeleteObject(bg);
            
            int cw = W / COLS;
            int ch = H / ROWS;
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    DrawCard(memDC, r * COLS + c, c * cw + 5, r * ch + 40 + 5, cw - 10, ch - 10);
                }
            }
            
            BitBlt(hdc, 0, 0, W, H + 40, memDC, 0, 0, SRCCOPY);
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
        CW_USEDEFAULT, CW_USEDEFAULT, 400 + 16, 440 + 59, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
