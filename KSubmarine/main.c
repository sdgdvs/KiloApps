#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIMER_ID 1
#define TIMER_INTERVAL 50 // ms

// Control IDs
#define ID_BTN_FLOOD_BALLAST    101
#define ID_BTN_BLOW_BALLAST     102
#define ID_BTN_TRIM_BOW         103
#define ID_BTN_TRIM_STERN       104
#define ID_BTN_SONAR_PING       105
#define ID_BTN_THROTTLE_REV     106
#define ID_BTN_THROTTLE_STOP    107
#define ID_BTN_THROTTLE_HALF    108
#define ID_BTN_THROTTLE_FLANK   109
#define ID_BTN_RUDDER_PORT      110
#define ID_BTN_RUDDER_STBD      111
#define ID_BTN_SEARCHLIGHTS     112
#define ID_BTN_SCRUBBER         113
#define ID_BTN_O2_PURGE         114
#define ID_BTN_BILGE_PUMP       115
#define ID_BTN_LOW_POWER        116
#define ID_BTN_EMERGENCY_BLOW   117
#define ID_BTN_SOUND_TOGGLE     118

// Colors
#define CLR_BG_DEEP       RGB(2, 11, 18)
#define CLR_BG_PANEL      RGB(7, 23, 36)
#define CLR_BG_HEADER     RGB(13, 34, 53)
#define CLR_BORDER_PANEL  RGB(19, 60, 90)
#define CLR_BORDER_GLOW   RGB(0, 210, 255)
#define CLR_TEXT_CYAN     RGB(56, 189, 248)
#define CLR_TEXT_BRIGHT   RGB(224, 242, 254)
#define CLR_TEXT_DIM      RGB(2, 132, 199)
#define CLR_ACCENT_SONAR  RGB(0, 240, 255)
#define CLR_ACCENT_EMERALD RGB(16, 185, 129)
#define CLR_ACCENT_AMBER  RGB(245, 158, 11)
#define CLR_ACCENT_RED    RGB(239, 68, 68)
#define CLR_GAUGE_BG      RGB(3, 16, 28)
#define CLR_BTN_BG        RGB(12, 36, 56)
#define CLR_BTN_ACTIVE    RGB(2, 132, 199)
#define CLR_BLACK         RGB(1, 6, 10)

typedef struct {
    float angle; // radians
    float dist;  // 0.0 - 1.0 (normalized to sonar radius)
    char label[32];
    int type;    // 0: terrain, 1: fauna, 2: wreck, 3: vent
} SonarContact;

typedef struct {
    char time[12];
    char text[128];
    COLORREF color;
} LogEntry;

#define MAX_LOGS 16

typedef struct {
    float depth;            // meters (0 - 11000)
    float vertRate;         // m/s
    float targetVertRate;
    float speed;            // knots (-2.5 to 11.5)
    float targetSpeed;
    int throttleMode;       // 0: REV, 1: STOP, 2: HALF, 3: FLANK
    float heading;          // degrees (0 - 359)
    float pitch;            // degrees (-15 to +15)

    // Vital systems
    float hull;             // 0 - 100%
    float crushDepth;       // 4500m
    float pressure;         // atm
    float hullStress;       // 0 - 100%

    // Life support
    float o2;               // 0 - 100%
    float co2;              // %
    float scrubberStatus;   // 0 - 100%
    int scrubberAuto;
    int o2PurgeCount;

    // Power & Electrical
    float battery;          // 0 - 100%
    float powerDrain;       // kW
    int lowPowerMode;
    int searchlights;

    // Ballast & Bilge
    float ballast;          // 0 - 100% (0 = surface, 45 = neutral, 100 = heavy)
    float airReservoir;     // 0 - 300 BAR
    float bilgeWater;       // gallons
    int bilgePumpActive;
    float waterIntrusionRate; // GPM

    // Environment
    float temp;             // Celsius

    // Sonar
    int isPinging;
    float pingRadius;
    float sweepAngle;
    SonarContact contacts[4];
    int contactCount;

    // Sound
    int soundEnabled;

    // Log
    LogEntry logs[MAX_LOGS];
    int logCount;
} SubmarineState;

static SubmarineState g_sub;
static HWND g_hWnd = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontNormal = NULL;
static HFONT g_hFontSmall = NULL;
static HFONT g_hFontBold = NULL;

void PlaySoundAsync(DWORD freq, DWORD duration);
void AddLog(const char* text, COLORREF color);
void InitSubmarineState(void);
void UpdateSimulation(float dt);
void DrawUI(HDC hdc, RECT* rcClient);

DWORD WINAPI SoundThreadProc(LPVOID lpParam) {
    DWORD packed = (DWORD)(UINT_PTR)lpParam;
    DWORD freq = LOWORD(packed);
    DWORD dur = HIWORD(packed);
    Beep(freq, dur);
    return 0;
}

void PlaySoundAsync(DWORD freq, DWORD duration) {
    if (!g_sub.soundEnabled) return;
    DWORD packed = MAKELONG((WORD)freq, (WORD)duration);
    CreateThread(NULL, 0, SoundThreadProc, (LPVOID)(UINT_PTR)packed, 0, NULL);
}

void AddLog(const char* text, COLORREF color) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    if (g_sub.logCount >= MAX_LOGS) {
        for (int i = 0; i < MAX_LOGS - 1; i++) {
            g_sub.logs[i] = g_sub.logs[i + 1];
        }
        g_sub.logCount = MAX_LOGS - 1;
    }
    
    LogEntry* e = &g_sub.logs[g_sub.logCount++];
    snprintf(e->time, sizeof(e->time), "[%02d:%02d:%02d]", st.wHour, st.wMinute, st.wSecond);
    strncpy(e->text, text, sizeof(e->text) - 1);
    e->text[sizeof(e->text) - 1] = '\0';
    e->color = color;
}

