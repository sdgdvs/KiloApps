#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#define BTN_GREEN  0
#define BTN_RED    1
#define BTN_YELLOW 2
#define BTN_BLUE   3
#define BTN_PURPLE 4
#define BTN_CYAN   5
#define BTN_ORANGE 6
#define BTN_PINK   7

#define TIMER_SEQUENCE  1
#define TIMER_FLASH     2
#define TIMER_GAME_OVER 3
#define TIMER_ANIM      4
#define TIMER_COUNTDOWN 5

#define MODE_4BTN_CLASSIC 0
#define MODE_6BTN_HEX     1
#define MODE_8BTN_OCTO    2
#define MODE_CHAOS_REV    3
#define MODE_PITCH_AUDIO  4
#define MODE_SPEED        5
#define MODE_ENDLESS      6
#define MODE_CAMPAIGN     7
#define MODE_CHAOS        8

#define NUM_MODES 9

typedef struct {
    int target_len;
    int num_colors;
    int speed_ms;
    int modifier; // 0: Normal, 1: Reverse, 2: Chaos, 3: Chaos Reverse
} CampaignStage;

CampaignStage campaign_stages[20] = {
    {4,  4, 400, 0}, // Stage 1
    {5,  4, 380, 0}, // Stage 2
    {6,  4, 350, 0}, // Stage 3
    {7,  4, 320, 1}, // Stage 4 (Reverse)
    {8,  4, 250, 0}, // Stage 5 (Speedy 4)
    {8,  6, 350, 0}, // Stage 6 (Hex Intro)
    {10, 6, 320, 1}, // Stage 7 (Reverse Hex)
    {12, 6, 300, 2}, // Stage 8 (Chaos Hex)
    {12, 6, 250, 0}, // Stage 9 (Speedy Hex)
    {14, 6, 220, 3}, // Stage 10 (Chaos Reverse Hex)
    {15, 8, 300, 0}, // Stage 11 (Octo Intro)
    {16, 8, 280, 1}, // Stage 12 (Reverse Octo)
    {18, 8, 250, 2}, // Stage 13 (Chaos Octo)
    {20, 8, 220, 3}, // Stage 14 (Chaos Reverse Octo)
    {22, 8, 200, 0}, // Stage 15 (Fast Octo)
    {24, 8, 180, 1}, // Stage 16 (Reverse Fast Octo)
    {25, 8, 160, 2}, // Stage 17 (Chaos Fast Octo)
    {26, 8, 140, 3}, // Stage 18 (Chaos Reverse Fast Octo)
    {28, 8, 120, 2}, // Stage 19 (Extreme Chaos Octo)
    {30, 8, 100, 3}  // Stage 20 (Grandmaster Memory Master Challenge)
};

int btn_freqs[8] = {415, 329, 261, 196, 493, 146, 554, 659};

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
HWND hwndFreezeBtn;

int current_mode = MODE_4BTN_CLASSIC;

int sequence[1000];
int sequence_length = 0;
int player_step = 0;
int is_playing_sequence = 0;
int current_flash_index = 0;
int flash_btn = -1;
int game_over_flash = 0;
int game_over_flash_count = 0;

float btn_scale[8] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
float btn_glow[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
int death_active = 0;
float death_scale = 0.0f;
float death_alpha = 0.0f;
int mascot_frame = 0;


int hints_remaining = 3;
int slowmo_remaining = 2;
int shields_remaining = 1;
int freezes_remaining = 2;

int is_slowmo_active = 0;
int is_time_frozen = 0;
int input_countdown = 150; // 15.0 seconds in 100ms units
int current_stage = 1;

RECT btn_rects[8];
COLORREF btn_colors[8] = {
    RGB(0, 170, 50),   // Green
    RGB(200, 0, 0),    // Red
    RGB(210, 170, 0),  // Yellow
    RGB(0, 80, 210),   // Blue
    RGB(160, 0, 200),  // Purple
    RGB(0, 180, 190),  // Cyan
    RGB(240, 110, 0),  // Orange
    RGB(255, 60, 150)  // Pink
};
COLORREF flash_colors[8] = {
    RGB(50, 255, 100),
    RGB(255, 70, 70),
    RGB(255, 255, 80),
    RGB(60, 160, 255),
    RGB(240, 80, 255),
    RGB(60, 240, 255),
    RGB(255, 170, 60),
    RGB(255, 140, 200)
};

char status_text[128] = "Press Space to Start";
int score = 0;
int high_scores[NUM_MODES] = {0};
int stat_games_played = 0;
int stat_longest_streak = 0;
int stat_best_time = 0;
time_t start_time = 0;

int GetActiveButtonCount() {
    if (current_mode == MODE_4BTN_CLASSIC) return 4;
    if (current_mode == MODE_6BTN_HEX) return 6;
    if (current_mode == MODE_8BTN_OCTO) return 8;
    if (current_mode == MODE_CAMPAIGN) return campaign_stages[current_stage - 1].num_colors;
    return 6;
}

// --- PARTICLE & GRAPHICS ANIMATION ENGINE ---
typedef struct {
    float x, y;
    float radius;
    float max_radius;
    COLORREF color;
    int active;
} SoundRipple;

#define MAX_RIPPLES 16
SoundRipple sound_ripples[MAX_RIPPLES];

typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float decay;
    COLORREF color;
    int active;
} SparkParticle;

#define MAX_SPARKS 80
SparkParticle spark_particles[MAX_SPARKS];

void TriggerSoundRipple(int btn_idx) {
    if (btn_idx < 0 || btn_idx >= 8) return;
    int cx = (btn_rects[btn_idx].left + btn_rects[btn_idx].right) / 2;
    int cy = (btn_rects[btn_idx].top + btn_rects[btn_idx].bottom) / 2;
    for (int i = 0; i < MAX_RIPPLES; i++) {
        if (!sound_ripples[i].active) {
            sound_ripples[i].x = (float)cx;
            sound_ripples[i].y = (float)cy;
            sound_ripples[i].radius = 20.0f;
            sound_ripples[i].max_radius = 75.0f;
            sound_ripples[i].color = flash_colors[btn_idx];
            sound_ripples[i].active = 1;
            break;
        }
    }
}

void TriggerVictoryFireworks(int cx, int cy) {
    COLORREF colors[8] = {
        RGB(0, 255, 200), RGB(255, 50, 100), RGB(255, 255, 50), RGB(50, 150, 255),
        RGB(220, 50, 255), RGB(50, 255, 100), RGB(255, 160, 40), RGB(255, 100, 180)
    };
    for (int i = 0; i < MAX_SPARKS; i++) {
        float angle = ((float)rand() / RAND_MAX) * 6.28318f;
        float speed = 2.0f + ((float)rand() / RAND_MAX) * 6.0f;
        spark_particles[i].x = (float)cx;
        spark_particles[i].y = (float)cy;
        spark_particles[i].vx = cosf(angle) * speed;
        spark_particles[i].vy = sinf(angle) * speed - 1.5f;
        spark_particles[i].life = 1.0f;
        spark_particles[i].decay = 0.02f + ((float)rand() / RAND_MAX) * 0.02f;
        spark_particles[i].color = colors[rand() % 8];
        spark_particles[i].active = 1;
    }
}

