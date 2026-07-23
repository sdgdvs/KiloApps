#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define W 500
#define H 670

#define NUM_CATEGORIES 14
#define TOTAL_CAT_COUNT 15 // 14 predefined + Custom

const char* CAT_NAMES[TOTAL_CAT_COUNT] = {
    "Technology", "Animals", "Countries", "Science", "Movies", 
    "Sports", "Food", "Music", "History", "Mythology", 
    "Space", "Literature", "Geography", "Pop Culture", "Custom"
};

const char* CAT_WORDS[NUM_CATEGORIES][20] = {
    {"COMPUTER", "PROGRAM", "APPLICATION", "SOFTWARE", "DEVELOPER", "INTERNET", "BROWSER", "HARDWARE", "NETWORK", "DATABASE", "KEYBOARD", "MONITOR", "ALGORITHM", "COMPILER", "PROCESSOR", "MEMORY", "VARIABLE", "FUNCTION", "ROUTER", "SERVER"},
    {"ELEPHANT", "GIRAFFE", "KANGAROO", "PENGUIN", "DOLPHIN", "TIGER", "MONKEY", "CHEETAH", "GORILLA", "RHINO", "ALLIGATOR", "CROCODILE", "PLATYPUS", "OSTRICH", "OCTOPUS", "PANTHER", "LEOPARD", "WALRUS", "CHIMPANZEE", "HIPPOPOTAMUS"},
    {"AUSTRALIA", "BRAZIL", "CANADA", "DENMARK", "EGYPT", "FRANCE", "GERMANY", "JAPAN", "MEXICO", "SPAIN", "ARGENTINA", "BELGIUM", "CHILE", "GREECE", "INDIA", "ITALY", "NORWAY", "SWEDEN", "THAILAND", "VIETNAM"},
    {"PHYSICS", "CHEMISTRY", "BIOLOGY", "ASTRONOMY", "GEOLOGY", "GRAVITY", "MOLECULE", "ATOM", "ENERGY", "RADIATION", "ECOLOGY", "GENETICS", "EVOLUTION", "QUANTUM", "VELOCITY", "PARTICLE", "THERMODYNAMICS", "NEUROLOGY", "BACTERIA", "TELESCOPE"},
    {"TITANIC", "AVATAR", "GLADIATOR", "MATRIX", "INCEPTION", "GODFATHER", "JAWS", "ROCKY", "TERMINATOR", "ALIEN", "JURASSIC", "CASABLANCA", "SUPERMAN", "BATMAN", "SPIDERMAN", "AVENGERS", "PREDATOR", "GHOSTBUSTERS", "HALLOWEEN", "PSYCHO"},
    {"BASKETBALL", "FOOTBALL", "BASEBALL", "SOCCER", "TENNIS", "CRICKET", "VOLLEYBALL", "RUGBY", "GOLF", "HOCKEY", "BADMINTON", "SWIMMING", "BOXING", "WRESTLING", "CYCLING", "ARCHERY", "FENCING", "GYMNASTICS", "MARATHON", "SURFING"},
    {"PIZZA", "BURGER", "SUSHI", "PASTA", "TACO", "STEAK", "SALAD", "SOUP", "SANDWICH", "NOODLES", "PANCAKE", "WAFFLE", "CHICKEN", "CHEESE", "BREAD", "RICE", "POTATO", "APPLE", "BANANA", "CHOCOLATE"},
    {"GUITAR", "PIANO", "DRUMS", "VIOLIN", "FLUTE", "TRUMPET", "SAXOPHONE", "CELLO", "BASS", "VOCALS", "MELODY", "RHYTHM", "CHORD", "ORCHESTRA", "SYMPHONY", "JAZZ", "ROCK", "BLUES", "CLASSICAL", "POP"},
    {"ROME", "GREECE", "EGYPT", "EMPIRE", "REVOLUTION", "WAR", "KING", "QUEEN", "KNIGHT", "CASTLE", "PYRAMID", "PHARAOH", "GLADIATOR", "SAMURAI", "NINJA", "VIKING", "PIRATE", "COLONY", "TREATY", "DISCOVERY"},
    {"ZEUS", "HERCULES", "ODYSSEY", "VALHALLA", "PEGASUS", "MINOTAUR", "POSEIDON", "HADES", "ATHENA", "THOR", "LOKI", "ANUBIS", "PHOENIX", "CENTAUR", "CHIMERA", "CERBERUS", "KRAKEN", "TITAN", "MEDUSA", "SPHINX"},
    {"SUPERNOVA", "NEBULA", "ASTEROID", "EXOPLANET", "BLACKHOLE", "GALAXY", "METEOR", "SATELLITE", "ECLIPSE", "COSMOS", "TACHYON", "HUBBLE", "QUASAR", "PULSAR", "LIGHTYEAR", "ORBIT", "COMET", "STARDUST", "PARSEC", "GRAVITY"},
    {"SHAKESPEARE", "SHERLOCK", "DRACULA", "ORWELL", "HOMER", "TOLKIEN", "DICKENS", "NOVEL", "FICTION", "POETRY", "MONTAGUE", "GATSBY", "MOBYCOLD", "ODYSSEY", "INFERNO", "UTOPIA", "HAMLET", "MACBETH", "BEOWULF", "ILIAD"},
    {"AMAZON", "EVEREST", "SAHARA", "HIMALAYAS", "PACIFIC", "ATLANTIC", "ANDES", "ARCHIPELAGO", "CANYON", "PENINSULA", "CONTINENT", "EQUATOR", "GLACIER", "VOLCANO", "REEF", "TUNDRA", "SAVANNA", "ISTHMUS", "ISLAND", "PLATEAU"},
    {"NETFLIX", "TIKTOK", "PODCAST", "GAMING", "STREAMING", "ANIME", "MARVEL", "DISNEY", "ESPORTS", "MEME", "COSPLAY", "SPOTIFY", "FANDOM", "BINGE", "TRENDING", "VLOGGER", "INFLUENCER", "REELS", "YOUTUBE", "AVATAR"}
};
const int NUM_WORDS_PER_CAT = 20;

