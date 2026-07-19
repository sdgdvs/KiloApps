#include <windows.h>

#define ID_BTN_NEWGAME 100

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect;

    switch (uMsg) {
        case WM_CREATE: {
            CreateWindowA("BUTTON", "New Game", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                20, 20, 100, 30, hwnd, (HMENU)ID_BTN_NEWGAME, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            return 0;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_NEWGAME) {
                MessageBoxA(hwnd, "New Game Started!", "KHangman", MB_OK | MB_ICONINFORMATION);
            }
            return 0;
        }

        case WM_PAINT: {
            hdc = BeginPaint(hwnd, &ps);
            GetClientRect(hwnd, &rect);

            // Dark background
            HBRUSH hBrush = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(200, 200, 200));

            HFONT hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                      DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

            TextOutA(hdc, 20, 80, "KHangman Skeleton", 17);

            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            
            // Hangman base drawing (text)
            HFONT hMono = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                      FIXED_PITCH | FF_MODERN, "Consolas");
            hOldFont = (HFONT)SelectObject(hdc, hMono);
            SetTextColor(hdc, RGB(255, 82, 82));
            
            const char* drawing[] = {
                "  +---+",
                "  |   |",
                "      |",
                "      |",
                "      |",
                "      |",
                "========="
            };
            
            for(int i = 0; i < 7; i++) {
                TextOutA(hdc, 20, 130 + (i * 20), drawing[i], lstrlenA(drawing[i]));
            }

            SelectObject(hdc, hOldFont);
            DeleteObject(hMono);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_CTLCOLORBTN:
        case WM_CTLCOLORSTATIC: {
            HDC hdcBtn = (HDC)wParam;
            SetBkMode(hdcBtn, TRANSPARENT);
            SetTextColor(hdcBtn, RGB(255, 255, 255));
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KHangmanWindow";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,                              
        CLASS_NAME,                     
        "KHangman",                     
        WS_OVERLAPPEDWINDOW,            
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 500,
        NULL,                           
        NULL,                           
        hInstance,                      
        NULL                            
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
