#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define W 500
#define H 550

const char* WORDS[] = {"HANGMAN", "COMPUTER", "PROGRAM", "APPLICATION", "SOFTWARE", "DEVELOPER"};
const int NUM_WORDS = 6;

const char* DRAWINGS[] = {
"  +---+\r\n  |   |\r\n      |\r\n      |\r\n      |\r\n      |\r\n=========",
"  +---+\r\n  |   |\r\n  O   |\r\n      |\r\n      |\r\n      |\r\n=========",
"  +---+\r\n  |   |\r\n  O   |\r\n  |   |\r\n      |\r\n      |\r\n=========",
"  +---+\r\n  |   |\r\n  O   |\r\n /|   |\r\n      |\r\n      |\r\n=========",
"  +---+\r\n  |   |\r\n  O   |\r\n /|\\  |\r\n      |\r\n      |\r\n=========",
"  +---+\r\n  |   |\r\n  O   |\r\n /|\\  |\r\n /    |\r\n      |\r\n=========",
"  +---+\r\n  |   |\r\n  O   |\r\n /|\\  |\r\n / \\  |\r\n      |\r\n========="
};

int errors = 0;
char target_word[32];
int guessed[26] = {0};
int game_over = 0;
int won = 0;
int initialized = 0;

unsigned int seed = 0;

int CustomRand() {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

void CustomSrand(unsigned int s) {
    seed = s;
}

void InitGame() {
    if (!initialized) {
        CustomSrand(GetTickCount());
        initialized = 1;
    }
    
    int w_idx = CustomRand() % NUM_WORDS;
    int i = 0;
    while(WORDS[w_idx][i]) {
        target_word[i] = WORDS[w_idx][i];
        i++;
    }
    target_word[i] = '\0';
    
    for(i=0; i<26; i++) guessed[i] = 0;
    errors = 0;
    game_over = 0;
    won = 0;
}

void Guess(char c) {
    if (game_over) return;
    if (c >= 'a' && c <= 'z') c -= 32;
    if (c < 'A' || c > 'Z') return;
    
    int idx = c - 'A';
    if (guessed[idx]) return;
    
    guessed[idx] = 1;
    
    int found = 0;
    int all_guessed = 1;
    for (int i = 0; target_word[i] != '\0'; i++) {
        if (target_word[i] == c) found = 1;
        if (!guessed[target_word[i] - 'A']) all_guessed = 0;
    }
    
    if (!found) {
        errors++;
        if (errors >= 6) {
            game_over = 1;
            won = 0;
        }
    } else {
        all_guessed = 1;
        for (int i = 0; target_word[i] != '\0'; i++) {
            if (!guessed[target_word[i] - 'A']) {
                all_guessed = 0;
                break;
            }
        }
        if (all_guessed) {
            game_over = 1;
            won = 1;
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            InitGame();
            break;

        case WM_KEYDOWN: {
            if (wParam >= 'A' && wParam <= 'Z') {
                Guess((char)wParam);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }

        case WM_LBUTTONDOWN: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            
            // Check restart button (center: 250, bottom: 450)
            if (x >= 150 && x <= 350 && y >= 450 && y <= 490) {
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Check keyboard clicks
            int kx = 110;
            int ky = 270;
            for (int i = 0; i < 26; i++) {
                int col = i % 7;
                int row = i / 7;
                int bx = kx + col * 40;
                int by = ky + row * 40;
                if (x >= bx && x <= bx + 35 && y >= by && y <= by + 35) {
                    Guess('A' + i);
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                }
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H);
            HBITMAP hOld = (HBITMAP)SelectObject(memDC, hbm);

            // Background
            HBRUSH bgBrush = CreateSolidBrush(RGB(18, 18, 18));
            RECT fullRc = {0, 0, W, H};
            FillRect(memDC, &fullRc, bgBrush);
            DeleteObject(bgBrush);

            SetBkMode(memDC, TRANSPARENT);
            
            HFONT hFontMain = CreateFontA(24, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            HFONT hFontMono = CreateFontA(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, FIXED_PITCH, "Consolas");
            
            // Title
            SetTextColor(memDC, RGB(79, 172, 254));
            SelectObject(memDC, hFontMain);
            TextOutA(memDC, 200, 20, "KHangman", 8);

            // Drawing
            SetTextColor(memDC, RGB(255, 82, 82));
            SelectObject(memDC, hFontMono);
            RECT drawRect = {190, 60, 400, 200};
            DrawTextA(memDC, DRAWINGS[errors], -1, &drawRect, DT_LEFT | DT_TOP);

            // Word display
            SetTextColor(memDC, RGB(224, 224, 224));
            SelectObject(memDC, hFontMain);
            char disp[100] = {0};
            int len = 0;
            for (int i = 0; target_word[i] != '\0'; i++) {
                if (guessed[target_word[i] - 'A']) {
                    disp[len++] = target_word[i];
                } else {
                    disp[len++] = '_';
                }
                disp[len++] = ' ';
            }
            disp[len] = '\0';
            
            // Center the word display
            SIZE tSize;
            GetTextExtentPoint32A(memDC, disp, len, &tSize);
            TextOutA(memDC, (W - tSize.cx) / 2, 200, disp, len);

            // Message
            char* msgTxt = "Guess a letter to start";
            COLORREF msgColor = RGB(224, 224, 224);
            if (game_over) {
                if (won) {
                    msgTxt = "You Win!";
                    msgColor = RGB(76, 175, 80);
                } else {
                    static char loseMsg[100];
                    wsprintfA(loseMsg, "Game Over! Word was %s", target_word);
                    msgTxt = loseMsg;
                    msgColor = RGB(244, 67, 54);
                }
            }
            SetTextColor(memDC, msgColor);
            GetTextExtentPoint32A(memDC, msgTxt, lstrlenA(msgTxt), &tSize);
            TextOutA(memDC, (W - tSize.cx) / 2, 240, msgTxt, lstrlenA(msgTxt));

            // Keyboard
            SelectObject(memDC, hFontMono);
            int kx = 110;
            int ky = 280;
            for (int i = 0; i < 26; i++) {
                int col = i % 7;
                int row = i / 7;
                int bx = kx + col * 40;
                int by = ky + row * 40;
                
                RECT btnRect = {bx, by, bx + 35, by + 35};
                
                if (guessed[i]) {
                    HBRUSH btnBg = CreateSolidBrush(RGB(18, 18, 18));
                    FillRect(memDC, &btnRect, btnBg);
                    DeleteObject(btnBg);
                    SetTextColor(memDC, RGB(85, 85, 85));
                } else {
                    HBRUSH btnBg = CreateSolidBrush(RGB(44, 44, 44));
                    FillRect(memDC, &btnRect, btnBg);
                    DeleteObject(btnBg);
                    SetTextColor(memDC, RGB(224, 224, 224));
                }
                
                char l[2] = {(char)('A' + i), 0};
                DrawTextA(memDC, l, 1, &btnRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            // Restart button
            RECT resRect = {150, 450, 350, 490};
            HBRUSH resBg = CreateSolidBrush(RGB(0, 122, 204));
            FillRect(memDC, &resRect, resBg);
            DeleteObject(resBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            SelectObject(memDC, hFontMain);
            DrawTextA(memDC, "New Game", -1, &resRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            DeleteObject(hFontMain);
            DeleteObject(hFontMono);

            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, hOld);
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
    wc.lpszClassName = "KHangmanApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KHangmanApp", "KHangman", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
