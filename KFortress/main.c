#include <windows.h>

int _fltused = 1;


float custom_sqrtf(float val) {
    if (val <= 0.0f) return 0.0f;
    float guess = val / 2.0f;
    for (int i = 0; i < 8; i++) {
        guess = 0.5f * (guess + val / guess);
    }
    return guess;
}

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TIMER_ID 1
#define TIMER_INTERVAL 33 // ~30 FPS

// Colors
#define BG_COLOR RGB(15, 19, 29)
#define CARD_BG RGB(26, 32, 44)
#define BORDER_COLOR RGB(51, 61, 82)
#define TEXT_GOLD RGB(255, 215, 0)
#define TEXT_WHITE RGB(241, 245, 249)
#define TEXT_MUTED RGB(148, 163, 184)
#define PATH_COLOR RGB(38, 46, 62)
#define PATH_BORDER RGB(71, 85, 105)
#define CASTLE_COLOR RGB(100, 116, 139)
#define TOWER_SLOT_BG RGB(30, 41, 59)
#define TOWER_SLOT_HOVER RGB(51, 65, 85)
#define GOBLIN_GREEN RGB(22, 163, 74)

#define MAX_SLOTS 12
#define MAX_ENEMIES 64
#define MAX_PROJECTILES 64
#define MAX_FLOATING_TEXTS 32
#define MAX_WAYPOINTS 6

typedef struct {
    int x, y;
} Point;

typedef struct {
    int x, y;
    BOOL occupied;
    int towerType; // 0 = none, 1 = archer
    int cooldown;
    int maxCooldown;
    int range;
    int damage;
} TowerSlot;

typedef struct {
    BOOL active;
    float x, y;
    int hp;
    int maxHp;
    float speed;
    int waypointIndex;
    int id;
} Enemy;

typedef struct {
    BOOL active;
    float x, y;
    int targetEnemyId;
    float targetX, targetY;
    float speed;
    int damage;
} Projectile;

typedef struct {
    BOOL active;
    float x, y;
    char text[32];
    COLORREF color;
    int life;
} FloatingText;

// Global State
static TowerSlot g_slots[MAX_SLOTS];
static int g_slotCount = 12;
static int g_gold = 100;
static int g_baseHp = 20;
static int g_maxBaseHp = 20;
static int g_wave = 1;
static BOOL g_waveActive = FALSE;
static BOOL g_gameOver = FALSE;
static int g_selectedSlot = -1;

static int g_goblinsToSpawn = 0;
static int g_spawnTimer = 0;

static Enemy g_enemies[MAX_ENEMIES];
static Projectile g_projectiles[MAX_PROJECTILES];
static FloatingText g_floatingTexts[MAX_FLOATING_TEXTS];

static Point g_waypoints[MAX_WAYPOINTS];
static int g_nextEnemyId = 1;

void AddFloatingText(float x, float y, const char* txt, COLORREF color) {
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) {
        if (!g_floatingTexts[i].active) {
            g_floatingTexts[i].active = TRUE;
            g_floatingTexts[i].x = x;
            g_floatingTexts[i].y = y;
            g_floatingTexts[i].color = color;
            g_floatingTexts[i].life = 25;
            lstrcpynA(g_floatingTexts[i].text, txt, 32);
            break;
        }
    }
}

void InitWaypoints(int bfX, int bfY, int bfW) {
    g_waypoints[0] = (Point){bfX + 30, bfY + 180};
    g_waypoints[1] = (Point){bfX + 280, bfY + 180};
    g_waypoints[2] = (Point){bfX + 280, bfY + 360};
    g_waypoints[3] = (Point){bfX + 480, bfY + 360};
    g_waypoints[4] = (Point){bfX + 480, bfY + 200};
    g_waypoints[5] = (Point){bfX + bfW - 60, bfY + 200};
}