// Game state variables
int current_category = 0;
int game_mode = 0; // 0: Freeplay, 1: Campaign (15 stages), 2: Time Attack Blitz
int campaign_level = 1;
int max_errors = 6;
int bombs = 1;
int shields = 1;
int blitz_time = 60;
int blitz_words = 0;

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
int blitz_timer_counter = 0;

void PlaySoundEffect(int type) {
    if (is_muted) return;
    if (type == 1) { // valid
        Beep(800, 80);
    } else if (type == 2) { // invalid
        Beep(200, 120);
    } else if (type == 3) { // win
        Beep(400, 80);
        Beep(600, 80);
        Beep(800, 150);
    } else if (type == 4) { // lose
        Beep(300, 150);
        Beep(150, 250);
    } else if (type == 5) { // shield absorbed
        Beep(1000, 100);
        Beep(1200, 150);
    } else if (type == 6) { // bomb used
        Beep(150, 180);
    }
}

typedef struct {
    int wins;
    int losses;
    int streak;
    int best;
    int blitz_best;
} Stats;

Stats stats = {0, 0, 0, 0, 0};

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
    int game_mode;
    int campaign_level;
    int max_errors;
    int bombs;
    int shields;
    int blitz_time;
    int blitz_words;
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
    st.game_mode = game_mode;
    st.campaign_level = campaign_level;
    st.max_errors = max_errors;
    st.bombs = bombs;
    st.shields = shields;
    st.blitz_time = blitz_time;
    st.blitz_words = blitz_words;
    
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
            game_mode = st.game_mode;
            campaign_level = st.campaign_level;
            max_errors = st.max_errors;
            bombs = st.bombs;
            shields = st.shields;
            blitz_time = st.blitz_time;
            blitz_words = st.blitz_words;
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

