#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define ID_EASY 1001
#define ID_HARD 1002
#define ID_EXPERT 1003
#define ID_CAMPAIGN 1006
#define ID_RUSH 1007
#define ID_RESTART 1004
#define ID_PEEK 1005
#define ID_THEME_LETTERS 1010
#define ID_THEME_NUMBERS 1011
#define ID_THEME_SYMBOLS 1012

int W = 400;
int H = 400;
int ROWS = 4;
int COLS = 4;

int cards[1024];
int flipped[1024] = {0};
int matched[1024] = {0};
int firstFlip = -1;
int secondFlip = -1;
int matches = 0;
int moves = 0;
int score = 0;
int combo = 1;
int streak = 1;

int best_easy = -1;
int best_hard = -1;
int best_expert = -1;
int stats_pairs = 0;
int stats_wins = 0;
int stats_campaign = 0;
int campaign_level = 1;
int rush_level = 1;
int rush_time_left = 60;
int is_hard = 0; // 0=easy, 1=hard, 2=expert, 3=campaign, 4=rush
int theme = 0; // 0=letters, 1=numbers, 2=symbols
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
        fread(&stats_pairs, sizeof(int), 1, f);
        fread(&stats_wins, sizeof(int), 1, f);
        fread(&stats_campaign, sizeof(int), 1, f);
        fclose(f);
    }
}

void SaveScores() {
    FILE *f = fopen("kmemory.dat", "wb");
    if (f) {
        fwrite(&best_easy, sizeof(int), 1, f);
        fwrite(&best_hard, sizeof(int), 1, f);
        fwrite(&best_expert, sizeof(int), 1, f);
        fwrite(&stats_pairs, sizeof(int), 1, f);
        fwrite(&stats_wins, sizeof(int), 1, f);
        fwrite(&stats_campaign, sizeof(int), 1, f);
        fclose(f);
    }
}

