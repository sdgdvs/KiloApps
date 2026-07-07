#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 400
#define H 500

const char* words[] = {
    "hello", "world", "kilo", "system", "typing",
    "speed", "code", "native", "win32", "game",
    "fast", "keyboard", "monitor", "software", "mouse"
};
int numWords = 15;

int score = 0;
int lives = 3;

int activeWordIdx = 0;
int wordY = 0;
int wordX = 150;
int matchLen = 0;

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
    int i = 0;
    while (val > 0) { temp[i++] = (val % 10) + '0'; val /= 10; }
    int j = 0;
    while (i > 0) buf[j++] = temp[--i];
    buf[j] = '\0';
}

void SpawnWord() {
    activeWordIdx = MyRand() % numWords;
    wordY = -20;
    wordX = 50 + (MyRand() % 200);
    matchLen = 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            randSeed = GetTickCount();
            SpawnWord();
            SetTimer(hwnd, 1, 30, NULL);
            break;
        case WM_TIMER: {
            if (lives > 0) {
                wordY += 2;
                if (wordY > H) {
                    lives--;
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
                SpawnWord();
            } else {
                char c = (char)wParam;
                const char* cur = words[activeWordIdx];
                if (c == cur[matchLen]) {
                    matchLen++;
                    if (matchLen == StrLen(cur)) {
                        score += 10;
                        SpawnWord();
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
            SelectObject(memDC, memBM);
            
            HBRUSH bg = CreateSolidBrush(RGB(20, 20, 40));
            RECT full = {0, 0, W, H};
            FillRect(memDC, &full, bg);
            DeleteObject(bg);
            
            SetBkMode(memDC, TRANSPARENT);
            
            HFONT hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
            HGDIOBJ oldFont = SelectObject(memDC, hFont);
            
            if (lives > 0) {
                const char* cur = words[activeWordIdx];
                int totalLen = StrLen(cur);
                
                // Draw unmatched part
                SetTextColor(memDC, RGB(255, 255, 255));
                TextOutA(memDC, wordX, wordY, cur, totalLen);
                
                // Draw matched part in green
                if (matchLen > 0) {
                    SetTextColor(memDC, RGB(100, 255, 100));
                    TextOutA(memDC, wordX, wordY, cur, matchLen);
                }
            } else {
                SetTextColor(memDC, RGB(255, 100, 100));
                TextOutA(memDC, 120, 200, "GAME OVER", 9);
                SetTextColor(memDC, RGB(200, 200, 200));
                TextOutA(memDC, 80, 240, "Press any key to restart", 24);
            }
            
            char sBuf[32];
            char lBuf[32];
            IntToStr(score, sBuf);
            IntToStr(lives, lBuf);
            
            SetTextColor(memDC, RGB(200, 200, 200));
            TextOutA(memDC, 10, 10, "Score: ", 7);
            TextOutA(memDC, 80, 10, sBuf, StrLen(sBuf));
            
            TextOutA(memDC, 10, 40, "Lives: ", 7);
            TextOutA(memDC, 80, 40, lBuf, StrLen(lBuf));
            
            SelectObject(memDC, oldFont);
            DeleteObject(hFont);
            
            BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
            DeleteObject(memBM);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_DESTROY:
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

    HWND hwnd = CreateWindowEx(0, "KTypeApp", "KType", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
