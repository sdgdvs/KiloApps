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
#define ID_BTN_SAVE            117
#define ID_BTN_LOAD            118

// Persistence
#define KNETMAP_SAVE_MAGIC   0x4B4E4D31 // "KNM1"
#define KNETMAP_SAVE_VERSION 1

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
    int hops[MAX_NODES];
    int hopCount;
    int currentHopIndex;
    int fromNode;
    int toNode;
    float progress;
    float speed;
    int protocol; // 0=ICMP Echo Request, 1=ICMP Echo Reply
    int ttl;
    int rtt;
} NetPacket;

typedef struct {
    DWORD magic;
    DWORD version;
    int nodeCount;
    NetNode nodes[MAX_NODES];
    int linkCount;
    NetLink links[MAX_LINKS];
    char baseIp[32];
    char baseCidr[16];
} KNetMapSaveData;

static NetNode g_nodes[MAX_NODES];
static int g_nodeCount = 0;
static NetLink g_links[MAX_LINKS];
static int g_linkCount = 0;
static NetPacket g_packets[MAX_PACKETS];
static int g_simPaused = 0;

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
static HBRUSH g_hBrushEdit = NULL;

static void LogMessage(const char* msg) {
    if (!g_hEditLog) return;
    int len = GetWindowTextLengthA(g_hEditLog);
    SendMessageA(g_hEditLog, EM_SETSEL, len, len);
    SendMessageA(g_hEditLog, EM_REPLACESEL, FALSE, (LPARAM)msg);
    SendMessageA(g_hEditLog, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
    SendMessageA(g_hEditLog, EM_SCROLLCARET, 0, 0);
}

static void AddNode(NodeType type, int x, int y, const char* name, const char* ip) {
    if (g_nodeCount >= MAX_NODES) {
        LogMessage("[WARN] Maximum node capacity (32) reached.");
        return;
    }
    NetNode* n = &g_nodes[g_nodeCount++];
    n->id = g_nodeCount;
    n->type = type;
    n->x = x;
    n->y = y;
    n->status = 1;
    if (name) {
        k_strcpy(n->name, name);
    } else {
        char buf[32];
        char num[8];
        k_itoa(g_nodeCount, num);
        k_strcpy(buf, type == NODE_ROUTER ? "RTR-" :
                      type == NODE_SWITCH ? "SW-" :
                      type == NODE_FIREWALL ? "FW-" :
                      type == NODE_SERVER ? "SRV-" : "WS-");
        k_strcat(buf, num);
        k_strcpy(n->name, buf);
    }
    if (ip) {
        k_strcpy(n->ip, ip);
    } else {
        char buf[32] = "192.168.1.";
        char num[8];
        k_itoa(10 + g_nodeCount, num);
        k_strcat(buf, num);
        k_strcpy(n->ip, buf);
    }
}

static void AddLink(int fromIdx, int toIdx, int type) {
    if (g_linkCount >= MAX_LINKS) {
        LogMessage("[WARN] Maximum link capacity (64) reached.");
        return;
    }
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

static void DeleteNode(int idx) {
    if (idx < 0 || idx >= g_nodeCount) return;
    for (int i = 0; i < g_linkCount; ) {
        if (g_links[i].fromNode == idx || g_links[i].toNode == idx) {
            for (int j = i; j < g_linkCount - 1; j++) {
                g_links[j] = g_links[j + 1];
            }
            g_linkCount--;
        } else {
            i++;
        }
    }
    for (int i = 0; i < g_linkCount; i++) {
        if (g_links[i].fromNode > idx) g_links[i].fromNode--;
        if (g_links[i].toNode > idx) g_links[i].toNode--;
    }
    for (int i = idx; i < g_nodeCount - 1; i++) {
        g_nodes[i] = g_nodes[i + 1];
        g_nodes[i].id = i + 1;
    }
    g_nodeCount--;
    g_selectedNode = -1;
    g_draggedNode = -1;
    g_connectStart = -1;
    g_pingSource = -1;
    LogMessage("[TOPOLOGY] Node deleted from topology.");
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

// Shortest path via BFS over active links
static int FindPath(int fromNode, int toNode, int* outPath) {
    if (fromNode == toNode || fromNode < 0 || toNode < 0 || fromNode >= g_nodeCount || toNode >= g_nodeCount) return 0;
    int queue[MAX_NODES];
    int head = 0, tail = 0;
    int parent[MAX_NODES];
    int visited[MAX_NODES];
    for (int i = 0; i < MAX_NODES; i++) {
        parent[i] = -1;
        visited[i] = 0;
    }

    queue[tail++] = fromNode;
    visited[fromNode] = 1;

    int found = 0;
    while (head < tail) {
        int curr = queue[head++];
        if (curr == toNode) {
            found = 1;
            break;
        }
        for (int i = 0; i < g_linkCount; i++) {
            if (g_links[i].status != 1) continue; // link severed or down
            int neighbor = -1;
            if (g_links[i].fromNode == curr) neighbor = g_links[i].toNode;
            else if (g_links[i].toNode == curr) neighbor = g_links[i].fromNode;

            if (neighbor >= 0 && neighbor < g_nodeCount && !visited[neighbor]) {
                visited[neighbor] = 1;
                parent[neighbor] = curr;
                queue[tail++] = neighbor;
            }
        }
    }

    if (!found) return 0;

    int tempPath[MAX_NODES];
    int count = 0;
    int curr = toNode;
    while (curr != -1 && count < MAX_NODES) {
        tempPath[count++] = curr;
        if (curr == fromNode) break;
        curr = parent[curr];
    }

    for (int i = 0; i < count; i++) {
        outPath[i] = tempPath[count - 1 - i];
    }
    return count;
}

static void DispatchPacket(int fromIdx, int toIdx) {
    if (fromIdx < 0 || toIdx < 0 || fromIdx >= g_nodeCount || toIdx >= g_nodeCount || fromIdx == toIdx) return;
    int path[MAX_NODES];
    int hopCount = FindPath(fromIdx, toIdx, path);
    if (hopCount < 2) {
        char buf[160];
        k_strcpy(buf, "[ROUTING ERROR] Network Unreachable: no active path from ");
        k_strcat(buf, g_nodes[fromIdx].name);
        k_strcat(buf, " to ");
        k_strcat(buf, g_nodes[toIdx].name);
        k_strcat(buf, "!");
        LogMessage(buf);
        MessageBeep(MB_ICONHAND);
        return;
    }

    for (int i = 0; i < MAX_PACKETS; i++) {
        if (!g_packets[i].active) {
            g_packets[i].active = 1;
            g_packets[i].hopCount = hopCount;
            for (int h = 0; h < hopCount; h++) {
                g_packets[i].hops[h] = path[h];
            }
            g_packets[i].currentHopIndex = 0;
            g_packets[i].fromNode = path[0];
            g_packets[i].toNode = path[1];
            g_packets[i].progress = 0.0f;
            g_packets[i].speed = 0.045f;
            g_packets[i].protocol = 0; // Echo Request
            g_packets[i].ttl = 64;
            g_packets[i].rtt = (hopCount - 1) * 4 + 2;

            char buf[160];
            char num[16];
            k_strcpy(buf, "[PING] Dispatched ICMP Echo Request: ");
            k_strcat(buf, g_nodes[fromIdx].name);
            k_strcat(buf, " -> ");
            k_strcat(buf, g_nodes[toIdx].name);
            k_strcat(buf, " (");
            k_itoa(hopCount - 1, num);
            k_strcat(buf, num);
            k_strcat(buf, " hops, TTL=64)");
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

static int SaveTopologyState(HWND hwnd) {
    KNetMapSaveData save;
    memset(&save, 0, sizeof(save));
    save.magic = KNETMAP_SAVE_MAGIC;
    save.version = KNETMAP_SAVE_VERSION;
    save.nodeCount = g_nodeCount;
    for (int i = 0; i < g_nodeCount; i++) {
        save.nodes[i] = g_nodes[i];
    }
    save.linkCount = g_linkCount;
    for (int i = 0; i < g_linkCount; i++) {
        save.links[i] = g_links[i];
    }
    if (g_hEditIp) GetWindowTextA(g_hEditIp, save.baseIp, sizeof(save.baseIp));
    if (g_hEditCidr) GetWindowTextA(g_hEditCidr, save.baseCidr, sizeof(save.baseCidr));

    HANDLE hFile = CreateFileA("knetmap_save.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LogMessage("[SYSTEM] Error: Failed to write knetmap_save.dat");
        MessageBeep(MB_ICONHAND);
        return 0;
    }
    DWORD written = 0;
    BOOL res = WriteFile(hFile, &save, sizeof(save), &written, NULL);
    CloseHandle(hFile);

    if (res && written == sizeof(save)) {
        LogMessage("[STATE SAVED] Topology configuration written to knetmap_save.dat [F5].");
        MessageBeep(MB_OK);
        return 1;
    } else {
        LogMessage("[SYSTEM] Error: Incomplete write to knetmap_save.dat");
        MessageBeep(MB_ICONHAND);
        return 0;
    }
}

static int LoadTopologyState(HWND hwnd) {
    HANDLE hFile = CreateFileA("knetmap_save.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LogMessage("[SYSTEM] No quicksave found (knetmap_save.dat). Press F5 to save.");
        MessageBeep(MB_ICONHAND);
        return 0;
    }
    KNetMapSaveData save;
    DWORD bytesRead = 0;
    BOOL res = ReadFile(hFile, &save, sizeof(save), &bytesRead, NULL);
    CloseHandle(hFile);

    if (!res || bytesRead != sizeof(save) || save.magic != KNETMAP_SAVE_MAGIC || save.version != KNETMAP_SAVE_VERSION) {
        LogMessage("[SYSTEM] Corrupt or incompatible quicksave file.");
        MessageBeep(MB_ICONHAND);
        return 0;
    }

    g_nodeCount = save.nodeCount;
    if (g_nodeCount > MAX_NODES) g_nodeCount = MAX_NODES;
    for (int i = 0; i < g_nodeCount; i++) {
        g_nodes[i] = save.nodes[i];
    }
    g_linkCount = save.linkCount;
    if (g_linkCount > MAX_LINKS) g_linkCount = MAX_LINKS;
    for (int i = 0; i < g_linkCount; i++) {
        g_links[i] = save.links[i];
    }
    if (g_hEditIp && save.baseIp[0]) SetWindowTextA(g_hEditIp, save.baseIp);
    if (g_hEditCidr && save.baseCidr[0]) SetWindowTextA(g_hEditCidr, save.baseCidr);

    g_selectedNode = -1;
    g_draggedNode = -1;
    g_connectStart = -1;
    g_pingSource = -1;
    g_activeMode = 0;
    for (int i = 0; i < MAX_PACKETS; i++) g_packets[i].active = 0;

    // Mark tutorial as seen so restored save never triggers tutorial
    HANDLE hTut = CreateFileA("knetmap_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTut != INVALID_HANDLE_VALUE) {
        DWORD tw = 0;
        WriteFile(hTut, "SEEN", 4, &tw, NULL);
        CloseHandle(hTut);
    }

    LogMessage("[STATE RESTORED] Loaded topology quicksave from knetmap_save.dat [F9].");
    MessageBeep(MB_OK);
    if (hwnd) InvalidateRect(hwnd, NULL, FALSE);
    return 1;
}

static void CheckFirstRunTutorial(HWND hwnd) {
    DWORD saveAttr = GetFileAttributesA("knetmap_save.dat");
    if (saveAttr != INVALID_FILE_ATTRIBUTES) {
        LogMessage("[SYSTEM] Quicksave available. Press [F9] to restore saved topology.");
        return;
    }
    DWORD tutAttr = GetFileAttributesA("knetmap_tutorial.dat");
    if (tutAttr != INVALID_FILE_ATTRIBUTES) {
        return;
    }

    LogMessage("==================================================");
    LogMessage("[TUTORIAL] Welcome to KNetMap Subnet Topology Visualizer!");
    LogMessage("[TUTORIAL] 1. Place devices with buttons or hotkeys [1-5].");
    LogMessage("[TUTORIAL] 2. Wire links between devices with Connect Link [C].");
    LogMessage("[TUTORIAL] 3. Dispatch multi-hop ICMP packets with Ping Test [P].");
    LogMessage("[TUTORIAL] 4. Quicksave topology with [F5], restore with [F9].");
    LogMessage("[TUTORIAL] Press [Esc] to cancel current action, [Del] to remove node.");
    LogMessage("==================================================");

    HANDLE hTut = CreateFileA("knetmap_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTut != INVALID_HANDLE_VALUE) {
        DWORD tw = 0;
        WriteFile(hTut, "SEEN", 4, &tw, NULL);
        CloseHandle(hTut);
    }
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

    // Font setup
    HFONT hOldFont = (HFONT)SelectObject(hdcMem, g_hFontMono);
    SetBkMode(hdcMem, TRANSPARENT);

    // Draw Links
    int linksUp = 0;
    for (int i = 0; i < g_linkCount; i++) {
        NetLink* l = &g_links[i];
        if (l->fromNode >= g_nodeCount || l->toNode >= g_nodeCount) continue;
        NetNode* n1 = &g_nodes[l->fromNode];
        NetNode* n2 = &g_nodes[l->toNode];

        COLORREF linkCol = (l->type == 2) ? COL_PURPLE : (l->type == 3) ? COL_EMERALD : (l->type == 1) ? COL_AMBER : COL_CYAN;
        int style = (l->status == 0) ? PS_DOT : PS_SOLID;
        if (l->status == 0) {
            linkCol = COL_ROSE;
        } else {
            linksUp++;
        }

        HPEN hPenLink = CreatePen(style, 2, linkCol);
        hOldPen = SelectObject(hdcMem, hPenLink);
        MoveToEx(hdcMem, n1->x, n1->y, NULL);
        LineTo(hdcMem, n2->x, n2->y);
        SelectObject(hdcMem, hOldPen);
        DeleteObject(hPenLink);

        // Link speed label at midpoint
        int mx = (n1->x + n2->x) / 2;
        int my = (n1->y + n2->y) / 2;
        if (l->status == 0) {
            SetTextColor(hdcMem, COL_ROSE);
            TextOutA(hdcMem, mx - 14, my - 7, "[CUT]", 5);
        } else {
            SetTextColor(hdcMem, linkCol);
            const char* spd = (l->type == 2) ? "FDDI" : (l->type == 3) ? "T1" : (l->type == 1) ? "10M" : "100M";
            TextOutA(hdcMem, mx - 12, my - 7, spd, k_strlen(spd));
        }
    }

    // Draw Packets (Telemetry pulse)
    int activePkts = 0;
    for (int i = 0; i < MAX_PACKETS; i++) {
        if (g_packets[i].active) {
            activePkts++;
            if (g_packets[i].fromNode >= g_nodeCount || g_packets[i].toNode >= g_nodeCount) continue;
            NetNode* n1 = &g_nodes[g_packets[i].fromNode];
            NetNode* n2 = &g_nodes[g_packets[i].toNode];
            float p = g_packets[i].progress;
            int px = (int)(n1->x + (n2->x - n1->x) * p);
            int py = (int)(n1->y + (n2->y - n1->y) * p);

            COLORREF pktCol = (g_packets[i].protocol == 0) ? COL_CYAN : COL_EMERALD;
            HBRUSH hBrPkt = CreateSolidBrush(pktCol);
            HGDIOBJ hOldBr = SelectObject(hdcMem, hBrPkt);
            Ellipse(hdcMem, px - 6, py - 6, px + 6, py + 6);
            SelectObject(hdcMem, hOldBr);
            DeleteObject(hBrPkt);

            SetTextColor(hdcMem, pktCol);
            const char* pLbl = (g_packets[i].protocol == 0) ? "ICMP" : "REPLY";
            TextOutA(hdcMem, px + 8, py - 6, pLbl, k_strlen(pLbl));
        }
    }

    // Draw Nodes
    for (int i = 0; i < g_nodeCount; i++) {
        NetNode* n = &g_nodes[i];
        COLORREF nodeCol = (n->type == NODE_ROUTER) ? RGB(12, 74, 110) :
                           (n->type == NODE_SWITCH) ? RGB(20, 83, 45) :
                           (n->type == NODE_FIREWALL) ? RGB(136, 19, 55) :
                           (n->type == NODE_SERVER) ? RGB(88, 28, 135) : RGB(30, 41, 59);

        COLORREF borderCol = (i == g_selectedNode) ? COL_CYAN :
                             (i == g_connectStart) ? COL_AMBER :
                             (i == g_pingSource) ? COL_EMERALD :
                             (n->type == NODE_ROUTER) ? COL_CYAN :
                             (n->type == NODE_SWITCH) ? COL_EMERALD :
                             (n->type == NODE_FIREWALL) ? COL_ROSE : COL_TEXT_MUTED;

        HBRUSH hBrNode = CreateSolidBrush(nodeCol);
        HPEN hPenNode = CreatePen(PS_SOLID, (i == g_selectedNode || i == g_connectStart || i == g_pingSource) ? 3 : 2, borderCol);
        HGDIOBJ hOldBr = SelectObject(hdcMem, hBrNode);
        hOldPen = SelectObject(hdcMem, hPenNode);

        Ellipse(hdcMem, n->x - 18, n->y - 18, n->x + 18, n->y + 18);

        SelectObject(hdcMem, hOldPen);
        SelectObject(hdcMem, hOldBr);
        DeleteObject(hPenNode);
        DeleteObject(hBrNode);

        // Hardware type glyph inside node
        const char* typeGlyph = (n->type == NODE_ROUTER) ? "RTR" :
                                (n->type == NODE_SWITCH) ? "SW" :
                                (n->type == NODE_FIREWALL) ? "FW" :
                                (n->type == NODE_SERVER) ? "SRV" : "WS";
        SetTextColor(hdcMem, RGB(255, 255, 255));
        TextOutA(hdcMem, n->x - 9, n->y - 6, typeGlyph, k_strlen(typeGlyph));

        // State indicator badge
        if (i == g_connectStart) {
            SetTextColor(hdcMem, COL_AMBER);
            TextOutA(hdcMem, n->x - 22, n->y - 30, "[C-SRC]", 7);
        } else if (i == g_pingSource) {
            SetTextColor(hdcMem, COL_EMERALD);
            TextOutA(hdcMem, n->x - 22, n->y - 30, "[P-SRC]", 7);
        }

        // Labels
        SetTextColor(hdcMem, COL_TEXT);
        TextOutA(hdcMem, n->x - 30, n->y + 22, n->name, k_strlen(n->name));
        SetTextColor(hdcMem, COL_TEXT_MUTED);
        TextOutA(hdcMem, n->x - 30, n->y + 36, n->ip, k_strlen(n->ip));
    }

    // Telemetry Status Bar at bottom of canvas
    RECT rcBar = { 0, CANVAS_H - 24, CANVAS_W, CANVAS_H };
    HBRUSH hBrBar = CreateSolidBrush(RGB(11, 17, 32));
    FillRect(hdcMem, &rcBar, hBrBar);
    DeleteObject(hBrBar);

    HPEN hPenDiv = CreatePen(PS_SOLID, 1, RGB(30, 41, 59));
    hOldPen = SelectObject(hdcMem, hPenDiv);
    MoveToEx(hdcMem, 0, CANVAS_H - 24, NULL);
    LineTo(hdcMem, CANVAS_W, CANVAS_H - 24);
    SelectObject(hdcMem, hOldPen);
    DeleteObject(hPenDiv);

    char statBuf[192];
    char n1[8], n2[8], n3[8], n4[8];
    k_itoa(g_nodeCount, n1);
    k_itoa(g_linkCount, n2);
    k_itoa(linksUp, n3);
    k_itoa(activePkts, n4);

    k_strcpy(statBuf, "NODES: ");
    k_strcat(statBuf, n1);
    k_strcat(statBuf, "  |  LINKS: ");
    k_strcat(statBuf, n3);
    k_strcat(statBuf, "/");
    k_strcat(statBuf, n2);
    k_strcat(statBuf, " UP  |  PKTS: ");
    k_strcat(statBuf, n4);
    k_strcat(statBuf, "  |  MODE: ");
    k_strcat(statBuf, (g_activeMode == 1) ? "[CONNECT LINK]" :
                      (g_activeMode == 2) ? "[PING ICMP]" : "[SELECT]");
    if (g_simPaused) k_strcat(statBuf, " [PAUSED]");

    SetTextColor(hdcMem, COL_CYAN);
    TextOutA(hdcMem, 8, CANVAS_H - 18, statBuf, k_strlen(statBuf));

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
            g_hBrushEdit = CreateSolidBrush(RGB(6, 10, 20));

            int px = CANVAS_W + 20;
            int py = 10;
            int bw = 130;
            int bh = 26;
            int bw2 = bw * 2 + 8;

            CreateWindowA("BUTTON", "Add Router [1]", WS_CHILD | WS_VISIBLE, px, py, bw, bh, hwnd, (HMENU)ID_BTN_ADD_ROUTER, NULL, NULL);
            CreateWindowA("BUTTON", "Add Switch [2]", WS_CHILD | WS_VISIBLE, px + bw + 8, py, bw, bh, hwnd, (HMENU)ID_BTN_ADD_SWITCH, NULL, NULL);
            py += 30;

            CreateWindowA("BUTTON", "Add Firewall [3]", WS_CHILD | WS_VISIBLE, px, py, bw, bh, hwnd, (HMENU)ID_BTN_ADD_FIREWALL, NULL, NULL);
            CreateWindowA("BUTTON", "Add Server [4]", WS_CHILD | WS_VISIBLE, px + bw + 8, py, bw, bh, hwnd, (HMENU)ID_BTN_ADD_SERVER, NULL, NULL);
            py += 30;

            CreateWindowA("BUTTON", "Add Workstation [5]", WS_CHILD | WS_VISIBLE, px, py, bw2, bh, hwnd, (HMENU)ID_BTN_ADD_WORKSTATION, NULL, NULL);
            py += 34;

            CreateWindowA("BUTTON", "Connect Link [C]", WS_CHILD | WS_VISIBLE, px, py, bw, bh, hwnd, (HMENU)ID_BTN_CONNECT, NULL, NULL);
            CreateWindowA("BUTTON", "Ping Test [P]", WS_CHILD | WS_VISIBLE, px + bw + 8, py, bw, bh, hwnd, (HMENU)ID_BTN_PING, NULL, NULL);
            py += 30;

            CreateWindowA("BUTTON", "Sever Link (Chaos)", WS_CHILD | WS_VISIBLE, px, py, bw, bh, hwnd, (HMENU)ID_BTN_CUT, NULL, NULL);
            CreateWindowA("BUTTON", "Clear Grid", WS_CHILD | WS_VISIBLE, px + bw + 8, py, bw, bh, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
            py += 30;

            CreateWindowA("BUTTON", "Quicksave [F5]", WS_CHILD | WS_VISIBLE, px, py, bw, bh, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            CreateWindowA("BUTTON", "Quickload [F9]", WS_CHILD | WS_VISIBLE, px + bw + 8, py, bw, bh, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            py += 30;

            CreateWindowA("BUTTON", "Preset: Corp DMZ", WS_CHILD | WS_VISIBLE, px, py, bw, bh, hwnd, (HMENU)ID_BTN_PRESET_CORP, NULL, NULL);
            CreateWindowA("BUTTON", "Preset: ISP Mesh", WS_CHILD | WS_VISIBLE, px + bw + 8, py, bw, bh, hwnd, (HMENU)ID_BTN_PRESET_ISP, NULL, NULL);
            py += 34;

            CreateWindowA("STATIC", "Base IP & Subnet CIDR:", WS_CHILD | WS_VISIBLE, px, py, 160, 16, hwnd, NULL, NULL, NULL);
            py += 18;

            g_hEditIp = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "192.168.1.0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, px, py, 130, 24, hwnd, (HMENU)ID_EDIT_IP, NULL, NULL);
            g_hEditCidr = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "24", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, px + 138, py, 50, 24, hwnd, (HMENU)ID_EDIT_CIDR, NULL, NULL);
            CreateWindowA("BUTTON", "Calc", WS_CHILD | WS_VISIBLE, px + 196, py, 72, 24, hwnd, (HMENU)ID_BTN_CALC, NULL, NULL);
            py += 30;

            CreateWindowA("STATIC", "Diagnostic Telemetry Log:", WS_CHILD | WS_VISIBLE, px, py, 180, 16, hwnd, NULL, NULL, NULL);
            py += 18;

            g_hEditLog = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL, px, py, bw2, 290, hwnd, (HMENU)ID_EDIT_LOG, NULL, NULL);
            SendMessageA(g_hEditLog, WM_SETFONT, (WPARAM)g_hFontMono, TRUE);

            CreateWindowA("BUTTON", "Help & Shortcuts [F1]", WS_CHILD | WS_VISIBLE, px, H - 75, bw2, 28, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);

            LoadPresetCorp();
            CheckFirstRunTutorial(hwnd);
            SetTimer(hwnd, 1, 30, NULL);
            return 0;
        }

        case WM_ERASEBKGND: {
            HDC hdcBg = (HDC)wParam;
            RECT rcCl;
            GetClientRect(hwnd, &rcCl);
            FillRect(hdcBg, &rcCl, g_hBrushPanel);
            return 1;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, COL_TEXT_MUTED);
            SetBkColor(hdcStatic, COL_PANEL);
            return (LRESULT)g_hBrushPanel;
        }

        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            SetTextColor(hdcEdit, COL_CYAN);
            SetBkColor(hdcEdit, RGB(6, 10, 20));
            return (LRESULT)g_hBrushEdit;
        }

        case WM_TIMER: {
            if (g_simPaused) return 0;
            for (int i = 0; i < MAX_PACKETS; i++) {
                if (g_packets[i].active) {
                    g_packets[i].progress += g_packets[i].speed;
                    if (g_packets[i].progress >= 1.0f) {
                        // Check if more hops remain
                        if (g_packets[i].currentHopIndex < g_packets[i].hopCount - 2) {
                            g_packets[i].currentHopIndex++;
                            int prev = g_packets[i].hops[g_packets[i].currentHopIndex];
                            int next = g_packets[i].hops[g_packets[i].currentHopIndex + 1];

                            // Verify link is still up
                            int linkUp = 0;
                            for (int l = 0; l < g_linkCount; l++) {
                                if ((g_links[l].fromNode == prev && g_links[l].toNode == next) ||
                                    (g_links[l].fromNode == next && g_links[l].toNode == prev)) {
                                    if (g_links[l].status == 1) linkUp = 1;
                                    break;
                                }
                            }

                            if (!linkUp) {
                                g_packets[i].active = 0;
                                char buf[160];
                                k_strcpy(buf, "[DROP] Packet dropped mid-transit between ");
                                k_strcat(buf, g_nodes[prev].name);
                                k_strcat(buf, " and ");
                                k_strcat(buf, g_nodes[next].name);
                                k_strcat(buf, " (Link Severed)!");
                                LogMessage(buf);
                                MessageBeep(MB_ICONHAND);
                            } else {
                                g_packets[i].fromNode = prev;
                                g_packets[i].toNode = next;
                                g_packets[i].progress = 0.0f;
                                g_packets[i].ttl--;
                                char buf[160];
                                char num[16];
                                k_strcpy(buf, "[HOP] ");
                                k_strcat(buf, g_nodes[prev].name);
                                k_strcat(buf, " forwarded packet (TTL=");
                                k_itoa(g_packets[i].ttl, num);
                                k_strcat(buf, num);
                                k_strcat(buf, ")");
                                LogMessage(buf);
                            }
                        } else {
                            // Reached destination
                            int dstIdx = g_packets[i].hops[g_packets[i].hopCount - 1];
                            if (g_packets[i].protocol == 0) {
                                // ICMP Echo Request reached target -> send Echo Reply back
                                char buf[160];
                                char num[16];
                                k_strcpy(buf, "[PING REPLY] 64 bytes from ");
                                k_strcat(buf, g_nodes[dstIdx].ip);
                                k_strcat(buf, " (");
                                k_strcat(buf, g_nodes[dstIdx].name);
                                k_strcat(buf, "): icmp_seq=1 ttl=");
                                k_itoa(g_packets[i].ttl, num);
                                k_strcat(buf, num);
                                k_strcat(buf, " time=");
                                k_itoa(g_packets[i].rtt, num);
                                k_strcat(buf, num);
                                k_strcat(buf, "ms");
                                LogMessage(buf);
                                MessageBeep(MB_OK);

                                // Turn into Echo Reply
                                g_packets[i].protocol = 1;
                                int temp[MAX_NODES];
                                for (int h = 0; h < g_packets[i].hopCount; h++) {
                                    temp[h] = g_packets[i].hops[g_packets[i].hopCount - 1 - h];
                                }
                                for (int h = 0; h < g_packets[i].hopCount; h++) {
                                    g_packets[i].hops[h] = temp[h];
                                }
                                g_packets[i].currentHopIndex = 0;
                                g_packets[i].fromNode = g_packets[i].hops[0];
                                g_packets[i].toNode = g_packets[i].hops[1];
                                g_packets[i].progress = 0.0f;
                            } else {
                                // Echo Reply arrived at origin
                                g_packets[i].active = 0;
                                char buf[160];
                                char num[16];
                                k_strcpy(buf, "[TELEMETRY] Round-trip ping complete: 0% packet loss, RTT=");
                                k_itoa(g_packets[i].rtt, num);
                                k_strcat(buf, num);
                                k_strcat(buf, "ms.");
                                LogMessage(buf);
                            }
                        }
                    }
                }
            }
            RECT rcCanvas = { 10, 10, 10 + CANVAS_W, 10 + CANVAS_H };
            InvalidateRect(hwnd, &rcCanvas, FALSE);
            return 0;
        }

        case WM_KEYDOWN: {
            HWND hFocus = GetFocus();
            if (hFocus == g_hEditIp || hFocus == g_hEditCidr || hFocus == g_hEditLog) {
                if (wParam != VK_F1 && wParam != VK_F5 && wParam != VK_F9 && wParam != VK_ESCAPE) {
                    break;
                }
            }
            switch (wParam) {
                case VK_F1:
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_HELP, 0), 0);
                    return 0;
                case VK_F5:
                    SaveTopologyState(hwnd);
                    return 0;
                case VK_F9:
                    LoadTopologyState(hwnd);
                    return 0;
                case VK_ESCAPE:
                    g_activeMode = 0;
                    g_connectStart = -1;
                    g_pingSource = -1;
                    g_selectedNode = -1;
                    LogMessage("[MODE] Select Mode active (Cancelled).");
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                case 'C':
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_CONNECT, 0), 0);
                    return 0;
                case 'P':
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_PING, 0), 0);
                    return 0;
                case VK_DELETE:
                case VK_BACK:
                    if (g_selectedNode >= 0) {
                        DeleteNode(g_selectedNode);
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    return 0;
                case '1':
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_ADD_ROUTER, 0), 0);
                    return 0;
                case '2':
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_ADD_SWITCH, 0), 0);
                    return 0;
                case '3':
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_ADD_FIREWALL, 0), 0);
                    return 0;
                case '4':
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_ADD_SERVER, 0), 0);
                    return 0;
                case '5':
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_ADD_WORKSTATION, 0), 0);
                    return 0;
                case VK_SPACE:
                    g_simPaused = !g_simPaused;
                    LogMessage(g_simPaused ? "[SIM] Simulation paused." : "[SIM] Simulation resumed.");
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
            }
            break;
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
                        char buf[128];
                        k_strcpy(buf, "[SELECT] Node: ");
                        k_strcat(buf, g_nodes[hit].name);
                        k_strcat(buf, " (IP: ");
                        k_strcat(buf, g_nodes[hit].ip);
                        k_strcat(buf, ")");
                        LogMessage(buf);
                    }
                } else if (g_activeMode == 1) { // Connect
                    if (hit >= 0) {
                        if (g_connectStart < 0) {
                            g_connectStart = hit;
                            LogMessage("[LINK] Click second node to complete cable connection.");
                        } else if (g_connectStart != hit) {
                            AddLink(g_connectStart, hit, 0);
                            LogMessage("[LINK] Wired 100Base-TX link between devices.");
                            g_connectStart = -1;
                            g_activeMode = 0;
                        }
                    }
                } else if (g_activeMode == 2) { // Ping
                    if (hit >= 0) {
                        if (g_pingSource < 0) {
                            g_pingSource = hit;
                            LogMessage("[PING] Select destination host to ping.");
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
                int nx = mx - g_dragOffsetX;
                int ny = my - g_dragOffsetY;
                if (nx < 20) nx = 20;
                if (nx > CANVAS_W - 20) nx = CANVAS_W - 20;
                if (ny < 20) ny = 20;
                if (ny > CANVAS_H - 40) ny = CANVAS_H - 40;
                g_nodes[g_draggedNode].x = nx;
                g_nodes[g_draggedNode].y = ny;
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
                    LogMessage("[MODE] Connect Link Mode: click first node, then second node.");
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case ID_BTN_PING:
                    g_activeMode = 2;
                    g_pingSource = -1;
                    LogMessage("[MODE] Ping Mode: click source host, then destination host.");
                    InvalidateRect(hwnd, NULL, FALSE);
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
                case ID_BTN_SAVE:
                    SaveTopologyState(hwnd);
                    break;
                case ID_BTN_LOAD:
                    LoadTopologyState(hwnd);
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
                        "  [F5] Quicksave Topology (knetmap_save.dat)\n"
                        "  [F9] Quickload Topology\n"
                        "  [C]  Connect Link Mode\n"
                        "  [P]  Ping Test Mode (ICMP)\n"
                        "  [Space] Pause / Resume Simulation\n"
                        "  [1-5] Add Hardware (Router, Switch, FW, Server, WS)\n"
                        "  [Del/Bksp] Delete Selected Node\n"
                        "  [Esc] Cancel Current Mode / Deselect\n\n"
                        "MOUSE CONTROLS:\n"
                        "  Left-Click + Drag: Reposition Nodes\n"
                        "  Connect Mode: Click node 1 then node 2\n"
                        "  Ping Mode: Click source host then target host",
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
            if (g_hBrushEdit) DeleteObject(g_hBrushEdit);
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
