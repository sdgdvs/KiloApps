#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>

#define ID_BTN_ENCRYPT 101
#define ID_BTN_DECRYPT 102
#define ID_BTN_CLEAR 103
#define ID_EDIT_PASS 104
#define ID_EDIT_DATA 105
#define ID_BTN_LOAD 106
#define ID_BTN_SAVE 107
#define ID_EDIT_FIND 108
#define ID_BTN_FIND 109
#define ID_BTN_GENERATE 110
#define ID_COMBO_TIMEOUT 111
#define ID_STATIC_STRENGTH 112
#define ID_COMBO_TEMPLATE 113
#define ID_BTN_INSERT_TEMPLATE 114
#define ID_BTN_COPY_DATA 115
#define ID_BTN_CLEAR_CLIP 116
#define ID_COMBO_THEME 117
#define ID_BTN_HELP 118

#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER 0x1501
#endif

// Custom memory and string functions for CRT-free compilation
void* my_memset(void* p, int c, size_t sz) {
    unsigned char* pb = (unsigned char*)p;
    while (sz--) *pb++ = (unsigned char)c;
    return p;
}
#pragma function(memset)
void* __cdecl memset(void* p, int c, size_t sz) {
    return my_memset(p, c, sz);
}

void* my_memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}
#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    return my_memcpy(dest, src, count);
}

void secure_zero(void* ptr, size_t size) {
    if (!ptr || size == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (size--) *p++ = 0;
}

size_t my_strlen(const char* s) {
    size_t len = 0;
    while (s && *s++) len++;
    return len;
}

void my_strcpy(char* dest, const char* src) {
    if (!dest || !src) return;
    while (*src) *dest++ = *src++;
    *dest = '\0';
}

void my_strcat(char* dest, const char* src) {
    if (!dest || !src) return;
    while (*dest) dest++;
    while (*src) *dest++ = *src++;
    *dest = '\0';
}

char to_lower_char(char c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

const char* my_strstr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;
    if (!*needle) return haystack;
    size_t hLen = my_strlen(haystack);
    size_t nLen = my_strlen(needle);
    if (nLen > hLen) return NULL;
    for (size_t i = 0; i <= hLen - nLen; i++) {
        int match = 1;
        for (size_t j = 0; j < nLen; j++) {
            if (haystack[i + j] != needle[j]) {
                match = 0;
                break;
            }
        }
        if (match) return haystack + i;
    }
    return NULL;
}

const char* my_strstr_ic(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;
    if (!*needle) return haystack;
    size_t hLen = my_strlen(haystack);
    size_t nLen = my_strlen(needle);
    if (nLen > hLen) return NULL;
    for (size_t i = 0; i <= hLen - nLen; i++) {
        int match = 1;
        for (size_t j = 0; j < nLen; j++) {
            if (to_lower_char(haystack[i + j]) != to_lower_char(needle[j])) {
                match = 0;
                break;
            }
        }
        if (match) return haystack + i;
    }
    return NULL;
}

static unsigned int g_rngSeed = 12345;
void my_srand(unsigned int seed) {
    g_rngSeed = seed;
}
unsigned int my_rand(void) {
    LARGE_INTEGER pc;
    QueryPerformanceCounter(&pc);
    g_rngSeed = g_rngSeed * 1103515245 + 12345 + (unsigned int)pc.LowPart;
    return (g_rngSeed / 65536) % 32768;
}

static const char HEX_DIGITS[] = "0123456789ABCDEF";

void byte_to_hex(unsigned char b, char* out) {
    out[0] = HEX_DIGITS[(b >> 4) & 0x0F];
    out[1] = HEX_DIGITS[b & 0x0F];
}

int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

int hex_to_byte(const char* hex, unsigned char* outByte) {
    int h1 = hex_val(hex[0]);
    int h2 = hex_val(hex[1]);
    if (h1 < 0 || h2 < 0) return 0;
    *outByte = (unsigned char)((h1 << 4) | h2);
    return 1;
}

HWND hPass, hData;
HBRUSH hbgBrush = NULL;
HBRUSH hDarkBrush = NULL;
HFONT hFont = NULL, hTitleFont = NULL;

DWORD g_lastActivity = 0;
int g_timeoutMs = 60000;
int g_theme = 0;

void EncryptData(HWND hTextEdit, HWND hPassEdit) {
    int textLen = GetWindowTextLengthA(hTextEdit);
    int passLen = GetWindowTextLengthA(hPassEdit);
    if (textLen == 0) {
        MessageBoxA(GetParent(hTextEdit), "Vault data is empty.", "KVault", MB_OK | MB_ICONWARNING);
        return;
    }
    if (passLen == 0) {
        MessageBoxA(GetParent(hTextEdit), "Please enter a Master Key first.", "KVault", MB_OK | MB_ICONWARNING);
        return;
    }
    
    char* text = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, textLen + 1);
    char* pass = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, passLen + 1);
    char* hexOut = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, textLen * 2 + 1);
    
    if (!text || !pass || !hexOut) {
        if (text) { secure_zero(text, textLen + 1); HeapFree(GetProcessHeap(), 0, text); }
        if (pass) { secure_zero(pass, passLen + 1); HeapFree(GetProcessHeap(), 0, pass); }
        if (hexOut) HeapFree(GetProcessHeap(), 0, hexOut);
        return;
    }

    GetWindowTextA(hTextEdit, text, textLen + 1);
    GetWindowTextA(hPassEdit, pass, passLen + 1);
    
    for (int i = 0; i < textLen; i++) {
        unsigned char cipher = (unsigned char)text[i] ^ (unsigned char)pass[i % passLen];
        byte_to_hex(cipher, &hexOut[i * 2]);
    }
    hexOut[textLen * 2] = '\0';
    
    SetWindowTextA(hTextEdit, hexOut);

    secure_zero(text, textLen + 1);
    secure_zero(pass, passLen + 1);
    secure_zero(hexOut, textLen * 2 + 1);
    HeapFree(GetProcessHeap(), 0, text);
    HeapFree(GetProcessHeap(), 0, pass);
    HeapFree(GetProcessHeap(), 0, hexOut);
}

