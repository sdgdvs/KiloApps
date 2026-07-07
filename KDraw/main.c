#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int W = 500, H = 400;
HDC hdcMem = NULL;
HBITMAP hbmMem = NULL;
COLORREF currentColor = RGB(0, 0, 0);
int isDrawing = 0;
int lastX = 0, lastY = 0;
HPEN currentPen = NULL;

const COLORREF colors[] = {
    RGB(0, 0, 0), RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255),
    RGB(255, 255, 0), RGB(255, 0, 255), RGB(0, 255, 255), RGB(255, 255, 255)
};

void SetupMemDC(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    if (hdcMem) { DeleteDC(hdcMem); DeleteObject(hbmMem); }
    hdcMem = CreateCompatibleDC(hdc);
    hbmMem = CreateCompatibleBitmap(hdc, W, H);
    SelectObject(hdcMem, hbmMem);
    
    RECT rc = {0, 0, W, H};
    HBRUSH bg = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdcMem, &rc, bg);
    DeleteObject(bg);
    
    currentPen = CreatePen(PS_SOLID, 2, currentColor);
    SelectObject(hdcMem, currentPen);
    
    ReleaseDC(hwnd, hdc);
}

void DrawColorPalette(HDC hdc) {
    for (int i = 0; i < 8; i++) {
        RECT r = {i * 30 + 10, 10, i * 30 + 35, 35};
        HBRUSH br = CreateSolidBrush(colors[i]);
        FillRect(hdc, &r, br);
        DeleteObject(br);
        if (currentColor == colors[i]) {
            DrawEdge(hdc, &r, BDR_SUNKENOUTER, BF_RECT);
        } else {
            DrawEdge(hdc, &r, BDR_RAISEDINNER, BF_RECT);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            SetupMemDC(hwnd);
            break;
        case WM_SIZE:
            // Minimalist: we don't resize the canvas in this simple version
            break;
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            if (y >= 10 && y <= 35 && x >= 10 && x < 10 + 8 * 30) {
                int idx = (x - 10) / 30;
                if (idx >= 0 && idx < 8) {
                    currentColor = colors[idx];
                    if (currentPen) DeleteObject(currentPen);
                    currentPen = CreatePen(PS_SOLID, 2, currentColor);
                    SelectObject(hdcMem, currentPen);
                    
                    RECT pRc = {0, 0, W, 45};
                    InvalidateRect(hwnd, &pRc, FALSE);
                }
            } else {
                isDrawing = 1;
                lastX = x;
                lastY = y;
                SetCapture(hwnd);
            }
            break;
        }
        case WM_MOUSEMOVE:
            if (isDrawing) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                MoveToEx(hdcMem, lastX, lastY, NULL);
                LineTo(hdcMem, x, y);
                lastX = x;
                lastY = y;
                
                HDC hdc = GetDC(hwnd);
                MoveToEx(hdc, lastX, lastY, NULL); // draw locally for speed
                HPEN old = (HPEN)SelectObject(hdc, currentPen);
                MoveToEx(hdc, lastX, lastY, NULL);
                LineTo(hdc, x, y);
                SelectObject(hdc, old);
                ReleaseDC(hwnd, hdc);
            }
            break;
        case WM_LBUTTONUP:
            if (isDrawing) {
                isDrawing = 0;
                ReleaseCapture();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            BitBlt(hdc, 0, 0, W, H, hdcMem, 0, 0, SRCCOPY);
            DrawColorPalette(hdc);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            if (currentPen) DeleteObject(currentPen);
            if (hdcMem) { DeleteDC(hdcMem); DeleteObject(hbmMem); }
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KDrawApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KDrawApp", "KDraw", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W + 16, H + 39, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
