#include <windows.h>

#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER 0x1501
#endif


void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

int my_strlen(const char* s) { int l=0; while(s && *s++) l++; return l; }
void my_strcpy(char* d, const char* s) { while(*s) *d++ = *s++; *d = 0; }
void my_strcat(char* d, const char* s) { while(*d) d++; while(*s) *d++ = *s++; *d = 0; }
int my_atoi(const char* str) {
    int res = 0;
    while(str && *str >= '0' && *str <= '9') { res = res * 10 + (*str - '0'); str++; }
    return res;
}
void my_itoa(int val, char* buf) {
    if(val == 0) { buf[0] = '0'; buf[1] = 0; return; }
    char tmp[16]; int i = 0;
    while(val > 0) { tmp[i++] = (val % 10) + '0'; val /= 10; }
    int j = 0;
    while(i > 0) { buf[j++] = tmp[--i]; }
    buf[j] = 0;
}
char to_lower_char(char c) {
    if(c >= 'A' && c <= 'Z') return c + 32;
    return c;
}
int my_strstr_ic(const char* haystack, const char* needle) {
    if(!needle || !*needle) return 1;
    int hLen = my_strlen(haystack);
    int nLen = my_strlen(needle);
    if(nLen > hLen) return 0;
    for(int i = 0; i <= hLen - nLen; i++) {
        int match = 1;
        for(int j = 0; j < nLen; j++) {
            if(to_lower_char(haystack[i+j]) != to_lower_char(needle[j])) {
                match = 0;
                break;
            }
        }
        if(match) return 1;
    }
    return 0;
}

typedef struct {
    char label[64];
    char pass[64];
    char strength[20];
} VaultEntry;

VaultEntry g_vault[100];
int g_vaultCount = 0;

HWND hDisplay, hStrengthDisplay, hBtnGen, hBtnCopy, hUpper, hLower, hNum, hSym, hLen;
HWND hLabelInput, hBtnSave, hVaultSearch, hVaultList, hBtnCopyVault, hBtnDelVault;
HFONT hFont, hBtnFont, hSmallFont;
HBRUSH hBgBrush;

void SaveVaultToFile() {
    HANDLE hFile = CreateFileA("kpass_vault.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile != INVALID_HANDLE_VALUE) {
        for(int i = 0; i < g_vaultCount; i++) {
            char line[200];
            my_strcpy(line, g_vault[i].label);
            my_strcat(line, "|");
            my_strcat(line, g_vault[i].pass);
            my_strcat(line, "|");
            my_strcat(line, g_vault[i].strength);
            my_strcat(line, "\r\n");
            DWORD written;
            WriteFile(hFile, line, my_strlen(line), &written, NULL);
        }
        CloseHandle(hFile);
    }
}

void LoadVaultFromFile() {
    g_vaultCount = 0;
    HANDLE hFile = CreateFileA("kpass_vault.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile != INVALID_HANDLE_VALUE) {
        char buf[8192] = {0};
        DWORD bytesRead = 0;
        ReadFile(hFile, buf, sizeof(buf)-1, &bytesRead, NULL);
        CloseHandle(hFile);

        char* p = buf;
        while(*p && g_vaultCount < 100) {
            char line[200] = {0};
            int len = 0;
            while(*p && *p != '\r' && *p != '\n' && len < 199) {
                line[len++] = *p++;
            }
            while(*p == '\r' || *p == '\n') p++;
            if(len > 0) {
                char* sep1 = NULL;
                char* sep2 = NULL;
                for(int i = 0; i < len; i++) {
                    if(line[i] == '|') {
                        if(!sep1) sep1 = line + i;
                        else if(!sep2) sep2 = line + i;
                    }
                }
                if(sep1) {
                    *sep1 = 0;
                    my_strcpy(g_vault[g_vaultCount].label, line);
                    if(sep2) {
                        *sep2 = 0;
                        my_strcpy(g_vault[g_vaultCount].pass, sep1 + 1);
                        my_strcpy(g_vault[g_vaultCount].strength, sep2 + 1);
                    } else {
                        my_strcpy(g_vault[g_vaultCount].pass, sep1 + 1);
                        my_strcpy(g_vault[g_vaultCount].strength, "Saved");
                    }
                    g_vaultCount++;
                }
            }
        }
    }
}

