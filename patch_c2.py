import re

c_path = r"d:\KiloApps\KTetris\main.c"

with open(c_path, "r", encoding="utf-8") as f:
    c = f.read()

# 1. Add headers
c = c.replace("#define WIN32_LEAN_AND_MEAN", "#define WIN32_LEAN_AND_MEAN\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>")

# 2. Add structs and state after line 92 (int shake_timer = 0;)
structs_and_vars = """
typedef struct { DWORD tick; char key; } ReplayEvent;
typedef struct { DWORD seed; int count; ReplayEvent events[5000]; } ReplayData;
ReplayData current_replay;
ReplayData saved_replay;
int is_replaying = 0;
int replay_index = 0;
DWORD replay_tick = 0;
int has_saved_replay = 0;

typedef struct { int up, down, left, right, drop, hold, pause, nuke, swap, freeze; } KeyBinds;
KeyBinds keys = { VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_SPACE, 'C', 'P', 'B', 'S', 'F' };
int show_keybinds = 0;
int bind_index = 0;
const char* bind_names[10] = {"UP", "DOWN", "LEFT", "RIGHT", "DROP", "HOLD", "PAUSE", "NUKE", "SWAP", "FREEZE"};
"""
c = c.replace("int shake_timer = 0;", "int shake_timer = 0;\n" + structs_and_vars)

# 3. Add functions after void SpawnPiece();
functions = """
void LoadKeys() {
    HANDLE h = CreateFileA("ktetris_keys.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { DWORD br; ReadFile(h, &keys, sizeof(KeyBinds), &br, NULL); CloseHandle(h); }
}
void SaveKeys() {
    HANDLE h = CreateFileA("ktetris_keys.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { DWORD bw; WriteFile(h, &keys, sizeof(KeyBinds), &bw, NULL); CloseHandle(h); }
}

void LoadReplay() {
    HANDLE h = CreateFileA("ktetris_replay.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { DWORD br; ReadFile(h, &saved_replay, sizeof(ReplayData), &br, NULL); CloseHandle(h); has_saved_replay = 1; }
}
void SaveReplay() {
    HANDLE h = CreateFileA("ktetris_replay.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { DWORD bw; WriteFile(h, &current_replay, sizeof(ReplayData), &bw, NULL); CloseHandle(h); has_saved_replay = 1; saved_replay = current_replay; }
}

void ExportStats() {
    char csv[512], json[512];
    float tr = (lines > 0) ? ((float)stat_lines[3] / lines * 100.0f) : 0.0f;
    wsprintfA(csv, "Score,Lines,Pieces,Time,TetrisRate,Single,Double,Triple,Tetris\\n%d,%d,%d,%d,%.1f,%d,%d,%d,%d", score, lines, pieces_placed, mode_timer_ms, tr, stat_lines[0], stat_lines[1], stat_lines[2], stat_lines[3]);
    wsprintfA(json, "{\\n  \\"score\\": %d,\\n  \\"lines\\": %d,\\n  \\"pieces\\": %d,\\n  \\"time\\": %d,\\n  \\"tetrisRate\\": %.1f,\\n  \\"singles\\": %d,\\n  \\"doubles\\": %d,\\n  \\"triples\\": %d,\\n  \\"tetrises\\": %d\\n}", score, lines, pieces_placed, mode_timer_ms, tr, stat_lines[0], stat_lines[1], stat_lines[2], stat_lines[3]);
    
    HANDLE hc = CreateFileA("ktetris_stats.csv", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hc != INVALID_HANDLE_VALUE) { DWORD bw; WriteFile(hc, csv, lstrlenA(csv), &bw, NULL); CloseHandle(hc); }
    HANDLE hj = CreateFileA("ktetris_stats.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hj != INVALID_HANDLE_VALUE) { DWORD bw; WriteFile(hj, json, lstrlenA(json), &bw, NULL); CloseHandle(hj); }
}

void ExportLeaderboardJSON() {
    char json[2048] = "[\\n";
    for(int i=0; i<num_leaderboard_entries; i++) {
        char entry[256];
        wsprintfA(entry, "  {\\"score\\":%d, \\"mode\\":%d, \\"lines\\":%d}%s\\n", leaderboard[i].score, leaderboard[i].mode, leaderboard[i].lines, (i<num_leaderboard_entries-1)?",":"");
        lstrcatA(json, entry);
    }
    lstrcatA(json, "]");
    HANDLE h = CreateFileA("ktetris_hiscores.json", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { DWORD bw; WriteFile(h, json, lstrlenA(json), &bw, NULL); CloseHandle(h); }
}

void ImportLeaderboardJSON() {
    HANDLE h = CreateFileA("ktetris_hiscores.json", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { 
        char json[2048] = {0}; DWORD br; ReadFile(h, json, 2047, &br, NULL); CloseHandle(h); 
        int new_hs = 0; num_leaderboard_entries = 0;
        char* ptr = json;
        while ((ptr = strstr(ptr, "\\"score\\":")) != NULL) {
            ptr += 8; int sc = atoi(ptr);
            if (sc > new_hs) new_hs = sc;
            leaderboard[num_leaderboard_entries].score = sc;
            ptr = strstr(ptr, "\\"mode\\":"); if(ptr) { ptr+=7; leaderboard[num_leaderboard_entries].mode = atoi(ptr); }
            ptr = strstr(ptr, "\\"lines\\":"); if(ptr) { ptr+=8; leaderboard[num_leaderboard_entries].lines = atoi(ptr); }
            num_leaderboard_entries++;
            if(num_leaderboard_entries >= 5) break;
        }
        high_score = new_hs;
    }
}
"""
c = c.replace("void SpawnPiece();", "void SpawnPiece();\n" + functions)


