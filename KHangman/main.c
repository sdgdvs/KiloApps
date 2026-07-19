#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define ID_BTN_NEWGAME 100
#define ID_BTN_KEY_BASE 200

const char* WORDS[] = {"HANGMAN", "COMPUTER", "PROGRAM", "APPLICATION", "SOFTWARE", "DEVELOPER"};
#define NUM_WORDS (sizeof(WORDS) / sizeof(WORDS[0]))
#define MAX_ERRORS 6

const char* DRAWINGS[] = {
"  +---+\r\n  |   |\r\n      |\r\n      |\r\n      |\r\n      |\r\n=========",
"  +---+\r\n  |   |\r\n  O   |\r\n      |\r\n      |\r\n      |\r\n=========",
"  +---+\r\n  |   |\r\n  O   |\r\n  |   |\r\n      |\r\n      |\r\n=========",
"  +---+\r\n  |   |\r\n  O   |\r\n /|   |\r\n      |\r\n      |\r\n=========",
"  +---+\r\n  |   |\r\n  O   |\r\n /|\\  |\r\n      |\r\n      |\r\n=========",
"  +---+\r\n  |   |\r\n  O   |\r\n /|\\  |\r\n /    |\r\n      |\r\n=========",
"  +---+\r\n  |   |\r\n  O   |\r\n /|\\  |\r\n / \\  |\r\n      |\r\n========="
};

const char* current_word;
bool guessed[26];
int errors = 0;
bool game_over = false;
bool game_won = false;

HWND hKeyboard[26];
HWND hNewGame;

void InitGame(HWND hwnd) {
    srand((unsigned int)time(NULL));
    current_word = WORDS[rand() % NUM_WORDS];
    for (int i = 0; i < 26; i++) {
        guessed[i] = false;
        if (hKeyboard[i]) EnableWindow(hKeyboard[i], TRUE);
    }
    errors = 0;
    game_over = false;
    game_won = false;
    InvalidateRect(hwnd, NULL, TRUE);
}

void CheckGameState(HWND hwnd) {
    bool win = true;
    for (int i = 0; i < (int)strlen(current_word); i++) {
        if (!guessed[current_word[i] - 'A']) {
            win = false;
            break;
        }
    }
    if (win) {
        game_won = true;
        game_over = true;
    } else if (errors >= MAX_ERRORS) {
        game_over = true;
    }
    
    if (game_over) {
        for (int i = 0; i < 26; i++) {
            if (hKeyboard[i]) EnableWindow(hKeyboard[i], FALSE);
        }
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void GuessLetter(HWND hwnd, char letter) {
    if (game_over || letter < 'A' || letter > 'Z') return;
    int idx = letter - 'A';
    if (guessed[idx]) return;

    guessed[idx] = true;
    if (hKeyboard[idx]) EnableWindow(hKeyboard[idx], FALSE);

    bool found = false;
    for (int i = 0; i < (int)strlen(current_word); i++) {
        if (current_word[i] == letter) {
            found = true;
            break;
        }
    }
    if (!found) {
        errors++;
    }
    CheckGameState(hwnd);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;

    switch (uMsg) {
        case WM_CREATE: {
            hNewGame = CreateWindowA("BUTTON", "New Game", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                20, 20, 100, 30, hwnd, (HMENU)ID_BTN_NEWGAME, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            
            // Create keyboard
            int startX = 20;
            int startY = 320;
            for (int i = 0; i < 26; i++) {
                char label[2] = { (char)('A' + i), '\0' };
                int x = startX + (i % 7) * 45;
                int y = startY + (i / 7) * 45;
                hKeyboard[i] = CreateWindowA("BUTTON", label, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                    x, y, 40, 40, hwnd, (HMENU)(ID_BTN_KEY_BASE + i), (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            }

            InitGame(hwnd);
            return 0;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_NEWGAME) {
                InitGame(hwnd);
            } else if (LOWORD(wParam) >= ID_BTN_KEY_BASE && LOWORD(wParam) < ID_BTN_KEY_BASE + 26) {
                char letter = 'A' + (LOWORD(wParam) - ID_BTN_KEY_BASE);
                GuessLetter(hwnd, letter);
            }
            SetFocus(hwnd);
            return 0;
        }

        case WM_KEYDOWN: {
            char key = (char)wParam;
            if (key >= 'a' && key <= 'z') {
                GuessLetter(hwnd, key - 'a' + 'A');
            } else if (key >= 'A' && key <= 'Z') {
                GuessLetter(hwnd, key);
            }
            return 0;
        }

        case WM_PAINT: {
            hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rect);

            // Dark background
            HBRUSH hBrush = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);

            SetBkMode(hdc, TRANSPARENT);

            HFONT hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                      DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

            // Draw title
            SetTextColor(hdc, RGB(79, 172, 254));
            TextOutA(hdc, 140, 20, "KHangman", 8);

            // Draw Word
            SetTextColor(hdc, RGB(255, 255, 255));
            char display_word[100] = {0};
            int len = strlen(current_word);
            for (int i = 0; i < len; i++) {
                if (guessed[current_word[i] - 'A']) {
                    display_word[i*2] = current_word[i];
                } else {
                    display_word[i*2] = '_';
                }
                display_word[i*2+1] = ' ';
            }
            TextOutA(hdc, 200, 150, display_word, strlen(display_word));

            // Draw Status Message
            if (game_won) {
                SetTextColor(hdc, RGB(76, 175, 80));
                TextOutA(hdc, 200, 200, "You Win!", 8);
            } else if (game_over) {
                SetTextColor(hdc, RGB(244, 67, 54));
                char msg[100];
                wsprintfA(msg, "Game Over! Word was %s", current_word);
                TextOutA(hdc, 200, 200, msg, strlen(msg));
            } else {
                SetTextColor(hdc, RGB(255, 255, 255));
                TextOutA(hdc, 200, 200, "Guess a letter", 14);
            }

            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            
            // Hangman base drawing (text)
            HFONT hMono = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                      FIXED_PITCH | FF_MODERN, "Consolas");
            hOldFont = (HFONT)SelectObject(hdc, hMono);
            SetTextColor(hdc, RGB(255, 82, 82));
            
            const char* drawing = DRAWINGS[errors];
            RECT drawRect = {20, 80, 150, 300};
            DrawTextA(hdc, drawing, -1, &drawRect, DT_LEFT | DT_TOP);

            SelectObject(hdc, hOldFont);
            DeleteObject(hMono);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CTLCOLORBTN:
        case WM_CTLCOLORSTATIC: {
            HDC hdcBtn = (HDC)wParam;
            SetBkMode(hdcBtn, TRANSPARENT);
            SetTextColor(hdcBtn, RGB(255, 255, 255));
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KHangmanWindow";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,                              
        CLASS_NAME,                     
        "KHangman",                     
        WS_OVERLAPPEDWINDOW,            
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 600,
        NULL,                           
        NULL,                           
        hInstance,                      
        NULL                            
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            char key = (char)msg.wParam;
            if (key >= 'A' && key <= 'Z') {
                GuessLetter(hwnd, key);
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
