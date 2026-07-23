#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define BTN_GREEN  0
#define BTN_RED    1
#define BTN_YELLOW 2
#define BTN_BLUE   3
#define BTN_PURPLE 4
#define BTN_CYAN   5

#define TIMER_SEQUENCE 1
#define TIMER_FLASH    2
#define TIMER_GAME_OVER 3

#define MODE_CLASSIC 0
#define MODE_REVERSE 1
#define MODE_SPEED   2
#define MODE_ENDLESS 3
#define MODE_CAMPAIGN 4
#define MODE_CHAOS   5

#define NUM_MODES 6

typedef struct {
    int target_len;
    int num_colors;
    int speed_ms;
    int modifier; // 0: Normal, 1: Reverse, 2: Chaos
} CampaignStage;

CampaignStage campaign_stages[15] = {
    {3, 4, 400, 0}, // Stage 1
    {4, 4, 380, 0}, // Stage 2
    {5, 4, 350, 0}, // Stage 3
    {5, 5, 350, 0}, // Stage 4
    {6, 5, 250, 0}, // Stage 5 (Speedy)
    {6, 5, 350, 1}, // Stage 6 (Reverse)
    {7, 5, 320, 0}, // Stage 7
    {8, 6, 300, 0}, // Stage 8
    {8, 6, 280, 2}, // Stage 9 (Chaos)
    {9, 6, 220, 0}, // Stage 10 (Speedy)
    {10, 6, 280, 0}, // Stage 11
    {10, 6, 300, 1}, // Stage 12 (Reverse)
    {11, 6, 250, 2}, // Stage 13 (Chaos)
    {12, 6, 220, 1}, // Stage 14 (Reverse Speed)
    {14, 6, 180, 2}  // Stage 15 (Final Chaos Boss)
};

int btn_freqs[6] = {415, 329, 261, 196, 493, 146};

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
HWND hwndSaveBtn;
HWND hwndLoadBtn;
HWND hwndResetBtn;
HWND hwndHelpBtn;
HWND hwndHintBtn;
HWND hwndSlowBtn;
HWND hwndShieldBtn;

int current_mode = MODE_CLASSIC;

int sequence[1000];
int sequence_length = 0;
int player_step = 0;
int is_playing_sequence = 0;
int current_flash_index = 0;
int flash_btn = -1;
int game_over_flash = 0;
int game_over_flash_count = 0;

int hints_remaining = 3;
int slowmo_remaining = 2;
int shields_remaining = 1;

int is_slowmo_active = 0;
int current_stage = 1;

RECT btn_rects[6];
COLORREF btn_colors[6] = {
    RGB(0, 170, 0),    // Green
    RGB(170, 0, 0),    // Red
    RGB(170, 170, 0),  // Yellow
    RGB(0, 0, 170),    // Blue
    RGB(170, 0, 170),  // Purple
    RGB(0, 170, 170)   // Cyan
};
COLORREF flash_colors[6] = {
    RGB(0, 255, 0),
    RGB(255, 0, 0),
    RGB(255, 255, 0),
    RGB(0, 0, 255),
    RGB(255, 0, 255),
    RGB(0, 255, 255)
};

char status_text[128] = "Press Space to Start";
int score = 0;
int high_scores[NUM_MODES] = {0, 0, 0, 0, 0, 0};
int stat_games_played = 0;
int stat_longest_streak = 0;
int stat_best_time = 0;
time_t start_time = 0;