# Seed capture (in InitGame)
c = c.replace("rng_state = GetTickCount();", """if (!is_replaying) {
        rng_state = GetTickCount();
        current_replay.seed = rng_state;
        current_replay.count = 0;
    } else {
        rng_state = saved_replay.seed;
        replay_tick = 0;
        replay_index = 0;
    }""")

# Add to WM_CREATE
c = c.replace("case WM_CREATE:", "case WM_CREATE:\n            LoadKeys();\n            LoadReplay();")


# WM_KEYDOWN Replacements
keydown_start = """
            if (show_keybinds) {
                if (wParam == VK_ESCAPE) { show_keybinds = 0; start_screen = 1; InvalidateRect(hwnd, NULL, FALSE); return 0; }
                int* bp[10] = {&keys.up, &keys.down, &keys.left, &keys.right, &keys.drop, &keys.hold, &keys.pause, &keys.nuke, &keys.swap, &keys.freeze};
                *(bp[bind_index]) = (int)wParam;
                bind_index++;
                SaveKeys();
                if (bind_index >= 10) { show_keybinds = 0; start_screen = 1; }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
"""
c = c.replace("if (show_help) {", keydown_start + "\n            if (show_help) {")

c = c.replace("if (wParam == 'H') { show_help = 1; }", "if (wParam == 'H') { show_help = 1; }\n                if (wParam == 'K') { show_keybinds = 1; bind_index = 0; start_screen = 0; }\n                if (wParam == 'W' && has_saved_replay) { is_replaying = 1; start_screen = 0; InitGame(); }\n")
c = c.replace("if (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == 'B') {", "if (wParam == 'E') { ExportLeaderboardJSON(); }\n                if (wParam == 'I') { ImportLeaderboardJSON(); }\n                if (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == 'B') {")
c = c.replace("if (game_over && wParam == VK_RETURN) {", "if (game_over) {\n                if (wParam == 'E') { ExportStats(); }\n                if (wParam == 'S' && !is_replaying) { SaveReplay(); }\n                if (wParam == VK_RETURN) {\n                    start_screen = 1; game_over = 0;\n                    InvalidateRect(hwnd, NULL, FALSE);\n                }\n                return 0;\n            }\n            //")
c = c.replace("if (win_screen && wParam == VK_RETURN) {", "if (win_screen) {\n                if (wParam == 'E') { ExportStats(); }\n                if (wParam == 'S' && !is_replaying) { SaveReplay(); }\n                if (wParam == VK_RETURN) {\n                    start_screen = 1; win_screen = 0;\n                    InvalidateRect(hwnd, NULL, FALSE);\n                }\n                return 0;\n            }\n            //")