void InitSubmarineState(void) {
    memset(&g_sub, 0, sizeof(g_sub));
    g_sub.depth = 0.0f;
    g_sub.vertRate = 0.0f;
    g_sub.targetVertRate = 0.0f;
    g_sub.speed = 0.0f;
    g_sub.targetSpeed = 0.0f;
    g_sub.throttleMode = 1; // STOP
    g_sub.heading = 42.0f;
    g_sub.pitch = 0.0f;

    g_sub.hull = 100.0f;
    g_sub.crushDepth = 4500.0f;
    g_sub.pressure = 1.0f;
    g_sub.hullStress = 0.1f;

    g_sub.o2 = 100.0f;
    g_sub.co2 = 0.04f;
    g_sub.scrubberStatus = 99.0f;
    g_sub.scrubberAuto = 1;
    g_sub.o2PurgeCount = 3;

    g_sub.battery = 100.0f;
    g_sub.powerDrain = 0.4f;
    g_sub.lowPowerMode = 0;
    g_sub.searchlights = 0;

    g_sub.ballast = 0.0f;
    g_sub.airReservoir = 300.0f;
    g_sub.bilgeWater = 0.0f;
    g_sub.bilgePumpActive = 0;
    g_sub.waterIntrusionRate = 0.0f;

    g_sub.temp = 21.4f;

    g_sub.isPinging = 0;
    g_sub.pingRadius = 0.0f;
    g_sub.sweepAngle = 0.0f;
    g_sub.soundEnabled = 1;

    g_sub.contactCount = 4;
    g_sub.contacts[0] = (SonarContact){ 0.8f, 0.55f, "Volcanic Ridge", 0 };
    g_sub.contacts[1] = (SonarContact){ 2.3f, 0.38f, "Megamouth Echo", 1 };
    g_sub.contacts[2] = (SonarContact){ 4.2f, 0.72f, "Derelict Probe", 2 };
    g_sub.contacts[3] = (SonarContact){ 5.6f, 0.85f, "Hydrothermal Smoker", 3 };

    g_sub.logCount = 0;
    AddLog("DSV Abyss Voyager Bathyscaphe computer online. Systems nominal.", CLR_TEXT_CYAN);
    AddLog("High-frequency hydrophones active. Epipelagic layer baseline calibrated.", CLR_ACCENT_EMERALD);
}

const char* GetZoneName(float depth) {
    if (depth < 200.0f) return "EPIPELAGIC (0-200M)";
    if (depth < 1000.0f) return "MESOPELAGIC (200-1000M)";
    if (depth < 4000.0f) return "BATHYPELAGIC (1000-4000M)";
    if (depth < 6000.0f) return "ABYSSOPELAGIC (4000-6000M)";
    return "HADALPELAGIC (6000M+)";
}

void UpdateSimulation(float dt) {
    float neutralBallast = 45.0f;
    float buoyancyForce = (neutralBallast - g_sub.ballast) * 0.4f;
    float pitchDescent = (g_sub.pitch / 15.0f) * (fabsf(g_sub.speed) * 0.3f);

    g_sub.targetVertRate = -buoyancyForce - pitchDescent;
    g_sub.vertRate += (g_sub.targetVertRate - g_sub.vertRate) * (dt * 1.5f);

    g_sub.depth += g_sub.vertRate * dt;
    if (g_sub.depth <= 0.0f) {
        g_sub.depth = 0.0f;
        if (g_sub.vertRate < 0.0f) g_sub.vertRate = 0.0f;
        if (g_sub.airReservoir < 300.0f) g_sub.airReservoir = min(300.0f, g_sub.airReservoir + dt * 5.0f);
        if (g_sub.battery < 100.0f) g_sub.battery = min(100.0f, g_sub.battery + dt * 2.0f);
    }
    if (g_sub.depth > 11000.0f) g_sub.depth = 11000.0f;

    g_sub.speed += (g_sub.targetSpeed - g_sub.speed) * (dt * 0.8f);

    g_sub.pressure = 1.0f + (g_sub.depth * 0.0995f);
    g_sub.hullStress = min(100.0f, (g_sub.depth / g_sub.crushDepth) * 100.0f);

    if (g_sub.depth > g_sub.crushDepth) {
        float excess = g_sub.depth - g_sub.crushDepth;
        float hullDamage = (excess * 0.02f + 0.5f) * dt;
        g_sub.hull = max(0.0f, g_sub.hull - hullDamage);
        g_sub.waterIntrusionRate = excess * 0.05f;
        if ((rand() % 100) < 3) {
            PlaySoundAsync(150, 200);
            AddLog("CRUSH WARNING: Extreme hydrostatic pressure deforming hull!", CLR_ACCENT_RED);
        }
    } else {
        g_sub.waterIntrusionRate = 0.0f;
    }

    if (g_sub.waterIntrusionRate > 0.0f) {
        g_sub.bilgeWater += g_sub.waterIntrusionRate * dt;
    }
    if (g_sub.bilgePumpActive && g_sub.bilgeWater > 0.0f && g_sub.battery > 0.0f) {
        float pumped = min(g_sub.bilgeWater, 10.0f * dt);
        g_sub.bilgeWater -= pumped;
    }

    float baseDrain = 0.3f;
    if (g_sub.searchlights) baseDrain += 0.8f;
    if (g_sub.throttleMode == 2) baseDrain += 1.2f;
    if (g_sub.throttleMode == 3) baseDrain += 3.5f;
    if (g_sub.bilgePumpActive) baseDrain += 0.6f;
    if (g_sub.scrubberAuto) baseDrain += 0.4f;
    if (g_sub.lowPowerMode) baseDrain *= 0.45f;

    g_sub.powerDrain = baseDrain;
    if (g_sub.depth > 0.0f) {
        g_sub.battery = max(0.0f, g_sub.battery - (baseDrain * 0.015f * dt));
    }

    if (g_sub.scrubberAuto && g_sub.battery > 0.0f) {
        g_sub.scrubberStatus = max(10.0f, g_sub.scrubberStatus - dt * 0.01f);
        g_sub.co2 = min(2.0f, g_sub.co2 + dt * 0.002f);
        g_sub.o2 = max(0.0f, g_sub.o2 - dt * 0.015f);
    } else {
        g_sub.co2 = min(8.0f, g_sub.co2 + dt * 0.02f);
        g_sub.o2 = max(0.0f, g_sub.o2 - dt * 0.05f);
    }

    if (g_sub.depth < 200.0f) {
        g_sub.temp = 21.4f - (g_sub.depth / 200.0f) * 8.0f;
    } else if (g_sub.depth < 1000.0f) {
        g_sub.temp = 13.4f - ((g_sub.depth - 200.0f) / 800.0f) * 9.0f;
    } else {
        g_sub.temp = max(1.2f, 4.4f - ((g_sub.depth - 1000.0f) / 9000.0f) * 3.2f);
    }

    g_sub.sweepAngle += 0.035f;
    if (g_sub.sweepAngle >= 6.2831853f) g_sub.sweepAngle -= 6.2831853f;

    if (g_sub.isPinging) {
        g_sub.pingRadius += 5.0f;
        if (g_sub.pingRadius > 180.0f) {
            g_sub.isPinging = 0;
            g_sub.pingRadius = 0.0f;
            AddLog("Active sonar omnidirectional ping cycle completed.", CLR_TEXT_DIM);
        }
    }
}

