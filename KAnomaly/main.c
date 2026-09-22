#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

void* __cdecl memcpy(void* dst, const void* src, size_t sz) {
    char* d = (char*)dst;
    const char* s = (const char*)src;
    while (sz--) *d++ = *s++;
    return dst;
}
#pragma function(memcpy)

static int k_strlen(const char* s) {
    int len = 0;
    while (s && s[len]) len++;
    return len;
}

static void k_strcpy(char* dst, const char* src) {
    while (*src) *dst++ = *src++;
    *dst = '\0';
}

static void k_strcat(char* dst, const char* src) {
    while (*dst) dst++;
    while (*src) *dst++ = *src++;
    *dst = '\0';
}

static void k_itoa(int val, char* buf) {
    char temp[16];
    int i = 0, is_neg = 0;
    if (val < 0) { is_neg = 1; val = -val; }
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (val > 0) {
        temp[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    int out = 0;
    if (is_neg) buf[out++] = '-';
    while (i > 0) buf[out++] = temp[--i];
    buf[out] = '\0';
}

// Control IDs
#define ID_BTN_STATION_PREV 101
#define ID_BTN_STATION_NEXT 102
#define ID_BTN_FREQ_DOWN_L  103
#define ID_BTN_FREQ_DOWN_S  104
#define ID_BTN_FREQ_UP_S    105
#define ID_BTN_FREQ_UP_L    106
#define ID_BTN_SWEEP        107
#define ID_BTN_MODE         108
#define ID_BTN_BANDWIDTH    109
#define ID_BTN_SAVE         110
#define ID_BTN_LOAD         111
#define ID_BTN_HELP         112
#define ID_BTN_TRIANGULATE  113
#define ID_BTN_CLEAR_LOG    114
#define ID_EDIT_LOG         115

#define TIMER_TICK          1

// Colors
static COLORREF COLOR_BG       = RGB(6, 14, 10);
static COLORREF COLOR_PANEL    = RGB(12, 28, 20);
static COLORREF COLOR_BORDER   = RGB(32, 96, 56);
static COLORREF COLOR_PHOSPHOR = RGB(51, 255, 102);
static COLORREF COLOR_DIM      = RGB(24, 120, 50);
static COLORREF COLOR_AMBER    = RGB(255, 170, 0);
static COLORREF COLOR_CYAN     = RGB(51, 204, 255);
static COLORREF COLOR_ALERT    = RGB(255, 60, 60);

// Global GDI Objects
static HBRUSH g_hBrushBg     = NULL;
static HBRUSH g_hBrushPanel  = NULL;
static HBRUSH g_hBrushBorder = NULL;
static HFONT  g_hFontMono    = NULL;
static HFONT  g_hFontBold    = NULL;
static HFONT  g_hFontSmall   = NULL;

static HWND g_hwnd = NULL;
static HWND g_hEditLog = NULL;

// Waterfall display buffer (Width: 420, Height: 180)
#define WF_W 420
#define WF_H 180
static DWORD g_waterfall[WF_H][WF_W];
static int g_wf_head = 0;

// Subterranean Station Struct
typedef struct {
    char id[8];
    char name[32];
    int depth_m;
    char strata[32];
    double lat;
    double lon;
    int noise_floor;
} Station;

#define NUM_STATIONS 6
static Station g_stations[NUM_STATIONS] = {
    {"BH-09", "Kola Wellhead",        12262, "Precambrian Gneiss",   69.39,  30.61, -88},
    {"SB-7F", "Carlsbad Sub-Bunker",   3420, "Permian Evaporite",    32.42,-104.23, -92},
    {"MA-01", "Mariana Abyssal Bed",  10994, "Basaltic Mantle Crust",11.35, 142.20, -82},
    {"YM-04", "Yamantau Granitic",     4100, "Crystalline Quartzite",54.25,  58.11, -95},
    {"HG-12", "Hadron Deep Ring",      1750, "Alpine Molasse Basin", 46.23,   6.05, -90},
    {"AC-08", "Atacama Borehole",      5600, "Volcanic Basement",   -23.86, -69.14, -94}
};
static int g_curr_station = 0;

// Anomaly Definition
typedef struct {
    char id[12];
    char name[36];
    int freq_centikhz; // Freq in centi-kHz: 1999 = 19.99 kHz
    char mode[12];
    int min_depth;
    char decoded_sig[64];
    char lore[96];
    int discovered;
} Anomaly;

#define NUM_ANOMALIES 12
static Anomaly g_anomalies[NUM_ANOMALIES] = {
    {"ANOM-01", "Schumann Earth Cavity",    783, "AM",    1000, "CAVITY_7.83HZ_HARMONIC",       "Planetary electromagnetic standing resonance in lithosphere.", 0},
    {"ANOM-02", "Microseismic Tectonic Hum",4200, "AM",    2000, "TECTONIC_SLIP_BASELINE",       "Continuous baseline friction hum of oceanic crust plates.",    0},
    {"ANOM-03", "Kola Borehole Acoustic",  1226, "CW",   10000, "BOREHOLE_WELL_ECHO_PULSE",     "Deep drill-string microtremor harmonic feedback.",             0},
    {"ANOM-04", "Carlsbad Bunker Relay",   1019, "TEL",   3000, "10.19.99.4:DARKNET_RELAY",      "Subterranean darknet heartbeat beacon from abandoned bunker.", 0},
    {"ANOM-05", "Precursor Seismic Glyph", 3333, "SSTV",  4000, "PRECURSOR_RASTER_MATRIX",       "Ancient structured seismic raster pulse inside granite slab.", 0},
    {"ANOM-06", "Autonomous Fleet Pulse",  1999, "FSK",   1500, "FLEET_CYCLE_1999_SYS_ONLINE",   "Autonomous fleet clock cycle leaking via mantle waveguide.",   0},
    {"ANOM-07", "Mariana Trench Abyssal",  1440, "AM",    9000, "HYDRO_ACOUSTIC_ABYSS_TONE",     "Ultra-deep hydro-acoustic fault wave below the benthic floor.",0},
    {"ANOM-08", "Geothermal Vent Osc",    5670, "FSK",   2500, "HYDROTHERMAL_CHAMBER_BURST",    "Superheated supercritical steam cavity resonance.",            0},
    {"ANOM-09", "Yamantau Timecode Clock", 2850, "CW",    3500, "TIMECODE:1999.12.31.23:59:59",  "Synchronized atomic sub-crust countdown carrier.",             0},
    {"ANOM-10", "Mantle Waveguide Leak",   2400, "TEL",   5000, "WAVEGUIDE_CARRIER_LINK_OK",     "Underground coaxial fiber shielding leakage.",                 0},
    {"ANOM-11", "Lithosphere Ghost Morse", 7770, "CW",    2000, "ECHO IS AWAKE IN THE ROCK",     "Repeating Morse transmission pulsating through fault lines.",  0},
    {"ANOM-12", "Deep Core Singularity",   9990, "SSTV",  8000, "COORDINATES_0x7F_1999_MASTER",  "Master anomaly: convergence of all subterranean telemetry.",  0}
};

// Application State
static int g_freq_centikhz = 1019; // Current tuned frequency: 10.19 kHz
static int g_bandwidth_idx = 1;    // 0: 50Hz, 1: 250Hz, 2: 1kHz
static int g_mode_idx = 0;         // 0: AM, 1: FSK, 2: CW, 3: SSTV, 4: TEL
static int g_is_sweeping = 0;
static int g_sweep_dir = 1;
static int g_carrier_locked = 0;
static int g_locked_anom_idx = -1;
static int g_snr_db = 0;
static int g_discovered_count = 0;
static unsigned long g_frame_counter = 0;

static const char* g_mode_names[5] = {"AM", "FSK", "CW", "SSTV", "TEL"};
static const char* g_bandwidth_names[3] = {"50 Hz", "250 Hz", "1.0 kHz"};

// Forward declarations
static void AppendLog(const char* text);
static void UpdateTelemetryUI(void);
static void CheckSignalLock(void);
static void QuickSaveState(void);
static void QuickLoadState(void);
static void ShowHelp(HWND hwnd);

// Simple pseudo-random generator
static unsigned int g_rnd_seed = 1999;
static unsigned int k_rand(void) {
    g_rnd_seed = g_rnd_seed * 1103515245 + 12345;
    return (g_rnd_seed / 65536) % 32768;
}

// Convert centi-kHz to string: e.g. 1019 -> "10.19 kHz"
static void FormatFreq(int centikhz, char* out) {
    int whole = centikhz / 100;
    int frac = centikhz % 100;
    char s_whole[16], s_frac[16];
    k_itoa(whole, s_whole);
    k_itoa(frac, s_frac);
    k_strcpy(out, s_whole);
    k_strcat(out, ".");
    if (frac < 10) k_strcat(out, "0");
    k_strcat(out, s_frac);
    k_strcat(out, " kHz");
}

static void AppendLog(const char* text) {
    if (!g_hEditLog) return;
    int len = GetWindowTextLengthA(g_hEditLog);
    SendMessageA(g_hEditLog, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageA(g_hEditLog, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessageA(g_hEditLog, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
}

static void CheckSignalLock(void) {
    int prev_locked = g_carrier_locked;
    int prev_anom = g_locked_anom_idx;
    g_carrier_locked = 0;
    g_locked_anom_idx = -1;
    g_snr_db = 0;

    int tolerance = (g_bandwidth_idx == 0) ? 5 : (g_bandwidth_idx == 1 ? 15 : 35);

    for (int i = 0; i < NUM_ANOMALIES; i++) {
        int diff = g_freq_centikhz - g_anomalies[i].freq_centikhz;
        if (diff < 0) diff = -diff;

        if (diff <= tolerance) {
            g_carrier_locked = 1;
            g_locked_anom_idx = i;
            g_snr_db = 32 - (diff * 20 / (tolerance + 1));
            if (g_snr_db < 8) g_snr_db = 8;

            if (!g_anomalies[i].discovered) {
                g_anomalies[i].discovered = 1;
                g_discovered_count++;

                char log_msg[256];
                char fstr[32];
                FormatFreq(g_anomalies[i].freq_centikhz, fstr);

                k_strcpy(log_msg, "[DISCOVERY!] ");
                k_strcat(log_msg, g_anomalies[i].id);
                k_strcat(log_msg, " (");
                k_strcat(log_msg, g_anomalies[i].name);
                k_strcat(log_msg, ") at ");
                k_strcat(log_msg, fstr);
                AppendLog(log_msg);

                k_strcpy(log_msg, "  >> Payload: ");
                k_strcat(log_msg, g_anomalies[i].decoded_sig);
                AppendLog(log_msg);

                k_strcpy(log_msg, "  >> Lore: ");
                k_strcat(log_msg, g_anomalies[i].lore);
                AppendLog(log_msg);
            }
            break;
        }
    }

    if (g_carrier_locked && !prev_locked) {
        char msg[128];
        k_strcpy(msg, "[CARRIER LOCK] Freq: ");
        char fstr[32];
        FormatFreq(g_freq_centikhz, fstr);
        k_strcat(msg, fstr);
        k_strcat(msg, " | Signal: ");
        k_strcat(msg, g_anomalies[g_locked_anom_idx].name);
        AppendLog(msg);
    }
}

// Update the rolling waterfall buffer
static void UpdateWaterfall(void) {
    g_wf_head = (g_wf_head + 1) % WF_H;
    int row = g_wf_head;

    int center_x = WF_W / 2;
    int noise_base = 20 + (k_rand() % 15);

    for (int x = 0; x < WF_W; x++) {
        int val = noise_base + (k_rand() % 25);

        // Center tuned frequency marker band
        int dist_center = x - center_x;
        if (dist_center < 0) dist_center = -dist_center;

        // Bandwidth highlight
        int bw_half = (g_bandwidth_idx == 0) ? 6 : (g_bandwidth_idx == 1 ? 18 : 45);
        if (dist_center <= bw_half) {
            val += 25;
        }

        // Active carrier wave signal peak
        if (g_carrier_locked) {
            int peak_dist = dist_center;
            if (peak_dist < 10) {
                val += (10 - peak_dist) * 18;
            }
        }

        // Occasional subterranean seismic microtremor spikes
        if ((k_rand() % 100) < 3) {
            val += 60;
        }

        if (val > 255) val = 255;

        // Phosphor Green Color palette mapping
        BYTE r = (BYTE)(val > 180 ? (val - 180) * 2 : 0);
        BYTE g = (BYTE)val;
        BYTE b = (BYTE)(val > 220 ? (val - 220) : (val / 6));
        g_waterfall[row][x] = ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
    }
}

static void DrawWaterfall(HDC hdc, int x, int y, int w, int h) {
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WF_W;
    bmi.bmiHeader.biHeight = -WF_H; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    // Linearize rolling ring buffer into a display buffer
    static DWORD display_buf[WF_H][WF_W];
    for (int r = 0; r < WF_H; r++) {
        int src_row = (g_wf_head - r + WF_H) % WF_H;
        memcpy(display_buf[r], g_waterfall[src_row], WF_W * sizeof(DWORD));
    }

    StretchDIBits(
        hdc,
        x, y, w, h,
        0, 0, WF_W, WF_H,
        display_buf,
        &bmi,
        DIB_RGB_COLORS,
        SRCCOPY
    );

    // Draw reticle and tuning brackets
    HPEN hPenReticle = CreatePen(PS_SOLID, 1, COLOR_ALERT);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenReticle);

    int cx = x + w / 2;
    MoveToEx(hdc, cx, y, NULL);
    LineTo(hdc, cx, y + h);

    // Bandwidth brackets
    int bw_px = (g_bandwidth_idx == 0) ? 12 : (g_bandwidth_idx == 1 ? 36 : 90);
    HPEN hPenBw = CreatePen(PS_DOT, 1, COLOR_CYAN);
    SelectObject(hdc, hPenBw);
    MoveToEx(hdc, cx - bw_px / 2, y, NULL);
    LineTo(hdc, cx - bw_px / 2, y + h);
    MoveToEx(hdc, cx + bw_px / 2, y, NULL);
    LineTo(hdc, cx + bw_px / 2, y + h);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPenReticle);
    DeleteObject(hPenBw);
}

static void DrawOscilloscope(HDC hdc, int x, int y, int w, int h) {
    // Fill background
    RECT rc = {x, y, x + w, y + h};
    FillRect(hdc, &rc, g_hBrushBg);
    FrameRect(hdc, &rc, g_hBrushBorder);

    // Grid lines
    HPEN hPenGrid = CreatePen(PS_DOT, 1, RGB(16, 48, 28));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenGrid);
    MoveToEx(hdc, x, y + h / 2, NULL);
    LineTo(hdc, x + w, y + h / 2);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPenGrid);

    // Trace line
    HPEN hPenTrace = CreatePen(PS_SOLID, 1, g_carrier_locked ? COLOR_PHOSPHOR : COLOR_DIM);
    hOldPen = (HPEN)SelectObject(hdc, hPenTrace);

    int mid_y = y + h / 2;
    int prev_py = mid_y;

    for (int px = 0; px < w; px++) {
        double phase = (px + g_frame_counter * 3) * 0.12;
        int wave = (int)(18.0 * ((px % 20 < 10) ? 0.7 : -0.7)); // Square/stepped seismic vibration

        if (g_carrier_locked) {
            // FM modulated sinusoidal carrier
            double carrier = (px * 0.25 + g_frame_counter * 0.4);
            int carrier_amp = (int)(24.0 * (carrier - (int)carrier - 0.5));
            wave += carrier_amp;
        }

        // Ground noise
        int noise = (k_rand() % 9) - 4;
        int py = mid_y + wave + noise;
        if (py < y + 2) py = y + 2;
        if (py > y + h - 2) py = y + h - 2;

        if (px == 0) {
            MoveToEx(hdc, x + px, py, NULL);
        } else {
            LineTo(hdc, x + px, py);
        }
        prev_py = py;
    }

    SelectObject(hdc, hOldPen);
    DeleteObject(hPenTrace);
}

static void DrawTelemetryCards(HDC hdc, int x, int y, int w, int h) {
    RECT rc = {x, y, x + w, y + h};
    FillRect(hdc, &rc, g_hBrushPanel);
    FrameRect(hdc, &rc, g_hBrushBorder);

    SetBkMode(hdc, TRANSPARENT);
    SelectObject(hdc, g_hFontBold);
    SetTextColor(hdc, COLOR_PHOSPHOR);

    TextOutA(hdc, x + 10, y + 8, "LITHOSPHERE SENSOR ARRAY", 24);

    SelectObject(hdc, g_hFontMono);
    char buf[128];
    int line_y = y + 32;

    // Station
    k_strcpy(buf, "Station: ");
    k_strcat(buf, g_stations[g_curr_station].id);
    k_strcat(buf, " - ");
    k_strcat(buf, g_stations[g_curr_station].name);
    SetTextColor(hdc, COLOR_CYAN);
    TextOutA(hdc, x + 10, line_y, buf, k_strlen(buf));
    line_y += 18;

    // Depth & Stratum
    k_strcpy(buf, "Depth: ");
    char num_buf[16];
    k_itoa(g_stations[g_curr_station].depth_m, num_buf);
    k_strcat(buf, num_buf);
    k_strcat(buf, " m | Stratum: ");
    k_strcat(buf, g_stations[g_curr_station].strata);
    SetTextColor(hdc, COLOR_PHOSPHOR);
    TextOutA(hdc, x + 10, line_y, buf, k_strlen(buf));
    line_y += 18;

    // Tuner
    k_strcpy(buf, "Tuned Freq: ");
    char fstr[32];
    FormatFreq(g_freq_centikhz, fstr);
    k_strcat(buf, fstr);
    k_strcat(buf, " | Mode: ");
    k_strcat(buf, g_mode_names[g_mode_idx]);
    k_strcat(buf, " | BW: ");
    k_strcat(buf, g_bandwidth_names[g_bandwidth_idx]);
    SetTextColor(hdc, COLOR_AMBER);
    TextOutA(hdc, x + 10, line_y, buf, k_strlen(buf));
    line_y += 24;

    // Signal Carrier Lock
    if (g_carrier_locked) {
        SetTextColor(hdc, COLOR_ALERT);
        k_strcpy(buf, "[!] CARRIER LOCK: ");
        k_strcat(buf, g_anomalies[g_locked_anom_idx].id);
        k_strcat(buf, " (SNR: +");
        char snr_s[16];
        k_itoa(g_snr_db, snr_s);
        k_strcat(buf, snr_s);
        k_strcat(buf, " dB)");
        TextOutA(hdc, x + 10, line_y, buf, k_strlen(buf));
        line_y += 18;

        SetTextColor(hdc, COLOR_PHOSPHOR);
        k_strcpy(buf, ">> Decoded: ");
        k_strcat(buf, g_anomalies[g_locked_anom_idx].decoded_sig);
        TextOutA(hdc, x + 10, line_y, buf, k_strlen(buf));
    } else {
        SetTextColor(hdc, COLOR_DIM);
        TextOutA(hdc, x + 10, line_y, "[ SEARCHING NOISE FLOOR... ]", 28);
        line_y += 18;
        k_strcpy(buf, "Ambient Noise: ");
        k_itoa(g_stations[g_curr_station].noise_floor, num_buf);
        k_strcat(buf, num_buf);
        k_strcat(buf, " dBm | Sweep: ");
        k_strcat(buf, g_is_sweeping ? "ACTIVE" : "STANDBY");
        TextOutA(hdc, x + 10, line_y, buf, k_strlen(buf));
    }
    line_y += 24;

    // Anomalies Discovered
    SetTextColor(hdc, COLOR_CYAN);
    k_strcpy(buf, "Anomalies Cataloged: ");
    k_itoa(g_discovered_count, num_buf);
    k_strcat(buf, num_buf);
    k_strcat(buf, " / 12 Discovered");
    TextOutA(hdc, x + 10, line_y, buf, k_strlen(buf));
}

static void StepFrequency(int delta) {
    g_freq_centikhz += delta;
    if (g_freq_centikhz < 100) g_freq_centikhz = 100;     // 1.00 kHz
    if (g_freq_centikhz > 12000) g_freq_centikhz = 12000; // 120.00 kHz
    CheckSignalLock();
    InvalidateRect(g_hwnd, NULL, FALSE);
}

static void DoTriangulation(void) {
    if (g_discovered_count < 3) {
        AppendLog("[TRIANGULATION ERROR] Requires >= 3 cataloged anomalies for cross-correlation.");
        return;
    }

    AppendLog("[CROSS-CORRELATING SEISMIC PROBES...]");
    AppendLog("  >> Calculating TDOA across BH-09, SB-7F, and MA-01...");
    AppendLog("  >> HYPERBOLIC FIX: Lat: 32.42 N, Lon: 104.23 W, Depth: 3,420m");
    AppendLog("  >> EPICENTER IDENTIFIED: Sub-Bunker 0x7F / Carlsbad Lithosphere Vault");
    AppendLog("  >> ARG NODE MATCH: 10.19.99.4 [Subterranean Darknet Relay Active]");
}

static void QuickSaveState(void) {
    HANDLE hFile = CreateFileA(
        "kanomaly.dat",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) return;

    DWORD written;
    WriteFile(hFile, &g_freq_centikhz, sizeof(g_freq_centikhz), &written, NULL);
    WriteFile(hFile, &g_curr_station, sizeof(g_curr_station), &written, NULL);
    WriteFile(hFile, &g_bandwidth_idx, sizeof(g_bandwidth_idx), &written, NULL);
    WriteFile(hFile, &g_mode_idx, sizeof(g_mode_idx), &written, NULL);
    WriteFile(hFile, &g_discovered_count, sizeof(g_discovered_count), &written, NULL);

    for (int i = 0; i < NUM_ANOMALIES; i++) {
        WriteFile(hFile, &g_anomalies[i].discovered, sizeof(int), &written, NULL);
    }

    CloseHandle(hFile);
    AppendLog("[STATE PERSISTENCE] Quicksave written to kanomaly.dat [F5]");
}

static void QuickLoadState(void) {
    HANDLE hFile = CreateFileA(
        "kanomaly.dat",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        AppendLog("[STATE WARNING] No previous save found in kanomaly.dat");
        return;
    }

    DWORD read;
    ReadFile(hFile, &g_freq_centikhz, sizeof(g_freq_centikhz), &read, NULL);
    ReadFile(hFile, &g_curr_station, sizeof(g_curr_station), &read, NULL);
    ReadFile(hFile, &g_bandwidth_idx, sizeof(g_bandwidth_idx), &read, NULL);
    ReadFile(hFile, &g_mode_idx, sizeof(g_mode_idx), &read, NULL);
    ReadFile(hFile, &g_discovered_count, sizeof(g_discovered_count), &read, NULL);

    for (int i = 0; i < NUM_ANOMALIES; i++) {
        ReadFile(hFile, &g_anomalies[i].discovered, sizeof(int), &read, NULL);
    }

    CloseHandle(hFile);
    CheckSignalLock();
    InvalidateRect(g_hwnd, NULL, FALSE);
    AppendLog("[STATE PERSISTENCE] Session restored from kanomaly.dat [F9]");
}

static void ShowHelp(HWND hwnd) {
    MessageBoxA(
        hwnd,
        "=== KANOMALY: SUBTERRANEAN SIGNAL ANALYZER ===\n\n"
        "Operation: Deep Lithosphere Acoustic & VLF Interception\n\n"
        "CONTROLS & SHORTCUTS:\n"
        "  [F1] Help / Operator Manual\n"
        "  [F5] Quicksave session to kanomaly.dat\n"
        "  [F9] Quickload session from kanomaly.dat\n"
        "  [Space] Toggle Auto-Sweep frequency scan\n"
        "  [<<] / [>>] Tune frequency in +/- 1.00 kHz steps\n"
        "  [<]  / [>]  Tune frequency in +/- 0.10 kHz steps\n"
        "  [Station] Cycle through 6 global subterranean observatories\n"
        "  [Bandwidth] Adjust filter passband (50Hz / 250Hz / 1kHz)\n"
        "  [Mode] Switch demodulator (AM, FSK, CW, SSTV, TEL)\n"
        "  [Triangulate] Compute anomaly epicenter coordinates\n\n"
        "OBJECTIVE:\n"
        "  Tune through the lithospheric noise floor to lock and decode\n"
        "  all 12 subterranean anomalies and uncover the hidden ARG relay!",
        "KAnomaly Operator Manual",
        MB_OK | MB_ICONINFORMATION
    );
}

// Window Procedure
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hBrushBg     = CreateSolidBrush(COLOR_BG);
            g_hBrushPanel  = CreateSolidBrush(COLOR_PANEL);
            g_hBrushBorder = CreateSolidBrush(COLOR_BORDER);

            g_hFontMono  = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
            g_hFontBold  = CreateFontA(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
            g_hFontSmall = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");

            // Buttons Toolbar
            CreateWindowA("BUTTON", "< Stn", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 10, 50, 24, hwnd, (HMENU)ID_BTN_STATION_PREV, NULL, NULL);
            CreateWindowA("BUTTON", "Stn >", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 65, 10, 50, 24, hwnd, (HMENU)ID_BTN_STATION_NEXT, NULL, NULL);
            CreateWindowA("BUTTON", "<< -1k", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 125, 10, 55, 24, hwnd, (HMENU)ID_BTN_FREQ_DOWN_L, NULL, NULL);
            CreateWindowA("BUTTON", "< -0.1k", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 185, 10, 55, 24, hwnd, (HMENU)ID_BTN_FREQ_DOWN_S, NULL, NULL);
            CreateWindowA("BUTTON", "+0.1k >", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 245, 10, 55, 24, hwnd, (HMENU)ID_BTN_FREQ_UP_S, NULL, NULL);
            CreateWindowA("BUTTON", "+1k >>", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 305, 10, 55, 24, hwnd, (HMENU)ID_BTN_FREQ_UP_L, NULL, NULL);
            CreateWindowA("BUTTON", "Sweep", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 370, 10, 60, 24, hwnd, (HMENU)ID_BTN_SWEEP, NULL, NULL);

            CreateWindowA("BUTTON", "Mode", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 440, 10, 55, 24, hwnd, (HMENU)ID_BTN_MODE, NULL, NULL);
            CreateWindowA("BUTTON", "Filter", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 500, 10, 55, 24, hwnd, (HMENU)ID_BTN_BANDWIDTH, NULL, NULL);
            CreateWindowA("BUTTON", "Triangulate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 565, 10, 85, 24, hwnd, (HMENU)ID_BTN_TRIANGULATE, NULL, NULL);
            CreateWindowA("BUTTON", "Save [F5]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 660, 10, 70, 24, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            CreateWindowA("BUTTON", "Load [F9]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 735, 10, 70, 24, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            CreateWindowA("BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 810, 10, 70, 24, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

            // Log Console (Bottom full width)
            g_hEditLog = CreateWindowExA(
                WS_EX_CLIENTEDGE,
                "EDIT",
                "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                10, 390, 870, 180,
                hwnd,
                (HMENU)ID_EDIT_LOG,
                NULL,
                NULL
            );
            SendMessageA(g_hEditLog, WM_SETFONT, (WPARAM)g_hFontSmall, TRUE);

            AppendLog("=== KANOMALY v1.0.0: SUBTERRANEAN SIGNAL ANALYZER INITIALIZED ===");
            AppendLog("Lithosphere Acoustic & Electromagnetic Observatory Network online.");
            AppendLog("Array: 6 Geophone probes calibrated. Ready for RF/Seismic sweep.");

            SetTimer(hwnd, TIMER_TICK, 50, NULL);
            CheckSignalLock();

            // First-run tutorial check
            HANDLE hCheck = CreateFileA("kanomaly_tut.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hCheck == INVALID_HANDLE_VALUE) {
                HANDLE hCreate = CreateFileA("kanomaly_tut.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hCreate != INVALID_HANDLE_VALUE) CloseHandle(hCreate);
                ShowHelp(hwnd);
            } else {
                CloseHandle(hCheck);
            }
            break;
        }

        case WM_TIMER: {
            g_frame_counter++;
            if (g_is_sweeping) {
                g_freq_centikhz += g_sweep_dir * 5;
                if (g_freq_centikhz > 10500) { g_freq_centikhz = 10500; g_sweep_dir = -1; }
                if (g_freq_centikhz < 500)   { g_freq_centikhz = 500;   g_sweep_dir = 1; }
                CheckSignalLock();
            }
            UpdateWaterfall();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Left Display: Waterfall Spectrogram (10, 45, 430, 210)
            DrawWaterfall(hdc, 10, 45, 430, 210);

            // Left Bottom Display: Oscilloscope (10, 265, 430, 115)
            DrawOscilloscope(hdc, 10, 265, 430, 115);

            // Right Display: Telemetry & Cards (450, 45, 430, 335)
            DrawTelemetryCards(hdc, 450, 45, 430, 335);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, COLOR_PHOSPHOR);
            SetBkColor(hdcStatic, COLOR_BG);
            return (LRESULT)g_hBrushBg;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case ID_BTN_STATION_PREV:
                    g_curr_station = (g_curr_station - 1 + NUM_STATIONS) % NUM_STATIONS;
                    CheckSignalLock();
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_STATION_NEXT:
                    g_curr_station = (g_curr_station + 1) % NUM_STATIONS;
                    CheckSignalLock();
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_FREQ_DOWN_L:
                    StepFrequency(-100);
                    break;
                case ID_BTN_FREQ_DOWN_S:
                    StepFrequency(-10);
                    break;
                case ID_BTN_FREQ_UP_S:
                    StepFrequency(10);
                    break;
                case ID_BTN_FREQ_UP_L:
                    StepFrequency(100);
                    break;
                case ID_BTN_SWEEP:
                    g_is_sweeping = !g_is_sweeping;
                    AppendLog(g_is_sweeping ? "[SWEEP] Auto-sweep active." : "[SWEEP] Auto-sweep paused.");
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_MODE:
                    g_mode_idx = (g_mode_idx + 1) % 5;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_BANDWIDTH:
                    g_bandwidth_idx = (g_bandwidth_idx + 1) % 3;
                    CheckSignalLock();
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_TRIANGULATE:
                    DoTriangulation();
                    break;
                case ID_BTN_SAVE:
                    QuickSaveState();
                    break;
                case ID_BTN_LOAD:
                    QuickLoadState();
                    break;
                case ID_BTN_HELP:
                    ShowHelp(hwnd);
                    break;
            }
            break;
        }

        case WM_KEYDOWN: {
            if (wParam == VK_F1) { ShowHelp(hwnd); return 0; }
            if (wParam == VK_F5) { QuickSaveState(); return 0; }
            if (wParam == VK_F9) { QuickLoadState(); return 0; }
            if (wParam == VK_SPACE) {
                g_is_sweeping = !g_is_sweeping;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            break;
        }

        case WM_DESTROY: {
            QuickSaveState();
            KillTimer(hwnd, TIMER_TICK);
            if (g_hBrushBg) DeleteObject(g_hBrushBg);
            if (g_hBrushPanel) DeleteObject(g_hBrushPanel);
            if (g_hBrushBorder) DeleteObject(g_hBrushBorder);
            if (g_hFontMono) DeleteObject(g_hFontMono);
            if (g_hFontBold) DeleteObject(g_hFontBold);
            if (g_hFontSmall) DeleteObject(g_hFontSmall);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Entry Point
void __cdecl MainEntry(void) {
    HINSTANCE hInstance = GetModuleHandleA(NULL);

    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KAnomalyWndClass";
    wc.hbrBackground = CreateSolidBrush(COLOR_BG);
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(1));

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        "KAnomalyWndClass",
        "KAnomaly - Subterranean Signal Analyzer v1.0.0",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 905, 620,
        NULL, NULL, hInstance, NULL
    );

    g_hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1) { ShowHelp(hwnd); continue; }
            if (msg.wParam == VK_F5) { QuickSaveState(); continue; }
            if (msg.wParam == VK_F9) { QuickLoadState(); continue; }
            if (msg.wParam == VK_SPACE) {
                SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_SWEEP, 0), 0);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    ExitProcess(0);
}