gameplay_keys = """
                if (is_replaying) return 0;
                
                if (wParam == keys.pause) { is_paused = !is_paused; InvalidateRect(hwnd, NULL, FALSE); break; }
                if (wParam == 'H') { show_help = 1; InvalidateRect(hwnd, NULL, FALSE); break; }
                if (wParam == 'V') { SaveGameStateToFile(); AddPopup((float)(W * CELL_SIZE / 2 - 30), (float)(H * CELL_SIZE / 2), "GAME SAVED!", RGB(0, 255, 255)); InvalidateRect(hwnd, NULL, FALSE); break; }
                if (wParam == keys.nuke) { UseRowNuke(); if(current_replay.count < 5000) { current_replay.events[current_replay.count].tick = replay_tick; current_replay.events[current_replay.count++].key = 'B'; } InvalidateRect(hwnd, NULL, FALSE); break; }
                if (wParam == keys.swap) { UsePieceSwap(); if(current_replay.count < 5000) { current_replay.events[current_replay.count].tick = replay_tick; current_replay.events[current_replay.count++].key = 'S'; } InvalidateRect(hwnd, NULL, FALSE); break; }
                if (wParam == keys.freeze) { UseGravityFreeze(); if(current_replay.count < 5000) { current_replay.events[current_replay.count].tick = replay_tick; current_replay.events[current_replay.count++].key = 'F'; } InvalidateRect(hwnd, NULL, FALSE); break; }

                if (is_paused) break;

                if (wParam == keys.left && !check_collision(current_piece, current_rot, current_x - 1, current_y)) { current_x--; if(current_replay.count < 5000) { current_replay.events[current_replay.count].tick = replay_tick; current_replay.events[current_replay.count++].key = 'L'; } }
                if (wParam == keys.right && !check_collision(current_piece, current_rot, current_x + 1, current_y)) { current_x++; if(current_replay.count < 5000) { current_replay.events[current_replay.count].tick = replay_tick; current_replay.events[current_replay.count++].key = 'R'; } }
                if (wParam == keys.down && !check_collision(current_piece, current_rot, current_x, current_y + 1)) { current_y++; if(current_replay.count < 5000) { current_replay.events[current_replay.count].tick = replay_tick; current_replay.events[current_replay.count++].key = 'D'; } }
                if (wParam == keys.up || wParam == 'Z' || wParam == 'X') {
                    int next_r = (current_rot + 1) % 4;
                    if (!check_collision(current_piece, next_r, current_x, current_y)) { current_rot = next_r; }
                    else if (!check_collision(current_piece, next_r, current_x - 1, current_y)) { current_x--; current_rot = next_r; }
                    else if (!check_collision(current_piece, next_r, current_x + 1, current_y)) { current_x++; current_rot = next_r; }
                    else if (!check_collision(current_piece, next_r, current_x - 2, current_y)) { current_x -= 2; current_rot = next_r; }
                    else if (!check_collision(current_piece, next_r, current_x + 2, current_y)) { current_x += 2; current_rot = next_r; }
                    if(current_replay.count < 5000) { current_replay.events[current_replay.count].tick = replay_tick; current_replay.events[current_replay.count++].key = 'U'; }
                }
                if (wParam == keys.hold || wParam == VK_SHIFT) {
                    if (!hold_used) {
                        if (hold_piece == -1) { hold_piece = current_piece; hold_is_bomb = current_is_bomb; SpawnPiece(); }
                        else { int temp = current_piece; int tb = current_is_bomb; current_piece = hold_piece; current_is_bomb = hold_is_bomb; hold_piece = temp; hold_is_bomb = tb; current_rot = 0; current_x = W / 2 - 2; current_y = -2; }
                        hold_used = 1; Beep(700, 30); if(current_replay.count < 5000) { current_replay.events[current_replay.count].tick = replay_tick; current_replay.events[current_replay.count++].key = 'C'; }
                    }
                }
                if (wParam == keys.drop) {
                    int start_y = current_y; int drop_dist = 0;
                    while (!check_collision(current_piece, current_rot, current_x, current_y + 1)) { current_y++; drop_dist++; }
                    score += drop_dist * 2; SpawnDropParticles(current_x, start_y, current_y, current_is_bomb ? 15 : (current_piece + 1));
                    int old_level = campaign_level; lock_piece();
                    if (!win_screen && !game_over && (game_mode != MODE_CAMPAIGN || campaign_level == old_level)) { SpawnPiece(); }
                    if(current_replay.count < 5000) { current_replay.events[current_replay.count].tick = replay_tick; current_replay.events[current_replay.count++].key = ' '; }
                }
"""

