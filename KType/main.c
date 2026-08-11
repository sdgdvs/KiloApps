#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#define W 1000
#define H 800

// --- Word Lists ---
const char* commonWords[] = {
    "hello", "world", "kilo", "system", "typing",
    "speed", "code", "native", "win32", "game",
    "fast", "keyboard", "monitor", "software", "mouse",
    "hardware", "interface", "compiler", "linker", "binary",
    "execute", "process", "thread", "memory", "storage",
    "network", "server", "client", "packet", "router"
};
int numCommonWords = 30;

const char* codeWords[] = {
    "function", "return", "windows.h", "typedef", "struct",
    "createfont", "selectobject", "invalidaterect", "wndproc", "hinstance",
    "sendmessage", "createwindow", "postquitmessage", "bitblt", "freelibrary",
    "regopenkey", "fillrect", "selectobject", "dispatchmessage", "registerclass"
};
int numCodeWords = 20;

// --- Modes ---
// 0: Arcade Cascade, 1: Timed Speed Test (30s), 2: Code Snippets, 3: Error Heatmap, 4: Help Screen
int currentMode = 4;

// --- Arcade State ---
#define MAX_FALLING 6
typedef struct {
    int active;
    int wordIdx;
    int y;
    int x;
    int matchLen;
} FallingWord;

FallingWord fWords[MAX_FALLING];
int arcadeScore = 0;
int arcadeLives = 3;
int arcadeCombo = 0;
int targetWord = -1;

// --- Speed Test State ---
int testActive = 0;
DWORD testStartTime = 0;
int testDuration = 30;
int testWordIndex = 0;
int testCharIndex = 0;
int totalTyped = 0;
int correctTyped = 0;
int testErrors = 0;
int testWordPool[40];

// --- Key Heatmap & Finger Stats ---
int keyHits[26];
int keyErrors[26];

// --- Persistent High Scores ---
DWORD highArcadeScore = 0;
DWORD bestWPM = 0;

DWORD lastKeyTime = 0;
DWORD currentCadence = 0;

// --- Helper Functions ---
int randSeed = 12345;
int MyRand() {
    randSeed = randSeed * 1103515245 + 12345;
    return (unsigned int)(randSeed / 65536) % 32768;
}

int StrLen(const char* s) {
    int c = 0;
    while (s && s[c]) c++;
    return c;
}

const char* my_strstr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;
    if (!*needle) return haystack;
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return haystack;
        haystack++;
    }
    return NULL;
}

const char* my_strrchr(const char* s, int c) {
    const char* last = NULL;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    return last;
}

void IntToStr(int val, char* buf) {
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    char temp[16];
    int i = 0, j = 0;
    if (val < 0) { buf[j++] = '-'; val = -val; }
    while (val > 0) { temp[i++] = (val % 10) + '0'; val /= 10; }
    while (i > 0) buf[j++] = temp[--i];
    buf[j] = '\0';
}

void SaveRegistryData() {
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\KiloApps\\KType", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "HighArcadeScore", 0, REG_DWORD, (BYTE*)&highArcadeScore, sizeof(DWORD));
        RegSetValueExA(hKey, "BestWPM", 0, REG_DWORD, (BYTE*)&bestWPM, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

void LoadRegistryData() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\KiloApps\\KType", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD sz = sizeof(DWORD);
        RegQueryValueExA(hKey, "HighArcadeScore", NULL, NULL, (BYTE*)&highArcadeScore, &sz);
        sz = sizeof(DWORD);
        RegQueryValueExA(hKey, "BestWPM", NULL, NULL, (BYTE*)&bestWPM, &sz);
        RegCloseKey(hKey);
    }
}

void SpawnArcadeWord() {
    int i;
    for (i = 0; i < MAX_FALLING; i++) {
        if (!fWords[i].active) {
            fWords[i].active = 1;
            fWords[i].wordIdx = MyRand() % numCommonWords;
            fWords[i].y = -20;
            fWords[i].x = 30 + (MyRand() % (W - 140));
            fWords[i].matchLen = 0;
            return;
        }
    }
}

void ResetSpeedTest() {
    testActive = 0;
    testStartTime = 0;
    testWordIndex = 0;
    testCharIndex = 0;
    totalTyped = 0;
    correctTyped = 0;
    testErrors = 0;

    int i;
    int count = (currentMode == 2) ? numCodeWords : numCommonWords;
    for (i = 0; i < 40; i++) {
        testWordPool[i] = MyRand() % count;
    }
}

