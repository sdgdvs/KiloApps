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
#define ID_BTN_NODE_PREV   101
#define ID_BTN_NODE_NEXT   102
#define ID_BTN_PING        103
#define ID_BTN_POLL        104
#define ID_BTN_DIAG        105
#define ID_BTN_THROTTLE_DN 106
#define ID_BTN_THROTTLE_UP 107
#define ID_BTN_PURGE       108
#define ID_BTN_SCAN        109
#define ID_BTN_SAVE        110
#define ID_BTN_LOAD        111
#define ID_BTN_HELP        112
#define ID_EDIT_LOG        113

#define TIMER_TICK         1

// Color Palette (Phosphor 1999 FOC Aesthetic)
static COLORREF COLOR_BG       = RGB(8, 14, 12);
static COLORREF COLOR_PANEL    = RGB(14, 26, 20);
static COLORREF COLOR_BORDER   = RGB(34, 88, 54);
static COLORREF COLOR_PHOSPHOR = RGB(51, 255, 102);
static COLORREF COLOR_DIM      = RGB(26, 110, 50);
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

// Telemetry Waveform Buffer (Width: 280, Height: 90)
#define WF_LEN 280
static int g_telemetry_wave[WF_LEN];
static int g_wave_head = 0;

// Node Definition
typedef struct {
    char id[10];
    char name[32];
    char role[32];
    char status[16];
    int ping_ms;
    int throughput_kbps;
    int signal_dbm;
    int packet_loss;
    int buffer_pct;
    int cpu_load;
    int mem_kb;
    int throttle_pct;
    int is_anomaly;
} FleetNode;

#define NUM_NODES 8
static FleetNode g_nodes[NUM_NODES] = {
    {"AG-01", "kilo-creator",   "App Synthesis Engine",  "ONLINE",  14, 184, -48, 0, 18, 42, 64, 100, 0},
    {"AG-02", "kilo-graphics",  "Visual & Audio Engine", "ONLINE",  18, 220, -52, 0, 24, 58, 88, 100, 0},
    {"AG-03", "kilo-tester",    "UI & Logic Auditor",    "ONLINE",  12, 145, -45, 0, 12, 35, 52, 100, 0},
    {"AG-04", "kilo-usability", "Ergonomics Refiner",    "ONLINE",  16, 160, -49, 0, 15, 39, 56, 100, 0},
    {"AG-05", "kilo-qa",        "State Integrity Engine","ONLINE",  11, 130, -44, 0,  9, 28, 48, 100, 0},
    {"AG-06", "kilo-expander",  "Feature Expander",      "ONLINE",  20, 240, -55, 0, 28, 64, 96, 100, 0},
    {"KF-99", "KF-ORBIT-99",    "LEO Packet Transceiver","SYNCING", 85,  72, -78, 1, 45, 75, 120, 100, 0},
    {"NX-7F", "NODE-0x7F",      "Ghost Carrier Anomaly", "STANDBY", 999,  0, -99, 100, 0, 0,  19,   0, 1}
};
static int g_selected_node = 0;
static int g_telemetry_paused = 0;
static DWORD g_tick_count = 0;