c = re.sub(r"if \(wParam == 'P'\) \{.*?if \(wParam == VK_SPACE\) \{.*?\}\n", gameplay_keys.strip() + "\n", c, flags=re.DOTALL)


# WM_TIMER playback
timer_replay_logic = """
            if (!game_over && !is_paused && !start_screen && !win_screen && !show_leaderboard && !show_help && !show_keybinds) {
                replay_tick += 20;
                if (is_replaying) {
                    while (replay_index < saved_replay.count && saved_replay.events[replay_index].tick <= replay_tick) {
                        char k = saved_replay.events[replay_index].key;
                        if (k == 'L' && !check_collision(current_piece, current_rot, current_x - 1, current_y)) current_x--;
                        if (k == 'R' && !check_collision(current_piece, current_rot, current_x + 1, current_y)) current_x++;
                        if (k == 'D' && !check_collision(current_piece, current_rot, current_x, current_y + 1)) current_y++;
                        if (k == 'U') {
                            int next_r = (current_rot + 1) % 4;
                            if (!check_collision(current_piece, next_r, current_x, current_y)) { current_rot = next_r; }
                            else if (!check_collision(current_piece, next_r, current_x - 1, current_y)) { current_x--; current_rot = next_r; }
                            else if (!check_collision(current_piece, next_r, current_x + 1, current_y)) { current_x++; current_rot = next_r; }
                        }
                        if (k == 'C' && !hold_used) {
                            if (hold_piece == -1) { hold_piece = current_piece; hold_is_bomb = current_is_bomb; SpawnPiece(); }
                            else { int temp = current_piece; int tb = current_is_bomb; current_piece = hold_piece; current_is_bomb = hold_is_bomb; hold_piece = temp; hold_is_bomb = tb; current_rot = 0; current_x = W / 2 - 2; current_y = -2; }
                            hold_used = 1; Beep(700, 30);
                        }
                        if (k == ' ') {
                            int start_y = current_y; int drop_dist = 0;
                            while (!check_collision(current_piece, current_rot, current_x, current_y + 1)) { current_y++; drop_dist++; }
                            score += drop_dist * 2; SpawnDropParticles(current_x, start_y, current_y, current_is_bomb ? 15 : (current_piece + 1));
                            int old_level = campaign_level; lock_piece();
                            if (!win_screen && !game_over && (game_mode != MODE_CAMPAIGN || campaign_level == old_level)) { SpawnPiece(); }
                        }
                        if (k == 'B') UseRowNuke();
                        if (k == 'S') UsePieceSwap();
                        if (k == 'F') UseGravityFreeze();
                        replay_index++;
                    }
                }
"""

c = c.replace("if (!game_over && !is_paused && !start_screen && !win_screen && !show_leaderboard && !show_help) {", timer_replay_logic.strip())

# Avoid missing nested local var syntax error in C
c = c.replace("int* bp[10] = {&keys.up, &keys.down, &keys.left, &keys.right, &keys.drop, &keys.hold, &keys.pause, &keys.nuke, &keys.swap, &keys.freeze};", 
"""
                int* bp[10];
                bp[0] = &keys.up; bp[1] = &keys.down; bp[2] = &keys.left; bp[3] = &keys.right; bp[4] = &keys.drop; bp[5] = &keys.hold; bp[6] = &keys.pause; bp[7] = &keys.nuke; bp[8] = &keys.swap; bp[9] = &keys.freeze;
""")

