#include <windows.h>
#include <stdio.h>
#include <string.h>

#define ID_BTN_SET_COURSE 101

typedef struct {
    int id;
    char name[32];
    int x;
    int y;
    char desc[256];
} System;

System systems[] = {
    {1, "Sol", 50, 50, "Cradle of humanity. Highly developed planetary system with a stable economy."},
    {2, "Alpha Centauri", 30, 70, "Primary mining colony. Resource rich, but known for pirate activity on the outskirts."},
    {3, "Sirius", 75, 35, "Major trading hub. High tech goods are cheap here."},
    {4, "Proxima", 20, 80, "Remote outpost. Dangerous but offers high rewards for smugglers."},
    {5, "Vega", 80, 80, "Agricultural world. Supplies food to neighboring industrial systems."}
};

HWND hMapArea, hInfoArea, hBtnCourse;
int selectedSystem = -1;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            CreateWindow("STATIC", "FUEL: 100%", WS_VISIBLE | WS_CHILD, 20, 20, 100, 20, hwnd, NULL, NULL, NULL);
            CreateWindow("STATIC", "CREDITS: 1,000", WS_VISIBLE | WS_CHILD, 140, 20, 150, 20, hwnd, NULL, NULL, NULL);
            CreateWindow("STATIC", "CARGO: 0/50 TONS", WS_VISIBLE | WS_CHILD, 300, 20, 150, 20, hwnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "LOCAL SYSTEMS", WS_VISIBLE | WS_CHILD, 440, 60, 150, 20, hwnd, NULL, NULL, NULL);
            hInfoArea = CreateWindow("STATIC", "Select a system on the map for details.", WS_VISIBLE | WS_CHILD, 440, 90, 200, 150, hwnd, NULL, NULL, NULL);
            hBtnCourse = CreateWindow("BUTTON", "Set Course", WS_VISIBLE | WS_CHILD, 440, 250, 150, 30, hwnd, (HMENU)ID_BTN_SET_COURSE, NULL, NULL);
            ShowWindow(hBtnCourse, SW_HIDE);
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH hBlackBrush = CreateSolidBrush(RGB(5, 16, 5));
            SelectObject(hdc, hBlackBrush);
            Rectangle(hdc, 20, 60, 420, 460);
            DeleteObject(hBlackBrush);
            
            HBRUSH hGreenBrush = CreateSolidBrush(RGB(0, 255, 0));
            HBRUSH hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255));
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 255, 0));
            
            for (int i = 0; i < 5; i++) {
                int px = 20 + (systems[i].x * 400 / 100);
                int py = 60 + (systems[i].y * 400 / 100);
                
                if (i == selectedSystem) {
                    SelectObject(hdc, hWhiteBrush);
                } else {
                    SelectObject(hdc, hGreenBrush);
                }
                
                Ellipse(hdc, px - 6, py - 6, px + 6, py + 6);
                TextOut(hdc, px + 10, py - 8, systems[i].name, strlen(systems[i].name));
            }
            
            DeleteObject(hGreenBrush);
            DeleteObject(hWhiteBrush);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            
            for (int i = 0; i < 5; i++) {
                int px = 20 + (systems[i].x * 400 / 100);
                int py = 60 + (systems[i].y * 400 / 100);
                
                if (abs(mx - px) < 15 && abs(my - py) < 15) {
                    selectedSystem = i;
                    
                    char infoText[512];
                    sprintf(infoText, "%s\n\n%s\n\nCoordinates: X:%d Y:%d", 
                        systems[i].name, systems[i].desc, systems[i].x, systems[i].y);
                        
                    SetWindowText(hInfoArea, infoText);
                    ShowWindow(hBtnCourse, SW_SHOW);
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                }
            }
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_SET_COURSE) {
                MessageBox(hwnd, "Navigation sequence initiated (Phase 5 feature)", "Navigation", MB_OK);
            }
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KStellar Window Class";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "KStellar Phase 3",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 680, 520,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
