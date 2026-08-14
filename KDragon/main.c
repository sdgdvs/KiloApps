#include <windows.h>
#include <stdio.h>
#include <string.h>

#define BTN_INCUBATE 1
#define BTN_FEED     2
#define BTN_PLAY     3
#define BTN_SLEEP    4
#define TIMER_ID     1

int state = 0; // 0 = egg, 1 = dragon
int element = 0; // 0=none, 2=fire, 3=earth, 4=water
int feed_count = 0;
int play_count = 0;
int sleep_count = 0;
int hunger = 50;
int happiness = 50;
int energy = 100;
int age = 0;

#define MAX_LOG_LINES 4
char log_messages[MAX_LOG_LINES][128] = {0};
int log_count = 0;

HWND btn_incubate, btn_feed, btn_play, btn_sleep;
HFONT hFontNormal, hFontLarge;
HBRUSH bgBrush;

void add_log(const char* msg) {
    for (int i = MAX_LOG_LINES - 1; i > 0; --i) {
        strcpy(log_messages[i], log_messages[i-1]);
    }
    strcpy(log_messages[0], msg);
    if (log_count < MAX_LOG_LINES) log_count++;
}

COLORREF egg_pixels[16][16] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(255,255,255), RGB(180,150,100), -1, -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1},
    {-1, -1, -1, -1, RGB(180,150,100), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(230,210,180), RGB(180,150,100), -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), RGB(180,150,100), -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

