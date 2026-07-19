import re

with open("c:/KiloApps/KiloApps/KHangman/main.c", "r") as f:
    code = f.read()

# 1. Update H to 600
code = code.replace("#define H 550", "#define H 600")

# 2. NUM_CATEGORIES 4 -> 5
code = code.replace("#define NUM_CATEGORIES 4", "#define NUM_CATEGORIES 5")

# 3. Add Custom to CAT_NAMES
code = code.replace('{"Technology", "Animals", "Countries", "Science"}', '{"Technology", "Animals", "Countries", "Science", "Custom"}')

# 4. Global Edit Control
code = code.replace("int errors = 0;", "HWND hCustomEdit = NULL;\nint errors = 0;")

# 5. InitGame updates
init_game_old = """void InitGame() {
    if (!initialized) {
        LoadStats();
        CustomSrand(GetTickCount());
        initialized = 1;
    }
    
    int w_idx = CustomRand() % NUM_WORDS_PER_CAT;
    int i = 0;
    while(CAT_WORDS[current_category][w_idx][i]) {
        target_word[i] = CAT_WORDS[current_category][w_idx][i];
        i++;
    }
    target_word[i] = '\\0';"""

init_game_new = """void InitGame() {
    if (!initialized) {
        LoadStats();
        CustomSrand(GetTickCount());
        initialized = 1;
    }
    
    if (current_category == 4) {
        char buf[1024] = {0};
        if (hCustomEdit) {
            GetWindowTextA(hCustomEdit, buf, sizeof(buf));
        } else {
            strcpy(buf, "APPLE, BANANA");
        }
        
        char words[50][32];
        int w_count = 0;
        char *p = buf;
        while (*p && w_count < 50) {
            while (*p == ' ' || *p == ',') p++;
            if (!*p) break;
            int i = 0;
            while (*p && *p != ',' && i < 31) {
                if (*p >= 'a' && *p <= 'z') words[w_count][i++] = *p - 32;
                else if (*p >= 'A' && *p <= 'Z') words[w_count][i++] = *p;
                p++;
            }
            words[w_count][i] = '\\0';
            if (i > 0) w_count++;
            while (*p && *p != ',') p++;
        }
        if (w_count == 0) {
            strcpy(target_word, "CUSTOM");
        } else {
            int w_idx = CustomRand() % w_count;
            strcpy(target_word, words[w_idx]);
        }
    } else {
        int w_idx = CustomRand() % NUM_WORDS_PER_CAT;
        int i = 0;
        while(CAT_WORDS[current_category][w_idx][i]) {
            target_word[i] = CAT_WORDS[current_category][w_idx][i];
            i++;
        }
        target_word[i] = '\\0';
    }"""
code = code.replace(init_game_old, init_game_new)

# 6. WM_CREATE create edit control
wm_create_old = """        case WM_CREATE:
            InitGame();
            SetTimer(hwnd, 1, 30, NULL);
            break;"""

wm_create_new = """        case WM_CREATE:
            hCustomEdit = CreateWindowEx(0, "EDIT", "APPLE, BANANA", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_LEFT, 100, 75, 300, 25, hwnd, (HMENU)101, GetModuleHandle(NULL), NULL);
            if (current_category == 4) ShowWindow(hCustomEdit, SW_SHOW);
            InitGame();
            SetTimer(hwnd, 1, 30, NULL);
            break;"""
code = code.replace(wm_create_old, wm_create_new)

# 7. Categories clicks (105->95, cx=40->15)
cat_click_old = """            // Check categories
            int cx = 40;
            int cy = 45;
            for (int i = 0; i < NUM_CATEGORIES; i++) {
                if (x >= cx && x <= cx + 90 && y >= cy && y <= cy + 25) {
                    if (current_category != i) {
                        current_category = i;
                        InitGame();
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                    break;
                }
                cx += 105;
            }"""

cat_click_new = """            // Check categories
            int cx = 15;
            int cy = 45;
            for (int i = 0; i < NUM_CATEGORIES; i++) {
                if (x >= cx && x <= cx + 85 && y >= cy && y <= cy + 25) {
                    if (current_category != i) {
                        current_category = i;
                        if (current_category == 4) ShowWindow(hCustomEdit, SW_SHOW);
                        else ShowWindow(hCustomEdit, SW_HIDE);
                        SetFocus(hwnd);
                        InitGame();
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                    break;
                }
                cx += 95;
            }"""
code = code.replace(cat_click_old, cat_click_new)

# 8. Shift clicks by 40
code = code.replace("x >= 30 && x <= 100 && y >= 450 && y <= 490", "x >= 30 && x <= 100 && y >= 490 && y <= 530")
code = code.replace("x >= 110 && x <= 210 && y >= 450 && y <= 490", "x >= 110 && x <= 210 && y >= 490 && y <= 530")
code = code.replace("x >= 220 && x <= 290 && y >= 450 && y <= 490", "x >= 220 && x <= 290 && y >= 490 && y <= 530")
code = code.replace("x >= 300 && x <= 370 && y >= 450 && y <= 490", "x >= 300 && x <= 370 && y >= 490 && y <= 530")
code = code.replace("x >= 380 && x <= 460 && y >= 450 && y <= 490", "x >= 380 && x <= 460 && y >= 490 && y <= 530")

