#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 400
#define H 500

const char* words[] = {
    "hello", "world", "kilo", "system", "typing",
    "speed", "code", "native", "win32", "game",
    "fast", "keyboard", "monitor", "software", "mouse",
    "hardware", "interface", "compiler", "linker", "binary",
    "execute", "process", "thread", "memory", "storage",
    "network", "server", "client", "packet", "router"
};
int numWords = 30;

int score = 0;
int lives = 3;

#define MAX_WORDS 5
typedef struct {
    int active;
    int wordIdx;
    int y;
    int x;
    int matchLen;
} FallingWord;

FallingWord fWords[MAX_WORDS];
int targetWord = -1;
int combo = 0;

int randSeed = 12345;
int MyRand() {
    randSeed = randSeed * 1103515245 + 12345;
    return (unsigned int)(randSeed / 65536) % 32768;
}

int StrLen(const char* s) {
    int c = 0;
    while (s[c]) c++;
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

void SpawnWord() {
    int i;
    for (i = 0; i < MAX_WORDS; i++) {
        if (!fWords[i].active) {
            fWords[i].active = 1;
            fWords[i].wordIdx = MyRand() % numWords;
            fWords[i].y = -20;
            fWords[i].x = 20 + (MyRand() % 250);
            fWords[i].matchLen = 0;
            return;
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            randSeed = GetTickCount();
            memset(fWords, 0, sizeof(fWords));
            SpawnWord();
            SetTimer(hwnd, 1, 30, NULL);
            break;
        case WM_TIMER: {
            if (lives > 0) {
                int i;
                int speed = 2 + (score / 100);
                for (i = 0; i < MAX_WORDS; i++) {
                    if (fWords[i].active) {
                        fWords[i].y += speed;
                        if (fWords[i].y > H) {
                            fWords[i].active = 0;
                            lives--;
                            if (targetWord == i) targetWord = -1;
                            combo = 0;
                        }
                    }
                }
                if ((MyRand() % 100) < (2 + score / 200)) {
                    SpawnWord();
                }
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_CHAR: {
            if (lives <= 0) {
                // Restart
                lives = 3;
                score = 0;
                combo = 0;
                memset(fWords, 0, sizeof(fWords));
                targetWord = -1;
                SpawnWord();
            } else {
                char c = (char)wParam;
                if (c >= 'A' && c <= 'Z') c += 32;
                if (c >= 'a' && c <= 'z') {
                    if (targetWord != -1 && fWords[targetWord].active) {
                        const char* cur = words[fWords[targetWord].wordIdx];
                        if (c == cur[fWords[targetWord].matchLen]) {
                            fWords[targetWord].matchLen++;
                            combo++;
                            if (fWords[targetWord].matchLen == StrLen(cur)) {
                                score += 10 + combo;
                                fWords[targetWord].active = 0;
                                targetWord = -1;
                            }
                        } else {
                            combo = 0;
                            score -= 2;
                            if (score < 0) score = 0;
                        }
                    } else {
                        int i;
                        int found = -1;
                        int highestY = -1000;
                        for (i = 0; i < MAX_WORDS; i++) {
                            if (fWords[i].active) {
                                const char* w = words[fWords[i].wordIdx];
                                if (w[0] == c && fWords[i].matchLen == 0) {
                                    if (fWords[i].y > highestY) {
                                        highestY = fWords[i].y;
                                        found = i;
                                    }
                                }
                            }
                        }
                        if (found != -1) {
                            targetWord = found;
                            fWords[targetWord].matchLen = 1;
                            combo++;
                            const char* cur = words[fWords[targetWord].wordIdx];
                            if (fWords[targetWord].matchLen == StrLen(cur)) {
                                score += 10 + combo;
                                fWords[targetWord].active = 0;
                                targetWord = -1;
                            }
                        } else {
                            combo = 0;
                            score -= 2;
                            if (score < 0) score = 0;
                        }
                    }
                }
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Double buffering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, W, H);
            HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);
            
            HBRUSH bg = CreateSolidBrush(RGB(20, 20, 40));
            RECT full = {0, 0, W, H};
            FillRect(memDC, &full, bg);
            DeleteObject(bg);
            
            SetBkMode(memDC, TRANSPARENT);
            
            HFONT hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            HGDIOBJ oldFont = SelectObject(memDC, hFont);
            
            if (lives > 0) {
                int i;
                for (i = 0; i < MAX_WORDS; i++) {
                    if (fWords[i].active) {
                        const char* cur = words[fWords[i].wordIdx];
                        int totalLen = StrLen(cur);
                        int mLen = fWords[i].matchLen;
                        
                        SetTextColor(memDC, (targetWord == i) ? RGB(255, 255, 100) : RGB(255, 255, 255));
                        TextOutA(memDC, fWords[i].x, fWords[i].y, cur, totalLen);
                        
                        if (mLen > 0) {
                            SetTextColor(memDC, RGB(100, 255, 100));
                            TextOutA(memDC, fWords[i].x, fWords[i].y, cur, mLen);
                        }
                    }
                }
            } else {
                SetTextColor(memDC, RGB(255, 100, 100));
                TextOutA(memDC, 120, 200, "GAME OVER", 9);
                SetTextColor(memDC, RGB(200, 200, 200));
                TextOutA(memDC, 80, 240, "Press any key to restart", 24);
            }
            
            char sBuf[32];
            char lBuf[32];
            char cBuf[32];
            IntToStr(score, sBuf);
            IntToStr(lives, lBuf);
            IntToStr(combo, cBuf);
            
            SetTextColor(memDC, RGB(200, 200, 200));
            TextOutA(memDC, 10, 10, "Score: ", 7);
            TextOutA(memDC, 80, 10, sBuf, StrLen(sBuf));
            
            TextOutA(memDC, 10, 40, "Lives: ", 7);
            TextOutA(memDC, 80, 40, lBuf, StrLen(lBuf));

            TextOutA(memDC, 10, 70, "Combo: ", 7);
            TextOutA(memDC, 80, 70, cBuf, StrLen(cBuf));
            
            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
            
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

    HWND hwnd = CreateWindowEx(0, "KTypeApp", "KType", style,
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