void Shuffle(HWND hwnd) {
    int numCards = ROWS * COLS;
    for (int i = 0; i < numCards; i++) {
        int val = i / 2;
        // Inject Power-Ups in Campaign Mode or Expert or Rush
        if ((is_hard == 2 || is_hard == 3 || is_hard == 4) && numCards >= 24) {
            if (val == (numCards/2 - 1)) val = 100; // Clock
            else if (val == (numCards/2 - 2)) val = 101; // Shuffle Trap
            else if (val == (numCards/2 - 3)) val = 102; // Bomb (-B)
            else if (val == (numCards/2 - 4) && numCards >= 48) val = 103; // X-Ray (+X)
            else if (val == (numCards/2 - 5) && numCards >= 64) val = 104; // Ghost (+G)
            else if (val == (numCards/2 - 6) && numCards >= 80) val = 105; // Freeze (+F)
        }
        cards[i] = val;
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
    combo = 1;
    
    if (is_hard != 4) {
        elapsed_time = 0;
        timer_running = 0;
    } else {
        // In Rush Mode, timer keeps running across boards, unless stopped.
        if (!timer_running) {
            start_time = GetTickCount();
            timer_running = 1;
            SetTimer(hwnd, 2, 1000, NULL);
        }
    }
    
    if (hwnd) {
        is_previewing = 1;
        for (int i = 0; i < numCards; i++) flipped[i] = 1;
        SetTimer(hwnd, 3, (is_hard == 2 ? 1000 : (is_hard == 1 ? 2000 : 3000)), NULL);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void SetDifficulty(HWND hwnd, int hard) {
    is_hard = hard;
    if (hard == 4) {
        if (rush_level == 1) { ROWS = 4; COLS = 4; W = 400; H = 400; }
        else if (rush_level == 2) { ROWS = 4; COLS = 6; W = 600; H = 400; }
        else if (rush_level == 3) { ROWS = 4; COLS = 8; W = 800; H = 400; }
        else if (rush_level == 4) { ROWS = 6; COLS = 8; W = 800; H = 600; }
        else { ROWS = 8; COLS = 8; W = 800; H = 800; }
    } else if (hard == 3) {
        if (campaign_level == 1) { ROWS = 4; COLS = 4; W = 400; H = 400; }
        else if (campaign_level == 2) { ROWS = 4; COLS = 6; W = 600; H = 400; }
        else if (campaign_level == 3) { ROWS = 4; COLS = 8; W = 800; H = 400; }
        else if (campaign_level == 4) { ROWS = 6; COLS = 8; W = 800; H = 600; }
        else if (campaign_level == 5) { ROWS = 8; COLS = 8; W = 800; H = 800; }
        else if (campaign_level == 6) { ROWS = 8; COLS = 10; W = 1000; H = 800; }
        else if (campaign_level == 7) { ROWS = 10; COLS = 10; W = 1000; H = 800; }
        else if (campaign_level == 8) { ROWS = 10; COLS = 12; W = 1200; H = 800; }
        else if (campaign_level == 9) { ROWS = 12; COLS = 12; W = 1200; H = 800; }
        else if (campaign_level == 10) { ROWS = 12; COLS = 14; W = 1400; H = 800; }
        else if (campaign_level == 11) { ROWS = 14; COLS = 14; W = 1400; H = 1000; }
        else if (campaign_level == 12) { ROWS = 14; COLS = 16; W = 1600; H = 1000; }
        else if (campaign_level == 13) { ROWS = 16; COLS = 16; W = 1600; H = 1100; }
        else if (campaign_level == 14) { ROWS = 16; COLS = 18; W = 1800; H = 1100; }
        else { ROWS = 18; COLS = 18; W = 1800; H = 1200; }
    } else if (hard == 2) {
        ROWS = 4; COLS = 8; W = 800; H = 400;
    } else if (hard == 1) {
        ROWS = 4; COLS = 6; W = 600; H = 400;
    } else {
        ROWS = 4; COLS = 4; W = 400; H = 400;
    }
    RECT rc = {0, 0, W, H + 40}; // extra for status bar text
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, TRUE);
    SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
    if (timer_running && hard != 4) KillTimer(hwnd, 2);
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
        
        char text[8] = {0};
        SetBkMode(hdc, TRANSPARENT);
        if (cards[idx] == 100) { strcpy(text, "+T"); SetTextColor(hdc, RGB(0, 0, 255)); }
        else if (cards[idx] == 101) { strcpy(text, "??"); SetTextColor(hdc, RGB(255, 0, 0)); }
        else if (cards[idx] == 102) { strcpy(text, "-B"); SetTextColor(hdc, RGB(128, 0, 0)); }
        else if (cards[idx] == 103) { strcpy(text, "+X"); SetTextColor(hdc, RGB(255, 0, 255)); }
        else if (cards[idx] == 104) { strcpy(text, "+G"); SetTextColor(hdc, RGB(128, 128, 128)); }
        else if (cards[idx] == 105) { strcpy(text, "+F"); SetTextColor(hdc, RGB(0, 255, 255)); }
        else if (theme == 0) {
            if (cards[idx] < 26) {
                text[0] = 'A' + cards[idx];
            } else {
                text[0] = 'A' + (cards[idx] / 26) - 1;
                text[1] = 'A' + (cards[idx] % 26);
            }
            SetTextColor(hdc, RGB(0, 0, 0));
        } else if (theme == 1) {
            sprintf(text, "%d", cards[idx] + 1);
            SetTextColor(hdc, RGB(0, 0, 0));
        } else {
            char sym[] = "!@#$%^&*+=?~OX<>";
            if (cards[idx] < 16) {
                text[0] = sym[cards[idx]];
            } else {
                text[0] = sym[cards[idx] / 16 - 1];
                text[1] = sym[cards[idx] % 16];
            }
            SetTextColor(hdc, RGB(0, 0, 0));
        }
        TextOutA(hdc, x + w / 2 - 8, y + h / 2 - 8, text, strlen(text));
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
            AppendMenu(hSubMenu, MF_STRING, ID_CAMPAIGN, "Campaign Mode");
            AppendMenu(hSubMenu, MF_STRING, ID_RUSH, "Rush Mode (Time Attack)");
            AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hSubMenu, MF_STRING, ID_PEEK, "Peek (+5 moves, -200 pts)");
            AppendMenu(hSubMenu, MF_STRING, ID_RESTART, "Restart");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "Game");
            
            HMENU hThemeMenu = CreatePopupMenu();
            AppendMenu(hThemeMenu, MF_STRING, ID_THEME_LETTERS, "Letters");
            AppendMenu(hThemeMenu, MF_STRING, ID_THEME_NUMBERS, "Numbers");
            AppendMenu(hThemeMenu, MF_STRING, ID_THEME_SYMBOLS, "Symbols");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hThemeMenu, "Theme");
            
            SetMenu(hwnd, hMenu);

            LoadScores();
            seed = GetTickCount();
            SetDifficulty(hwnd, 0); // start easy
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_EASY) {
                score = 0; SetDifficulty(hwnd, 0);
            } else if (LOWORD(wParam) == ID_HARD) {
                score = 0; SetDifficulty(hwnd, 1);
            } else if (LOWORD(wParam) == ID_EXPERT) {
                score = 0; SetDifficulty(hwnd, 2);
            } else if (LOWORD(wParam) == ID_CAMPAIGN) {
                score = 0; SetDifficulty(hwnd, 3);
            } else if (LOWORD(wParam) == ID_RUSH) {
                rush_level = 1;
                rush_time_left = 60;
                score = 0;
                SetDifficulty(hwnd, 4);
            } else if (LOWORD(wParam) == ID_PEEK) {
                if (!is_previewing && firstFlip == -1 && (timer_running || is_hard == 4)) {
                    moves += 5;
                    score -= 200;
                    if (score < 0) score = 0;
                    combo = 1;
                    int numCards = ROWS * COLS;
                    for (int i = 0; i < numCards; i++) {
                        if (!matched[i]) flipped[i] = 1;
                    }
                    is_previewing = 1;
                    SetTimer(hwnd, 3, 1500, NULL);
                    InvalidateRect(hwnd, NULL, FALSE);
                    MessageBeep(MB_ICONASTERISK);
                }
            } else if (LOWORD(wParam) == ID_RESTART) {
                if (is_hard == 3) campaign_level = 1;
                if (is_hard == 4) { rush_level = 1; rush_time_left = 60; }
                score = 0;
                SetDifficulty(hwnd, is_hard);
            } else if (LOWORD(wParam) == ID_THEME_LETTERS) {
                theme = 0; SetDifficulty(hwnd, is_hard);
            } else if (LOWORD(wParam) == ID_THEME_NUMBERS) {
                theme = 1; SetDifficulty(hwnd, is_hard);
            } else if (LOWORD(wParam) == ID_THEME_SYMBOLS) {
                theme = 2; SetDifficulty(hwnd, is_hard);
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
                    if (!timer_running && is_hard != 4) {
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
                            score += 100 * combo;
                            combo++;
                            matched[firstFlip] = 1;
                            matched[secondFlip] = 1;
                            stats_pairs++;
                            
                            if (is_hard == 4) rush_time_left += 2;
                            
                            if (cards[firstFlip] == 100) {
                                score += 500;
                                if (is_hard == 4) rush_time_left += 10;
                                else elapsed_time = (elapsed_time > 10) ? elapsed_time - 10 : 0;
                            } else if (cards[firstFlip] == 101) {
                                // Shuffle remaining
                                int numCards = ROWS * COLS;
                                for (int i = numCards - 1; i > 0; i--) {
                                    if (!matched[i]) {
                                        int j = rnd() % (i + 1);
                                        while(matched[j]) j = rnd() % (i + 1);
                                        if (!matched[j]) {
                                            int temp = cards[i];
                                            cards[i] = cards[j];
                                            cards[j] = temp;
                                        }
                                    }
                                }
                            } else if (cards[firstFlip] == 102) {
                                score -= 500;
                                if (score < 0) score = 0;
                                if (is_hard == 4) {
                                    rush_time_left -= 15;
                                    if (rush_time_left < 0) rush_time_left = 0;
                                } else {
                                    elapsed_time += 15;
                                }
                            } else if (cards[firstFlip] == 103) {
                                score += 500;
                                int numCards = ROWS * COLS;
                                for (int i = 0; i < numCards; i++) {
                                    if (!matched[i]) flipped[i] = 1;
                                }
                                is_previewing = 1;
                                SetTimer(hwnd, 3, 2000, NULL);
                            } else if (cards[firstFlip] == 104) {
                                score += 500;
                                int pairs_to_match = 2;
                                int numCards = ROWS * COLS;
                                for (int k = 0; k < numCards && pairs_to_match > 0; k++) {
                                    if (!matched[k] && cards[k] < 100) {
                                        for (int l = k + 1; l < numCards; l++) {
                                            if (!matched[l] && cards[l] == cards[k]) {
                                                matched[k] = 1;
                                                matched[l] = 1;
                                                matches++;
                                                pairs_to_match--;
                                                break;
                                            }
                                        }
                                    }
                                }
                            } else if (cards[firstFlip] == 105) {
                                score += 500;
                                if (is_hard == 4) rush_time_left += 15;
                                else elapsed_time = (elapsed_time > 15) ? elapsed_time - 15 : 0;
                            }
                            
                            firstFlip = -1;
                            secondFlip = -1;
                            matches++;
                            MessageBeep(MB_ICONASTERISK);
                            
                            if (matches >= (ROWS * COLS) / 2) {
                                if (is_hard != 4) {
                                    if (timer_running) {
                                        KillTimer(hwnd, 2);
                                        timer_running = 0;
                                    }
                                }
                                InvalidateRect(hwnd, NULL, FALSE);
                                UpdateWindow(hwnd);
                                
                                int is_new_best = 0;
                                if (is_hard == 2) {
                                    if (best_expert == -1 || score > best_expert) { best_expert = score; is_new_best = 1; }
                                } else if (is_hard == 1) {
                                    if (best_hard == -1 || score > best_hard) { best_hard = score; is_new_best = 1; }
                                } else if (is_hard == 0) {
                                    if (best_easy == -1 || score > best_easy) { best_easy = score; is_new_best = 1; }
                                }
                                stats_wins++;
                                SaveScores();
                                
                                if (is_hard == 4) {
                                    rush_level++;
                                    rush_time_left += 20;
                                    char msgBuf[256];
                                    sprintf(msgBuf, "Board %d Cleared! +20s", rush_level - 1);
                                    MessageBoxA(hwnd, msgBuf, "KMemory", MB_OK);
                                    SetDifficulty(hwnd, 4);
                                    return 0;
                                } else if (is_hard == 3) {
                                    stats_campaign++;
                                    campaign_level++;
                                    if (campaign_level > 15) {
                                        MessageBoxA(hwnd, "You beat the Campaign!", "KMemory", MB_OK);
                                        campaign_level = 1;
                                    }
                                } else {
                                    char msgBuf[256];
                                    sprintf(msgBuf, "You won with score %d in %d moves and %d seconds!", score, moves, elapsed_time);
                                    MessageBoxA(hwnd, msgBuf, "KMemory", MB_OK);
                                }
                                SetDifficulty(hwnd, is_hard);
                            }
                        } else {
                            combo = 1;
                            score -= 10;
                            if (score < 0) score = 0;
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
                for (int i = 0; i < numCards; i++) if (!matched[i]) flipped[i] = 0;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 1) {
                KillTimer(hwnd, 1);
                flipped[firstFlip] = 0;
                flipped[secondFlip] = 0;
                firstFlip = -1;
                secondFlip = -1;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 2) {
                if (is_hard == 4) {
                    rush_time_left--;
                    if (rush_time_left <= 0) {
                        KillTimer(hwnd, 2);
                        timer_running = 0;
                        rush_time_left = 0;
                        InvalidateRect(hwnd, NULL, FALSE);
                        UpdateWindow(hwnd);
                        char msgBuf[256];
                        sprintf(msgBuf, "Time's up! Rush Mode over. Score: %d", score);
                        MessageBoxA(hwnd, msgBuf, "KMemory", MB_OK);
                        rush_level = 1;
                        rush_time_left = 60;
                        score = 0;
                        SetDifficulty(hwnd, 4); // restart rush
                        return 0;
                    }
                } else {
                    elapsed_time = (GetTickCount() - start_time) / 1000;
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbm = CreateCompatibleBitmap(hdc, W, H + 40);
            HBITMAP oldBm = (HBITMAP)SelectObject(memDC, hbm);
            
            // Draw status area
            RECT statusRc = {0, 0, W, 40};
            HBRUSH statusBg = CreateSolidBrush(RGB(20, 20, 20));
            FillRect(memDC, &statusRc, statusBg);
            DeleteObject(statusBg);
            
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(255, 255, 255));
            char statusText[256];
            int best = (is_hard == 2) ? best_expert : (is_hard == 1 ? best_hard : best_easy);
            if (is_hard == 4)
                sprintf(statusText, "Rush %d | Time Left: %ds  Moves: %d  Score: %d | Total Wins: %d", rush_level, rush_time_left, moves, score, stats_wins);
            else if (is_hard == 3)
                sprintf(statusText, "Campaign %d/15 | Time: %ds  Moves: %d  Score: %d | Total Wins: %d", campaign_level, elapsed_time, moves, score, stats_wins);
            else if (best == -1)
                sprintf(statusText, "Time: %ds  Moves: %d  Score: %d (x%d)  Best: - | Pairs: %d", elapsed_time, moves, score, combo, stats_pairs);
            else
                sprintf(statusText, "Time: %ds  Moves: %d  Score: %d (x%d)  Best: %d | Pairs: %d", elapsed_time, moves, score, combo, best, stats_pairs);
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
            SelectObject(memDC, oldBm);
            DeleteObject(hbm);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            KillTimer(hwnd, 2);
            KillTimer(hwnd, 3);
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
