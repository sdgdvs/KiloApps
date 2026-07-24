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

#define TIMER_SEQUENCE 1
#define TIMER_FLASH    2
#define TIMER_GAME_OVER 3
#define TIMER_ANIM     4

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
    RGB(0, 170, 50),   // Green
    RGB(200, 0, 0),    // Red
    RGB(210, 170, 0),  // Yellow
    RGB(0, 80, 210),   // Blue
    RGB(160, 0, 200),  // Purple
    RGB(0, 180, 190)   // Cyan
};
COLORREF flash_colors[6] = {
    RGB(50, 255, 100),
    RGB(255, 70, 70),
    RGB(255, 255, 80),
    RGB(60, 160, 255),
    RGB(240, 80, 255),
    RGB(60, 240, 255)
};

char status_text[128] = "Press Space to Start";
int score = 0;
int high_scores[NUM_MODES] = {0, 0, 0, 0, 0, 0};
int stat_games_played = 0;
int stat_longest_streak = 0;
int stat_best_time = 0;
time_t start_time = 0;

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
    if (btn_idx < 0 || btn_idx >= 6) return;
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
    COLORREF colors[6] = {RGB(0, 255, 200), RGB(255, 50, 100), RGB(255, 255, 50), RGB(50, 150, 255), RGB(220, 50, 255), RGB(50, 255, 100)};
    for (int i = 0; i < MAX_SPARKS; i++) {
        float angle = ((float)rand() / RAND_MAX) * 6.28318f;
        float speed = 2.0f + ((float)rand() / RAND_MAX) * 6.0f;
        spark_particles[i].x = (float)cx;
        spark_particles[i].y = (float)cy;
        spark_particles[i].vx = cosf(angle) * speed;
        spark_particles[i].vy = sinf(angle) * speed - 1.5f;
        spark_particles[i].life = 1.0f;
        spark_particles[i].decay = 0.02f + ((float)rand() / RAND_MAX) * 0.02f;
        spark_particles[i].color = colors[rand() % 6];
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
}

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
        // 🎼 Treble Clef / Note
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
    }

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);
}

