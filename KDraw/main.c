#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int W = 500, H = 400;
HDC hdcMem = NULL;
HBITMAP hbmMem = NULL;
COLORREF currentColor = RGB(255, 255, 255);
int currentSize = 2;
int isDrawing = 0;
int lastX = 0, lastY = 0;
HPEN currentPen = NULL;

// New color palette: white, red, green, blue, yellow, magenta, cyan, black
const COLORREF colors[] = {
    RGB(255, 255, 255), RGB(255, 85, 85), RGB(85, 255, 85), RGB(85, 85, 255),
    RGB(255, 255, 85), RGB(255, 85, 255), RGB(85, 255, 255), RGB(0, 0, 0)
};

void ClearCanvas(HDC hdc) {
    RECT rc = {0, 0, W, H};
    HBRUSH bg = CreateSolidBrush(RGB(26, 30, 35)); // #1a1e23
    FillRect(hdc, &rc, bg);
    DeleteObject(bg);
}

void SetupMemDC(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    if (hdcMem) { DeleteDC(hdcMem); DeleteObject(hbmMem); }
    hdcMem = CreateCompatibleDC(hdc);
    hbmMem = CreateCompatibleBitmap(hdc, W, H);
    SelectObject(hdcMem, hbmMem);
    
    ClearCanvas(hdcMem);
    
    currentPen = CreatePen(PS_SOLID, currentSize, currentColor);
    SelectObject(hdcMem, currentPen);
    
    ReleaseDC(hwnd, hdc);
}

void DrawToolbar(HDC hdc) {
    // Background for toolbar
    RECT tbRc = {0, 0, W, 45};
    HBRUSH tbBg = CreateSolidBrush(RGB(40, 44, 52));
    FillRect(hdc, &tbRc, tbBg);
    DeleteObject(tbBg);

    // Draw colors
    for (int i = 0; i < 8; i++) {
        RECT r = {i * 30 + 10, 10, i * 30 + 35, 35};
        HBRUSH br = CreateSolidBrush(colors[i]);
        FillRect(hdc, &r, br);
        DeleteObject(br);
        if (currentColor == colors[i]) {
            DrawEdge(hdc, &r, BDR_SUNKENOUTER, BF_RECT);
        } else {
            HBRUSH border = CreateSolidBrush(RGB(80, 80, 80));
            FrameRect(hdc, &r, border);
            DeleteObject(border);
        }
    }
    
    // Draw Size button (toggle)
    RECT sizeRc = {270, 10, 330, 35};
    HBRUSH sizeBg = CreateSolidBrush(currentSize > 2 ? RGB(100, 150, 255) : RGB(80, 80, 80));
    FillRect(hdc, &sizeRc, sizeBg);
    DeleteObject(sizeBg);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawTextA(hdc, currentSize > 2 ? "Thick" : "Thin", -1, &sizeRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // Draw Clear button
    RECT clearRc = {340, 10, 400, 35};
    HBRUSH clearBg = CreateSolidBrush(RGB(80, 80, 80));
    FillRect(hdc, &clearRc, clearBg);
    DeleteObject(clearBg);
    DrawTextA(hdc, "Clear", -1, &clearRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            SetupMemDC(hwnd);
            break;
        case WM_SIZE:
            break;
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            if (y >= 0 && y <= 45) { // Toolbar area
                if (y >= 10 && y <= 35) {
                    if (x >= 10 && x < 10 + 8 * 30) {
                        // Clicked a color
                        int idx = (x - 10) / 30;
                        if (idx >= 0 && idx < 8) {
                            currentColor = colors[idx];
                            if (currentPen) DeleteObject(currentPen);
                            currentPen = CreatePen(PS_SOLID, currentSize, currentColor);
                            SelectObject(hdcMem, currentPen);
                            RECT pRc = {0, 0, W, 45};
                            InvalidateRect(hwnd, &pRc, FALSE);
                        }
                    } else if (x >= 270 && x <= 330) {
                        // Clicked Size toggle
                        currentSize = currentSize == 2 ? 8 : 2;
                        if (currentPen) DeleteObject(currentPen);
                        currentPen = CreatePen(PS_SOLID, currentSize, currentColor);
                        SelectObject(hdcMem, currentPen);
                        RECT pRc = {270, 10, 330, 35};
                        InvalidateRect(hwnd, &pRc, FALSE);
                    } else if (x >= 340 && x <= 400) {
                        // Clicked Clear
                        ClearCanvas(hdcMem);
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
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
            BitBlt(hdc, 0, 45, W, H-45, hdcMem, 0, 45, SRCCOPY);
            DrawToolbar(hdc);
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
    wc.hbrBackground = CreateSolidBrush(RGB(40, 44, 52));
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