void TriggerErrorShards(int cx, int cy) {
    for (int i = 0; i < 40; i++) {
        float angle = ((float)rand() / RAND_MAX) * 6.28318f;
        float speed = 3.0f + ((float)rand() / RAND_MAX) * 7.0f;
        spark_particles[i].x = (float)cx;
        spark_particles[i].y = (float)cy;
        spark_particles[i].vx = cosf(angle) * speed;
        spark_particles[i].vy = sinf(angle) * speed;
        spark_particles[i].life = 1.0f;
        spark_particles[i].decay = 0.03f + ((float)rand() / RAND_MAX) * 0.02f;
        spark_particles[i].color = RGB(255, 30, 60);
        spark_particles[i].active = 1;
    }
}

void TriggerButtonBurst(int btn_idx) {
    if (btn_idx < 0 || btn_idx >= 8) return;
    int cx = (btn_rects[btn_idx].left + btn_rects[btn_idx].right) / 2;
    int cy = (btn_rects[btn_idx].top + btn_rects[btn_idx].bottom) / 2;
    int spawned = 0;
    for (int i = 0; i < MAX_SPARKS && spawned < 15; i++) {
        if (!spark_particles[i].active) {
            float angle = ((float)rand() / RAND_MAX) * 6.28318f;
            float speed = 1.0f + ((float)rand() / RAND_MAX) * 3.0f;
            spark_particles[i].x = (float)cx;
            spark_particles[i].y = (float)cy;
            spark_particles[i].vx = cosf(angle) * speed;
            spark_particles[i].vy = sinf(angle) * speed;
            spark_particles[i].life = 1.0f;
            spark_particles[i].decay = 0.03f + ((float)rand() / RAND_MAX) * 0.03f;
            spark_particles[i].color = flash_colors[btn_idx];
            spark_particles[i].active = 1;
            spawned++;
        }
    }
}

void ActivateButton(int btn_idx) {
    flash_btn = btn_idx;
    btn_scale[btn_idx] = 1.15f;
    btn_glow[btn_idx] = 1.0f;
    TriggerSoundRipple(btn_idx);
    TriggerButtonBurst(btn_idx);
}

void UpdateParticles() {
    for (int i = 0; i < MAX_RIPPLES; i++) {
        if (sound_ripples[i].active) {
            sound_ripples[i].radius += 3.0f;
            if (sound_ripples[i].radius >= sound_ripples[i].max_radius) {
                sound_ripples[i].active = 0;
            }
        }
    }
    for (int i = 0; i < MAX_SPARKS; i++) {
        if (spark_particles[i].active) {
            spark_particles[i].x += spark_particles[i].vx;
            spark_particles[i].y += spark_particles[i].vy;
            spark_particles[i].vy += 0.12f; // Gravity
            spark_particles[i].life -= spark_particles[i].decay;
            if (spark_particles[i].life <= 0.0f) {
                spark_particles[i].active = 0;
            }
        }
    }
    for (int i = 0; i < 8; i++) {
        if (btn_scale[i] > 1.0f) {
            btn_scale[i] -= 0.02f;
            if (btn_scale[i] < 1.0f) btn_scale[i] = 1.0f;
        }
        if (btn_glow[i] > 0.0f) {
            btn_glow[i] -= 0.05f;
            if (btn_glow[i] < 0.0f) btn_glow[i] = 0.0f;
        }
    }
    if (death_active) {
        death_scale += 0.05f;
        death_alpha -= 0.02f;
        if (death_alpha <= 0.0f) death_active = 0;
    }
    mascot_frame = (GetTickCount() / 200) % 4;
}

void LoadHighScores() {
    high_scores[MODE_4BTN_CLASSIC] = GetPrivateProfileInt("HighScores", "Classic4", 0, ".\\ksimon.ini");
    high_scores[MODE_6BTN_HEX]     = GetPrivateProfileInt("HighScores", "Hex6", 0, ".\\ksimon.ini");
    high_scores[MODE_8BTN_OCTO]    = GetPrivateProfileInt("HighScores", "Octo8", 0, ".\\ksimon.ini");
    high_scores[MODE_CHAOS_REV]   = GetPrivateProfileInt("HighScores", "ChaosRev", 0, ".\\ksimon.ini");
    high_scores[MODE_PITCH_AUDIO] = GetPrivateProfileInt("HighScores", "PitchAudio", 0, ".\\ksimon.ini");
    high_scores[MODE_SPEED]       = GetPrivateProfileInt("HighScores", "Speed", 0, ".\\ksimon.ini");
    high_scores[MODE_ENDLESS]     = GetPrivateProfileInt("HighScores", "Endless", 0, ".\\ksimon.ini");
    high_scores[MODE_CAMPAIGN]    = GetPrivateProfileInt("HighScores", "Campaign", 0, ".\\ksimon.ini");
    high_scores[MODE_CHAOS]       = GetPrivateProfileInt("HighScores", "Chaos", 0, ".\\ksimon.ini");
    
    stat_games_played   = GetPrivateProfileInt("Stats", "GamesPlayed", 0, ".\\ksimon.ini");
    stat_longest_streak = GetPrivateProfileInt("Stats", "LongestStreak", 0, ".\\ksimon.ini");
    stat_best_time      = GetPrivateProfileInt("Stats", "BestTime", 0, ".\\ksimon.ini");
}

void SaveHighScore(int mode, int s) {
    char str[32];
    sprintf(str, "%d", s);
    const char* keys[NUM_MODES] = {
        "Classic4", "Hex6", "Octo8", "ChaosRev", "PitchAudio", "Speed", "Endless", "Campaign", "Chaos"
    };
    if (mode >= 0 && mode < NUM_MODES) {
        WritePrivateProfileString("HighScores", keys[mode], str, ".\\ksimon.ini");
    }
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
    sprintf(temp, "%d", freezes_remaining);
    WritePrivateProfileString("GameState", "Freezes", temp, ".\\ksimon.ini");
    sprintf(temp, "%d", current_stage);
    WritePrivateProfileString("GameState", "Stage", temp, ".\\ksimon.ini");
    strcpy(status_text, "Game Saved!");
    InvalidateRect(hwndMain, NULL, FALSE);
}

