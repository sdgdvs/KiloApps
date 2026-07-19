#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define BTN_GREEN  0
#define BTN_RED    1
#define BTN_YELLOW 2
#define BTN_BLUE   3

#define TIMER_SEQUENCE 1
#define TIMER_FLASH    2

#define MODE_CLASSIC 0
#define MODE_REVERSE 1
#define MODE_SPEED 2

int btn_freqs[4] = {415, 329, 261, 196};

DWORD WINAPI PlayBeep(LPVOID lpParam) {
    INT_PTR param = (INT_PTR)lpParam;
    int freq = param & 0xFFFF;
    int duration = (param >> 16) & 0xFFFF;
    Beep(freq, duration);
    return 0;
}

void PlaySoundAsync(int freq, int duration) {
    INT_PTR param = (freq & 0xFFFF) | ((duration & 0xFFFF) << 16);
    CreateThread(NULL, 0, PlayBeep, (LPVOID)param, 0, NULL);
}

HWND hwndMain;
HWND hwndModeBox;
int current_mode = MODE_CLASSIC;

int sequence[1000];
int sequence_length = 0;
int player_step = 0;
int is_playing_sequence = 0;
int current_flash_index = 0;
int flash_btn = -1;

RECT btn_rects[4];
COLORREF btn_colors[4] = {
    RGB(0, 170, 0),    // Green
    RGB(170, 0, 0),    // Red
    RGB(170, 170, 0),  // Yellow
    RGB(0, 0, 170)     // Blue
};
COLORREF flash_colors[4] = {
    RGB(0, 255, 0),
    RGB(255, 0, 0),
    RGB(255, 255, 0),
    RGB(0, 0, 255)
};

char status_text[128] = "Press Space to Start";
int score = 0;
int high_scores[3] = {0, 0, 0};

void LoadHighScores() {
    high_scores[MODE_CLASSIC] = GetPrivateProfileInt("HighScores", "Classic", 0, ".\\ksimon.ini");
    high_scores[MODE_REVERSE] = GetPrivateProfileInt("HighScores", "Reverse", 0, ".\\ksimon.ini");
    high_scores[MODE_SPEED]   = GetPrivateProfileInt("HighScores", "Speed", 0, ".\\ksimon.ini");
}

void SaveHighScore(int mode, int s) {
    char str[32];
    sprintf(str, "%d", s);
    if (mode == MODE_CLASSIC) WritePrivateProfileString("HighScores", "Classic", str, ".\\ksimon.ini");
    else if (mode == MODE_REVERSE) WritePrivateProfileString("HighScores", "Reverse", str, ".\\ksimon.ini");
    else if (mode == MODE_SPEED) WritePrivateProfileString("HighScores", "Speed", str, ".\\ksimon.ini");
}

void DrawBoard(HDC hdc) {
    for (int i = 0; i < 4; i++) {
        HBRUSH brush = CreateSolidBrush(flash_btn == i ? flash_colors[i] : btn_colors[i]);
        FillRect(hdc, &btn_rects[i], brush);
        DeleteObject(brush);
    }

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(224, 224, 224));
    TextOutA(hdc, 10, 10, status_text, strlen(status_text));
    
    char score_text[32];
    sprintf(score_text, "Score: %d", score);
    TextOutA(hdc, 10, 30, score_text, strlen(score_text));

    char hi_score_text[64];
    sprintf(hi_score_text, "High Score: %d", high_scores[current_mode]);
    TextOutA(hdc, 10, 50, hi_score_text, strlen(hi_score_text));
}

void StartGame() {
    current_mode = SendMessage(hwndModeBox, CB_GETCURSEL, 0, 0);
    EnableWindow(hwndModeBox, FALSE);
    sequence_length = 0;
    score = 0;
    is_playing_sequence = 1;
    strcpy(status_text, "Get Ready...");
    InvalidateRect(hwndMain, NULL, TRUE);
    SetTimer(hwndMain, TIMER_SEQUENCE, 1000, NULL);
}

void NextRound() {
    player_step = 0;
    sequence[sequence_length++] = rand() % 4;
    score = sequence_length - 1;
    is_playing_sequence = 1;
    current_flash_index = 0;
    strcpy(status_text, "Watch...");
    InvalidateRect(hwndMain, NULL, TRUE);
    SetTimer(hwndMain, TIMER_SEQUENCE, 500, NULL);
}

