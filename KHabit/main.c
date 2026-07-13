#include <windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                // Set background to a dark color
                HBRUSH hBrush = CreateSolidBrush(RGB(18, 18, 18));
                FillRect(hdc, &ps.rcPaint, hBrush);
                DeleteObject(hBrush);

                // Draw title
                SetTextColor(hdc, RGB(243, 244, 246));
                SetBkMode(hdc, TRANSPARENT);
                HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
                SelectObject(hdc, hFont);
                TextOut(hdc, 20, 20, "KHabit Tracker", 14);
                DeleteObject(hFont);
                
                // Draw a sample habit card
                HBRUSH hCardBrush = CreateSolidBrush(RGB(40, 40, 50));
                RECT cardRect = {20, 60, 400, 140};
                FillRect(hdc, &cardRect, hCardBrush);
                DeleteObject(hCardBrush);
                
                HFONT hHabitFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
                SelectObject(hdc, hHabitFont);
                TextOut(hdc, 30, 70, "Read 20 pages", 13);
                
                HFONT hSmallFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
                SelectObject(hdc, hSmallFont);
                SetTextColor(hdc, RGB(156, 163, 175));
                TextOut(hdc, 30, 95, "Daily reading goal", 18);
                
                // Draw checkboxes for days
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(99, 102, 241));
                SelectObject(hdc, hPen);
                
                int startX = 200;
                char* days[] = {"M", "T", "W", "T", "F", "S", "S"};
                for(int i = 0; i < 7; i++) {
                    // Check first two days
                    if(i < 2) {
                        HBRUSH hCheckBrush = CreateSolidBrush(RGB(99, 102, 241));
                        SelectObject(hdc, hCheckBrush);
                        Ellipse(hdc, startX + i*25, 80, startX + i*25 + 20, 100);
                        DeleteObject(hCheckBrush);
                    } else {
                        SelectObject(hdc, GetStockObject(NULL_BRUSH));
                        Ellipse(hdc, startX + i*25, 80, startX + i*25 + 20, 100);
                    }
                }
                
                DeleteObject(hPen);
                DeleteObject(hHabitFont);
                DeleteObject(hSmallFont);

                EndPaint(hwnd, &ps);
            }
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KHabitWindowClass";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "KHabit Tracker",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 450, 250,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