void LoadGameState() {
    int len = GetPrivateProfileInt("GameState", "Length", 0, ".\\ksimon.ini");
    if (len == 0) {
        strcpy(status_text, "No saved game found.");
        InvalidateRect(hwndMain, NULL, FALSE);
        return;
    }
    sequence_length = len;
    current_mode = GetPrivateProfileInt("GameState", "Mode", MODE_4BTN_CLASSIC, ".\\ksimon.ini");
    int elapsed = GetPrivateProfileInt("GameState", "ElapsedTime", 0, ".\\ksimon.ini");
    start_time = time(NULL) - elapsed;
    hints_remaining = GetPrivateProfileInt("GameState", "Hints", 3, ".\\ksimon.ini");
    slowmo_remaining = GetPrivateProfileInt("GameState", "Slowmo", 2, ".\\ksimon.ini");
    shields_remaining = GetPrivateProfileInt("GameState", "Shields", 1, ".\\ksimon.ini");
    freezes_remaining = GetPrivateProfileInt("GameState", "Freezes", 2, ".\\ksimon.ini");
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
    InvalidateRect(hwndMain, NULL, FALSE);
    SetTimer(hwndMain, TIMER_SEQUENCE, 1000, NULL);
}

void ResetStats() {
    stat_games_played = 0;
    stat_longest_streak = 0;
    stat_best_time = 0;
    SaveStats();
    strcpy(status_text, "Stats Reset!");
    InvalidateRect(hwndMain, NULL, FALSE);
}

// --- GDI VECTOR ICON DRAWING ---
void DrawButtonIcon(HDC hdc, int btn_idx, int cx, int cy, COLORREF color) {
    HPEN pen = CreatePen(PS_SOLID, 3, color);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    HBRUSH brush = CreateSolidBrush(color);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);

    if (btn_idx == BTN_GREEN) {
        // 🎼 Treble Clef
        Ellipse(hdc, cx - 10, cy + 2, cx - 2, cy + 10);
        MoveToEx(hdc, cx - 2, cy + 6, NULL);
        LineTo(hdc, cx - 2, cy - 12);
        Arc(hdc, cx - 12, cy - 16, cx + 8, cy, cx - 2, cy - 12, cx - 2, cy - 2);
    } else if (btn_idx == BTN_RED) {
        // 🎵 Twin Eighth Notes
        Ellipse(hdc, cx - 14, cy + 4, cx - 6, cy + 12);
        Ellipse(hdc, cx + 2, cy + 2, cx + 10, cy + 10);
        MoveToEx(hdc, cx - 6, cy + 8, NULL); LineTo(hdc, cx - 6, cy - 8);
        MoveToEx(hdc, cx + 10, cy + 6, NULL); LineTo(hdc, cx + 10, cy - 10);
        MoveToEx(hdc, cx - 6, cy - 8, NULL); LineTo(hdc, cx + 10, cy - 10);
    } else if (btn_idx == BTN_YELLOW) {
        // ⭐ 5-Pointed Star
        POINT pts[10];
        for (int i = 0; i < 5; i++) {
            float aO = i * 4.0f * 3.14159f / 5.0f - 1.5708f;
            float aI = aO + 0.6283f;
            pts[i*2].x = cx + (int)(cosf(aO) * 14.0f);
            pts[i*2].y = cy + (int)(sinf(aO) * 14.0f);
            pts[i*2+1].x = cx + (int)(cosf(aI) * 6.0f);
            pts[i*2+1].y = cy + (int)(sinf(aI) * 6.0f);
        }
        Polygon(hdc, pts, 10);
    } else if (btn_idx == BTN_BLUE) {
        // ✨ 4-Pointed Sparkle Star
        POINT pts[8];
        for (int i = 0; i < 4; i++) {
            float aO = i * 1.5708f;
            float aI = aO + 0.7854f;
            pts[i*2].x = cx + (int)(cosf(aO) * 15.0f);
            pts[i*2].y = cy + (int)(sinf(aO) * 15.0f);
            pts[i*2+1].x = cx + (int)(cosf(aI) * 4.0f);
            pts[i*2+1].y = cy + (int)(sinf(aI) * 4.0f);
        }
        Polygon(hdc, pts, 8);
    } else if (btn_idx == BTN_PURPLE) {
        // 💎 Faceted Diamond
        POINT pts[4] = {
            {cx, cy - 12}, {cx + 13, cy - 2}, {cx, cy + 14}, {cx - 13, cy - 2}
        };
        Polygon(hdc, pts, 4);
    } else if (btn_idx == BTN_CYAN) {
        // ⚡ Lightning Bolt
        POINT pts[6] = {
            {cx + 2, cy - 14}, {cx - 10, cy + 1}, {cx - 1, cy + 1},
            {cx - 3, cy + 14}, {cx + 9, cy - 1}, {cx, cy - 1}
        };
        Polygon(hdc, pts, 6);
    } else if (btn_idx == BTN_ORANGE) {
        // 🔥 Flame
        POINT pts[5] = {
            {cx, cy - 14}, {cx + 8, cy - 2}, {cx + 5, cy + 12},
            {cx - 5, cy + 12}, {cx - 8, cy - 2}
        };
        Polygon(hdc, pts, 5);
    } else if (btn_idx == BTN_PINK) {
        // 💖 Heart
        Ellipse(hdc, cx - 12, cy - 10, cx, cy + 2);
        Ellipse(hdc, cx, cy - 10, cx + 12, cy + 2);
        POINT pts[3] = {{cx - 11, cy - 1}, {cx + 11, cy - 1}, {cx, cy + 14}};
        Polygon(hdc, pts, 3);
    }

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);
}

