#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <wincrypt.h>

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
#define ID_BTN_QUICKSAVE 119
#define ID_BTN_QUICKLOAD 120

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
    BYTE randBytes[16];
    
    HCRYPTPROV hProv = 0;
    if (CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CryptGenRandom(hProv, len, randBytes);
        CryptReleaseContext(hProv, 0);
    } else {
        for (int i = 0; i < len; i++) {
            randBytes[i] = (BYTE)(my_rand() & 0xFF);
        }
    }
    
    for (int i = 0; i < len; i++) {
        pass[i] = chars[randBytes[i] % charCount];
    }
    pass[len] = '\0';
    
    SendMessage(hTextEdit, EM_REPLACESEL, TRUE, (LPARAM)pass);
    SetFocus(hTextEdit);

    secure_zero(randBytes, sizeof(randBytes));
    secure_zero(pass, sizeof(pass));
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
            if (fileSize != INVALID_FILE_SIZE && fileSize > 0 && fileSize < 10485760) {
                char* buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
                if (buffer) {
                    DWORD bytesRead = 0;
                    if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                        buffer[bytesRead] = '\0';
                        SetWindowTextA(hTextEdit, buffer);
                    }
                    secure_zero(buffer, fileSize + 1);
                    HeapFree(GetProcessHeap(), 0, buffer);
                }
            } else if (fileSize == 0) {
                SetWindowTextA(hTextEdit, "");
            } else {
                MessageBoxA(hwnd, "File size exceeds 10MB limit.", "KVault", MB_OK | MB_ICONWARNING);
            }
            CloseHandle(hFile);
        } else {
            MessageBoxA(hwnd, "Could not open selected file.", "KVault", MB_OK | MB_ICONERROR);
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
    ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.txt\0JSON Files\0*.json\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileNameA(&ofn) == TRUE) {
        HANDLE hFile = CreateFileA(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            int textLen = GetWindowTextLengthA(hTextEdit);
            BOOL writeOk = TRUE;
            if (textLen > 0) {
                char* buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, textLen + 1);
                if (buffer) {
                    GetWindowTextA(hTextEdit, buffer, textLen + 1);
                    DWORD bytesWritten = 0;
                    if (!WriteFile(hFile, buffer, textLen, &bytesWritten, NULL) || bytesWritten != (DWORD)textLen) {
                        writeOk = FALSE;
                    }
                    secure_zero(buffer, textLen + 1);
                    HeapFree(GetProcessHeap(), 0, buffer);
                } else {
                    writeOk = FALSE;
                }
            }
            CloseHandle(hFile);
            if (writeOk) {
                MessageBoxA(hwnd, "Vault data saved successfully.", "KVault", MB_OK | MB_ICONINFORMATION);
            } else {
                MessageBoxA(hwnd, "Failed to write complete file data.", "KVault", MB_OK | MB_ICONERROR);
            }
        } else {
            MessageBoxA(hwnd, "Could not create or write destination file.", "KVault", MB_OK | MB_ICONERROR);
        }
    }
}

void ClearVault(HWND hwnd) {
    SetWindowTextA(hData, "");
    SetWindowTextA(hPass, "");
    SetWindowTextA(GetDlgItem(hwnd, ID_STATIC_STRENGTH), "");
    SetWindowTextA(GetDlgItem(hwnd, ID_EDIT_FIND), "");
    InvalidateRect(GetDlgItem(hwnd, ID_STATIC_STRENGTH), NULL, TRUE);
    if (OpenClipboard(hwnd)) {
        EmptyClipboard();
        CloseClipboard();
    }
}

