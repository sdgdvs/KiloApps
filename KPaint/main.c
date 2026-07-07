#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

int isPainting = 0;
HDC hdcWindow = NULL;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hdcWindow = GetDC(hwnd);
            // Create a slightly thicker pen for drawing
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0,0,0));
            SelectObject(hdcWindow, hPen);
            break;
        }
        case WM_LBUTTONDOWN: {
            isPainting = 1;
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            MoveToEx(hdcWindow, x, y, NULL);
            break;
        }
        case WM_MOUSEMOVE: {
            if (isPainting) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                LineTo(hdcWindow, x, y);
            }
            break;
        }
        case WM_LBUTTONUP: {
            isPainting = 0;
            break;
        }
        case WM_DESTROY:
            ReleaseDC(hwnd, hdcWindow);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KPaintClass";
    wc.hCursor = LoadCursorA(NULL, IDC_CROSS);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // White background

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KPaintClass", "KPaint", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