void LayoutButtons(int width, int height) {
    int cx = width / 2;
    int cy = (height + 170) / 2;
    int num_btns = GetActiveButtonCount();

    if (num_btns == 4) {
        int size = 76;
        int spacing = 16;
        int c1 = cx - size - spacing/2;
        int c2 = cx + spacing/2;
        SetRect(&btn_rects[0], c1, cy - size - spacing/2, c1 + size, cy - spacing/2);
        SetRect(&btn_rects[1], c2, cy - size - spacing/2, c2 + size, cy - spacing/2);
        SetRect(&btn_rects[2], c1, cy + spacing/2, c1 + size, cy + size + spacing/2);
        SetRect(&btn_rects[3], c2, cy + spacing/2, c2 + size, cy + size + spacing/2);
    } else if (num_btns == 6) {
        int size = 72;
        int spacing = 12;
        int c1 = cx - size - size/2 - spacing;
        int c2 = cx - size/2;
        int c3 = cx + size/2 + spacing;
        SetRect(&btn_rects[0], c1, cy - size - spacing, c1 + size, cy - spacing);
        SetRect(&btn_rects[1], c2, cy - size - spacing, c2 + size, cy - spacing);
        SetRect(&btn_rects[2], c3, cy - size - spacing, c3 + size, cy - spacing);
        SetRect(&btn_rects[3], c1, cy + spacing, c1 + size, cy + size + spacing);
        SetRect(&btn_rects[4], c2, cy + spacing, c2 + size, cy + size + spacing);
        SetRect(&btn_rects[5], c3, cy + spacing, c3 + size, cy + size + spacing);
    } else { // 8 buttons
        int size = 64;
        int spacing = 8;
        int c1 = cx - 2*size - 15;
        int c2 = cx - size - 5;
        int c3 = cx + 5;
        int c4 = cx + size + 15;
        SetRect(&btn_rects[0], c1, cy - size - spacing, c1 + size, cy - spacing);
        SetRect(&btn_rects[1], c2, cy - size - spacing, c2 + size, cy - spacing);
        SetRect(&btn_rects[2], c3, cy - size - spacing, c3 + size, cy - spacing);
        SetRect(&btn_rects[3], c4, cy - size - spacing, c4 + size, cy - spacing);
        SetRect(&btn_rects[4], c1, cy + spacing, c1 + size, cy + size + spacing);
        SetRect(&btn_rects[5], c2, cy + spacing, c2 + size, cy + size + spacing);
        SetRect(&btn_rects[6], c3, cy + spacing, c3 + size, cy + size + spacing);
        SetRect(&btn_rects[7], c4, cy + spacing, c4 + size, cy + size + spacing);
    }
}

