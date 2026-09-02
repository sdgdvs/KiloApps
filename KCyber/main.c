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

int player_max_mem = 100;
int player_mem = 100;
int player_max_cpu = 5;
int player_max_cloak = 3;
int player_max_slow = 3;
int tool_cloak = 3;
int tool_slow = 3;
int in_shop = 0;
int ice_damage = 0;
char ice_name[32] = "";
int ice_frozen_ticks = 0;
int player_heat = 0;

typedef struct {
    int id;
    int node;
    char file[32];
    int reward;
    char diff[16];
} Mission;

Mission missions[3];
int active_mission_node = 0;
char active_mission_file[32] = "";

static unsigned long int my_next = 1;
int my_rand(void) {
    my_next = my_next * 1103515245 + 12345;
    return (unsigned int)(my_next / 65536) % 32768;
}
void my_srand(unsigned int seed) {
    my_next = seed;
}

// Fast integer sine lookup table (32 steps = 0..2*PI), output -128..127
static const signed char g_sinTable[32] = {
    0, 25, 49, 71, 90, 106, 117, 125, 127, 125, 117, 106, 90, 71, 49, 25,
    0, -25, -49, -71, -90, -106, -117, -125, -127, -125, -117, -106, -90, -71, -49, -25
};

int FastSin(int angleStep) {
    return (int)g_sinTable[angleStep & 31];
}

int FastCos(int angleStep) {
    return (int)g_sinTable[(angleStep + 8) & 31];
}

#define MAX_PARTICLES 64

typedef struct {
    int active;
    int x, y;
    int vx, vy;
    int life;
    int maxLife;
    COLORREF color;
    int type; // 0=spark, 1=data bit, 2=debris shard, 3=star
    int size;
} CyberParticle;

CyberParticle g_particles[MAX_PARTICLES];
int g_animTick = 0;
int g_shake = 0;
int g_shockwaveR = 0;
COLORREF g_shockwaveColor = RGB(255, 50, 50);

void SpawnParticles(int x, int y, int count, COLORREF color, int type) {
    for (int i = 0; i < count; i++) {
        for (int p = 0; p < MAX_PARTICLES; p++) {
            if (!g_particles[p].active) {
                g_particles[p].active = 1;
                g_particles[p].x = x + (my_rand() % 16) - 8;
                g_particles[p].y = y + (my_rand() % 16) - 8;
                g_particles[p].vx = (my_rand() % 9) - 4;
                g_particles[p].vy = (my_rand() % 9) - 4;
                if (type == 1) { // data motes drift upward
                    g_particles[p].vy = -(2 + (my_rand() % 3));
                    g_particles[p].vx = (my_rand() % 5) - 2;
                }
                g_particles[p].life = 12 + (my_rand() % 12);
                g_particles[p].maxLife = g_particles[p].life;
                g_particles[p].color = color;
                g_particles[p].type = type;
                g_particles[p].size = 2 + (my_rand() % 3);
                break;
            }
        }
    }
}

void GenerateMissions() {
    const char* fileNames[] = {"paydata.zip", "prototype.exe", "blackmail.txt", "employee_db.sql", "admin_creds.dat", "source_code.c", "financials.xls", "auth_keys.rsa"};
    for (int i = 0; i < 3; i++) {
        missions[i].id = i + 1;
        missions[i].node = (my_rand() % 4) + 2;
        int fidx = my_rand() % 8;
        lstrcpyA(missions[i].file, fileNames[fidx]);
        if (missions[i].node == 2) { missions[i].reward = 150 + (my_rand()%100); lstrcpyA(missions[i].diff, "Easy"); }
        else if (missions[i].node == 3) { missions[i].reward = 300 + (my_rand()%200); lstrcpyA(missions[i].diff, "Medium"); }
        else if (missions[i].node == 4) { missions[i].reward = 600 + (my_rand()%300); lstrcpyA(missions[i].diff, "Hard"); }
        else if (missions[i].node == 5) { missions[i].reward = 1200 + (my_rand()%800); lstrcpyA(missions[i].diff, "Extreme"); }
    }
}