# UI Adjustments
c = c.replace("TextOutA(memDC, sideX, 15, mode_names[game_mode], lstrlenA(mode_names[game_mode]));", 
              """if(is_replaying){ char mStr[64]; wsprintfA(mStr, "%s [REPLAY]", mode_names[game_mode]); TextOutA(memDC, sideX, 15, mStr, lstrlenA(mStr)); } else { TextOutA(memDC, sideX, 15, mode_names[game_mode], lstrlenA(mode_names[game_mode])); }""")


c = c.replace("TextOutA(memDC, total_w / 2 - 120, 480, \"Press H for Help, L for Leaderboard\", 35);",
"""TextOutA(memDC, total_w / 2 - 120, 480, "Press H for Help, L for Leaderboard", 35);
                SetTextColor(memDC, RGB(0, 204, 255));
                TextOutA(memDC, 45, 325, "[K]. Configure Keybinds", 23);
                if (has_saved_replay) { SetTextColor(memDC, RGB(255, 85, 170)); TextOutA(memDC, 45, 355, "[W]. Watch Last Replay", 22); }
""")


# Show keybinds config overlay
show_keybinds_render = """else if (show_keybinds) {
                HBRUSH ov = CreateSolidBrush(RGB(10, 11, 16)); RECT ovRc = {0, 0, total_w, total_h}; FillRect(memDC, &ovRc, ov); DeleteObject(ov);
                SetTextColor(memDC, RGB(0, 255, 255)); TextOutA(memDC, total_w / 2 - 60, 40, "KEYBINDS CONFIG", 15);
                SetTextColor(memDC, RGB(255, 255, 255));
                for(int i=0; i<10; i++) {
                    char buf[64];
                    if (i == bind_index) wsprintfA(buf, "%s: [PRESS KEY]", bind_names[i]);
                    else wsprintfA(buf, "%s: SET", bind_names[i]);
                    SetTextColor(memDC, (i == bind_index) ? RGB(0,255,102) : RGB(170,170,170));
                    TextOutA(memDC, 50, 100 + i * 25, buf, lstrlenA(buf));
                }
                SetTextColor(memDC, RGB(100, 100, 120)); TextOutA(memDC, total_w / 2 - 60, 480, "Press ESC to cancel", 19);
            }"""

c = c.replace("} else if (show_leaderboard) {", show_keybinds_render + " else if (show_leaderboard) {")

c = c.replace("TextOutA(memDC, 40, H * CELL_SIZE / 2 + 10, \"PRESS ENTER\", 11);",
              "TextOutA(memDC, 40, H * CELL_SIZE / 2 + 10, \"PRESS ENTER\", 11);\n                TextOutA(memDC, 40, H * CELL_SIZE / 2 + 30, \"[E] EXPORT STATS\", 16);\n                TextOutA(memDC, 40, H * CELL_SIZE / 2 + 50, \"[S] SAVE REPLAY\", 15);")
c = c.replace("TextOutA(memDC, 25, H * CELL_SIZE / 2 + 10, \"ENTER TO MENU\", 13);",
              "TextOutA(memDC, 25, H * CELL_SIZE / 2 + 10, \"ENTER TO MENU\", 13);\n                TextOutA(memDC, 25, H * CELL_SIZE / 2 + 30, \"[E] EXPORT STATS\", 16);\n                TextOutA(memDC, 25, H * CELL_SIZE / 2 + 50, \"[S] SAVE REPLAY\", 15);")
c = c.replace("TextOutA(memDC, total_w / 2 - 80, 420, \"Press ENTER or ESC to return\", 28);",
              "TextOutA(memDC, total_w / 2 - 100, 420, \"ENTER: Menu | E: Export | I: Import\", 35);")

# Quick fix for nested function RecordInput which is illegal in standard C:
# Move RecordInput outside or replace with inline macro
c = c.replace("void RecordInput(char k) { if(current_replay.count < 5000) { current_replay.events[current_replay.count].tick = replay_tick; current_replay.events[current_replay.count].key = k; current_replay.count++; } }", "")

with open(c_path, "w", encoding="utf-8") as f:
    f.write(c)
print("C patched successfully.")