void DrawBoard(HDC hdc, int width, int height) {
    COLORREF bgColor = game_over_flash ? RGB(140, 0, 0) : RGB(18, 18, 24);
    HBRUSH bgBrush = CreateSolidBrush(bgColor);
    RECT fullRc = {0, 0, width, height};
    FillRect(hdc, &fullRc, bgBrush);
    DeleteObject(bgBrush);

    int cx = width / 2;
    int cy = (height + 170) / 2;
    int consoleRadius = 200;

    // Outer Metallic Rim
    for (int r = consoleRadius; r >= consoleRadius - 12; r--) {
        int v = 60 + (consoleRadius - r) * 12;
        if (v > 220) v = 220;
        HPEN rimPen = CreatePen(PS_SOLID, 1, RGB(v, v, v + 10));
        HGDIOBJ oldPen = SelectObject(hdc, rimPen);
        HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(rimPen);
    }

    // Inner Dark Console Faceplate
    HBRUSH faceBrush = CreateSolidBrush(RGB(28, 28, 36));
    HGDIOBJ oldBrush = SelectObject(hdc, faceBrush);
    Ellipse(hdc, cx - (consoleRadius - 12), cy - (consoleRadius - 12), cx + (consoleRadius - 12), cy + (consoleRadius - 12));
    SelectObject(hdc, oldBrush);
    DeleteObject(faceBrush);

    // Screws
    for (int i = 0; i < 8; i++) {
        float a = i * 0.785398f;
        int sx = cx + (int)(cosf(a) * (consoleRadius - 6));
        int sy = cy + (int)(sinf(a) * (consoleRadius - 6));
        
        HBRUSH screwBrush = CreateSolidBrush(RGB(190, 190, 200));
        HGDIOBJ oldB = SelectObject(hdc, screwBrush);
        Ellipse(hdc, sx - 4, sy - 4, sx + 4, sy + 4);
        SelectObject(hdc, oldB);
        DeleteObject(screwBrush);

        HPEN sPen = CreatePen(PS_SOLID, 1, RGB(40, 40, 50));
        HGDIOBJ oldP = SelectObject(hdc, sPen);
        MoveToEx(hdc, sx - 2, sy, NULL); LineTo(hdc, sx + 2, sy);
        MoveToEx(hdc, sx, sy - 2, NULL); LineTo(hdc, sx, sy + 2);
        SelectObject(hdc, oldP);
        DeleteObject(sPen);
    }

    // Sound Ripples
    for (int i = 0; i < MAX_RIPPLES; i++) {
        if (sound_ripples[i].active) {
            int r = (int)sound_ripples[i].radius;
            HPEN ripPen = CreatePen(PS_SOLID, 2, sound_ripples[i].color);
            HGDIOBJ oldP = SelectObject(hdc, ripPen);
            HGDIOBJ oldB = SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Ellipse(hdc, (int)sound_ripples[i].x - r, (int)sound_ripples[i].y - r, (int)sound_ripples[i].x + r, (int)sound_ripples[i].y + r);
            SelectObject(hdc, oldP);
            SelectObject(hdc, oldB);
            DeleteObject(ripPen);
        }
    }

    // Render Buttons
    int num_btns = GetActiveButtonCount();
    const char* keyLabels[8] = {"Q", "W", "E", "R", "A", "S", "D", "F"};
    if (num_btns == 4) {
        keyLabels[0] = "Q"; keyLabels[1] = "W"; keyLabels[2] = "A"; keyLabels[3] = "S";
    } else if (num_btns == 6) {
        keyLabels[0] = "Q"; keyLabels[1] = "W"; keyLabels[2] = "E";
        keyLabels[3] = "A"; keyLabels[4] = "S"; keyLabels[5] = "D";
    }

    for (int i = 0; i < num_btns; i++) {
        RECT r = btn_rects[i];
        int isFlash = (flash_btn == i);

        int bCx = (r.left + r.right) / 2;
        int bCy = (r.top + r.bottom) / 2;
        int radius = (int)(((r.right - r.left) / 2) * btn_scale[i]);

        int num_sides = 4;
        float angleOffset = 0.785398f; // 45 degrees
        if (num_btns == 6) { num_sides = 6; angleOffset = 0.523599f; /* 30 deg */ }
        if (num_btns == 8) { num_sides = 8; angleOffset = 0.392699f; /* 22.5 deg */ }
        
        COLORREF cFill = btn_colors[i];
        if (isFlash) {
            cFill = (current_mode == MODE_PITCH_AUDIO && is_playing_sequence) ? RGB(70, 70, 80) : flash_colors[i];
        }

        // Define Points
        POINT ptsOuter[8];
        POINT ptsInner[8];
        for (int j = 0; j < num_sides; j++) {
            float angle = j * 6.283185f / num_sides + angleOffset;
            float rOut = radius + 2;
            float rInn = radius;
            if (num_sides == 4) { rOut *= 1.414f; rInn *= 1.414f; }
            
            ptsOuter[j].x = bCx + (int)(cosf(angle) * rOut);
            ptsOuter[j].y = bCy + (int)(sinf(angle) * rOut);
            ptsInner[j].x = bCx + (int)(cosf(angle) * rInn);
            ptsInner[j].y = bCy + (int)(sinf(angle) * rInn);
        }

        // Glow
        if (isFlash && current_mode != MODE_PITCH_AUDIO) {
            COLORREF gCol = flash_colors[i];
            for (int g = 8; g >= 1; g -= 2) {
                POINT ptsGlow[8];
                for(int j=0; j<num_sides; j++) {
                    float angle = j * 6.283185f / num_sides + angleOffset;
                    float rGlow = radius + g;
                    if (num_sides == 4) rGlow *= 1.414f;
                    ptsGlow[j].x = bCx + (int)(cosf(angle) * rGlow);
                    ptsGlow[j].y = bCy + (int)(sinf(angle) * rGlow);
                }
                HPEN glowPen = CreatePen(PS_SOLID, 2, gCol);
                HGDIOBJ oldP = SelectObject(hdc, glowPen);
                HGDIOBJ oldB = SelectObject(hdc, GetStockObject(NULL_BRUSH));
                Polygon(hdc, ptsGlow, num_sides);
                SelectObject(hdc, oldP);
                SelectObject(hdc, oldB);
                DeleteObject(glowPen);
            }
        }

        // Bezel
        HBRUSH bezelBrush = CreateSolidBrush(RGB(10, 10, 15));
        HGDIOBJ oldB = SelectObject(hdc, bezelBrush);
        HPEN bezPen = CreatePen(PS_SOLID, 1, RGB(10,10,15));
        HGDIOBJ oldP = SelectObject(hdc, bezPen);
        Polygon(hdc, ptsOuter, num_sides);
        SelectObject(hdc, oldB);
        SelectObject(hdc, oldP);
        DeleteObject(bezelBrush);
        DeleteObject(bezPen);

        // Body
        HBRUSH btnBrush = CreateSolidBrush(cFill);
        oldB = SelectObject(hdc, btnBrush);
        HPEN borderPen = CreatePen(PS_SOLID, 2, isFlash ? RGB(255,255,255) : RGB(80,80,80));
        oldP = SelectObject(hdc, borderPen);
        Polygon(hdc, ptsInner, num_sides);
        SelectObject(hdc, oldB);
        SelectObject(hdc, oldP);
        DeleteObject(btnBrush);
        DeleteObject(borderPen);

        // 3D Bevel (Top half highlight)
        HPEN hiPen = CreatePen(PS_SOLID, 2, isFlash ? RGB(255, 255, 255) : RGB(220, 220, 220));
        oldP = SelectObject(hdc, hiPen);
        MoveToEx(hdc, ptsInner[num_sides/2].x, ptsInner[num_sides/2].y, NULL);
        for(int j=num_sides/2; j<=num_sides; j++) {
            LineTo(hdc, ptsInner[j%num_sides].x, ptsInner[j%num_sides].y);
        }
        SelectObject(hdc, oldP);
        DeleteObject(hiPen);

        // Icon & Label
        int iconCy = bCy - 4;
        DrawButtonIcon(hdc, i, bCx, iconCy, isFlash ? RGB(255, 255, 255) : RGB(230, 230, 230));

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, isFlash ? RGB(255, 255, 255) : RGB(180, 180, 180));
        TextOutA(hdc, bCx - 4, iconCy + 14, keyLabels[i], strlen(keyLabels[i]));
    }

    // Center Disc
    int discR = 45;
    HBRUSH discB = CreateSolidBrush(RGB(42, 44, 54));
    HGDIOBJ oldB = SelectObject(hdc, discB);
    Ellipse(hdc, cx - discR, cy - discR, cx + discR, cy + discR);
    SelectObject(hdc, oldB);
    DeleteObject(discB);

    // Mascot Frame Cycle
    HBRUSH mascotFace = CreateSolidBrush(RGB(17, 17, 17));
    oldB = SelectObject(hdc, mascotFace);
    HPEN mascotPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 204));
    HPEN oldP = SelectObject(hdc, mascotPen);
    Ellipse(hdc, cx - 16, cy - 16 + 15, cx + 16, cy + 16 + 15);
    SelectObject(hdc, oldB);
    DeleteObject(mascotFace);

    HBRUSH eyeBrush = CreateSolidBrush(RGB(0, 255, 204));
    oldB = SelectObject(hdc, eyeBrush);
    if (mascot_frame == 1) { // blink
        RECT le = {cx - 6, cy - 4 + 15, cx - 2, cy - 3 + 15};
        RECT re = {cx + 2, cy - 4 + 15, cx + 6, cy - 3 + 15};
        FillRect(hdc, &le, eyeBrush); FillRect(hdc, &re, eyeBrush);
    } else {
        RECT le = {cx - 6, cy - 6 + 15, cx - 2, cy - 2 + 15};
        RECT re = {cx + 2, cy - 6 + 15, cx + 6, cy - 2 + 15};
        FillRect(hdc, &le, eyeBrush); FillRect(hdc, &re, eyeBrush);
    }

    if (is_playing_sequence) {
        if (mascot_frame == 0 || mascot_frame == 2) {
            Ellipse(hdc, cx - 3, cy + 1 + 15, cx + 3, cy + 7 + 15);
        } else {
            RECT mouth = {cx - 4, cy + 3 + 15, cx + 4, cy + 5 + 15};
            FillRect(hdc, &mouth, eyeBrush);
        }
    } else {
        if (mascot_frame == 3) {
            MoveToEx(hdc, cx - 5, cy + 2 + 15, NULL);
            LineTo(hdc, cx, cy + 6 + 15);
            LineTo(hdc, cx + 5, cy + 2 + 15);
        } else {
            RECT mouth = {cx - 4, cy + 3 + 15, cx + 4, cy + 4 + 15};
            FillRect(hdc, &mouth, eyeBrush);
        }
    }
    SelectObject(hdc, oldB);
    DeleteObject(eyeBrush);
    SelectObject(hdc, oldP);
    DeleteObject(mascotPen);

    // Death Effect (Skull Burst)
    if (death_active) {
        int dr = (int)(60 * death_scale);
        HBRUSH skullB = CreateSolidBrush(RGB(255, 0, 0));
        oldB = SelectObject(hdc, skullB);
        HPEN nullPen = CreatePen(PS_NULL, 0, 0);
        oldP = SelectObject(hdc, nullPen);
        
        Ellipse(hdc, cx - dr, cy - dr - 10, cx + dr, cy + dr - 10);
        RECT jaw = {cx - dr/2, cy + dr - 10, cx + dr/2, cy + dr + dr/2 - 10};
        FillRect(hdc, &jaw, skullB);
        
        HBRUSH blackB = CreateSolidBrush(RGB(0, 0, 0));
        SelectObject(hdc, blackB);
        int er = dr / 4;
        Ellipse(hdc, cx - dr/2 - er, cy - 10 - er, cx - dr/2 + er, cy - 10 + er);
        Ellipse(hdc, cx + dr/2 - er, cy - 10 - er, cx + dr/2 + er, cy - 10 + er);
        
        SelectObject(hdc, oldB);
        SelectObject(hdc, oldP);
        DeleteObject(skullB);
        DeleteObject(blackB);
        DeleteObject(nullPen);
    }

    // LED Score Box
    RECT ledRc = {cx - 32, cy - 18, cx + 32, cy + 6};
    HBRUSH ledB = CreateSolidBrush(RGB(6, 22, 14));
    FillRect(hdc, &ledRc, ledB);
    DeleteObject(ledB);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, is_time_frozen ? RGB(100, 220, 255) : RGB(0, 255, 204));
    HFONT ledFont = CreateFontA(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
    HGDIOBJ oldF = SelectObject(hdc, ledFont);
    char scoreStr[32];
    if (!is_playing_sequence && sequence_length > 0) {
        if (is_time_frozen) sprintf(scoreStr, "FROZ");
        else sprintf(scoreStr, "%02d|%02d", score, input_countdown / 10);
    } else {
        sprintf(scoreStr, "%02d", score);
    }
    DrawTextA(hdc, scoreStr, -1, &ledRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldF);
    DeleteObject(ledFont);

    // Power Indicator Dot
    HBRUSH pwrB = CreateSolidBrush(is_playing_sequence ? RGB(255, 255, 0) : (sequence_length > 0 ? (is_time_frozen ? RGB(0, 200, 255) : RGB(0, 255, 0)) : RGB(0, 136, 204)));
    oldB = SelectObject(hdc, pwrB);
    Ellipse(hdc, cx - 4, cy + 12, cx + 4, cy + 20);
    SelectObject(hdc, oldB);
    DeleteObject(pwrB);

    SetTextColor(hdc, RGB(150, 150, 160));
    TextOutA(hdc, cx - 22, cy + 24, "K-SIMON", 7);

    // Particles
    for (int i = 0; i < MAX_SPARKS; i++) {
        if (spark_particles[i].active) {
            int px = (int)spark_particles[i].x;
            int py = (int)spark_particles[i].y;
            HBRUSH spkB = CreateSolidBrush(spark_particles[i].color);
            RECT spkRc = {px - 2, py - 2, px + 3, py + 3};
            FillRect(hdc, &spkRc, spkB);
            DeleteObject(spkB);
        }
    }

    // HUD Text
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 255, 204));
    TextOutA(hdc, 10, 8, status_text, strlen(status_text));
    
    char score_text[64];
    sprintf(score_text, "Score: %d | High Score: %d", score, high_scores[current_mode]);
    SetTextColor(hdc, RGB(220, 220, 220));
    TextOutA(hdc, 10, 26, score_text, strlen(score_text));

    char stats_text[128];
    sprintf(stats_text, "Games: %d | Streak: %d | Time: %ds", stat_games_played, stat_longest_streak, stat_best_time);
    SetTextColor(hdc, RGB(160, 160, 170));
    TextOutA(hdc, 10, 44, stats_text, strlen(stats_text));

    char pwr_text[128];
    sprintf(pwr_text, "Hint(H):%d | Slow(S):%d | Shield(B):%d | Freeze(F):%d", 
            hints_remaining, slowmo_remaining, shields_remaining, freezes_remaining);
    SetTextColor(hdc, RGB(255, 215, 0));
    TextOutA(hdc, 10, 62, pwr_text, strlen(pwr_text));

    if (current_mode == MODE_CAMPAIGN) {
        char stage_text[128];
        const char* mod_str = "NORMAL";
        if (campaign_stages[current_stage-1].modifier == 1) mod_str = "REVERSE";
        else if (campaign_stages[current_stage-1].modifier == 2) mod_str = "CHAOS";
        else if (campaign_stages[current_stage-1].modifier == 3) mod_str = "CHAOS REVERSE";
        sprintf(stage_text, "Campaign Stage: %d/20 [%s] (Target Len: %d | Btns: %d)", 
                current_stage, mod_str, campaign_stages[current_stage-1].target_len, campaign_stages[current_stage-1].num_colors);
        SetTextColor(hdc, RGB(0, 255, 204));
        TextOutA(hdc, 10, 80, stage_text, strlen(stage_text));
    } else if (current_mode == MODE_PITCH_AUDIO) {
        SetTextColor(hdc, RGB(255, 200, 50));
        TextOutA(hdc, 10, 80, "Mode: PITCH AUDIO - Sound-Only! Hear the tone pitch!", 53);
    } else if (current_mode == MODE_CHAOS_REV) {
        SetTextColor(hdc, RGB(255, 100, 255));
        TextOutA(hdc, 10, 80, "Mode: CHAOS REVERSE - Random pitch/speed & Reverse!", 51);
    } else {
        SetTextColor(hdc, RGB(180, 180, 180));
        TextOutA(hdc, 10, 80, "Controls: Q,W,E,R / A,S,D,F or 1-8 keys", 39);
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
    freezes_remaining = 2;
    current_stage = 1;
    is_slowmo_active = 0;
    is_time_frozen = 0;
    is_playing_sequence = 1;
    start_time = time(NULL);
    RECT rc;
    GetClientRect(hwndMain, &rc);
    LayoutButtons(rc.right, rc.bottom);
    strcpy(status_text, "Get Ready...");
    InvalidateRect(hwndMain, NULL, FALSE);
    SetTimer(hwndMain, TIMER_SEQUENCE, 1000, NULL);
}

