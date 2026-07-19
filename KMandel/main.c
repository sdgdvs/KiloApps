#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <commdlg.h>

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
int isJulia = 0;
double juliaCRe = 0.0, juliaCIm = 0.0;

#define MAX_HISTORY 256
typedef struct {
    double minRe, maxRe, minIm, maxIm;
    unsigned int max_iter;
    int isJulia;
    double juliaCRe, juliaCIm;
    int theme;
} ViewState;

ViewState history[MAX_HISTORY];
int history_idx = -1;
int history_max = -1;

void SaveState() {
    if (history_idx < MAX_HISTORY - 1) {
        history_idx++;
    } else {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            history[i] = history[i+1];
        }
    }
    history[history_idx].minRe = minRe;
    history[history_idx].maxRe = maxRe;
    history[history_idx].minIm = minIm;
    history[history_idx].maxIm = maxIm;
    history[history_idx].max_iter = max_iter;
    history[history_idx].isJulia = isJulia;
    history[history_idx].juliaCRe = juliaCRe;
    history[history_idx].juliaCIm = juliaCIm;
    history[history_idx].theme = theme;
    history_max = history_idx;
}

void LoadState(int idx) {
    if (idx >= 0 && idx <= history_max) {
        minRe = history[idx].minRe;
        maxRe = history[idx].maxRe;
        minIm = history[idx].minIm;
        maxIm = history[idx].maxIm;
        max_iter = history[idx].max_iter;
        isJulia = history[idx].isJulia;
        juliaCRe = history[idx].juliaCRe;
        juliaCIm = history[idx].juliaCIm;
        theme = history[idx].theme;
    }
}

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
        double c_im_view = maxIm - y * im_factor;
        for (int x = 0; x < width; ++x) {
            double c_re_view = minRe + x * re_factor;
            double c_re = isJulia ? juliaCRe : c_re_view;
            double c_im = isJulia ? juliaCIm : c_im_view;
            double Z_re = c_re_view, Z_im = c_im_view;
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

void SaveImage(HWND hwnd) {
    if (!pixels || bmpW <= 0 || bmpH <= 0) return;
    OPENFILENAME ofn = {0};
    char szFileName[MAX_PATH] = "kmandel.bmp";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "bmp";
    
    if (GetSaveFileName(&ofn)) {
        HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            BITMAPFILEHEADER bfh = {0};
            BITMAPINFOHEADER bih = {0};
            int imageSize32 = bmpW * bmpH * 4;
            
            bfh.bfType = 0x4D42;
            bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageSize32;
            bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
            
            bih.biSize = sizeof(BITMAPINFOHEADER);
            bih.biWidth = bmpW;
            bih.biHeight = -bmpH;
            bih.biPlanes = 1;
            bih.biBitCount = 32;
            bih.biCompression = BI_RGB;
            bih.biSizeImage = imageSize32;
            
            DWORD dwWritten;
            WriteFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
            WriteFile(hFile, &bih, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
            WriteFile(hFile, pixels, imageSize32, &dwWritten, NULL);
            CloseHandle(hFile);
        }
    }
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
            SaveState();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_RBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            Zoom(2.0, x, y); // Zoom out
            SaveState();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_KEYDOWN: {
            if (wParam == 'R') {
                if (isJulia) {
                    minRe = -2.0; maxRe = 2.0;
                    minIm = -2.0; maxIm = 2.0;
                } else {
                    minRe = -2.0; maxRe = 1.0;
                    minIm = -1.2; maxIm = 1.2;
                }
                max_iter = 100;
                SaveState();
                RenderMandelbrot(bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'T') {
                theme = (theme + 1) % 4;
                SaveState();
                RenderMandelbrot(bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'J') {
                if (!isJulia) {
                    isJulia = 1;
                    juliaCRe = minRe + (maxRe - minRe) / 2.0;
                    juliaCIm = minIm + (maxIm - minIm) / 2.0;
                    minRe = -2.0; maxRe = 2.0;
                    minIm = -2.0; maxIm = 2.0;
                } else {
                    isJulia = 0;
                    minRe = -2.0; maxRe = 1.0;
                    minIm = -1.2; maxIm = 1.2;
                }
                max_iter = 100;
                SaveState();
                RenderMandelbrot(bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'Z') {
                if (history_idx > 0) {
                    history_idx--;
                    LoadState(history_idx);
                    RenderMandelbrot(bmpW, bmpH);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 'Y') {
                if (history_idx < history_max) {
                    history_idx++;
                    LoadState(history_idx);
                    RenderMandelbrot(bmpW, bmpH);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 'S') {
                SaveImage(hwnd);
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

    HWND hwnd = CreateWindowEx(0, "KMandelApp", "KMandel - L/R Click: Zoom, Z/Y: Undo/Redo, S: Save, R: Reset, T: Theme, J: Julia", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    SaveState(); // Save initial state

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