void DecryptData(HWND hTextEdit, HWND hPassEdit) {
    int textLen = GetWindowTextLengthA(hTextEdit);
    int passLen = GetWindowTextLengthA(hPassEdit);
    if (textLen == 0) {
        MessageBoxA(GetParent(hTextEdit), "No encrypted data to decrypt.", "KVault", MB_OK | MB_ICONWARNING);
        return;
    }
    if (passLen == 0) {
        MessageBoxA(GetParent(hTextEdit), "Please enter a Master Key first.", "KVault", MB_OK | MB_ICONWARNING);
        return;
    }
    if (textLen % 2 != 0) {
        MessageBoxA(GetParent(hTextEdit), "Data is not valid hex-encoded ciphertext.", "KVault", MB_OK | MB_ICONERROR);
        return;
    }
    
    char* hexText = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, textLen + 1);
    char* pass = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, passLen + 1);
    char* plainOut = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, textLen / 2 + 1);
    
    if (!hexText || !pass || !plainOut) {
        if (hexText) HeapFree(GetProcessHeap(), 0, hexText);
        if (pass) { secure_zero(pass, passLen + 1); HeapFree(GetProcessHeap(), 0, pass); }
        if (plainOut) { secure_zero(plainOut, textLen / 2 + 1); HeapFree(GetProcessHeap(), 0, plainOut); }
        return;
    }

    GetWindowTextA(hTextEdit, hexText, textLen + 1);
    GetWindowTextA(hPassEdit, pass, passLen + 1);
    
    int valid = 1;
    for (int i = 0; i < textLen / 2; i++) {
        unsigned char cipher = 0;
        if (!hex_to_byte(&hexText[i * 2], &cipher)) {
            valid = 0;
            break;
        }
        plainOut[i] = (char)(cipher ^ (unsigned char)pass[i % passLen]);
    }
    
    if (!valid) {
        MessageBoxA(GetParent(hTextEdit), "Invalid hex characters in encrypted payload.", "KVault", MB_OK | MB_ICONERROR);
    } else {
        plainOut[textLen / 2] = '\0';
        SetWindowTextA(hTextEdit, plainOut);
    }

    secure_zero(hexText, textLen + 1);
    secure_zero(pass, passLen + 1);
    secure_zero(plainOut, textLen / 2 + 1);
    HeapFree(GetProcessHeap(), 0, hexText);
    HeapFree(GetProcessHeap(), 0, pass);
    HeapFree(GetProcessHeap(), 0, plainOut);
}