void StartSpeedTest() {
    testActive = 1;
    testStartTime = GetTickCount();
    lastKeyTime = GetTickCount();
}

// --- Audio Thread ---
DWORD WINAPI SoundThread(LPVOID lpParam) {
    DWORD params = (DWORD)(UINT_PTR)lpParam;
    DWORD freq = params & 0xFFFF;
    DWORD dur = params >> 16;
    Beep(freq, dur);
    return 0;
}
void AsyncBeep(DWORD freq, DWORD dur) {
    DWORD param = freq | (dur << 16);
    CreateThread(NULL, 0, SoundThread, (LPVOID)(UINT_PTR)param, 0, NULL);
}

// --- Heatmap Export ---
void ExportHeatmapBMP(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 800, 400);
    HBITMAP oldBM = (HBITMAP)SelectObject(memDC, hBitmap);
    
    HBRUSH bg = CreateSolidBrush(RGB(13, 15, 24));
    RECT full = {0, 0, 800, 400};
    FillRect(memDC, &full, bg);
    DeleteObject(bg);
    
    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, RGB(0, 242, 254));
    TextOutA(memDC, 30, 50, "Finger Weakness & Key Heatmap Analysis", 38);
    const char* r1 = "QWERTYUIOP"; const char* r2 = "ASDFGHJKL"; const char* r3 = "ZXCVBNM";
    int startY = 120; int kSize = 40; int i;
    for (i = 0; i < 10; i++) {
        int kIdx = r1[i] - 'A'; int errs = keyErrors[kIdx];
        HBRUSH kBrush = (errs > 3) ? CreateSolidBrush(RGB(239, 68, 68)) : (errs > 0) ? CreateSolidBrush(RGB(245, 158, 11)) : CreateSolidBrush(RGB(30, 41, 59));
        RECT kRect = {40 + i*48, startY, 40 + i*48 + kSize, startY + kSize}; FillRect(memDC, &kRect, kBrush); DeleteObject(kBrush);
        char label[2] = {r1[i], '\0'}; SetTextColor(memDC, RGB(248, 250, 252)); TextOutA(memDC, 52 + i*48, startY + 8, label, 1);
    }
    for (i = 0; i < 9; i++) {
        int kIdx = r2[i] - 'A'; int errs = keyErrors[kIdx];
        HBRUSH kBrush = (errs > 3) ? CreateSolidBrush(RGB(239, 68, 68)) : (errs > 0) ? CreateSolidBrush(RGB(245, 158, 11)) : CreateSolidBrush(RGB(30, 41, 59));
        RECT kRect = {60 + i*48, startY + 52, 60 + i*48 + kSize, startY + 52 + kSize}; FillRect(memDC, &kRect, kBrush); DeleteObject(kBrush);
        char label[2] = {r2[i], '\0'}; SetTextColor(memDC, RGB(248, 250, 252)); TextOutA(memDC, 72 + i*48, startY + 60, label, 1);
    }
    for (i = 0; i < 7; i++) {
        int kIdx = r3[i] - 'A'; int errs = keyErrors[kIdx];
        HBRUSH kBrush = (errs > 3) ? CreateSolidBrush(RGB(239, 68, 68)) : (errs > 0) ? CreateSolidBrush(RGB(245, 158, 11)) : CreateSolidBrush(RGB(30, 41, 59));
        RECT kRect = {90 + i*48, startY + 104, 90 + i*48 + kSize, startY + 104 + kSize}; FillRect(memDC, &kRect, kBrush); DeleteObject(kBrush);
        char label[2] = {r3[i], '\0'}; SetTextColor(memDC, RGB(248, 250, 252)); TextOutA(memDC, 102 + i*48, startY + 112, label, 1);
    }
    SetTextColor(memDC, RGB(148, 163, 184));
    TextOutA(memDC, 40, 360, "Legend: Dark = Perfect | Amber = 1-3 Mistypes | Red = >3 Mistypes", 64);
    
    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = 800;
    bi.biHeight = 400; 
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    
    DWORD dataSize = ((800 * 24 + 31) / 32) * 4 * 400;
    BYTE* pixels = (BYTE*)HeapAlloc(GetProcessHeap(), 0, dataSize);
    GetDIBits(hdc, hBitmap, 0, 400, pixels, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    
    HANDLE hFile = CreateFileA("heatmap.bmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        BITMAPFILEHEADER bmf = {0};
        bmf.bfType = 0x4D42;
        bmf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dataSize;
        bmf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        DWORD written;
        WriteFile(hFile, &bmf, sizeof(bmf), &written, NULL);
        WriteFile(hFile, &bi, sizeof(bi), &written, NULL);
        WriteFile(hFile, pixels, dataSize, &written, NULL);
        CloseHandle(hFile);
    }
    
    HeapFree(GetProcessHeap(), 0, pixels);
    SelectObject(memDC, oldBM);
    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(hwnd, hdc);
    MessageBoxA(hwnd, "Heatmap exported to heatmap.bmp", "Export Success", MB_OK);
}

