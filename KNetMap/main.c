#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

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
    *dst = 0;
}

static int k_strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

static void k_strcat(char* dst, const char* src) {
    while (*dst) dst++;
    while (*src) *dst++ = *src++;
    *dst = 0;
}

static int k_atoi(const char* s) {
    int v = 0;
    while (*s >= '0' && *s <= '9') {
        v = v * 10 + (*s - '0');
        s++;
    }
    return v;
}

static void k_itoa(int n, char* s) {
    int i = 0, sign = n;
    if (n < 0) n = -n;
    do {
        s[i++] = (char)(n % 10 + '0');
    } while ((n /= 10) > 0);
    if (sign < 0) s[i++] = '-';
    s[i] = '\0';
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char temp = s[j];
        s[j] = s[k];
        s[k] = temp;
    }
}

// Window Dimensions
#define W 980
#define H 680
#define CANVAS_W 660
#define CANVAS_H 620

// Control IDs
#define ID_BTN_ADD_ROUTER      101
#define ID_BTN_ADD_SWITCH      102
#define ID_BTN_ADD_FIREWALL    103
#define ID_BTN_ADD_SERVER      104
#define ID_BTN_ADD_WORKSTATION 105
#define ID_BTN_CONNECT         106
#define ID_BTN_PING            107
#define ID_BTN_CUT             108
#define ID_BTN_CLEAR           109
#define ID_BTN_PRESET_CORP     110
#define ID_BTN_PRESET_ISP      111
#define ID_BTN_HELP            112
#define ID_EDIT_IP             113
#define ID_EDIT_CIDR           114
#define ID_BTN_CALC            115
#define ID_EDIT_LOG            116

// Colors
static COLORREF COL_BG = RGB(8, 13, 25);
static COLORREF COL_PANEL = RGB(14, 23, 42);
static COLORREF COL_CYAN = RGB(0, 240, 255);
static COLORREF COL_EMERALD = RGB(16, 185, 129);
static COLORREF COL_AMBER = RGB(245, 158, 11);
static COLORREF COL_ROSE = RGB(244, 63, 94);
static COLORREF COL_PURPLE = RGB(168, 85, 247);
static COLORREF COL_TEXT = RGB(226, 232, 240);
static COLORREF COL_TEXT_MUTED = RGB(148, 163, 184);

// Topology structures
#define MAX_NODES 32
#define MAX_LINKS 64
#define MAX_PACKETS 16

typedef enum {
    NODE_ROUTER,
    NODE_SWITCH,
    NODE_FIREWALL,
    NODE_SERVER,
    NODE_WORKSTATION
} NodeType;

typedef struct {
    int id;
    NodeType type;
    char name[32];
    char ip[20];
    int x;
    int y;
    int status; // 1 = up, 0 = down
} NetNode;

typedef struct {
    int id;
    int fromNode;
    int toNode;
    int type; // 0=100BaseTX, 1=10BaseT, 2=Fiber, 3=Serial
    int status; // 1 = up, 0 = severed
    int latency;
} NetLink;

typedef struct {
    int active;
    int fromNode;
    int toNode;
    float progress;
    float speed;
    int protocol; // 0=ICMP, 1=TCP
} NetPacket;

static NetNode g_nodes[MAX_NODES];
static int g_nodeCount = 0;
static NetLink g_links[MAX_LINKS];
static int g_linkCount = 0;
static NetPacket g_packets[MAX_PACKETS];

// Interaction state
static int g_selectedNode = -1;
static int g_draggedNode = -1;
static int g_dragOffsetX = 0;
static int g_dragOffsetY = 0;
static int g_connectStart = -1;
static int g_pingSource = -1;
static int g_activeMode = 0; // 0=select, 1=connect, 2=ping, 3=cut

// UI Controls
static HWND g_hwndMain = NULL;
static HWND g_hEditLog = NULL;
static HWND g_hEditIp = NULL;
static HWND g_hEditCidr = NULL;
static HFONT g_hFont = NULL;
static HFONT g_hFontMono = NULL;
static HBRUSH g_hBrushBg = NULL;
static HBRUSH g_hBrushPanel = NULL;