void DrawGaugeBar(HDC hdc, int x, int y, int w, int h, float percent, COLORREF fillClr) {
    RECT rcTrack = { x, y, x + w, y + h };
    HBRUSH hBrTrack = CreateSolidBrush(CLR_GAUGE_BG);
    HBRUSH hBrBorder = CreateSolidBrush(CLR_BORDER_PANEL);
    FillRect(hdc, &rcTrack, hBrTrack);
    FrameRect(hdc, &rcTrack, hBrBorder);
    DeleteObject(hBrTrack);
    DeleteObject(hBrBorder);

    if (percent > 0.0f) {
        int fillW = (int)((w - 2) * (percent / 100.0f));
        if (fillW > w - 2) fillW = w - 2;
        if (fillW > 0) {
            RECT rcFill = { x + 1, y + 1, x + 1 + fillW, y + h - 1 };
            HBRUSH hBrFill = CreateSolidBrush(fillClr);
            FillRect(hdc, &rcFill, hBrFill);
            DeleteObject(hBrFill);
        }
    }
}

void DrawPanelBox(HDC hdc, int x, int y, int w, int h, const char* title, const char* status, COLORREF statusClr) {
    RECT rcBox = { x, y, x + w, y + h };
    HBRUSH hBrPanel = CreateSolidBrush(CLR_BG_PANEL);
    HBRUSH hBrBorder = CreateSolidBrush(CLR_BORDER_PANEL);
    FillRect(hdc, &rcBox, hBrPanel);
    FrameRect(hdc, &rcBox, hBrBorder);
    DeleteObject(hBrPanel);

    RECT rcHdr = { x, y, x + w, y + 24 };
    HBRUSH hBrHdr = CreateSolidBrush(CLR_BG_HEADER);
    FillRect(hdc, &rcHdr, hBrHdr);
    FrameRect(hdc, &rcHdr, hBrBorder);
    DeleteObject(hBrHdr);
    DeleteObject(hBrBorder);

    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, CLR_TEXT_BRIGHT);
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, x + 8, y + 4, title, (int)strlen(title));

    if (status && status[0]) {
        SetTextColor(hdc, statusClr);
        SIZE sz;
        GetTextExtentPoint32A(hdc, status, (int)strlen(status), &sz);
        TextOutA(hdc, x + w - sz.cx - 8, y + 4, status, (int)strlen(status));
    }
}

void DrawCustomButton(HDC hdc, int id, int x, int y, int w, int h, const char* label, int isActive, COLORREF accentClr) {
    RECT rc = { x, y, x + w, y + h };
    HBRUSH hBr = CreateSolidBrush(isActive ? CLR_BTN_ACTIVE : CLR_BTN_BG);
    HBRUSH hBrBorder = CreateSolidBrush(isActive ? accentClr : CLR_BORDER_PANEL);
    FillRect(hdc, &rc, hBr);
    FrameRect(hdc, &rc, hBrBorder);
    DeleteObject(hBr);
    DeleteObject(hBrBorder);

    SelectObject(hdc, g_hFontSmall);
    SetTextColor(hdc, isActive ? CLR_TEXT_BRIGHT : (accentClr != CLR_BORDER_GLOW ? accentClr : CLR_TEXT_CYAN));
    SetBkMode(hdc, TRANSPARENT);
    
    SIZE sz;
    GetTextExtentPoint32A(hdc, label, (int)strlen(label), &sz);
    int tx = x + (w - sz.cx) / 2;
    int ty = y + (h - sz.cy) / 2;
    TextOutA(hdc, tx, ty, label, (int)strlen(label));
}

