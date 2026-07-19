#include <windows.h>

const char g_szClassName[] = "KConnect4WindowClass";

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            HBRUSH bg = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(hdc, &rect, bg);
            DeleteObject(bg);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));
            TextOut(hdc, 20, 20, "KConnect4 - Skeleton", 20);
            
            // Draw simple board background
            HBRUSH boardBg = CreateSolidBrush(RGB(0, 68, 204));
            RECT boardRect = {20, 50, 20 + 7 * 40, 50 + 6 * 40};
            FillRect(hdc, &boardRect, boardBg);
            DeleteObject(boardBg);
            
            // Draw cells
            HBRUSH emptyCell = CreateSolidBrush(RGB(30, 30, 30));
            for (int r = 0; r < 6; r++) {
                for (int c = 0; c < 7; c++) {
                    int x = 20 + c * 40;
                    int y = 50 + r * 40;
                    SelectObject(hdc, emptyCell);
                    SelectObject(hdc, GetStockObject(NULL_PEN));
                    Ellipse(hdc, x + 2, y + 2, x + 38, y + 38);
                }
            }
            DeleteObject(emptyCell);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
        0, g_szClassName, "KConnect4",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 400,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