void LoadHighScores() {
    high_scores[MODE_CLASSIC]  = GetPrivateProfileInt("HighScores", "Classic", 0, ".\\ksimon.ini");
    high_scores[MODE_REVERSE]  = GetPrivateProfileInt("HighScores", "Reverse", 0, ".\\ksimon.ini");
    high_scores[MODE_SPEED]    = GetPrivateProfileInt("HighScores", "Speed", 0, ".\\ksimon.ini");
    high_scores[MODE_ENDLESS]  = GetPrivateProfileInt("HighScores", "Endless", 0, ".\\ksimon.ini");
    high_scores[MODE_CAMPAIGN] = GetPrivateProfileInt("HighScores", "Campaign", 0, ".\\ksimon.ini");
    high_scores[MODE_CHAOS]    = GetPrivateProfileInt("HighScores", "Chaos", 0, ".\\ksimon.ini");
    
    stat_games_played   = GetPrivateProfileInt("Stats", "GamesPlayed", 0, ".\\ksimon.ini");
    stat_longest_streak = GetPrivateProfileInt("Stats", "LongestStreak", 0, ".\\ksimon.ini");
    stat_best_time      = GetPrivateProfileInt("Stats", "BestTime", 0, ".\\ksimon.ini");
}

void SaveHighScore(int mode, int s) {
    char str[32];
    sprintf(str, "%d", s);
    if (mode == MODE_CLASSIC)       WritePrivateProfileString("HighScores", "Classic", str, ".\\ksimon.ini");
    else if (mode == MODE_REVERSE)  WritePrivateProfileString("HighScores", "Reverse", str, ".\\ksimon.ini");
    else if (mode == MODE_SPEED)    WritePrivateProfileString("HighScores", "Speed", str, ".\\ksimon.ini");
    else if (mode == MODE_ENDLESS)  WritePrivateProfileString("HighScores", "Endless", str, ".\\ksimon.ini");
    else if (mode == MODE_CAMPAIGN) WritePrivateProfileString("HighScores", "Campaign", str, ".\\ksimon.ini");
    else if (mode == MODE_CHAOS)    WritePrivateProfileString("HighScores", "Chaos", str, ".\\ksimon.ini");
}

void SaveStats() {
    char str[32];
    sprintf(str, "%d", stat_games_played);
    WritePrivateProfileString("Stats", "GamesPlayed", str, ".\\ksimon.ini");
    sprintf(str, "%d", stat_longest_streak);
    WritePrivateProfileString("Stats", "LongestStreak", str, ".\\ksimon.ini");
    sprintf(str, "%d", stat_best_time);
    WritePrivateProfileString("Stats", "BestTime", str, ".\\ksimon.ini");
}

void SaveGameState() {
    if (sequence_length == 0 || is_playing_sequence) return;
    char str[2048] = {0};
    char temp[16];
    for(int i = 0; i < sequence_length; i++) {
        sprintf(temp, "%d,", sequence[i]);
        strcat(str, temp);
    }
    WritePrivateProfileString("GameState", "Sequence", str, ".\\ksimon.ini");
    sprintf(temp, "%d", current_mode);
    WritePrivateProfileString("GameState", "Mode", temp, ".\\ksimon.ini");
    sprintf(temp, "%d", (int)(time(NULL) - start_time));
    WritePrivateProfileString("GameState", "ElapsedTime", temp, ".\\ksimon.ini");
    sprintf(temp, "%d", sequence_length);
    WritePrivateProfileString("GameState", "Length", temp, ".\\ksimon.ini");
    sprintf(temp, "%d", hints_remaining);
    WritePrivateProfileString("GameState", "Hints", temp, ".\\ksimon.ini");
    sprintf(temp, "%d", slowmo_remaining);
    WritePrivateProfileString("GameState", "Slowmo", temp, ".\\ksimon.ini");
    sprintf(temp, "%d", shields_remaining);
    WritePrivateProfileString("GameState", "Shields", temp, ".\\ksimon.ini");
    sprintf(temp, "%d", current_stage);
    WritePrivateProfileString("GameState", "Stage", temp, ".\\ksimon.ini");
    strcpy(status_text, "Game Saved!");
    InvalidateRect(hwndMain, NULL, TRUE);
}