void RefreshVaultList() {
    SendMessage(hVaultList, LB_RESETCONTENT, 0, 0);
    char query[64] = {0};
    GetWindowTextA(hVaultSearch, query, sizeof(query));

    for(int i = 0; i < g_vaultCount; i++) {
        if(query[0] == 0 || my_strstr_ic(g_vault[i].label, query) || my_strstr_ic(g_vault[i].pass, query)) {
            char displayLine[160];
            my_strcpy(displayLine, "[");
            my_strcat(displayLine, g_vault[i].label);
            my_strcat(displayLine, "] ");
            my_strcat(displayLine, g_vault[i].pass);
            my_strcat(displayLine, " (");
            my_strcat(displayLine, g_vault[i].strength);
            my_strcat(displayLine, ")");
            
            int index = SendMessageA(hVaultList, LB_ADDSTRING, 0, (LPARAM)displayLine);
            SendMessageA(hVaultList, LB_SETITEMDATA, index, (LPARAM)i);
        }
    }
}

void CopyToClipboard(HWND hwnd, const char* text) {
    if(OpenClipboard(hwnd)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, my_strlen(text) + 1);
        if(hMem) {
            char* ptr = (char*)GlobalLock(hMem);
            my_strcpy(ptr, text);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
        }
        CloseClipboard();
    }
}

void CalculateStrength(const char* pwd, char* outStr, char* outRating) {
    if(!pwd || !*pwd || my_strstr_ic(pwd, "Click Generate")) {
        my_strcpy(outStr, "Strength: - (0 bits)");
        my_strcpy(outRating, "-");
        return;
    }
    int hasUpper = 0, hasLower = 0, hasNum = 0, hasSym = 0;
    int len = my_strlen(pwd);
    for(int i = 0; i < len; i++) {
        if(pwd[i] >= 'A' && pwd[i] <= 'Z') hasUpper = 1;
        else if(pwd[i] >= 'a' && pwd[i] <= 'z') hasLower = 1;
        else if(pwd[i] >= '0' && pwd[i] <= '9') hasNum = 1;
        else hasSym = 1;
    }
    int pool = 0;
    if(hasUpper) pool += 26;
    if(hasLower) pool += 26;
    if(hasNum) pool += 10;
    if(hasSym) pool += 12;
    if(pool == 0) pool = 26;

    int bitsPerCharX100 = 470;
    if(pool >= 74) bitsPerCharX100 = 621;
    else if(pool >= 62) bitsPerCharX100 = 595;
    else if(pool >= 52) bitsPerCharX100 = 570;
    else if(pool >= 26) bitsPerCharX100 = 470;
    else if(pool >= 12) bitsPerCharX100 = 358;
    else if(pool >= 10) bitsPerCharX100 = 332;

    int entropy = (len * bitsPerCharX100) / 100;
    const char* rating = "Weak";
    if(entropy >= 80) rating = "Very Strong";
    else if(entropy >= 60) rating = "Strong";
    else if(entropy >= 40) rating = "Fair";

    my_strcpy(outRating, rating);

    char entStr[16];
    my_itoa(entropy, entStr);

    my_strcpy(outStr, "Strength: ");
    my_strcat(outStr, rating);
    my_strcat(outStr, " (");
    my_strcat(outStr, entStr);
    my_strcat(outStr, " bits)");
}