int HasSavedState(const char* filename) {
    DWORD attr = GetFileAttributesA(filename);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

int HasSeenTutorial(void) {
    DWORD attr = GetFileAttributesA("kvault_tutorial.dat");
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

void MarkTutorialSeen(void) {
    HANDLE h = CreateFileA("kvault_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        char tag[] = "TUTORIAL_SEEN_V1\r\n";
        DWORD written = 0;
        WriteFile(h, tag, sizeof(tag) - 1, &written, NULL);
        CloseHandle(h);
    }
}

int SaveSnapshot(HWND hwnd) {
    HANDLE hFile = CreateFileA("kvault.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0;

    char magic[4] = {'K', 'V', 'L', 'T'};
    DWORD version = 1;
    DWORD timeout = (DWORD)g_timeoutMs;
    DWORD theme = (DWORD)g_theme;
    DWORD passLen = (DWORD)GetWindowTextLengthA(hPass);
    DWORD dataLen = (DWORD)GetWindowTextLengthA(hData);
    DWORD written = 0;

    WriteFile(hFile, magic, 4, &written, NULL);
    WriteFile(hFile, &version, sizeof(DWORD), &written, NULL);
    WriteFile(hFile, &timeout, sizeof(DWORD), &written, NULL);
    WriteFile(hFile, &theme, sizeof(DWORD), &written, NULL);
    WriteFile(hFile, &passLen, sizeof(DWORD), &written, NULL);

    if (passLen > 0) {
        char* passBuf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, passLen + 1);
        if (passBuf) {
            GetWindowTextA(hPass, passBuf, passLen + 1);
            WriteFile(hFile, passBuf, passLen, &written, NULL);
            secure_zero(passBuf, passLen + 1);
            HeapFree(GetProcessHeap(), 0, passBuf);
        }
    }

    WriteFile(hFile, &dataLen, sizeof(DWORD), &written, NULL);
    if (dataLen > 0) {
        char* dataBuf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dataLen + 1);
        if (dataBuf) {
            GetWindowTextA(hData, dataBuf, dataLen + 1);
            WriteFile(hFile, dataBuf, dataLen, &written, NULL);
            secure_zero(dataBuf, dataLen + 1);
            HeapFree(GetProcessHeap(), 0, dataBuf);
        }
    }

    CloseHandle(hFile);
    return 1;
}

int LoadSnapshot(HWND hwnd, int showFeedback) {
    HANDLE hFile = CreateFileA("kvault.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        if (showFeedback) {
            MessageBoxA(hwnd, "No quicksave snapshot found (kvault.dat).\nPress F5 to create a snapshot.", "KVault", MB_OK | MB_ICONWARNING);
        }
        return 0;
    }

    char magic[4] = {0};
    DWORD version = 0, timeout = 0, theme = 0, passLen = 0, dataLen = 0;
    DWORD readBytes = 0;

    if (!ReadFile(hFile, magic, 4, &readBytes, NULL) || readBytes != 4 ||
        magic[0] != 'K' || magic[1] != 'V' || magic[2] != 'L' || magic[3] != 'T') {
        CloseHandle(hFile);
        if (showFeedback) MessageBoxA(hwnd, "Corrupted quicksave data.", "KVault", MB_OK | MB_ICONERROR);
        return 0;
    }

    ReadFile(hFile, &version, sizeof(DWORD), &readBytes, NULL);
    ReadFile(hFile, &timeout, sizeof(DWORD), &readBytes, NULL);
    ReadFile(hFile, &theme, sizeof(DWORD), &readBytes, NULL);
    ReadFile(hFile, &passLen, sizeof(DWORD), &readBytes, NULL);

    g_timeoutMs = (int)timeout;
    HWND hComboTimeout = GetDlgItem(hwnd, ID_COMBO_TIMEOUT);
    if (hComboTimeout) {
        if (g_timeoutMs == 60000) SendMessage(hComboTimeout, CB_SETCURSEL, 0, 0);
        else if (g_timeoutMs == 300000) SendMessage(hComboTimeout, CB_SETCURSEL, 1, 0);
        else if (g_timeoutMs == 900000) SendMessage(hComboTimeout, CB_SETCURSEL, 2, 0);
        else if (g_timeoutMs == 0) SendMessage(hComboTimeout, CB_SETCURSEL, 3, 0);
    }

    g_theme = (int)theme;
    HWND hComboTheme = GetDlgItem(hwnd, ID_COMBO_THEME);
    if (hComboTheme) {
        SendMessage(hComboTheme, CB_SETCURSEL, g_theme, 0);
    }
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

    if (passLen > 0 && passLen < 1024) {
        char* passBuf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, passLen + 1);
        if (passBuf) {
            ReadFile(hFile, passBuf, passLen, &readBytes, NULL);
            passBuf[readBytes] = '\0';
            SetWindowTextA(hPass, passBuf);
            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_EDIT_PASS, EN_CHANGE), (LPARAM)hPass);
            secure_zero(passBuf, passLen + 1);
            HeapFree(GetProcessHeap(), 0, passBuf);
        }
    } else {
        SetWindowTextA(hPass, "");
        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_EDIT_PASS, EN_CHANGE), (LPARAM)hPass);
    }

    ReadFile(hFile, &dataLen, sizeof(DWORD), &readBytes, NULL);
    if (dataLen > 0 && dataLen < 10485760) {
        char* dataBuf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dataLen + 1);
        if (dataBuf) {
            ReadFile(hFile, dataBuf, dataLen, &readBytes, NULL);
            dataBuf[readBytes] = '\0';
            SetWindowTextA(hData, dataBuf);
            secure_zero(dataBuf, dataLen + 1);
            HeapFree(GetProcessHeap(), 0, dataBuf);
        }
    } else {
        SetWindowTextA(hData, "");
    }

    CloseHandle(hFile);
    InvalidateRect(hwnd, NULL, TRUE);
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);

    if (showFeedback) {
        MessageBoxA(hwnd, "Workspace state restored from kvault.dat [F9]", "KVault", MB_OK | MB_ICONINFORMATION);
    }
    return 1;
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
            
            HWND hComboTimeout = CreateWindowA("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 385, 20, 95, 100, hwnd, (HMENU)ID_COMBO_TIMEOUT, NULL, NULL);
            SendMessage(hComboTimeout, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hComboTimeout, CB_ADDSTRING, 0, (LPARAM)"1 min");
            SendMessage(hComboTimeout, CB_ADDSTRING, 0, (LPARAM)"5 min");
            SendMessage(hComboTimeout, CB_ADDSTRING, 0, (LPARAM)"15 min");
            SendMessage(hComboTimeout, CB_ADDSTRING, 0, (LPARAM)"Never");
            SendMessage(hComboTimeout, CB_SETCURSEL, 0, 0);
            
            HWND hComboTheme = CreateWindowA("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 490, 20, 90, 100, hwnd, (HMENU)ID_COMBO_THEME, NULL, NULL);
            SendMessage(hComboTheme, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hComboTheme, CB_ADDSTRING, 0, (LPARAM)"Dark");
            SendMessage(hComboTheme, CB_ADDSTRING, 0, (LPARAM)"Light");
            SendMessage(hComboTheme, CB_ADDSTRING, 0, (LPARAM)"Neon");
            SendMessage(hComboTheme, CB_SETCURSEL, 0, 0);
            
            SetTimer(hwnd, 1, 1000, NULL);
            
            HWND hStat = CreateWindowA("STATIC", "Master Key:", WS_VISIBLE | WS_CHILD, 15, 62, 80, 25, hwnd, NULL, NULL, NULL);
            SendMessage(hStat, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hPass = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL | WS_TABSTOP, 100, 60, 200, 25, hwnd, (HMENU)ID_EDIT_PASS, NULL, NULL);
            SendMessage(hPass, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hPass, EM_SETCUEBANNER, 0, (LPARAM)L"Enter Master Password");
            
            HWND hStrength = CreateWindowA("STATIC", "", WS_VISIBLE | WS_CHILD, 100, 85, 200, 15, hwnd, (HMENU)ID_STATIC_STRENGTH, NULL, NULL);
            SendMessage(hStrength, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnEnc = CreateWindowA("BUTTON", "Encrypt", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 315, 60, 85, 25, hwnd, (HMENU)ID_BTN_ENCRYPT, NULL, NULL);
            HWND hBtnDec = CreateWindowA("BUTTON", "Decrypt", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 410, 60, 85, 25, hwnd, (HMENU)ID_BTN_DECRYPT, NULL, NULL);
            HWND hBtnClr = CreateWindowA("BUTTON", "Clear", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 505, 60, 75, 25, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
            
            SendMessage(hBtnEnc, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnDec, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnClr, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hData = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_TABSTOP, 15, 105, 565, 205, hwnd, (HMENU)ID_EDIT_DATA, NULL, NULL);
            SendMessage(hData, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hData, EM_LIMITTEXT, 0x100000, 0);
            SendMessage(hData, EM_SETCUEBANNER, 0, (LPARAM)L"Enter or load confidential data here...");
            
            HWND hBtnLoad = CreateWindowA("BUTTON", "Load File", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 15, 320, 100, 25, hwnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            HWND hBtnSave = CreateWindowA("BUTTON", "Save File", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 125, 320, 100, 25, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            SendMessage(hBtnLoad, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSave, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hFindEdit = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP, 235, 320, 155, 25, hwnd, (HMENU)ID_EDIT_FIND, NULL, NULL);
            SendMessage(hFindEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hFindEdit, EM_SETCUEBANNER, 0, (LPARAM)L"Find text...");
            
            HWND hBtnFind = CreateWindowA("BUTTON", "Find", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 400, 320, 80, 25, hwnd, (HMENU)ID_BTN_FIND, NULL, NULL);
            SendMessage(hBtnFind, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND hBtnGen = CreateWindowA("BUTTON", "Gen Pass", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 490, 320, 90, 25, hwnd, (HMENU)ID_BTN_GENERATE, NULL, NULL);
            SendMessage(hBtnGen, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hComboTpl = CreateWindowA("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 15, 355, 90, 100, hwnd, (HMENU)ID_COMBO_TEMPLATE, NULL, NULL);
            SendMessage(hComboTpl, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hComboTpl, CB_ADDSTRING, 0, (LPARAM)"Login");
            SendMessage(hComboTpl, CB_ADDSTRING, 0, (LPARAM)"Finance");
            SendMessage(hComboTpl, CB_ADDSTRING, 0, (LPARAM)"Note");
            SendMessage(hComboTpl, CB_SETCURSEL, 0, 0);
            
            HWND hBtnTpl = CreateWindowA("BUTTON", "Insert", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 110, 355, 55, 25, hwnd, (HMENU)ID_BTN_INSERT_TEMPLATE, NULL, NULL);
            SendMessage(hBtnTpl, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnCopyData = CreateWindowA("BUTTON", "Copy", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 170, 355, 50, 25, hwnd, (HMENU)ID_BTN_COPY_DATA, NULL, NULL);
            SendMessage(hBtnCopyData, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnClearClip = CreateWindowA("BUTTON", "Clr Clip", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 225, 355, 65, 25, hwnd, (HMENU)ID_BTN_CLEAR_CLIP, NULL, NULL);
            SendMessage(hBtnClearClip, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            HWND hBtnHelp = CreateWindowA("BUTTON", "Help [F1]", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 295, 355, 80, 25, hwnd, (HMENU)ID_BTN_HELP, NULL, NULL);
            SendMessage(hBtnHelp, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hBtnQuickSave = CreateWindowA("BUTTON", "Save [F5]", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 380, 355, 95, 25, hwnd, (HMENU)ID_BTN_QUICKSAVE, NULL, NULL);
            SendMessage(hBtnQuickSave, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hBtnQuickLoad = CreateWindowA("BUTTON", "Load [F9]", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_TABSTOP, 480, 355, 95, 25, hwnd, (HMENU)ID_BTN_QUICKLOAD, NULL, NULL);
            SendMessage(hBtnQuickLoad, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            LARGE_INTEGER pc;
            QueryPerformanceCounter(&pc);
            my_srand((unsigned int)GetTickCount() ^ (unsigned int)pc.LowPart);
            
            SetClassLongPtrA(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbgBrush);
            DragAcceptFiles(hwnd, TRUE);

            if (HasSavedState("kvault.dat")) {
                LoadSnapshot(hwnd, 0);
            } else if (!HasSeenTutorial()) {
                MessageBoxA(hwnd,
                    "Welcome to KVault (Pass 5 Verified)\n\n"
                    "Your client-side encrypted password & secret manager.\n\n"
                    "- Enter a Master Key to encrypt/decrypt sensitive text.\n"
                    "- Press F5 at any time to quicksave snapshot to kvault.dat.\n"
                    "- Press F9 to restore your quicksaved workspace snapshot.\n"
                    "- Press Ctrl+L to lock and wipe clipboard immediately.\n"
                    "- Press F1 for complete hotkey list & security guide.",
                    "KVault - Quickstart Guide", MB_OK | MB_ICONINFORMATION);
                MarkTutorialSeen();
            }
            break;
        }
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            char szFile[260];
            if (DragQueryFileA(hDrop, 0, szFile, sizeof(szFile))) {
                HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD fileSize = GetFileSize(hFile, NULL);
                    if (fileSize != INVALID_FILE_SIZE && fileSize > 0 && fileSize < 10485760) {
                        char* buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
                        if (buffer) {
                            DWORD bytesRead = 0;
                            if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                                buffer[bytesRead] = '\0';
                                SetWindowTextA(hData, buffer);
                            }
                            secure_zero(buffer, fileSize + 1);
                            HeapFree(GetProcessHeap(), 0, buffer);
                        }
                    } else if (fileSize == 0) {
                        SetWindowTextA(hData, "");
                    } else {
                        MessageBoxA(hwnd, "File size exceeds 10MB limit.", "KVault", MB_OK | MB_ICONWARNING);
                    }
                    CloseHandle(hFile);
                } else {
                    MessageBoxA(hwnd, "Could not open dropped file.", "KVault", MB_OK | MB_ICONERROR);
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
                if (textLen == 0) {
                    MessageBoxA(hwnd, "Vault data is empty.", "KVault", MB_OK | MB_ICONINFORMATION);
                } else {
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
                                    if (SetClipboardData(CF_TEXT, hMem)) {
                                        hMem = NULL;
                                    }
                                }
                                if (hMem) GlobalFree(hMem);
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
            } else if (LOWORD(wParam) == ID_BTN_QUICKSAVE) {
                if (SaveSnapshot(hwnd)) {
                    MessageBoxA(hwnd, "Workspace state quicksaved to kvault.dat [F5].", "KVault", MB_OK | MB_ICONINFORMATION);
                } else {
                    MessageBoxA(hwnd, "Failed to write quicksave snapshot.", "KVault", MB_OK | MB_ICONERROR);
                }
            } else if (LOWORD(wParam) == ID_BTN_QUICKLOAD) {
                LoadSnapshot(hwnd, 1);
            } else if (LOWORD(wParam) == ID_BTN_HELP) {
                MessageBoxA(hwnd, 
                    "KVault Help & Security Guide\n\n"
                    "1. Master Key: Encrypts/decrypts data with stream cipher.\n"
                    "2. Quicksave & Restore:\n"
                    "   F5: Quicksave workspace snapshot to kvault.dat\n"
                    "   F9: Quickload restored snapshot from kvault.dat\n"
                    "3. Auto-Lock: Wipes displayed text and clipboard on inactivity.\n"
                    "4. Shortcuts:\n"
                    "   F5: Quicksave snapshot\n"
                    "   F9: Quickload snapshot\n"
                    "   Ctrl+S: Save file | Ctrl+O: Open file\n"
                    "   Ctrl+L: Lock vault | Ctrl+E: Encrypt data\n"
                    "   Ctrl+D: Decrypt data | Ctrl+G: Gen password\n"
                    "   Ctrl+F: Focus Find | F1: Help\n"
                    "5. Drag & Drop: Drop file to load contents into editor.\n"
                    "6. Clipboard: Clear Clip wipes clipboard after use.",
                    "KVault Help", MB_OK | MB_ICONINFORMATION);
            } else if (LOWORD(wParam) == ID_BTN_FIND) {
                char findText[256];
                GetDlgItemTextA(hwnd, ID_EDIT_FIND, findText, sizeof(findText));
                if (my_strlen(findText) == 0) {
                    MessageBoxA(hwnd, "Please enter search text first.", "KVault", MB_OK | MB_ICONINFORMATION);
                } else {
                    int textLen = GetWindowTextLengthA(hData);
                    if (textLen == 0) {
                        MessageBoxA(hwnd, "Vault data is empty.", "KVault", MB_OK | MB_ICONINFORMATION);
                    } else {
                        char* text = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, textLen + 1);
                        if (text) {
                            GetWindowTextA(hData, text, textLen + 1);
                            DWORD startSel = 0, endSel = 0;
                            SendMessage(hData, EM_GETSEL, (WPARAM)&startSel, (LPARAM)&endSel);
                            if (endSel > (DWORD)textLen) endSel = (DWORD)textLen;
                            
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
                            secure_zero(text, textLen + 1);
                            HeapFree(GetProcessHeap(), 0, text);
                        }
                    }
                }
                secure_zero(findText, sizeof(findText));
            }
            break;
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, hbgBrush);
            return 1;
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
            KillTimer(hwnd, 1);
            ClearVault(hwnd);
            SetClassLongPtrA(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)NULL);
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
    wc.hbrBackground = NULL;
    
    RegisterClassA(&wc);
    
    HWND hwnd = CreateWindowExA(0, "KVaultClass", "KVault Security & Vault Manager", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 610, 435, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    g_lastActivity = GetTickCount();
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) {
            g_lastActivity = GetTickCount();
            if (msg.message == WM_KEYDOWN) {
                BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                BOOL bShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                BOOL bAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;

                if (bCtrl && !bShift && !bAlt) {
                    if (msg.wParam == 'S') {
                        SendMessage(hwnd, WM_COMMAND, ID_BTN_SAVE, 0);
                        continue;
                    } else if (msg.wParam == 'L') {
                        ClearVault(hwnd);
                        MessageBoxA(hwnd, "Vault locked (Ctrl+L).", "KVault", MB_OK | MB_ICONINFORMATION);
                        continue;
                    } else if (msg.wParam == 'O') {
                        SendMessage(hwnd, WM_COMMAND, ID_BTN_LOAD, 0);
                        continue;
                    } else if (msg.wParam == 'E') {
                        SendMessage(hwnd, WM_COMMAND, ID_BTN_ENCRYPT, 0);
                        continue;
                    } else if (msg.wParam == 'D') {
                        SendMessage(hwnd, WM_COMMAND, ID_BTN_DECRYPT, 0);
                        continue;
                    } else if (msg.wParam == 'G') {
                        SendMessage(hwnd, WM_COMMAND, ID_BTN_GENERATE, 0);
                        continue;
                    } else if (msg.wParam == 'F') {
                        SetFocus(GetDlgItem(hwnd, ID_EDIT_FIND));
                        continue;
                    }
                } else if (!bCtrl && !bShift && !bAlt) {
                    if (msg.wParam == VK_F1) {
                        SendMessage(hwnd, WM_COMMAND, ID_BTN_HELP, 0);
                        continue;
                    } else if (msg.wParam == VK_F5) {
                        SendMessage(hwnd, WM_COMMAND, ID_BTN_QUICKSAVE, 0);
                        continue;
                    } else if (msg.wParam == VK_F9) {
                        SendMessage(hwnd, WM_COMMAND, ID_BTN_QUICKLOAD, 0);
                        continue;
                    } else if (msg.wParam == VK_RETURN) {
                        if (msg.hwnd == GetDlgItem(hwnd, ID_EDIT_FIND)) {
                            SendMessage(hwnd, WM_COMMAND, ID_BTN_FIND, 0);
                            continue;
                        } else if (msg.hwnd == hPass) {
                            SendMessage(hwnd, WM_COMMAND, ID_BTN_ENCRYPT, 0);
                            continue;
                        }
                    }
                }
            }
        }
        if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) {
            g_lastActivity = GetTickCount();
        }
        if (!IsDialogMessageA(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    ExitProcess(0);
}