COLORREF dragon_pixels[16][16] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, RGB(30,100,30), RGB(30,100,30), -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1, RGB(30,100,30), -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(30,100,30), RGB(30,100,30), -1, -1},
    {-1, RGB(30,100,30), -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1},
    {-1, RGB(30,100,30), RGB(30,100,30), -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), -1, -1},
    {-1, RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), -1, -1},
    {-1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), RGB(30,100,30), RGB(30,100,30), -1, -1},
    {-1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1, -1, -1, -1},
    {-1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(60,180,60), RGB(30,100,30), -1, -1, -1, -1, -1},
    {-1, -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), -1, -1, RGB(30,100,30), RGB(60,180,60), RGB(30,100,30), -1, -1, -1, -1, -1},
    {-1, -1, -1, RGB(30,100,30), RGB(30,100,30), -1, -1, -1, -1, RGB(30,100,30), RGB(30,100,30), -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

COLORREF adult_dragon_pixels[16][16] = {
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, RGB(180,30,30), RGB(180,30,30), -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1, -1, -1, RGB(180,30,30), -1, -1, -1},
    {-1, -1, -1, RGB(180,30,30), RGB(220,60,60), RGB(255,255,255), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1, RGB(180,30,30), RGB(180,30,30), -1, -1, -1},
    {-1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), RGB(180,30,30), RGB(180,30,30), -1, -1, -1, -1},
    {-1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), RGB(180,30,30), -1, -1, -1, -1, -1},
    {-1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1, -1, -1, -1, -1},
    {-1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1, -1, -1, -1, -1},
    {-1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), RGB(180,30,30), RGB(180,30,30), -1, -1, -1, -1},
    {-1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1, -1},
    {-1, -1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), RGB(180,30,30), -1, -1},
    {-1, -1, -1, RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), RGB(180,30,30), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(220,60,60), RGB(180,30,30), -1, -1},
    {-1, -1, -1, RGB(180,30,30), RGB(180,30,30), RGB(180,30,30), RGB(180,30,30), -1, -1, RGB(180,30,30), RGB(180,30,30), RGB(180,30,30), RGB(180,30,30), -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

void DrawPixelArt(HDC hdc, int x, int y, int scale, COLORREF pixels[16][16], int element_type) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            if (pixels[i][j] != -1) {
                COLORREF c = pixels[i][j];
                if (element_type == 4) { // Water
                    if (c == RGB(180,30,30)) c = RGB(30,60,180);
                    else if (c == RGB(220,60,60)) c = RGB(60,120,220);
                } else if (element_type == 3) { // Earth
                    if (c == RGB(180,30,30)) c = RGB(100,60,30);
                    else if (c == RGB(220,60,60)) c = RGB(150,100,60);
                }
                HBRUSH b = CreateSolidBrush(c);
                RECT r = {x + j*scale, y + i*scale, x + (j+1)*scale, y + (i+1)*scale};
                FillRect(hdc, &r, b);
                DeleteObject(b);
            }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            hFontNormal = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                     DEFAULT_PITCH | FF_DONTCARE, "Times New Roman");
            hFontLarge = CreateFont(80, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                    DEFAULT_PITCH | FF_DONTCARE, "Segoe UI Emoji");
            bgBrush = CreateSolidBrush(RGB(220, 184, 129));
            
            btn_incubate = CreateWindow("BUTTON", "Incubate Egg", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                                        230, 260, 120, 40, hwnd, (HMENU)BTN_INCUBATE, NULL, NULL);
            btn_feed = CreateWindow("BUTTON", "Feed", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                    130, 260, 100, 40, hwnd, (HMENU)BTN_FEED, NULL, NULL);
            btn_play = CreateWindow("BUTTON", "Play", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                    240, 260, 100, 40, hwnd, (HMENU)BTN_PLAY, NULL, NULL);
            btn_sleep = CreateWindow("BUTTON", "Sleep", WS_TABSTOP | WS_CHILD | BS_DEFPUSHBUTTON,
                                     350, 260, 100, 40, hwnd, (HMENU)BTN_SLEEP, NULL, NULL);
                                     
            SendMessage(btn_incubate, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_feed, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_play, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            SendMessage(btn_sleep, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
            break;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == BTN_INCUBATE) {
                state = 1;
                add_log("The egg hatched! A baby dragon emerged.");
                ShowWindow(btn_incubate, SW_HIDE);
                ShowWindow(btn_feed, SW_SHOW);
                ShowWindow(btn_play, SW_SHOW);
                ShowWindow(btn_sleep, SW_SHOW);
                SetTimer(hwnd, TIMER_ID, 3000, NULL);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_FEED) {
                if (hunger >= 100) {
                    add_log("Dragon is full and refuses to eat.");
                } else {
                    feed_count++;
                    hunger = hunger + 20;
                    if (hunger > 100) hunger = 100;
                    energy = energy - 5;
                    if (energy < 0) energy = 0;
                    add_log("You fed the dragon. It looks satisfied.");
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_PLAY) {
                if (energy < 20) {
                    add_log("Dragon is too tired to play.");
                } else {
                    play_count++;
                    happiness = happiness + 20;
                    if (happiness > 100) happiness = 100;
                    energy = energy - 20;
                    if (energy < 0) energy = 0;
                    hunger = hunger - 10;
                    if (hunger < 0) hunger = 0;
                    add_log("You played with the dragon! It's happy.");
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (LOWORD(wParam) == BTN_SLEEP) {
                sleep_count++;
                energy = energy + 40;
                if (energy > 100) energy = 100;
                hunger = hunger - 10;
                if (hunger < 0) hunger = 0;
                add_log("Dragon took a nap and regained energy.");
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case WM_TIMER:
            if (state == 1 || state == 2) {
                hunger = hunger - 2;
                if (hunger < 0) hunger = 0;
                happiness = happiness - 1;
                if (happiness < 0) happiness = 0;
                age++;
                
                if (hunger < 20) add_log("Dragon is getting hungry...");
                if (happiness < 20) add_log("Dragon is feeling sad...");
                
                if (state == 1 && age >= 10) {
                    state = 2;
                    int max_c = feed_count;
                    if (play_count > max_c) max_c = play_count;
                    if (sleep_count > max_c) max_c = sleep_count;
                    
                    char msg[128];
                    if (max_c == feed_count) {
                        element = 3;
                        strcpy(msg, "Your baby dragon evolved into an ADULT EARTH DRAGON!");
                    } else if (max_c == play_count) {
                        element = 2;
                        strcpy(msg, "Your baby dragon evolved into an ADULT FIRE DRAGON!");
                    } else {
                        element = 4;
                        strcpy(msg, "Your baby dragon evolved into an ADULT WATER DRAGON!");
                    }
                    add_log(msg);
                }
                
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            FillRect(hdc, &ps.rcPaint, bgBrush);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(42, 23, 4));
            
            SelectObject(hdc, hFontNormal);
            
            if (state == 0) {
                const char* msg = "A mysterious egg awaits...";
                TextOut(hdc, 190, 80, msg, strlen(msg));
                
                DrawPixelArt(hdc, 236, 110, 8, egg_pixels, 0);
            } else {
                char buf[128];
                const char* type_str = "None";
                if (element == 2) type_str = "Fire";
                else if (element == 3) type_str = "Earth";
                else if (element == 4) type_str = "Water";
                sprintf(buf, "Hunger: %d/100   Happiness: %d/100   Energy: %d/100   Age: %d   Type: %s", hunger, happiness, energy, age, type_str);
                
                RECT r = {0, 30, 600, 60};
                DrawText(hdc, buf, strlen(buf), &r, DT_CENTER | DT_TOP);
                
                if (state == 1) {
                    DrawPixelArt(hdc, 236, 90, 8, dragon_pixels, 0);
                } else if (state == 2) {
                    DrawPixelArt(hdc, 236, 90, 8, adult_dragon_pixels, element);
                }
                
                SelectObject(hdc, hFontNormal);
                // Draw log box
                HBRUSH logBrush = CreateSolidBrush(RGB(235, 208, 165));
                RECT logRect = {20, 320, 560, 420};
                FillRect(hdc, &logRect, logBrush);
                DeleteObject(logBrush);
                
                SetTextColor(hdc, RGB(42, 23, 4));
                for (int i = 0; i < log_count; i++) {
                    char logStr[150];
                    sprintf(logStr, "> %s", log_messages[i]);
                    TextOut(hdc, 30, 330 + i * 20, logStr, strlen(logStr));
                }
            }
            
            EndPaint(hwnd, &ps);
            break;
        }

        case WM_CTLCOLORBTN: {
            HDC hdcBtn = (HDC)wParam;
            SetBkColor(hdcBtn, RGB(139, 90, 43));
            SetTextColor(hdcBtn, RGB(42, 23, 4));
            return (LRESULT)GetStockObject(WHITE_BRUSH); // simple dark theme button
        }

        case WM_DESTROY:
            KillTimer(hwnd, TIMER_ID);
            DeleteObject(hFontNormal);
            DeleteObject(hFontLarge);
            DeleteObject(bgBrush);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[]  = "KDragonClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KDragon",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 500,
        NULL, NULL, hInstance, NULL
    );
    
    if (hwnd == NULL) {
        return 0;
    }
    
    ShowWindow(hwnd, nCmdShow);
    
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}