void PlayKeyClack() { Beep(800, 10); }
void PlayAccessGranted() {
    Beep(400, 100); Beep(600, 100); Beep(800, 200);
}
void PlayAccessDenied() {
    Beep(150, 300); Beep(150, 400);
}
void PlayAlarm() {
    Beep(600, 200); Beep(400, 200);
}
void PlayDialup() {
    for(int i=0; i<10; i++) {
        Beep(500 + (my_rand() % 1000), 100);
    }
}
void PlayFailTone() { Beep(300, 100); }


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

    if (in_shop) {
        wsprintfA(buffer, "shop> %s", cmd);
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

        if (lstrcmpiA(command, "exit") == 0 || lstrcmpiA(command, "abort") == 0) {
            in_shop = 0;
            PrintLine(hwnd, "Exiting shop.");
            return;
        }

        if (lstrcmpiA(command, "ls") == 0 || lstrcmpiA(command, "list") == 0) {
            PrintLine(hwnd, "--- UPGRADE SHOP ---");
            wsprintfA(buffer, "Credits: %d cr", player_credits);
            PrintLine(hwnd, buffer);
            PrintLine(hwnd, "Available upgrades:");
            PrintLine(hwnd, "  buy mem   - +50 Max Memory (Cost: 500 cr)");
            PrintLine(hwnd, "  buy cpu   - +1 Hacking Attempt (Cost: 1000 cr)");
            PrintLine(hwnd, "  buy cloak - +1 Cloak Charge (Cost: 200 cr)");
            PrintLine(hwnd, "  buy slow  - +1 Slow Charge (Cost: 300 cr)");
            PrintLine(hwnd, "  buy proxy - -20% Heat (Cost: 150 cr)");
            return;
        }

        if (lstrcmpiA(command, "buy") == 0) {
            if (args[0] == '\0') {
                PrintLine(hwnd, "Usage: buy <item>");
            } else {
                if (lstrcmpiA(args, "mem") == 0) {
                    if (player_credits >= 500) {
                        player_credits -= 500;
                        player_max_mem += 50;
                        player_mem += 50;
                        wsprintfA(buffer, "Purchase successful. Max memory increased to %d.", player_max_mem);
                        PrintLine(hwnd, buffer);
                    } else {
                        PrintLine(hwnd, "Insufficient credits.");
                    }
                } else if (lstrcmpiA(args, "cpu") == 0) {
                    if (player_credits >= 1000) {
                        player_credits -= 1000;
                        player_max_cpu += 1;
                        wsprintfA(buffer, "Purchase successful. Hacking attempts increased to %d.", player_max_cpu);
                        PrintLine(hwnd, buffer);
                    } else {
                        PrintLine(hwnd, "Insufficient credits.");
                    }
                } else if (lstrcmpiA(args, "cloak") == 0) {
                    if (player_credits >= 200) {
                        player_credits -= 200;
                        player_max_cloak += 1;
                        tool_cloak += 1;
                        wsprintfA(buffer, "Purchase successful. Max Cloak charges: %d.", player_max_cloak);
                        PrintLine(hwnd, buffer);
                    } else {
                        PrintLine(hwnd, "Insufficient credits.");
                    }
                } else if (lstrcmpiA(args, "slow") == 0) {
                    if (player_credits >= 300) {
                        player_credits -= 300;
                        player_max_slow += 1;
                        tool_slow += 1;
                        wsprintfA(buffer, "Purchase successful. Max Slow charges: %d.", player_max_slow);
                        PrintLine(hwnd, buffer);
                    } else {
                        PrintLine(hwnd, "Insufficient credits.");
                    }
                } else if (lstrcmpiA(args, "proxy") == 0) {
                    if (player_credits >= 150) {
                        player_credits -= 150;
                        player_heat -= 20;
                        if (player_heat < 0) player_heat = 0;
                        wsprintfA(buffer, "Purchase successful. Heat reduced to %d%%.", player_heat);
                        PrintLine(hwnd, buffer);
                    } else {
                        PrintLine(hwnd, "Insufficient credits.");
                    }
                } else {
                    PrintLine(hwnd, "Unknown item. Use 'ls' to see available items.");
                }
            }
            return;
        }

        PrintLine(hwnd, "Command not found. Available: ls, buy <item>, exit");
        return;
    }

    if (hacking_node) {
        wsprintfA(buffer, "[NODE 0%d] hack> %s", hacking_node, cmd);
        PrintLine(hwnd, buffer);
        
        char guess[MAX_LINE_LENGTH];
        lstrcpynA(guess, cmd, MAX_LINE_LENGTH);
        if (lstrcmpiA(guess, "abort") == 0) {
            hacking_node = 0;
            PrintLine(hwnd, "Hacking aborted.");
            player_heat += 5;
            if (player_heat > 100) player_heat = 100;
            wsprintfA(buffer, "[WARNING] Heat increased to %d%%", player_heat);
            PrintLine(hwnd, buffer);
            return;
        }
        if (lstrcmpiA(guess, "cloak") == 0) {
            if (tool_cloak > 0) {
                tool_cloak--;
                ice_frozen_ticks += 2;
                wsprintfA(buffer, "[CLOAK] Activated. %d remaining. ICE blinded for 2 cycles.", tool_cloak);
                PrintLine(hwnd, buffer);
                RECT cr; GetClientRect(hwnd, &cr);
                g_shockwaveR = 5; g_shockwaveColor = RGB(0, 220, 255);
                SpawnParticles(cr.right - 90, 85, 20, RGB(0, 220, 255), 3);
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
                int current_interval = 3000 - (player_heat * 15);
                if (current_interval < 500) current_interval = 500;
                SetTimer(hwnd, 2, current_interval * 2, NULL);
                RECT cr; GetClientRect(hwnd, &cr);
                SpawnParticles(cr.right - 90, 85, 15, RGB(255, 255, 0), 0);
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
            PlayAccessGranted();
            g_shake = 5;
            g_shockwaveR = 6;
            g_shockwaveColor = RGB(0, 255, 150);
            RECT cr; GetClientRect(hwnd, &cr);
            SpawnParticles(cr.right - 90, 85, 30, RGB(0, 255, 200), 3);
            SpawnParticles(cr.right - 90, 85, 20, RGB(255, 255, 100), 0);
            wsprintfA(buffer, "ACCESS GRANTED. Connected to node [0%d].", hacking_node);
            PrintLine(hwnd, buffer);
            PrintLine(hwnd, "Type 'ls' to list files, 'download <file>' to extract, 'disconnect' to exit.");
            connected_node = hacking_node;
            hacking_node = 0;
        } else {
            hacking_attempts--;
            PlayFailTone();
            g_shake = 3;
            RECT cr; GetClientRect(hwnd, &cr);
            SpawnParticles(cr.right - 90, 85, 12, RGB(255, 170, 0), 0);
            wsprintfA(buffer, "Result: %d EXACT, %d PARTIAL", exact, partial);
            PrintLine(hwnd, buffer);
            if (hacking_attempts > 0) {
                wsprintfA(buffer, "Attempts remaining: %d", hacking_attempts);
                PrintLine(hwnd, buffer);
            } else {
                PlayAccessDenied();
                g_shake = 12;
                g_shockwaveR = 6;
                g_shockwaveColor = RGB(255, 20, 20);
                SpawnParticles(cr.right - 90, 85, 25, RGB(255, 30, 30), 2);
                PrintLine(hwnd, "ACCESS DENIED. TRACE DETECTED. CONNECTION TERMINATED.");
                player_heat += 20;
                if (player_heat > 100) player_heat = 100;
                wsprintfA(buffer, "[WARNING] Global heat increased to %d%%", player_heat);
                PrintLine(hwnd, buffer);
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
                    RECT cr; GetClientRect(hwnd, &cr);
                    SpawnParticles(cr.right - 90, 85, 25, RGB(0, 255, 255), 1);
                    SpawnParticles(cr.right - 90, 85, 15, RGB(255, 255, 255), 3);
                    
                    if (active_mission_node == connected_node && lstrcmpiA(active_mission_file, args) == 0) {
                        PrintLine(hwnd, "CONTRACT COMPLETE! Reward transferred.");
                        active_mission_node = 0;
                        active_mission_file[0] = '\0';
                    }
                    
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
        PrintLine(hwnd, "  help      - Show this message");
        PrintLine(hwnd, "  clear     - Clear terminal output");
        PrintLine(hwnd, "  status    - Show deck status");
        PrintLine(hwnd, "  reboot    - Restart deck to restore MEM and software");
        PrintLine(hwnd, "  map       - Display network topology");
        PrintLine(hwnd, "  shop      - Enter upgrade shop");
        PrintLine(hwnd, "  connect   - Attempt connection to network node");
        PrintLine(hwnd, "  contracts - View available hacking contracts");
        PrintLine(hwnd, "  accept    - Accept a contract (e.g. 'accept 1')");
        PrintLine(hwnd, "  guide     - Open Runner's Guide (e.g. 'guide ice')");
        PrintLine(hwnd, "During hack:");
        PrintLine(hwnd, "  abort     - Disconnect immediately");
        PrintLine(hwnd, "  cloak     - Blind ICE for 2 cycles");
        PrintLine(hwnd, "  slow      - Halve ICE attack speed");
    } else if (lstrcmpiA(command, "guide") == 0) {
        if (args[0] == '\0') {
            PrintLine(hwnd, "--- RUNNER'S GUIDE ---");
            PrintLine(hwnd, "Usage: guide <topic>");
            PrintLine(hwnd, "Topics: commands, ice, upgrade, hacking");
        } else {
            if (lstrcmpiA(args, "commands") == 0) {
                PrintLine(hwnd, "--- GUIDE: COMMANDS ---");
                PrintLine(hwnd, "connect <node>: Initiates a hack against a target node.");
                PrintLine(hwnd, "shop: Access the black market to upgrade your cyberdeck.");
                PrintLine(hwnd, "contracts: List available data theft jobs.");
                PrintLine(hwnd, "accept <id>: Take a contract, then connect to the node to download.");
            } else if (lstrcmpiA(args, "ice") == 0) {
                PrintLine(hwnd, "--- GUIDE: ICE (Intrusion Countermeasures Electronics) ---");
                PrintLine(hwnd, "ICE defends network nodes by attacking your deck's Memory (MEM).");
                PrintLine(hwnd, "Basic ICE: Weak damage. Found on low-sec nodes.");
                PrintLine(hwnd, "Tracer ICE: Moderate damage.");
                PrintLine(hwnd, "Hunter ICE: High damage. Will rapidly deplete your MEM.");
                PrintLine(hwnd, "Black ICE: Lethal damage. Reserved for extreme-sec nodes.");
                PrintLine(hwnd, "If MEM reaches 0, you are forcibly disconnected.");
            } else if (lstrcmpiA(args, "upgrade") == 0) {
                PrintLine(hwnd, "--- GUIDE: DECK UPGRADING ---");
                PrintLine(hwnd, "Spend credits earned from data theft in the 'shop'.");
                PrintLine(hwnd, "MEM: Increases maximum memory, letting you survive more ICE attacks.");
                PrintLine(hwnd, "CPU: Gives you more attempts to crack node passwords.");
                PrintLine(hwnd, "CLOAK: Buy charges to temporarily blind ICE during a hack.");
                PrintLine(hwnd, "SLOW: Buy charges to permanently slow down ICE attack speed for one hack.");
                PrintLine(hwnd, "PROXY: Reduces your global HEAT, which makes ICE attack slower.");
            } else if (lstrcmpiA(args, "hacking") == 0) {
                PrintLine(hwnd, "--- GUIDE: HACKING ---");
                PrintLine(hwnd, "1. Connect to a node. You must crack a 4-digit PIN.");
                PrintLine(hwnd, "2. Enter 4 digits. The system returns EXACT (right number, right place)");
                PrintLine(hwnd, "   and PARTIAL (right number, wrong place).");
                PrintLine(hwnd, "3. Use this feedback to deduce the PIN before you run out of attempts.");
                PrintLine(hwnd, "4. Once in, use 'ls' to find files and 'download <file>' to steal them.");
                PrintLine(hwnd, "5. Higher HEAT speeds up ICE. Aborting or failing hacks increases HEAT.");
            } else {
                PrintLine(hwnd, "Unknown topic. Topics: commands, ice, upgrade, hacking");
            }
        }
    } else if (lstrcmpiA(command, "clear") == 0) {
        history_count = 0;
        InvalidateRect(hwnd, NULL, TRUE);
    } else if (lstrcmpiA(command, "shop") == 0) {
        in_shop = 1;
        PrintLine(hwnd, "Entering upgrade shop... Type 'ls' to view items, 'exit' to leave.");
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
        wsprintfA(memBuf, "  HEAT: %d%%", player_heat);
        PrintLine(hwnd, memBuf);
        PrintLine(hwnd, "  NET: DISCONNECTED");
    } else if (lstrcmpiA(command, "reboot") == 0) {
        player_mem = player_max_mem;
        tool_cloak = player_max_cloak;
        tool_slow = player_max_slow;
        if (player_heat > 0) {
            player_heat -= 10;
            if (player_heat < 0) player_heat = 0;
        }
        active_mission_node = 0;
        active_mission_file[0] = '\0';
        GenerateMissions();
        lstrcpyA(node_files[2].name, "sys_logs.dat"); node_files[2].size = 12; node_files[2].value = 100;
        lstrcpyA(node_files[3].name, "customer_db.sql"); node_files[3].size = 45; node_files[3].value = 250;
        lstrcpyA(node_files[4].name, "r_and_d_schematics.zip"); node_files[4].size = 105; node_files[4].value = 600;
        lstrcpyA(node_files[5].name, "zero_day_exploit.exe"); node_files[5].size = 15; node_files[5].value = 1500;
        PrintLine(hwnd, "System rebooting...");
        PrintLine(hwnd, "Memory and software restored to 100%. Nodes and contracts reset.");
        char buf[64];
        wsprintfA(buf, "Heat cooled down to %d%%.", player_heat);
        PrintLine(hwnd, buf);
    } else if (lstrcmpiA(command, "contracts") == 0) {
        if (missions[0].id == 0 && missions[1].id == 0 && missions[2].id == 0) GenerateMissions();
        PrintLine(hwnd, "AVAILABLE CONTRACTS:");
        for(int k=0; k<3; k++) {
            if (missions[k].id != 0) {
                char b[128];
                wsprintfA(b, " [%d] Target Node: 0%d | File: %s | Diff: %s | Reward: %d cr", missions[k].id, missions[k].node, missions[k].file, missions[k].diff, missions[k].reward);
                PrintLine(hwnd, b);
            }
        }
        PrintLine(hwnd, "Use 'accept <id>' to take a contract.");
    } else if (lstrcmpiA(command, "accept") == 0) {
        if (args[0] == '\0') {
            PrintLine(hwnd, "Usage: accept <id>");
        } else {
            int id = args[0] - '0';
            int found = 0;
            for(int k=0; k<3; k++) {
                if (missions[k].id == id) {
                    active_mission_node = missions[k].node;
                    lstrcpyA(active_mission_file, missions[k].file);
                    
                    lstrcpyA(node_files[missions[k].node].name, missions[k].file);
                    node_files[missions[k].node].size = 10 + (my_rand() % 90);
                    node_files[missions[k].node].value = missions[k].reward;
                    
                    char b[128];
                    wsprintfA(b, "Contract [%d] accepted.", missions[k].id);
                    PrintLine(hwnd, b);
                    wsprintfA(b, "Objective: Download %s from NODE 0%d.", missions[k].file, missions[k].node);
                    PrintLine(hwnd, b);
                    
                    missions[k].id = 0;
                    found = 1;
                    
                    int all_taken = 1;
                    for(int j=0; j<3; j++) { if(missions[j].id != 0) all_taken = 0; }
                    if(all_taken) GenerateMissions();
                    
                    break;
                }
            }
            if (!found) {
                PrintLine(hwnd, "Invalid contract ID.");
            }
        }
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
                hacking_attempts = player_max_cpu;
                ice_frozen_ticks = 0;
                int current_interval = 3000 - (player_heat * 15);
                if (current_interval < 500) current_interval = 500;
                SetTimer(hwnd, 2, current_interval, NULL);
                for(int i=0; i<4; i++) {
                    hacking_target[i] = '0' + (my_rand() % 10);
                }
                hacking_target[4] = '\0';
                
                if (node == 2) { lstrcpyA(ice_name, "Basic ICE"); ice_damage = 5; }
                else if (node == 3) { lstrcpyA(ice_name, "Tracer ICE"); ice_damage = 10; }
                else if (node == 4) { lstrcpyA(ice_name, "Hunter ICE"); ice_damage = 15; }
                else if (node == 5) { lstrcpyA(ice_name, "Black ICE"); ice_damage = 25; }
                
                ice_damage += player_heat / 5;

                char msg[MAX_LINE_LENGTH + 32];
                lstrcpyA(msg, "Attempting connection to node [");
                lstrcatA(msg, args);
                lstrcatA(msg, "]...");
                PrintLine(hwnd, msg);
                PlayDialup();
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
            SetTimer(hwnd, 3, 33, NULL);
            break;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            KillTimer(hwnd, 2);
            KillTimer(hwnd, 3);
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
                        RECT cr; GetClientRect(hwnd, &cr);
                        SpawnParticles(cr.right - 100, 95, 12, RGB(0, 220, 255), 3);
                        InvalidateRect(hwnd, NULL, FALSE);
                    } else {
                        PlayAlarm();
                        player_mem -= ice_damage;
                        char buf[128];
                        if (player_mem <= 0) {
                            player_mem = 0;
                            g_shake = 22;
                            g_shockwaveR = 8;
                            g_shockwaveColor = RGB(255, 0, 0);
                            RECT cr; GetClientRect(hwnd, &cr);
                            SpawnParticles(cr.right - 100, 95, 30, RGB(255, 20, 20), 2);
                            SpawnParticles(cr.right - 100, 95, 25, RGB(255, 255, 255), 0);
                            wsprintfA(buf, "[%s] FATAL: Memory depleted. Connection forcefully terminated.", ice_name);
                            PrintLine(hwnd, buf);
                            PlayAccessDenied();
                            hacking_node = 0;
                            connected_node = 0;
                        } else {
                            g_shake = 12;
                            g_shockwaveR = 6;
                            g_shockwaveColor = RGB(255, 30, 30);
                            RECT cr; GetClientRect(hwnd, &cr);
                            SpawnParticles(cr.right - 100, 95, 20, RGB(255, 50, 50), 2);
                            SpawnParticles(cr.right - 100, 95, 15, RGB(255, 180, 0), 0);
                            wsprintfA(buf, "[%s] attacks! System MEM reduced to %d%%", ice_name, player_mem);
                            PrintLine(hwnd, buf);
                        }
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                }
            } else if (wParam == 3) {
                g_animTick++;
                if (g_shake > 0) g_shake--;
                if (g_shockwaveR > 0) {
                    g_shockwaveR += 3;
                    if (g_shockwaveR > 65) g_shockwaveR = 0;
                }

                // Update particle physics
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    if (g_particles[i].active) {
                        g_particles[i].x += g_particles[i].vx;
                        g_particles[i].y += g_particles[i].vy;
                        if (g_particles[i].type == 2) {
                            g_particles[i].vy += 1; // debris gravity
                        } else if (g_particles[i].type == 0) {
                            g_particles[i].vx = (g_particles[i].vx * 9) / 10;
                            g_particles[i].vy = (g_particles[i].vy * 9) / 10;
                        }
                        g_particles[i].life--;
                        if (g_particles[i].life <= 0) {
                            g_particles[i].active = 0;
                        }
                    }
                }

                // Ambient particle spawns in visual viewport
                if ((g_animTick % 7) == 0) {
                    RECT cr;
                    GetClientRect(hwnd, &cr);
                    int vx = cr.right - 180;
                    int vy = 25;
                    if (vx > 220) {
                        if (hacking_node) {
                            if (ice_frozen_ticks > 0) {
                                SpawnParticles(vx + 80, vy + 75, 1, RGB(0, 220, 255), 3);
                            } else {
                                SpawnParticles(vx + 80, vy + 75, 1, RGB(255, 50, 50), 0);
                            }
                        } else if (connected_node) {
                            SpawnParticles(vx + 80, vy + 105, 1, RGB(0, 255, 255), 1);
                        } else {
                            SpawnParticles(vx + 80, vy + 95, 1, RGB(0, 255, 100), 1);
                        }
                    }
                }

                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            // Double buffering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
            HGDIOBJ oldBitmap = SelectObject(memDC, memBitmap);

            HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(memDC, &clientRect, bgBrush);
            DeleteObject(bgBrush);

            SelectObject(memDC, hFont);
            SetTextColor(memDC, RGB(0, 255, 0));
            SetBkColor(memDC, RGB(0, 0, 0));
            SetBkMode(memDC, OPAQUE);

            int lineHeight = 18;
            int maxLinesVisible = (clientRect.bottom - 20) / lineHeight;
            if (maxLinesVisible < 1) maxLinesVisible = 1;
            
            int startIdx = history_count - maxLinesVisible + 1;
            if (startIdx < 0) startIdx = 0;

            int y = 10;
            for (int i = startIdx; i < history_count; i++) {
                TextOutA(memDC, 10, y, history[i], lstrlenA(history[i]));
                y += lineHeight;
            }

            char prompt[MAX_LINE_LENGTH + 32];
            if (hacking_node) {
                wsprintfA(prompt, "[NODE 0%d] hack> %s", hacking_node, current_input);
            } else if (connected_node) {
                wsprintfA(prompt, "[NODE 0%d] root> %s", connected_node, current_input);
            } else if (in_shop) {
                wsprintfA(prompt, "shop> %s", current_input);
            } else {
                lstrcpyA(prompt, "root@cyberdeck:~# ");
                lstrcatA(prompt, current_input);
            }
            
            TextOutA(memDC, 10, y, prompt, lstrlenA(prompt));

            if (cursor_visible) {
                SIZE size;
                GetTextExtentPoint32A(memDC, prompt, lstrlenA(prompt), &size);
                GetTextExtentPoint32A(memDC, "A", 1, &size); // Get width of one character
                int charWidth = size.cx;
                GetTextExtentPoint32A(memDC, prompt, lstrlenA(prompt), &size);
                
                RECT cursorRect = { 10 + size.cx, y + 2, 10 + size.cx + charWidth, y + lineHeight - 2 };
                HBRUSH cursorBrush = CreateSolidBrush(RGB(0, 255, 0));
                FillRect(memDC, &cursorRect, cursorBrush);
                DeleteObject(cursorBrush);
            }

            // --- ANIMATED CYBER HUD & SPRITE ENGINE ---
            int vw = 160;
            int vh = 145;
            int vx = clientRect.right - vw - 20;
            int vy = 25;
            if (vx > 220) { // ensure space available
                int cx = vx + vw / 2;
                int cy = vy + 75;

                // HUD Box Background
                RECT hudBox = { vx, vy, vx + vw, vy + vh };
                HBRUSH hudBg = CreateSolidBrush(RGB(0, 15, 5));
                FillRect(memDC, &hudBox, hudBg);
                DeleteObject(hudBg);

                // HUD Color & Theme
                COLORREF hudColor = RGB(0, 255, 100);
                const char* hudTitle = "[SYS://DECK.OS]";
                if (hacking_node) {
                    hudColor = ice_frozen_ticks > 0 ? RGB(0, 220, 255) : RGB(255, 50, 50);
                    hudTitle = ice_frozen_ticks > 0 ? "[ICE://BLINDED]" : "[ICE://TARGET]";
                } else if (connected_node) {
                    hudColor = RGB(0, 255, 255);
                    hudTitle = "[DATA://ROOT]";
                }

                // Corner L-Brackets
                HPEN hudPen = CreatePen(PS_SOLID, 2, hudColor);
                HGDIOBJ oldPen = SelectObject(memDC, hudPen);
                int bLen = 10;
                // Top-Left
                MoveToEx(memDC, vx, vy + bLen, NULL); LineTo(memDC, vx, vy); LineTo(memDC, vx + bLen, vy);
                // Top-Right
                MoveToEx(memDC, vx + vw - bLen, vy, NULL); LineTo(memDC, vx + vw, vy); LineTo(memDC, vx + vw, vy + bLen);
                // Bottom-Left
                MoveToEx(memDC, vx, vy + vh - bLen, NULL); LineTo(memDC, vx, vy + vh); LineTo(memDC, vx + bLen, vy + vh);
                // Bottom-Right
                MoveToEx(memDC, vx + vw - bLen, vy + vh, NULL); LineTo(memDC, vx + vw, vy + vh); LineTo(memDC, vx + vw, vy + vh - bLen);

                // Top Header Text & Status Diode
                SetTextColor(memDC, hudColor);
                SetBkMode(memDC, TRANSPARENT);
                TextOutA(memDC, vx + 22, vy + 4, hudTitle, lstrlenA(hudTitle));

                // Blinking Diode
                if ((g_animTick & 8) != 0) {
                    HBRUSH diodeBrush = CreateSolidBrush(hudColor);
                    HGDIOBJ oldD = SelectObject(memDC, diodeBrush);
                    Ellipse(memDC, vx + 8, vy + 6, vx + 16, vy + 14);
                    SelectObject(memDC, oldD);
                    DeleteObject(diodeBrush);
                }

                if (hacking_node) {
                    // --- MAIN ENEMY: ICE DAEMON ---
                    int pulse = (FastSin(g_animTick * 3) * 5) / 128;
                    int glitch = ((g_animTick & 15) == 0) ? ((g_animTick % 5) - 2) : 0;

                    if (ice_frozen_ticks > 0) {
                        // Frozen ICE: Stun Barrier & Hexagonal Crystal Spikes
                        HPEN frostPen = CreatePen(PS_SOLID, 2, RGB(0, 220, 255));
                        HBRUSH frostBrush = CreateSolidBrush(RGB(0, 40, 60));
                        SelectObject(memDC, frostPen);
                        SelectObject(memDC, frostBrush);

                        // Outer Frost Snowflake Spokes
                        for (int a = 0; a < 6; a++) {
                            int ang = (g_animTick / 2) + a * 5;
                            int fx = cx + (FastCos(ang) * 44) / 128;
                            int fy = cy + (FastSin(ang) * 44) / 128;
                            MoveToEx(memDC, cx, cy, NULL);
                            LineTo(memDC, fx, fy);
                            Rectangle(memDC, fx - 2, fy - 2, fx + 3, fy + 3);
                        }

                        // Hexagon Frost Core
                        POINT hPts[6];
                        for (int i = 0; i < 6; i++) {
                            int ang = i * 5 + 2;
                            hPts[i].x = cx + (FastCos(ang) * 26) / 128;
                            hPts[i].y = cy + (FastSin(ang) * 26) / 128;
                        }
                        Polygon(memDC, hPts, 6);

                        DeleteObject(frostPen);
                        DeleteObject(frostBrush);

                        SetTextColor(memDC, RGB(0, 220, 255));
                        TextOutA(memDC, cx - 24, cy + 40, "BLINDED", 7);

                    } else {
                        // ACTIVE ICE DAEMON: Orbiting Razor Shards & Demonic Core
                        // 6 Orbiting Razor Defense Shards
                        HPEN shardPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                        HBRUSH shardBrush = CreateSolidBrush(RGB(255, 30, 30));
                        HPEN linkPen = CreatePen(PS_SOLID, 1, RGB(180, 20, 20));

                        for (int s = 0; s < 6; s++) {
                            int sAngle = (g_animTick * 2) + (s * 5);
                            int sDist = 42 + pulse;
                            int sx = cx + glitch + (FastCos(sAngle) * sDist) / 128;
                            int sy = cy + (FastSin(sAngle) * sDist) / 128;

                            // Laser Link
                            SelectObject(memDC, linkPen);
                            MoveToEx(memDC, cx + glitch, cy, NULL);
                            LineTo(memDC, sx, sy);

                            // Triangular Razor Shard
                            SelectObject(memDC, shardPen);
                            SelectObject(memDC, shardBrush);
                            int tipX = sx + (FastCos(sAngle) * 7) / 128;
                            int tipY = sy + (FastSin(sAngle) * 7) / 128;
                            int perpA = sAngle + 8;
                            POINT sPts[3] = {
                                { tipX, tipY },
                                { sx + (FastCos(perpA) * 5) / 128, sy + (FastSin(perpA) * 5) / 128 },
                                { sx - (FastCos(perpA) * 5) / 128, sy - (FastSin(perpA) * 5) / 128 }
                            };
                            Polygon(memDC, sPts, 3);
                        }
                        DeleteObject(shardPen);
                        DeleteObject(shardBrush);
                        DeleteObject(linkPen);

                        // Central Demonic ICE Core (8-point hazard star polygon)
                        HPEN corePen = CreatePen(PS_SOLID, 2, RGB(255, 50, 50));
                        HBRUSH coreBrush = CreateSolidBrush(RGB(130, 0, 0));
                        SelectObject(memDC, corePen);
                        SelectObject(memDC, coreBrush);

                        POINT octPts[8];
                        for (int i = 0; i < 8; i++) {
                            int r = (i % 2 == 0) ? (26 + pulse) : 17;
                            int ang = i * 4;
                            octPts[i].x = cx + glitch + (FastCos(ang) * r) / 128;
                            octPts[i].y = cy + (FastSin(ang) * r) / 128;
                        }
                        Polygon(memDC, octPts, 8);
                        DeleteObject(corePen);
                        DeleteObject(coreBrush);

                        // Visor Slit & Sweeping Eye
                        HBRUSH eyeBg = CreateSolidBrush(RGB(0, 0, 0));
                        RECT visor = { cx + glitch - 16, cy - 3, cx + glitch + 16, cy + 3 };
                        FillRect(memDC, &visor, eyeBg);
                        DeleteObject(eyeBg);

                        int eyeX = cx + glitch + (FastSin(g_animTick * 3) * 11) / 128;
                        HBRUSH eyeBrush = CreateSolidBrush(RGB(255, 0, 0));
                        SelectObject(memDC, eyeBrush);
                        Ellipse(memDC, eyeX - 2, cy - 3, eyeX + 3, cy + 3);
                        DeleteObject(eyeBrush);
                    }

                } else if (connected_node) {
                    // --- KEY ITEM: CLASSIFIED DATA CORE / VAULT ---
                    int bobY = (FastSin(g_animTick) * 4) / 128;

                    // Holographic Extraction Beam Lines
                    HPEN beamPen = CreatePen(PS_SOLID, 1, RGB(0, 80, 100));
                    SelectObject(memDC, beamPen);
                    MoveToEx(memDC, cx - 25, vy + vh - 5, NULL); LineTo(memDC, cx - 12, cy + bobY + 28);
                    MoveToEx(memDC, cx + 25, vy + vh - 5, NULL); LineTo(memDC, cx + 12, cy + bobY + 28);
                    DeleteObject(beamPen);

                    // Dual Intersecting Holographic Data Rings
                    HPEN ringPen = CreatePen(PS_SOLID, 1, RGB(0, 200, 255));
                    SelectObject(memDC, ringPen);
                    int rWidth1 = 38 + (FastCos(g_animTick * 2) * 6) / 128;
                    Arc(memDC, cx - rWidth1, cy + bobY - 14, cx + rWidth1, cy + bobY + 14, 0, 0, 0, 0);
                    DeleteObject(ringPen);

                    // High-Tech Data Cartridge
                    int cartW = 42;
                    int cartH = 54;
                    int cartX = cx - cartW / 2;
                    int cartY = cy + bobY - cartH / 2;

                    HPEN cartPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 255));
                    HBRUSH cartBrush = CreateSolidBrush(RGB(0, 30, 40));
                    SelectObject(memDC, cartPen);
                    SelectObject(memDC, cartBrush);
                    Rectangle(memDC, cartX, cartY, cartX + cartW, cartY + cartH);
                    DeleteObject(cartPen);
                    DeleteObject(cartBrush);

                    // Gold Connector Pins
                    HBRUSH goldBrush = CreateSolidBrush(RGB(255, 215, 0));
                    for (int p = 0; p < 4; p++) {
                        RECT pin = { cartX + 6 + p * 8, cartY + cartH - 4, cartX + 10 + p * 8, cartY + cartH };
                        FillRect(memDC, &pin, goldBrush);
                    }
                    DeleteObject(goldBrush);

                    // Central Microchip
                    HPEN chipPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 200));
                    HBRUSH chipBrush = CreateSolidBrush(RGB(0, 60, 80));
                    SelectObject(memDC, chipPen);
                    SelectObject(memDC, chipBrush);
                    Rectangle(memDC, cx - 8, cy + bobY - 7, cx + 8, cy + bobY + 7);
                    DeleteObject(chipPen);
                    DeleteObject(chipBrush);

                    // Traveling Specular Sheen Stripe
                    int sheenOff = (g_animTick * 2) % (cartH + 15);
                    if (sheenOff < cartH) {
                        HPEN sheenPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                        SelectObject(memDC, sheenPen);
                        MoveToEx(memDC, cartX + 2, cartY + sheenOff, NULL);
                        LineTo(memDC, cartX + cartW - 2, cartY + sheenOff);
                        DeleteObject(sheenPen);
                    }

                    SetTextColor(memDC, RGB(0, 255, 255));
                    TextOutA(memDC, cx - 18, cartY + cartH - 16, "SEC-DAT", 7);

                } else {
                    // --- PLAYER: CYBERDECK CONSOLE ---
                    int bobY = (FastSin(g_animTick) * 3) / 128;

                    // Holographic Pedestal Ring
                    HPEN pedPen = CreatePen(PS_SOLID, 1, RGB(0, 100, 40));
                    SelectObject(memDC, pedPen);
                    Arc(memDC, cx - 44, cy + 34 + bobY / 2, cx + 44, cy + 48 + bobY / 2, 0, 0, 0, 0);

                    // Rotating Radar Beacon Blip
                    int pedAng = g_animTick;
                    int bx = cx + (FastCos(pedAng) * 44) / 128;
                    int by = cy + 41 + (FastSin(pedAng) * 7) / 128;
                    HBRUSH blipBrush = CreateSolidBrush(RGB(0, 255, 170));
                    SelectObject(memDC, blipBrush);
                    Ellipse(memDC, bx - 2, by - 2, bx + 3, by + 3);
                    DeleteObject(blipBrush);
                    DeleteObject(pedPen);

                    // Cyberdeck Base (Trapezoid keyboard chassis)
                    int baseY = cy + 10 + bobY;
                    HPEN deckPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 100));
                    HBRUSH deckBrush = CreateSolidBrush(RGB(0, 38, 12));
                    SelectObject(memDC, deckPen);
                    SelectObject(memDC, deckBrush);
                    POINT deckPts[4] = {
                        { cx - 36, baseY },
                        { cx + 36, baseY },
                        { cx + 46, baseY + 24 },
                        { cx - 46, baseY + 24 }
                    };
                    Polygon(memDC, deckPts, 4);

                    // Keyboard Lines
                    HPEN keyPen = CreatePen(PS_SOLID, 1, RGB(0, 140, 50));
                    SelectObject(memDC, keyPen);
                    MoveToEx(memDC, cx - 32, baseY + 7, NULL); LineTo(memDC, cx + 32, baseY + 7);
                    MoveToEx(memDC, cx - 38, baseY + 15, NULL); LineTo(memDC, cx + 38, baseY + 15);
                    DeleteObject(keyPen);

                    // 4 Diagnostic Status LEDs
                    COLORREF ledClrs[4] = { RGB(0, 255, 100), RGB(255, 255, 0), RGB(0, 255, 255), RGB(255, 0, 255) };
                    for (int l = 0; l < 4; l++) {
                        int on = (((g_animTick / 8) + l) % 4) == 0;
                        HBRUSH ledB = CreateSolidBrush(on ? ledClrs[l] : RGB(0, 45, 20));
                        SelectObject(memDC, ledB);
                        int lx = cx - 24 + l * 16;
                        Ellipse(memDC, lx - 2, baseY + 19, lx + 3, baseY + 24);
                        DeleteObject(ledB);
                    }
                    DeleteObject(deckPen);
                    DeleteObject(deckBrush);

                    // Angled Cyberdeck Screen Display
                    int scrW = 60;
                    int scrH = 34;
                    int scrX = cx - scrW / 2;
                    int scrY = baseY - scrH + 2;

                    HPEN scrPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 100));
                    HBRUSH scrBrush = CreateSolidBrush(RGB(0, 20, 5));
                    SelectObject(memDC, scrPen);
                    SelectObject(memDC, scrBrush);
                    Rectangle(memDC, scrX, scrY, scrX + scrW, scrY + scrH);
                    DeleteObject(scrPen);
                    DeleteObject(scrBrush);

                    // Oscilloscope Waveform on Screen
                    HPEN wavePen = CreatePen(PS_SOLID, 1, RGB(0, 255, 140));
                    SelectObject(memDC, wavePen);
                    for (int sx = 2; sx < scrW - 4; sx += 4) {
                        int wy1 = (scrY + scrH / 2) + (FastSin((sx + g_animTick * 3)) * 6) / 128;
                        int wy2 = (scrY + scrH / 2) + (FastSin((sx + 4 + g_animTick * 3)) * 6) / 128;
                        MoveToEx(memDC, scrX + sx, wy1, NULL);
                        LineTo(memDC, scrX + sx + 4, wy2);
                    }
                    DeleteObject(wavePen);

                    // Cyber Antenna & Signal Wave
                    HPEN antPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 100));
                    SelectObject(memDC, antPen);
                    int antX = scrX;
                    int antY = scrY + 4;
                    MoveToEx(memDC, antX, antY + 8, NULL);
                    LineTo(memDC, antX - 7, antY - 6);
                    DeleteObject(antPen);

                    int sigR = (g_animTick * 2) % 20;
                    HPEN sigPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 180));
                    SelectObject(memDC, sigPen);
                    Arc(memDC, antX - 7 - sigR, antY - 6 - sigR, antX - 7 + sigR, antY - 6 + sigR, 0, 0, 0, 0);
                    DeleteObject(sigPen);
                }

                // Shockwave Expansion Ring
                if (g_shockwaveR > 0) {
                    HPEN swPen = CreatePen(PS_SOLID, 2, g_shockwaveColor);
                    HGDIOBJ oldSw = SelectObject(memDC, swPen);
                    HGDIOBJ oldNull = SelectObject(memDC, GetStockObject(NULL_BRUSH));
                    Ellipse(memDC, cx - g_shockwaveR, cy - g_shockwaveR, cx + g_shockwaveR, cy + g_shockwaveR);
                    SelectObject(memDC, oldNull);
                    SelectObject(memDC, oldSw);
                    DeleteObject(swPen);
                }

                // Multi-Layered Particle System
                for (int p = 0; p < MAX_PARTICLES; p++) {
                    if (g_particles[p].active) {
                        int px = g_particles[p].x;
                        int py = g_particles[p].y;
                        int sz = g_particles[p].size;

                        HPEN pPen = CreatePen(PS_SOLID, 1, g_particles[p].color);
                        HBRUSH pBrush = CreateSolidBrush(g_particles[p].color);
                        SelectObject(memDC, pPen);
                        SelectObject(memDC, pBrush);

                        if (g_particles[p].type == 0) { // Spark
                            MoveToEx(memDC, px, py, NULL);
                            LineTo(memDC, px - g_particles[p].vx, py - g_particles[p].vy);
                        } else if (g_particles[p].type == 1) { // Data Bit
                            Rectangle(memDC, px - 1, py - 1, px + 2, py + 2);
                        } else if (g_particles[p].type == 2) { // Debris Shard
                            Rectangle(memDC, px - sz / 2, py - sz / 2, px + sz / 2, py + sz / 2);
                        } else if (g_particles[p].type == 3) { // Star
                            MoveToEx(memDC, px - sz, py, NULL); LineTo(memDC, px + sz, py);
                            MoveToEx(memDC, px, py - sz, NULL); LineTo(memDC, px, py + sz);
                        }

                        DeleteObject(pPen);
                        DeleteObject(pBrush);
                    }
                }

                SelectObject(memDC, oldPen);
                DeleteObject(hudPen);
            }

            // Draw Scanlines across the viewport
            HPEN scanPen = CreatePen(PS_SOLID, 1, RGB(0, 24, 0));
            SelectObject(memDC, scanPen);
            for (int sy = 0; sy < clientRect.bottom; sy += 2) {
                MoveToEx(memDC, 0, sy, NULL);
                LineTo(memDC, clientRect.right, sy);
            }
            DeleteObject(scanPen);

            // Blit double buffer to screen with Screen Shake offset
            int shakeX = 0, shakeY = 0;
            if (g_shake > 0) {
                shakeX = (my_rand() % (g_shake * 2 + 1)) - g_shake;
                shakeY = (my_rand() % (g_shake * 2 + 1)) - g_shake;
            }

            BitBlt(hdc, shakeX, shakeY, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CHAR:
            if (wParam >= 32 && wParam <= 126) {
                PlayKeyClack();
                if (current_input_len < MAX_LINE_LENGTH - 1) {
                    current_input[current_input_len++] = (char)wParam;
                    current_input[current_input_len] = '\0';
                    cursor_visible = 1;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == '\b' && current_input_len > 0) {
                PlayKeyClack();
                current_input[--current_input_len] = '\0';
                cursor_visible = 1;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == '\r') {
                RECT cr; GetClientRect(hwnd, &cr);
                SpawnParticles(cr.right - 90, 85, 6, RGB(0, 255, 80), 0);
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