void DrawUI(HDC hdc, RECT* rcClient) {
    int clientW = rcClient->right - rcClient->left;
    int clientH = rcClient->bottom - rcClient->top;

    HBRUSH hBrBg = CreateSolidBrush(CLR_BG_DEEP);
    FillRect(hdc, rcClient, hBrBg);
    DeleteObject(hBrBg);

    RECT rcHeader = { 0, 0, clientW, 36 };
    HBRUSH hBrHdr = CreateSolidBrush(CLR_BG_HEADER);
    HBRUSH hBrBrd = CreateSolidBrush(CLR_BORDER_PANEL);
    FillRect(hdc, &rcHeader, hBrHdr);
    FrameRect(hdc, &rcHeader, hBrBrd);
    DeleteObject(hBrHdr);
    DeleteObject(hBrBrd);

    SelectObject(hdc, g_hFontTitle);
    SetTextColor(hdc, CLR_TEXT_BRIGHT);
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, 12, 8, "DSV ABYSS VOYAGER // SUB-09", 27);

    SelectObject(hdc, g_hFontBold);
    const char* zoneStr = GetZoneName(g_sub.depth);
    RECT rcZone = { 285, 8, 510, 28 };
    HBRUSH hBrZone = CreateSolidBrush(CLR_TEXT_DIM);
    FillRect(hdc, &rcZone, hBrZone);
    DeleteObject(hBrZone);
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawTextA(hdc, zoneStr, -1, &rcZone, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DrawCustomButton(hdc, ID_BTN_SOUND_TOGGLE, clientW - 270, 6, 120, 24, g_sub.soundEnabled ? "AUDIO: ON" : "AUDIO: OFF", g_sub.soundEnabled, CLR_ACCENT_SONAR);
    DrawCustomButton(hdc, ID_BTN_EMERGENCY_BLOW, clientW - 142, 6, 130, 24, "BLOW BALLAST", 0, CLR_ACCENT_RED);

    int margin = 8;
    int panelY = 44;
    int panelH = clientH - panelY - margin;
    int leftW = 280;
    int rightW = 290;
    int centerW = clientW - leftW - rightW - (margin * 4);
    int centerX = leftW + (margin * 2);
    int rightX = clientW - rightW - margin;

    const char* statusText = g_sub.hull < 35.0f ? "CRITICAL RISK" : (g_sub.hull < 70.0f || g_sub.hullStress > 80.0f ? "HIGH STRESS" : "NOMINAL");
    COLORREF statusClr = g_sub.hull < 35.0f ? CLR_ACCENT_RED : (g_sub.hull < 70.0f || g_sub.hullStress > 80.0f ? CLR_ACCENT_AMBER : CLR_ACCENT_EMERALD);
    DrawPanelBox(hdc, margin, panelY, leftW, panelH, "VITAL TELEMETRY", statusText, statusClr);

    int gy = panelY + 32;
    int gw = leftW - 20;
    int gx = margin + 10;
    char buf[128];

    SelectObject(hdc, g_hFontSmall);
    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, gx, gy, "DEPTH / VERTICAL RATE", 22);
    snprintf(buf, sizeof(buf), "%.1f m", g_sub.depth);
    SetTextColor(hdc, CLR_TEXT_BRIGHT);
    SIZE sz;
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    DrawGaugeBar(hdc, gx, gy, gw, 10, min(100.0f, (g_sub.depth / 5000.0f) * 100.0f), CLR_ACCENT_SONAR);
    gy += 12;
    snprintf(buf, sizeof(buf), "CRUSH: 4,500m   RATE: %+.2f m/s", g_sub.vertRate);
    SetTextColor(hdc, g_sub.vertRate > 0.0f ? CLR_ACCENT_AMBER : (g_sub.vertRate < 0.0f ? CLR_ACCENT_EMERALD : CLR_TEXT_DIM));
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 22;

    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, gx, gy, "HULL INTEGRITY", 14);
    snprintf(buf, sizeof(buf), "%.1f%%", g_sub.hull);
    SetTextColor(hdc, CLR_TEXT_BRIGHT);
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    COLORREF hullClr = g_sub.hull < 35.0f ? CLR_ACCENT_RED : (g_sub.hull < 70.0f ? CLR_ACCENT_AMBER : CLR_ACCENT_SONAR);
    DrawGaugeBar(hdc, gx, gy, gw, 10, g_sub.hull, hullClr);
    gy += 12;
    snprintf(buf, sizeof(buf), "PRESSURE: %.1f atm   STRESS: %.1f%%", g_sub.pressure, g_sub.hullStress);
    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 22;

    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, gx, gy, "O2 CONCENTRATION", 16);
    snprintf(buf, sizeof(buf), "%.1f%%", g_sub.o2);
    SetTextColor(hdc, CLR_TEXT_BRIGHT);
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    COLORREF o2Clr = g_sub.o2 < 40.0f ? CLR_ACCENT_RED : (g_sub.o2 < 70.0f ? CLR_ACCENT_AMBER : CLR_ACCENT_SONAR);
    DrawGaugeBar(hdc, gx, gy, gw, 10, g_sub.o2, o2Clr);
    gy += 12;
    snprintf(buf, sizeof(buf), "CO2: %.2f%%   SCRUBBER: %.0f%%", g_sub.co2, g_sub.scrubberStatus);
    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 22;

    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, gx, gy, "BATTERY BANK", 12);
    snprintf(buf, sizeof(buf), "%.1f%%", g_sub.battery);
    SetTextColor(hdc, CLR_TEXT_BRIGHT);
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    COLORREF batClr = g_sub.battery < 20.0f ? CLR_ACCENT_RED : (g_sub.battery < 45.0f ? CLR_ACCENT_AMBER : CLR_ACCENT_EMERALD);
    DrawGaugeBar(hdc, gx, gy, gw, 10, g_sub.battery, batClr);
    gy += 12;
    snprintf(buf, sizeof(buf), "LOAD: %.2f kW   GRID: %s", g_sub.powerDrain, g_sub.depth <= 0.0f ? "SURFACE AUX" : "INTERNAL");
    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 22;

    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, gx, gy, "BALLAST FLOOD", 13);
    snprintf(buf, sizeof(buf), "%.0f%% (%s)", g_sub.ballast, g_sub.ballast < 40.0f ? "BUOYANT" : (g_sub.ballast > 55.0f ? "HEAVY" : "NEUTRAL"));
    SetTextColor(hdc, CLR_TEXT_BRIGHT);
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    DrawGaugeBar(hdc, gx, gy, gw, 10, g_sub.ballast, CLR_TEXT_CYAN);
    gy += 12;
    int netBuoy = (int)((45.0f - g_sub.ballast) * 25.0f);
    snprintf(buf, sizeof(buf), "AIR RES: %.0f BAR   BUOY: %+d KG", g_sub.airReservoir, netBuoy);
    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 22;

    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, gx, gy, "BILGE ACCUMULATION", 18);
    snprintf(buf, sizeof(buf), "%.1f GAL", g_sub.bilgeWater);
    SetTextColor(hdc, CLR_TEXT_BRIGHT);
    GetTextExtentPoint32A(hdc, buf, (int)strlen(buf), &sz);
    TextOutA(hdc, gx + gw - sz.cx, gy, buf, (int)strlen(buf));
    gy += 15;
    DrawGaugeBar(hdc, gx, gy, gw, 10, min(100.0f, g_sub.bilgeWater * 2.5f), CLR_ACCENT_AMBER);
    gy += 12;
    snprintf(buf, sizeof(buf), "INTRUSION: %.1f GPM  PUMP: %s", g_sub.waterIntrusionRate, g_sub.bilgePumpActive ? "10 GPM" : "0 GPM");
    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 24;

    snprintf(buf, sizeof(buf), "PITCH TRIM: %+.1f deg (%s)", g_sub.pitch, g_sub.pitch == 0.0f ? "LEVEL" : (g_sub.pitch > 0.0f ? "STERN DOWN" : "BOW DOWN"));
    SetTextColor(hdc, CLR_TEXT_BRIGHT);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));
    gy += 16;
    snprintf(buf, sizeof(buf), "SEAWATER TEMP: %.1f deg C", g_sub.temp);
    SetTextColor(hdc, CLR_ACCENT_SONAR);
    TextOutA(hdc, gx, gy, buf, (int)strlen(buf));

    int sonarH = (panelH * 60) / 100;
    int logH = panelH - sonarH - 8;
    int logY = panelY + sonarH + 8;

    DrawPanelBox(hdc, centerX, panelY, centerW, sonarH, "ACTIVE SONAR & ACOUSTIC SWEEP", "360 deg SCAN", CLR_ACCENT_EMERALD);

    int sonarContentY = panelY + 28;
    int sonarContentH = sonarH - 32;
    int scx = centerX + centerW / 2;
    int scy = sonarContentY + sonarContentH / 2;
    int sRadius = min(centerW, sonarContentH) / 2 - 16;

    RECT rcRadar = { scx - sRadius - 10, scy - sRadius - 10, scx + sRadius + 10, scy + sRadius + 10 };
    HBRUSH hBrRadar = CreateSolidBrush(CLR_BLACK);
    FillRect(hdc, &rcRadar, hBrRadar);
    DeleteObject(hBrRadar);

    HPEN hPenRing = CreatePen(PS_SOLID, 1, RGB(0, 90, 130));
    HPEN hPenOld = (HPEN)SelectObject(hdc, hPenRing);
    HBRUSH hBrNull = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH hBrOld = (HBRUSH)SelectObject(hdc, hBrNull);

    for (int r = 1; r <= 4; r++) {
        int curR = (sRadius * r) / 4;
        Ellipse(hdc, scx - curR, scy - curR, scx + curR, scy + curR);
    }
    MoveToEx(hdc, scx - sRadius, scy, NULL);
    LineTo(hdc, scx + sRadius, scy);
    MoveToEx(hdc, scx, scy - sRadius, NULL);
    LineTo(hdc, scx, scy + sRadius);

    HPEN hPenSweep = CreatePen(PS_SOLID, 2, CLR_ACCENT_SONAR);
    SelectObject(hdc, hPenSweep);
    int sx = scx + (int)(cosf(g_sub.sweepAngle) * sRadius);
    int sy = scy + (int)(sinf(g_sub.sweepAngle) * sRadius);
    MoveToEx(hdc, scx, scy, NULL);
    LineTo(hdc, sx, sy);
    DeleteObject(hPenSweep);

    if (g_sub.isPinging && g_sub.pingRadius > 0.0f) {
        HPEN hPenPing = CreatePen(PS_SOLID, 2, CLR_ACCENT_SONAR);
        SelectObject(hdc, hPenPing);
        int pr = (int)min((float)sRadius, g_sub.pingRadius);
        Ellipse(hdc, scx - pr, scy - pr, scx + pr, scy + pr);
        DeleteObject(hPenPing);
    }

    HBRUSH hBrSub = CreateSolidBrush(CLR_ACCENT_EMERALD);
    SelectObject(hdc, hBrSub);
    Ellipse(hdc, scx - 4, scy - 4, scx + 4, scy + 4);
    DeleteObject(hBrSub);

    float hRad = (g_sub.heading - 90.0f) * (3.14159265f / 180.0f);
    HPEN hPenHeading = CreatePen(PS_SOLID, 2, CLR_ACCENT_EMERALD);
    SelectObject(hdc, hPenHeading);
    MoveToEx(hdc, scx, scy, NULL);
    LineTo(hdc, scx + (int)(cosf(hRad) * 16), scy + (int)(sinf(hRad) * 16));
    DeleteObject(hPenHeading);

    SelectObject(hdc, g_hFontSmall);
    for (int i = 0; i < g_sub.contactCount; i++) {
        SonarContact* c = &g_sub.contacts[i];
        int cx = scx + (int)(cosf(c->angle) * (sRadius * c->dist));
        int cy = scy + (int)(sinf(c->angle) * (sRadius * c->dist));

        float angleDiff = fabsf(g_sub.sweepAngle - c->angle);
        int isSwept = (angleDiff < 0.25f) || (g_sub.isPinging && fabsf(g_sub.pingRadius - sRadius * c->dist) < 20.0f);

        HBRUSH hBrContact = CreateSolidBrush(isSwept ? CLR_ACCENT_SONAR : RGB(0, 70, 100));
        SelectObject(hdc, hBrContact);
        Ellipse(hdc, cx - 3, cy - 3, cx + 3, cy + 3);
        DeleteObject(hBrContact);

        if (isSwept) {
            SetTextColor(hdc, CLR_TEXT_BRIGHT);
            TextOutA(hdc, cx + 6, cy - 6, c->label, (int)strlen(c->label));
            snprintf(buf, sizeof(buf), "%.0fm", c->dist * 2000.0f);
            SetTextColor(hdc, CLR_TEXT_DIM);
            TextOutA(hdc, cx + 6, cy + 4, buf, (int)strlen(buf));
        }
    }

    SelectObject(hdc, hPenOld);
    SelectObject(hdc, hBrOld);
    DeleteObject(hPenRing);

    SelectObject(hdc, g_hFontSmall);
    SetTextColor(hdc, CLR_TEXT_CYAN);
    snprintf(buf, sizeof(buf), "HEADING: %03.0f deg  SPEED: %.1f KTS  SEABED: %.0f M", g_sub.heading, g_sub.speed, max(0.0f, 11000.0f - g_sub.depth));
    TextOutA(hdc, centerX + 10, sonarContentY + 6, buf, (int)strlen(buf));

    DrawPanelBox(hdc, centerX, logY, centerW, logH, "SYSTEM LOG & TELEMETRY STREAM", "LIVE", CLR_ACCENT_SONAR);
    int logLineY = logY + 28;
    for (int i = 0; i < g_sub.logCount; i++) {
        LogEntry* e = &g_sub.logs[i];
        SetTextColor(hdc, CLR_TEXT_DIM);
        TextOutA(hdc, centerX + 10, logLineY, e->time, (int)strlen(e->time));
        SetTextColor(hdc, e->color);
        TextOutA(hdc, centerX + 80, logLineY, e->text, (int)strlen(e->text));
        logLineY += 15;
        if (logLineY > logY + logH - 16) break;
    }

    DrawPanelBox(hdc, rightX, panelY, rightW, panelH, "HELM & SUBSYSTEM COMMAND", "CMD-CTRL", CLR_ACCENT_SONAR);

    int cy = panelY + 30;
    int bw = (rightW - 24) / 2;
    int bx = rightX + 8;

    SelectObject(hdc, g_hFontSmall);
    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, bx, cy, "BALLAST DIVE ENGINE", 19);
    cy += 14;

    DrawCustomButton(hdc, ID_BTN_FLOOD_BALLAST, bx, cy, bw, 24, "FLOOD BALLAST (+)", 0, CLR_TEXT_CYAN);
    DrawCustomButton(hdc, ID_BTN_BLOW_BALLAST, bx + bw + 6, cy, bw, 24, "BLOW BALLAST (-)", 0, CLR_ACCENT_EMERALD);
    cy += 28;

    DrawCustomButton(hdc, ID_BTN_TRIM_BOW, bx, cy, bw, 24, "TRIM BOW (-1 deg)", 0, CLR_TEXT_CYAN);
    DrawCustomButton(hdc, ID_BTN_TRIM_STERN, bx + bw + 6, cy, bw, 24, "TRIM STERN (+1 deg)", 0, CLR_TEXT_CYAN);
    cy += 28;

    DrawCustomButton(hdc, ID_BTN_SONAR_PING, bx, cy, rightW - 18, 26, "ACOUSTIC SONAR PING", g_sub.isPinging, CLR_ACCENT_SONAR);
    cy += 34;

    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, bx, cy, "PROPULSION THROTTLE", 19);
    cy += 14;

    int bw3 = (rightW - 28) / 3;
    DrawCustomButton(hdc, ID_BTN_THROTTLE_REV, bx, cy, bw3, 22, "REV", g_sub.throttleMode == 0, CLR_ACCENT_AMBER);
    DrawCustomButton(hdc, ID_BTN_THROTTLE_STOP, bx + bw3 + 4, cy, bw3, 22, "STOP", g_sub.throttleMode == 1, CLR_ACCENT_SONAR);
    DrawCustomButton(hdc, ID_BTN_THROTTLE_HALF, bx + (bw3 + 4) * 2, cy, bw3, 22, "HALF", g_sub.throttleMode == 2, CLR_ACCENT_SONAR);
    cy += 26;

    DrawCustomButton(hdc, ID_BTN_THROTTLE_FLANK, bx, cy, rightW - 18, 24, "FLANK SPEED (FULL AHEAD)", g_sub.throttleMode == 3, CLR_ACCENT_AMBER);
    cy += 28;

    DrawCustomButton(hdc, ID_BTN_RUDDER_PORT, bx, cy, bw, 24, "< RUDDER PORT", 0, CLR_TEXT_CYAN);
    DrawCustomButton(hdc, ID_BTN_RUDDER_STBD, bx + bw + 6, cy, bw, 24, "RUDDER STBD >", 0, CLR_TEXT_CYAN);
    cy += 34;

    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, bx, cy, "SUBSYSTEM MANAGEMENT", 20);
    cy += 14;

    snprintf(buf, sizeof(buf), "SEARCHLIGHTS: %s", g_sub.searchlights ? "ENGAGED [HIGH LUX]" : "OFF");
    DrawCustomButton(hdc, ID_BTN_SEARCHLIGHTS, bx, cy, rightW - 18, 22, buf, g_sub.searchlights, CLR_ACCENT_SONAR);
    cy += 26;

    snprintf(buf, sizeof(buf), "O2 SCRUBBER: %s", g_sub.scrubberAuto ? "AUTO [ONLINE]" : "MANUAL [STANDBY]");
    DrawCustomButton(hdc, ID_BTN_SCRUBBER, bx, cy, rightW - 18, 22, buf, g_sub.scrubberAuto, CLR_ACCENT_EMERALD);
    cy += 26;

    snprintf(buf, sizeof(buf), "PURGE EMERGENCY O2 (%d LEFT)", g_sub.o2PurgeCount);
    DrawCustomButton(hdc, ID_BTN_O2_PURGE, bx, cy, rightW - 18, 22, buf, 0, CLR_TEXT_CYAN);
    cy += 26;

    snprintf(buf, sizeof(buf), "BILGE PUMPS: %s", g_sub.bilgePumpActive ? "RUNNING [MAX]" : "AUTO (STANDBY)");
    DrawCustomButton(hdc, ID_BTN_BILGE_PUMP, bx, cy, rightW - 18, 22, buf, g_sub.bilgePumpActive, CLR_ACCENT_AMBER);
    cy += 26;

    snprintf(buf, sizeof(buf), "ECO LOW-POWER: %s", g_sub.lowPowerMode ? "ACTIVE" : "OFF");
    DrawCustomButton(hdc, ID_BTN_LOW_POWER, bx, cy, rightW - 18, 22, buf, g_sub.lowPowerMode, CLR_ACCENT_EMERALD);
    cy += 32;

    RECT rcDirect = { bx, cy, rightX + rightW - 10, panelY + panelH - 10 };
    HBRUSH hBrDirect = CreateSolidBrush(CLR_BG_DEEP);
    FillRect(hdc, &rcDirect, hBrDirect);
    FrameRect(hdc, &rcDirect, hBrBrd);
    DeleteObject(hBrDirect);
    SetTextColor(hdc, CLR_TEXT_DIM);
    TextOutA(hdc, bx + 6, cy + 4, "CURRENT DIRECTIVE:", 18);
    SetTextColor(hdc, CLR_TEXT_BRIGHT);
    TextOutA(hdc, bx + 6, cy + 20, "- Submerge to Mesopelagic (200m+)", 33);
    TextOutA(hdc, bx + 6, cy + 34, "- Conduct active sonar ping sweep", 33);
    TextOutA(hdc, bx + 6, cy + 48, "- Balance ballast for neutral trim", 34);
    TextOutA(hdc, bx + 6, cy + 62, "- Monitor O2 & hydrostatic stress", 33);
}