void NextRound() {
    player_step = 0;
    is_time_frozen = 0;
    input_countdown = (current_mode == MODE_SPEED) ? 100 : 150;
    
    int num_colors = GetActiveButtonCount();
    sequence[sequence_length++] = rand() % num_colors;
    score = sequence_length - 1;
    is_playing_sequence = 1;
    current_flash_index = (current_mode == MODE_ENDLESS && sequence_length > 0) ? sequence_length - 1 : 0;
    strcpy(status_text, is_slowmo_active ? "Watch (Slow-Mo)..." : "Watch...");
    InvalidateRect(hwndMain, NULL, FALSE);
    SetTimer(hwndMain, TIMER_SEQUENCE, 500, NULL);
}

void UseHint() {
    if (!is_playing_sequence && sequence_length > 0 && hints_remaining > 0) {
        hints_remaining--;
        is_playing_sequence = 1;
        current_flash_index = (current_mode == MODE_ENDLESS && sequence_length > 0) ? sequence_length - 1 : 0;
        strcpy(status_text, "Hint: Watch...");
        InvalidateRect(hwndMain, NULL, FALSE);
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
        InvalidateRect(hwndMain, NULL, FALSE);
        SetTimer(hwndMain, TIMER_SEQUENCE, 800, NULL);
    }
}

