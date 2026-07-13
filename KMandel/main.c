#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>

#define W 400
#define H 400

int _fltused = 0;

HBITMAP hBitmap = NULL;
DWORD* pixels = NULL;
int bmpW = 0, bmpH = 0;

double minRe = -2.0, maxRe = 1.0;
double minIm = -1.2, maxIm = 1.2;
unsigned int max_iter = 100;
int theme = 0; // 0: Fire, 1: Ocean, 2: Cyberpunk, 3: BW

void GetColors(unsigned int n, unsigned int iter, int t, unsigned char* r, unsigned char* g, unsigned char* b) {
    if (t == 0) { // Fire
        *r = (unsigned char)((n * 255) / iter);
        *g = (unsigned char)((n * n * 255) / (iter * iter));
        *b = (unsigned char)((n * 128) / iter);
    } else if (t == 1) { // Ocean
        *r = (unsigned char)((n * 128) / iter);
        *g = (unsigned char)((n * n * 255) / (iter * iter));
        *b = (unsigned char)((n * 255) / iter);
    } else if (t == 2) { // Cyberpunk
        double f = (double)n / iter;
        double vr = sin(f * 3.14159) * 255;
        double vg = sin(f * 3.14159 * 2) * 128;
        double vb = cos(f * 3.14159) * 255;
        *r = (unsigned char)(vr < 0 ? 0 : (vr > 255 ? 255 : vr));
        *g = (unsigned char)(vg < 0 ? 0 : (vg > 255 ? 255 : vg));
        *b = (unsigned char)(vb < 0 ? 0 : (vb > 255 ? 255 : vb));
    } else { // BW
        unsigned char v = ((n % 20) > 10) ? 255 : 0;
        *r = v; *g = v; *b = v;
    }
}

void RenderMandelbrot(int width, int height) {
    if (!pixels || width <= 1 || height <= 1) return;
    
    double re_factor = (maxRe - minRe) / (width - 1);
    double im_factor = (maxIm - minIm) / (height - 1);
    
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
                unsigned char r, g, b;
                GetColors(n, max_iter, theme, &r, &g, &b);
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

void Zoom(double factor, int mouseX, int mouseY) {
    if (bmpW <= 0 || bmpH <= 0) return;
    
    double re_factor = (maxRe - minRe) / (bmpW - 1);
    double im_factor = (maxIm - minIm) / (bmpH - 1);
    
    double centerRe = minRe + mouseX * re_factor;
    double centerIm = maxIm - mouseY * im_factor;
    
    double newWRe = (maxRe - minRe) * factor;
    double newWIm = (maxIm - minIm) * factor;
    
    minRe = centerRe - ((double)mouseX / bmpW) * newWRe;
    maxRe = minRe + newWRe;
    
    maxIm = centerIm + ((double)mouseY / bmpH) * newWIm;
    minIm = maxIm - newWIm;
    
    double zoomLevel = 3.0 / newWRe;
    unsigned int needed_iter = (unsigned int)(100 * pow(zoomLevel, 0.2));
    if (needed_iter > max_iter && zoomLevel > 5.0) {
        max_iter = needed_iter;
        if (max_iter > 2000) max_iter = 2000;
    }
    
    RenderMandelbrot(bmpW, bmpH);
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
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            Zoom(0.5, x, y); // Zoom in
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_RBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            Zoom(2.0, x, y); // Zoom out
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == 'R') {
                minRe = -2.0; maxRe = 1.0;
                minIm = -1.2; maxIm = 1.2;
                max_iter = 100;
                RenderMandelbrot(bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'T') {
                theme = (theme + 1) % 4;
                RenderMandelbrot(bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            }
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

#pragma function(floor)
double __cdecl floor(double x) {
    return (double)((int)x);
}

// Since we use sin and cos now, we just link against standard math functions (which msvcrt provides).
// The linker options in build.bat should handle it (no special libs needed for basic math if not using /NODEFAULTLIB fully, 
// wait, build.bat has /NODEFAULTLIB! Let's check how build.bat is defined).

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KMandelApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KMandelApp", "KMandel - LClick: In, RClick: Out, R: Reset, T: Theme", WS_OVERLAPPEDWINDOW,
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