void HandleClick(int btn_id) {
    if (is_playing_sequence || sequence_length == 0) return;

    flash_btn = btn_id;
    InvalidateRect(hwndMain, NULL, TRUE);
    SetTimer(hwndMain, TIMER_FLASH, (current_mode == MODE_SPEED) ? 150 : 300, NULL);

    int expected_index;
    if (current_mode == MODE_REVERSE) {
        expected_index = sequence[sequence_length - 1 - player_step];
    } else {
        expected_index = sequence[player_step];
    }

    if (btn_id != expected_index) {
        PlaySoundAsync(100, 800);
        if (score > high_scores[current_mode]) {
            high_scores[current_mode] = score;
            SaveHighScore(current_mode, score);
            sprintf(status_text, "Game Over! Score: %d (New High Score!)", score);
        } else {
            sprintf(status_text, "Game Over! Score: %d (Space to restart)", score);
        }
        sequence_length = 0;
        EnableWindow(hwndModeBox, TRUE);
        InvalidateRect(hwndMain, NULL, TRUE);
        return;
    }

    PlaySoundAsync(btn_freqs[btn_id], 200);

    player_step++;
    if (player_step == sequence_length) {
        strcpy(status_text, "Good! Get ready...");
        is_playing_sequence = 1;
        InvalidateRect(hwndMain, NULL, TRUE);
        SetTimer(hwndMain, TIMER_SEQUENCE, 1000, NULL);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hwndMain = hwnd;
            srand((unsigned)time(NULL));
            LoadHighScores();
            hwndModeBox = CreateWindowEx(
                0, "COMBOBOX", "", 
                CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL, 
                10, 70, 150, 100, hwnd, (HMENU)1001, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Classic Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Reverse Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Speed Mode");
            SendMessage(hwndModeBox, CB_SETCURSEL, 0, 0);
            break;
        case WM_COMMAND:
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == 1001) {
                current_mode = SendMessage(hwndModeBox, CB_GETCURSEL, 0, 0);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            int cx = width / 2;
            int cy = height / 2;
            int size = 100;
            SetRect(&btn_rects[0], cx - size - 5, cy - size - 5, cx - 5, cy - 5);
            SetRect(&btn_rects[1], cx + 5, cy - size - 5, cx + size + 5, cy - 5);
            SetRect(&btn_rects[2], cx - size - 5, cy + 5, cx - 5, cy + size + 5);
            SetRect(&btn_rects[3], cx + 5, cy + 5, cx + size + 5, cy + size + 5);
            break;
        }
        case WM_LBUTTONDOWN: {
            if (is_playing_sequence) break;
            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            for (int i = 0; i < 4; i++) {
                if (PtInRect(&btn_rects[i], pt)) {
                    HandleClick(i);
                    break;
                }
            }
            break;
        }
        case WM_KEYDOWN:
            if (wParam == VK_SPACE && sequence_length == 0) {
                StartGame();
            }
            break;
        case WM_TIMER:
            if (wParam == TIMER_FLASH) {
                KillTimer(hwnd, TIMER_FLASH);
                flash_btn = -1;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == TIMER_SEQUENCE) {
                if (sequence_length == 0) {
                    KillTimer(hwnd, TIMER_SEQUENCE);
                    NextRound();
                } else {
                    if (flash_btn == -1) {
                        if (current_flash_index >= sequence_length) {
                            KillTimer(hwnd, TIMER_SEQUENCE);
                            is_playing_sequence = 0;
                            if (current_mode == MODE_REVERSE) {
                                strcpy(status_text, "Your Turn (Reverse)!");
                            } else {
                                strcpy(status_text, "Your Turn!");
                            }
                            InvalidateRect(hwnd, NULL, TRUE);
                        } else {
                            flash_btn = sequence[current_flash_index++];
                            InvalidateRect(hwnd, NULL, TRUE);
                            PlaySoundAsync(btn_freqs[flash_btn], (current_mode == MODE_SPEED) ? 200 : 400);
                            SetTimer(hwnd, TIMER_SEQUENCE, (current_mode == MODE_SPEED) ? 200 : 400, NULL);
                        }
                    } else {
                        flash_btn = -1;
                        InvalidateRect(hwnd, NULL, TRUE);
                        SetTimer(hwnd, TIMER_SEQUENCE, (current_mode == MODE_SPEED) ? 100 : 200, NULL);
                    }
                }
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH bgBrush = CreateSolidBrush(RGB(26, 26, 26));
            FillRect(hdc, &ps.rcPaint, bgBrush);
            DeleteObject(bgBrush);
            DrawBoard(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KSimonClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KSimon",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        400, 450, NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