void UseShield() {
    if (shields_remaining > 0) {
        sprintf(status_text, "Shield Active! (%d Shields available)", shields_remaining);
        InvalidateRect(hwndMain, NULL, FALSE);
    }
}

void UseFreeze() {
    if (!is_playing_sequence && sequence_length > 0 && freezes_remaining > 0 && !is_time_frozen) {
        freezes_remaining--;
        is_time_frozen = 1;
        sprintf(status_text, "Time Frozen! (%d Freezes left)", freezes_remaining);
        InvalidateRect(hwndMain, NULL, FALSE);
    }
}

void TriggerGameOver() {
    KillTimer(hwndMain, TIMER_COUNTDOWN);
    RECT rc;
    GetClientRect(hwndMain, &rc);
    TriggerErrorShards(rc.right / 2, rc.bottom / 2);
    death_active = 1;
    death_scale = 0.5f;
    death_alpha = 1.0f;
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
    InvalidateRect(hwndMain, NULL, FALSE);
}

void HandleClick(int btn_id) {
    if (is_playing_sequence || sequence_length == 0) return;
    if (btn_id >= GetActiveButtonCount()) return;

    ActivateButton(btn_id);
    EnableWindow(hwndSaveBtn, FALSE);
    InvalidateRect(hwndMain, NULL, FALSE);
    SetTimer(hwndMain, TIMER_FLASH, (current_mode == MODE_SPEED) ? 150 : 300, NULL);

    int is_reverse = 0;
    if (current_mode == MODE_CHAOS_REV) {
        is_reverse = 1;
    } else if (current_mode == MODE_CAMPAIGN) {
        int mod = campaign_stages[current_stage-1].modifier;
        if (mod == 1 || mod == 3) is_reverse = 1;
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
            InvalidateRect(hwndMain, NULL, FALSE);
            return;
        }

        TriggerGameOver();
        return;
    }

    PlaySoundAsync(btn_freqs[btn_id], 200);
    input_countdown = (current_mode == MODE_SPEED) ? 100 : 150; // Reset timer for fairness

    player_step++;
    if (player_step == sequence_length) {
        KillTimer(hwndMain, TIMER_COUNTDOWN);
        if (current_mode == MODE_CAMPAIGN) {
            int target = campaign_stages[current_stage - 1].target_len;
            if (sequence_length >= target) {
                current_stage++;
                RECT rc;
                GetClientRect(hwndMain, &rc);
                TriggerVictoryFireworks(rc.right / 2, rc.bottom / 2);
                if (current_stage > 20) {
                    strcpy(status_text, "GRANDMASTER VICTORY! Campaign Complete!");
                    is_playing_sequence = 1;
                    score += 200;
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
                    InvalidateRect(hwndMain, NULL, FALSE);
                    return;
                } else {
                    sprintf(status_text, "Stage %d Cleared! +1 Skill Charges", current_stage - 1);
                    hints_remaining++;
                    slowmo_remaining++;
                    shields_remaining++;
                    freezes_remaining++;
                    sequence_length = 0;
                    LayoutButtons(rc.right, rc.bottom);
                }
            } else {
                strcpy(status_text, "Good! Get ready...");
            }
        } else {
            strcpy(status_text, "Good! Get ready...");
        }
        is_playing_sequence = 1;
        InvalidateRect(hwndMain, NULL, FALSE);
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
                10, 110, 160, 200, hwnd, (HMENU)1001, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"4-Button Classic");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"6-Button Hex");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"8-Button Octo");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Chaos Reverse");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Pitch Audio");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Speed Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Endless Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Campaign (20 Stages)");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Chaos Mode");
            SendMessage(hwndModeBox, CB_SETCURSEL, current_mode, 0);

            hwndSaveBtn   = CreateWindowEx(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 175, 110, 45, 25, hwnd, (HMENU)1002, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndLoadBtn   = CreateWindowEx(0, "BUTTON", "Load", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 223, 110, 45, 25, hwnd, (HMENU)1003, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndResetBtn  = CreateWindowEx(0, "BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 271, 110, 45, 25, hwnd, (HMENU)1004, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndHelpBtn   = CreateWindowEx(0, "BUTTON", "Help", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 319, 110, 45, 25, hwnd, (HMENU)1005, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hwndHintBtn   = CreateWindowEx(0, "BUTTON", "Hint (H)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 140, 75, 25, hwnd, (HMENU)1006, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndSlowBtn   = CreateWindowEx(0, "BUTTON", "Slow (S)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 90, 140, 75, 25, hwnd, (HMENU)1007, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndShieldBtn = CreateWindowEx(0, "BUTTON", "Shield (B)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 170, 140, 75, 25, hwnd, (HMENU)1008, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndFreezeBtn = CreateWindowEx(0, "BUTTON", "Freeze (F)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 250, 140, 75, 25, hwnd, (HMENU)1009, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            EnableWindow(hwndSaveBtn, FALSE);
            SetTimer(hwnd, TIMER_ANIM, 16, NULL); // 60 FPS animation
            break;
        case WM_COMMAND:
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == 1001) {
                current_mode = SendMessage(hwndModeBox, CB_GETCURSEL, 0, 0);
                RECT rc; GetClientRect(hwnd, &rc); LayoutButtons(rc.right, rc.bottom);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == 1002) {
                SaveGameState();
            } else if (LOWORD(wParam) == 1003) {
                LoadGameState();
            } else if (LOWORD(wParam) == 1004) {
                ResetStats();
            } else if (LOWORD(wParam) == 1005) {
                MessageBox(hwnd, "KSimon 3D Arcade - How to Play\n\n"
                                 "Observe the sequence of LED buttons & pitch tones and repeat it back.\n\n"
                                 "Controls:\n"
                                 "Mouse: Click colored buttons.\n"
                                 "Keyboard: Q,W,E,R (Top) / A,S,D,F (Bottom) or 1-8 keys.\n"
                                 "Space: Start game.\n"
                                 "H: Sequence Replay Hint (Replays sequence at slow speed).\n"
                                 "S / L: Slow-Motion Flash (Halves flash playback speed).\n"
                                 "B / J: Strike Shield (Protects against 1 wrong button).\n"
                                 "F / T: Time Freeze (Pauses stage input countdown).\n\n"
                                 "Modes:\n"
                                 "- 4-Button Classic / 6-Button Hex / 8-Button Octo\n"
                                 "- Chaos Reverse: Random pitches/speeds + Reverse playback!\n"
                                 "- Pitch Audio: Sound-only memory training without color flash hints!\n"
                                 "- Campaign: 20 stages culminating in Stage 20 Grandmaster Challenge!", "Help / How-to-Play", MB_OK | MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == 1006) {
                UseHint();
            } else if (LOWORD(wParam) == 1007) {
                UseSlowmo();
            } else if (LOWORD(wParam) == 1008) {
                UseShield();
            } else if (LOWORD(wParam) == 1009) {
                UseFreeze();
            }
            break;
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            LayoutButtons(width, height);
            break;
        }
        case WM_LBUTTONDOWN: {
            if (is_playing_sequence) break;
            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            int num_btns = GetActiveButtonCount();
            for (int i = 0; i < num_btns; i++) {
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
            } else if (wParam == 'L') {
                UseSlowmo();
            } else if (wParam == 'B' || wParam == 'J') {
                UseShield();
            } else if (wParam == 'T') {
                UseFreeze();
            } else if (!is_playing_sequence && sequence_length > 0) {
                int num_btns = GetActiveButtonCount();
                if (num_btns == 4) {
                    if (wParam == 'Q' || wParam == '1') HandleClick(0);
                    else if (wParam == 'W' || wParam == '2') HandleClick(1);
                    else if (wParam == 'A' || wParam == '3') HandleClick(2);
                    else if (wParam == 'S' || wParam == '4') HandleClick(3);
                } else if (num_btns == 6) {
                    if (wParam == 'Q' || wParam == '1') HandleClick(0);
                    else if (wParam == 'W' || wParam == '2') HandleClick(1);
                    else if (wParam == 'E' || wParam == '3') HandleClick(2);
                    else if (wParam == 'A' || wParam == '4') HandleClick(3);
                    else if (wParam == 'S' || wParam == '5') HandleClick(4);
                    else if (wParam == 'D' || wParam == '6') HandleClick(5);
                } else {
                    if (wParam == 'Q' || wParam == '1') HandleClick(0);
                    else if (wParam == 'W' || wParam == '2') HandleClick(1);
                    else if (wParam == 'E' || wParam == '3') HandleClick(2);
                    else if (wParam == 'R' || wParam == '4') HandleClick(3);
                    else if (wParam == 'A' || wParam == '5') HandleClick(4);
                    else if (wParam == 'S' || wParam == '6') HandleClick(5);
                    else if (wParam == 'D' || wParam == '7') HandleClick(6);
                    else if (wParam == 'F' || wParam == '8') HandleClick(7);
                }
            }
            break;
        case WM_TIMER:
            if (wParam == TIMER_ANIM) {
                UpdateParticles();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == TIMER_COUNTDOWN) {
                if (!is_playing_sequence && sequence_length > 0 && !is_time_frozen) {
                    input_countdown--;
                    InvalidateRect(hwnd, NULL, FALSE);
                    if (input_countdown <= 0) {
                        if (shields_remaining > 0) {
                            shields_remaining--;
                            PlaySoundAsync(750, 300);
                            input_countdown = 100;
                            player_step = 0;
                            sprintf(status_text, "Time Expired! Shield Absorbed Error!");
                            InvalidateRect(hwndMain, NULL, FALSE);
                        } else {
                            TriggerGameOver();
                        }
                    }
                }
            } else if (wParam == TIMER_FLASH) {
                KillTimer(hwnd, TIMER_FLASH);
                flash_btn = -1;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == TIMER_GAME_OVER) {
                game_over_flash_count++;
                game_over_flash = game_over_flash_count % 2;
                InvalidateRect(hwnd, NULL, FALSE);
                if (game_over_flash_count >= 5) {
                    KillTimer(hwnd, TIMER_GAME_OVER);
                    game_over_flash = 0;
                    game_over_flash_count = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
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
                            if (current_mode == MODE_CHAOS_REV) {
                                is_reverse = 1;
                            } else if (current_mode == MODE_CAMPAIGN) {
                                int mod = campaign_stages[current_stage-1].modifier;
                                if (mod == 1 || mod == 3) is_reverse = 1;
                            }
                            if (is_reverse) {
                                strcpy(status_text, "Your Turn (REVERSE)!");
                            } else {
                                strcpy(status_text, "Your Turn!");
                            }
                            EnableWindow(hwndSaveBtn, TRUE);
                            InvalidateRect(hwndMain, NULL, FALSE);
                            SetTimer(hwnd, TIMER_COUNTDOWN, 100, NULL);
                        } else {
                            ActivateButton(sequence[current_flash_index++]);
                            InvalidateRect(hwndMain, NULL, FALSE);
                            int speed_factor = sequence_length - 1;
                            if (speed_factor < 0) speed_factor = 0;
                            
                            int f_dur = 400 - speed_factor * 10;
                            if (current_mode == MODE_SPEED) {
                                f_dur = 200 - speed_factor * 8;
                                if (f_dur < 80) f_dur = 80;
                            } else if (current_mode == MODE_CAMPAIGN) {
                                f_dur = campaign_stages[current_stage - 1].speed_ms - speed_factor * 4;
                                if (f_dur < 100) f_dur = 100;
                            } else if (current_mode == MODE_CHAOS || current_mode == MODE_CHAOS_REV) {
                                f_dur = 150 + rand() % 250;
                            } else {
                                if (f_dur < 150) f_dur = 150;
                            }

                            if (is_slowmo_active) f_dur *= 2;

                            int pitch = btn_freqs[flash_btn];
                            int isChaos = (current_mode == MODE_CHAOS || current_mode == MODE_CHAOS_REV || 
                                          (current_mode == MODE_CAMPAIGN && (campaign_stages[current_stage-1].modifier == 2 || campaign_stages[current_stage-1].modifier == 3)));
                            if (isChaos) {
                                pitch += (rand() % 160) - 80;
                            }

                            PlaySoundAsync(pitch, f_dur);
                            SetTimer(hwnd, TIMER_SEQUENCE, f_dur, NULL);
                        }
                    } else {
                        flash_btn = -1;
                        InvalidateRect(hwndMain, NULL, FALSE);
                        int speed_factor = sequence_length - 1;
                        if (speed_factor < 0) speed_factor = 0;
                        
                        int p_dur = 200 - speed_factor * 5;
                        if (current_mode == MODE_SPEED) {
                            p_dur = 100 - speed_factor * 4;
                            if (p_dur < 40) p_dur = 40;
                        } else if (current_mode == MODE_CAMPAIGN) {
                            p_dur = (campaign_stages[current_stage - 1].speed_ms / 2) - speed_factor * 2;
                            if (p_dur < 50) p_dur = 50;
                        } else if (current_mode == MODE_CHAOS || current_mode == MODE_CHAOS_REV) {
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
            
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
            HGDIOBJ hOldBm = SelectObject(hdcMem, hbmMem);

            DrawBoard(hdcMem, rcClient.right, rcClient.bottom);

            BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, hdcMem, 0, 0, SRCCOPY);
            SelectObject(hdcMem, hOldBm);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

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
        0, CLASS_NAME, "KSimon 3D Arcade",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        560, 620, NULL, NULL, hInstance, NULL
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
