#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define W 500
#define H 550

#define NUM_CATEGORIES 4
const char* CAT_NAMES[NUM_CATEGORIES] = {"Technology", "Animals", "Countries", "Science"};

const char* CAT_WORDS[NUM_CATEGORIES][10] = {
    {"COMPUTER", "PROGRAM", "APPLICATION", "SOFTWARE", "DEVELOPER", "INTERNET", "BROWSER", "HARDWARE", "NETWORK", "DATABASE"},
    {"ELEPHANT", "GIRAFFE", "KANGAROO", "PENGUIN", "DOLPHIN", "TIGER", "MONKEY", "CHEETAH", "GORILLA", "RHINO"},
    {"AUSTRALIA", "BRAZIL", "CANADA", "DENMARK", "EGYPT", "FRANCE", "GERMANY", "JAPAN", "MEXICO", "SPAIN"},
    {"PHYSICS", "CHEMISTRY", "BIOLOGY", "ASTRONOMY", "GEOLOGY", "GRAVITY", "MOLECULE", "ATOM", "ENERGY", "RADIATION"}
};
const int NUM_WORDS_PER_CAT = 10;
int current_category = 0;



int errors = 0;
char target_word[32];
int guessed[26] = {0};
int game_over = 0;
int won = 0;
int initialized = 0;
int hint_used = 0;
int is_muted = 0;
int shake_frames = 0;
int win_pulse_phase = 0;

void PlaySoundEffect(int type) {
    if (is_muted) return;
    if (type == 1) { // valid
        Beep(800, 100);
    } else if (type == 2) { // invalid
        Beep(200, 150);
    } else if (type == 3) { // win
        Beep(400, 100);
        Beep(600, 100);
        Beep(800, 200);
    } else if (type == 4) { // lose
        Beep(300, 200);
        Beep(150, 300);
    }
}

typedef struct {
    int wins;
    int losses;
    int streak;
    int best;
} Stats;

Stats stats = {0, 0, 0, 0};

void LoadStats() {
    FILE* f = fopen("khangman_stats.dat", "rb");
    if (f) {
        fread(&stats, sizeof(Stats), 1, f);
        fclose(f);
    }
}

void SaveStats() {
    FILE* f = fopen("khangman_stats.dat", "wb");
    if (f) {
        fwrite(&stats, sizeof(Stats), 1, f);
        fclose(f);
    }
}

typedef struct {
    char target_word[32];
    int current_category;
    int guessed[26];
    int errors;
    int hint_used;
    int game_over;
    int won;
} GameState;

void SaveGame(HWND hwnd) {
    if (game_over) {
        MessageBoxA(hwnd, "Cannot save a finished game.", "Save", MB_OK);
        return;
    }
    GameState st;
    strcpy(st.target_word, target_word);
    st.current_category = current_category;
    for (int i = 0; i < 26; i++) st.guessed[i] = guessed[i];
    st.errors = errors;
    st.hint_used = hint_used;
    st.game_over = game_over;
    st.won = won;
    
    FILE* f = fopen("khangman_save.dat", "wb");
    if (f) {
        fwrite(&st, sizeof(GameState), 1, f);
        fclose(f);
        MessageBoxA(hwnd, "Game saved successfully.", "Save", MB_OK);
    }
}

