import sys

with open(r'd:\KiloApps\KMaze\main.c', 'r') as f:
    content = f.read()

# Replace map definitions with const orig_maps
content = content.replace('int map1[10][10] = {', 'const int orig_map1[10][10] = {')
content = content.replace('int map2[12][12] = {', 'const int orig_map2[12][12] = {')
content = content.replace('int map3[15][15] = {', 'const int orig_map3[15][15] = {')
content = content.replace('int map4[10][10] = {', 'const int orig_map4[10][10] = {')
content = content.replace('int map5[12][12] = {', 'const int orig_map5[12][12] = {')

# Add actual maps and ResetMaps after maps
maps_decl = """
int map1[10][10];
int map2[12][12];
int map3[15][15];
int map4[10][10];
int map5[12][12];

void ResetMaps() {
    memcpy(map1, orig_map1, sizeof(map1));
    memcpy(map2, orig_map2, sizeof(map2));
    memcpy(map3, orig_map3, sizeof(map3));
    memcpy(map4, orig_map4, sizeof(map4));
    memcpy(map5, orig_map5, sizeof(map5));
}
"""
content = content.replace('int currentLevel = 0;', maps_decl + '\nint currentLevel = 0;')

# Add gameState variables
game_vars = """
int gameState = 0; // 0=start, 1=play, 2=win
DWORD startTime = 0;
DWORD endTime = 0;
float bestTime = 9999.9f;

void LoadBest() {
    FILE* f = fopen("kmaze_score.dat", "rb");
    if (f) {
        fread(&bestTime, sizeof(float), 1, f);
        fclose(f);
    }
}
void SaveBest() {
    FILE* f = fopen("kmaze_score.dat", "wb");
    if (f) {
        fwrite(&bestTime, sizeof(float), 1, f);
        fclose(f);
    }
}
"""
content = content.replace('int keysHeld = 0;', 'int keysHeld = 0;\n' + game_vars)

# Load best on init
content = content.replace('pX = 1.5f, pY = 1.5f;', 'pX = 1.5f, pY = 1.5f;\nvoid InitGame() { LoadBest(); ResetMaps(); }')

# Replace NextLevel
next_level_orig = """void NextLevel() {
    keysHeld = 0;
    currentLevel++;
    if (currentLevel > 4) currentLevel = 0;
    
    // Copy default maps back if we wanted strict reset, but this is simple version so we won't fully deep copy here for Native unless needed.
    // For simplicity, Native C will just let blocks stay erased if you die or loop around.
    
    pX = 1.5f; pY = 1.5f;
    dX = 1.0f; dY = 0.0f;
    planeX = 0.0f; planeY = 0.66f;
}"""

next_level_new = """void NextLevel() {
    keysHeld = 0;
    currentLevel++;
    if (currentLevel > 4) {
        gameState = 2;
        endTime = GetTickCount();
        float elapsed = (endTime - startTime) / 1000.0f;
        if (elapsed < bestTime) {
            bestTime = elapsed;
            SaveBest();
        }
        return;
    }
    
    pX = 1.5f; pY = 1.5f;
    dX = 1.0f; dY = 0.0f;
    planeX = 0.0f; planeY = 0.66f;
}"""
content = content.replace(next_level_orig, next_level_new)

# Add gameState check to WM_TIMER
timer_start = """        case WM_TIMER: {
            float moveSpeed = 0.1f;"""
timer_new = """        case WM_TIMER: {
            if (gameState == 0 || gameState == 2) {
                if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                    ResetMaps();
                    gameState = 1;
                    startTime = GetTickCount();
                    currentLevel = -1;
                    NextLevel();
                }
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }
            float moveSpeed = 0.1f;"""
content = content.replace(timer_start, timer_new)

# Replace UI drawing in WM_PAINT
ui_orig = """            // Draw UI
            char uiText[64];
            sprintf(uiText, "Keys: %d  Level: %d", keysHeld, currentLevel + 1);
            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(255, 255, 255));
            TextOutA(hdcMem, 10, 10, uiText, lstrlenA(uiText));"""
ui_new = """            // Draw UI
            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(255, 255, 255));
            char uiText[128];
            if (gameState == 0) {
                TextOutA(hdcMem, W/2 - 20, H/2 - 20, "KMAZE", 5);
                TextOutA(hdcMem, W/2 - 70, H/2 + 10, "Press ENTER to start", 20);
            } else if (gameState == 2) {
                float elapsed = (endTime - startTime) / 1000.0f;
                TextOutA(hdcMem, W/2 - 40, H/2 - 20, "You Escaped!", 12);
                sprintf(uiText, "Time: %.1fs", elapsed);
                TextOutA(hdcMem, W/2 - 40, H/2, uiText, lstrlenA(uiText));
                sprintf(uiText, "Best: %.1fs", bestTime);
                TextOutA(hdcMem, W/2 - 40, H/2 + 20, uiText, lstrlenA(uiText));
                TextOutA(hdcMem, W/2 - 80, H/2 + 40, "Press ENTER to play again", 25);
            } else {
                float elapsed = (GetTickCount() - startTime) / 1000.0f;
                sprintf(uiText, "Keys: %d  Level: %d  Time: %.1f", keysHeld, currentLevel + 1, elapsed);
                TextOutA(hdcMem, 10, 10, uiText, lstrlenA(uiText));
                if (bestTime < 9999.0f) {
                    sprintf(uiText, "Best: %.1f", bestTime);
                    TextOutA(hdcMem, 10, 25, uiText, lstrlenA(uiText));
                }
            }"""
content = content.replace(ui_orig, ui_new)

# Add InitGame() call to WM_CREATE
create_orig = """        case WM_CREATE:
            SetTimer(hwnd, 1, 30, NULL);
            break;"""
create_new = """        case WM_CREATE:
            InitGame();
            SetTimer(hwnd, 1, 30, NULL);
            break;"""
content = content.replace(create_orig, create_new)


with open(r'd:\KiloApps\KMaze\main.c', 'w') as f:
    f.write(content)
print("Updated main.c")