void LoadGameState() {
    int len = GetPrivateProfileInt("GameState", "Length", 0, ".\\ksimon.ini");
    if (len == 0) {
        strcpy(status_text, "No saved game found.");
        InvalidateRect(hwndMain, NULL, TRUE);
        return;
    }
    sequence_length = len;
    current_mode = GetPrivateProfileInt("GameState", "Mode", MODE_CLASSIC, ".\\ksimon.ini");
    int elapsed = GetPrivateProfileInt("GameState", "ElapsedTime", 0, ".\\ksimon.ini");
    start_time = time(NULL) - elapsed;
    hints_remaining = GetPrivateProfileInt("GameState", "Hints", 3, ".\\ksimon.ini");
    slowmo_remaining = GetPrivateProfileInt("GameState", "Slowmo", 2, ".\\ksimon.ini");
    shields_remaining = GetPrivateProfileInt("GameState", "Shields", 1, ".\\ksimon.ini");
    current_stage = GetPrivateProfileInt("GameState", "Stage", 1, ".\\ksimon.ini");
    
    char str[2048] = {0};
    GetPrivateProfileString("GameState", "Sequence", "", str, sizeof(str), ".\\ksimon.ini");
    
    char* token = strtok(str, ",");
    int i = 0;
    while (token != NULL && i < sequence_length) {
        sequence[i++] = atoi(token);
        token = strtok(NULL, ",");
    }
    
    SendMessage(hwndModeBox, CB_SETCURSEL, current_mode, 0);
    EnableWindow(hwndModeBox, FALSE);
    EnableWindow(hwndSaveBtn, FALSE);
    
    score = sequence_length - 1;
    player_step = 0;
    is_playing_sequence = 1;
    current_flash_index = (current_mode == MODE_ENDLESS && sequence_length > 0) ? sequence_length - 1 : 0;
    strcpy(status_text, "Game Loaded! Watch...");
    InvalidateRect(hwndMain, NULL, TRUE);
    SetTimer(hwndMain, TIMER_SEQUENCE, 1000, NULL);
}

void ResetStats() {
    stat_games_played = 0;
    stat_longest_streak = 0;
    stat_best_time = 0;
    SaveStats();
    strcpy(status_text, "Stats Reset!");
    InvalidateRect(hwndMain, NULL, TRUE);
}

void DrawBoard(HDC hdc) {
    for (int i = 0; i < 6; i++) {
        RECT r = btn_rects[i];
        if (flash_btn == i) {
            InflateRect(&r, 4, 4);
        }
        HBRUSH brush = CreateSolidBrush(flash_btn == i ? flash_colors[i] : btn_colors[i]);
        FillRect(hdc, &r, brush);
        DeleteObject(brush);
        
        if (flash_btn == i) {
            HPEN pen = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
            HGDIOBJ oldPen = SelectObject(hdc, pen);
            HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, r.left, r.top, r.right, r.bottom);
            SelectObject(hdc, oldPen);
            SelectObject(hdc, oldBrush);
            DeleteObject(pen);
        }
    }

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(224, 224, 224));
    TextOutA(hdc, 10, 10, status_text, strlen(status_text));
    
    char score_text[64];
    sprintf(score_text, "Score: %d | High Score: %d", score, high_scores[current_mode]);
    TextOutA(hdc, 10, 30, score_text, strlen(score_text));

    char stats_text[128];
    sprintf(stats_text, "Games: %d | Streak: %d | Time: %ds", stat_games_played, stat_longest_streak, stat_best_time);
    TextOutA(hdc, 10, 50, stats_text, strlen(stats_text));

    char pwr_text[128];
    sprintf(pwr_text, "Hints (H): %d | Slow (F): %d | Shields (J): %d", hints_remaining, slowmo_remaining, shields_remaining);
    TextOutA(hdc, 10, 70, pwr_text, strlen(pwr_text));

    if (current_mode == MODE_CAMPAIGN) {
        char stage_text[128];
        const char* mod_str = "Normal";
        if (campaign_stages[current_stage-1].modifier == 1) mod_str = "REVERSE";
        else if (campaign_stages[current_stage-1].modifier == 2) mod_str = "CHAOS";
        sprintf(stage_text, "Campaign Stage: %d/15 [%s] (Target Len: %d)", 
                current_stage, mod_str, campaign_stages[current_stage-1].target_len);
        TextOutA(hdc, 10, 90, stage_text, strlen(stage_text));
    } else if (current_mode == MODE_CHAOS) {
        TextOutA(hdc, 10, 90, "Mode: CHAOS - Unpredictable speeds & pitches!", 45);
    } else if (current_mode == MODE_REVERSE) {
        TextOutA(hdc, 10, 90, "Mode: REVERSE - Repeat sequence backwards!", 42);
    } else {
        TextOutA(hdc, 10, 90, "Controls: Q,W,E / A,S,D or 1-6 keys", 35);
    }
}