void InitTowerSlots(int bfX, int bfY) {
    int relCoords[12][2] = {
        {120, 100}, {250, 100}, {380, 100}, {520, 100},
        {120, 270}, {250, 270}, {380, 270}, {520, 270},
        {120, 440}, {250, 440}, {380, 440}, {520, 440}
    };
    g_slotCount = 12;
    for (int i = 0; i < 12; i++) {
        g_slots[i].x = bfX + relCoords[i][0];
        g_slots[i].y = bfY + relCoords[i][1];
        g_slots[i].occupied = FALSE;
        g_slots[i].towerType = 0;
        g_slots[i].cooldown = 0;
        g_slots[i].maxCooldown = 18; // ~0.6s
        g_slots[i].range = 130;
        g_slots[i].damage = 12;
    }
}

void InitGameState() {
    g_gold = 100;
    g_baseHp = 20;
    g_maxBaseHp = 20;
    g_wave = 1;
    g_waveActive = FALSE;
    g_gameOver = FALSE;
    g_selectedSlot = -1;
    g_goblinsToSpawn = 0;
    g_spawnTimer = 0;

    for (int i = 0; i < MAX_ENEMIES; i++) g_enemies[i].active = FALSE;
    for (int i = 0; i < MAX_PROJECTILES; i++) g_projectiles[i].active = FALSE;
    for (int i = 0; i < MAX_FLOATING_TEXTS; i++) g_floatingTexts[i].active = FALSE;

    int bfX = 10, bfY = 70, bfW = WINDOW_WIDTH - 220;
    InitWaypoints(bfX, bfY, bfW);
    InitTowerSlots(bfX, bfY);
}