# For new game, ensure focus is restored
restart_old = """            // Check restart button
            if (x >= 110 && x <= 210 && y >= 490 && y <= 530) {
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }"""
restart_new = """            // Check restart button
            if (x >= 110 && x <= 210 && y >= 490 && y <= 530) {
                SetFocus(hwnd);
                InitGame();
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }"""
code = code.replace(restart_old, restart_new)

# Shift keyboard clicks
code = code.replace("int ky = 270;", "int ky = 330;")

# 9. Categories drawing
cat_draw_old = """            // Categories
            int cx = 40;
            int cy = 45;
            SelectObject(memDC, hFontMono);
            for (int i = 0; i < NUM_CATEGORIES; i++) {
                RECT cRect = {cx, cy, cx + 90, cy + 25};
                HBRUSH cBg = CreateSolidBrush(i == current_category ? RGB(0, 122, 204) : RGB(44, 44, 44));
                FillRect(memDC, &cRect, cBg);
                DeleteObject(cBg);
                SetTextColor(memDC, RGB(255, 255, 255));
                DrawTextA(memDC, CAT_NAMES[i], -1, &cRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                cx += 105;
            }"""

cat_draw_new = """            // Categories
            int cx = 15;
            int cy = 45;
            SelectObject(memDC, hFontMono);
            for (int i = 0; i < NUM_CATEGORIES; i++) {
                RECT cRect = {cx, cy, cx + 85, cy + 25};
                HBRUSH cBg = CreateSolidBrush(i == current_category ? RGB(0, 122, 204) : RGB(44, 44, 44));
                FillRect(memDC, &cRect, cBg);
                DeleteObject(cBg);
                SetTextColor(memDC, RGB(255, 255, 255));
                DrawTextA(memDC, CAT_NAMES[i], -1, &cRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                cx += 95;
            }"""
code = code.replace(cat_draw_old, cat_draw_new)

# 10. Draw shifts
code = code.replace("MoveToEx(memDC, 190 + ox, 200, NULL);", "MoveToEx(memDC, 190 + ox, 240, NULL);")
code = code.replace("LineTo(memDC, 310 + ox, 200); // Base", "LineTo(memDC, 310 + ox, 240); // Base")
code = code.replace("MoveToEx(memDC, 210 + ox, 200, NULL);", "MoveToEx(memDC, 210 + ox, 240, NULL);")
code = code.replace("LineTo(memDC, 210 + ox, 60);  // Pole", "LineTo(memDC, 210 + ox, 100);  // Pole")
code = code.replace("LineTo(memDC, 270 + ox, 60);  // Top", "LineTo(memDC, 270 + ox, 100);  // Top")
code = code.replace("LineTo(memDC, 270 + ox, 80);  // Rope", "LineTo(memDC, 270 + ox, 120);  // Rope")

code = code.replace("Ellipse(memDC, 250 + ox, 80, 290 + ox, 120);", "Ellipse(memDC, 250 + ox, 120, 290 + ox, 160);")

code = code.replace("MoveToEx(memDC, 270 + ox, 120, NULL);", "MoveToEx(memDC, 270 + ox, 160, NULL);")
code = code.replace("LineTo(memDC, 270 + ox, 160);", "LineTo(memDC, 270 + ox, 200);")

code = code.replace("MoveToEx(memDC, 270 + ox, 130, NULL);", "MoveToEx(memDC, 270 + ox, 170, NULL);")
code = code.replace("LineTo(memDC, 240 + ox, 150);", "LineTo(memDC, 240 + ox, 190);")
code = code.replace("LineTo(memDC, 300 + ox, 150);", "LineTo(memDC, 300 + ox, 190);")

code = code.replace("MoveToEx(memDC, 270 + ox, 160, NULL);", "MoveToEx(memDC, 270 + ox, 200, NULL);")
code = code.replace("LineTo(memDC, 240 + ox, 190);", "LineTo(memDC, 240 + ox, 230);")
code = code.replace("LineTo(memDC, 300 + ox, 190);", "LineTo(memDC, 300 + ox, 230);")

# 11. Word display, message, keyboard, buttons
code = code.replace("TextOutA(memDC, (W - tSize.cx) / 2, 200, disp, len);", "TextOutA(memDC, (W - tSize.cx) / 2, 250, disp, len);")
code = code.replace("TextOutA(memDC, (W - tSize.cx) / 2, 240, msgTxt, lstrlenA(msgTxt));", "TextOutA(memDC, (W - tSize.cx) / 2, 290, msgTxt, lstrlenA(msgTxt));")
code = code.replace("int ky = 280;", "int ky = 330;")

code = code.replace("RECT hintRect = {30, 450, 100, 490};", "RECT hintRect = {30, 490, 100, 530};")
code = code.replace("RECT resRect = {110, 450, 210, 490};", "RECT resRect = {110, 490, 210, 530};")
code = code.replace("RECT saveRect = {220, 450, 290, 490};", "RECT saveRect = {220, 490, 290, 530};")
code = code.replace("RECT loadRect = {300, 450, 370, 490};", "RECT loadRect = {300, 490, 370, 530};")
code = code.replace("RECT muteRect = {380, 450, 460, 490};", "RECT muteRect = {380, 490, 460, 530};")

code = code.replace("TextOutA(memDC, (W - tSize.cx) / 2, 510, statText, lstrlenA(statText));", "TextOutA(memDC, (W - tSize.cx) / 2, 550, statText, lstrlenA(statText));")

with open("c:/KiloApps/KiloApps/KHangman/main.c", "w") as f:
    f.write(code)

print("Updates applied.")
