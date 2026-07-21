#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define W 500
#define H 650

#define NUM_CATEGORIES 10
const char* CAT_NAMES[NUM_CATEGORIES] = {"Technology", "Animals", "Countries", "Science", "Movies", "Sports", "Food", "Music", "History", "Custom"};

const char* CAT_WORDS[NUM_CATEGORIES][20] = {
    {"COMPUTER", "PROGRAM", "APPLICATION", "SOFTWARE", "DEVELOPER", "INTERNET", "BROWSER", "HARDWARE", "NETWORK", "DATABASE", "KEYBOARD", "MONITOR", "ALGORITHM", "COMPILER", "PROCESSOR", "MEMORY", "VARIABLE", "FUNCTION", "ROUTER", "SERVER"},
    {"ELEPHANT", "GIRAFFE", "KANGAROO", "PENGUIN", "DOLPHIN", "TIGER", "MONKEY", "CHEETAH", "GORILLA", "RHINO", "ALLIGATOR", "CROCODILE", "PLATYPUS", "OSTRICH", "OCTOPUS", "PANTHER", "LEOPARD", "WALRUS", "CHIMPANZEE", "HIPPOPOTAMUS"},
    {"AUSTRALIA", "BRAZIL", "CANADA", "DENMARK", "EGYPT", "FRANCE", "GERMANY", "JAPAN", "MEXICO", "SPAIN", "ARGENTINA", "BELGIUM", "CHILE", "GREECE", "INDIA", "ITALY", "NORWAY", "SWEDEN", "THAILAND", "VIETNAM"},
    {"PHYSICS", "CHEMISTRY", "BIOLOGY", "ASTRONOMY", "GEOLOGY", "GRAVITY", "MOLECULE", "ATOM", "ENERGY", "RADIATION", "ECOLOGY", "GENETICS", "EVOLUTION", "QUANTUM", "VELOCITY", "PARTICLE", "THERMODYNAMICS", "NEUROLOGY", "BACTERIA", "TELESCOPE"},
    {"TITANIC", "AVATAR", "GLADIATOR", "MATRIX", "INCEPTION", "GODFATHER", "JAWS", "ROCKY", "TERMINATOR", "ALIEN", "JURASSIC", "CASABLANCA", "SUPERMAN", "BATMAN", "SPIDERMAN", "AVENGERS", "PREDATOR", "GHOSTBUSTERS", "HALLOWEEN", "PSYCHO"},
    {"BASKETBALL", "FOOTBALL", "BASEBALL", "SOCCER", "TENNIS", "CRICKET", "VOLLEYBALL", "RUGBY", "GOLF", "HOCKEY", "BADMINTON", "SWIMMING", "BOXING", "WRESTLING", "CYCLING", "ARCHERY", "FENCING", "GYMNASTICS", "MARATHON", "SURFING"},
    {"PIZZA", "BURGER", "SUSHI", "PASTA", "TACO", "STEAK", "SALAD", "SOUP", "SANDWICH", "NOODLES", "PANCAKE", "WAFFLE", "CHICKEN", "CHEESE", "BREAD", "RICE", "POTATO", "APPLE", "BANANA", "CHOCOLATE"},
    {"GUITAR", "PIANO", "DRUMS", "VIOLIN", "FLUTE", "TRUMPET", "SAXOPHONE", "CELLO", "BASS", "VOCALS", "MELODY", "RHYTHM", "CHORD", "ORCHESTRA", "SYMPHONY", "JAZZ", "ROCK", "BLUES", "CLASSICAL", "POP"},
    {"ROME", "GREECE", "EGYPT", "EMPIRE", "REVOLUTION", "WAR", "KING", "QUEEN", "KNIGHT", "CASTLE", "PYRAMID", "PHARAOH", "GLADIATOR", "SAMURAI", "NINJA", "VIKING", "PIRATE", "COLONY", "TREATY", "DISCOVERY"}
};
const int NUM_WORDS_PER_CAT = 20;
int current_category = 0;
int is_campaign = 0;
int campaign_level = 1;
int max_errors = 6;
int bombs = 1;