void GeneratePassword() {
    char pool[200] = {0};
    int pLen = 0;
    if (SendMessage(hUpper, BM_GETCHECK, 0, 0)) { my_strcpy(pool + pLen, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"); pLen += 26; }
    if (SendMessage(hLower, BM_GETCHECK, 0, 0)) { my_strcpy(pool + pLen, "abcdefghijklmnopqrstuvwxyz"); pLen += 26; }
    if (SendMessage(hNum, BM_GETCHECK, 0, 0)) { my_strcpy(pool + pLen, "0123456789"); pLen += 10; }
    if (SendMessage(hSym, BM_GETCHECK, 0, 0)) { my_strcpy(pool + pLen, "!@#$%^&*()_+"); pLen += 12; }
    if (pLen == 0) { my_strcpy(pool, "abcdefghijklmnopqrstuvwxyz"); pLen = 26; SendMessage(hLower, BM_SETCHECK, BST_CHECKED, 0); }
    
    char lenStr[10];
    GetWindowTextA(hLen, lenStr, 10);
    int len = my_atoi(lenStr);
    if (len < 8) len = 8;
    if (len > 64) len = 64;
    
    char pwd[65] = {0};
    for(int i = 0; i < len; i++) {
        pwd[i] = pool[GetTickCount() % (pLen + i) % pLen];
        Sleep(1);
    }
    SetWindowTextA(hDisplay, pwd);

    char strDisplay[64];
    char strRating[20];
    CalculateStrength(pwd, strDisplay, strRating);
    SetWindowTextA(hStrengthDisplay, strDisplay);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Click Generate...", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, 20, 15, 420, 32, hwnd, NULL, NULL, NULL);
            hStrengthDisplay = CreateWindowA("STATIC", "Strength: - (0 bits)", WS_CHILD | WS_VISIBLE | SS_CENTER, 20, 52, 420, 20, hwnd, NULL, NULL, NULL);

            hUpper = CreateWindowA("BUTTON", "Uppercase", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, 78, 100, 20, hwnd, NULL, NULL, NULL);
            hLower = CreateWindowA("BUTTON", "Lowercase", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 125, 78, 100, 20, hwnd, NULL, NULL, NULL);
            hNum = CreateWindowA("BUTTON", "Numbers", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 230, 78, 90, 20, hwnd, NULL, NULL, NULL);
            hSym = CreateWindowA("BUTTON", "Symbols", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 330, 78, 90, 20, hwnd, NULL, NULL, NULL);

            CreateWindowA("STATIC", "Length:", WS_CHILD | WS_VISIBLE, 20, 104, 55, 20, hwnd, NULL, NULL, NULL);
            hLen = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "16", WS_CHILD | WS_VISIBLE | ES_NUMBER, 75, 102, 45, 22, hwnd, NULL, NULL, NULL);
            
            SendMessage(hUpper, BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(hLower, BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(hNum, BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(hSym, BM_SETCHECK, BST_CHECKED, 0);
            
            hBtnGen = CreateWindowA("BUTTON", "Generate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 130, 102, 140, 24, hwnd, (HMENU)1001, NULL, NULL);
            hBtnCopy = CreateWindowA("BUTTON", "Copy Password", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 280, 102, 160, 24, hwnd, (HMENU)1002, NULL, NULL);

            CreateWindowA("STATIC", "--- Vault & Security Manager ---", WS_CHILD | WS_VISIBLE | SS_CENTER, 20, 135, 420, 18, hwnd, NULL, NULL, NULL);

            hLabelInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 20, 155, 240, 24, hwnd, NULL, NULL, NULL);
            SendMessageA(hLabelInput, EM_SETCUEBANNER, FALSE, (LPARAM)L"Label (e.g. Work Email)");
            hBtnSave = CreateWindowA("BUTTON", "Save to Vault", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 270, 155, 170, 24, hwnd, (HMENU)1003, NULL, NULL);

            hVaultSearch = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 20, 185, 240, 24, hwnd, (HMENU)2001, NULL, NULL);
            SendMessageA(hVaultSearch, EM_SETCUEBANNER, FALSE, (LPARAM)L"Search Vault...");

            hBtnCopyVault = CreateWindowA("BUTTON", "Copy Item", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 270, 185, 80, 24, hwnd, (HMENU)1004, NULL, NULL);
            hBtnDelVault = CreateWindowA("BUTTON", "Delete Item", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 355, 185, 85, 24, hwnd, (HMENU)1005, NULL, NULL);

            hVaultList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 20, 215, 420, 180, hwnd, (HMENU)2002, NULL, NULL);
            
            hBgBrush = CreateSolidBrush(RGB(20, 20, 20));
            hFont = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnFont = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            SendMessageA(hBtnGen, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessageA(hBtnCopy, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessageA(hBtnSave, WM_SETFONT, (WPARAM)hBtnFont, TRUE);

            hSmallFont = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            SendMessageA(hVaultList, WM_SETFONT, (WPARAM)hSmallFont, TRUE);
            SendMessageA(hStrengthDisplay, WM_SETFONT, (WPARAM)hBtnFont, TRUE);

            LoadVaultFromFile();
            RefreshVaultList();
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == 1001) {
                GeneratePassword();
            } else if (wmId == 1002) {
                char pwd[65];
                GetWindowTextA(hDisplay, pwd, 65);
                CopyToClipboard(hwnd, pwd);
            } else if (wmId == 1003) {
                char label[64] = {0};
                char pass[64] = {0};
                GetWindowTextA(hLabelInput, label, 64);
                GetWindowTextA(hDisplay, pass, 64);

                if(label[0] != 0 && pass[0] != 0 && !my_strstr_ic(pass, "Click Generate")) {
                    if(g_vaultCount < 100) {
                        my_strcpy(g_vault[g_vaultCount].label, label);
                        my_strcpy(g_vault[g_vaultCount].pass, pass);
                        char dummy[64], strRating[20];
                        CalculateStrength(pass, dummy, strRating);
                        my_strcpy(g_vault[g_vaultCount].strength, strRating);
                        g_vaultCount++;

                        SaveVaultToFile();
                        RefreshVaultList();
                        SetWindowTextA(hLabelInput, "");
                        MessageBoxA(hwnd, "Entry saved to vault!", "KPass Vault", MB_OK | MB_ICONINFORMATION);
                    } else {
                        MessageBoxA(hwnd, "Vault is full (max 100 entries).", "KPass Vault", MB_OK | MB_ICONWARNING);
                    }
                } else {
                    MessageBoxA(hwnd, "Please enter a label and generate a password first.", "KPass Vault", MB_OK | MB_ICONWARNING);
                }
            } else if (wmId == 1004) {
                int sel = SendMessageA(hVaultList, LB_GETCURSEL, 0, 0);
                if(sel != LB_ERR) {
                    int realIdx = SendMessageA(hVaultList, LB_GETITEMDATA, sel, 0);
                    if(realIdx >= 0 && realIdx < g_vaultCount) {
                        CopyToClipboard(hwnd, g_vault[realIdx].pass);
                        MessageBoxA(hwnd, "Password copied to clipboard!", "KPass Vault", MB_OK | MB_ICONINFORMATION);
                    }
                } else {
                    MessageBoxA(hwnd, "Please select an entry in the vault list.", "KPass Vault", MB_OK | MB_ICONWARNING);
                }
            } else if (wmId == 1005) {
                int sel = SendMessageA(hVaultList, LB_GETCURSEL, 0, 0);
                if(sel != LB_ERR) {
                    int realIdx = SendMessageA(hVaultList, LB_GETITEMDATA, sel, 0);
                    if(realIdx >= 0 && realIdx < g_vaultCount) {
                        for(int i = realIdx; i < g_vaultCount - 1; i++) {
                            g_vault[i] = g_vault[i+1];
                        }
                        g_vaultCount--;
                        SaveVaultToFile();
                        RefreshVaultList();
                    }
                } else {
                    MessageBoxA(hwnd, "Please select an entry to delete.", "KPass Vault", MB_OK | MB_ICONWARNING);
                }
            } else if (wmId == 2001 && wmEvent == EN_CHANGE) {
                RefreshVaultList();
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(20, 20, 20));
            SetTextColor(hdc, RGB(231, 76, 60));
            return (LRESULT)hBgBrush;
        }
        case WM_DESTROY:
            if (hBgBrush) DeleteObject(hBgBrush);
            if (hFont) DeleteObject(hFont);
            if (hBtnFont) DeleteObject(hBtnFont);
            if (hSmallFont) DeleteObject(hSmallFont);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KPassClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KPassClass", "KPass Security & Vault Manager", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 475, 450, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