void StartGame() {
    current_mode = SendMessage(hwndModeBox, CB_GETCURSEL, 0, 0);
    EnableWindow(hwndModeBox, FALSE);
    EnableWindow(hwndSaveBtn, FALSE);
    sequence_length = 0;
    score = 0;
    hints_remaining = 3;
    slowmo_remaining = 2;
    shields_remaining = 1;
    current_stage = 1;
    is_slowmo_active = 0;
    is_playing_sequence = 1;
    start_time = time(NULL);
    strcpy(status_text, "Get Ready...");
    InvalidateRect(hwndMain, NULL, TRUE);
    SetTimer(hwndMain, TIMER_SEQUENCE, 1000, NULL);
}

void NextRound() {
    player_step = 0;
    int num_colors = 6;
    if (current_mode == MODE_CAMPAIGN) {
        num_colors = campaign_stages[current_stage - 1].num_colors;
    }
    sequence[sequence_length++] = rand() % num_colors;
    score = sequence_length - 1;
    is_playing_sequence = 1;
    current_flash_index = (current_mode == MODE_ENDLESS && sequence_length > 0) ? sequence_length - 1 : 0;
    strcpy(status_text, is_slowmo_active ? "Watch (Slow-Mo)..." : "Watch...");
    InvalidateRect(hwndMain, NULL, TRUE);
    SetTimer(hwndMain, TIMER_SEQUENCE, 500, NULL);
}

void UseHint() {
    if (!is_playing_sequence && sequence_length > 0 && hints_remaining > 0) {
        hints_remaining--;
        is_playing_sequence = 1;
        current_flash_index = (current_mode == MODE_ENDLESS && sequence_length > 0) ? sequence_length - 1 : 0;
        strcpy(status_text, "Hint: Watch...");
        InvalidateRect(hwndMain, NULL, TRUE);
        SetTimer(hwndMain, TIMER_SEQUENCE, 800, NULL);
    }
}

void UseSlowmo() {
    if (!is_playing_sequence && sequence_length > 0 && slowmo_remaining > 0) {
        slowmo_remaining--;
        is_slowmo_active = 1;
        is_playing_sequence = 1;
        current_flash_index = (current_mode == MODE_ENDLESS && sequence_length > 0) ? sequence_length - 1 : 0;
        strcpy(status_text, "Slow-Mo Active! Watch...");
        InvalidateRect(hwndMain, NULL, TRUE);
        SetTimer(hwndMain, TIMER_SEQUENCE, 800, NULL);
    }
}

void UseShield() {
    if (shields_remaining > 0) {
        sprintf(status_text, "Shield Active! (%d Shields available)", shields_remaining);
        InvalidateRect(hwndMain, NULL, TRUE);
    }
}

