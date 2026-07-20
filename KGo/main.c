#include <windows.h>

#define ID_BTN_PASS 101
#define ID_BTN_RESIGN 102
#define ID_BTN_NEW 103

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hBtnPass, hBtnResign, hBtnNew;

    switch (uMsg) {
        case WM_CREATE:
            hBtnPass = CreateWindow("BUTTON", "Pass", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                20, 370, 80, 30, hwnd, (HMENU)ID_BTN_PASS, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hBtnResign = CreateWindow("BUTTON", "Resign", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                110, 370, 80, 30, hwnd, (HMENU)ID_BTN_RESIGN, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hBtnNew = CreateWindow("BUTTON", "New Game", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                200, 370, 80, 30, hwnd, (HMENU)ID_BTN_NEW, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Dark theme background
            HBRUSH hBrush = CreateSolidBrush(RGB(18, 18, 18));
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);

            // Draw basic board skeleton
            int padding = 40;
            int cellSize = 30;
            int boardSize = 9;
            
            // Draw board background
            HBRUSH boardBrush = CreateSolidBrush(RGB(220, 179, 92)); // #dcb35c
            RECT boardRect = { padding, padding, padding + (boardSize-1)*cellSize, padding + (boardSize-1)*cellSize };
            // Adjust rect to cover cell edges
            boardRect.left -= 15; boardRect.top -= 15;
            boardRect.right += 15; boardRect.bottom += 15;
            FillRect(hdc, &boardRect, boardBrush);
            DeleteObject(boardBrush);

            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            SelectObject(hdc, hPen);
            for (int i = 0; i < boardSize; i++) {
                // Horizontal lines
                MoveToEx(hdc, padding, padding + i * cellSize, NULL);
                LineTo(hdc, padding + (boardSize - 1) * cellSize, padding + i * cellSize);
                // Vertical lines
                MoveToEx(hdc, padding + i * cellSize, padding, NULL);
                LineTo(hdc, padding + i * cellSize, padding + (boardSize - 1) * cellSize);
            }
            DeleteObject(hPen);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(224, 224, 224));
            TextOut(hdc, 20, 10, "KGo - Current Turn: Black", 25);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BTN_PASS || LOWORD(wParam) == ID_BTN_RESIGN || LOWORD(wParam) == ID_BTN_NEW) {
                // To be implemented in Phase 2+
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KGoWindowClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KGo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 470,
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
