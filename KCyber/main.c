#include <windows.h>
#include <stdlib.h>

#define MAX_LINES 100
#define MAX_LINE_LENGTH 128

char history[MAX_LINES][MAX_LINE_LENGTH];
int history_count = 0;
char current_input[MAX_LINE_LENGTH];
int current_input_len = 0;
HFONT hFont;
int cursor_visible = 1;

int hacking_node = 0;
char hacking_target[5] = {0};
int hacking_attempts = 0;

int connected_node = 0;
int player_credits = 0;

typedef struct {
    char name[32];
    int size;
    int value;
} NodeFile;

NodeFile node_files[6] = {
    {"", 0, 0},
    {"", 0, 0},
    {"sys_logs.dat", 12, 100},
    {"customer_db.sql", 45, 250},
    {"r_and_d_schematics.zip", 105, 600},
    {"zero_day_exploit.exe", 15, 1500}
};

int player_mem = 100;
int tool_cloak = 3;
int tool_slow = 3;
int ice_damage = 0;
char ice_name[32] = "";
int ice_frozen_ticks = 0;

static unsigned long int my_next = 1;
int my_rand(void) {
    my_next = my_next * 1103515245 + 12345;
    return (unsigned int)(my_next / 65536) % 32768;
}
void my_srand(unsigned int seed) {
    my_next = seed;
}