void DrawBoard(HDC hdc, int width, int height) {
    // 1. Background Fill with game over flash state
    COLORREF bgColor = game_over_flash ? RGB(140, 0, 0) : RGB(18, 18, 24);
    HBRUSH bgBrush = CreateSolidBrush(bgColor);
    RECT fullRc = {0, 0, width, height};
    FillRect(hdc, &fullRc, bgBrush);
    DeleteObject(bgBrush);

    // 2. Center 3D Circular Arcade Console Base
    int cx = width / 2;
    int cy = (height + 170) / 2;
    int consoleRadius = 200;

    // Outer Metallic Chrome Rim
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

    // 8 Metallic Screws around Rim
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

    // 3. Render Sound Wave Ripple Rings
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

    // 4. Render 6 Glossy 3D Quadrant LED Buttons
    const char* keyLabels[6] = {"Q", "W", "E", "A", "S", "D"};
    for (int i = 0; i < 6; i++) {
        RECT r = btn_rects[i];
        int isFlash = (flash_btn == i);
        if (isFlash) {
            InflateRect(&r, 3, 3);
        }

        // Outer Dark Bezel
        HBRUSH bezelBrush = CreateSolidBrush(RGB(10, 10, 15));
        RECT rBezel = r;
        InflateRect(&rBezel, 2, 2);
        FillRect(hdc, &rBezel, bezelBrush);
        DeleteObject(bezelBrush);

        // Neon Glow Halo on Flash
        if (isFlash) {
            COLORREF gCol = flash_colors[i];
            for (int g = 8; g >= 1; g -= 2) {
                RECT rGlow = r;
                InflateRect(&rGlow, g, g);
                HPEN glowPen = CreatePen(PS_SOLID, 2, gCol);
                HGDIOBJ oldP = SelectObject(hdc, glowPen);
                HGDIOBJ oldB = SelectObject(hdc, GetStockObject(NULL_BRUSH));
                RoundRect(hdc, rGlow.left, rGlow.top, rGlow.right, rGlow.bottom, 16, 16);
                SelectObject(hdc, oldP);
                SelectObject(hdc, oldB);
                DeleteObject(glowPen);
            }
        }

        // Glossy Button Body Fill
        COLORREF cFill = isFlash ? flash_colors[i] : btn_colors[i];
        HBRUSH btnBrush = CreateSolidBrush(cFill);
        FillRect(hdc, &r, btnBrush);
        DeleteObject(btnBrush);

        // 3D Bevel Top-Left Glass Specular Line & Bottom Drop Edge
        HPEN hiPen = CreatePen(PS_SOLID, 2, isFlash ? RGB(255, 255, 255) : RGB(220, 220, 220));
        HGDIOBJ oldP = SelectObject(hdc, hiPen);
        MoveToEx(hdc, r.left, r.bottom - 2, NULL);
        LineTo(hdc, r.left, r.top);
        LineTo(hdc, r.right - 2, r.top);

        HPEN shPen = CreatePen(PS_SOLID, 2, RGB(20, 20, 30));
        SelectObject(hdc, shPen);
        LineTo(hdc, r.right - 2, r.bottom - 2);
        LineTo(hdc, r.left, r.bottom - 2);
        SelectObject(hdc, oldP);
        DeleteObject(hiPen);
        DeleteObject(shPen);

        // Inner Vector Icon
        int bCx = (r.left + r.right) / 2;
        int bCy = (r.top + r.bottom) / 2 - 4;
        DrawButtonIcon(hdc, i, bCx, bCy, isFlash ? RGB(255, 255, 255) : RGB(230, 230, 230));

        // Keycap Label Text
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, isFlash ? RGB(255, 255, 255) : RGB(180, 180, 180));
        TextOutA(hdc, r.left + 6, r.bottom - 18, keyLabels[i], 1);
    }

    // 5. Center Metal Control Disc & LED Digital Score Display
    int discR = 45;
    HBRUSH discB = CreateSolidBrush(RGB(42, 44, 54));
    HGDIOBJ oldB = SelectObject(hdc, discB);
    Ellipse(hdc, cx - discR, cy - discR, cx + discR, cy + discR);
    SelectObject(hdc, oldB);
    DeleteObject(discB);

    HPEN dPen = CreatePen(PS_SOLID, 2, RGB(100, 100, 120));
    HGDIOBJ oldP = SelectObject(hdc, dPen);
    HGDIOBJ oldBr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Ellipse(hdc, cx - discR, cy - discR, cx + discR, cy + discR);
    SelectObject(hdc, oldP);
    SelectObject(hdc, oldBr);
    DeleteObject(dPen);

    // Dark Green Glass Digital LED Display Box
    RECT ledRc = {cx - 30, cy - 18, cx + 30, cy + 6};
    HBRUSH ledB = CreateSolidBrush(RGB(6, 22, 14));
    FillRect(hdc, &ledRc, ledB);
    DeleteObject(ledB);

    // Digital LED Score Text
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 255, 204));
    HFONT ledFont = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
    HGDIOBJ oldF = SelectObject(hdc, ledFont);
    char scoreStr[16];
    sprintf(scoreStr, "%02d", score);
    DrawTextA(hdc, scoreStr, -1, &ledRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldF);
    DeleteObject(ledFont);

    // Power Indicator LED Dot
    HBRUSH pwrB = CreateSolidBrush(is_playing_sequence ? RGB(255, 255, 0) : (sequence_length > 0 ? RGB(0, 255, 0) : RGB(0, 136, 204)));
    oldB = SelectObject(hdc, pwrB);
    Ellipse(hdc, cx - 4, cy + 12, cx + 4, cy + 20);
    SelectObject(hdc, oldB);
    DeleteObject(pwrB);

    // "K-SIMON" Silver Label
    SetTextColor(hdc, RGB(150, 150, 160));
    TextOutA(hdc, cx - 22, cy + 24, "K-SIMON", 7);

    // 6. Celebration Fireworks & Error Shards Particles
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

    // 7. HUD Text Overlay
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
    sprintf(pwr_text, "Hints (H): %d | Slow (F): %d | Shields (J): %d", hints_remaining, slowmo_remaining, shields_remaining);
    SetTextColor(hdc, RGB(255, 215, 0));
    TextOutA(hdc, 10, 62, pwr_text, strlen(pwr_text));

    if (current_mode == MODE_CAMPAIGN) {
        char stage_text[128];
        const char* mod_str = "Normal";
        if (campaign_stages[current_stage-1].modifier == 1) mod_str = "REVERSE";
        else if (campaign_stages[current_stage-1].modifier == 2) mod_str = "CHAOS";
        sprintf(stage_text, "Campaign Stage: %d/15 [%s] (Target Len: %d)", 
                current_stage, mod_str, campaign_stages[current_stage-1].target_len);
        SetTextColor(hdc, RGB(0, 255, 204));
        TextOutA(hdc, 10, 80, stage_text, strlen(stage_text));
    } else if (current_mode == MODE_CHAOS) {
        SetTextColor(hdc, RGB(255, 100, 255));
        TextOutA(hdc, 10, 80, "Mode: CHAOS - Unpredictable speeds & pitches!", 45);
    } else if (current_mode == MODE_REVERSE) {
        SetTextColor(hdc, RGB(255, 200, 100));
        TextOutA(hdc, 10, 80, "Mode: REVERSE - Repeat sequence backwards!", 42);
    } else {
        SetTextColor(hdc, RGB(180, 180, 180));
        TextOutA(hdc, 10, 80, "Controls: Q,W,E / A,S,D or 1-6 keys", 35);
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
    InvalidateRect(hwndMain, NULL, FALSE);
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

void HandleClick(int btn_id) {
    if (is_playing_sequence || sequence_length == 0) return;

    flash_btn = btn_id;
    TriggerSoundRipple(btn_id);
    EnableWindow(hwndSaveBtn, FALSE);
    InvalidateRect(hwndMain, NULL, FALSE);
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
            InvalidateRect(hwndMain, NULL, FALSE);
            return;
        }

        RECT rc;
        GetClientRect(hwndMain, &rc);
        TriggerErrorShards(rc.right / 2, rc.bottom / 2);
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
        return;
    }

    PlaySoundAsync(btn_freqs[btn_id], 200);

    player_step++;
    if (player_step == sequence_length) {
        if (current_mode == MODE_CAMPAIGN) {
            int target = campaign_stages[current_stage - 1].target_len;
            if (sequence_length >= target) {
                current_stage++;
                RECT rc;
                GetClientRect(hwndMain, &rc);
                TriggerVictoryFireworks(rc.right / 2, rc.bottom / 2);
                if (current_stage > 15) {
                    strcpy(status_text, "Campaign Complete! YOU WIN!");
                    is_playing_sequence = 1;
                    score += 100;
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
                10, 110, 130, 150, hwnd, (HMENU)1001, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Classic Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Reverse Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Speed Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Endless Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Campaign Mode");
            SendMessage(hwndModeBox, CB_ADDSTRING, 0, (LPARAM)"Chaos Mode");
            SendMessage(hwndModeBox, CB_SETCURSEL, 0, 0);

            hwndSaveBtn   = CreateWindowEx(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 145, 110, 50, 25, hwnd, (HMENU)1002, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndLoadBtn   = CreateWindowEx(0, "BUTTON", "Load", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 200, 110, 50, 25, hwnd, (HMENU)1003, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndResetBtn  = CreateWindowEx(0, "BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 255, 110, 50, 25, hwnd, (HMENU)1004, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndHelpBtn   = CreateWindowEx(0, "BUTTON", "Help", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 310, 110, 50, 25, hwnd, (HMENU)1005, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            hwndHintBtn   = CreateWindowEx(0, "BUTTON", "Hint (H)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 140, 75, 25, hwnd, (HMENU)1006, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndSlowBtn   = CreateWindowEx(0, "BUTTON", "Slow (F)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 90, 140, 75, 25, hwnd, (HMENU)1007, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            hwndShieldBtn = CreateWindowEx(0, "BUTTON", "Shield (J)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 170, 140, 75, 25, hwnd, (HMENU)1008, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            EnableWindow(hwndSaveBtn, FALSE);
            SetTimer(hwnd, TIMER_ANIM, 16, NULL); // 60 FPS animation timer
            break;
        case WM_COMMAND:
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == 1001) {
                current_mode = SendMessage(hwndModeBox, CB_GETCURSEL, 0, 0);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (LOWORD(wParam) == 1002) {
                SaveGameState();
            } else if (LOWORD(wParam) == 1003) {
                LoadGameState();
            } else if (LOWORD(wParam) == 1004) {
                ResetStats();
            } else if (LOWORD(wParam) == 1005) {
                MessageBox(hwnd, "KSimon 3D Arcade - How to Play\n\n"
                                 "Rules:\n"
                                 "Observe the pattern of glowing 3D LED buttons and repeat it back. The sequence grows each round.\n\n"
                                 "Controls:\n"
                                 "Mouse: Click colored buttons.\n"
                                 "Keyboard: Q, W, E (Top Row) / A, S, D (Bottom Row) or 1-6 keys.\n"
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
            int cy = (height + 170) / 2;
            int size = 76;
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
            if (wParam == TIMER_ANIM) {
                UpdateParticles();
                InvalidateRect(hwnd, NULL, FALSE);
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
                            InvalidateRect(hwndMain, NULL, FALSE);
                        } else {
                            flash_btn = sequence[current_flash_index++];
                            TriggerSoundRipple(flash_btn);
                            InvalidateRect(hwndMain, NULL, FALSE);
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
                        InvalidateRect(hwndMain, NULL, FALSE);
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
        540, 600, NULL, NULL, hInstance, NULL
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