void GeneratePassword(HWND hTextEdit) {
    const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+~`|}{[]:;?><,./-=";
    int charCount = sizeof(chars) - 1;
    int len = 16;
    char pass[17];
    
    for (int i = 0; i < len; i++) {
        pass[i] = chars[my_rand() % charCount];
    }
    pass[len] = '\0';
    
    int textLen = GetWindowTextLengthA(hTextEdit);
    char* newText = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, textLen + len + 4);
    if (!newText) return;

    if (textLen > 0) {
        GetWindowTextA(hTextEdit, newText, textLen + 1);
        my_strcat(newText, "\r\n");
        my_strcat(newText, pass);
    } else {
        my_strcpy(newText, pass);
    }
    SetWindowTextA(hTextEdit, newText);

    secure_zero(pass, sizeof(pass));
    secure_zero(newText, textLen + len + 4);
    HeapFree(GetProcessHeap(), 0, newText);
}

void LoadFromFile(HWND hwnd, HWND hTextEdit) {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    my_memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.txt\0JSON Files\0*.json\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileNameA(&ofn) == TRUE) {
        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD fileSize = GetFileSize(hFile, NULL);
            if (fileSize > 0 && fileSize < 10485760) {
                char* buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
                if (buffer) {
                    DWORD bytesRead = 0;
                    if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                        buffer[bytesRead] = '\0';
                        SetWindowTextA(hTextEdit, buffer);
                    }
                    HeapFree(GetProcessHeap(), 0, buffer);
                }
            } else if (fileSize == 0) {
                SetWindowTextA(hTextEdit, "");
            } else {
                MessageBoxA(hwnd, "File size exceeds 10MB limit.", "KVault", MB_OK | MB_ICONWARNING);
            }
            CloseHandle(hFile);
        }
    }
}

void SaveToFile(HWND hwnd, HWND hTextEdit) {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    my_memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.txt\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileNameA(&ofn) == TRUE) {
        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            int textLen = GetWindowTextLengthA(hTextEdit);
            if (textLen > 0) {
                char* buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, textLen + 1);
                if (buffer) {
                    GetWindowTextA(hTextEdit, buffer, textLen + 1);
                    DWORD bytesWritten = 0;
                    WriteFile(hFile, buffer, textLen, &bytesWritten, NULL);
                    secure_zero(buffer, textLen + 1);
                    HeapFree(GetProcessHeap(), 0, buffer);
                }
            }
            CloseHandle(hFile);
        }
    }
}

void ClearVault(HWND hwnd) {
    SetWindowTextA(hData, "");
    SetWindowTextA(hPass, "");
    SetWindowTextA(GetDlgItem(hwnd, ID_STATIC_STRENGTH), "");
    if (OpenClipboard(hwnd)) {
        EmptyClipboard();
        CloseClipboard();
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            hbgBrush = CreateSolidBrush(RGB(15, 15, 19));
            hDarkBrush = CreateSolidBrush(RGB(25, 25, 30));
            hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hTitleFont = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            
            HWND hTitle = CreateWindowA("STATIC", "🔒 KVault - Secure Storage", WS_VISIBLE | WS_CHILD, 15, 15, 290, 30, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
            
            HWND hStatTimeout = CreateWindowA("STATIC", "Auto-lock:", WS_VISIBLE | WS_CHILD, 315, 22, 70, 20, hwnd, NULL, NULL, NULL);
            SendMessage(hStatTimeout, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hComboTimeout = CreateWindowA("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 385, 20, 95, 100, hwnd, (HMENU)ID_COMBO_TIMEOUT, NULL, NULL);
            SendMessage(hComboTimeout, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hComboTimeout, CB_ADDSTRING, 0, (LPARAM)"1 min");
            SendMessage(hComboTimeout, CB_ADDSTRING, 0, (LPARAM)"5 min");
            SendMessage(hComboTimeout, CB_ADDSTRING, 0, (LPARAM)"15 min");
            SendMessage(hComboTimeout, CB_ADDSTRING, 0, (LPARAM)"Never");
            SendMessage(hComboTimeout, CB_SETCURSEL, 0, 0);
            
            HWND hComboTheme = CreateWindowA("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 490, 20, 90, 100, hwnd, (HMENU)ID_COMBO_THEME, NULL, NULL);
            SendMessage(hComboTheme, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hComboTheme, CB_ADDSTRING, 0, (LPARAM)"Dark");
            SendMessage(hComboTheme, CB_ADDSTRING, 0, (LPARAM)"Light");
            SendMessage(hComboTheme, CB_ADDSTRING, 0, (LPARAM)"Neon");
            SendMessage(hComboTheme, CB_SETCURSEL, 0, 0);
            
            SetTimer(hwnd, 1, 1000, NULL);
            
            HWND hStat = CreateWindowA("STATIC", "Master Key:", WS_VISIBLE | WS_CHILD, 15, 62, 80, 25, hwnd, NULL, NULL, NULL);
            SendMessage(hStat, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hPass = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL, 100, 60, 200, 25, hwnd, (HMENU)ID_EDIT_PASS, NULL, NULL);
            SendMessage(hPass, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hPass, EM_SETCUEBANNER, 0, (LPARAM)L"Enter Master Password");
            
            HWND hStrength = CreateWindowA("STATIC", "", WS_VISIBLE | WS_CHILD, 100, 85, 200, 15, hwnd, (HMENU)ID_STATIC_STRENGTH, NULL, NULL);
            SendMessage(hStrength, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnEnc = CreateWindowA("BUTTON", "Encrypt", WS_VISIBLE | WS_CHILD | BS_FLAT, 315, 60, 85, 25, hwnd, (HMENU)ID_BTN_ENCRYPT, NULL, NULL);
            HWND hBtnDec = CreateWindowA("BUTTON", "Decrypt", WS_VISIBLE | WS_CHILD | BS_FLAT, 410, 60, 85, 25, hwnd, (HMENU)ID_BTN_DECRYPT, NULL, NULL);
            HWND hBtnClr = CreateWindowA("BUTTON", "Clear", WS_VISIBLE | WS_CHILD | BS_FLAT, 505, 60, 75, 25, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
            
            SendMessage(hBtnEnc, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnDec, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnClr, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hData = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN, 15, 105, 565, 205, hwnd, (HMENU)ID_EDIT_DATA, NULL, NULL);
            SendMessage(hData, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hData, EM_SETCUEBANNER, 0, (LPARAM)L"Enter or load confidential data here...");
            
            HWND hBtnLoad = CreateWindowA("BUTTON", "Load File", WS_VISIBLE | WS_CHILD | BS_FLAT, 15, 320, 100, 25, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            HWND hBtnSave = CreateWindowA("BUTTON", "Save File", WS_VISIBLE | WS_CHILD | BS_FLAT, 125, 320, 100, 25, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            SendMessage(hBtnLoad, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSave, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hFindEdit = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 235, 320, 155, 25, hwnd, (HMENU)ID_EDIT_FIND, NULL, NULL);
            SendMessage(hFindEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hFindEdit, EM_SETCUEBANNER, 0, (LPARAM)L"Find text...");
            
            HWND hBtnFind = CreateWindowA("BUTTON", "Find", WS_VISIBLE | WS_CHILD | BS_FLAT, 400, 320, 80, 25, hwnd, (HMENU)ID_BTN_FIND, NULL, NULL);
            SendMessage(hBtnFind, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND hBtnGen = CreateWindowA("BUTTON", "Gen Pass", WS_VISIBLE | WS_CHILD | BS_FLAT, 490, 320, 90, 25, hwnd, (HMENU)ID_BTN_GENERATE, NULL, NULL);
            SendMessage(hBtnGen, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hComboTpl = CreateWindowA("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 15, 355, 100, 100, hwnd, (HMENU)ID_COMBO_TEMPLATE, NULL, NULL);
            SendMessage(hComboTpl, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hComboTpl, CB_ADDSTRING, 0, (LPARAM)"Login");
            SendMessage(hComboTpl, CB_ADDSTRING, 0, (LPARAM)"Finance");
            SendMessage(hComboTpl, CB_ADDSTRING, 0, (LPARAM)"Note");
            SendMessage(hComboTpl, CB_SETCURSEL, 0, 0);
            
            HWND hBtnTpl = CreateWindowA("BUTTON", "Insert", WS_VISIBLE | WS_CHILD | BS_FLAT, 125, 355, 60, 25, hwnd, (HMENU)ID_BTN_INSERT_TEMPLATE, NULL, NULL);
            SendMessage(hBtnTpl, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnCopyData = CreateWindowA("BUTTON", "Copy Data", WS_VISIBLE | WS_CHILD | BS_FLAT, 195, 355, 85, 25, hwnd, (HMENU)ID_BTN_COPY_DATA, NULL, NULL);
            SendMessage(hBtnCopyData, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnClearClip = CreateWindowA("BUTTON", "Clear Clip", WS_VISIBLE | WS_CHILD | BS_FLAT, 290, 355, 85, 25, hwnd, (HMENU)ID_BTN_CLEAR_CLIP, NULL, NULL);
            SendMessage(hBtnClearClip, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnHelp = CreateWindowA("BUTTON", "Help", WS_VISIBLE | WS_CHILD | BS_FLAT, 385, 355, 85, 25, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            SendMessage(hBtnHelp, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            LARGE_INTEGER pc;
            QueryPerformanceCounter(&pc);
            my_srand((unsigned int)GetTickCount() ^ (unsigned int)pc.LowPart);
            
            DragAcceptFiles(hwnd, TRUE);
            break;
        }
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            char szFile[260];
            if (DragQueryFileA(hDrop, 0, szFile, sizeof(szFile))) {
                HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD fileSize = GetFileSize(hFile, NULL);
                    if (fileSize > 0 && fileSize < 10485760) {
                        char* buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
                        if (buffer) {
                            DWORD bytesRead = 0;
                            if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                                buffer[bytesRead] = '\0';
                                SetWindowTextA(hData, buffer);
                            }
                            HeapFree(GetProcessHeap(), 0, buffer);
                        }
                    } else if (fileSize == 0) {
                        SetWindowTextA(hData, "");
                    }
                    CloseHandle(hFile);
                }
            }
            DragFinish(hDrop);
            break;
        }
        case WM_TIMER: {
            if (wParam == 1 && g_timeoutMs > 0) {
                DWORD idle = GetTickCount() - g_lastActivity;
                if (idle > (DWORD)g_timeoutMs) {
                    int textLen = GetWindowTextLengthA(hData);
                    int passLen = GetWindowTextLengthA(hPass);
                    if (textLen > 0 || passLen > 0) {
                        ClearVault(hwnd);
                        MessageBoxA(hwnd, "Vault locked due to inactivity.", "KVault", MB_OK | MB_ICONINFORMATION);
                    }
                    g_lastActivity = GetTickCount();
                }
            }
            break;
        }
        case WM_COMMAND: {
            if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == ID_EDIT_PASS) {
                char pwd[256];
                GetWindowTextA((HWND)lParam, pwd, sizeof(pwd));
                size_t len = my_strlen(pwd);
                if (len == 0) {
                    SetWindowTextA(GetDlgItem(hwnd, ID_STATIC_STRENGTH), "");
                } else {
                    int score = 0;
                    int hasUpper = 0, hasNum = 0, hasSym = 0;
                    for (size_t i = 0; i < len; i++) {
                        if (pwd[i] >= 'A' && pwd[i] <= 'Z') hasUpper = 1;
                        else if (pwd[i] >= '0' && pwd[i] <= '9') hasNum = 1;
                        else if (!(pwd[i] >= 'a' && pwd[i] <= 'z')) hasSym = 1;
                    }
                    if (len > 4) score++;
                    if (len >= 8) score++;
                    if (len >= 12) score++;
                    if (hasUpper) score++;
                    if (hasNum) score++;
                    if (hasSym) score++;
                    
                    if (score < 3) SetWindowTextA(GetDlgItem(hwnd, ID_STATIC_STRENGTH), "Strength: Weak");
                    else if (score < 5) SetWindowTextA(GetDlgItem(hwnd, ID_STATIC_STRENGTH), "Strength: Medium");
                    else SetWindowTextA(GetDlgItem(hwnd, ID_STATIC_STRENGTH), "Strength: Strong");
                    
                    InvalidateRect(GetDlgItem(hwnd, ID_STATIC_STRENGTH), NULL, TRUE);
                }
                secure_zero(pwd, sizeof(pwd));
            }
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == ID_COMBO_TIMEOUT) {
                int sel = (int)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                if (sel == 0) g_timeoutMs = 60000;
                else if (sel == 1) g_timeoutMs = 300000;
                else if (sel == 2) g_timeoutMs = 900000;
                else if (sel == 3) g_timeoutMs = 0;
                g_lastActivity = GetTickCount();
            }
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == ID_COMBO_THEME) {
                g_theme = (int)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                if (hbgBrush) DeleteObject(hbgBrush);
                if (hDarkBrush) DeleteObject(hDarkBrush);
                
                if (g_theme == 0) {
                    hbgBrush = CreateSolidBrush(RGB(15, 15, 19));
                    hDarkBrush = CreateSolidBrush(RGB(25, 25, 30));
                } else if (g_theme == 1) {
                    hbgBrush = CreateSolidBrush(RGB(240, 240, 245));
                    hDarkBrush = CreateSolidBrush(RGB(255, 255, 255));
                } else if (g_theme == 2) {
                    hbgBrush = CreateSolidBrush(RGB(5, 5, 5));
                    hDarkBrush = CreateSolidBrush(RGB(10, 15, 10));
                }
                SetClassLongPtrA(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbgBrush);
                InvalidateRect(hwnd, NULL, TRUE);
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
            }
            if (LOWORD(wParam) == ID_BTN_ENCRYPT) {
                EncryptData(hData, hPass);
            } else if (LOWORD(wParam) == ID_BTN_DECRYPT) {
                DecryptData(hData, hPass);
            } else if (LOWORD(wParam) == ID_BTN_CLEAR) {
                if (GetWindowTextLengthA(hData) > 0) {
                    if (MessageBoxA(hwnd, "Are you sure you want to clear the vault display?", "Confirm Clear", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        SetWindowTextA(hData, "");
                    }
                }
            } else if (LOWORD(wParam) == ID_BTN_LOAD) {
                LoadFromFile(hwnd, hData);
            } else if (LOWORD(wParam) == ID_BTN_SAVE) {
                SaveToFile(hwnd, hData);
            } else if (LOWORD(wParam) == ID_BTN_GENERATE) {
                GeneratePassword(hData);
            } else if (LOWORD(wParam) == ID_BTN_INSERT_TEMPLATE) {
                int sel = (int)SendMessage(GetDlgItem(hwnd, ID_COMBO_TEMPLATE), CB_GETCURSEL, 0, 0);
                const char* tpl = "";
                if (sel == 0) tpl = "\r\n--- Login ---\r\nURL: \r\nUsername: \r\nPassword: \r\n-------------\r\n";
                else if (sel == 1) tpl = "\r\n--- Finance ---\r\nBank: \r\nAccount: \r\nRouting: \r\nPIN: \r\n---------------\r\n";
                else if (sel == 2) tpl = "\r\n--- Secure Note ---\r\nTitle: \r\nNote: \r\n-------------------\r\n";
                SendMessage(hData, EM_REPLACESEL, TRUE, (LPARAM)tpl);
                SetFocus(hData);
            } else if (LOWORD(wParam) == ID_BTN_COPY_DATA) {
                int textLen = GetWindowTextLengthA(hData);
                if (textLen > 0) {
                    char* text = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, textLen + 1);
                    if (text) {
                        GetWindowTextA(hData, text, textLen + 1);
                        if (OpenClipboard(hwnd)) {
                            EmptyClipboard();
                            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, textLen + 1);
                            if (hMem) {
                                void* pMem = GlobalLock(hMem);
                                if (pMem) {
                                    my_memcpy(pMem, text, textLen + 1);
                                    GlobalUnlock(hMem);
                                    SetClipboardData(CF_TEXT, hMem);
                                }
                            }
                            CloseClipboard();
                            MessageBoxA(hwnd, "Data copied to clipboard.", "KVault", MB_OK | MB_ICONINFORMATION);
                        }
                        secure_zero(text, textLen + 1);
                        HeapFree(GetProcessHeap(), 0, text);
                    }
                }
            } else if (LOWORD(wParam) == ID_BTN_CLEAR_CLIP) {
                if (OpenClipboard(hwnd)) {
                    EmptyClipboard();
                    CloseClipboard();
                    MessageBoxA(hwnd, "Clipboard cleared.", "KVault", MB_OK | MB_ICONINFORMATION);
                }
            } else if (LOWORD(wParam) == ID_BTN_HELP) {
                MessageBoxA(hwnd, 
                    "KVault Help & Security Guide\n\n"
                    "1. Master Key: Used to encrypt/decrypt data. Never stored anywhere.\n"
                    "2. Encryption: Standard symmetric stream cipher for lightweight offline storage.\n"
                    "3. Auto-Lock: Automatically wipes displayed text and clipboard on inactivity.\n"
                    "4. Shortcuts: Ctrl+S to save, Ctrl+L to lock vault instantly.\n"
                    "5. Drag & Drop: Drop a backup or text file to load contents.\n"
                    "6. Clear Clip: Always clear clipboard after copying passwords.",
                    "KVault Help", MB_OK | MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == ID_BTN_FIND) {
                char findText[256];
                GetDlgItemTextA(hwnd, ID_EDIT_FIND, findText, sizeof(findText));
                if (my_strlen(findText) > 0) {
                    int textLen = GetWindowTextLengthA(hData);
                    if (textLen > 0) {
                        char* text = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, textLen + 1);
                        if (text) {
                            GetWindowTextA(hData, text, textLen + 1);
                            DWORD startSel = 0, endSel = 0;
                            SendMessage(hData, EM_GETSEL, (WPARAM)&startSel, (LPARAM)&endSel);
                            
                            const char* pos = my_strstr_ic(text + endSel, findText);
                            if (!pos && endSel > 0) {
                                pos = my_strstr_ic(text, findText);
                            }
                            
                            if (pos) {
                                int index = (int)(pos - text);
                                SendMessage(hData, EM_SETSEL, index, index + (int)my_strlen(findText));
                                SendMessage(hData, EM_SCROLLCARET, 0, 0);
                                SetFocus(hData);
                            } else {
                                MessageBoxA(hwnd, "Text not found.", "Find", MB_OK | MB_ICONINFORMATION);
                            }
                            HeapFree(GetProcessHeap(), 0, text);
                        }
                    }
                }
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hStatic = (HWND)lParam;
            SetBkMode(hdc, TRANSPARENT);
            if (GetDlgCtrlID(hStatic) == ID_STATIC_STRENGTH) {
                char text[64];
                GetWindowTextA(hStatic, text, sizeof(text));
                if (my_strstr(text, "Weak")) SetTextColor(hdc, RGB(255, 71, 87));
                else if (my_strstr(text, "Medium")) SetTextColor(hdc, RGB(255, 165, 2));
                else if (my_strstr(text, "Strong")) SetTextColor(hdc, RGB(46, 213, 115));
                else SetTextColor(hdc, g_theme == 1 ? RGB(10, 10, 10) : (g_theme == 2 ? RGB(0, 255, 204) : RGB(240, 240, 245)));
                return (LRESULT)hbgBrush;
            }
            SetTextColor(hdc, g_theme == 1 ? RGB(10, 10, 10) : (g_theme == 2 ? RGB(0, 255, 204) : RGB(240, 240, 245)));
            return (LRESULT)hbgBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, g_theme == 1 ? RGB(10, 10, 10) : (g_theme == 2 ? RGB(0, 255, 204) : RGB(240, 240, 245)));
            SetBkColor(hdc, g_theme == 1 ? RGB(255, 255, 255) : (g_theme == 2 ? RGB(10, 15, 10) : RGB(25, 25, 30)));
            return (LRESULT)hDarkBrush;
        }
        case WM_DESTROY: {
            if (hbgBrush) DeleteObject(hbgBrush);
            if (hDarkBrush) DeleteObject(hDarkBrush);
            if (hFont) DeleteObject(hFont);
            if (hTitleFont) DeleteObject(hTitleFont);
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void __stdcall MainEntry() {
    WNDCLASSA wc;
    my_memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KVaultClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(15, 15, 19));
    
    RegisterClassA(&wc);
    
    HWND hwnd = CreateWindowExA(0, "KVaultClass", "KVault Security & Vault Manager", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 610, 435, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    g_lastActivity = GetTickCount();
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) {
            g_lastActivity = GetTickCount();
            if (msg.message == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000)) {
                if (msg.wParam == 'S') {
                    SendMessage(hwnd, WM_COMMAND, ID_BTN_SAVE, 0);
                    continue;
                } else if (msg.wParam == 'L') {
                    ClearVault(hwnd);
                    MessageBoxA(hwnd, "Vault locked (Ctrl+L).", "KVault", MB_OK | MB_ICONINFORMATION);
                    continue;
                }
            }
        }
        if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) {
            g_lastActivity = GetTickCount();
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
