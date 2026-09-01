#include <windows.h>
#include <wincrypt.h>
#include <commdlg.h>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comdlg32.lib")

#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER 0x1501
#endif

void* __cdecl memset(void* p, int c, size_t sz) { char* pb = (char*)p; while (sz--) *pb++ = (char)c; return p; }
void* __cdecl memcpy(void* d, const void* s, size_t sz) { char* pd = (char*)d; char* ps = (char*)s; while (sz--) *pd++ = *ps++; return d; }
#pragma function(memset)
#pragma function(memcpy)

int my_strlen(const char* s) { int l=0; while(s && *s++) l++; return l; }
void my_strcpy(char* d, const char* s) { while(*s) *d++ = *s++; *d = 0; }
void my_strcat(char* d, const char* s) { while(*d) d++; while(*s) *d++ = *s++; *d = 0; }
void my_strncpy(char* d, const char* s, int max_len) {
    int i = 0;
    while(s && s[i] && i < max_len - 1) { d[i] = s[i]; i++; }
    d[i] = 0;
}
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
char to_lower_char(char c) { if(c >= 'A' && c <= 'Z') return c + 32; return c; }
int my_strstr_ic(const char* haystack, const char* needle) {
    if(!needle || !*needle) return 1;
    int hLen = my_strlen(haystack);
    int nLen = my_strlen(needle);
    if(nLen > hLen) return 0;
    for(int i = 0; i <= hLen - nLen; i++) {
        int match = 1;
        for(int j = 0; j < nLen; j++) {
            if(to_lower_char(haystack[i+j]) != to_lower_char(needle[j])) { match = 0; break; }
        }
        if(match) return 1;
    }
    return 0;
}

typedef struct {
    char label[64];
    char category[32];
    char pass[64];
    char strength[20];
} VaultEntry;

VaultEntry g_vault[200];
int g_vaultCount = 0;
char g_masterPass[128] = {0};
int g_locked = 1;

HWND hDisplay, hStrengthDisplay, hBtnGen, hBtnCopy, hUpper, hLower, hNum, hSym, hLen, hLenLabel;
HWND hHelpLabel, hBtnHelp;
HWND hLabelInput, hCatInput, hBtnSave, hVaultSearch, hFilterCat, hVaultList;
HWND hBtnCopyVault, hBtnDelVault, hBtnExpCSV, hBtnExpJSON, hBtnImp, hBtnLockMain;
HWND hLockInput, hBtnUnlock, hLockLabel, hLockHelpLabel;
HFONT hFont, hBtnFont, hSmallFont;
HBRUSH hBgBrush, hEditBrush;