void SelectNewWord() {
    if (current_category == TOTAL_CAT_COUNT - 1) {
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
        int cat_idx = current_category % NUM_CATEGORIES;
        int w_idx = CustomRand() % NUM_WORDS_PER_CAT;
        int i = 0;
        while(CAT_WORDS[cat_idx][w_idx][i]) {
            target_word[i] = CAT_WORDS[cat_idx][w_idx][i];
            i++;
        }
        target_word[i] = '\0';
    }
    for(int i=0; i<26; i++) guessed[i] = 0;
    errors = 0;
    hint_used = 0;
}

void InitGame() {
    if (!initialized) {
        LoadStats();
        CustomSrand(GetTickCount());
        initialized = 1;
    }
    
    if (game_mode == 1) { // Campaign (15 Stages)
        current_category = (campaign_level - 1) % NUM_CATEGORIES;
        if (campaign_level <= 3) max_errors = 7;
        else if (campaign_level <= 7) max_errors = 6;
        else if (campaign_level <= 11) max_errors = 5;
        else max_errors = 4;
        
        if (campaign_level == 1) {
            bombs = 3;
            shields = 2;
        }
    } else if (game_mode == 2) { // Time Attack Blitz
        max_errors = 6;
        if (blitz_time <= 0 || game_over) {
            blitz_time = 60;
            blitz_words = 0;
            bombs = 2;
            shields = 1;
        }
    } else { // Freeplay
        max_errors = 6;
        bombs = 1;
        shields = 1;
    }
    
    SelectNewWord();
    game_over = 0;
    won = 0;
    shake_frames = 0;
    win_pulse_phase = 0;
    blitz_timer_counter = 0;
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
        if (shields > 0) {
            shields--;
            PlaySoundEffect(5); // Shield absorbed mistarget
        } else {
            errors++;
            shake_frames = 15;
            if (errors >= max_errors) {
                game_over = 1;
                won = 0;
                PlaySoundEffect(4); // lose
                stats.losses++;
                stats.streak = 0;
                if (game_mode == 1) {
                    game_mode = 0;
                    campaign_level = 1;
                }
                SaveStats();
            } else {
                PlaySoundEffect(2); // invalid guess
            }
        }
    } else {
        if (game_mode == 2) {
            blitz_time += 2; // +2 seconds for correct letter
        }
        all_guessed = 1;
        for (int i = 0; target_word[i] != '\0'; i++) {
            if (!guessed[target_word[i] - 'A']) {
                all_guessed = 0;
                break;
            }
        }
        if (all_guessed) {
            if (game_mode == 2) {
                blitz_words++;
                blitz_time += 15; // +15 seconds for completing a word
                if (blitz_words > stats.blitz_best) {
                    stats.blitz_best = blitz_words;
                }
                PlaySoundEffect(3);
                stats.wins++;
                stats.streak++;
                if (stats.streak > stats.best) stats.best = stats.streak;
                SaveStats();
                SelectNewWord(); // Immediately advance to next word in Blitz mode!
            } else {
                game_over = 1;
                won = 1;
                PlaySoundEffect(3); // win
                stats.wins++;
                stats.streak++;
                if (stats.streak > stats.best) stats.best = stats.streak;
                SaveStats();
            }
        } else {
            PlaySoundEffect(1); // valid
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            hCustomEdit = CreateWindowEx(0, "EDIT", "APPLE, BANANA", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_LEFT, 100, 115, 300, 22, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);
            if (current_category == TOTAL_CAT_COUNT - 1) ShowWindow(hCustomEdit, SW_SHOW);
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
                if (game_mode == 2 && !game_over) {
                    blitz_timer_counter++;
                    if (blitz_timer_counter >= 33) { // approx 1 sec
                        blitz_timer_counter = 0;
                        if (blitz_time > 0) {
                            blitz_time--;
                            needs_redraw = 1;
                            if (blitz_time <= 0) {
                                game_over = 1;
                                won = 0;
                                PlaySoundEffect(4);
                                stats.losses++;
                                stats.streak = 0;
                                if (blitz_words > stats.blitz_best) stats.blitz_best = blitz_words;
                                SaveStats();
                            }
                        }
                    }
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
            
            // Bomb button (20..75, y: 530..560)
            if (x >= 15 && x <= 75 && y >= 530 && y <= 560) {
                if (bombs > 0 && !game_over) {
                    bombs--;
                    PlaySoundEffect(6);
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

            // Hint button (85..145, y: 530..560)
            if (x >= 85 && x <= 145 && y >= 530 && y <= 560) {
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

            // Shield button (155..215, y: 530..560)
            if (x >= 155 && x <= 215 && y >= 530 && y <= 560) {
                // Info message or extra shield call
                MessageBoxA(hwnd, "Shield absorbs 1 wrong letter guess automatically!", "Shield Info", MB_OK | MB_ICONINFORMATION);
                break;
            }

            // Campaign button (225..315, y: 530..560)
            if (x >= 225 && x <= 315 && y >= 530 && y <= 560) {
                SetFocus(hwnd);
                if (game_over && won && game_mode == 1 && campaign_level < 15) {
                    campaign_level++;
                } else {
                    game_mode = 1;
                    campaign_level = 1;
                }
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Time Attack Blitz button (325..405, y: 530..560)
            if (x >= 325 && x <= 405 && y >= 530 && y <= 560) {
                SetFocus(hwnd);
                game_mode = 2;
                blitz_time = 60;
                blitz_words = 0;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Freeplay button (415..485, y: 530..560)
            if (x >= 415 && x <= 485 && y >= 530 && y <= 560) {
                SetFocus(hwnd);
                game_mode = 0;
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            
            // Save button (35..105, y: 570..600)
            if (x >= 35 && x <= 105 && y >= 570 && y <= 600) {
                SaveGame(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            
            // Load button (115..185, y: 570..600)
            if (x >= 115 && x <= 185 && y >= 570 && y <= 600) {
                LoadGame(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Help button (195..265, y: 570..600)
            if (x >= 195 && x <= 265 && y >= 570 && y <= 600) {
                MessageBoxA(hwnd,
                    "KHangman - Rules & Game Modes\n\n"
                    "Rules: Guess the word letter by letter.\n"
                    "Modes:\n"
                    " • Freeplay: Unlimited time classic Hangman.\n"
                    " • Campaign: 15 Stages with increasing difficulty.\n"
                    " • Time Attack Blitz: 60s timer. Solve as many words as you can!\n\n"
                    "Power-Ups:\n"
                    " • Bomb: Removes up to 3 wrong letters.\n"
                    " • Hint: Reveals 1 correct letter.\n"
                    " • Shield: Absorbs 1 wrong guess penalty.",
                    "Help / How to Play", MB_OK | MB_ICONINFORMATION);
                break;
            }

            // Mute button (275..345, y: 570..600)
            if (x >= 275 && x <= 345 && y >= 570 && y <= 600) {
                is_muted = !is_muted;
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            // Reset Stats button (355..455, y: 570..600)
            if (x >= 355 && x <= 455 && y >= 570 && y <= 600) {
                if (MessageBoxA(hwnd, "Reset all lifetime statistics?", "Reset Stats", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    stats.wins = 0;
                    stats.losses = 0;
                    stats.streak = 0;
                    stats.best = 0;
                    stats.blitz_best = 0;
                    SaveStats();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                break;
            }

            // Check categories (3 rows of 5)
            int cy = 35;
            for (int r = 0; r < 3; r++) {
                int cx = 15;
                for (int c = 0; c < 5; c++) {
                    int cat_idx = r * 5 + c;
                    if (cat_idx < TOTAL_CAT_COUNT) {
                        if (x >= cx && x <= cx + 90 && y >= cy && y <= cy + 22) {
                            if (current_category != cat_idx) {
                                current_category = cat_idx;
                                game_mode = 0; // switch to freeplay on manual category pick
                                if (current_category == TOTAL_CAT_COUNT - 1) ShowWindow(hCustomEdit, SW_SHOW);
                                else ShowWindow(hCustomEdit, SW_HIDE);
                                SetFocus(hwnd);
                                InitGame();
                                InvalidateRect(hwnd, NULL, TRUE);
                            }
                            break;
                        }
                    }
                    cx += 95;
                }
                cy += 25;
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
            HFONT hFontMono = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, FIXED_PITCH, "Consolas");
            HFONT hFontSmall = CreateFontA(13, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            
            // Title
            SetTextColor(memDC, RGB(79, 172, 254));
            SelectObject(memDC, hFontMain);
            TextOutA(memDC, 190, 5, "KHangman", 8);

            // Categories rendering (3 rows of 5)
            SelectObject(memDC, hFontSmall);
            int cy = 32;
            for (int r = 0; r < 3; r++) {
                int cx = 15;
                for (int c = 0; c < 5; c++) {
                    int cat_idx = r * 5 + c;
                    if (cat_idx < TOTAL_CAT_COUNT) {
                        RECT cRect = {cx, cy, cx + 90, cy + 22};
                        HBRUSH cBg = CreateSolidBrush((cat_idx == current_category && game_mode == 0) ? RGB(0, 122, 204) : RGB(44, 44, 44));
                        FillRect(memDC, &cRect, cBg);
                        DeleteObject(cBg);
                        SetTextColor(memDC, RGB(255, 255, 255));
                        DrawTextA(memDC, CAT_NAMES[cat_idx], -1, &cRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                    cx += 95;
                }
                cy += 25;
            }

            // Drawing gallows
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
                // Extra error
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

            // Message & Info Display
            char msgBuf[128] = "Guess a letter to start";
            COLORREF msgColor = RGB(224, 224, 224);
            if (game_mode == 1) {
                wsprintfA(msgBuf, "Campaign Stage %d / 15", campaign_level);
                msgColor = RGB(255, 193, 7);
            } else if (game_mode == 2) {
                wsprintfA(msgBuf, "⚡ Blitz Time: %ds | Words Solved: %d", blitz_time, blitz_words);
                msgColor = (blitz_time <= 10) ? RGB(244, 67, 54) : RGB(79, 172, 254);
            }
            if (game_over) {
                if (won) {
                    if (game_mode == 1 && campaign_level == 15) strcpy(msgBuf, "Campaign Complete!");
                    else strcpy(msgBuf, "You Win!");
                    msgColor = RGB(76, 175, 80);
                } else {
                    wsprintfA(msgBuf, "Game Over! Word: %s", target_word);
                    msgColor = RGB(244, 67, 54);
                }
            }
            SetTextColor(memDC, msgColor);
            GetTextExtentPoint32A(memDC, msgBuf, lstrlenA(msgBuf), &tSize);
            TextOutA(memDC, (W - tSize.cx) / 2, 330, msgBuf, lstrlenA(msgBuf));

            // On-screen Keyboard
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

            SelectObject(memDC, hFontSmall);

            // Row 1 Power-ups & Modes (y: 530..560)
            // Bomb button
            RECT bombRect = {15, 530, 75, 560};
            HBRUSH bombBg = CreateSolidBrush((bombs <= 0 || game_over) ? RGB(68, 68, 68) : RGB(244, 67, 54));
            FillRect(memDC, &bombRect, bombBg);
            DeleteObject(bombBg);
            SetTextColor(memDC, (bombs <= 0 || game_over) ? RGB(119, 119, 119) : RGB(255, 255, 255));
            char bombTxt[32];
            wsprintfA(bombTxt, "Bomb(%d)", bombs);
            DrawTextA(memDC, bombTxt, -1, &bombRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Hint button
            RECT hintRect = {85, 530, 145, 560};
            HBRUSH hintBg = CreateSolidBrush((hint_used || game_over) ? RGB(68, 68, 68) : RGB(255, 152, 0));
            FillRect(memDC, &hintRect, hintBg);
            DeleteObject(hintBg);
            SetTextColor(memDC, (hint_used || game_over) ? RGB(119, 119, 119) : RGB(255, 255, 255));
            DrawTextA(memDC, "Hint", -1, &hintRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Shield button
            RECT shieldRect = {155, 530, 215, 560};
            HBRUSH shieldBg = CreateSolidBrush((shields <= 0 || game_over) ? RGB(68, 68, 68) : RGB(156, 39, 176));
            FillRect(memDC, &shieldRect, shieldBg);
            DeleteObject(shieldBg);
            SetTextColor(memDC, (shields <= 0 || game_over) ? RGB(119, 119, 119) : RGB(255, 255, 255));
            char shieldTxt[32];
            wsprintfA(shieldTxt, "Shield(%d)", shields);
            DrawTextA(memDC, shieldTxt, -1, &shieldRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Campaign button
            RECT campRect = {225, 530, 315, 560};
            HBRUSH campBg = CreateSolidBrush(game_mode == 1 ? RGB(0, 150, 136) : RGB(80, 80, 80));
            FillRect(memDC, &campRect, campBg);
            DeleteObject(campBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            if (game_over && won && game_mode == 1 && campaign_level < 15) {
                DrawTextA(memDC, "Next Stage", -1, &campRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            } else {
                DrawTextA(memDC, "Campaign", -1, &campRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            // Time Attack Blitz button
            RECT blitzRect = {325, 530, 405, 560};
            HBRUSH blitzBg = CreateSolidBrush(game_mode == 2 ? RGB(233, 30, 99) : RGB(80, 80, 80));
            FillRect(memDC, &blitzRect, blitzBg);
            DeleteObject(blitzBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Blitz Mode", -1, &blitzRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Freeplay button
            RECT resRect = {415, 530, 485, 560};
            HBRUSH resBg = CreateSolidBrush(game_mode == 0 ? RGB(0, 122, 204) : RGB(80, 80, 80));
            FillRect(memDC, &resRect, resBg);
            DeleteObject(resBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Freeplay", -1, &resRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            // Row 2 System & Utility Buttons (y: 570..600)
            RECT saveRect = {35, 570, 105, 600};
            HBRUSH saveBg = CreateSolidBrush(RGB(76, 175, 80));
            FillRect(memDC, &saveRect, saveBg);
            DeleteObject(saveBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Save", -1, &saveRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT loadRect = {115, 570, 185, 600};
            HBRUSH loadBg = CreateSolidBrush(RGB(156, 39, 176));
            FillRect(memDC, &loadRect, loadBg);
            DeleteObject(loadBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Load", -1, &loadRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT helpRect = {195, 570, 265, 600};
            HBRUSH helpBg = CreateSolidBrush(RGB(33, 150, 243));
            FillRect(memDC, &helpRect, helpBg);
            DeleteObject(helpBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Help", -1, &helpRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT muteRect = {275, 570, 345, 600};
            HBRUSH muteBg = CreateSolidBrush(RGB(85, 85, 85));
            FillRect(memDC, &muteRect, muteBg);
            DeleteObject(muteBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, is_muted ? "Muted" : "Sound", -1, &muteRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT rstRect = {355, 570, 455, 600};
            HBRUSH rstBg = CreateSolidBrush(RGB(120, 120, 120));
            FillRect(memDC, &rstRect, rstBg);
            DeleteObject(rstBg);
            SetTextColor(memDC, RGB(255, 255, 255));
            DrawTextA(memDC, "Reset Stats", -1, &rstRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            DeleteObject(hFontSmall);

            // Stats footer
            char statText[160];
            wsprintfA(statText, "Wins: %d | Loss: %d | Streak: %d (Best: %d) | Blitz Best: %d", stats.wins, stats.losses, stats.streak, stats.best, stats.blitz_best);
            SetTextColor(memDC, RGB(79, 172, 254));
            SelectObject(memDC, hFontMono);
            GetTextExtentPoint32A(memDC, statText, lstrlenA(statText), &tSize);
            TextOutA(memDC, (W - tSize.cx) / 2, 618, statText, lstrlenA(statText));

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
