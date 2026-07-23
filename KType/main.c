#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 600
#define H 500

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
// 0: Arcade Cascade, 1: Timed Speed Test (30s), 2: Code Snippets, 3: Error Heatmap
int currentMode = 0;

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
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            randSeed = GetTickCount();
            LoadRegistryData();
            memset(fWords, 0, sizeof(fWords));
            memset(keyHits, 0, sizeof(keyHits));
            memset(keyErrors, 0, sizeof(keyErrors));
            SpawnArcadeWord();
            ResetSpeedTest();
            SetTimer(hwnd, 1, 30, NULL);
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
                        DWORD minutes = elapsedSec / 60;
                        DWORD wpm = 0;
                        if (minutes > 0) {
                            wpm = (correctTyped / 5) / minutes;
                        } else {
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
                            if (c == cur[fWords[targetWord].matchLen]) {
                                fWords[targetWord].matchLen++;
                                arcadeCombo++;
                                keyHits[keyIdx]++;
                                if (fWords[targetWord].matchLen == StrLen(cur)) {
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

                const char** pool = (currentMode == 2) ? codeWords : commonWords;
                const char* curWord = pool[testWordPool[testWordIndex]];

                if (c == ' ') {
                    testWordIndex++;
                    testCharIndex = 0;
                    if (testWordIndex >= 40) testWordIndex = 0;
                } else if (c >= 32 && c <= 126) {
                    totalTyped++;
                    char expected = curWord[testCharIndex];
                    if (expected >= 'A' && expected <= 'Z') expected += 32;
                    char typedLow = c;
                    if (typedLow >= 'A' && typedLow <= 'Z') typedLow += 32;

                    if (typedLow >= 'a' && typedLow <= 'z') {
                        int kIdx = typedLow - 'a';
                        if (typedLow == expected) {
                            correctTyped++;
                            keyHits[kIdx]++;
                        } else {
                            testErrors++;
                            keyErrors[kIdx]++;
                        }
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

            HFONT fontNav = CreateFontA(14, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            HGDIOBJ oldFont = SelectObject(memDC, fontNav);

            SetTextColor(memDC, (currentMode == 0) ? RGB(0, 242, 254) : RGB(148, 163, 184));
            TextOutA(memDC, 15, 10, "F1: Arcade", 10);

            SetTextColor(memDC, (currentMode == 1) ? RGB(0, 242, 254) : RGB(148, 163, 184));
            TextOutA(memDC, 120, 10, "F2: Speed Test", 14);

            SetTextColor(memDC, (currentMode == 2) ? RGB(0, 242, 254) : RGB(148, 163, 184));
            TextOutA(memDC, 250, 10, "F3: Code Snippets", 17);

            SetTextColor(memDC, (currentMode == 3) ? RGB(0, 242, 254) : RGB(148, 163, 184));
            TextOutA(memDC, 410, 10, "F4: Heatmap", 11);

            HFONT fontMain = CreateFontA(22, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            SelectObject(memDC, fontMain);

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
                    if (i == testWordIndex) {
                        SetTextColor(memDC, RGB(0, 242, 254));
                        TextOutA(memDC, 40, yPos, "> ", 2);
                        TextOutA(memDC, 65, yPos, w, StrLen(w));
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
            }

            SelectObject(memDC, oldFont);
            DeleteObject(fontMain);
            DeleteObject(fontNav);

            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
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