void UpdateGameLogic() {
    if (g_gameOver) return;

    // Spawning Logic
    if (g_waveActive && g_goblinsToSpawn > 0) {
        g_spawnTimer++;
        if (g_spawnTimer >= 35) { // ~1.1s
            g_spawnTimer = 0;
            g_goblinsToSpawn--;

            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!g_enemies[i].active) {
                    g_enemies[i].active = TRUE;
                    g_enemies[i].id = g_nextEnemyId++;
                    g_enemies[i].x = (float)g_waypoints[0].x;
                    g_enemies[i].y = (float)g_waypoints[0].y;
                    g_enemies[i].waypointIndex = 0;
                    g_enemies[i].hp = 25 + g_wave * 5;
                    g_enemies[i].maxHp = g_enemies[i].hp;
                    g_enemies[i].speed = 2.0f;
                    break;
                }
            }
        }
    }

    // Update Enemies (Pathfinding)
    int activeEnemyCount = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!g_enemies[i].active) continue;
        activeEnemyCount++;

        Point targetWP = g_waypoints[g_enemies[i].waypointIndex + 1];
        float dx = targetWP.x - g_enemies[i].x;
        float dy = targetWP.y - g_enemies[i].y;
        float dist = custom_sqrtf(dx * dx + dy * dy);

        if (dist < g_enemies[i].speed) {
            g_enemies[i].x = (float)targetWP.x;
            g_enemies[i].y = (float)targetWP.y;
            g_enemies[i].waypointIndex++;

            if (g_enemies[i].waypointIndex >= MAX_WAYPOINTS - 1) {
                // Reached Castle Fortress
                g_baseHp--;
                Beep(180, 60);
                AddFloatingText(g_enemies[i].x - 15, g_enemies[i].y - 20, "-1 HP", RGB(239, 68, 68));
                g_enemies[i].active = FALSE;

                if (g_baseHp <= 0) {
                    g_baseHp = 0;
                    g_gameOver = TRUE;
                    g_waveActive = FALSE;
                    Beep(120, 300);
                }
                continue;
            }
        } else {
            g_enemies[i].x += (dx / dist) * g_enemies[i].speed;
            g_enemies[i].y += (dy / dist) * g_enemies[i].speed;
        }
    }

    // Update Towers & Target Acquisition
    for (int i = 0; i < g_slotCount; i++) {
        if (!g_slots[i].occupied) continue;

        if (g_slots[i].cooldown > 0) {
            g_slots[i].cooldown--;
        } else {
            // Find target enemy in range
            int targetIdx = -1;
            float maxProgress = -1.0f;

            for (int e = 0; e < MAX_ENEMIES; e++) {
                if (!g_enemies[e].active) continue;

                float edx = g_enemies[e].x - g_slots[i].x;
                float edy = g_enemies[e].y - g_slots[i].y;
                float edist = custom_sqrtf(edx * edx + edy * edy);

                if (edist <= g_slots[i].range) {
                    float progress = g_enemies[e].waypointIndex * 1000.0f + edist;
                    if (progress > maxProgress) {
                        maxProgress = progress;
                        targetIdx = e;
                    }
                }
            }

            if (targetIdx != -1) {
                g_slots[i].cooldown = g_slots[i].maxCooldown;

                // Spawn Projectile
                for (int p = 0; p < MAX_PROJECTILES; p++) {
                    if (!g_projectiles[p].active) {
                        g_projectiles[p].active = TRUE;
                        g_projectiles[p].x = (float)g_slots[i].x;
                        g_projectiles[p].y = (float)g_slots[i].y;
                        g_projectiles[p].targetEnemyId = g_enemies[targetIdx].id;
                        g_projectiles[p].targetX = g_enemies[targetIdx].x;
                        g_projectiles[p].targetY = g_enemies[targetIdx].y;
                        g_projectiles[p].speed = 10.0f;
                        g_projectiles[p].damage = g_slots[i].damage;
                        Beep(850, 15);
                        break;
                    }
                }
            }
        }
    }

    // Update Projectiles
    for (int p = 0; p < MAX_PROJECTILES; p++) {
        if (!g_projectiles[p].active) continue;

        // Find target enemy
        int targetIdx = -1;
        for (int e = 0; e < MAX_ENEMIES; e++) {
            if (g_enemies[e].active && g_enemies[e].id == g_projectiles[p].targetEnemyId) {
                targetIdx = e;
                g_projectiles[p].targetX = g_enemies[e].x;
                g_projectiles[p].targetY = g_enemies[e].y;
                break;
            }
        }

        float dx = g_projectiles[p].targetX - g_projectiles[p].x;
        float dy = g_projectiles[p].targetY - g_projectiles[p].y;
        float dist = custom_sqrtf(dx * dx + dy * dy);

        if (dist < g_projectiles[p].speed) {
            // Hit target
            if (targetIdx != -1) {
                g_enemies[targetIdx].hp -= g_projectiles[p].damage;
                Beep(450, 15);

                if (g_enemies[targetIdx].hp <= 0) {
                    g_enemies[targetIdx].active = FALSE;
                    g_gold += 15;
                    AddFloatingText(g_enemies[targetIdx].x, g_enemies[targetIdx].y - 10, "+15g", TEXT_GOLD);
                    Beep(700, 25);
                }
            }
            g_projectiles[p].active = FALSE;
        } else {
            g_projectiles[p].x += (dx / dist) * g_projectiles[p].speed;
            g_projectiles[p].y += (dy / dist) * g_projectiles[p].speed;
        }
    }

    // Update Floating Texts
    for (int f = 0; f < MAX_FLOATING_TEXTS; f++) {
        if (!g_floatingTexts[f].active) continue;
        g_floatingTexts[f].y -= 0.8f;
        g_floatingTexts[f].life--;
        if (g_floatingTexts[f].life <= 0) {
            g_floatingTexts[f].active = FALSE;
        }
    }

    // Check Wave Completion
    if (g_waveActive && g_goblinsToSpawn == 0 && activeEnemyCount == 0) {
        g_waveActive = FALSE;
        int bonus = 20 + g_wave * 5;
        g_gold += bonus;

        char buf[32];
        wsprintfA(buf, "WAVE CLEAR! +%dg", bonus);
        AddFloatingText((float)(WINDOW_WIDTH / 2 - 50), (float)(WINDOW_HEIGHT / 2), buf, RGB(16, 185, 129));

        g_wave++;
        Beep(520, 40);
        Beep(650, 40);
        Beep(780, 60);
    }
}