void LoadGame(HWND hwnd) {
    FILE* f = fopen("khangman_save.dat", "rb");
    if (f) {
        GameState st;
        if (fread(&st, sizeof(GameState), 1, f) == 1) {
            strcpy(target_word, st.target_word);
            current_category = st.current_category;
            for (int i = 0; i < 26; i++) guessed[i] = st.guessed[i];
            errors = st.errors;
            hint_used = st.hint_used;
            game_over = st.game_over;
            won = st.won;
            MessageBoxA(hwnd, "Game loaded successfully.", "Load", MB_OK);
        }
        fclose(f);
    } else {
        MessageBoxA(hwnd, "No saved game found.", "Load", MB_OK);
    }
}

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
        LoadStats();
        CustomSrand(GetTickCount());
        initialized = 1;
    }
    
    int w_idx = CustomRand() % NUM_WORDS_PER_CAT;
    int i = 0;
    while(CAT_WORDS[current_category][w_idx][i]) {
        target_word[i] = CAT_WORDS[current_category][w_idx][i];
        i++;
    }
    target_word[i] = '\0';
    
    for(i=0; i<26; i++) guessed[i] = 0;
    errors = 0;
    game_over = 0;
    won = 0;
    hint_used = 0;
    shake_frames = 0;
    win_pulse_phase = 0;
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
        shake_frames = 15;
        if (errors >= 6) {
            game_over = 1;
            won = 0;
            PlaySoundEffect(4); // lose
            stats.losses++;
            stats.streak = 0;
            SaveStats();
        } else {
            PlaySoundEffect(2); // invalid
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
            PlaySoundEffect(3); // win
            stats.wins++;
            stats.streak++;
            if (stats.streak > stats.best) stats.best = stats.streak;
            SaveStats();
        } else {
            PlaySoundEffect(1); // valid
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            InitGame();
            SetTimer(hwnd, 1, 30, NULL);
            break;

        case WM_TIMER:
            if (wParam == 1) {
                int needs_redraw = 0;
                if (shake_frames > 0) {
                    shake_frames--;
                    needs_redraw = 1;
                }
                if (game_over && won) {
                    win_pulse_phase += 10;
                    if (win_pulse_phase > 360) win_pulse_phase -= 360;
                    needs_redraw = 1;
                }
                if (needs_redraw) InvalidateRect(hwnd, NULL, TRUE);
            }
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
            
            // Check hint button
            if (x >= 30 && x <= 100 && y >= 450 && y <= 490) {
                if (!hint_used && !game_over) {
                    char unguessed[32];
                    int uCount = 0;
                    for (int i = 0; target_word[i] != '\0'; i++) {
                        if (!guessed[target_word[i] - 'A']) {
                            int already = 0;
                            for (int j = 0; j < uCount; j++) {
                                if (unguessed[j] == target_word[i]) { already = 1; break; }
                            }
                            if (!already) unguessed[uCount++] = target_word[i];
                        }
                    }
                    if (uCount > 0) {
                        hint_used = 1;
                        Guess(unguessed[CustomRand() % uCount]);
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
                break;
            }

            // Check restart button
            if (x >= 110 && x <= 210 && y >= 450 && y <= 490) {
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            
            // Check save button
            if (x >= 220 && x <= 290 && y >= 450 && y <= 490) {
                SaveGame(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            
            // Check load button
            if (x >= 300 && x <= 370 && y >= 450 && y <= 490) {
                LoadGame(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Check mute button
            if (x >= 380 && x <= 460 && y >= 450 && y <= 490) {
                is_muted = !is_muted;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Check categories
            int cx = 40;
            int cy = 45;
            for (int i = 0; i < NUM_CATEGORIES; i++) {
                if (x >= cx && x <= cx + 90 && y >= cy && y <= cy + 25) {
                    if (current_category != i) {
                        current_category = i;
                        InitGame();
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                    break;
                }
                cx += 105;
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
            
            HFONT hFontMain;
            if (game_over && won) {
                int size = 24 + (4 * abs(win_pulse_phase - 180) / 180);
                hFontMain = CreateFontA(size, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            } else {
                hFontMain = CreateFontA(24, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            }
            HFONT hFontMono = CreateFontA(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, FIXED_PITCH, "Consolas");
            
            // Title
            SetTextColor(memDC, RGB(79, 172, 254));
            SelectObject(memDC, hFontMain);
            TextOutA(memDC, 190, 10, "KHangman", 8);

            // Categories
            int cx = 40;
            int cy = 45;
            SelectObject(memDC, hFontMono);
            for (int i = 0; i < NUM_CATEGORIES; i++) {
                RECT cRect = {cx, cy, cx + 90, cy + 25};
                HBRUSH cBg = CreateSolidBrush(i == current_category ? RGB(0, 122, 204) : RGB(44, 44, 44));
                FillRect(memDC, &cRect, cBg);
                DeleteObject(cBg);
                SetTextColor(memDC, RGB(255, 255, 255));
                DrawTextA(memDC, CAT_NAMES[i], -1, &cRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                cx += 105;
            }

            // Drawing
            HPEN hPen = CreatePen(PS_SOLID, 4, RGB(255, 82, 82));
            HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
            
            int ox = 0;
            if (shake_frames > 0) {
                int offsets[] = {-10, 10, -8, 8, -6, 6, -4, 4, -2, 2, 0, 0, 0, 0, 0, 0};
                int idx = 15 - shake_frames;
                if (idx < 0) idx = 0;
                if (idx > 15) idx = 15;
                ox = offsets[idx];
            }
            
            // Base gallows
            MoveToEx(memDC, 190 + ox, 200, NULL);
            LineTo(memDC, 310 + ox, 200); // Base
            MoveToEx(memDC, 210 + ox, 200, NULL);
            LineTo(memDC, 210 + ox, 60);  // Pole
            LineTo(memDC, 270 + ox, 60);  // Top
            LineTo(memDC, 270 + ox, 80);  // Rope
            
            if (errors > 0) {
                // Head
                Ellipse(memDC, 250 + ox, 80, 290 + ox, 120);
            }
            if (errors > 1) {
                // Body
                MoveToEx(memDC, 270 + ox, 120, NULL);
                LineTo(memDC, 270 + ox, 160);
            }
            if (errors > 2) {
                // Left arm
                MoveToEx(memDC, 270 + ox, 130, NULL);
                LineTo(memDC, 240 + ox, 150);
            }
            if (errors > 3) {
                // Right arm
                MoveToEx(memDC, 270 + ox, 130, NULL);
                LineTo(memDC, 300 + ox, 150);
            }
            if (errors > 4) {
                // Left leg
                MoveToEx(memDC, 270 + ox, 160, NULL);
                LineTo(memDC, 240 + ox, 190);
            }
            if (errors > 5) {
                // Right leg
                MoveToEx(memDC, 270 + ox, 160, NULL);
                LineTo(memDC, 300 + ox, 190);
            }
            SelectObject(memDC, hOldPen);
            SelectObject(memDC, hOldBrush);
            DeleteObject(hPen);

            // Word display
            COLORREF wordColor = RGB(224, 224, 224);
            if (game_over && won) {
                int g = 175 + (50 * abs(win_pulse_phase - 180) / 180);
                int r = 76 + (50 * abs(win_pulse_phase - 180) / 180);
                int b = 80 + (50 * abs(win_pulse_phase - 180) / 180);
                wordColor = RGB(r, g, b);
            }
            SetTextColor(memDC, wordColor);
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

            // Hint button
            RECT hintRect = {30, 450, 100, 490};
            HBRUSH hintBg;
            if (hint_used || game_over) {
                hintBg = CreateSolidBrush(RGB(68, 68, 68));
                SetTextColor(memDC, RGB(119, 119, 119));
            } else {
                hintBg = CreateSolidBrush(RGB(255, 152, 0));
                SetTextColor(memDC, RGB(255, 255, 255));
            }
            FillRect(memDC, &hintRect, hintBg);
            DeleteObject(hintBg);
            
            HFONT hFontBtn = CreateFontA(16, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            SelectObject(memDC, hFontBtn);
            DrawTextA(memDC, "Hint", -1, &hintRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Restart button
            RECT resRect = {110, 450, 210, 490};
            HBRUSH resBg = CreateSolidBrush(RGB(0, 122, 204));
            FillRect(memDC, &resRect, resBg);
            DeleteObject(resBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "New Game", -1, &resRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            // Save button
            RECT saveRect = {220, 450, 290, 490};
            HBRUSH saveBg = CreateSolidBrush(RGB(76, 175, 80));
            FillRect(memDC, &saveRect, saveBg);
            DeleteObject(saveBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Save", -1, &saveRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Load button
            RECT loadRect = {300, 450, 370, 490};
            HBRUSH loadBg = CreateSolidBrush(RGB(156, 39, 176));
            FillRect(memDC, &loadRect, loadBg);
            DeleteObject(loadBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Load", -1, &loadRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Mute button
            RECT muteRect = {380, 450, 460, 490};
            HBRUSH muteBg = CreateSolidBrush(RGB(85, 85, 85));
            FillRect(memDC, &muteRect, muteBg);
            DeleteObject(muteBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, is_muted ? "Muted" : "Sound", -1, &muteRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            DeleteObject(hFontBtn);

            // Stats
            char statText[128];
            wsprintfA(statText, "Wins: %d  |  Losses: %d  |  Streak: %d  |  Best: %d", stats.wins, stats.losses, stats.streak, stats.best);
            SetTextColor(memDC, RGB(79, 172, 254));
            SelectObject(memDC, hFontMono);
            GetTextExtentPoint32A(memDC, statText, lstrlenA(statText), &tSize);
            TextOutA(memDC, (W - tSize.cx) / 2, 510, statText, lstrlenA(statText));

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
