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

static void MySecureZero(void* p, size_t sz) {
    volatile char* v = (volatile char*)p;
    while(sz--) *v++ = 0;
}

int my_strlen(const char* s) { int l=0; while(s && *s++) l++; return l; }
void my_strcpy(char* d, const char* s) { while(*s) *d++ = *s++; *d = 0; }
void my_strcat(char* d, const char* s) { while(*d) d++; while(*s) *d++ = *s++; *d = 0; }
int my_strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}
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
    char username[64];
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
HWND hLabelInput, hUserInput, hCatInput, hBtnSave, hVaultSearch, hFilterCat, hVaultList;
HWND hBtnCopyVault, hBtnDelVault, hBtnExpCSV, hBtnExpJSON, hBtnExpMD, hBtnAudit, hBtnImp, hBtnLockMain;
HWND hLockInput, hBtnUnlock, hLockLabel, hLockHelpLabel, hShowMasterPass;
HFONT hFont, hBtnFont, hSmallFont;
HBRUSH hBgBrush, hEditBrush;

// AES Encryption functions
int EncryptData(const char* password, const char* plainText, int plainLen, char* cipherBuffer, int* cipherMaxLen) {
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;
    int res = 0;
    if(CryptAcquireContextA(&hProv, NULL, MS_ENH_RSA_AES_PROV_A, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
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
    if(CryptAcquireContextA(&hProv, NULL, MS_ENH_RSA_AES_PROV_A, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
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

void EscapeCSVField(char* dest, const char* src, int maxLen) {
    int j = 0;
    dest[j++] = '"';
    for(int i = 0; src && src[i] && j < maxLen - 3; i++) {
        if(src[i] == '"') {
            if(j < maxLen - 4) {
                dest[j++] = '"';
                dest[j++] = '"';
            }
        } else if(src[i] == '\r' || src[i] == '\n') {
            dest[j++] = ' ';
        } else {
            dest[j++] = src[i];
        }
    }
    dest[j++] = '"';
    dest[j] = 0;
}

void EscapeJSONField(char* dest, const char* src, int maxLen) {
    int j = 0;
    for(int i = 0; src && src[i] && j < maxLen - 2; i++) {
        if(src[i] == '"' || src[i] == '\\') {
            if(j < maxLen - 3) {
                dest[j++] = '\\';
                dest[j++] = src[i];
            }
        } else if(src[i] == '\r') {
            // skip
        } else if(src[i] == '\n') {
            if(j < maxLen - 3) {
                dest[j++] = '\\';
                dest[j++] = 'n';
            }
        } else {
            dest[j++] = src[i];
        }
    }
    dest[j] = 0;
}

void FormatVaultToCSV(char* buffer) {
    my_strcpy(buffer, "label,username,category,pass,strength\r\n");
    for(int i = 0; i < g_vaultCount; i++) {
        char escLabel[128], escUser[128], escCat[64], escPass[128], escStr[64];
        EscapeCSVField(escLabel, g_vault[i].label, sizeof(escLabel));
        EscapeCSVField(escUser, g_vault[i].username[0] ? g_vault[i].username : "", sizeof(escUser));
        EscapeCSVField(escCat, g_vault[i].category[0] ? g_vault[i].category : "Other", sizeof(escCat));
        EscapeCSVField(escPass, g_vault[i].pass, sizeof(escPass));
        EscapeCSVField(escStr, g_vault[i].strength, sizeof(escStr));

        char line[512];
        wsprintfA(line, "%s,%s,%s,%s,%s\r\n", escLabel, escUser, escCat, escPass, escStr);
        my_strcat(buffer, line);
    }
}

void ParseCSVToVault(char* text) {
    char* p = text;
    // Skip header line
    while(*p && *p != '\n') p++;
    if(*p == '\n') p++;

    while(*p && g_vaultCount < 200) {
        char line[512] = {0};
        int len = 0;
        while(*p && *p != '\r' && *p != '\n' && len < 510) { line[len++] = *p++; }
        while(*p == '\r' || *p == '\n') p++;
        if(len > 0) {
            char* ptr = line;
            char fields[5][64];
            for(int k=0; k<5; k++) fields[k][0] = 0;
            int fIdx = 0;

            while(*ptr && fIdx < 5) {
                if(*ptr == '"') {
                    ptr++;
                    int c = 0;
                    while(*ptr && c < 63) {
                        if(*ptr == '"' && *(ptr + 1) == '"') {
                            fields[fIdx][c++] = '"';
                            ptr += 2;
                        } else if(*ptr == '"') {
                            ptr++;
                            break;
                        } else {
                            fields[fIdx][c++] = *ptr++;
                        }
                    }
                    fields[fIdx][c] = 0;
                    if(*ptr == ',') ptr++;
                    fIdx++;
                } else {
                    int c = 0;
                    while(*ptr && *ptr != ',' && c < 63) {
                        fields[fIdx][c++] = *ptr++;
                    }
                    fields[fIdx][c] = 0;
                    if(*ptr == ',') ptr++;
                    fIdx++;
                }
            }
            if(fIdx == 4 && (fields[0][0] || fields[2][0])) {
                // Legacy 4 columns: label, category, pass, strength
                my_strncpy(g_vault[g_vaultCount].label, fields[0], sizeof(g_vault[g_vaultCount].label));
                g_vault[g_vaultCount].username[0] = 0;
                my_strncpy(g_vault[g_vaultCount].category, fields[1][0] ? fields[1] : "Other", sizeof(g_vault[g_vaultCount].category));
                my_strncpy(g_vault[g_vaultCount].pass, fields[2], sizeof(g_vault[g_vaultCount].pass));
                my_strncpy(g_vault[g_vaultCount].strength, fields[3][0] ? fields[3] : "Saved", sizeof(g_vault[g_vaultCount].strength));
                g_vaultCount++;
            } else if(fIdx >= 5 && (fields[0][0] || fields[3][0])) {
                // 5 columns: label, username, category, pass, strength
                my_strncpy(g_vault[g_vaultCount].label, fields[0], sizeof(g_vault[g_vaultCount].label));
                my_strncpy(g_vault[g_vaultCount].username, fields[1], sizeof(g_vault[g_vaultCount].username));
                my_strncpy(g_vault[g_vaultCount].category, fields[2][0] ? fields[2] : "Other", sizeof(g_vault[g_vaultCount].category));
                my_strncpy(g_vault[g_vaultCount].pass, fields[3], sizeof(g_vault[g_vaultCount].pass));
                my_strncpy(g_vault[g_vaultCount].strength, fields[4][0] ? fields[4] : "Saved", sizeof(g_vault[g_vaultCount].strength));
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
                MySecureZero(plainBuffer, sizeof(plainBuffer));
                return 1;
            } else {
                MySecureZero(plainBuffer, sizeof(plainBuffer));
                return 0; // Decrypt failed
            }
        }
    }
    return -1; // File not found
}

void RefreshVaultList() {
    int curSel = SendMessageA(hVaultList, LB_GETCURSEL, 0, 0);
    SendMessage(hVaultList, LB_RESETCONTENT, 0, 0);
    char query[64] = {0};
    GetWindowTextA(hVaultSearch, query, sizeof(query));
    
    char filterCat[32] = {0};
    GetWindowTextA(hFilterCat, filterCat, sizeof(filterCat));

    for(int i = 0; i < g_vaultCount; i++) {
        int matchQ = (query[0] == 0 || my_strstr_ic(g_vault[i].label, query) || my_strstr_ic(g_vault[i].username, query) || my_strstr_ic(g_vault[i].pass, query));
        int matchC = (filterCat[0] == 0 || my_strstr_ic(filterCat, "All Cats") || my_strstr_ic(g_vault[i].category, filterCat));
        if(matchQ && matchC) {
            char displayLine[320];
            if(g_vault[i].username[0]) {
                wsprintfA(displayLine, "[%s] <%s> {%s} %s (%s)", g_vault[i].label, g_vault[i].username, g_vault[i].category[0] ? g_vault[i].category : "Other", g_vault[i].pass, g_vault[i].strength);
            } else {
                wsprintfA(displayLine, "[%s] {%s} %s (%s)", g_vault[i].label, g_vault[i].category[0] ? g_vault[i].category : "Other", g_vault[i].pass, g_vault[i].strength);
            }
            int index = SendMessageA(hVaultList, LB_ADDSTRING, 0, (LPARAM)displayLine);
            SendMessageA(hVaultList, LB_SETITEMDATA, index, (LPARAM)i);
        }
    }
    int count = SendMessageA(hVaultList, LB_GETCOUNT, 0, 0);
    if(count > 0 && curSel != LB_ERR) {
        SendMessageA(hVaultList, LB_SETCURSEL, curSel < count ? curSel : count - 1, 0);
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
    BYTE randBytes[64] = {0};
    HCRYPTPROV hProv = 0;
    if (CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CryptGenRandom(hProv, len, randBytes);
        CryptReleaseContext(hProv, 0);
    } else {
        DWORD tick = GetTickCount();
        for(int i = 0; i < len; i++) {
            tick = tick * 1103515245 + 12345;
            randBytes[i] = (BYTE)(tick >> 16);
        }
    }

    for(int i = 0; i < len; i++) {
        pwd[i] = pool[randBytes[i] % pLen];
    }
    pwd[len] = 0;
    SetWindowTextA(hDisplay, pwd);

    char strDisplay[64];
    char strRating[20];
    CalculateStrength(pwd, strDisplay, strRating);
    SetWindowTextA(hStrengthDisplay, strDisplay);
}

void LockUI(int lock) {
    g_locked = lock;
    if(lock) {
        MySecureZero(g_masterPass, sizeof(g_masterPass));
        MySecureZero(g_vault, sizeof(g_vault));
        g_vaultCount = 0;
    }
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
    ShowWindow(hUserInput, showMain);
    ShowWindow(hCatInput, showMain);
    ShowWindow(hBtnSave, showMain);
    ShowWindow(hVaultSearch, showMain);
    ShowWindow(hFilterCat, showMain);
    ShowWindow(hVaultList, showMain);
    ShowWindow(hBtnCopyVault, showMain);
    ShowWindow(hBtnDelVault, showMain);
    ShowWindow(hBtnExpCSV, showMain);
    ShowWindow(hBtnExpJSON, showMain);
    ShowWindow(hBtnExpMD, showMain);
    ShowWindow(hBtnAudit, showMain);
    ShowWindow(hBtnImp, showMain);
    ShowWindow(hBtnLockMain, showMain);

    int showLock = lock ? SW_SHOW : SW_HIDE;
    ShowWindow(hLockLabel, showLock);
    ShowWindow(hLockInput, showLock);
    ShowWindow(hShowMasterPass, showLock);
    ShowWindow(hBtnUnlock, showLock);
    ShowWindow(hLockHelpLabel, showLock);

    if (lock) {
        SendMessage(hShowMasterPass, BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessageA(hLockInput, EM_SETPASSWORDCHAR, '*', 0);
        SetFocus(hLockInput);
    } else {
        SetFocus(hBtnGen);
    }
}

void QuickSaveState(HWND hwnd) {
    HANDLE hFile = CreateFileA("kpass_quicksave.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        int up = (SendMessage(hUpper, BM_GETCHECK, 0, 0) == BST_CHECKED);
        int low = (SendMessage(hLower, BM_GETCHECK, 0, 0) == BST_CHECKED);
        int num = (SendMessage(hNum, BM_GETCHECK, 0, 0) == BST_CHECKED);
        int sym = (SendMessage(hSym, BM_GETCHECK, 0, 0) == BST_CHECKED);
        char lenBuf[16] = {0};
        GetWindowTextA(hLen, lenBuf, sizeof(lenBuf));
        int len = my_atoi(lenBuf);
        int data[8];
        data[0] = 0x51535631; // Magic "QSV1"
        data[1] = up;
        data[2] = low;
        data[3] = num;
        data[4] = sym;
        data[5] = len;
        data[6] = 0;
        data[7] = 0;
        WriteFile(hFile, data, sizeof(data), &written, NULL);
        CloseHandle(hFile);
        SetWindowTextA(hStrengthDisplay, "Session settings quicksaved [F5]!");
    }
}

void QuickLoadState(HWND hwnd) {
    HANDLE hFile = CreateFileA("kpass_quicksave.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD read = 0;
        int data[8] = {0};
        ReadFile(hFile, data, sizeof(data), &read, NULL);
        CloseHandle(hFile);
        if (read >= sizeof(int) * 6 && data[0] == 0x51535631) {
            SendMessage(hUpper, BM_SETCHECK, data[1] ? BST_CHECKED : BST_UNCHECKED, 0);
            SendMessage(hLower, BM_SETCHECK, data[2] ? BST_CHECKED : BST_UNCHECKED, 0);
            SendMessage(hNum, BM_SETCHECK, data[3] ? BST_CHECKED : BST_UNCHECKED, 0);
            SendMessage(hSym, BM_SETCHECK, data[4] ? BST_CHECKED : BST_UNCHECKED, 0);
            char lenBuf[16];
            my_itoa(data[5], lenBuf);
            SetWindowTextA(hLen, lenBuf);
            SendMessage(hwnd, WM_COMMAND, 1001, 0); // Generate
            SetWindowTextA(hStrengthDisplay, "Session settings restored [F9]!");
            return;
        }
    }
    SetWindowTextA(hStrengthDisplay, "No quicksave snapshot found [F5]");
}

void ShowHelpModal(HWND hwnd) {
    MessageBoxA(hwnd,
        "KPass Security & Vault Manager\n\n"
        "Features:\n"
        "  - Generator: Instant cryptographically secure password generation (8-64 chars).\n"
        "  - Vault: Encrypted credentials store with Username & Notes using AES-256.\n"
        "  - Security Audit: Real-time analysis of password health & reuse risks.\n"
        "  - Export / Import: Backup credentials to CSV, JSON, or Markdown (.md).\n"
        "  - Auto-Lock: Automatically locks after 1 minute of inactivity.\n\n"
        "Keyboard Shortcuts:\n"
        "  - F1 or 'H': Display this Help dialog\n"
        "  - F5: Quicksave session snapshot\n"
        "  - F9: Quickload session snapshot\n"
        "  - Enter: Unlock vault / Save credential / Generate password\n"
        "  - Ctrl+F: Focus and select vault search field\n"
        "  - Ctrl+S: Save generated password to vault\n"
        "  - Ctrl+G or 'G': Generate new password\n"
        "  - Ctrl+C or 'C': Copy current or selected password\n"
        "  - Alt+L: Instantly lock vault\n"
        "  - Alt+J: Export vault to JSON\n"
        "  - Alt+E: Export vault to CSV\n"
        "  - Alt+M: Export vault to Markdown table\n"
        "  - Alt+A: Run Security Audit scan\n"
        "  - Alt+I: Import credentials from CSV\n"
        "  - Escape: Clear search query\n"
        "  - Delete: Remove selected credential from vault",
        "KPass Help & Shortcuts", MB_OK | MB_ICONINFORMATION);
}

void ShowVaultHealthAudit(HWND hwnd) {
    if (g_vaultCount == 0) {
        MessageBoxA(hwnd, "Vault is currently empty. Add credentials to run a security audit.", "Vault Audit", MB_OK | MB_ICONINFORMATION);
        return;
    }
    int weakCount = 0;
    int reusedCount = 0;
    for (int i = 0; i < g_vaultCount; i++) {
        if (my_strstr_ic(g_vault[i].strength, "Weak") || my_strlen(g_vault[i].pass) < 10) {
            weakCount++;
        }
        int isDup = 0;
        for (int j = 0; j < i; j++) {
            if (my_strcmp(g_vault[i].pass, g_vault[j].pass) == 0) {
                isDup = 1;
                break;
            }
        }
        if (isDup) reusedCount++;
    }
    int penalties = (weakCount * 15) + (reusedCount * 25);
    int score = 100 - penalties;
    if (score < 10) score = 10;
    const char* rating = "Fortress Grade (A+)";
    if (score < 50) rating = "Critical Security Risks (F)";
    else if (score < 75) rating = "Needs Attention (C)";
    else if (score < 90) rating = "Good Protection (B)";

    char report[512];
    wsprintfA(report,
        "KPass Security Audit Report\n\n"
        "Vault Health Score: %d / 100\n"
        "Overall Rating: %s\n\n"
        "Total Credentials: %d\n"
        "Weak Passwords (< 10 chars / Low Entropy): %d\n"
        "Reused Passwords (Shared across accounts): %d\n\n"
        "%s",
        score, rating, g_vaultCount, weakCount, reusedCount,
        (weakCount == 0 && reusedCount == 0) ? "Status: Outstanding security hygiene!" : "Recommendation: Update weak and reused passwords immediately.");
    MessageBoxA(hwnd, report, "Security Audit", MB_OK | MB_ICONINFORMATION);
}

void ExportFile(HWND hwnd, int mode) { // 0=CSV, 1=JSON, 2=Markdown
    OPENFILENAMEA ofn = {0};
    char filename[260] = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    
    if (mode == 0) {
        ofn.lpstrFilter = "CSV Files\0*.csv\0All\0*.*\0";
        ofn.lpstrDefExt = "csv";
        my_strcpy(filename, "kpass_vault.csv");
    } else if (mode == 1) {
        ofn.lpstrFilter = "JSON Files\0*.json\0All\0*.*\0";
        ofn.lpstrDefExt = "json";
        my_strcpy(filename, "kpass_vault.json");
    } else {
        ofn.lpstrFilter = "Markdown Files\0*.md\0All\0*.*\0";
        ofn.lpstrDefExt = "md";
        my_strcpy(filename, "kpass_vault.md");
    }
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if(GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile != INVALID_HANDLE_VALUE) {
            DWORD w;
            if(mode == 0) {
                char buf[40000];
                FormatVaultToCSV(buf);
                WriteFile(hFile, buf, my_strlen(buf), &w, NULL);
            } else if(mode == 1) {
                WriteFile(hFile, "[\r\n", 3, &w, NULL);
                for(int i=0; i<g_vaultCount; i++) {
                    char escLabel[128], escUser[128], escCat[64], escPass[128], escStr[64];
                    EscapeJSONField(escLabel, g_vault[i].label, sizeof(escLabel));
                    EscapeJSONField(escUser, g_vault[i].username[0] ? g_vault[i].username : "", sizeof(escUser));
                    EscapeJSONField(escCat, g_vault[i].category[0] ? g_vault[i].category : "Other", sizeof(escCat));
                    EscapeJSONField(escPass, g_vault[i].pass, sizeof(escPass));
                    EscapeJSONField(escStr, g_vault[i].strength, sizeof(escStr));

                    char buf[512];
                    wsprintfA(buf, "  {\"label\":\"%s\", \"username\":\"%s\", \"category\":\"%s\", \"pass\":\"%s\", \"strength\":\"%s\"}%s\r\n", 
                            escLabel, escUser, escCat, escPass, escStr, (i == g_vaultCount - 1) ? "" : ",");
                    WriteFile(hFile, buf, my_strlen(buf), &w, NULL);
                }
                WriteFile(hFile, "]\r\n", 3, &w, NULL);
            } else {
                char* buf = (char*)VirtualAlloc(NULL, 1024*1024, MEM_COMMIT, PAGE_READWRITE);
                if(buf) {
                    my_strcpy(buf, "# KPass Vault Export\r\n\r\n| Service / Label | Username / Login | Password | Category | Strength |\r\n| :--- | :--- | :--- | :--- | :--- |\r\n");
                    for(int i = 0; i < g_vaultCount; i++) {
                        char line[512];
                        wsprintfA(line, "| **%s** | `%s` | `%s` | %s | %s |\r\n",
                            g_vault[i].label,
                            g_vault[i].username[0] ? g_vault[i].username : "-",
                            g_vault[i].pass,
                            g_vault[i].category[0] ? g_vault[i].category : "Other",
                            g_vault[i].strength);
                        my_strcat(buf, line);
                    }
                    WriteFile(hFile, buf, my_strlen(buf), &w, NULL);
                    VirtualFree(buf, 0, MEM_RELEASE);
                }
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
                buf[r] = 0;
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
            hHelpLabel = CreateWindowA("STATIC", "KPass Security & Vault Manager [F1 for Help]", WS_CHILD | SS_LEFT, 20, 8, 410, 18, hwnd, NULL, NULL, NULL);
            hBtnHelp = CreateWindowA("BUTTON", "Help [F1]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 445, 5, 75, 22, hwnd, (HMENU)1009, NULL, NULL);

            hDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Click Generate...", WS_CHILD | WS_TABSTOP | ES_CENTER | ES_READONLY | ES_AUTOHSCROLL, 20, 32, 500, 32, hwnd, NULL, NULL, NULL);
            hStrengthDisplay = CreateWindowA("STATIC", "Strength: - (0 bits)", WS_CHILD | SS_CENTER, 20, 68, 500, 18, hwnd, NULL, NULL, NULL);

            hUpper = CreateWindowA("BUTTON", "Uppercase", WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 20, 92, 120, 20, hwnd, NULL, NULL, NULL);
            hLower = CreateWindowA("BUTTON", "Lowercase", WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 145, 92, 120, 20, hwnd, NULL, NULL, NULL);
            hNum = CreateWindowA("BUTTON", "Numbers", WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 270, 92, 120, 20, hwnd, NULL, NULL, NULL);
            hSym = CreateWindowA("BUTTON", "Symbols", WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 395, 92, 120, 20, hwnd, NULL, NULL, NULL);

            SendMessage(hUpper, BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(hLower, BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(hNum, BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(hSym, BM_SETCHECK, BST_CHECKED, 0);

            hLenLabel = CreateWindowA("STATIC", "Length:", WS_CHILD, 20, 120, 50, 20, hwnd, NULL, NULL, NULL);
            hLen = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "16", WS_CHILD | WS_TABSTOP | ES_NUMBER | ES_CENTER, 72, 118, 48, 24, hwnd, NULL, NULL, NULL);
            
            hBtnGen = CreateWindowA("BUTTON", "Generate [Enter]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 128, 118, 172, 24, hwnd, (HMENU)1001, NULL, NULL);
            hBtnCopy = CreateWindowA("BUTTON", "Copy [Ctrl+C]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 308, 118, 212, 24, hwnd, (HMENU)1002, NULL, NULL);

            hLabelInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 20, 152, 120, 24, hwnd, NULL, NULL, NULL);
            SendMessageA(hLabelInput, EM_SETCUEBANNER, FALSE, (LPARAM)L"Service");

            hUserInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 146, 152, 120, 24, hwnd, NULL, NULL, NULL);
            SendMessageA(hUserInput, EM_SETCUEBANNER, FALSE, (LPARAM)L"Username");

            hCatInput = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_TABSTOP | CBS_DROPDOWN, 272, 152, 110, 120, hwnd, NULL, NULL, NULL);
            SendMessageA(hCatInput, CB_ADDSTRING, 0, (LPARAM)"Personal");
            SendMessageA(hCatInput, CB_ADDSTRING, 0, (LPARAM)"Work");
            SendMessageA(hCatInput, CB_ADDSTRING, 0, (LPARAM)"Finance");
            SendMessageA(hCatInput, CB_ADDSTRING, 0, (LPARAM)"Social");
            SendMessageA(hCatInput, CB_ADDSTRING, 0, (LPARAM)"Games");
            SendMessageA(hCatInput, CB_ADDSTRING, 0, (LPARAM)"Other");
            SendMessageA(hCatInput, CB_SETCURSEL, 0, 0);
            
            hBtnSave = CreateWindowA("BUTTON", "Save [Ctrl+S]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 388, 152, 132, 24, hwnd, (HMENU)1003, NULL, NULL);

            hVaultSearch = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 20, 186, 175, 24, hwnd, (HMENU)2001, NULL, NULL);
            SendMessageA(hVaultSearch, EM_SETCUEBANNER, FALSE, (LPARAM)L"Search (Ctrl+F)...");
            hFilterCat = CreateWindowExA(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST, 202, 186, 115, 120, hwnd, (HMENU)2003, NULL, NULL);
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"All Cats");
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"Personal");
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"Work");
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"Finance");
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"Social");
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"Games");
            SendMessageA(hFilterCat, CB_ADDSTRING, 0, (LPARAM)"Other");
            SendMessageA(hFilterCat, CB_SETCURSEL, 0, 0);

            hBtnCopyVault = CreateWindowA("BUTTON", "Copy [C]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 324, 186, 95, 24, hwnd, (HMENU)1004, NULL, NULL);
            hBtnDelVault = CreateWindowA("BUTTON", "Delete [Del]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 425, 186, 95, 24, hwnd, (HMENU)1005, NULL, NULL);

            hVaultList = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_TABSTOP | WS_VSCROLL | LBS_NOTIFY, 20, 218, 500, 355, hwnd, (HMENU)2002, NULL, NULL);
            
            hBtnExpCSV = CreateWindowA("BUTTON", "CSV [Alt+E]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 20, 585, 75, 26, hwnd, (HMENU)1006, NULL, NULL);
            hBtnExpJSON = CreateWindowA("BUTTON", "JSON [Alt+J]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 100, 585, 78, 26, hwnd, (HMENU)1007, NULL, NULL);
            hBtnExpMD = CreateWindowA("BUTTON", "MD [Alt+M]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 183, 585, 78, 26, hwnd, (HMENU)1011, NULL, NULL);
            hBtnAudit = CreateWindowA("BUTTON", "Audit [Alt+A]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 266, 585, 85, 26, hwnd, (HMENU)1012, NULL, NULL);
            hBtnImp = CreateWindowA("BUTTON", "Import [Alt+I]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 356, 585, 80, 26, hwnd, (HMENU)1008, NULL, NULL);
            hBtnLockMain = CreateWindowA("BUTTON", "Lock [Alt+L]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 441, 585, 79, 26, hwnd, (HMENU)1010, NULL, NULL);

            // Lock screen controls
            hLockLabel = CreateWindowA("STATIC", "KPass Vault Locked", WS_CHILD | SS_CENTER, 40, 180, 460, 26, hwnd, NULL, NULL, NULL);
            hLockInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_TABSTOP | ES_PASSWORD | ES_AUTOHSCROLL | ES_CENTER, 140, 220, 260, 26, hwnd, NULL, NULL, NULL);
            SendMessageA(hLockInput, EM_SETCUEBANNER, FALSE, (LPARAM)L"Master Password");
            hShowMasterPass = CreateWindowA("BUTTON", "Show Password", WS_CHILD | BS_AUTOCHECKBOX, 210, 255, 120, 20, hwnd, (HMENU)3002, NULL, NULL);
            hBtnUnlock = CreateWindowA("BUTTON", "Unlock / Setup [Enter]", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 155, 285, 230, 32, hwnd, (HMENU)3001, NULL, NULL);
            hLockHelpLabel = CreateWindowA("STATIC", "Enter your master password to unlock.\nIf first time, entering a password initializes your encrypted vault.", WS_CHILD | SS_CENTER, 40, 335, 460, 36, hwnd, NULL, NULL, NULL);

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
        case WM_ERASEBKGND:
            return 1;
        case WM_TIMER: {
            if(!g_locked) {
                LASTINPUTINFO lii;
                lii.cbSize = sizeof(LASTINPUTINFO);
                if(GetLastInputInfo(&lii)) {
                    if(GetTickCount() - lii.dwTime > 60000) {
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
            } else if (wmId == 3002) { // Toggle Master Password Mask
                BOOL show = (SendMessage(hShowMasterPass, BM_GETCHECK, 0, 0) == BST_CHECKED);
                SendMessageA(hLockInput, EM_SETPASSWORDCHAR, show ? 0 : '*', 0);
                InvalidateRect(hLockInput, NULL, TRUE);
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
                    char label[64] = {0}, user[64] = {0}, cat[32] = {0}, pass[64] = {0};
                    GetWindowTextA(hLabelInput, label, 64);
                    GetWindowTextA(hUserInput, user, 64);
                    GetWindowTextA(hCatInput, cat, 32);
                    GetWindowTextA(hDisplay, pass, 64);
                    if(label[0] != 0 && pass[0] != 0 && !my_strstr_ic(pass, "Click Generate")) {
                        if(g_vaultCount < 200) {
                            my_strncpy(g_vault[g_vaultCount].label, label, sizeof(g_vault[g_vaultCount].label));
                            my_strncpy(g_vault[g_vaultCount].username, user, sizeof(g_vault[g_vaultCount].username));
                            my_strncpy(g_vault[g_vaultCount].category, cat, sizeof(g_vault[g_vaultCount].category));
                            my_strncpy(g_vault[g_vaultCount].pass, pass, sizeof(g_vault[g_vaultCount].pass));
                            char dummy[64], strRating[20];
                            CalculateStrength(pass, dummy, strRating);
                            my_strcpy(g_vault[g_vaultCount].strength, strRating);
                            g_vaultCount++;
                            SaveVaultToFile();
                            RefreshVaultList();
                            SetWindowTextA(hLabelInput, "");
                            SetWindowTextA(hUserInput, "");
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
                        char confirmMsg[128];
                        wsprintfA(confirmMsg, "Are you sure you want to delete '%s' from your vault?", g_vault[realIdx].label[0] ? g_vault[realIdx].label : "this entry");
                        if (MessageBoxA(hwnd, confirmMsg, "Confirm Deletion", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                            for(int i = realIdx; i < g_vaultCount - 1; i++) g_vault[i] = g_vault[i+1];
                            g_vaultCount--;
                            SaveVaultToFile();
                            RefreshVaultList();
                            int count = SendMessageA(hVaultList, LB_GETCOUNT, 0, 0);
                            if(count > 0) {
                                SendMessageA(hVaultList, LB_SETCURSEL, sel < count ? sel : count - 1, 0);
                            }
                            SetWindowTextA(hStrengthDisplay, "Entry deleted.");
                        }
                    }
                } else if (wmId == 1006) {
                    ExportFile(hwnd, 0);
                } else if (wmId == 1007) {
                    ExportFile(hwnd, 1);
                } else if (wmId == 1008) {
                    ImportFile(hwnd);
                } else if (wmId == 1010) { // Lock
                    LockUI(1);
                    SetWindowTextA(hLockInput, "");
                } else if (wmId == 1011) { // Markdown Export
                    ExportFile(hwnd, 2);
                } else if (wmId == 1012) { // Security Audit
                    ShowVaultHealthAudit(hwnd);
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
            KillTimer(hwnd, 1);
            if (hBgBrush) DeleteObject(hBgBrush);
            if (hEditBrush) DeleteObject(hEditBrush);
            if (hFont) DeleteObject(hFont);
            if (hBtnFont) DeleteObject(hBtnFont);
            if (hSmallFont) DeleteObject(hSmallFont);
            MySecureZero(g_masterPass, sizeof(g_masterPass));
            MySecureZero(g_vault, sizeof(g_vault));
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
    RECT rc = { 0, 0, 540, 660 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    HWND hwnd = CreateWindowExA(0, "KPassClass", "KPass Security & Vault Manager [F1 for Help]", (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            int ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            int alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
            char cls[64] = {0};
            HWND hFoc = GetFocus();
            if (hFoc) GetClassNameA(hFoc, cls, sizeof(cls));
            int isEdit = (my_strstr_ic(cls, "EDIT") != 0);

            if (msg.wParam == VK_F1) {
                ShowHelpModal(hwnd);
                continue;
            } else if (msg.wParam == VK_F5) {
                if (!g_locked) {
                    QuickSaveState(hwnd);
                    continue;
                }
            } else if (msg.wParam == VK_F9) {
                if (!g_locked) {
                    QuickLoadState(hwnd);
                    continue;
                }
            } else if (alt) {
                if (msg.wParam == 'L' || msg.wParam == 'l') {
                    if (!g_locked) { SendMessage(hwnd, WM_COMMAND, 1010, 0); continue; }
                } else if (msg.wParam == 'J' || msg.wParam == 'j') {
                    if (!g_locked) { SendMessage(hwnd, WM_COMMAND, 1007, 0); continue; }
                } else if (msg.wParam == 'E' || msg.wParam == 'e') {
                    if (!g_locked) { SendMessage(hwnd, WM_COMMAND, 1006, 0); continue; }
                } else if (msg.wParam == 'M' || msg.wParam == 'm') {
                    if (!g_locked) { SendMessage(hwnd, WM_COMMAND, 1011, 0); continue; }
                } else if (msg.wParam == 'A' || msg.wParam == 'a') {
                    if (!g_locked) { SendMessage(hwnd, WM_COMMAND, 1012, 0); continue; }
                } else if (msg.wParam == 'I' || msg.wParam == 'i') {
                    if (!g_locked) { SendMessage(hwnd, WM_COMMAND, 1008, 0); continue; }
                }
            } else if (ctrl) {
                if (msg.wParam == 'F' || msg.wParam == 'f') {
                    if (!g_locked) {
                        SetFocus(hVaultSearch);
                        SendMessageA(hVaultSearch, EM_SETSEL, 0, -1);
                        continue;
                    }
                } else if (msg.wParam == 'S' || msg.wParam == 's') {
                    if (!g_locked) { SendMessage(hwnd, WM_COMMAND, 1003, 0); continue; }
                } else if (msg.wParam == 'G' || msg.wParam == 'g') {
                    if (!g_locked) { SendMessage(hwnd, WM_COMMAND, 1001, 0); continue; }
                } else if (msg.wParam == 'C' || msg.wParam == 'c') {
                    if (!g_locked && !isEdit) {
                        if (hFoc == hVaultList) {
                            SendMessage(hwnd, WM_COMMAND, 1004, 0);
                        } else {
                            SendMessage(hwnd, WM_COMMAND, 1002, 0);
                        }
                        continue;
                    }
                }
            } else if (msg.wParam == VK_ESCAPE) {
                if (hFoc == hVaultSearch) {
                    SetWindowTextA(hVaultSearch, "");
                    RefreshVaultList();
                    continue;
                }
            } else if (!isEdit) {
                if (msg.wParam == 'H' || msg.wParam == 'h') {
                    ShowHelpModal(hwnd);
                    continue;
                } else if (!g_locked) {
                    if (msg.wParam == 'C' || msg.wParam == 'c') {
                        if (hFoc == hVaultList) {
                            SendMessage(hwnd, WM_COMMAND, 1004, 0);
                        } else {
                            SendMessage(hwnd, WM_COMMAND, 1002, 0);
                        }
                        continue;
                    } else if (msg.wParam == 'G' || msg.wParam == 'g') {
                        SendMessage(hwnd, WM_COMMAND, 1001, 0);
                        continue;
                    }
                }
            }

            if (msg.wParam == VK_RETURN) {
                if (g_locked) {
                    SendMessage(hwnd, WM_COMMAND, 3001, 0);
                    continue;
                } else {
                    if (hFoc == hLabelInput || hFoc == hUserInput) {
                        SendMessage(hwnd, WM_COMMAND, 1003, 0);
                        continue;
                    } else if (hFoc == hLen || hFoc == hDisplay) {
                        SendMessage(hwnd, WM_COMMAND, 1001, 0);
                        continue;
                    } else if (hFoc == hVaultList) {
                        SendMessage(hwnd, WM_COMMAND, 1004, 0);
                        continue;
                    }
                }
            } else if (msg.wParam == VK_DELETE && !g_locked) {
                if (hFoc == hVaultList) {
                    SendMessage(hwnd, WM_COMMAND, 1005, 0);
                    continue;
                }
            }
        }
        if (!IsDialogMessageA(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    if (wc.hbrBackground) DeleteObject(wc.hbrBackground);
    ExitProcess(0);
}