void DrawRoundedRect(HDC hdc, int left, int top, int right, int bottom, COLORREF fillColor, COLORREF borderColor, int radius) {
    HBRUSH fillBrush = CreateSolidBrush(fillColor);
    HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, fillBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    RoundRect(hdc, left, top, right, bottom, radius, radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(fillBrush);
    DeleteObject(pen);
}

void Render(HDC hdc, HWND hwnd) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int w = clientRect.right;
    int h = clientRect.bottom;

    // Create Backbuffer
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // Background
    HBRUSH bgBrush = CreateSolidBrush(BG_COLOR);
    FillRect(memDC, &clientRect, bgBrush);
    DeleteObject(bgBrush);

    SetBkMode(memDC, TRANSPARENT);

    // Top Header / HUD Bar
    DrawRoundedRect(memDC, 10, 10, w - 10, 60, CARD_BG, BORDER_COLOR, 8);

    HFONT hFontTitle = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(memDC, hFontTitle);

    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, 25, 22, "KFORTRESS", 9);

    HFONT hFontSub = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontSub);
    SetTextColor(memDC, TEXT_MUTED);
    TextOutA(memDC, 140, 26, "Phase 3: Native C Parity", 24);

    // Stats HUD
    char buf[128];
    HFONT hFontStat = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontStat);

    SetTextColor(memDC, TEXT_GOLD);
    wsprintfA(buf, "Gold: %d", g_gold);
    TextOutA(memDC, w - 320, 24, buf, (int)lstrlenA(buf));

    SetTextColor(memDC, RGB(239, 68, 68));
    wsprintfA(buf, "Base HP: %d/%d", g_baseHp, g_maxBaseHp);
    TextOutA(memDC, w - 210, 24, buf, (int)lstrlenA(buf));

    SetTextColor(memDC, TEXT_WHITE);
    wsprintfA(buf, "Wave: %d", g_wave);
    TextOutA(memDC, w - 90, 24, buf, (int)lstrlenA(buf));

    DeleteObject(hFontTitle);
    DeleteObject(hFontSub);
    DeleteObject(hFontStat);

    // Battlefield Area
    int bfX = 10, bfY = 70, bfW = w - 220, bfH = h - 80;
    DrawRoundedRect(memDC, bfX, bfY, bfX + bfW, bfY + bfH, CARD_BG, BORDER_COLOR, 8);

    // Draw Winding Path
    POINT pts[MAX_WAYPOINTS];
    for (int i = 0; i < MAX_WAYPOINTS; i++) {
        pts[i].x = g_waypoints[i].x;
        pts[i].y = g_waypoints[i].y;
    }

    HPEN pathPen = CreatePen(PS_SOLID, 42, PATH_COLOR);
    HPEN oldPen = (HPEN)SelectObject(memDC, pathPen);
    Polyline(memDC, pts, MAX_WAYPOINTS);

    HPEN pathBorderPen = CreatePen(PS_SOLID, 2, PATH_BORDER);
    SelectObject(memDC, pathBorderPen);
    Polyline(memDC, pts, MAX_WAYPOINTS);
    SelectObject(memDC, oldPen);
    DeleteObject(pathPen);
    DeleteObject(pathBorderPen);

    // Draw Range Circle for selected slot
    if (g_selectedSlot != -1) {
        HPEN rangePen = CreatePen(PS_DOT, 1, TEXT_GOLD);
        HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HPEN oP = (HPEN)SelectObject(memDC, rangePen);
        HBRUSH oB = (HBRUSH)SelectObject(memDC, nullBrush);

        int sx = g_slots[g_selectedSlot].x;
        int sy = g_slots[g_selectedSlot].y;
        int r = g_slots[g_selectedSlot].range;
        Ellipse(memDC, sx - r, sy - r, sx + r, sy + r);

        SelectObject(memDC, oP);
        SelectObject(memDC, oB);
        DeleteObject(rangePen);
    }

    // Draw Spawn Gate
    DrawRoundedRect(memDC, g_waypoints[0].x - 22, g_waypoints[0].y - 22, g_waypoints[0].x + 22, g_waypoints[0].y + 22, RGB(180, 40, 40), RGB(239, 68, 68), 6);
    SelectObject(memDC, oldFont);
    SetTextColor(memDC, TEXT_WHITE);
    TextOutA(memDC, g_waypoints[0].x - 14, g_waypoints[0].y - 6, "GATE", 4);

    // Draw Castle Fortress Base
    DrawRoundedRect(memDC, g_waypoints[5].x - 30, g_waypoints[5].y - 30, g_waypoints[5].x + 35, g_waypoints[5].y + 35, CASTLE_COLOR, TEXT_GOLD, 8);
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, g_waypoints[5].x - 24, g_waypoints[5].y - 8, "CASTLE", 6);

    // Draw Tower Slots
    for (int i = 0; i < g_slotCount; i++) {
        COLORREF fill = (g_selectedSlot == i) ? TOWER_SLOT_HOVER : TOWER_SLOT_BG;
        COLORREF border = (g_selectedSlot == i) ? TEXT_GOLD : BORDER_COLOR;
        DrawRoundedRect(memDC, g_slots[i].x - 22, g_slots[i].y - 22, g_slots[i].x + 22, g_slots[i].y + 22, fill, border, 6);

        if (g_slots[i].occupied) {
            // Archer Tower
            HBRUSH tBrush = CreateSolidBrush(CASTLE_COLOR);
            HPEN tPen = CreatePen(PS_SOLID, 2, TEXT_GOLD);
            HBRUSH oB = (HBRUSH)SelectObject(memDC, tBrush);
            HPEN oP = (HPEN)SelectObject(memDC, tPen);

            Ellipse(memDC, g_slots[i].x - 14, g_slots[i].y - 14, g_slots[i].x + 14, g_slots[i].y + 14);

            SelectObject(memDC, oB);
            SelectObject(memDC, oP);
            DeleteObject(tBrush);
            DeleteObject(tPen);

            SetTextColor(memDC, TEXT_GOLD);
            TextOutA(memDC, g_slots[i].x - 9, g_slots[i].y - 8, "[A]", 3);
        } else {
            SetTextColor(memDC, TEXT_MUTED);
            TextOutA(memDC, g_slots[i].x - 4, g_slots[i].y - 12, "+", 1);
            SetTextColor(memDC, TEXT_GOLD);
            HFONT hSmallFont = CreateFontA(10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            SelectObject(memDC, hSmallFont);
            TextOutA(memDC, g_slots[i].x - 9, g_slots[i].y + 2, "50g", 3);
            DeleteObject(hSmallFont);
        }
    }

    // Draw Enemies (Goblins)
    for (int e = 0; e < MAX_ENEMIES; e++) {
        if (!g_enemies[e].active) continue;

        int ex = (int)g_enemies[e].x;
        int ey = (int)g_enemies[e].y;

        HBRUSH gobBrush = CreateSolidBrush(GOBLIN_GREEN);
        HPEN gobPen = CreatePen(PS_SOLID, 1, RGB(21, 128, 61));
        HBRUSH oB = (HBRUSH)SelectObject(memDC, gobBrush);
        HPEN oP = (HPEN)SelectObject(memDC, gobPen);

        Ellipse(memDC, ex - 11, ey - 11, ex + 11, ey + 11);

        SelectObject(memDC, oB);
        SelectObject(memDC, oP);
        DeleteObject(gobBrush);
        DeleteObject(gobPen);

        // HP Bar overhead
        int barW = 24;
        int barH = 4;
        float hpRatio = (float)g_enemies[e].hp / (float)g_enemies[e].maxHp;
        if (hpRatio < 0.0f) hpRatio = 0.0f;

        DrawRoundedRect(memDC, ex - barW / 2, ey - 18, ex + barW / 2, ey - 18 + barH, RGB(20, 20, 20), RGB(0, 0, 0), 2);
        COLORREF hpColor = hpRatio > 0.5f ? RGB(34, 197, 94) : (hpRatio > 0.25f ? RGB(234, 179, 8) : RGB(239, 68, 68));
        HBRUSH hpBrush = CreateSolidBrush(hpColor);
        RECT hpR = { ex - barW / 2, ey - 18, ex - barW / 2 + (int)(barW * hpRatio), ey - 18 + barH };
        FillRect(memDC, &hpR, hpBrush);
        DeleteObject(hpBrush);
    }

    // Draw Projectiles (Arrows)
    for (int p = 0; p < MAX_PROJECTILES; p++) {
        if (!g_projectiles[p].active) continue;

        HPEN arrowPen = CreatePen(PS_SOLID, 2, TEXT_GOLD);
        HPEN oP = (HPEN)SelectObject(memDC, arrowPen);

        MoveToEx(memDC, (int)g_projectiles[p].x, (int)g_projectiles[p].y, NULL);
        LineTo(memDC, (int)g_projectiles[p].x + 4, (int)g_projectiles[p].y + 4);

        SelectObject(memDC, oP);
        DeleteObject(arrowPen);
    }

    // Draw Floating Text
    for (int f = 0; f < MAX_FLOATING_TEXTS; f++) {
        if (!g_floatingTexts[f].active) continue;
        SetTextColor(memDC, g_floatingTexts[f].color);
        TextOutA(memDC, (int)g_floatingTexts[f].x, (int)g_floatingTexts[f].y, g_floatingTexts[f].text, (int)lstrlenA(g_floatingTexts[f].text));
    }

    // Game Over Overlay Banner
    if (g_gameOver) {
        DrawRoundedRect(memDC, bfX + 50, bfY + 180, bfX + bfW - 50, bfY + 300, RGB(20, 24, 33), RGB(239, 68, 68), 12);
        HFONT hFontGO = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        SelectObject(memDC, hFontGO);
        SetTextColor(memDC, RGB(239, 68, 68));
        TextOutA(memDC, bfX + 180, bfY + 205, "DEFENSE FALLEN - GAME OVER!", 27);

        HFONT hFontSubGO = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        SelectObject(memDC, hFontSubGO);
        SetTextColor(memDC, TEXT_WHITE);
        TextOutA(memDC, bfX + 210, bfY + 245, "Click RESET DEFENSE to play again.", 34);

        DeleteObject(hFontGO);
        DeleteObject(hFontSubGO);
    }

    // Right Control Sidebar
    int sbX = w - 200, sbY = 70, sbW = 190, sbH = h - 80;
    DrawRoundedRect(memDC, sbX, sbY, sbX + sbW, sbY + sbH, CARD_BG, BORDER_COLOR, 8);

    HFONT hFontHeader = CreateFontA(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontHeader);
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, sbX + 15, sbY + 15, "COMMAND POST", 12);

    // Shop Info
    DrawRoundedRect(memDC, sbX + 15, sbY + 45, sbX + sbW - 15, sbY + 105, RGB(30, 41, 59), TEXT_GOLD, 6);
    SetTextColor(memDC, TEXT_WHITE);
    TextOutA(memDC, sbX + 25, sbY + 52, "Archer Tower [A]", 16);
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, sbX + 25, sbY + 68, "Cost: 50 Gold", 13);
    SetTextColor(memDC, TEXT_MUTED);
    TextOutA(memDC, sbX + 25, sbY + 84, "Dmg: 12 | Range: 130", 20);

    // Button: Start Wave
    COLORREF btnBg = g_waveActive ? RGB(60, 70, 85) : RGB(16, 185, 129);
    DrawRoundedRect(memDC, sbX + 15, sbY + 120, sbX + sbW - 15, sbY + 165, btnBg, BORDER_COLOR, 6);
    SetTextColor(memDC, TEXT_WHITE);
    SelectObject(memDC, hFontHeader);
    wsprintfA(buf, g_waveActive ? "WAVE IN PROGRESS" : "START WAVE %d", g_wave);
    TextOutA(memDC, sbX + 25, sbY + 134, buf, (int)lstrlenA(buf));

    // Button: Reset Game
    DrawRoundedRect(memDC, sbX + 15, sbY + 180, sbX + sbW - 15, sbY + 220, RGB(225, 29, 72), BORDER_COLOR, 6);
    TextOutA(memDC, sbX + 35, sbY + 192, "RESET DEFENSE", 13);

    // Info Box
    DrawRoundedRect(memDC, sbX + 15, sbY + 235, sbX + sbW - 15, sbY + sbH - 15, BG_COLOR, BORDER_COLOR, 6);
    HFONT hFontBody = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, hFontBody);
    SetTextColor(memDC, TEXT_GOLD);
    TextOutA(memDC, sbX + 25, sbY + 245, "Phase 3 Rules:", 14);
    SetTextColor(memDC, TEXT_MUTED);
    TextOutA(memDC, sbX + 25, sbY + 268, "- Click (+) slots to", 20);
    TextOutA(memDC, sbX + 25, sbY + 284, "  build Archer (50g)", 20);
    TextOutA(memDC, sbX + 25, sbY + 304, "- Goblins yield +15g", 20);
    TextOutA(memDC, sbX + 25, sbY + 324, "- Defend Castle HP!", 19);

    DeleteObject(hFontHeader);
    DeleteObject(hFontBody);

    // Copy Backbuffer to Window DC
    BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

    // Cleanup
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        InitGameState();
        SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
        break;

    case WM_TIMER:
        UpdateGameLogic();
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        int w = clientRect.right;
        int h = clientRect.bottom;
        int sbX = w - 200, sbY = 70, sbW = 190;

        // Check button clicks
        // Start Wave
        if (x >= sbX + 15 && x <= sbX + sbW - 15 && y >= sbY + 120 && y <= sbY + 165) {
            if (!g_waveActive && !g_gameOver) {
                g_waveActive = TRUE;
                g_goblinsToSpawn = 5 + g_wave * 3;
                g_spawnTimer = 0;
                Beep(600, 40);
            }
        }
        // Reset Defense
        else if (x >= sbX + 15 && x <= sbX + sbW - 15 && y >= sbY + 180 && y <= sbY + 220) {
            InitGameState();
            Beep(300, 60);
        }
        // Check slot clicks
        else if (!g_gameOver) {
            g_selectedSlot = -1;
            for (int i = 0; i < g_slotCount; i++) {
                int dx = x - g_slots[i].x;
                int dy = y - g_slots[i].y;
                if (dx * dx + dy * dy <= 22 * 22) {
                    g_selectedSlot = i;

                    if (!g_slots[i].occupied) {
                        if (g_gold >= 50) {
                            g_gold -= 50;
                            g_slots[i].occupied = TRUE;
                            g_slots[i].towerType = 1;
                            AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), "-50g", RGB(239, 68, 68));
                            Beep(800, 40);
                        } else {
                            AddFloatingText((float)g_slots[i].x, (float)(g_slots[i].y - 20), "NEED 50G!", RGB(239, 68, 68));
                            Beep(200, 100);
                        }
                    } else {
                        Beep(500, 30);
                    }
                    break;
                }
            }
        }
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Render(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_ERASEBKGND:
        return 1; // Prevent flicker

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KFortressWindowClass";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "KFortress - Fantasy Tower Defense & Siege Defense",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