static void LogMessage(const char* msg) {
    if (!g_hEditLog) return;
    int len = GetWindowTextLengthA(g_hEditLog);
    SendMessageA(g_hEditLog, EM_SETSEL, len, len);
    SendMessageA(g_hEditLog, EM_REPLACESEL, FALSE, (LPARAM)msg);
    SendMessageA(g_hEditLog, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
    SendMessageA(g_hEditLog, EM_SCROLLCARET, 0, 0);
}

static void AddNode(NodeType type, int x, int y, const char* name, const char* ip) {
    if (g_nodeCount >= MAX_NODES) return;
    NetNode* n = &g_nodes[g_nodeCount++];
    n->id = g_nodeCount;
    n->type = type;
    n->x = x;
    n->y = y;
    n->status = 1;
    if (name) k_strcpy(n->name, name);
    else {
        char buf[32];
        k_strcpy(buf, type == NODE_ROUTER ? "CYBER-7200" :
                      type == NODE_SWITCH ? "3CON-SW" :
                      type == NODE_FIREWALL ? "BASTION-FW" :
                      type == NODE_SERVER ? "SOL-SRV" : "WIN99-WS");
        k_strcpy(n->name, buf);
    }
    if (ip) k_strcpy(n->ip, ip);
    else {
        char buf[32] = "192.168.1.";
        char num[8];
        k_itoa(10 + g_nodeCount, num);
        k_strcat(buf, num);
        k_strcpy(n->ip, buf);
    }
}

static void AddLink(int fromIdx, int toIdx, int type) {
    if (g_linkCount >= MAX_LINKS) return;
    for (int i = 0; i < g_linkCount; i++) {
        if ((g_links[i].fromNode == fromIdx && g_links[i].toNode == toIdx) ||
            (g_links[i].fromNode == toIdx && g_links[i].toNode == fromIdx)) {
            return; // already linked
        }
    }
    NetLink* l = &g_links[g_linkCount++];
    l->id = g_linkCount;
    l->fromNode = fromIdx;
    l->toNode = toIdx;
    l->type = type;
    l->status = 1;
    l->latency = (type == 2) ? 1 : (type == 3) ? 15 : (type == 1) ? 6 : 2;
}

static void ClearTopology(void) {
    g_nodeCount = 0;
    g_linkCount = 0;
    g_selectedNode = -1;
    g_draggedNode = -1;
    g_connectStart = -1;
    g_pingSource = -1;
    for (int i = 0; i < MAX_PACKETS; i++) g_packets[i].active = 0;
}

static void LoadPresetCorp(void) {
    ClearTopology();
    AddNode(NODE_ROUTER, 240, 90, "GW-CYBER-7200", "198.51.100.1");
    AddNode(NODE_FIREWALL, 240, 180, "FW-BASTION", "198.51.100.2");
    AddNode(NODE_SWITCH, 140, 280, "SW-DMZ-3CON", "198.51.100.10");
    AddNode(NODE_SWITCH, 340, 280, "SW-ENG-3CON", "192.168.10.1");
    AddNode(NODE_SERVER, 80, 400, "SOL-APACHE", "198.51.100.20");
    AddNode(NODE_SERVER, 180, 400, "SOL-MAIL", "198.51.100.21");
    AddNode(NODE_WORKSTATION, 300, 400, "WS-DEV-01", "192.168.10.101");
    AddNode(NODE_WORKSTATION, 400, 400, "WS-DEV-02", "192.168.10.102");

    AddLink(0, 1, 2); // Router to Firewall (Fiber)
    AddLink(1, 2, 0); // Firewall to DMZ Switch (100BaseTX)
    AddLink(1, 3, 0); // Firewall to Eng Switch (100BaseTX)
    AddLink(2, 4, 0); // DMZ to Web
    AddLink(2, 5, 0); // DMZ to Mail
    AddLink(3, 6, 0); // Eng to WS1
    AddLink(3, 7, 0); // Eng to WS2

    LogMessage("[PRESET] Loaded 1999 Corporate DMZ & Multi-Subnet Architecture.");
}

static void LoadPresetIsp(void) {
    ClearTopology();
    AddNode(NODE_ROUTER, 140, 120, "CORE-NYC-01", "10.0.1.1");
    AddNode(NODE_ROUTER, 380, 120, "CORE-CHI-02", "10.0.2.1");
    AddNode(NODE_ROUTER, 380, 360, "CORE-SFO-03", "10.0.3.1");
    AddNode(NODE_ROUTER, 140, 360, "CORE-ATL-04", "10.0.4.1");

    AddLink(0, 1, 2); // Fiber
    AddLink(1, 2, 2); // Fiber
    AddLink(2, 3, 2); // Fiber
    AddLink(3, 0, 2); // Fiber
    AddLink(0, 2, 3); // Serial T1 diagonal failover

    LogMessage("[PRESET] Loaded Multi-Homed ISP Autonomous System Backbone.");
}

static void DispatchPacket(int fromIdx, int toIdx) {
    for (int i = 0; i < MAX_PACKETS; i++) {
        if (!g_packets[i].active) {
            g_packets[i].active = 1;
            g_packets[i].fromNode = fromIdx;
            g_packets[i].toNode = toIdx;
            g_packets[i].progress = 0.0f;
            g_packets[i].speed = 0.04f;
            g_packets[i].protocol = 0; // ICMP
            char buf[128];
            char num[16];
            k_strcpy(buf, "[PING] Sent ICMP Echo Request from ");
            k_strcat(buf, g_nodes[fromIdx].name);
            k_strcat(buf, " to ");
            k_strcat(buf, g_nodes[toIdx].name);
            LogMessage(buf);
            MessageBeep(MB_OK);
            return;
        }
    }
}

static int HitTestNode(int x, int y) {
    for (int i = g_nodeCount - 1; i >= 0; i--) {
        int dx = g_nodes[i].x - x;
        int dy = g_nodes[i].y - y;
        if (dx * dx + dy * dy <= 22 * 22) {
            return i;
        }
    }
    return -1;
}

static void CalculateSubnet(void) {
    char ipBuf[32];
    char cidrBuf[16];
    GetWindowTextA(g_hEditIp, ipBuf, sizeof(ipBuf));
    GetWindowTextA(g_hEditCidr, cidrBuf, sizeof(cidrBuf));

    int bits = k_atoi(cidrBuf);
    if (bits < 1 || bits > 30) bits = 24;

    int total = 1 << (32 - bits);
    int usable = total >= 2 ? total - 2 : 1;

    char msg[256];
    char num[16];
    k_strcpy(msg, "[SUBNET] Calculated CIDR /");
    k_itoa(bits, num);
    k_strcat(msg, num);
    k_strcat(msg, " -> Total IPs: ");
    k_itoa(total, num);
    k_strcat(msg, num);
    k_strcat(msg, ", Usable Hosts: ");
    k_itoa(usable, num);
    k_strcat(msg, num);
    LogMessage(msg);
}

static void DrawTopologyCanvas(HDC hdc) {
    // Create double-buffer
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdc, CANVAS_W, CANVAS_H);
    HGDIOBJ hOldBm = SelectObject(hdcMem, hbmMem);

    // Canvas background
    RECT rc = { 0, 0, CANVAS_W, CANVAS_H };
    HBRUSH hBrBg = CreateSolidBrush(COL_BG);
    FillRect(hdcMem, &rc, hBrBg);
    DeleteObject(hBrBg);

    // Draw Grid
    HPEN hPenGrid = CreatePen(PS_SOLID, 1, RGB(18, 28, 48));
    HGDIOBJ hOldPen = SelectObject(hdcMem, hPenGrid);
    for (int x = 0; x < CANVAS_W; x += 40) {
        MoveToEx(hdcMem, x, 0, NULL);
        LineTo(hdcMem, x, CANVAS_H);
    }
    for (int y = 0; y < CANVAS_H; y += 40) {
        MoveToEx(hdcMem, 0, y, NULL);
        LineTo(hdcMem, CANVAS_W, y);
    }
    SelectObject(hdcMem, hOldPen);
    DeleteObject(hPenGrid);

    // Draw Links
    for (int i = 0; i < g_linkCount; i++) {
        NetLink* l = &g_links[i];
        NetNode* n1 = &g_nodes[l->fromNode];
        NetNode* n2 = &g_nodes[l->toNode];

        COLORREF linkCol = (l->type == 2) ? COL_PURPLE : (l->type == 3) ? COL_EMERALD : (l->type == 1) ? COL_AMBER : COL_CYAN;
        int style = (l->status == 0) ? PS_DOT : PS_SOLID;
        if (l->status == 0) linkCol = COL_ROSE;

        HPEN hPenLink = CreatePen(style, 2, linkCol);
        hOldPen = SelectObject(hdcMem, hPenLink);
        MoveToEx(hdcMem, n1->x, n1->y, NULL);
        LineTo(hdcMem, n2->x, n2->y);
        SelectObject(hdcMem, hOldPen);
        DeleteObject(hPenLink);
    }

    // Draw Packets
    for (int i = 0; i < MAX_PACKETS; i++) {
        if (g_packets[i].active) {
            NetNode* n1 = &g_nodes[g_packets[i].fromNode];
            NetNode* n2 = &g_nodes[g_packets[i].toNode];
            float p = g_packets[i].progress;
            int px = (int)(n1->x + (n2->x - n1->x) * p);
            int py = (int)(n1->y + (n2->y - n1->y) * p);

            HBRUSH hBrPkt = CreateSolidBrush(COL_CYAN);
            HGDIOBJ hOldBr = SelectObject(hdcMem, hBrPkt);
            Ellipse(hdcMem, px - 5, py - 5, px + 5, py + 5);
            SelectObject(hdcMem, hOldBr);
            DeleteObject(hBrPkt);
        }
    }

    // Draw Nodes
    HFONT hOldFont = (HFONT)SelectObject(hdcMem, g_hFontMono);
    SetBkMode(hdcMem, TRANSPARENT);

    for (int i = 0; i < g_nodeCount; i++) {
        NetNode* n = &g_nodes[i];
        COLORREF nodeCol = (n->type == NODE_ROUTER) ? RGB(12, 74, 110) :
                           (n->type == NODE_SWITCH) ? RGB(20, 83, 45) :
                           (n->type == NODE_FIREWALL) ? RGB(136, 19, 55) :
                           (n->type == NODE_SERVER) ? RGB(88, 28, 135) : RGB(30, 41, 59);

        COLORREF borderCol = (i == g_selectedNode) ? COL_CYAN : (n->type == NODE_ROUTER) ? COL_CYAN :
                             (n->type == NODE_SWITCH) ? COL_EMERALD :
                             (n->type == NODE_FIREWALL) ? COL_ROSE : COL_TEXT_MUTED;

        HBRUSH hBrNode = CreateSolidBrush(nodeCol);
        HPEN hPenNode = CreatePen(PS_SOLID, (i == g_selectedNode) ? 3 : 2, borderCol);
        HGDIOBJ hOldBr = SelectObject(hdcMem, hBrNode);
        hOldPen = SelectObject(hdcMem, hPenNode);

        Ellipse(hdcMem, n->x - 16, n->y - 16, n->x + 16, n->y + 16);

        SelectObject(hdcMem, hOldPen);
        SelectObject(hdcMem, hOldBr);
        DeleteObject(hPenNode);
        DeleteObject(hBrNode);

        // Labels
        SetTextColor(hdcMem, COL_TEXT);
        TextOutA(hdcMem, n->x - 30, n->y + 20, n->name, k_strlen(n->name));
        SetTextColor(hdcMem, COL_TEXT_MUTED);
        TextOutA(hdcMem, n->x - 30, n->y + 34, n->ip, k_strlen(n->ip));
    }

    SelectObject(hdcMem, hOldFont);

    // Blit to target HDC
    BitBlt(hdc, 10, 10, CANVAS_W, CANVAS_H, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, hOldBm);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hwndMain = hwnd;
            g_hFont = CreateFontA(-11, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, ANSI_CHARSET, 0, 0, DEFAULT_QUALITY, 0, "Segoe UI");
            g_hFontMono = CreateFontA(-11, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 0, 0, DEFAULT_QUALITY, 0, "Consolas");
            g_hBrushBg = CreateSolidBrush(COL_BG);
            g_hBrushPanel = CreateSolidBrush(COL_PANEL);

            int px = CANVAS_W + 24;
            int py = 10;
            int bw = 130;
            int bh = 26;

            CreateWindowA("BUTTON", "Add Router", WS_CHILD | WS_VISIBLE, px, py, bw, bh, hwnd, (HMENU)ID_BTN_ADD_ROUTER, NULL, NULL);
            CreateWindowA("BUTTON", "Add Switch", WS_CHILD | WS_VISIBLE, px + bw + 8, py, bw, bh, hwnd, (HMENU)ID_BTN_ADD_SWITCH, NULL, NULL);
            py += 32;

            CreateWindowA("BUTTON", "Add Firewall", WS_CHILD | WS_VISIBLE, px, py, bw, bh, hwnd, (HMENU)ID_BTN_ADD_FIREWALL, NULL, NULL);
            CreateWindowA("BUTTON", "Add Server", WS_CHILD | WS_VISIBLE, px + bw + 8, py, bw, bh, hwnd, (HMENU)ID_BTN_ADD_SERVER, NULL, NULL);
            py += 32;

            CreateWindowA("BUTTON", "Add Workstation", WS_CHILD | WS_VISIBLE, px, py, bw * 2 + 8, bh, hwnd, (HMENU)ID_BTN_ADD_WORKSTATION, NULL, NULL);
            py += 38;

            CreateWindowA("BUTTON", "Connect Link [C]", WS_CHILD | WS_VISIBLE, px, py, bw, bh, hwnd, (HMENU)ID_BTN_CONNECT, NULL, NULL);
            CreateWindowA("BUTTON", "Ping Test [P]", WS_CHILD | WS_VISIBLE, px + bw + 8, py, bw, bh, hwnd, (HMENU)ID_BTN_PING, NULL, NULL);
            py += 32;

            CreateWindowA("BUTTON", "Sever Link (Chaos)", WS_CHILD | WS_VISIBLE, px, py, bw, bh, hwnd, (HMENU)ID_BTN_CUT, NULL, NULL);
            CreateWindowA("BUTTON", "Clear Grid", WS_CHILD | WS_VISIBLE, px + bw + 8, py, bw, bh, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
            py += 38;

            CreateWindowA("BUTTON", "Preset: Corp DMZ", WS_CHILD | WS_VISIBLE, px, py, bw, bh, hwnd, (HMENU)ID_BTN_PRESET_CORP, NULL, NULL);
            CreateWindowA("BUTTON", "Preset: ISP Mesh", WS_CHILD | WS_VISIBLE, px + bw + 8, py, bw, bh, hwnd, (HMENU)ID_BTN_PRESET_ISP, NULL, NULL);
            py += 38;

            CreateWindowA("STATIC", "Base IP & Mask:", WS_CHILD | WS_VISIBLE, px, py, 120, 16, hwnd, NULL, NULL, NULL);
            py += 20;

            g_hEditIp = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "192.168.1.0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, px, py, 140, 24, hwnd, (HMENU)ID_EDIT_IP, NULL, NULL);
            g_hEditCidr = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "24", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, px + 148, py, 45, 24, hwnd, (HMENU)ID_EDIT_CIDR, NULL, NULL);
            CreateWindowA("BUTTON", "Calc", WS_CHILD | WS_VISIBLE, px + 200, py, 68, 24, hwnd, (HMENU)ID_BTN_CALC, NULL, NULL);
            py += 34;

            CreateWindowA("STATIC", "Diagnostic Event Log:", WS_CHILD | WS_VISIBLE, px, py, 160, 16, hwnd, NULL, NULL, NULL);
            py += 20;

            g_hEditLog = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL, px, py, bw * 2 + 8, 230, hwnd, (HMENU)ID_EDIT_LOG, NULL, NULL);
            SendMessageA(g_hEditLog, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            CreateWindowA("BUTTON", "Help & Shortcuts [F1]", WS_CHILD | WS_VISIBLE, px, H - 75, bw * 2 + 8, 28, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

            LoadPresetCorp();
            SetTimer(hwnd, 1, 30, NULL);
            return 0;
        }

        case WM_TIMER: {
            for (int i = 0; i < MAX_PACKETS; i++) {
                if (g_packets[i].active) {
                    g_packets[i].progress += g_packets[i].speed;
                    if (g_packets[i].progress >= 1.0f) {
                        g_packets[i].active = 0;
                        char buf[128];
                        k_strcpy(buf, "[REPLY] ICMP Echo Reply from ");
                        k_strcat(buf, g_nodes[g_packets[i].toNode].name);
                        k_strcat(buf, " (RTT=4ms)");
                        LogMessage(buf);
                    }
                }
            }
            RECT rcCanvas = { 10, 10, 10 + CANVAS_W, 10 + CANVAS_H };
            InvalidateRect(hwnd, &rcCanvas, FALSE);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam) - 10;
            int my = HIWORD(lParam) - 10;

            if (mx >= 0 && mx < CANVAS_W && my >= 0 && my < CANVAS_H) {
                int hit = HitTestNode(mx, my);
                if (g_activeMode == 0) { // Select
                    g_selectedNode = hit;
                    if (hit >= 0) {
                        g_draggedNode = hit;
                        g_dragOffsetX = mx - g_nodes[hit].x;
                        g_dragOffsetY = my - g_nodes[hit].y;
                    }
                } else if (g_activeMode == 1) { // Connect
                    if (hit >= 0) {
                        if (g_connectStart < 0) {
                            g_connectStart = hit;
                            LogMessage("[LINK] Click second node to complete cable connection.");
                        } else if (g_connectStart != hit) {
                            AddLink(g_connectStart, hit, 0);
                            LogMessage("[LINK] Wired 100Base-TX link.");
                            g_connectStart = -1;
                            g_activeMode = 0;
                        }
                    }
                } else if (g_activeMode == 2) { // Ping
                    if (hit >= 0) {
                        if (g_pingSource < 0) {
                            g_pingSource = hit;
                            LogMessage("[PING] Select destination node.");
                        } else {
                            DispatchPacket(g_pingSource, hit);
                            g_pingSource = -1;
                            g_activeMode = 0;
                        }
                    }
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }

        case WM_MOUSEMOVE: {
            if (g_draggedNode >= 0) {
                int mx = LOWORD(lParam) - 10;
                int my = HIWORD(lParam) - 10;
                g_nodes[g_draggedNode].x = mx - g_dragOffsetX;
                g_nodes[g_draggedNode].y = my - g_dragOffsetY;
                RECT rcCanvas = { 10, 10, 10 + CANVAS_W, 10 + CANVAS_H };
                InvalidateRect(hwnd, &rcCanvas, FALSE);
            }
            return 0;
        }

        case WM_LBUTTONUP: {
            g_draggedNode = -1;
            return 0;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            switch (id) {
                case ID_BTN_ADD_ROUTER:
                    AddNode(NODE_ROUTER, 100, 100, NULL, NULL);
                    LogMessage("[NODE] Placed CYBER-7200 Router.");
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_ADD_SWITCH:
                    AddNode(NODE_SWITCH, 100, 100, NULL, NULL);
                    LogMessage("[NODE] Placed 3-CON FastSwitch.");
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_ADD_FIREWALL:
                    AddNode(NODE_FIREWALL, 100, 100, NULL, NULL);
                    LogMessage("[NODE] Placed Bastion Firewall.");
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_ADD_SERVER:
                    AddNode(NODE_SERVER, 100, 100, NULL, NULL);
                    LogMessage("[NODE] Placed Sol Unix Server.");
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_ADD_WORKSTATION:
                    AddNode(NODE_WORKSTATION, 100, 100, NULL, NULL);
                    LogMessage("[NODE] Placed Win99 Workstation.");
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_CONNECT:
                    g_activeMode = 1;
                    g_connectStart = -1;
                    LogMessage("[MODE] Connect Link Mode: click node 1, then node 2.");
                    break;
                case ID_BTN_PING:
                    g_activeMode = 2;
                    g_pingSource = -1;
                    LogMessage("[MODE] Ping Mode: click source host, then target host.");
                    break;
                case ID_BTN_CUT:
                    if (g_linkCount > 0) {
                        g_links[g_linkCount - 1].status = (g_links[g_linkCount - 1].status == 1) ? 0 : 1;
                        LogMessage("[CHAOS] Toggled link up/down state.");
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    break;
                case ID_BTN_CLEAR:
                    ClearTopology();
                    LogMessage("[TOPOLOGY] Workspace cleared.");
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_PRESET_CORP:
                    LoadPresetCorp();
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_PRESET_ISP:
                    LoadPresetIsp();
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_CALC:
                    CalculateSubnet();
                    break;
                case ID_BTN_HELP:
                    MessageBoxA(hwnd,
                        "KNetMap - Subnet Topology Visualizer (Native 1999 Edition)\n\n"
                        "SHORTCUTS:\n"
                        "  [F1] Help Dialog\n"
                        "  [C]  Link Cable Mode\n"
                        "  [P]  Ping Test Mode\n"
                        "  [Del] Remove Selected Node\n\n"
                        "Click and drag nodes to adjust subnet layout.\n"
                        "Click 'Connect Link' to wire Ethernet or Fiber between devices.\n"
                        "Click 'Ping Test' to simulate ICMP packet flow across active links.",
                        "KNetMap Help", MB_OK | MB_ICONINFORMATION);
                    break;
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            DrawTopologyCanvas(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY: {
            KillTimer(hwnd, 1);
            if (g_hFont) DeleteObject(g_hFont);
            if (g_hFontMono) DeleteObject(g_hFontMono);
            if (g_hBrushBg) DeleteObject(g_hBrushBg);
            if (g_hBrushPanel) DeleteObject(g_hBrushPanel);
            PostQuitMessage(0);
            return 0;
        }

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

void MainEntry(void) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KNetMapClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        "KNetMapClass",
        "KNetMap - Subnet Topology Visualizer",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        W, H,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    ExitProcess((UINT)msg.wParam);
}