int HitTestButton(int mx, int my, int clientW, int clientH) {
    int margin = 8;
    int panelY = 44;
    int panelH = clientH - panelY - margin;
    int rightW = 290;
    int rightX = clientW - rightW - margin;

    if (my >= 6 && my <= 30) {
        if (mx >= clientW - 270 && mx <= clientW - 150) return ID_BTN_SOUND_TOGGLE;
        if (mx >= clientW - 142 && mx <= clientW - 12) return ID_BTN_EMERGENCY_BLOW;
    }

    int cy = panelY + 30;
    int bw = (rightW - 24) / 2;
    int bx = rightX + 8;
    cy += 14;

    if (my >= cy && my <= cy + 24) {
        if (mx >= bx && mx <= bx + bw) return ID_BTN_FLOOD_BALLAST;
        if (mx >= bx + bw + 6 && mx <= bx + bw * 2 + 6) return ID_BTN_BLOW_BALLAST;
    }
    cy += 28;

    if (my >= cy && my <= cy + 24) {
        if (mx >= bx && mx <= bx + bw) return ID_BTN_TRIM_BOW;
        if (mx >= bx + bw + 6 && mx <= bx + bw * 2 + 6) return ID_BTN_TRIM_STERN;
    }
    cy += 28;

    if (my >= cy && my <= cy + 26 && mx >= bx && mx <= bx + rightW - 18) {
        return ID_BTN_SONAR_PING;
    }
    cy += 34 + 14;

    int bw3 = (rightW - 28) / 3;
    if (my >= cy && my <= cy + 22) {
        if (mx >= bx && mx <= bx + bw3) return ID_BTN_THROTTLE_REV;
        if (mx >= bx + bw3 + 4 && mx <= bx + bw3 * 2 + 4) return ID_BTN_THROTTLE_STOP;
        if (mx >= bx + (bw3 + 4) * 2 && mx <= bx + (bw3 + 4) * 3) return ID_BTN_THROTTLE_HALF;
    }
    cy += 26;

    if (my >= cy && my <= cy + 24 && mx >= bx && mx <= bx + rightW - 18) {
        return ID_BTN_THROTTLE_FLANK;
    }
    cy += 28;

    if (my >= cy && my <= cy + 24) {
        if (mx >= bx && mx <= bx + bw) return ID_BTN_RUDDER_PORT;
        if (mx >= bx + bw + 6 && mx <= bx + bw * 2 + 6) return ID_BTN_RUDDER_STBD;
    }
    cy += 34 + 14;

    if (my >= cy && my <= cy + 22 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_SEARCHLIGHTS;
    cy += 26;
    if (my >= cy && my <= cy + 22 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_SCRUBBER;
    cy += 26;
    if (my >= cy && my <= cy + 22 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_O2_PURGE;
    cy += 26;
    if (my >= cy && my <= cy + 22 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_BILGE_PUMP;
    cy += 26;
    if (my >= cy && my <= cy + 22 && mx >= bx && mx <= bx + rightW - 18) return ID_BTN_LOW_POWER;

    return 0;
}

void HandleCommand(int cmdId) {
    char msg[128];
    switch (cmdId) {
        case ID_BTN_FLOOD_BALLAST:
            if (g_sub.ballast < 100.0f) {
                g_sub.ballast = min(100.0f, g_sub.ballast + 10.0f);
                PlaySoundAsync(280, 100);
                snprintf(msg, sizeof(msg), "Kingston flood valves opened: Ballast %.0f%% flooded.", g_sub.ballast);
                AddLog(msg, CLR_TEXT_CYAN);
            }
            break;

        case ID_BTN_BLOW_BALLAST:
            if (g_sub.airReservoir > 5.0f && g_sub.ballast > 0.0f) {
                g_sub.ballast = max(0.0f, g_sub.ballast - 10.0f);
                g_sub.airReservoir = max(0.0f, g_sub.airReservoir - 8.0f);
                PlaySoundAsync(600, 120);
                snprintf(msg, sizeof(msg), "Blowing ballast with HP air: Ballast %.0f%%, Air Res: %.0f BAR.", g_sub.ballast, g_sub.airReservoir);
                AddLog(msg, CLR_TEXT_CYAN);
            } else if (g_sub.airReservoir <= 5.0f) {
                PlaySoundAsync(180, 200);
                AddLog("WARNING: Insufficient compressed air reservoir to blow ballast!", CLR_ACCENT_RED);
            }
            break;

        case ID_BTN_TRIM_BOW:
            g_sub.pitch = max(-15.0f, g_sub.pitch - 2.5f);
            PlaySoundAsync(330, 80);
            snprintf(msg, sizeof(msg), "Trim shifted forward. Submarine pitch: %+.1f deg", g_sub.pitch);
            AddLog(msg, CLR_TEXT_CYAN);
            break;

        case ID_BTN_TRIM_STERN:
            g_sub.pitch = min(15.0f, g_sub.pitch + 2.5f);
            PlaySoundAsync(330, 80);
            snprintf(msg, sizeof(msg), "Trim shifted aft. Submarine pitch: %+.1f deg", g_sub.pitch);
            AddLog(msg, CLR_TEXT_CYAN);
            break;

        case ID_BTN_SONAR_PING:
            if (!g_sub.isPinging) {
                g_sub.isPinging = 1;
                g_sub.pingRadius = 0.0f;
                g_sub.battery = max(0.0f, g_sub.battery - 0.2f);
                PlaySoundAsync(1920, 250);
                AddLog("Active acoustic sonar pulse generated. Omnidirectional sweep...", CLR_ACCENT_EMERALD);
            }
            break;

        case ID_BTN_THROTTLE_REV:
            g_sub.throttleMode = 0;
            g_sub.targetSpeed = -2.5f;
            PlaySoundAsync(380, 80);
            AddLog("Engine telegraph set to [REV] -> Target speed: -2.5 kts", CLR_TEXT_CYAN);
            break;

        case ID_BTN_THROTTLE_STOP:
            g_sub.throttleMode = 1;
            g_sub.targetSpeed = 0.0f;
            PlaySoundAsync(380, 80);
            AddLog("Engine telegraph set to [STOP] -> Target speed: 0.0 kts", CLR_TEXT_CYAN);
            break;

        case ID_BTN_THROTTLE_HALF:
            g_sub.throttleMode = 2;
            g_sub.targetSpeed = 5.0f;
            PlaySoundAsync(380, 80);
            AddLog("Engine telegraph set to [HALF] -> Target speed: 5.0 kts", CLR_TEXT_CYAN);
            break;

        case ID_BTN_THROTTLE_FLANK:
            g_sub.throttleMode = 3;
            g_sub.targetSpeed = 11.5f;
            PlaySoundAsync(380, 80);
            AddLog("Engine telegraph set to [FLANK] -> Target speed: 11.5 kts", CLR_ACCENT_AMBER);
            break;

        case ID_BTN_RUDDER_PORT:
            g_sub.heading = fmodf(g_sub.heading - 5.0f + 360.0f, 360.0f);
            PlaySoundAsync(450, 60);
            snprintf(msg, sizeof(msg), "Rudder Port 5 deg -> Heading: %03.0f deg", g_sub.heading);
            AddLog(msg, CLR_TEXT_CYAN);
            break;

        case ID_BTN_RUDDER_STBD:
            g_sub.heading = fmodf(g_sub.heading + 5.0f, 360.0f);
            PlaySoundAsync(450, 60);
            snprintf(msg, sizeof(msg), "Rudder Starboard 5 deg -> Heading: %03.0f deg", g_sub.heading);
            AddLog(msg, CLR_TEXT_CYAN);
            break;

        case ID_BTN_SEARCHLIGHTS:
            g_sub.searchlights = !g_sub.searchlights;
            PlaySoundAsync(520, 80);
            snprintf(msg, sizeof(msg), "High-lux forward exploration floodlights %s.", g_sub.searchlights ? "ENGAGED" : "DISENGAGED");
            AddLog(msg, CLR_TEXT_CYAN);
            break;

        case ID_BTN_SCRUBBER:
            g_sub.scrubberAuto = !g_sub.scrubberAuto;
            PlaySoundAsync(400, 80);
            snprintf(msg, sizeof(msg), "O2 Life support scrubber switched to %s.", g_sub.scrubberAuto ? "AUTO" : "MANUAL");
            AddLog(msg, CLR_TEXT_CYAN);
            break;

        case ID_BTN_O2_PURGE:
            if (g_sub.o2PurgeCount > 0) {
                g_sub.o2PurgeCount--;
                g_sub.o2 = min(100.0f, g_sub.o2 + 25.0f);
                g_sub.co2 = max(0.04f, g_sub.co2 - 0.5f);
                PlaySoundAsync(750, 150);
                snprintf(msg, sizeof(msg), "Emergency O2 canister injected! O2 boosted to %.1f%%. [%d canisters remaining]", g_sub.o2, g_sub.o2PurgeCount);
                AddLog(msg, CLR_ACCENT_EMERALD);
            } else {
                PlaySoundAsync(180, 200);
                AddLog("Emergency O2 canisters exhausted!", CLR_ACCENT_RED);
            }
            break;

        case ID_BTN_BILGE_PUMP:
            g_sub.bilgePumpActive = !g_sub.bilgePumpActive;
            PlaySoundAsync(360, 80);
            snprintf(msg, sizeof(msg), "Bilge drainage pumps set to %s.", g_sub.bilgePumpActive ? "MAX RUNNING" : "AUTO STANDBY");
            AddLog(msg, CLR_TEXT_CYAN);
            break;

        case ID_BTN_LOW_POWER:
            g_sub.lowPowerMode = !g_sub.lowPowerMode;
            PlaySoundAsync(480, 80);
            snprintf(msg, sizeof(msg), "Submersible electrical grid set to %s.", g_sub.lowPowerMode ? "EMERGENCY CONSERVATION" : "STANDARD DISTRIBUTION");
            AddLog(msg, CLR_TEXT_CYAN);
            break;

        case ID_BTN_EMERGENCY_BLOW:
            g_sub.ballast = 0.0f;
            g_sub.pitch = 10.0f;
            g_sub.airReservoir = max(0.0f, g_sub.airReservoir - 50.0f);
            PlaySoundAsync(700, 300);
            AddLog("EMERGENCY MAIN BALLAST BLOW EXECUTED! Ascending at maximum positive buoyancy!", CLR_ACCENT_RED);
            break;

        case ID_BTN_SOUND_TOGGLE:
            g_sub.soundEnabled = !g_sub.soundEnabled;
            break;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            InitSubmarineState();
            g_hFontTitle = CreateFontA(-15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_hFontBold = CreateFontA(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_hFontNormal = CreateFontA(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            g_hFontSmall = CreateFontA(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
            SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, NULL);
            break;
        }

        case WM_TIMER: {
            if (wParam == TIMER_ID) {
                UpdateSimulation(TIMER_INTERVAL / 1000.0f);
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        }

        case WM_LBUTTONDOWN: {
            int mx = GET_X_LPARAM(lParam);
            int my = GET_Y_LPARAM(lParam);
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            int cmdId = HitTestButton(mx, my, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
            if (cmdId != 0) {
                HandleCommand(cmdId);
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            int w = rcClient.right - rcClient.left;
            int h = rcClient.bottom - rcClient.top;

            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, w, h);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

            DrawUI(hdcMem, &rcClient);

            BitBlt(hdc, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            EndPaint(hWnd, &ps);
            break;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_DESTROY: {
            KillTimer(hWnd, TIMER_ID);
            if (g_hFontTitle) DeleteObject(g_hFontTitle);
            if (g_hFontBold) DeleteObject(g_hFontBold);
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            if (g_hFontSmall) DeleteObject(g_hFontSmall);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProcA(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "KSubmarineClass";

    if (!RegisterClassExA(&wc)) return 0;

    int winW = 1040;
    int winH = 740;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = max(0, (screenW - winW) / 2);
    int posY = max(0, (screenH - winH) / 2);

    g_hWnd = CreateWindowExA(
        WS_EX_APPWINDOW,
        "KSubmarineClass",
        "KSubmarine - Bathyscaphe Submersible Dashboard & Deep Trench Exploration",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, winW, winH,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hWnd) return 0;

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}