void PrintLine(HWND hwnd, const char* text) {
    if (history_count < MAX_LINES) {
        lstrcpynA(history[history_count], text, MAX_LINE_LENGTH);
        history_count++;
    } else {
        for (int i = 0; i < MAX_LINES - 1; i++) {
            lstrcpyA(history[i], history[i + 1]);
        }
        lstrcpynA(history[MAX_LINES - 1], text, MAX_LINE_LENGTH);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void ProcessCommand(HWND hwnd, const char* cmd) {
    char buffer[MAX_LINE_LENGTH + 32];
    
    // Trim leading spaces
    while(*cmd == ' ') cmd++;
    if (*cmd == '\0') return;

    if (hacking_node) {
        wsprintfA(buffer, "[NODE 0%d] hack> %s", hacking_node, cmd);
        PrintLine(hwnd, buffer);
        
        char guess[MAX_LINE_LENGTH];
        lstrcpynA(guess, cmd, MAX_LINE_LENGTH);
        if (lstrcmpiA(guess, "abort") == 0) {
            hacking_node = 0;
            PrintLine(hwnd, "Hacking aborted.");
            return;
        }
        if (lstrcmpiA(guess, "cloak") == 0) {
            if (tool_cloak > 0) {
                tool_cloak--;
                ice_frozen_ticks += 2;
                wsprintfA(buffer, "[CLOAK] Activated. %d remaining. ICE blinded for 2 cycles.", tool_cloak);
                PrintLine(hwnd, buffer);
            } else {
                PrintLine(hwnd, "[CLOAK] Out of charges.");
            }
            return;
        }
        if (lstrcmpiA(guess, "slow") == 0) {
            if (tool_slow > 0) {
                tool_slow--;
                wsprintfA(buffer, "[SLOW] Activated. %d remaining. Trace speed halved.", tool_slow);
                PrintLine(hwnd, buffer);
                SetTimer(hwnd, 2, 6000, NULL);
            } else {
                PrintLine(hwnd, "[SLOW] Out of charges.");
            }
            return;
        }
        
        int valid = 1;
        if (lstrlenA(guess) != 4) valid = 0;
        for(int i=0; i<4; i++) {
            if(guess[i] < '0' || guess[i] > '9') valid = 0;
        }
        if (!valid) {
            PrintLine(hwnd, "Invalid input. Enter 4 digits (e.g., 1234) or 'abort' to cancel.");
            return;
        }
        
        int exact = 0, partial = 0;
        int targetUsed[4] = {0,0,0,0};
        int guessUsed[4] = {0,0,0,0};
        for(int i=0; i<4; i++){
            if(guess[i] == hacking_target[i]){
                exact++;
                targetUsed[i] = 1;
                guessUsed[i] = 1;
            }
        }
        for(int i=0; i<4; i++){
            if(!guessUsed[i]){
                for(int j=0; j<4; j++){
                    if(!targetUsed[j] && guess[i] == hacking_target[j]){
                        partial++;
                        targetUsed[j] = 1;
                        break;
                    }
                }
            }
        }
        
        if (exact == 4) {
            wsprintfA(buffer, "ACCESS GRANTED. Connected to node [0%d].", hacking_node);
            PrintLine(hwnd, buffer);
            PrintLine(hwnd, "Type 'ls' to list files, 'download <file>' to extract, 'disconnect' to exit.");
            connected_node = hacking_node;
            hacking_node = 0;
        } else {
            hacking_attempts--;
            wsprintfA(buffer, "Result: %d EXACT, %d PARTIAL", exact, partial);
            PrintLine(hwnd, buffer);
            if (hacking_attempts > 0) {
                wsprintfA(buffer, "Attempts remaining: %d", hacking_attempts);
                PrintLine(hwnd, buffer);
            } else {
                PrintLine(hwnd, "ACCESS DENIED. TRACE DETECTED. CONNECTION TERMINATED.");
                hacking_node = 0;
            }
        }
        return;
    }

    if (connected_node) {
        wsprintfA(buffer, "[NODE 0%d] root> %s", connected_node, cmd);
        PrintLine(hwnd, buffer);
        
        char command[MAX_LINE_LENGTH];
        char args[MAX_LINE_LENGTH];
        command[0] = '\0';
        args[0] = '\0';
        
        int i = 0;
        while(cmd[i] != ' ' && cmd[i] != '\0' && i < MAX_LINE_LENGTH - 1) {
            command[i] = cmd[i];
            i++;
        }
        command[i] = '\0';
        
        if (cmd[i] == ' ') {
            i++;
            while(cmd[i] == ' ') i++;
            int j = 0;
            while(cmd[i] != '\0' && j < MAX_LINE_LENGTH - 1) {
                args[j] = cmd[i];
                i++; j++;
            }
            args[j] = '\0';
        }

        if (lstrcmpiA(command, "disconnect") == 0 || lstrcmpiA(command, "abort") == 0) {
            connected_node = 0;
            PrintLine(hwnd, "Disconnected from node.");
            KillTimer(hwnd, 2);
            return;
        }
        
        if (lstrcmpiA(command, "ls") == 0) {
            if (lstrcmpiA(node_files[connected_node].name, "empty") == 0) {
                PrintLine(hwnd, "No files found. Directory is empty.");
            } else {
                PrintLine(hwnd, "Files:");
                char buf[128];
                wsprintfA(buf, "  %s (%dMB)", node_files[connected_node].name, node_files[connected_node].size);
                PrintLine(hwnd, buf);
            }
            return;
        }
        
        if (lstrcmpiA(command, "download") == 0) {
            if (args[0] == '\0') {
                PrintLine(hwnd, "Usage: download <filename>");
            } else {
                if (lstrcmpiA(node_files[connected_node].name, "empty") != 0 && lstrcmpiA(args, node_files[connected_node].name) == 0) {
                    char buf[128];
                    wsprintfA(buf, "Downloading %s...", args);
                    PrintLine(hwnd, buf);
                    player_credits += node_files[connected_node].value;
                    wsprintfA(buf, "Download complete. Data value: %d credits.", node_files[connected_node].value);
                    PrintLine(hwnd, buf);
                    lstrcpyA(node_files[connected_node].name, "empty");
                } else {
                    PrintLine(hwnd, "File not found.");
                }
            }
            return;
        }
        
        PrintLine(hwnd, "Command not found. Available: ls, download, disconnect");
        return;
    }

    lstrcpyA(buffer, "root@cyberdeck:~# ");
    lstrcatA(buffer, cmd);
    PrintLine(hwnd, buffer);

    char command[MAX_LINE_LENGTH];
    char args[MAX_LINE_LENGTH];
    command[0] = '\0';
    args[0] = '\0';
    
    int i = 0;
    while(cmd[i] != ' ' && cmd[i] != '\0' && i < MAX_LINE_LENGTH - 1) {
        command[i] = cmd[i];
        i++;
    }
    command[i] = '\0';
    
    if (cmd[i] == ' ') {
        i++;
        while(cmd[i] == ' ') i++; // skip extra spaces
        int j = 0;
        while(cmd[i] != '\0' && j < MAX_LINE_LENGTH - 1) {
            args[j] = cmd[i];
            i++; j++;
        }
        args[j] = '\0';
    }

    if (lstrcmpiA(command, "help") == 0) {
        PrintLine(hwnd, "Available commands:");
        PrintLine(hwnd, "  help    - Show this message");
        PrintLine(hwnd, "  clear   - Clear terminal output");
        PrintLine(hwnd, "  status  - Show deck status");
        PrintLine(hwnd, "  reboot  - Restart deck to restore MEM and software");
        PrintLine(hwnd, "  map     - Display network topology");
        PrintLine(hwnd, "  connect - Attempt connection to network node");
        PrintLine(hwnd, "During hack:");
        PrintLine(hwnd, "  abort   - Disconnect immediately");
        PrintLine(hwnd, "  cloak   - Blind ICE for 2 cycles");
        PrintLine(hwnd, "  slow    - Halve ICE attack speed");
    } else if (lstrcmpiA(command, "clear") == 0) {
        history_count = 0;
        InvalidateRect(hwnd, NULL, TRUE);
    } else if (lstrcmpiA(command, "status") == 0) {
        char memBuf[64];
        wsprintfA(memBuf, "  MEM: %d%%", player_mem);
        PrintLine(hwnd, "DECK STATUS:");
        PrintLine(hwnd, "  CPU: 100%");
        PrintLine(hwnd, memBuf);
        wsprintfA(memBuf, "  CLOAK: %d charges", tool_cloak);
        PrintLine(hwnd, memBuf);
        wsprintfA(memBuf, "  SLOW: %d charges", tool_slow);
        PrintLine(hwnd, memBuf);
        wsprintfA(memBuf, "  CREDITS: %d cr", player_credits);
        PrintLine(hwnd, memBuf);
        PrintLine(hwnd, "  NET: DISCONNECTED");
    } else if (lstrcmpiA(command, "reboot") == 0) {
        player_mem = 100;
        tool_cloak = 3;
        tool_slow = 3;
        lstrcpyA(node_files[2].name, "sys_logs.dat"); node_files[2].size = 12; node_files[2].value = 100;
        lstrcpyA(node_files[3].name, "customer_db.sql"); node_files[3].size = 45; node_files[3].value = 250;
        lstrcpyA(node_files[4].name, "r_and_d_schematics.zip"); node_files[4].size = 105; node_files[4].value = 600;
        lstrcpyA(node_files[5].name, "zero_day_exploit.exe"); node_files[5].size = 15; node_files[5].value = 1500;
        PrintLine(hwnd, "System rebooting...");
        PrintLine(hwnd, "Memory and software restored to 100%. Nodes reset.");
    } else if (lstrcmpiA(command, "map") == 0) {
        PrintLine(hwnd, "NETWORK TOPOLOGY:");
        PrintLine(hwnd, " [01] GATEWAY (LOCAL)");
        PrintLine(hwnd, "   |");
        PrintLine(hwnd, "   +-- [02] PUB_ROUTER");
        PrintLine(hwnd, "   |     |");
        PrintLine(hwnd, "   |     +-- [03] DATA_VAULT");
        PrintLine(hwnd, "   |");
        PrintLine(hwnd, "   +-- [04] SEC_SERVER");
        PrintLine(hwnd, "   |");
        PrintLine(hwnd, "   +-- [05] BLACK_ICE_NODE");
        PrintLine(hwnd, "");
        PrintLine(hwnd, "Use 'connect <node_id>' to access a node.");
    } else if (lstrcmpiA(command, "connect") == 0) {
        if (args[0] == '\0') {
            PrintLine(hwnd, "Error: No target node specified. Usage: connect <node_id>");
        } else {
            int node = 0;
            if (lstrcmpiA(args, "02") == 0) node = 2;
            else if (lstrcmpiA(args, "03") == 0) node = 3;
            else if (lstrcmpiA(args, "04") == 0) node = 4;
            else if (lstrcmpiA(args, "05") == 0) node = 5;
            
            if (node > 0) {
                hacking_node = node;
                hacking_attempts = 5;
                ice_frozen_ticks = 0;
                SetTimer(hwnd, 2, 3000, NULL);
                for(int i=0; i<4; i++) {
                    hacking_target[i] = '0' + (my_rand() % 10);
                }
                hacking_target[4] = '\0';
                
                if (node == 2) { lstrcpyA(ice_name, "Basic ICE"); ice_damage = 5; }
                else if (node == 3) { lstrcpyA(ice_name, "Tracer ICE"); ice_damage = 10; }
                else if (node == 4) { lstrcpyA(ice_name, "Hunter ICE"); ice_damage = 15; }
                else if (node == 5) { lstrcpyA(ice_name, "Black ICE"); ice_damage = 25; }

                char msg[MAX_LINE_LENGTH + 32];
                lstrcpyA(msg, "Attempting connection to node [");
                lstrcatA(msg, args);
                lstrcatA(msg, "]...");
                PrintLine(hwnd, msg);
                PrintLine(hwnd, "Establishing handshake...");
                char warnBuf[128];
                wsprintfA(warnBuf, "WARNING: %s detected on this node.", ice_name);
                PrintLine(hwnd, warnBuf);
                PrintLine(hwnd, "PASSWORD REQUIRED. INITIATING BRUTEFORCE MODULE...");
                PrintLine(hwnd, "Crack the 4-digit access code (0-9).");
                char buf[64];
                wsprintfA(buf, "Attempts remaining: %d", hacking_attempts);
                PrintLine(hwnd, buf);
            } else {
                char msg[MAX_LINE_LENGTH + 32];
                lstrcpyA(msg, "Attempting connection to node [");
                lstrcatA(msg, args);
                lstrcatA(msg, "]...");
                PrintLine(hwnd, msg);
                PrintLine(hwnd, "Establishing handshake...");
                PrintLine(hwnd, "Error: Connection refused. Invalid node or ICE active.");
            }
        }
    } else {
        lstrcpyA(buffer, "Command not found: ");
        lstrcatA(buffer, command);
        PrintLine(hwnd, buffer);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            my_srand(GetTickCount());
            hFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                FIXED_PITCH | FF_MODERN, "Courier New");
            PrintLine(hwnd, "KCyber OS booting...");
            PrintLine(hwnd, "Loading modules... OK");
            PrintLine(hwnd, "Initializing memory... OK");
            PrintLine(hwnd, "Type 'help' for a list of commands.");
            SetTimer(hwnd, 1, 530, NULL);
            SetTimer(hwnd, 2, 3000, NULL);
            break;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            KillTimer(hwnd, 2);
            DeleteObject(hFont);
            PostQuitMessage(0);
            return 0;

        case WM_TIMER:
            if (wParam == 1) {
                cursor_visible = !cursor_visible;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 2) {
                if ((hacking_node || connected_node) && ice_damage > 0) {
                    if (ice_frozen_ticks > 0) {
                        ice_frozen_ticks--;
                        char buf[128];
                        wsprintfA(buf, "[%s] ICE is blinded...", ice_name);
                        PrintLine(hwnd, buf);
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else {
                        player_mem -= ice_damage;
                        char buf[128];
                        if (player_mem <= 0) {
                            player_mem = 0;
                            wsprintfA(buf, "[%s] FATAL: Memory depleted. Connection forcefully terminated.", ice_name);
                            PrintLine(hwnd, buf);
                            hacking_node = 0;
                            connected_node = 0;
                        } else {
                            wsprintfA(buf, "[%s] attacks! System MEM reduced to %d%%", ice_name, player_mem);
                            PrintLine(hwnd, buf);
                        }
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            }
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &clientRect, bgBrush);
            DeleteObject(bgBrush);

            SelectObject(hdc, hFont);
            SetTextColor(hdc, RGB(0, 255, 0));
            SetBkColor(hdc, RGB(0, 0, 0));
            SetBkMode(hdc, OPAQUE);

            int lineHeight = 18;
            int maxLinesVisible = (clientRect.bottom - 20) / lineHeight;
            if (maxLinesVisible < 1) maxLinesVisible = 1;
            
            int startIdx = history_count - maxLinesVisible + 1;
            if (startIdx < 0) startIdx = 0;

            int y = 10;
            for (int i = startIdx; i < history_count; i++) {
                TextOutA(hdc, 10, y, history[i], lstrlenA(history[i]));
                y += lineHeight;
            }

            char prompt[MAX_LINE_LENGTH + 32];
            if (hacking_node) {
                wsprintfA(prompt, "[NODE 0%d] hack> %s", hacking_node, current_input);
            } else if (connected_node) {
                wsprintfA(prompt, "[NODE 0%d] root> %s", connected_node, current_input);
            } else {
                lstrcpyA(prompt, "root@cyberdeck:~# ");
                lstrcatA(prompt, current_input);
            }
            
            TextOutA(hdc, 10, y, prompt, lstrlenA(prompt));

            if (cursor_visible) {
                SIZE size;
                GetTextExtentPoint32A(hdc, prompt, lstrlenA(prompt), &size);
                GetTextExtentPoint32A(hdc, "A", 1, &size); // Get width of one character
                int charWidth = size.cx;
                GetTextExtentPoint32A(hdc, prompt, lstrlenA(prompt), &size);
                
                RECT cursorRect = { 10 + size.cx, y + 2, 10 + size.cx + charWidth, y + lineHeight - 2 };
                HBRUSH cursorBrush = CreateSolidBrush(RGB(0, 255, 0));
                FillRect(hdc, &cursorRect, cursorBrush);
                DeleteObject(cursorBrush);
            }

            // Draw scanlines
            HPEN scanPen = CreatePen(PS_SOLID, 1, RGB(0, 30, 0));
            HPEN oldPen = SelectObject(hdc, scanPen);
            for (int sy = 0; sy < clientRect.bottom; sy += 2) {
                MoveToEx(hdc, 0, sy, NULL);
                LineTo(hdc, clientRect.right, sy);
            }
            SelectObject(hdc, oldPen);
            DeleteObject(scanPen);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CHAR:
            if (wParam >= 32 && wParam <= 126) {
                if (current_input_len < MAX_LINE_LENGTH - 1) {
                    current_input[current_input_len++] = (char)wParam;
                    current_input[current_input_len] = '\0';
                    cursor_visible = 1;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == '\b' && current_input_len > 0) {
                current_input[--current_input_len] = '\0';
                cursor_visible = 1;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == '\r') {
                ProcessCommand(hwnd, current_input);
                current_input[0] = '\0';
                current_input_len = 0;
                cursor_visible = 1;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void __stdcall MainEntry(void) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    const char CLASS_NAME[] = "KCyberClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KCyber v1.0 - Cyberdeck Interface",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        ExitProcess(0);
    }

    ShowWindow(hwnd, SW_SHOW);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ExitProcess(0);
}
