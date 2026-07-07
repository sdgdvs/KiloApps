#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 400
#define H 400

int _fltused = 0;

HBITMAP hBitmap = NULL;
DWORD* pixels = NULL;
int bmpW = 0, bmpH = 0;

void RenderMandelbrot(int width, int height) {
    if (!pixels) return;
    
    double minRe = -2.0, maxRe = 1.0;
    double minIm = -1.2, maxIm = 1.2;
    double re_factor = (maxRe - minRe) / (width - 1);
    double im_factor = (maxIm - minIm) / (height - 1);
    unsigned int max_iter = 100;
    
    for (int y = 0; y < height; ++y) {
        double c_im = maxIm - y * im_factor;
        for (int x = 0; x < width; ++x) {
            double c_re = minRe + x * re_factor;
            double Z_re = c_re, Z_im = c_im;
            int isInside = 1;
            unsigned int n = 0;
            for (n = 0; n < max_iter; ++n) {
                double Z_re2 = Z_re * Z_re, Z_im2 = Z_im * Z_im;
                if (Z_re2 + Z_im2 > 4) {
                    isInside = 0;
                    break;
                }
                Z_im = 2 * Z_re * Z_im + c_im;
                Z_re = Z_re2 - Z_im2 + c_re;
            }
            if (isInside) {
                pixels[y * width + x] = 0; // Black
            } else {
                // Smooth coloring based on iteration count
                unsigned char r = (unsigned char)((n * 255) / max_iter);
                unsigned char g = (unsigned char)((n * n * 255) / (max_iter * max_iter));
                unsigned char b = (unsigned char)((n * 128) / max_iter);
                pixels[y * width + x] = (r << 16) | (g << 8) | b;
            }
        }
    }
}

void ResizeBitmap(HWND hwnd, int width, int height) {
    if (width == 0 || height == 0) return;
    if (hBitmap) DeleteObject(hBitmap);
    
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    HDC hdc = GetDC(hwnd);
    hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
    ReleaseDC(hwnd, hdc);
    
    bmpW = width;
    bmpH = height;
    
    if (pixels) RenderMandelbrot(width, height);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            ResizeBitmap(hwnd, nw, nh);
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (hBitmap) {
                HDC hdcMem = CreateCompatibleDC(hdc);
                HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBitmap);
                BitBlt(hdc, 0, 0, bmpW, bmpH, hdcMem, 0, 0, SRCCOPY);
                SelectObject(hdcMem, hOld);
                DeleteDC(hdcMem);
            }
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_ERASEBKGND:
            return 1; // Handled in WM_PAINT
        case WM_DESTROY:
            if (hBitmap) DeleteObject(hBitmap);
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

// Convert double to float for basic math if needed but MSVC handles double natively well enough.
#pragma function(floor)
double __cdecl floor(double x) {
    return (double)((int)x);
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KMandelApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KMandelApp", "KMandel", WS_OVERLAPPEDWINDOW,
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