HWND hCustomEdit = NULL;
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
    int is_campaign;
    int campaign_level;
    int max_errors;
    int bombs;
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
    st.is_campaign = is_campaign;
    st.campaign_level = campaign_level;
    st.max_errors = max_errors;
    st.bombs = bombs;
    
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
            is_campaign = st.is_campaign;
            campaign_level = st.campaign_level;
            max_errors = st.max_errors;
            bombs = st.bombs;
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
    
    if (is_campaign) {
        current_category = (campaign_level - 1) % 9;
        max_errors = 7 - (campaign_level / 2);
        if (max_errors < 4) max_errors = 4;
        if (campaign_level == 1) bombs = 3;
    } else {
        max_errors = 6;
        bombs = 1;
    }
    
    if (current_category == NUM_CATEGORIES - 1) {
        char buf[1024] = {0};
        if (hCustomEdit) {
            GetWindowTextA(hCustomEdit, buf, sizeof(buf));
        } else {
            strcpy(buf, "APPLE, BANANA");
        }
        
        char words[50][32];
        int w_count = 0;
        char *p = buf;
        while (*p && w_count < 50) {
            while (*p == ' ' || *p == ',') p++;
            if (!*p) break;
            int i = 0;
            while (*p && *p != ',' && i < 31) {
                if (*p >= 'a' && *p <= 'z') words[w_count][i++] = *p - 32;
                else if (*p >= 'A' && *p <= 'Z') words[w_count][i++] = *p;
                p++;
            }
            words[w_count][i] = '\0';
            if (i > 0) w_count++;
            while (*p && *p != ',') p++;
        }
        if (w_count == 0) {
            strcpy(target_word, "CUSTOM");
        } else {
            int w_idx = CustomRand() % w_count;
            strcpy(target_word, words[w_idx]);
        }
    } else {
        int w_idx = CustomRand() % NUM_WORDS_PER_CAT;
        int i = 0;
        while(CAT_WORDS[current_category][w_idx][i]) {
            target_word[i] = CAT_WORDS[current_category][w_idx][i];
            i++;
        }
        target_word[i] = '\0';
    }
    
    for(int i=0; i<26; i++) guessed[i] = 0;
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
        if (errors >= max_errors) {
            game_over = 1;
            won = 0;
            PlaySoundEffect(4); // lose
            stats.losses++;
            stats.streak = 0;
            if (is_campaign) {
                is_campaign = 0;
                campaign_level = 1;
            }
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
            hCustomEdit = CreateWindowEx(0, "EDIT", "APPLE, BANANA", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_LEFT, 100, 105, 300, 25, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);
            if (current_category == NUM_CATEGORIES - 1) ShowWindow(hCustomEdit, SW_SHOW);
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
            
            // Bomb button
            if (x >= 20 && x <= 80 && y >= 530 && y <= 560) {
                if (bombs > 0 && !game_over) {
                    bombs--;
                    int elims = 0;
                    for (int k = 0; k < 50 && elims < 3; k++) {
                        int r = CustomRand() % 26;
                        if (!guessed[r]) {
                            int in_word = 0;
                            for (int w = 0; target_word[w] != '\0'; w++) {
                                if (target_word[w] == 'A' + r) { in_word = 1; break; }
                            }
                            if (!in_word) {
                                guessed[r] = 1;
                                elims++;
                            }
                        }
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                break;
            }

            // Hint button
            if (x >= 90 && x <= 150 && y >= 530 && y <= 560) {
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

            // Campaign button
            if (x >= 160 && x <= 250 && y >= 530 && y <= 560) {
                SetFocus(hwnd);
                if (game_over && won && is_campaign && campaign_level < 10) {
                    campaign_level++;
                } else {
                    is_campaign = 1;
                    campaign_level = 1;
                }
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Restart button
            if (x >= 260 && x <= 350 && y >= 530 && y <= 560) {
                SetFocus(hwnd);
                is_campaign = 0;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            
            // Save button
            if (x >= 150 && x <= 220 && y >= 570 && y <= 600) {
                SaveGame(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            
            // Load button
            if (x >= 230 && x <= 300 && y >= 570 && y <= 600) {
                LoadGame(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Help button
            if (x >= 310 && x <= 370 && y >= 570 && y <= 600) {
                MessageBoxA(hwnd,
                    "How to Play KHangman\n\n"
                    "Rules: Guess the hidden word one letter at a time.\n"
                    "Campaign Mode: 10 stages of increasing difficulty.\n"
                    "Bomb: Eliminates up to 3 incorrect letters from the keyboard.\n"
                    "Hint: Reveals one correct letter.\n"
                    "Categories: Select from predefined themes or choose Custom.",
                    "Help / How to Play", MB_OK | MB_ICONINFORMATION);
                break;
            }

            // Mute button
            if (x >= 380 && x <= 450 && y >= 570 && y <= 600) {
                is_muted = !is_muted;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Check categories
            int cx = 15;
            int cy = 40;
            for (int i = 0; i < NUM_CATEGORIES; i++) {
                if (x >= cx && x <= cx + 85 && y >= cy && y <= cy + 25) {
                    if (current_category != i) {
                        current_category = i;
                        is_campaign = 0;
                        if (current_category == NUM_CATEGORIES - 1) ShowWindow(hCustomEdit, SW_SHOW);
                        else ShowWindow(hCustomEdit, SW_HIDE);
                        SetFocus(hwnd);
                        InitGame();
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                    break;
                }
                cx += 95;
                if (i == 4) {
                    cx = 15;
                    cy = 70;
                }
            }

            // Check keyboard clicks
            int kx = 110;
            int ky = 360;
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
            int cx = 15;
            int cy = 40;
            SelectObject(memDC, hFontMono);
            for (int i = 0; i < NUM_CATEGORIES; i++) {
                RECT cRect = {cx, cy, cx + 85, cy + 25};
                HBRUSH cBg = CreateSolidBrush((i == current_category && !is_campaign) ? RGB(0, 122, 204) : RGB(44, 44, 44));
                FillRect(memDC, &cRect, cBg);
                DeleteObject(cBg);
                SetTextColor(memDC, RGB(255, 255, 255));
                DrawTextA(memDC, CAT_NAMES[i], -1, &cRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                cx += 95;
                if (i == 4) { cx = 15; cy = 70; }
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
            MoveToEx(memDC, 190 + ox, 280, NULL);
            LineTo(memDC, 310 + ox, 280); // Base
            MoveToEx(memDC, 210 + ox, 280, NULL);
            LineTo(memDC, 210 + ox, 140);  // Pole
            LineTo(memDC, 270 + ox, 140);  // Top
            LineTo(memDC, 270 + ox, 160);  // Rope
            
            if (errors > 0) {
                // Head
                Ellipse(memDC, 250 + ox, 160, 290 + ox, 200);
            }
            if (errors > 1) {
                // Body
                MoveToEx(memDC, 270 + ox, 200, NULL);
                LineTo(memDC, 270 + ox, 240);
            }
            if (errors > 2) {
                // Left arm
                MoveToEx(memDC, 270 + ox, 210, NULL);
                LineTo(memDC, 240 + ox, 240);
            }
            if (errors > 3) {
                // Right arm
                MoveToEx(memDC, 270 + ox, 210, NULL);
                LineTo(memDC, 300 + ox, 240);
            }
            if (errors > 4) {
                // Left leg
                MoveToEx(memDC, 270 + ox, 240, NULL);
                LineTo(memDC, 240 + ox, 270);
            }
            if (errors > 5) {
                // Right leg
                MoveToEx(memDC, 270 + ox, 240, NULL);
                LineTo(memDC, 300 + ox, 270);
            }
            if (errors > 6) {
                // Extra error (Campaign mode 7 errors)
                MoveToEx(memDC, 260 + ox, 175, NULL);
                LineTo(memDC, 265 + ox, 180);
                MoveToEx(memDC, 265 + ox, 175, NULL);
                LineTo(memDC, 260 + ox, 180);
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
            TextOutA(memDC, (W - tSize.cx) / 2, 290, disp, len);

            // Message
            char* msgTxt = "Guess a letter to start";
            COLORREF msgColor = RGB(224, 224, 224);
            char infoMsg[100];
            if (is_campaign) {
                wsprintfA(infoMsg, "Campaign Stage %d / 10", campaign_level);
                msgTxt = infoMsg;
                msgColor = RGB(255, 193, 7);
            }
            if (game_over) {
                if (won) {
                    if (is_campaign && campaign_level == 10) msgTxt = "Campaign Complete!";
                    else msgTxt = "You Win!";
                    msgColor = RGB(76, 175, 80);
                } else {
                    static char loseMsg[100];
                    wsprintfA(loseMsg, "Game Over! Word: %s", target_word);
                    msgTxt = loseMsg;
                    msgColor = RGB(244, 67, 54);
                }
            }
            SetTextColor(memDC, msgColor);
            GetTextExtentPoint32A(memDC, msgTxt, lstrlenA(msgTxt), &tSize);
            TextOutA(memDC, (W - tSize.cx) / 2, 330, msgTxt, lstrlenA(msgTxt));

            // Keyboard
            SelectObject(memDC, hFontMono);
            int kx = 110;
            int ky = 360;
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

            // Bomb button
            RECT bombRect = {20, 530, 80, 560};
            HBRUSH bombBg;
            if (bombs <= 0 || game_over) {
                bombBg = CreateSolidBrush(RGB(68, 68, 68));
                SetTextColor(memDC, RGB(119, 119, 119));
            } else {
                bombBg = CreateSolidBrush(RGB(244, 67, 54));
                SetTextColor(memDC, RGB(255, 255, 255));
            }
            FillRect(memDC, &bombRect, bombBg);
            DeleteObject(bombBg);
            HFONT hFontBtn = CreateFontA(14, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            SelectObject(memDC, hFontBtn);
            char bombTxt[32];
            wsprintfA(bombTxt, "Bomb(%d)", bombs);
            DrawTextA(memDC, bombTxt, -1, &bombRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Hint button
            RECT hintRect = {90, 530, 150, 560};
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
            DrawTextA(memDC, "Hint", -1, &hintRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Campaign button
            RECT campRect = {160, 530, 250, 560};
            HBRUSH campBg = CreateSolidBrush(is_campaign ? RGB(0, 150, 136) : RGB(100, 100, 100));
            FillRect(memDC, &campRect, campBg);
            DeleteObject(campBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            if (game_over && won && is_campaign && campaign_level < 10) {
                DrawTextA(memDC, "Next Stage", -1, &campRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else {
                DrawTextA(memDC, "Campaign", -1, &campRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            // Restart button
            RECT resRect = {260, 530, 350, 560};
            HBRUSH resBg = CreateSolidBrush(RGB(0, 122, 204));
            FillRect(memDC, &resRect, resBg);
            DeleteObject(resBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Freeplay", -1, &resRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            // Save button
            RECT saveRect = {150, 570, 220, 600};
            HBRUSH saveBg = CreateSolidBrush(RGB(76, 175, 80));
            FillRect(memDC, &saveRect, saveBg);
            DeleteObject(saveBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Save", -1, &saveRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Load button
            RECT loadRect = {230, 570, 300, 600};
            HBRUSH loadBg = CreateSolidBrush(RGB(156, 39, 176));
            FillRect(memDC, &loadRect, loadBg);
            DeleteObject(loadBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Load", -1, &loadRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Help button
            RECT helpRect = {310, 570, 370, 600};
            HBRUSH helpBg = CreateSolidBrush(RGB(33, 150, 243));
            FillRect(memDC, &helpRect, helpBg);
            DeleteObject(helpBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Help", -1, &helpRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Mute button
            RECT muteRect = {380, 570, 450, 600};
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
            TextOutA(memDC, (W - tSize.cx) / 2, 615, statText, lstrlenA(statText));

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