// AES Encryption functions
int EncryptData(const char* password, const char* plainText, int plainLen, char* cipherBuffer, int* cipherMaxLen) {
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;
    int res = 0;
    if(CryptAcquireContext(&hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        if(CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
            if(CryptHashData(hHash, (BYTE*)password, my_strlen(password), 0)) {
                if(CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey)) {
                    DWORD len = plainLen;
                    memcpy(cipherBuffer, plainText, plainLen);
                    if(CryptEncrypt(hKey, 0, TRUE, 0, (BYTE*)cipherBuffer, &len, *cipherMaxLen)) {
                        *cipherMaxLen = len;
                        res = 1;
                    }
                    CryptDestroyKey(hKey);
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }
    return res;
}

int DecryptData(const char* password, char* cipherData, int cipherLen, char* plainBuffer, int* plainLen) {
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;
    int res = 0;
    if(CryptAcquireContext(&hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        if(CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
            if(CryptHashData(hHash, (BYTE*)password, my_strlen(password), 0)) {
                if(CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey)) {
                    DWORD len = cipherLen;
                    memcpy(plainBuffer, cipherData, cipherLen);
                    if(CryptDecrypt(hKey, 0, TRUE, 0, (BYTE*)plainBuffer, &len)) {
                        plainBuffer[len] = 0;
                        *plainLen = len;
                        res = 1;
                    }
                    CryptDestroyKey(hKey);
                }
            }
            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }
    return res;
}

void FormatVaultToCSV(char* buffer) {
    my_strcpy(buffer, "label,category,pass,strength\n");
    for(int i = 0; i < g_vaultCount; i++) {
        char line[512];
        wsprintfA(line, "\"%s\",\"%s\",\"%s\",\"%s\"\n", g_vault[i].label, g_vault[i].category, g_vault[i].pass, g_vault[i].strength);
        my_strcat(buffer, line);
    }
}

void ParseCSVToVault(char* text) {
    char* p = text;
    // Skip first line (header)
    while(*p && *p != '\n') p++;
    if(*p == '\n') p++;

    while(*p && g_vaultCount < 200) {
        char line[256] = {0};
        int len = 0;
        while(*p && *p != '\r' && *p != '\n' && len < 255) { line[len++] = *p++; }
        while(*p == '\r' || *p == '\n') p++;
        if(len > 0) {
            char* ptr = line;
            char* fields[4] = {0};
            int fIdx = 0;
            while(*ptr && fIdx < 4) {
                if(*ptr == '"') {
                    ptr++;
                    fields[fIdx++] = ptr;
                    while(*ptr && *ptr != '"') ptr++;
                    if(*ptr == '"') *ptr++ = 0;
                    if(*ptr == ',') ptr++;
                } else {
                    fields[fIdx++] = ptr;
                    while(*ptr && *ptr != ',') ptr++;
                    if(*ptr == ',') *ptr++ = 0;
                }
            }
            if(fIdx >= 3) {
                my_strncpy(g_vault[g_vaultCount].label, fields[0] ? fields[0] : "", sizeof(g_vault[g_vaultCount].label));
                my_strncpy(g_vault[g_vaultCount].category, fields[1] ? fields[1] : "", sizeof(g_vault[g_vaultCount].category));
                my_strncpy(g_vault[g_vaultCount].pass, fields[2] ? fields[2] : "", sizeof(g_vault[g_vaultCount].pass));
                my_strncpy(g_vault[g_vaultCount].strength, (fIdx >= 4 && fields[3]) ? fields[3] : "", sizeof(g_vault[g_vaultCount].strength));
                g_vaultCount++;
            }
        }
    }
}

void SaveVaultToFile() {
    if(!g_masterPass[0]) return;
    char plainBuffer[40000] = {0};
    FormatVaultToCSV(plainBuffer);
    
    int plainLen = my_strlen(plainBuffer);
    char cipherBuffer[40000] = {0};
    int cipherLen = sizeof(cipherBuffer);
    
    if(EncryptData(g_masterPass, plainBuffer, plainLen, cipherBuffer, &cipherLen)) {
        HANDLE hFile = CreateFileA("kpass_vault.enc", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(hFile, cipherBuffer, cipherLen, &written, NULL);
            CloseHandle(hFile);
        }
    }
}

int LoadVaultFromFile(const char* password) {
    HANDLE hFile = CreateFileA("kpass_vault.enc", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile != INVALID_HANDLE_VALUE) {
        char cipherBuffer[40000] = {0};
        DWORD bytesRead = 0;
        ReadFile(hFile, cipherBuffer, sizeof(cipherBuffer), &bytesRead, NULL);
        CloseHandle(hFile);

        if(bytesRead > 0) {
            char plainBuffer[40000] = {0};
            int plainLen = 0;
            if(DecryptData(password, cipherBuffer, bytesRead, plainBuffer, &plainLen)) {
                g_vaultCount = 0;
                ParseCSVToVault(plainBuffer);
                return 1;
            } else {
                return 0; // Decrypt failed
            }
        }
    }
    return -1; // File not found
}

void RefreshVaultList() {
    SendMessage(hVaultList, LB_RESETCONTENT, 0, 0);
    char query[64] = {0};
    GetWindowTextA(hVaultSearch, query, sizeof(query));
    
    char filterCat[32] = {0};
    GetWindowTextA(hFilterCat, filterCat, sizeof(filterCat));

    for(int i = 0; i < g_vaultCount; i++) {
        int matchQ = (query[0] == 0 || my_strstr_ic(g_vault[i].label, query) || my_strstr_ic(g_vault[i].pass, query));
        int matchC = (filterCat[0] == 0 || my_strstr_ic(filterCat, "All Cats") || my_strstr_ic(g_vault[i].category, filterCat));
        if(matchQ && matchC) {
            char displayLine[256];
            wsprintfA(displayLine, "[%s] {%s} %s (%s)", g_vault[i].label, g_vault[i].category[0] ? g_vault[i].category : "Other", g_vault[i].pass, g_vault[i].strength);
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

    int entropy = (len * (pool > 70 ? 621 : (pool > 50 ? 570 : (pool > 25 ? 470 : 332)))) / 100;
    const char* rating = "Weak";
    if(entropy >= 80) rating = "Very Strong";
    else if(entropy >= 60) rating = "Strong";
    else if(entropy >= 40) rating = "Fair";

    my_strcpy(outRating, rating);
    wsprintfA(outStr, "Strength: %s (%d bits)", rating, entropy);
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

void LockUI(int lock) {
    g_locked = lock;
    int showMain = lock ? SW_HIDE : SW_SHOW;
    ShowWindow(hHelpLabel, showMain);
    ShowWindow(hBtnHelp, showMain);
    ShowWindow(hDisplay, showMain);
    ShowWindow(hStrengthDisplay, showMain);
    ShowWindow(hBtnGen, showMain);
    ShowWindow(hBtnCopy, showMain);
    ShowWindow(hUpper, showMain);
    ShowWindow(hLower, showMain);
    ShowWindow(hNum, showMain);
    ShowWindow(hSym, showMain);
    ShowWindow(hLenLabel, showMain);
    ShowWindow(hLen, showMain);
    ShowWindow(hLabelInput, showMain);
    ShowWindow(hCatInput, showMain);
    ShowWindow(hBtnSave, showMain);
    ShowWindow(hVaultSearch, showMain);
    ShowWindow(hFilterCat, showMain);
    ShowWindow(hVaultList, showMain);
    ShowWindow(hBtnCopyVault, showMain);
    ShowWindow(hBtnDelVault, showMain);
    ShowWindow(hBtnExpCSV, showMain);
    ShowWindow(hBtnExpJSON, showMain);
    ShowWindow(hBtnImp, showMain);
    ShowWindow(hBtnLockMain, showMain);

    int showLock = lock ? SW_SHOW : SW_HIDE;
    ShowWindow(hLockLabel, showLock);
    ShowWindow(hLockInput, showLock);
    ShowWindow(hBtnUnlock, showLock);
    ShowWindow(hLockHelpLabel, showLock);

    if (lock) {
        SetFocus(hLockInput);
    } else {
        SetFocus(hBtnGen);
    }
}

void ShowHelpModal(HWND hwnd) {
    MessageBoxA(hwnd,
        "KPass Security & Vault Manager\n\n"
        "Features:\n"
        "  - Generator: Customize password length (8-64) and character sets.\n"
        "  - Vault: Encrypted credentials store using AES-256.\n"
        "  - Search: Real-time search by label or category.\n"
        "  - Auto-Lock: Automatically locks after 1 minute of inactivity.\n"
        "  - Export / Import: Backup credentials to CSV or JSON.\n\n"
        "Keyboard Shortcuts:\n"
        "  - F1 or 'H': Display this Help dialog\n"
        "  - Enter: Unlock vault (on lock screen) / Save / Generate\n"
        "  - Double-Click list entry: Copy password immediately",
        "KPass Help", MB_OK | MB_ICONINFORMATION);
}

void ExportFile(HWND hwnd, int isJSON) {
    OPENFILENAMEA ofn = {0};
    char filename[260] = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrFilter = isJSON ? "JSON Files\0*.json\0All\0*.*\0" : "CSV Files\0*.csv\0All\0*.*\0";
    ofn.lpstrDefExt = isJSON ? "json" : "csv";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if(GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile != INVALID_HANDLE_VALUE) {
            DWORD w;
            if(!isJSON) {
                char buf[40000];
                FormatVaultToCSV(buf);
                WriteFile(hFile, buf, my_strlen(buf), &w, NULL);
            } else {
                WriteFile(hFile, "[\r\n", 3, &w, NULL);
                for(int i=0; i<g_vaultCount; i++) {
                    char buf[512];
                    wsprintfA(buf, "  {\"label\":\"%s\", \"category\":\"%s\", \"pass\":\"%s\", \"strength\":\"%s\"}%s\r\n", 
                            g_vault[i].label, g_vault[i].category, g_vault[i].pass, g_vault[i].strength, (i == g_vaultCount - 1) ? "" : ",");
                    WriteFile(hFile, buf, my_strlen(buf), &w, NULL);
                }
                WriteFile(hFile, "]\r\n", 3, &w, NULL);
            }
            CloseHandle(hFile);
            MessageBoxA(hwnd, "Export successful!", "Export", MB_OK);
        }
    }
}

void ImportFile(HWND hwnd) {
    OPENFILENAMEA ofn = {0};
    char filename[260] = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrFilter = "CSV Files\0*.csv\0All\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if(GetOpenFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile != INVALID_HANDLE_VALUE) {
            char* buf = (char*)VirtualAlloc(NULL, 1024*1024, MEM_COMMIT, PAGE_READWRITE);
            if(buf) {
                DWORD r;
                ReadFile(hFile, buf, 1024*1024-1, &r, NULL);
                ParseCSVToVault(buf);
                VirtualFree(buf, 0, MEM_RELEASE);
                SaveVaultToFile();
                RefreshVaultList();
                MessageBoxA(hwnd, "Import successful!", "Import", MB_OK);
            }
            CloseHandle(hFile);
        }
    }
}

static BOOL CALLBACK SetChildFont(HWND hChild, LPARAM lParam) {
    SendMessageA(hChild, WM_SETFONT, (WPARAM)lParam, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Window client width is 500, height is 620
            hHelpLabel = CreateWindowA("STATIC", "KPass Security & Vault Manager [Press H or F1 for Help]", WS_CHILD | SS_LEFT, 20, 8, 380, 18, hwnd, NULL, NULL, NULL);
            hBtnHelp = CreateWindowA("BUTTON", "Help (F1)", WS_CHILD | BS_PUSHBUTTON, 405, 5, 75, 22, hwnd, (HMENU)1009, NULL, NULL);

            hDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Click Generate...", WS_CHILD | ES_CENTER | ES_READONLY | ES_AUTOHSCROLL, 20, 32, 460, 32, hwnd, NULL, NULL, NULL);
            hStrengthDisplay = CreateWindowA("STATIC", "Strength: - (0 bits)", WS_CHILD | SS_CENTER, 20, 68, 460, 18, hwnd, NULL, NULL, NULL);

            hUpper = CreateWindowA("BUTTON", "Uppercase", WS_CHILD | BS_AUTOCHECKBOX, 20, 92, 110, 20, hwnd, NULL, NULL, NULL);
            hLower = CreateWindowA("BUTTON", "Lowercase", WS_CHILD | BS_AUTOCHECKBOX, 135, 92, 110, 20, hwnd, NULL, NULL, NULL);
            hNum = CreateWindowA("BUTTON", "Numbers", WS_CHILD | BS_AUTOCHECKBOX, 250, 92, 110, 20, hwnd, NULL, NULL, NULL);
            hSym = CreateWindowA("BUTTON", "Symbols", WS_CHILD | BS_AUTOCHECKBOX, 365, 92, 110, 20, hwnd, NULL, NULL, NULL);

            SendMessage(hUpper, BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(hLower, BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(hNum, BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(hSym, BM_SETCHECK, BST_CHECKED, 0);

            hLenLabel = CreateWindowA("STATIC", "Length:", WS_CHILD, 20, 120, 52, 20, hwnd, NULL, NULL, NULL);
            hLen = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "16", WS_CHILD | ES_NUMBER | ES_CENTER, 74, 118, 46, 24, hwnd, NULL, NULL, NULL);
            
            hBtnGen = CreateWindowA("BUTTON", "Generate (Enter)", WS_CHILD | BS_PUSHBUTTON, 130, 118, 150, 24, hwnd, (HMENU)1001, NULL, NULL);
            hBtnCopy = CreateWindowA("BUTTON", "Copy Password", WS_CHILD | BS_PUSHBUTTON, 290, 118, 190, 24, hwnd, (HMENU)1002, NULL, NULL);

            hLabelInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 20, 152, 160, 24, hwnd, NULL, NULL, NULL);
            SendMessageA(hLabelInput, EM_SETCUEBANNER, FALSE, (LPARAM)L"Label (e.g. Email)");
            hCatInput = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | CBS_DROPDOWN, 190, 152, 110, 120, hwnd, NULL, NULL, NULL);
            SendMessageA(hCatInput, CB_ADDSTRING, 0, (LPARAM)"Personal");
            SendMessageA(hCatInput, CB_ADDSTRING, 0, (LPARAM)"Work");
            SendMessageA(hCatInput, CB_ADDSTRING, 0, (LPARAM)"Finance");
            SendMessageA(hCatInput, CB_ADDSTRING, 0, (LPARAM)"Social");
            SendMessageA(hCatInput, CB_ADDSTRING, 0, (LPARAM)"Other");
            SendMessageA(hCatInput, CB_SETCURSEL, 0, 0);
            
            hBtnSave = CreateWindowA("BUTTON", "Save to Vault", WS_CHILD | BS_PUSHBUTTON, 310, 152, 170, 24, hwnd, (HMENU)1003, NULL, NULL);

            hVaultSearch = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 20, 186, 160, 24, hwnd, (HMENU)2001, NULL, NULL);
            SendMessageA(hVaultSearch, EM_SETCUEBANNER, FALSE, (LPARAM)L"Search vault...");
            hFilterCat = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | CBS_DROPDOWNLIST, 190, 186, 110, 120, hwnd, (HMENU)2003, NULL, NULL);
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"All Cats");
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"Personal");
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"Work");
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"Finance");
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"Social");
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"Other");
            SendMessageA(hFilterCat, CB_SETCURSEL, 0, 0);

            hBtnCopyVault = CreateWindowA("BUTTON", "Copy Pass", WS_CHILD | BS_PUSHBUTTON, 310, 186, 85, 24, hwnd, (HMENU)1004, NULL, NULL);
            hBtnDelVault = CreateWindowA("BUTTON", "Delete", WS_CHILD | BS_PUSHBUTTON, 402, 186, 78, 24, hwnd, (HMENU)1005, NULL, NULL);

            hVaultList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VSCROLL | LBS_NOTIFY, 20, 220, 460, 320, hwnd, (HMENU)2002, NULL, NULL);
            
            hBtnExpCSV = CreateWindowA("BUTTON", "Exp CSV", WS_CHILD | BS_PUSHBUTTON, 20, 550, 80, 26, hwnd, (HMENU)1006, NULL, NULL);
            hBtnExpJSON = CreateWindowA("BUTTON", "Exp JSON", WS_CHILD | BS_PUSHBUTTON, 106, 550, 80, 26, hwnd, (HMENU)1007, NULL, NULL);
            hBtnImp = CreateWindowA("BUTTON", "Imp CSV", WS_CHILD | BS_PUSHBUTTON, 192, 550, 80, 26, hwnd, (HMENU)1008, NULL, NULL);
            hBtnLockMain = CreateWindowA("BUTTON", "Lock Vault", WS_CHILD | BS_PUSHBUTTON, 360, 550, 120, 26, hwnd, (HMENU)1010, NULL, NULL);

            // Lock screen controls
            hLockLabel = CreateWindowA("STATIC", "KPass Vault Locked", WS_CHILD | SS_CENTER, 40, 185, 420, 26, hwnd, NULL, NULL, NULL);
            hLockInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_PASSWORD | ES_AUTOHSCROLL | ES_CENTER, 125, 220, 250, 26, hwnd, NULL, NULL, NULL);
            SendMessageA(hLockInput, EM_SETCUEBANNER, FALSE, (LPARAM)L"Master Password");
            hBtnUnlock = CreateWindowA("BUTTON", "Unlock / Setup (Enter)", WS_CHILD | BS_PUSHBUTTON, 150, 258, 200, 32, hwnd, (HMENU)3001, NULL, NULL);
            hLockHelpLabel = CreateWindowA("STATIC", "Enter your master password to unlock.\nIf first time, entering a password initializes your encrypted vault.", WS_CHILD | SS_CENTER, 40, 305, 420, 36, hwnd, NULL, NULL, NULL);

            hBgBrush = CreateSolidBrush(RGB(30, 30, 30));
            hEditBrush = CreateSolidBrush(RGB(22, 22, 22));

            hFont = CreateFontA(-22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5 /*CLEARTYPE_QUALITY*/, DEFAULT_PITCH | FF_MODERN, "Consolas");
            hBtnFont = CreateFontA(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5 /*CLEARTYPE_QUALITY*/, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hSmallFont = CreateFontA(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5 /*CLEARTYPE_QUALITY*/, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            EnumChildWindows(hwnd, SetChildFont, (LPARAM)hSmallFont);

            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnGen, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessageA(hBtnCopy, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessageA(hBtnSave, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessageA(hBtnUnlock, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessageA(hBtnLockMain, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessageA(hStrengthDisplay, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            SendMessageA(hLockLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

            LockUI(1);
            SetTimer(hwnd, 1, 1000, NULL);
            break;
        }
        case WM_TIMER: {
            if(!g_locked) {
                LASTINPUTINFO lii;
                lii.cbSize = sizeof(LASTINPUTINFO);
                if(GetLastInputInfo(&lii)) {
                    if(GetTickCount() - lii.dwTime > 60000) {
                        g_masterPass[0] = 0;
                        g_vaultCount = 0;
                        LockUI(1);
                    }
                }
            }
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == 3001) { // Unlock
                GetWindowTextA(hLockInput, g_masterPass, sizeof(g_masterPass));
                if(g_masterPass[0] == 0) break;
                
                int res = LoadVaultFromFile(g_masterPass);
                if(res == 1) { // Success
                    LockUI(0);
                    RefreshVaultList();
                    SetWindowTextA(hLockInput, "");
                } else if(res == 0) { // Fail
                    MessageBoxA(hwnd, "Incorrect Master Password", "Error", MB_ICONERROR);
                } else { // Not found, setup
                    SaveVaultToFile();
                    LockUI(0);
                    RefreshVaultList();
                    SetWindowTextA(hLockInput, "");
                }
            } else if (wmId == 1009) { // Help
                ShowHelpModal(hwnd);
            } else if(!g_locked) {
                if (wmId == 1001) {
                    GeneratePassword();
                } else if (wmId == 1002) {
                    char pwd[65];
                    GetWindowTextA(hDisplay, pwd, 65);
                    if (pwd[0] && !my_strstr_ic(pwd, "Click Generate")) {
                        CopyToClipboard(hwnd, pwd);
                        SetWindowTextA(hStrengthDisplay, "Password copied to clipboard!");
                    }
                } else if (wmId == 1003) {
                    char label[64] = {0}, cat[32] = {0}, pass[64] = {0};
                    GetWindowTextA(hLabelInput, label, 64);
                    GetWindowTextA(hCatInput, cat, 32);
                    GetWindowTextA(hDisplay, pass, 64);
                    if(label[0] != 0 && pass[0] != 0 && !my_strstr_ic(pass, "Click Generate")) {
                        if(g_vaultCount < 200) {
                            my_strncpy(g_vault[g_vaultCount].label, label, sizeof(g_vault[g_vaultCount].label));
                            my_strncpy(g_vault[g_vaultCount].category, cat, sizeof(g_vault[g_vaultCount].category));
                            my_strncpy(g_vault[g_vaultCount].pass, pass, sizeof(g_vault[g_vaultCount].pass));
                            char dummy[64], strRating[20];
                            CalculateStrength(pass, dummy, strRating);
                            my_strcpy(g_vault[g_vaultCount].strength, strRating);
                            g_vaultCount++;
                            SaveVaultToFile();
                            RefreshVaultList();
                            SetWindowTextA(hLabelInput, "");
                            SetWindowTextA(hStrengthDisplay, "Saved to vault successfully!");
                        } else { MessageBoxA(hwnd, "Vault is full (max 200 entries).", "Error", MB_OK); }
                    } else {
                        MessageBoxA(hwnd, "Please enter a label and generate a password first.", "KPass", MB_OK | MB_ICONINFORMATION);
                    }
                } else if (wmId == 1004 || (wmId == 2002 && wmEvent == LBN_DBLCLK)) {
                    int sel = SendMessageA(hVaultList, LB_GETCURSEL, 0, 0);
                    if(sel != LB_ERR) {
                        int realIdx = SendMessageA(hVaultList, LB_GETITEMDATA, sel, 0);
                        CopyToClipboard(hwnd, g_vault[realIdx].pass);
                        SetWindowTextA(hStrengthDisplay, "Vault password copied to clipboard!");
                    }
                } else if (wmId == 1005) {
                    int sel = SendMessageA(hVaultList, LB_GETCURSEL, 0, 0);
                    if(sel != LB_ERR) {
                        int realIdx = SendMessageA(hVaultList, LB_GETITEMDATA, sel, 0);
                        for(int i = realIdx; i < g_vaultCount - 1; i++) g_vault[i] = g_vault[i+1];
                        g_vaultCount--;
                        SaveVaultToFile();
                        RefreshVaultList();
                        SetWindowTextA(hStrengthDisplay, "Entry deleted.");
                    }
                } else if (wmId == 1006) {
                    ExportFile(hwnd, 0);
                } else if (wmId == 1007) {
                    ExportFile(hwnd, 1);
                } else if (wmId == 1008) {
                    ImportFile(hwnd);
                } else if (wmId == 1010) { // Lock
                    g_masterPass[0] = 0;
                    g_vaultCount = 0;
                    LockUI(1);
                    SetWindowTextA(hLockInput, "");
                } else if ((wmId == 2001 && wmEvent == EN_CHANGE) || (wmId == 2003 && wmEvent == CBN_SELCHANGE)) {
                    RefreshVaultList();
                }
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hCtrl = (HWND)lParam;
            SetBkColor(hdc, RGB(30, 30, 30));
            if (hCtrl == hStrengthDisplay) {
                SetTextColor(hdc, RGB(46, 204, 113));
            } else if (hCtrl == hHelpLabel || hCtrl == hLockHelpLabel) {
                SetTextColor(hdc, RGB(160, 160, 160));
            } else if (hCtrl == hLockLabel) {
                SetTextColor(hdc, RGB(231, 76, 60));
            } else {
                SetTextColor(hdc, RGB(225, 225, 225));
            }
            return (LRESULT)hBgBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(22, 22, 22));
            SetTextColor(hdc, RGB(245, 245, 245));
            return (LRESULT)hEditBrush;
        }
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(22, 22, 22));
            SetTextColor(hdc, RGB(235, 235, 235));
            return (LRESULT)hEditBrush;
        }
        case WM_DESTROY:
            if (hBgBrush) DeleteObject(hBgBrush);
            if (hEditBrush) DeleteObject(hEditBrush);
            if (hFont) DeleteObject(hFont);
            if (hBtnFont) DeleteObject(hBtnFont);
            if (hSmallFont) DeleteObject(hSmallFont);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    typedef BOOL (WINAPI *SetProcessDPIAwareFunc)();
    HMODULE hUser32 = LoadLibraryA("user32.dll");
    if(hUser32) {
        SetProcessDPIAwareFunc setDPI = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if(setDPI) setDPI();
    }
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KPassClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));

    RegisterClassA(&wc);
    RECT rc = { 0, 0, 500, 620 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    HWND hwnd = CreateWindowExA(0, "KPassClass", "KPass Security & Vault Manager [Press H or F1 for Help]", (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1) {
                ShowHelpModal(hwnd);
                continue;
            } else if (msg.wParam == 'H' || msg.wParam == 'h') {
                char cls[64] = {0};
                GetClassNameA(GetFocus(), cls, 64);
                if (my_strstr_ic(cls, "EDIT") == 0) {
                    ShowHelpModal(hwnd);
                    continue;
                }
            } else if (msg.wParam == VK_RETURN) {
                if (g_locked) {
                    SendMessage(hwnd, WM_COMMAND, 3001, 0);
                    continue;
                } else {
                    HWND hFoc = GetFocus();
                    if (hFoc == hLabelInput) {
                        SendMessage(hwnd, WM_COMMAND, 1003, 0);
                        continue;
                    } else if (hFoc == hLen || hFoc == hDisplay) {
                        SendMessage(hwnd, WM_COMMAND, 1001, 0);
                        continue;
                    }
                }
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    if (wc.hbrBackground) DeleteObject(wc.hbrBackground);
    ExitProcess(0);
}