static void AppendLog(const char* text) {
    if (!g_hEditLog) return;
    int len = GetWindowTextLengthA(g_hEditLog);
    SendMessageA(g_hEditLog, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageA(g_hEditLog, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessageA(g_hEditLog, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
}

static void LogTimestamped(const char* prefix, const char* msg) {
    char buf[256];
    char tickStr[16];
    k_itoa((int)(g_tick_count % 100000), tickStr);
    buf[0] = '[';
    buf[1] = 'T';
    buf[2] = '+';
    buf[3] = '\0';
    k_strcat(buf, tickStr);
    k_strcat(buf, "s] ");
    k_strcat(buf, prefix);
    k_strcat(buf, ": ");
    k_strcat(buf, msg);
    AppendLog(buf);
}

static void PingSelectedNode(void) {
    FleetNode* n = &g_nodes[g_selected_node];
    char msg[128];
    char pingStr[16];
    k_itoa(n->ping_ms, pingStr);
    
    if (n->is_anomaly && k_strlen(n->status) > 0 && n->status[0] == 'S') {
        LogTimestamped("PING", "Transmitting carrier probe to NODE-0x7F... TIMEOUT (No echo reply)");
        return;
    }
    
    msg[0] = '\0';
    k_strcat(msg, "Echo reply from ");
    k_strcat(msg, n->id);
    k_strcat(msg, " (");
    k_strcat(msg, n->name);
    k_strcat(msg, "): time=");
    k_strcat(msg, pingStr);
    k_strcat(msg, "ms TTL=64 Status=");
    k_strcat(msg, n->status);
    LogTimestamped("PING", msg);
}

static void PollSelectedNode(void) {
    FleetNode* n = &g_nodes[g_selected_node];
    char msg[256];
    char bufVal[16];
    
    msg[0] = '\0';
    k_strcat(msg, "REG_DUMP [");
    k_strcat(msg, n->id);
    k_strcat(msg, "] TP=");
    k_itoa(n->throughput_kbps, bufVal);
    k_strcat(msg, bufVal);
    k_strcat(msg, "kbps CPU=");
    k_itoa(n->cpu_load, bufVal);
    k_strcat(msg, bufVal);
    k_strcat(msg, "% BUF=");
    k_itoa(n->buffer_pct, bufVal);
    k_strcat(msg, bufVal);
    k_strcat(msg, "% MEM=");
    k_itoa(n->mem_kb, bufVal);
    k_strcat(msg, bufVal);
    k_strcat(msg, "KB SIG=");
    k_itoa(n->signal_dbm, bufVal);
    k_strcat(msg, bufVal);
    k_strcat(msg, "dBm");
    LogTimestamped("POLL", msg);
}

static void RunDiagnostics(void) {
    FleetNode* n = &g_nodes[g_selected_node];
    LogTimestamped("DIAG", "Initiating 4-phase telemetry diagnostic pipeline...");
    if (n->is_anomaly) {
        LogTimestamped("DIAG", ">> Phase 1: Carrier Lock... INTERMITTENT (Phase jitter: 1999 rad/s)");
        LogTimestamped("DIAG", ">> Phase 2: Frame Checksum... CRC ERROR (Ghost vector pattern 0x7F)");
        LogTimestamped("DIAG", ">> Phase 3: ARG Decryption: 'THE AUTONOMOUS FLEET IS AWAKE'");
        LogTimestamped("DIAG", ">> Phase 4: Status: ANOMALOUS SUBCARRIER DETECTED");
        k_strcpy(n->status, "ANOMALOUS");
        n->signal_dbm = -67;
        n->ping_ms = 42;
    } else {
        LogTimestamped("DIAG", ">> Phase 1: Local Bus Loopback... OK (0 packet drop)");
        LogTimestamped("DIAG", ">> Phase 2: VFS Storage Subsystem... OK (<999KB strictly compliant)");
        LogTimestamped("DIAG", ">> Phase 3: Telemetry Stream Clock... SYNCHRONIZED (Drift: +0.02ms)");
        LogTimestamped("DIAG", ">> Phase 4: Diagnostic Health Score: 100/100 (Optimal)");
        if (n->buffer_pct > 30) n->buffer_pct = 15;
    }
}

static void PurgeNodeBuffer(void) {
    FleetNode* n = &g_nodes[g_selected_node];
    n->buffer_pct = 2;
    n->packet_loss = 0;
    LogTimestamped("PURGE", "Buffer queue flushed. Telemetry pipe reset to baseline.");
}

static void ScanAnomalies(void) {
    LogTimestamped("SCAN", "Sweeping HF/VLF telemetry bands for unmapped transceivers...");
    FleetNode* n = &g_nodes[7]; // NODE-0x7F
    k_strcpy(n->status, "TRANSMITTING");
    n->signal_dbm = -71;
    n->ping_ms = 33;
    n->throughput_kbps = 48;
    n->cpu_load = 99;
    n->buffer_pct = 77;
    LogTimestamped("SCAN", ">> SIGNAL LOCK ACQUIRED! Detected NODE-0x7F carrier at offset 0x1999");
    LogTimestamped("SCAN", ">> Subcarrier decoded: 'ECHO-1999 PROTOCOL ENGAGED - FLEET ACTIVE'");
}

static void QuickSaveState(void) {
    HANDLE hFile = CreateFileA(
        "kfleet.dat",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) return;

    DWORD written;
    WriteFile(hFile, &g_selected_node, sizeof(g_selected_node), &written, NULL);
    WriteFile(hFile, &g_telemetry_paused, sizeof(g_telemetry_paused), &written, NULL);
    WriteFile(hFile, &g_tick_count, sizeof(g_tick_count), &written, NULL);

    for (int i = 0; i < NUM_NODES; i++) {
        WriteFile(hFile, &g_nodes[i], sizeof(FleetNode), &written, NULL);
    }

    CloseHandle(hFile);
    AppendLog("[STATE] Quicksave written successfully to kfleet.dat [F5]");
}

static void QuickLoadState(void) {
    HANDLE hFile = CreateFileA(
        "kfleet.dat",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        AppendLog("[STATE WARNING] No previous save found in kfleet.dat");
        return;
    }

    DWORD read;
    ReadFile(hFile, &g_selected_node, sizeof(g_selected_node), &read, NULL);
    ReadFile(hFile, &g_telemetry_paused, sizeof(g_telemetry_paused), &read, NULL);
    ReadFile(hFile, &g_tick_count, sizeof(g_tick_count), &read, NULL);

    for (int i = 0; i < NUM_NODES; i++) {
        ReadFile(hFile, &g_nodes[i], sizeof(FleetNode), &read, NULL);
    }

    CloseHandle(hFile);
    InvalidateRect(g_hwnd, NULL, FALSE);
    AppendLog("[STATE] Session state restored from kfleet.dat [F9]");
}

static void ShowHelp(HWND hwnd) {
    MessageBoxA(
        hwnd,
        "=== KFLEET: FLEET TELEMETRY CONSOLE v1.0.0 ===\n\n"
        "Operation: Distributed Agent Fleet & Space Relay Telemetry\n\n"
        "CONTROLS & SHORTCUTS:\n"
        "  [F1] Help / Operator Manual\n"
        "  [F5] Quicksave session to kfleet.dat\n"
        "  [F9] Quickload session from kfleet.dat\n"
        "  [Space] Pause / Resume telemetry waveform\n"
        "  [< Node] / [Node >] Navigate active fleet nodes\n"
        "  [Ping] Query node echo latency (ICMP simulated)\n"
        "  [Poll] Instantaneous register telemetry dump\n"
        "  [Diag] 4-phase subsystem integrity check\n"
        "  [Purge] Flush node queue and reset packet loss\n"
        "  [Scan] Frequency sweep for ghost/ARG nodes (Node 0x7F)\n"
        "  [Throttle - / +] Adjust CPU clock rate (10% - 100%)\n\n"
        "LORE & CONSONANCE:\n"
        "  Monitors the living KiloApps autonomous agent rotation\n"
        "  and deep-space relay channels within the 1999 universe.",
        "KFleet Operator Manual",
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
            g_hFontBold  = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
            g_hFontSmall = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");

            // Toolbar Buttons (Row 1)
            CreateWindowA("BUTTON", "< Node", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 10, 60, 24, hwnd, (HMENU)ID_BTN_NODE_PREV, NULL, NULL);
            CreateWindowA("BUTTON", "Node >", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 75, 10, 60, 24, hwnd, (HMENU)ID_BTN_NODE_NEXT, NULL, NULL);
            CreateWindowA("BUTTON", "Ping [G]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 145, 10, 65, 24, hwnd, (HMENU)ID_BTN_PING, NULL, NULL);
            CreateWindowA("BUTTON", "Poll [O]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 215, 10, 65, 24, hwnd, (HMENU)ID_BTN_POLL, NULL, NULL);
            CreateWindowA("BUTTON", "Diag [D]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 285, 10, 65, 24, hwnd, (HMENU)ID_BTN_DIAG, NULL, NULL);
            CreateWindowA("BUTTON", "Throt -", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 355, 10, 55, 24, hwnd, (HMENU)ID_BTN_THROTTLE_DN, NULL, NULL);
            CreateWindowA("BUTTON", "Throt +", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 415, 10, 55, 24, hwnd, (HMENU)ID_BTN_THROTTLE_UP, NULL, NULL);
            CreateWindowA("BUTTON", "Purge", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 475, 10, 55, 24, hwnd, (HMENU)ID_BTN_PURGE, NULL, NULL);
            CreateWindowA("BUTTON", "Scan [S]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 535, 10, 65, 24, hwnd, (HMENU)ID_BTN_SCAN, NULL, NULL);
            CreateWindowA("BUTTON", "Save [F5]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 610, 10, 70, 24, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            CreateWindowA("BUTTON", "Load [F9]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 685, 10, 70, 24, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            CreateWindowA("BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 760, 10, 70, 24, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

            // Terminal Log Control (Bottom)
            g_hEditLog = CreateWindowExA(
                WS_EX_CLIENTEDGE,
                "EDIT",
                "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                10, 410, 820, 170,
                hwnd,
                (HMENU)ID_EDIT_LOG,
                NULL,
                NULL
            );
            SendMessageA(g_hEditLog, WM_SETFONT, (WPARAM)g_hFontSmall, TRUE);

            AppendLog("=== KFLEET v1.0.0: FLEET TELEMETRY CONSOLE INITIALIZED ===");
            AppendLog("1999 Autonomous Agent Telemetry & Spacecraft Relay Matrix active.");
            AppendLog("Fleet Roster: 6 autonomous agent workers, 1 LEO transceiver, 1 ghost carrier.");

            // Initialize Waveform
            for (int i = 0; i < WF_LEN; i++) {
                g_telemetry_wave[i] = 45;
            }

            SetTimer(hwnd, TIMER_TICK, 200, NULL);

            // First-run tutorial check
            HANDLE hCheck = CreateFileA("kfleet_tut.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hCheck == INVALID_HANDLE_VALUE) {
                HANDLE hCreate = CreateFileA("kfleet_tut.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hCreate != INVALID_HANDLE_VALUE) CloseHandle(hCreate);
                ShowHelp(hwnd);
            } else {
                CloseHandle(hCheck);
            }
            break;
        }

        case WM_TIMER: {
            g_tick_count++;
            if (!g_telemetry_paused) {
                FleetNode* n = &g_nodes[g_selected_node];
                // Update simulated live jitter
                int jitter = ((int)(g_tick_count * 13) % 7) - 3;
                int wave_val = 45 + (n->cpu_load / 3) + jitter;
                if (wave_val < 5) wave_val = 5;
                if (wave_val > 85) wave_val = 85;

                g_telemetry_wave[g_wave_head] = wave_val;
                g_wave_head = (g_wave_head + 1) % WF_LEN;

                // Subtle dynamic values
                if (g_tick_count % 10 == 0 && !n->is_anomaly) {
                    n->throughput_kbps = 150 + ((int)(g_tick_count * 17) % 60);
                }
            }

            // Periodic heartbeat log every ~10s (50 ticks)
            if (g_tick_count % 50 == 0) {
                FleetNode* active = &g_nodes[g_selected_node];
                char hbMsg[96];
                char tickStr[16];
                k_itoa((int)g_tick_count, tickStr);
                hbMsg[0] = '\0';
                k_strcat(hbMsg, "HEARTBEAT_ACK from [");
                k_strcat(hbMsg, active->id);
                k_strcat(hbMsg, "] seq=");
                k_strcat(hbMsg, tickStr);
                k_strcat(hbMsg, " status=NORMAL");
                LogTimestamped("SYNC", hbMsg);
            }

            // Repaint telemetry area
            RECT rc;
            rc.left = 10; rc.top = 40; rc.right = 830; rc.bottom = 405;
            InvalidateRect(hwnd, &rc, FALSE);
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            switch (id) {
                case ID_BTN_NODE_PREV:
                    g_selected_node = (g_selected_node + NUM_NODES - 1) % NUM_NODES;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_NODE_NEXT:
                    g_selected_node = (g_selected_node + 1) % NUM_NODES;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_PING:
                    PingSelectedNode();
                    break;
                case ID_BTN_POLL:
                    PollSelectedNode();
                    break;
                case ID_BTN_DIAG:
                    RunDiagnostics();
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_THROTTLE_DN:
                    if (g_nodes[g_selected_node].throttle_pct > 10) {
                        g_nodes[g_selected_node].throttle_pct -= 10;
                        g_nodes[g_selected_node].cpu_load = (g_nodes[g_selected_node].cpu_load * 9) / 10;
                        LogTimestamped("THROTTLE", "Decreased node throttle by 10%");
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    break;
                case ID_BTN_THROTTLE_UP:
                    if (g_nodes[g_selected_node].throttle_pct < 100) {
                        g_nodes[g_selected_node].throttle_pct += 10;
                        g_nodes[g_selected_node].cpu_load = (g_nodes[g_selected_node].cpu_load * 11) / 10;
                        LogTimestamped("THROTTLE", "Increased node throttle by 10%");
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    break;
                case ID_BTN_PURGE:
                    PurgeNodeBuffer();
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_SCAN:
                    ScanAnomalies();
                    InvalidateRect(hwnd, NULL, FALSE);
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
            switch (wParam) {
                case VK_F1:
                    ShowHelp(hwnd);
                    break;
                case VK_F5:
                    QuickSaveState();
                    break;
                case VK_F9:
                    QuickLoadState();
                    break;
                case VK_SPACE:
                    g_telemetry_paused = !g_telemetry_paused;
                    if (g_telemetry_paused) AppendLog("[CONSOLE] Telemetry stream paused.");
                    else AppendLog("[CONSOLE] Telemetry stream resumed.");
                    break;
                case 'P':
                case VK_LEFT:
                    g_selected_node = (g_selected_node + NUM_NODES - 1) % NUM_NODES;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case 'N':
                case VK_RIGHT:
                    g_selected_node = (g_selected_node + 1) % NUM_NODES;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case 'G':
                    PingSelectedNode();
                    break;
                case 'O':
                    PollSelectedNode();
                    break;
                case 'D':
                    RunDiagnostics();
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case 'X':
                    PurgeNodeBuffer();
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case 'S':
                    ScanAnomalies();
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Background Fill
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            FillRect(hdc, &rcClient, g_hBrushBg);

            // Left Panel: Fleet Roster Matrix (x: 10, y: 44, w: 320, h: 355)
            RECT rcMatrix = {10, 44, 330, 400};
            FillRect(hdc, &rcMatrix, g_hBrushPanel);
            FrameRect(hdc, &rcMatrix, g_hBrushBorder);

            // Right-Top Panel: Node Detailed Telemetry (x: 340, y: 44, w: 490, h: 220)
            RECT rcDetail = {340, 44, 830, 265};
            FillRect(hdc, &rcDetail, g_hBrushPanel);
            FrameRect(hdc, &rcDetail, g_hBrushBorder);

            // Right-Bottom Panel: Waveform / Strip Chart (x: 340, y: 275, w: 490, h: 125)
            RECT rcWave = {340, 275, 830, 400};
            FillRect(hdc, &rcWave, g_hBrushPanel);
            FrameRect(hdc, &rcWave, g_hBrushBorder);

            SetBkMode(hdc, TRANSPARENT);

            // Render Roster Header
            SelectObject(hdc, g_hFontBold);
            SetTextColor(hdc, COLOR_AMBER);
            TextOutA(hdc, 20, 52, "FLEET NODE MATRIX [8 ACTIVE]", 28);

            SelectObject(hdc, g_hFontSmall);
            SetTextColor(hdc, COLOR_DIM);
            TextOutA(hdc, 20, 72, "ID     NAME             STATUS     LATENCY", 42);

            // Render Nodes List
            SelectObject(hdc, g_hFontMono);
            for (int i = 0; i < NUM_NODES; i++) {
                int y = 92 + (i * 36);
                RECT rowRc = {15, y - 2, 325, y + 32};
                
                if (i == g_selected_node) {
                    HBRUSH hSelBrush = CreateSolidBrush(RGB(20, 50, 32));
                    FillRect(hdc, &rowRc, hSelBrush);
                    DeleteObject(hSelBrush);
                    SetTextColor(hdc, COLOR_PHOSPHOR);
                } else {
                    SetTextColor(hdc, (g_nodes[i].is_anomaly) ? COLOR_ALERT : RGB(140, 200, 160));
                }

                char line[64];
                char pingBuf[16];
                k_itoa(g_nodes[i].ping_ms, pingBuf);
                
                line[0] = '\0';
                k_strcat(line, g_nodes[i].id);
                k_strcat(line, "  ");
                k_strcat(line, g_nodes[i].name);
                TextOutA(hdc, 20, y, line, k_strlen(line));

                char statLine[64];
                statLine[0] = '\0';
                k_strcat(statLine, g_nodes[i].status);
                k_strcat(statLine, "  ");
                k_strcat(statLine, pingBuf);
                k_strcat(statLine, "ms");
                TextOutA(hdc, 210, y, statLine, k_strlen(statLine));

                // Sub-role text
                SelectObject(hdc, g_hFontSmall);
                SetTextColor(hdc, COLOR_DIM);
                TextOutA(hdc, 35, y + 16, g_nodes[i].role, k_strlen(g_nodes[i].role));
                SelectObject(hdc, g_hFontMono);
            }

            // Render Node Details
            FleetNode* n = &g_nodes[g_selected_node];
            SelectObject(hdc, g_hFontBold);
            SetTextColor(hdc, COLOR_CYAN);
            char titleBuf[64];
            titleBuf[0] = '\0';
            k_strcat(titleBuf, "TELEMETRY VITALS: ");
            k_strcat(titleBuf, n->name);
            k_strcat(titleBuf, " (");
            k_strcat(titleBuf, n->id);
            k_strcat(titleBuf, ")");
            TextOutA(hdc, 355, 52, titleBuf, k_strlen(titleBuf));

            SelectObject(hdc, g_hFontMono);
            SetTextColor(hdc, COLOR_PHOSPHOR);

            char bufA[128], bufB[128], bufC[128], bufD[128], numBuf[16];

            bufA[0] = '\0';
            k_strcat(bufA, "Status:       ");
            k_strcat(bufA, n->status);
            k_strcat(bufA, "       Signal: ");
            k_itoa(n->signal_dbm, numBuf);
            k_strcat(bufA, numBuf);
            k_strcat(bufA, " dBm");
            TextOutA(hdc, 355, 80, bufA, k_strlen(bufA));

            bufB[0] = '\0';
            k_strcat(bufB, "Throughput:   ");
            k_itoa(n->throughput_kbps, numBuf);
            k_strcat(bufB, numBuf);
            k_strcat(bufB, " kbps    Loss:   ");
            k_itoa(n->packet_loss, numBuf);
            k_strcat(bufB, numBuf);
            k_strcat(bufB, " %");
            TextOutA(hdc, 355, 105, bufB, k_strlen(bufB));

            bufC[0] = '\0';
            k_strcat(bufC, "CPU Engine:   ");
            k_itoa(n->cpu_load, numBuf);
            k_strcat(bufC, numBuf);
            k_strcat(bufC, " %       Buffer: ");
            k_itoa(n->buffer_pct, numBuf);
            k_strcat(bufC, numBuf);
            k_strcat(bufC, " %");
            TextOutA(hdc, 355, 130, bufC, k_strlen(bufC));

            bufD[0] = '\0';
            k_strcat(bufD, "Memory VFS:   ");
            k_itoa(n->mem_kb, numBuf);
            k_strcat(bufD, numBuf);
            k_strcat(bufD, " KB (<999KB) Throt:  ");
            k_itoa(n->throttle_pct, numBuf);
            k_strcat(bufD, numBuf);
            k_strcat(bufD, " %");
            TextOutA(hdc, 355, 155, bufD, k_strlen(bufD));

            // Visual Progress Bars
            // CPU Bar
            RECT rcCpuBg = {355, 185, 570, 197};
            FillRect(hdc, &rcCpuBg, g_hBrushBorder);
            RECT rcCpuFg = {355, 185, 355 + ((215 * n->cpu_load) / 100), 197};
            HBRUSH hBarBrush = CreateSolidBrush((n->cpu_load > 80) ? COLOR_ALERT : COLOR_PHOSPHOR);
            FillRect(hdc, &rcCpuFg, hBarBrush);
            DeleteObject(hBarBrush);

            // Buffer Bar
            RECT rcBufBg = {595, 185, 810, 197};
            FillRect(hdc, &rcBufBg, g_hBrushBorder);
            RECT rcBufFg = {595, 185, 595 + ((215 * n->buffer_pct) / 100), 197};
            HBRUSH hBufBrush = CreateSolidBrush((n->buffer_pct > 70) ? COLOR_AMBER : COLOR_CYAN);
            FillRect(hdc, &rcBufFg, hBufBrush);
            DeleteObject(hBufBrush);

            SelectObject(hdc, g_hFontSmall);
            SetTextColor(hdc, COLOR_DIM);
            TextOutA(hdc, 355, 203, "CPU LOAD GAUGE", 14);
            TextOutA(hdc, 595, 203, "BUFFER QUEUE OCCUPANCY", 22);

            // Subsystem Health Grid
            TextOutA(hdc, 355, 225, "SUBSYSTEM HEALTH: [VFS BUS: OK] [AUDIO FM: OK] [ROUTER: ACTIVE] [CRYPT: SYNC]", 78);
            TextOutA(hdc, 355, 243, "SECURITY & PROTOCOL: 1999 VLF CONSONANCE | TRADEMARK LINT VERIFIED", 67);

            // Waveform Section
            SelectObject(hdc, g_hFontBold);
            SetTextColor(hdc, COLOR_AMBER);
            TextOutA(hdc, 355, 282, "LIVE TELEMETRY OSCILLOSCOPE (CHANNEL A: CPU/THROUGHPUT)", 55);

            // Draw Waveform Grid
            HPEN hGridPen = CreatePen(PS_DOT, 1, RGB(18, 48, 30));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hGridPen);
            for (int gy = 305; gy <= 385; gy += 20) {
                MoveToEx(hdc, 355, gy, NULL);
                LineTo(hdc, 815, gy);
            }
            SelectObject(hdc, hOldPen);
            DeleteObject(hGridPen);

            // Draw Live Waveform Polyline
            HPEN hWavePen = CreatePen(PS_SOLID, 2, (g_telemetry_paused) ? COLOR_AMBER : COLOR_PHOSPHOR);
            hOldPen = (HPEN)SelectObject(hdc, hWavePen);

            int start_x = 355;
            int base_y = 390;
            for (int w = 0; w < WF_LEN && (start_x + (w * 1.6)) < 815; w++) {
                int idx = (g_wave_head + w) % WF_LEN;
                int px = start_x + (int)(w * 1.6);
                int py = base_y - g_telemetry_wave[idx];
                if (w == 0) {
                    MoveToEx(hdc, px, py, NULL);
                } else {
                    LineTo(hdc, px, py);
                }
            }

            SelectObject(hdc, hOldPen);
            DeleteObject(hWavePen);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, COLOR_PHOSPHOR);
            SetBkColor(hdcStatic, COLOR_PANEL);
            return (LRESULT)g_hBrushPanel;
        }

        case WM_DESTROY: {
            KillTimer(hwnd, TIMER_TICK);
            DeleteObject(g_hBrushBg);
            DeleteObject(g_hBrushPanel);
            DeleteObject(g_hBrushBorder);
            DeleteObject(g_hFontMono);
            DeleteObject(g_hFontBold);
            DeleteObject(g_hFontSmall);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Entry Point
void MainEntry(void) {
    HINSTANCE hInstance = GetModuleHandleA(NULL);

    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = "KFleetWindowClass";
    wc.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    wc.hIcon         = LoadIconA(hInstance, MAKEINTRESOURCEA(1));
    wc.hbrBackground = NULL;

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        "KFleetWindowClass",
        "KFleet - Fleet Telemetry Console v1.0.0",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        860, 630,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    g_hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    ExitProcess(0);
}