void HandleClick(int btn_id) {
    if (is_playing_sequence || sequence_length == 0) return;

    flash_btn = btn_id;
    EnableWindow(hwndSaveBtn, FALSE);
    InvalidateRect(hwndMain, NULL, TRUE);
    SetTimer(hwndMain, TIMER_FLASH, (current_mode == MODE_SPEED) ? 150 : 300, NULL);

    int is_reverse = 0;
    if (current_mode == MODE_REVERSE) {
        is_reverse = 1;
    } else if (current_mode == MODE_CAMPAIGN && campaign_stages[current_stage-1].modifier == 1) {
        is_reverse = 1;
    }

    int expected_index;
    if (is_reverse) {
        expected_index = sequence[sequence_length - 1 - player_step];
    } else {
        expected_index = sequence[player_step];
    }

    if (btn_id != expected_index) {
        if (shields_remaining > 0) {
            shields_remaining--;
            PlaySoundAsync(750, 300);
            player_step = 0;
            sprintf(status_text, "Shield Absorbed Error! (%d Shields left)", shields_remaining);
            InvalidateRect(hwndMain, NULL, TRUE);
            return;
        }

        PlaySoundAsync(100, 800);
        game_over_flash_count = 0;
        game_over_flash = 1;
        SetTimer(hwndMain, TIMER_GAME_OVER, 100, NULL);
        if (score > high_scores[current_mode]) {
            high_scores[current_mode] = score;
            SaveHighScore(current_mode, score);
            sprintf(status_text, "Game Over! Score: %d (New High Score!)", score);
        } else {
            sprintf(status_text, "Game Over! Score: %d (Space to restart)", score);
        }
        
        stat_games_played++;
        if (score > stat_longest_streak) stat_longest_streak = score;
        int elapsed = (int)(time(NULL) - start_time);
        if (elapsed > stat_best_time) stat_best_time = elapsed;
        SaveStats();
        sequence_length = 0;
        EnableWindow(hwndModeBox, TRUE);
        EnableWindow(hwndSaveBtn, FALSE);
        InvalidateRect(hwndMain, NULL, TRUE);
        return;
    }

    PlaySoundAsync(btn_freqs[btn_id], 200);

    player_step++;
    if (player_step == sequence_length) {
        if (current_mode == MODE_CAMPAIGN) {
            int target = campaign_stages[current_stage - 1].target_len;
            if (sequence_length >= target) {
                current_stage++;
                if (current_stage > 15) {
                    strcpy(status_text, "Campaign Complete! YOU WIN!");
                    is_playing_sequence = 1;
                    score += 100; // Bonus for winning campaign
                    if (score > high_scores[current_mode]) {
                        high_scores[current_mode] = score;
                        SaveHighScore(current_mode, score);
                    }
                    stat_games_played++;
                    if (score > stat_longest_streak) stat_longest_streak = score;
                    int elapsed = (int)(time(NULL) - start_time);
                    if (elapsed > stat_best_time) stat_best_time = elapsed;
                    SaveStats();
                    sequence_length = 0;
                    EnableWindow(hwndModeBox, TRUE);
                    InvalidateRect(hwndMain, NULL, TRUE);
                    return;
                } else {
                    sprintf(status_text, "Stage %d Cleared! +1 Powerups", current_stage - 1);
                    hints_remaining++;
                    slowmo_remaining++;
                    shields_remaining++;
                    sequence_length = 0;
                }
            } else {
                strcpy(status_text, "Good! Get ready...");
            }
        } else {
            strcpy(status_text, "Good! Get ready...");
        }
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
                10, 130, 130, 150, hwnd, (HMENU)1001, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Classic Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Reverse Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Speed Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Endless Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Campaign Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Chaos Mode");
            SendMessage(hwndModeBox, CB_SETCURSEL, 0, 0);

            hwndSaveBtn   = CreateWindowEx(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 145, 130, 50, 25, hwnd, (HMENU)1002, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndLoadBtn   = CreateWindowEx(0, "BUTTON", "Load", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 200, 130, 50, 25, hwnd, (HMENU)1003, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndResetBtn  = CreateWindowEx(0, "BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 255, 130, 50, 25, hwnd, (HMENU)1004, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndHelpBtn   = CreateWindowEx(0, "BUTTON", "Help", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 310, 130, 50, 25, hwnd, (HMENU)1005, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hwndHintBtn   = CreateWindowEx(0, "BUTTON", "Hint (H)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 160, 75, 25, hwnd, (HMENU)1006, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndSlowBtn   = CreateWindowEx(0, "BUTTON", "Slow (F)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 90, 160, 75, 25, hwnd, (HMENU)1007, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndShieldBtn = CreateWindowEx(0, "BUTTON", "Shield (J)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 170, 160, 75, 25, hwnd, (HMENU)1008, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            EnableWindow(hwndSaveBtn, FALSE);
            break;
        case WM_COMMAND:
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == 1001) {
                current_mode = SendMessage(hwndModeBox, CB_GETCURSEL, 0, 0);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 1002) {
                SaveGameState();
            } else if (LOWORD(wParam) == 1003) {
                LoadGameState();
            } else if (LOWORD(wParam) == 1004) {
                ResetStats();
            } else if (LOWORD(wParam) == 1005) {
                MessageBox(hwnd, "KSimon - How to Play\n\n"
                                 "Rules:\n"
                                 "Observe the pattern and repeat it back. The sequence grows each round.\n\n"
                                 "Controls:\n"
                                 "Mouse: Click colored buttons.\n"
                                 "Keyboard: Q, W, E (Top Row), A, S, D (Bottom Row) or 1-6 keys.\n"
                                 "Space: Start game.\n"
                                 "H: Use Hint (Replays sequence).\n"
                                 "F: Use Slow-Mo (2x slower replay speed).\n"
                                 "J: Shield (Absorbs 1 mistake).\n\n"
                                 "Modes:\n"
                                 "- Classic: Standard 6 colors.\n"
                                 "- Reverse: Repeat sequence backwards.\n"
                                 "- Speed: Faster flashing.\n"
                                 "- Endless: Only new color flashes each round.\n"
                                 "- Campaign: 15 stages with varying speeds, sequence targets & Reverse/Chaos modifiers.\n"
                                 "- Chaos: Unpredictable speeds & pitch variations.", "Help / How-to-Play", MB_OK | MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == 1006) {
                UseHint();
            } else if (LOWORD(wParam) == 1007) {
                UseSlowmo();
            } else if (LOWORD(wParam) == 1008) {
                UseShield();
            }
            break;
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            int cx = width / 2;
            int cy = (height + 200) / 2;
            int size = 80;
            int spacing = 5;
            
            int c1 = cx - size - size/2 - spacing;
            int c2 = cx - size/2;
            int c3 = cx + size/2 + spacing;
            
            SetRect(&btn_rects[0], c1, cy - size - spacing, c1 + size, cy - spacing);
            SetRect(&btn_rects[1], c2, cy - size - spacing, c2 + size, cy - spacing);
            SetRect(&btn_rects[2], c3, cy - size - spacing, c3 + size, cy - spacing);
            
            SetRect(&btn_rects[3], c1, cy + spacing, c1 + size, cy + size + spacing);
            SetRect(&btn_rects[4], c2, cy + spacing, c2 + size, cy + size + spacing);
            SetRect(&btn_rects[5], c3, cy + spacing, c3 + size, cy + size + spacing);
            break;
        }
        case WM_LBUTTONDOWN: {
            if (is_playing_sequence) break;
            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            for (int i = 0; i < 6; i++) {
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
            } else if (wParam == 'H') {
                UseHint();
            } else if (wParam == 'F') {
                UseSlowmo();
            } else if (wParam == 'J') {
                UseShield();
            } else if (!is_playing_sequence && sequence_length > 0) {
                if (wParam == 'Q' || wParam == '1') {
                    HandleClick(0);
                } else if (wParam == 'W' || wParam == '2') {
                    HandleClick(1);
                } else if (wParam == 'E' || wParam == '3') {
                    HandleClick(2);
                } else if (wParam == 'A' || wParam == '4') {
                    HandleClick(3);
                } else if (wParam == 'S' || wParam == '5') {
                    HandleClick(4);
                } else if (wParam == 'D' || wParam == '6') {
                    HandleClick(5);
                }
            }
            break;
        case WM_TIMER:
            if (wParam == TIMER_FLASH) {
                KillTimer(hwnd, TIMER_FLASH);
                flash_btn = -1;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (wParam == TIMER_GAME_OVER) {
                game_over_flash_count++;
                game_over_flash = game_over_flash_count % 2;
                InvalidateRect(hwnd, NULL, TRUE);
                if (game_over_flash_count >= 5) {
                    KillTimer(hwnd, TIMER_GAME_OVER);
                    game_over_flash = 0;
                    game_over_flash_count = 0;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            } else if (wParam == TIMER_SEQUENCE) {
                if (sequence_length == 0) {
                    KillTimer(hwnd, TIMER_SEQUENCE);
                    NextRound();
                } else {
                    if (flash_btn == -1) {
                        if (current_flash_index >= sequence_length) {
                            KillTimer(hwnd, TIMER_SEQUENCE);
                            is_playing_sequence = 0;
                            is_slowmo_active = 0;
                            int is_reverse = 0;
                            if (current_mode == MODE_REVERSE) {
                                is_reverse = 1;
                            } else if (current_mode == MODE_CAMPAIGN && campaign_stages[current_stage-1].modifier == 1) {
                                is_reverse = 1;
                            }
                            if (is_reverse) {
                                strcpy(status_text, "Your Turn (REVERSE)!");
                            } else {
                                strcpy(status_text, "Your Turn!");
                            }
                            EnableWindow(hwndSaveBtn, TRUE);
                            InvalidateRect(hwndMain, NULL, TRUE);
                        } else {
                            flash_btn = sequence[current_flash_index++];
                            InvalidateRect(hwndMain, NULL, TRUE);
                            int speed_factor = sequence_length - 1;
                            if (speed_factor < 0) speed_factor = 0;
                            
                            int f_dur = 400 - speed_factor * 20;
                            if (current_mode == MODE_SPEED) {
                                f_dur = 200 - speed_factor * 10;
                                if (f_dur < 80) f_dur = 80;
                            } else if (current_mode == MODE_CAMPAIGN) {
                                f_dur = campaign_stages[current_stage - 1].speed_ms - speed_factor * 8;
                                if (f_dur < 100) f_dur = 100;
                            } else if (current_mode == MODE_CHAOS) {
                                f_dur = 150 + rand() % 250;
                            } else {
                                if (f_dur < 150) f_dur = 150;
                            }

                            if (is_slowmo_active) f_dur *= 2;

                            int pitch = btn_freqs[flash_btn];
                            if (current_mode == MODE_CHAOS || (current_mode == MODE_CAMPAIGN && campaign_stages[current_stage-1].modifier == 2)) {
                                pitch += (rand() % 160) - 80;
                            }

                            PlaySoundAsync(pitch, f_dur);
                            SetTimer(hwnd, TIMER_SEQUENCE, f_dur, NULL);
                        }
                    } else {
                        flash_btn = -1;
                        InvalidateRect(hwndMain, NULL, TRUE);
                        int speed_factor = sequence_length - 1;
                        if (speed_factor < 0) speed_factor = 0;
                        
                        int p_dur = 200 - speed_factor * 10;
                        if (current_mode == MODE_SPEED) {
                            p_dur = 100 - speed_factor * 5;
                            if (p_dur < 40) p_dur = 40;
                        } else if (current_mode == MODE_CAMPAIGN) {
                            p_dur = (campaign_stages[current_stage - 1].speed_ms / 2) - speed_factor * 4;
                            if (p_dur < 50) p_dur = 50;
                        } else if (current_mode == MODE_CHAOS) {
                            p_dur = 80 + rand() % 120;
                        } else {
                            if (p_dur < 75) p_dur = 75;
                        }

                        if (is_slowmo_active) p_dur *= 2;

                        SetTimer(hwnd, TIMER_SEQUENCE, p_dur, NULL);
                    }
                }
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH bgBrush = CreateSolidBrush(game_over_flash ? RGB(170, 0, 0) : RGB(26, 26, 26));
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
        500, 560, NULL, NULL, hInstance, NULL
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