// --- Certificate Export ---
void ExportCertificateBMP(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 800, 600);
    HBITMAP oldBM = (HBITMAP)SelectObject(memDC, hBitmap);
    
    HBRUSH bg = CreateSolidBrush(RGB(11, 13, 20));
    RECT full = {0, 0, 800, 600};
    FillRect(memDC, &full, bg);
    DeleteObject(bg);
    
    HBRUSH border1 = CreateSolidBrush(RGB(0, 242, 254));
    RECT r1T = {20, 20, 780, 24}; FillRect(memDC, &r1T, border1);
    RECT r1B = {20, 576, 780, 580}; FillRect(memDC, &r1B, border1);
    RECT r1L = {20, 20, 24, 580}; FillRect(memDC, &r1L, border1);
    RECT r1R = {776, 20, 780, 580}; FillRect(memDC, &r1R, border1);
    DeleteObject(border1);
    
    HBRUSH border2 = CreateSolidBrush(RGB(157, 78, 221));
    RECT r2T = {30, 30, 770, 32}; FillRect(memDC, &r2T, border2);
    RECT r2B = {30, 568, 770, 570}; FillRect(memDC, &r2B, border2);
    RECT r2L = {30, 30, 32, 570}; FillRect(memDC, &r2L, border2);
    RECT r2R = {768, 30, 770, 570}; FillRect(memDC, &r2R, border2);
    DeleteObject(border2);

    SetBkMode(memDC, TRANSPARENT);
    
    HFONT fontTitle = CreateFontA(48, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Arial");
    HFONT fontSub = CreateFontA(24, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Arial");
    HFONT fontScore = CreateFontA(80, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");
    HFONT fontNormal = CreateFontA(20, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Arial");

    SetTextColor(memDC, RGB(0, 242, 254));
    SelectObject(memDC, fontTitle);
    TextOutA(memDC, 140, 80, "KType Studio Certificate", 24);
    
    SetTextColor(memDC, RGB(248, 250, 252));
    SelectObject(memDC, fontSub);
    TextOutA(memDC, 250, 150, "Official Typing Assessment", 26);
    
    SetTextColor(memDC, RGB(148, 163, 184));
    SelectObject(memDC, fontNormal);
    TextOutA(memDC, 170, 230, "This certifies that the user achieved top scores:", 49);
    
    char wpmBuf[64] = {0};
    char wpmNum[32] = {0};
    IntToStr(bestWPM, wpmNum);
    int p1 = 0, p2 = 0;
    while(wpmNum[p2]) { wpmBuf[p1++] = wpmNum[p2++]; }
    wpmBuf[p1++] = ' '; wpmBuf[p1++] = 'W'; wpmBuf[p1++] = 'P'; wpmBuf[p1++] = 'M'; wpmBuf[p1] = '\0';
    
    SetTextColor(memDC, RGB(16, 185, 129));
    SelectObject(memDC, fontScore);
    TextOutA(memDC, 280, 320, wpmBuf, StrLen(wpmBuf));
    
    char arcBuf[64] = "High Arcade Score: ";
    char arcNum[32] = {0};
    IntToStr(highArcadeScore, arcNum);
    p1 = StrLen(arcBuf); p2 = 0;
    while(arcNum[p2]) { arcBuf[p1++] = arcNum[p2++]; }
    arcBuf[p1] = '\0';
    
    SetTextColor(memDC, RGB(248, 250, 252));
    SelectObject(memDC, fontNormal);
    TextOutA(memDC, 280, 440, arcBuf, StrLen(arcBuf));
    
    SetTextColor(memDC, RGB(100, 116, 139));
    SelectObject(memDC, fontNormal);
    char dateBuf[64] = "Authorized by KiloOS System";
    TextOutA(memDC, 280, 520, dateBuf, StrLen(dateBuf));

    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = 800;
    bi.biHeight = -600; 
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    
    DWORD dataSize = ((800 * 24 + 31) / 32) * 4 * 600;
    BYTE* pixels = (BYTE*)HeapAlloc(GetProcessHeap(), 0, dataSize);
    GetDIBits(hdc, hBitmap, 0, 600, pixels, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    
    HANDLE hFile = CreateFileA("certificate.bmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        BITMAPFILEHEADER bmf = {0};
        bmf.bfType = 0x4D42;
        bmf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dataSize;
        bmf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        DWORD written;
        WriteFile(hFile, &bmf, sizeof(bmf), &written, NULL);
        WriteFile(hFile, &bi, sizeof(bi), &written, NULL);
        WriteFile(hFile, pixels, dataSize, &written, NULL);
        CloseHandle(hFile);
    }
    
    HeapFree(GetProcessHeap(), 0, pixels);
    DeleteObject(fontTitle);
    DeleteObject(fontSub);
    DeleteObject(fontScore);
    DeleteObject(fontNormal);
    SelectObject(memDC, oldBM);
    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(hwnd, hdc);
    MessageBoxA(hwnd, "Certificate exported to certificate.bmp", "Export Success", MB_OK);
}

// --- Persistent Font Handles ---
HFONT g_fontNav = NULL;
HFONT g_fontMain = NULL;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            randSeed = GetTickCount();
            LoadRegistryData();
            memset(fWords, 0, sizeof(fWords));
            memset(keyHits, 0, sizeof(keyHits));
            memset(keyErrors, 0, sizeof(keyErrors));
            g_fontNav = CreateFontA(16, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            g_fontMain = CreateFontA(24, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");
            SpawnArcadeWord();
            ResetSpeedTest();
            SetTimer(hwnd, 1, 30, NULL);
            DragAcceptFiles(hwnd, TRUE);
            break;

        case WM_TIMER: {
            if (currentMode == 0) { // Arcade Mode
                if (arcadeLives > 0) {
                    int i;
                    int speed = 2 + (arcadeScore / 100);
                    for (i = 0; i < MAX_FALLING; i++) {
                        if (fWords[i].active) {
                            fWords[i].y += speed;
                            if (fWords[i].y > H - 80) {
                                fWords[i].active = 0;
                                arcadeLives--;
                                if (targetWord == i) targetWord = -1;
                                arcadeCombo = 0;
                            }
                        }
                    }
                    if ((MyRand() % 100) < (2 + arcadeScore / 200)) {
                        SpawnArcadeWord();
                    }
                }
            } else if (currentMode == 1 || currentMode == 2) { // Speed Test Modes
                if (testActive) {
                    DWORD elapsedSec = (GetTickCount() - testStartTime) / 1000;
                    if (elapsedSec >= (DWORD)testDuration) {
                        testActive = 0;
                        DWORD wpm = 0;
                        if (elapsedSec > 0) {
                            wpm = (correctTyped / 5) * 60 / elapsedSec;
                        }
                        if (wpm > bestWPM) {
                            bestWPM = wpm;
                            SaveRegistryData();
                        }
                    }
                }
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }

        case WM_KEYDOWN: {
            // Mode switching via F1 - F4
            if (wParam == VK_F1) { currentMode = 0; InvalidateRect(hwnd, NULL, TRUE); break; }
            if (wParam == VK_F2) { currentMode = 1; ResetSpeedTest(); InvalidateRect(hwnd, NULL, TRUE); break; }
            if (wParam == VK_F3) { currentMode = 2; ResetSpeedTest(); InvalidateRect(hwnd, NULL, TRUE); break; }
            if (wParam == VK_F4) { currentMode = 3; InvalidateRect(hwnd, NULL, TRUE); break; }
            if (wParam == VK_F5) { currentMode = 4; InvalidateRect(hwnd, NULL, TRUE); break; }
            if (wParam == VK_F6 && currentMode == 3) { ExportHeatmapBMP(hwnd); break; }
            if (wParam == VK_F7) { ExportCertificateBMP(hwnd); break; }
            if (wParam == 'H') { currentMode = 4; InvalidateRect(hwnd, NULL, TRUE); break; }
            if (wParam == VK_ESCAPE) {
                if (currentMode == 0) {
                    arcadeLives = 3; arcadeScore = 0; arcadeCombo = 0;
                    memset(fWords, 0, sizeof(fWords)); targetWord = -1; SpawnArcadeWord();
                } else {
                    ResetSpeedTest();
                }
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            break;
        }

        case WM_CHAR: {
            char c = (char)wParam;

            if (currentMode == 0) { // Arcade Mode logic
                if (arcadeLives <= 0) {
                    arcadeLives = 3; arcadeScore = 0; arcadeCombo = 0;
                    memset(fWords, 0, sizeof(fWords)); targetWord = -1; SpawnArcadeWord();
                } else {
                    if (c >= 'A' && c <= 'Z') c += 32;
                    if (c >= 'a' && c <= 'z') {
                        int keyIdx = c - 'a';
                        if (targetWord != -1 && fWords[targetWord].active) {
                            const char* cur = commonWords[fWords[targetWord].wordIdx];
                            int curLen = StrLen(cur);
                            if (fWords[targetWord].matchLen < curLen && c == cur[fWords[targetWord].matchLen]) {
                                fWords[targetWord].matchLen++;
                                arcadeCombo++;
                                keyHits[keyIdx]++;
                                if (fWords[targetWord].matchLen == curLen) {
                                    arcadeScore += 10 + arcadeCombo;
                                    if ((DWORD)arcadeScore > highArcadeScore) {
                                        highArcadeScore = (DWORD)arcadeScore;
                                        SaveRegistryData();
                                    }
                                    fWords[targetWord].active = 0;
                                    targetWord = -1;
                                }
                            } else {
                                arcadeCombo = 0;
                                keyErrors[keyIdx]++;
                                arcadeScore = (arcadeScore > 2) ? arcadeScore - 2 : 0;
                            }
                        } else {
                            int i, found = -1, highestY = -1000;
                            for (i = 0; i < MAX_FALLING; i++) {
                                if (fWords[i].active) {
                                    const char* w = commonWords[fWords[i].wordIdx];
                                    if (w[0] == c && fWords[i].matchLen == 0) {
                                        if (fWords[i].y > highestY) { highestY = fWords[i].y; found = i; }
                                    }
                                }
                            }
                            if (found != -1) {
                                targetWord = found;
                                fWords[targetWord].matchLen = 1;
                                arcadeCombo++;
                                keyHits[keyIdx]++;
                            } else {
                                arcadeCombo = 0;
                                keyErrors[keyIdx]++;
                            }
                        }
                    }
                }
            } else if (currentMode == 1 || currentMode == 2) { // Speed Test logic
                if (!testActive) StartSpeedTest();

                DWORD nowTime = GetTickCount();
                if (lastKeyTime > 0) {
                    DWORD dt = nowTime - lastKeyTime;
                    currentCadence = (dt > 0) ? (1000 / dt) : 0;
                }
                lastKeyTime = nowTime;

                const char** pool = (currentMode == 2) ? codeWords : commonWords;
                const char* curWord = pool[testWordPool[testWordIndex]];
                int curWordLen = StrLen(curWord);

                if (c == ' ') {
                    testWordIndex++;
                    testCharIndex = 0;
                    if (testWordIndex >= 40) testWordIndex = 0;
                } else if (c >= 32 && c <= 126) {
                    totalTyped++;
                    if (testCharIndex < curWordLen) {
                        char expected = curWord[testCharIndex];
                        char typedLow = c;
                        if (typedLow >= 'A' && typedLow <= 'Z') typedLow += 32;
                        char expLow = expected;
                        if (expLow >= 'A' && expLow <= 'Z') expLow += 32;

                        if (typedLow == expLow) {
                            correctTyped++;
                            if (typedLow >= 'a' && typedLow <= 'z') keyHits[typedLow - 'a']++;
                            DWORD freq = 300 + (currentCadence * 15);
                            if (freq > 800) freq = 800;
                            AsyncBeep(freq, 40);
                        } else {
                            testErrors++;
                            if (typedLow >= 'a' && typedLow <= 'z') keyErrors[typedLow - 'a']++;
                            AsyncBeep(150, 100);
                        }
                    } else {
                        testErrors++;
                        AsyncBeep(150, 100);
                    }
                    testCharIndex++;
                }
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Double buffer
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, W, H);
            HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

            HBRUSH bg = CreateSolidBrush(RGB(13, 15, 24));
            RECT full = {0, 0, W, H};
            FillRect(memDC, &full, bg);
            DeleteObject(bg);

            SetBkMode(memDC, TRANSPARENT);

            // Top Header Bar
            HBRUSH headerBg = CreateSolidBrush(RGB(24, 28, 44));
            RECT hRect = {0, 0, W, 40};
            FillRect(memDC, &hRect, headerBg);
            DeleteObject(headerBg);

            HGDIOBJ oldFont = SelectObject(memDC, g_fontNav ? g_fontNav : GetStockObject(DEFAULT_GUI_FONT));

            SetTextColor(memDC, (currentMode == 0) ? RGB(0, 242, 254) : RGB(148, 163, 184));
            TextOutA(memDC, 15, 10, "F1: Arcade", 10);

            SetTextColor(memDC, (currentMode == 1) ? RGB(0, 242, 254) : RGB(148, 163, 184));
            TextOutA(memDC, 130, 10, "F2: Speed Test", 14);

            SetTextColor(memDC, (currentMode == 2) ? RGB(0, 242, 254) : RGB(148, 163, 184));
            TextOutA(memDC, 270, 10, "F3: Code Snippets", 17);

            SetTextColor(memDC, (currentMode == 3) ? RGB(0, 242, 254) : RGB(148, 163, 184));
            TextOutA(memDC, 440, 10, "F4: Heatmap", 11);

            SetTextColor(memDC, (currentMode == 4) ? RGB(0, 242, 254) : RGB(148, 163, 184));
            TextOutA(memDC, 560, 10, "Press H for Help", 16);

            if (g_fontMain) SelectObject(memDC, g_fontMain);

            if (currentMode == 0) { // Arcade Mode Render
                if (arcadeLives > 0) {
                    int i;
                    for (i = 0; i < MAX_FALLING; i++) {
                        if (fWords[i].active) {
                            const char* cur = commonWords[fWords[i].wordIdx];
                            int totalLen = StrLen(cur);
                            int mLen = fWords[i].matchLen;

                            SetTextColor(memDC, (targetWord == i) ? RGB(245, 158, 11) : RGB(248, 250, 252));
                            TextOutA(memDC, fWords[i].x, fWords[i].y, cur, totalLen);

                            if (mLen > 0) {
                                SetTextColor(memDC, RGB(16, 185, 129));
                                TextOutA(memDC, fWords[i].x, fWords[i].y, cur, mLen);
                            }
                        }
                    }
                } else {
                    SetTextColor(memDC, RGB(239, 68, 68));
                    TextOutA(memDC, W/2 - 60, 180, "GAME OVER", 9);
                    SetTextColor(memDC, RGB(148, 163, 184));
                    TextOutA(memDC, W/2 - 130, 220, "Press ESC to restart Arcade", 27);
                }

                // HUD Bar Bottom
                char sBuf[32], lBuf[32], cBuf[32], hBuf[32];
                IntToStr(arcadeScore, sBuf);
                IntToStr(arcadeLives, lBuf);
                IntToStr(arcadeCombo, cBuf);
                IntToStr(highArcadeScore, hBuf);

                SetTextColor(memDC, RGB(148, 163, 184));
                TextOutA(memDC, 20, H - 40, "Score: ", 7);
                SetTextColor(memDC, RGB(0, 242, 254));
                TextOutA(memDC, 80, H - 40, sBuf, StrLen(sBuf));

                SetTextColor(memDC, RGB(148, 163, 184));
                TextOutA(memDC, 160, H - 40, "Lives: ", 7);
                SetTextColor(memDC, RGB(239, 68, 68));
                TextOutA(memDC, 220, H - 40, lBuf, StrLen(lBuf));

                SetTextColor(memDC, RGB(148, 163, 184));
                TextOutA(memDC, 300, H - 40, "Combo: ", 7);
                SetTextColor(memDC, RGB(245, 158, 11));
                TextOutA(memDC, 370, H - 40, cBuf, StrLen(cBuf));

                SetTextColor(memDC, RGB(148, 163, 184));
                TextOutA(memDC, 450, H - 40, "Best: ", 6);
                SetTextColor(memDC, RGB(16, 185, 129));
                TextOutA(memDC, 510, H - 40, hBuf, StrLen(hBuf));

            } else if (currentMode == 1 || currentMode == 2) { // Speed Test Render
                DWORD elapsedSec = testActive ? (GetTickCount() - testStartTime) / 1000 : 0;
                DWORD remSec = (elapsedSec < (DWORD)testDuration) ? ((DWORD)testDuration - elapsedSec) : 0;

                DWORD wpm = 0;
                if (elapsedSec > 0) wpm = (correctTyped / 5) * 60 / elapsedSec;

                DWORD acc = totalTyped > 0 ? (correctTyped * 100 / totalTyped) : 100;

                char wpmBuf[32], accBuf[32], remBuf[32], bestBuf[32];
                IntToStr(wpm, wpmBuf);
                IntToStr(acc, accBuf);
                IntToStr(remSec, remBuf);
                IntToStr(bestWPM, bestBuf);

                // Stats row
                SetTextColor(memDC, RGB(148, 163, 184));
                TextOutA(memDC, 30, 60, "WPM: ", 5);
                SetTextColor(memDC, RGB(0, 242, 254));
                TextOutA(memDC, 80, 60, wpmBuf, StrLen(wpmBuf));

                SetTextColor(memDC, RGB(148, 163, 184));
                TextOutA(memDC, 160, 60, "Acc: ", 5);
                SetTextColor(memDC, RGB(16, 185, 129));
                TextOutA(memDC, 210, 60, accBuf, StrLen(accBuf));
                TextOutA(memDC, 210 + StrLen(accBuf)*12, 60, "%", 1);

                SetTextColor(memDC, RGB(148, 163, 184));
                TextOutA(memDC, 300, 60, "Time: ", 6);
                SetTextColor(memDC, RGB(245, 158, 11));
                TextOutA(memDC, 360, 60, remBuf, StrLen(remBuf));
                TextOutA(memDC, 360 + StrLen(remBuf)*12, 60, "s", 1);

                SetTextColor(memDC, RGB(148, 163, 184));
                TextOutA(memDC, 450, 60, "Best WPM: ", 10);
                SetTextColor(memDC, RGB(0, 242, 254));
                TextOutA(memDC, 540, 60, bestBuf, StrLen(bestBuf));

                // Words stream display
                const char** pool = (currentMode == 2) ? codeWords : commonWords;
                int startIdx = (testWordIndex >= 5) ? testWordIndex - 2 : 0;
                int yPos = 140;
                int i;

                for (i = startIdx; i < startIdx + 8 && i < 40; i++) {
                    const char* w = pool[testWordPool[i]];
                    int jitterX = 0, jitterY = 0;
                    if (i == testWordIndex && currentCadence > 5) {
                        jitterX = (MyRand() % 4) - 2;
                        jitterY = (MyRand() % 4) - 2;
                    }

                    if (i == testWordIndex) {
                        SetTextColor(memDC, RGB(0, 242, 254));
                        TextOutA(memDC, 40, yPos, "> ", 2);
                        TextOutA(memDC, 65 + jitterX, yPos + jitterY, w, StrLen(w));
                    } else if (i < testWordIndex) {
                        SetTextColor(memDC, RGB(16, 185, 129));
                        TextOutA(memDC, 65, yPos, w, StrLen(w));
                    } else {
                        SetTextColor(memDC, RGB(100, 116, 139));
                        TextOutA(memDC, 65, yPos, w, StrLen(w));
                    }
                    yPos += 30;
                }

                if (!testActive && elapsedSec == 0) {
                    SetTextColor(memDC, RGB(148, 163, 184));
                    TextOutA(memDC, 40, H - 40, "Start typing any key to begin 30s Speed Test...", 46);
                }

            } else if (currentMode == 3) { // QWERTY Heatmap Render
                SetTextColor(memDC, RGB(0, 242, 254));
                TextOutA(memDC, 30, 50, "Finger Weakness & Key Heatmap Analysis", 38);
                SetTextColor(memDC, RGB(148, 163, 184));
                TextOutA(memDC, 440, 50, "(F6 to Export BMP)", 18);

                const char* r1 = "QWERTYUIOP";
                const char* r2 = "ASDFGHJKL";
                const char* r3 = "ZXCVBNM";

                int startY = 120;
                int kSize = 40;

                // Draw Row 1
                int i;
                for (i = 0; i < 10; i++) {
                    int kIdx = r1[i] - 'A';
                    int errs = keyErrors[kIdx];
                    HBRUSH kBrush = (errs > 3) ? CreateSolidBrush(RGB(239, 68, 68)) :
                                    (errs > 0) ? CreateSolidBrush(RGB(245, 158, 11)) :
                                                 CreateSolidBrush(RGB(30, 41, 59));
                    RECT kRect = {40 + i*48, startY, 40 + i*48 + kSize, startY + kSize};
                    FillRect(memDC, &kRect, kBrush);
                    DeleteObject(kBrush);

                    char label[2] = {r1[i], '\0'};
                    SetTextColor(memDC, RGB(248, 250, 252));
                    TextOutA(memDC, 52 + i*48, startY + 8, label, 1);
                }

                // Draw Row 2
                for (i = 0; i < 9; i++) {
                    int kIdx = r2[i] - 'A';
                    int errs = keyErrors[kIdx];
                    HBRUSH kBrush = (errs > 3) ? CreateSolidBrush(RGB(239, 68, 68)) :
                                    (errs > 0) ? CreateSolidBrush(RGB(245, 158, 11)) :
                                                 CreateSolidBrush(RGB(30, 41, 59));
                    RECT kRect = {60 + i*48, startY + 52, 60 + i*48 + kSize, startY + 52 + kSize};
                    FillRect(memDC, &kRect, kBrush);
                    DeleteObject(kBrush);

                    char label[2] = {r2[i], '\0'};
                    SetTextColor(memDC, RGB(248, 250, 252));
                    TextOutA(memDC, 72 + i*48, startY + 60, label, 1);
                }

                // Draw Row 3
                for (i = 0; i < 7; i++) {
                    int kIdx = r3[i] - 'A';
                    int errs = keyErrors[kIdx];
                    HBRUSH kBrush = (errs > 3) ? CreateSolidBrush(RGB(239, 68, 68)) :
                                    (errs > 0) ? CreateSolidBrush(RGB(245, 158, 11)) :
                                                 CreateSolidBrush(RGB(30, 41, 59));
                    RECT kRect = {90 + i*48, startY + 104, 90 + i*48 + kSize, startY + 104 + kSize};
                    FillRect(memDC, &kRect, kBrush);
                    DeleteObject(kBrush);

                    char label[2] = {r3[i], '\0'};
                    SetTextColor(memDC, RGB(248, 250, 252));
                    TextOutA(memDC, 102 + i*48, startY + 112, label, 1);
                }

                // Legend
                SetTextColor(memDC, RGB(148, 163, 184));
                TextOutA(memDC, 40, H - 60, "Legend: Dark = Perfect | Amber = 1-3 Mistypes | Red = >3 Mistypes", 64);
            } else if (currentMode == 4) { // Help Screen
                SetTextColor(memDC, RGB(0, 242, 254));
                TextOutA(memDC, 30, 50, "Help & Controls", 15);
                SetTextColor(memDC, RGB(248, 250, 252));
                TextOutA(memDC, 30, 90, "F1: Arcade Cascade Mode", 23);
                TextOutA(memDC, 30, 120, "F2: 30s Timed Speed Test", 24);
                TextOutA(memDC, 30, 150, "F3: Code Snippets Speed Test", 28);
                TextOutA(memDC, 30, 180, "F4: Finger Weakness Heatmap", 27);
                TextOutA(memDC, 30, 210, "F6: Export Heatmap to BMP", 25);
                TextOutA(memDC, 30, 240, "F7: Export Certificate to BMP", 29);
                TextOutA(memDC, 30, 270, "F5 or H: Toggle Help", 20);
                TextOutA(memDC, 30, 300, "ESC: Restart current mode", 25);
            }

            SelectObject(memDC, oldFont);

            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            char filePath[MAX_PATH];
            if (DragQueryFileA(hDrop, 0, filePath, MAX_PATH)) {
                if (my_strstr(filePath, ".ttf") || my_strstr(filePath, ".otf") || my_strstr(filePath, ".TTF") || my_strstr(filePath, ".OTF")) {
                    if (AddFontResourceExA(filePath, FR_PRIVATE, 0)) {
                        char fontName[64] = {0};
                        const char* slash = my_strrchr(filePath, '\\');
                        if (slash) slash++; else slash = filePath;
                        int i = 0;
                        while (slash[i] && slash[i] != '.' && i < 63) { fontName[i] = slash[i]; i++; }
                        if (g_fontMain) DeleteObject(g_fontMain);
                        g_fontMain = CreateFontA(24, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, fontName);
                        MessageBoxA(hwnd, "Custom font file loaded. (Font face name must match the filename for it to render correctly)", "Font Loaded", MB_OK);
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            }
            DragFinish(hDrop);
            break;
        }

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            if (g_fontNav) DeleteObject(g_fontNav);
            if (g_fontMain) DeleteObject(g_fontMain);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

void MainEntry() {
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KTypeApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    DWORD style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    RECT rect = {0, 0, W, H};
    AdjustWindowRect(&rect, style, FALSE);

    HWND hwnd = CreateWindowEx(0, "KTypeApp", "KType Studio", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
