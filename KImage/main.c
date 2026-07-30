#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#define WINDOW_WIDTH 920
#define WINDOW_HEIGHT 640
#define TOOLBAR_HEIGHT 42
#define SIDEBAR_WIDTH 170
#define MAX_FILES 256
#define TIMER_SLIDESHOW 1001

// UI Control IDs
#define ID_BTN_OPEN        101
#define ID_BTN_SAVE        102
#define ID_BTN_ZOOM_IN     103
#define ID_BTN_ZOOM_OUT    104
#define ID_BTN_ZOOM_RESET  105
#define ID_BTN_ROT_CCW     106
#define ID_BTN_ROT_CW      107
#define ID_BTN_FLIP_H      108
#define ID_BTN_FLIP_V      109
#define ID_BTN_GRAYSCALE   110
#define ID_BTN_SEPIA       111
#define ID_BTN_INVERT      112
#define ID_BTN_BLUR        113
#define ID_BTN_BRIGHT_UP   114
#define ID_BTN_BRIGHT_DOWN 115
#define ID_BTN_RESET       116
#define ID_BTN_CROP        117
#define ID_BTN_RESIZE_HALF 118
#define ID_BTN_RESIZE_DOUBLE 119
#define ID_BTN_DRAW        120
#define ID_BTN_PREV        121
#define ID_BTN_PLAY        122
#define ID_BTN_NEXT        123

// Global State
HBITMAP g_hBmpWork = NULL;
HBITMAP g_hBmpOrig = NULL;
RGBQUAD* g_pBitsWork = NULL;
RGBQUAD* g_pBitsOrig = NULL;
int g_bmpW = 0;
int g_bmpH = 0;
int g_origW = 0;
int g_origH = 0;
float g_zoom = 1.0f;

// Tool & Interaction Modes
int g_drawMode = 0;
int g_isDrawing = 0;
POINT g_lastPt;
HPEN g_hPenDraw = NULL;

int g_cropMode = 0;
int g_isCropping = 0;
RECT g_cropRect = {0};

// Directory Playlist & Slideshow
char g_fileList[MAX_FILES][MAX_PATH];
int g_fileCount = 0;
int g_fileIndex = -1;
char g_currentDir[MAX_PATH] = {0};
int g_slideshowPlaying = 0;

// Controls
HWND g_hBtnPlay = NULL;
HWND g_hBtnCrop = NULL;
HWND g_hBtnDraw = NULL;

// CRT Helper Implementations for no-CRT link
#pragma function(memset)
void* __cdecl memset(void* dest, int c, size_t count) {
    char* bytes = (char*)dest;
    while (count--) *bytes++ = (char)c;
    return dest;
}

#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

char* FindLastChar(const char* str, char ch) {
    char* last = NULL;
    while (*str) {
        if (*str == ch) last = (char*)str;
        str++;
    }
    return last;
}

int _fltused = 1;
long _ftol2_sse(float f) { return (long)f; }
long _ftol2(float f) { return (long)f; }

// Helper function to create top-down 32-bit DIB Section
HBITMAP Create32BitDIB(int w, int h, RGBQUAD** ppBits) {
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(NULL);
    HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)ppBits, NULL, 0);
    ReleaseDC(NULL, hdc);
    return hBmp;
}

// Clone DIB Section
void CopyBitmapToWork(HBITMAP hSrc, int w, int h) {
    if (g_hBmpWork) DeleteObject(g_hBmpWork);
    g_hBmpWork = Create32BitDIB(w, h, &g_pBitsWork);
    g_bmpW = w;
    g_bmpH = h;

    HDC hdcSrc = CreateCompatibleDC(NULL);
    HDC hdcDst = CreateCompatibleDC(NULL);
    HGDIOBJ oldSrc = SelectObject(hdcSrc, hSrc);
    HGDIOBJ oldDst = SelectObject(hdcDst, g_hBmpWork);

    BitBlt(hdcDst, 0, 0, w, h, hdcSrc, 0, 0, SRCCOPY);

    SelectObject(hdcSrc, oldSrc);
    SelectObject(hdcDst, oldDst);
    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);
}

// Load BMP and build playlist
void LoadBitmapFile(HWND hwnd, const char* szFile) {
    HBITMAP hLoaded = (HBITMAP)LoadImageA(NULL, szFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (!hLoaded) {
        MessageBoxA(hwnd, "Failed to load image. Ensure file is a valid 24-bit/32-bit BMP.", "KImage Error", MB_OK | MB_ICONERROR);
        return;
    }

    BITMAP bm;
    GetObject(hLoaded, sizeof(bm), &bm);
    int w = bm.bmWidth;
    int h = bm.bmHeight;

    if (w <= 0 || h <= 0 || w > 8192 || h > 8192) {
        DeleteObject(hLoaded);
        MessageBoxA(hwnd, "Image dimensions out of bounds (max 8192x8192).", "KImage Error", MB_OK | MB_ICONERROR);
        return;
    }

    if (g_hBmpOrig) DeleteObject(g_hBmpOrig);
    g_hBmpOrig = Create32BitDIB(w, h, &g_pBitsOrig);
    g_origW = w;
    g_origH = h;

    HDC hdcSrc = CreateCompatibleDC(NULL);
    HDC hdcDst = CreateCompatibleDC(NULL);
    HGDIOBJ oldSrc = SelectObject(hdcSrc, hLoaded);
    HGDIOBJ oldDst = SelectObject(hdcDst, g_hBmpOrig);

    BitBlt(hdcDst, 0, 0, w, h, hdcSrc, 0, 0, SRCCOPY);

    SelectObject(hdcSrc, oldSrc);
    SelectObject(hdcDst, oldDst);
    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);
    DeleteObject(hLoaded);

    CopyBitmapToWork(g_hBmpOrig, w, h);
    g_zoom = 1.0f;
    g_cropMode = 0;
    if (g_hBtnCrop) SendMessage(g_hBtnCrop, BM_SETCHECK, BST_UNCHECKED, 0);

    SetWindowTextA(hwnd, szFile);
    InvalidateRect(hwnd, NULL, TRUE);
}

// Scan directory for BMP files
void ScanDirectoryForPlaylist(const char* szFile) {
    char dir[MAX_PATH];
    lstrcpyA(dir, szFile);
    char* lastSlash = FindLastChar(dir, '\\');
    if (!lastSlash) lastSlash = FindLastChar(dir, '/');
    if (lastSlash) {
        *lastSlash = '\0';
        lstrcpyA(g_currentDir, dir);
        lstrcatA(dir, "\\*.bmp");

        g_fileCount = 0;
        g_fileIndex = -1;
        WIN32_FIND_DATAA fd;
        HANDLE hFind = FindFirstFileA(dir, &fd);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    char fullPath[MAX_PATH];
                    lstrcpyA(fullPath, g_currentDir);
                    lstrcatA(fullPath, "\\");
                    lstrcatA(fullPath, fd.cFileName);
                    lstrcpyA(g_fileList[g_fileCount], fullPath);

                    if (lstrcmpiA(fullPath, szFile) == 0) {
                        g_fileIndex = g_fileCount;
                    }
                    g_fileCount++;
                    if (g_fileCount >= MAX_FILES) break;
                }
            } while (FindNextFileA(hFind, &fd));
            FindClose(hFind);
        }
    }
}

void OpenFileDlg(HWND hwnd) {
    OPENFILENAMEA ofn = {0};
    char szFile[MAX_PATH] = {0};
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileNameA(&ofn) == TRUE) {
        LoadBitmapFile(hwnd, szFile);
        ScanDirectoryForPlaylist(szFile);
    }
}

void SaveFileDlg(HWND hwnd) {
    if (!g_hBmpWork || !g_pBitsWork) return;

    OPENFILENAMEA ofn = {0};
    char szFile[MAX_PATH] = "edited_image.bmp";

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Bitmap Files (*.bmp)\0*.bmp\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn) == TRUE) {
        BITMAPFILEHEADER bfh = {0};
        bfh.bfType = 0x4D42; // "BM"
        bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bfh.bfSize = bfh.bfOffBits + g_bmpW * g_bmpH * 4;

        BITMAPINFOHEADER bih = {0};
        bih.biSize = sizeof(BITMAPINFOHEADER);
        bih.biWidth = g_bmpW;
        bih.biHeight = -g_bmpH; // top-down
        bih.biPlanes = 1;
        bih.biBitCount = 32;
        bih.biCompression = BI_RGB;
        bih.biSizeImage = g_bmpW * g_bmpH * 4;

        HANDLE hFile = CreateFileA(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteFile(hFile, &bfh, sizeof(bfh), &written, NULL);
            WriteFile(hFile, &bih, sizeof(bih), &written, NULL);
            WriteFile(hFile, g_pBitsWork, g_bmpW * g_bmpH * 4, &written, NULL);
            CloseHandle(hFile);
            MessageBoxA(hwnd, "Image saved successfully!", "KImage Export", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBoxA(hwnd, "Failed to save file.", "KImage Error", MB_OK | MB_ICONERROR);
        }
    }
}

// Image Filters
void FilterGrayscale() {
    if (!g_pBitsWork) return;
    int count = g_bmpW * g_bmpH;
    for (int i = 0; i < count; i++) {
        BYTE b = g_pBitsWork[i].rgbBlue;
        BYTE g = g_pBitsWork[i].rgbGreen;
        BYTE r = g_pBitsWork[i].rgbRed;
        BYTE gray = (BYTE)(0.299f * r + 0.587f * g + 0.114f * b);
        g_pBitsWork[i].rgbBlue = gray;
        g_pBitsWork[i].rgbGreen = gray;
        g_pBitsWork[i].rgbRed = gray;
    }
}

void FilterSepia() {
    if (!g_pBitsWork) return;
    int count = g_bmpW * g_bmpH;
    for (int i = 0; i < count; i++) {
        BYTE b = g_pBitsWork[i].rgbBlue;
        BYTE g = g_pBitsWork[i].rgbGreen;
        BYTE r = g_pBitsWork[i].rgbRed;
        int tr = (int)(0.393f * r + 0.769f * g + 0.189f * b);
        int tg = (int)(0.349f * r + 0.686f * g + 0.168f * b);
        int tb = (int)(0.272f * r + 0.534f * g + 0.131f * b);
        g_pBitsWork[i].rgbRed   = (BYTE)(tr > 255 ? 255 : tr);
        g_pBitsWork[i].rgbGreen = (BYTE)(tg > 255 ? 255 : tg);
        g_pBitsWork[i].rgbBlue  = (BYTE)(tb > 255 ? 255 : tb);
    }
}

void FilterInvert() {
    if (!g_pBitsWork) return;
    int count = g_bmpW * g_bmpH;
    for (int i = 0; i < count; i++) {
        g_pBitsWork[i].rgbBlue  = 255 - g_pBitsWork[i].rgbBlue;
        g_pBitsWork[i].rgbGreen = 255 - g_pBitsWork[i].rgbGreen;
        g_pBitsWork[i].rgbRed   = 255 - g_pBitsWork[i].rgbRed;
    }
}

void FilterBrightness(int delta) {
    if (!g_pBitsWork) return;
    int count = g_bmpW * g_bmpH;
    for (int i = 0; i < count; i++) {
        int b = g_pBitsWork[i].rgbBlue + delta;
        int g = g_pBitsWork[i].rgbGreen + delta;
        int r = g_pBitsWork[i].rgbRed + delta;
        g_pBitsWork[i].rgbBlue  = (BYTE)(b < 0 ? 0 : (b > 255 ? 255 : b));
        g_pBitsWork[i].rgbGreen = (BYTE)(g < 0 ? 0 : (g > 255 ? 255 : g));
        g_pBitsWork[i].rgbRed   = (BYTE)(r < 0 ? 0 : (r > 255 ? 255 : r));
    }
}

void FilterBlur() {
    if (!g_pBitsWork || g_bmpW < 3 || g_bmpH < 3) return;
    if ((long long)g_bmpW * g_bmpH * sizeof(RGBQUAD) > 256 * 1024 * 1024) return;
    SIZE_T bufSize = g_bmpW * g_bmpH * sizeof(RGBQUAD);
    RGBQUAD* temp = (RGBQUAD*)HeapAlloc(GetProcessHeap(), 0, bufSize);
    if (!temp) return;
    memcpy(temp, g_pBitsWork, bufSize);

    for (int y = 1; y < g_bmpH - 1; y++) {
        for (int x = 1; x < g_bmpW - 1; x++) {
            int r = 0, g = 0, b = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    RGBQUAD p = temp[(y + dy) * g_bmpW + (x + dx)];
                    r += p.rgbRed; g += p.rgbGreen; b += p.rgbBlue;
                }
            }
            g_pBitsWork[y * g_bmpW + x].rgbRed   = (BYTE)(r / 9);
            g_pBitsWork[y * g_bmpW + x].rgbGreen = (BYTE)(g / 9);
            g_pBitsWork[y * g_bmpW + x].rgbBlue  = (BYTE)(b / 9);
        }
    }
    HeapFree(GetProcessHeap(), 0, temp);
}

// Transformations
void RotateImage90(int cw) {
    if (!g_pBitsWork) return;
    int nw = g_bmpH;
    int nh = g_bmpW;
    RGBQUAD* pNewBits = NULL;
    HBITMAP hNewBmp = Create32BitDIB(nw, nh, &pNewBits);

    for (int y = 0; y < g_bmpH; y++) {
        for (int x = 0; x < g_bmpW; x++) {
            int nx = cw ? (g_bmpH - 1 - y) : y;
            int ny = cw ? x : (g_bmpW - 1 - x);
            pNewBits[ny * nw + nx] = g_pBitsWork[y * g_bmpW + x];
        }
    }

    DeleteObject(g_hBmpWork);
    g_hBmpWork = hNewBmp;
    g_pBitsWork = pNewBits;
    g_bmpW = nw;
    g_bmpH = nh;
}

void FlipImage(int horiz) {
    if (!g_pBitsWork) return;
    if (horiz) {
        for (int y = 0; y < g_bmpH; y++) {
            for (int x = 0; x < g_bmpW / 2; x++) {
                RGBQUAD tmp = g_pBitsWork[y * g_bmpW + x];
                g_pBitsWork[y * g_bmpW + x] = g_pBitsWork[y * g_bmpW + (g_bmpW - 1 - x)];
                g_pBitsWork[y * g_bmpW + (g_bmpW - 1 - x)] = tmp;
            }
        }
    } else {
        for (int y = 0; y < g_bmpH / 2; y++) {
            for (int x = 0; x < g_bmpW; x++) {
                RGBQUAD tmp = g_pBitsWork[y * g_bmpW + x];
                g_pBitsWork[y * g_bmpW + x] = g_pBitsWork[(g_bmpH - 1 - y) * g_bmpW + x];
                g_pBitsWork[(g_bmpH - 1 - y) * g_bmpW + x] = tmp;
            }
        }
    }
}

void ResizeImageScale(float factor) {
    if (!g_pBitsWork || factor <= 0.0f) return;
    int nw = (int)(g_bmpW * factor);
    int nh = (int)(g_bmpH * factor);
    if (nw < 1) nw = 1;
    if (nh < 1) nh = 1;
    if (nw > 8192 || nh > 8192) return; // Dimension scaling safety bounds

    RGBQUAD* pNewBits = NULL;
    HBITMAP hNewBmp = Create32BitDIB(nw, nh, &pNewBits);

    for (int y = 0; y < nh; y++) {
        int sy = y * g_bmpH / nh;
        for (int x = 0; x < nw; x++) {
            int sx = x * g_bmpW / nw;
            pNewBits[y * nw + x] = g_pBitsWork[sy * g_bmpW + sx];
        }
    }

    DeleteObject(g_hBmpWork);
    g_hBmpWork = hNewBmp;
    g_pBitsWork = pNewBits;
    g_bmpW = nw;
    g_bmpH = nh;
}

void CropImageToRect(RECT rc) {
    if (!g_pBitsWork) return;
    int x1 = rc.left < rc.right ? rc.left : rc.right;
    int y1 = rc.top < rc.bottom ? rc.top : rc.bottom;
    int x2 = rc.left > rc.right ? rc.left : rc.right;
    int y2 = rc.top > rc.bottom ? rc.top : rc.bottom;

    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= g_bmpW) x2 = g_bmpW - 1;
    if (y2 >= g_bmpH) y2 = g_bmpH - 1;

    int nw = x2 - x1 + 1;
    int nh = y2 - y1 + 1;
    if (nw <= 0 || nh <= 0) return;

    RGBQUAD* pNewBits = NULL;
    HBITMAP hNewBmp = Create32BitDIB(nw, nh, &pNewBits);

    for (int y = 0; y < nh; y++) {
        for (int x = 0; x < nw; x++) {
            pNewBits[y * nw + x] = g_pBitsWork[(y1 + y) * g_bmpW + (x1 + x)];
        }
    }

    DeleteObject(g_hBmpWork);
    g_hBmpWork = hNewBmp;
    g_pBitsWork = pNewBits;
    g_bmpW = nw;
    g_bmpH = nh;
}

// RGB Histogram Drawing in Sidebar
void DrawRGBHistogram(HDC hdc, RECT rc) {
    HBRUSH hBg = CreateSolidBrush(RGB(20, 25, 35));
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);

    FrameRect(hdc, &rc, (HBRUSH)GetStockObject(GRAY_BRUSH));

    if (!g_pBitsWork || g_bmpW == 0 || g_bmpH == 0) return;

    int rCount[256] = {0};
    int gCount[256] = {0};
    int bCount[256] = {0};
    int total = g_bmpW * g_bmpH;
    int step = total > 100000 ? total / 100000 : 1;

    for (int i = 0; i < total; i += step) {
        rCount[g_pBitsWork[i].rgbRed]++;
        gCount[g_pBitsWork[i].rgbGreen]++;
        bCount[g_pBitsWork[i].rgbBlue]++;
    }

    int maxVal = 1;
    for (int i = 0; i < 256; i++) {
        if (rCount[i] > maxVal) maxVal = rCount[i];
        if (gCount[i] > maxVal) maxVal = gCount[i];
        if (bCount[i] > maxVal) maxVal = bCount[i];
    }

    int w = rc.right - rc.left - 4;
    int h = rc.bottom - rc.top - 4;
    int ox = rc.left + 2;
    int oy = rc.bottom - 2;

    HPEN hPenR = CreatePen(PS_SOLID, 1, RGB(239, 68, 68));
    HPEN hPenG = CreatePen(PS_SOLID, 1, RGB(34, 197, 94));
    HPEN hPenB = CreatePen(PS_SOLID, 1, RGB(59, 130, 246));

    // Draw Red
    HGDIOBJ oldPen = SelectObject(hdc, hPenR);
    for (int i = 0; i < 256; i++) {
        int x = ox + (i * w) / 255;
        int y = oy - (rCount[i] * h) / maxVal;
        if (i == 0) MoveToEx(hdc, x, y, NULL);
        else LineTo(hdc, x, y);
    }

    // Draw Green
    SelectObject(hdc, hPenG);
    for (int i = 0; i < 256; i++) {
        int x = ox + (i * w) / 255;
        int y = oy - (gCount[i] * h) / maxVal;
        if (i == 0) MoveToEx(hdc, x, y, NULL);
        else LineTo(hdc, x, y);
    }

    // Draw Blue
    SelectObject(hdc, hPenB);
    for (int i = 0; i < 256; i++) {
        int x = ox + (i * w) / 255;
        int y = oy - (bCount[i] * h) / maxVal;
        if (i == 0) MoveToEx(hdc, x, y, NULL);
        else LineTo(hdc, x, y);
    }

    SelectObject(hdc, oldPen);
    DeleteObject(hPenR);
    DeleteObject(hPenG);
    DeleteObject(hPenB);
}

// Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(13, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            int x = 6, y = 6, btnH = 28;

            HWND hBtn = CreateWindowEx(0, "BUTTON", "Open", WS_CHILD | WS_VISIBLE, x, y, 50, btnH, hwnd, (HMENU)ID_BTN_OPEN, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 54;

            hBtn = CreateWindowEx(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE, x, y, 50, btnH, hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 54;

            hBtn = CreateWindowEx(0, "BUTTON", "-", WS_CHILD | WS_VISIBLE, x, y, 26, btnH, hwnd, (HMENU)ID_BTN_ZOOM_OUT, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 28;

            hBtn = CreateWindowEx(0, "BUTTON", "+", WS_CHILD | WS_VISIBLE, x, y, 26, btnH, hwnd, (HMENU)ID_BTN_ZOOM_IN, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 28;

            hBtn = CreateWindowEx(0, "BUTTON", "1:1", WS_CHILD | WS_VISIBLE, x, y, 32, btnH, hwnd, (HMENU)ID_BTN_ZOOM_RESET, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 36;

            hBtn = CreateWindowEx(0, "BUTTON", "↺", WS_CHILD | WS_VISIBLE, x, y, 28, btnH, hwnd, (HMENU)ID_BTN_ROT_CCW, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 30;

            hBtn = CreateWindowEx(0, "BUTTON", "↻", WS_CHILD | WS_VISIBLE, x, y, 28, btnH, hwnd, (HMENU)ID_BTN_ROT_CW, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 30;

            hBtn = CreateWindowEx(0, "BUTTON", "FlipH", WS_CHILD | WS_VISIBLE, x, y, 44, btnH, hwnd, (HMENU)ID_BTN_FLIP_H, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 46;

            hBtn = CreateWindowEx(0, "BUTTON", "FlipV", WS_CHILD | WS_VISIBLE, x, y, 44, btnH, hwnd, (HMENU)ID_BTN_FLIP_V, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 46;

            hBtn = CreateWindowEx(0, "BUTTON", "Gray", WS_CHILD | WS_VISIBLE, x, y, 40, btnH, hwnd, (HMENU)ID_BTN_GRAYSCALE, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 42;

            hBtn = CreateWindowEx(0, "BUTTON", "Sepia", WS_CHILD | WS_VISIBLE, x, y, 44, btnH, hwnd, (HMENU)ID_BTN_SEPIA, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 46;

            hBtn = CreateWindowEx(0, "BUTTON", "Invert", WS_CHILD | WS_VISIBLE, x, y, 46, btnH, hwnd, (HMENU)ID_BTN_INVERT, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 48;

            hBtn = CreateWindowEx(0, "BUTTON", "Blur", WS_CHILD | WS_VISIBLE, x, y, 38, btnH, hwnd, (HMENU)ID_BTN_BLUR, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 40;

            hBtn = CreateWindowEx(0, "BUTTON", "Br+", WS_CHILD | WS_VISIBLE, x, y, 32, btnH, hwnd, (HMENU)ID_BTN_BRIGHT_UP, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 34;

            hBtn = CreateWindowEx(0, "BUTTON", "Br-", WS_CHILD | WS_VISIBLE, x, y, 32, btnH, hwnd, (HMENU)ID_BTN_BRIGHT_DOWN, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 34;

            hBtn = CreateWindowEx(0, "BUTTON", "Reset", WS_CHILD | WS_VISIBLE, x, y, 44, btnH, hwnd, (HMENU)ID_BTN_RESET, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 46;

            g_hBtnCrop = CreateWindowEx(0, "BUTTON", "Crop", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE, x, y, 42, btnH, hwnd, (HMENU)ID_BTN_CROP, NULL, NULL);
            SendMessage(g_hBtnCrop, WM_SETFONT, (WPARAM)hFont, TRUE); x += 44;

            g_hBtnDraw = CreateWindowEx(0, "BUTTON", "Draw", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE, x, y, 42, btnH, hwnd, (HMENU)ID_BTN_DRAW, NULL, NULL);
            SendMessage(g_hBtnDraw, WM_SETFONT, (WPARAM)hFont, TRUE); x += 44;

            hBtn = CreateWindowEx(0, "BUTTON", "◀", WS_CHILD | WS_VISIBLE, x, y, 26, btnH, hwnd, (HMENU)ID_BTN_PREV, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE); x += 28;

            g_hBtnPlay = CreateWindowEx(0, "BUTTON", "▶ Play", WS_CHILD | WS_VISIBLE, x, y, 54, btnH, hwnd, (HMENU)ID_BTN_PLAY, NULL, NULL);
            SendMessage(g_hBtnPlay, WM_SETFONT, (WPARAM)hFont, TRUE); x += 56;

            hBtn = CreateWindowEx(0, "BUTTON", "▶", WS_CHILD | WS_VISIBLE, x, y, 26, btnH, hwnd, (HMENU)ID_BTN_NEXT, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

            g_hPenDraw = CreatePen(PS_SOLID, 3, RGB(59, 130, 246));
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hFont);
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            switch (id) {
                case ID_BTN_OPEN:
                    OpenFileDlg(hwnd);
                    break;
                case ID_BTN_SAVE:
                    SaveFileDlg(hwnd);
                    break;
                case ID_BTN_ZOOM_IN:
                    g_zoom *= 1.2f;
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_ZOOM_OUT:
                    g_zoom /= 1.2f;
                    if (g_zoom < 0.1f) g_zoom = 0.1f;
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_ZOOM_RESET:
                    g_zoom = 1.0f;
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_ROT_CCW:
                    RotateImage90(0);
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_ROT_CW:
                    RotateImage90(1);
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_FLIP_H:
                    FlipImage(1);
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_FLIP_V:
                    FlipImage(0);
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_GRAYSCALE:
                    FilterGrayscale();
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_SEPIA:
                    FilterSepia();
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_INVERT:
                    FilterInvert();
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_BLUR:
                    FilterBlur();
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_BRIGHT_UP:
                    FilterBrightness(15);
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_BRIGHT_DOWN:
                    FilterBrightness(-15);
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case ID_BTN_RESET:
                    if (g_hBmpOrig) {
                        CopyBitmapToWork(g_hBmpOrig, g_origW, g_origH);
                        g_zoom = 1.0f;
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                    break;
                case ID_BTN_CROP:
                    g_cropMode = SendMessage(g_hBtnCrop, BM_GETCHECK, 0, 0) == BST_CHECKED;
                    if (g_cropMode) {
                        g_drawMode = 0;
                        SendMessage(g_hBtnDraw, BM_SETCHECK, BST_UNCHECKED, 0);
                    }
                    break;
                case ID_BTN_DRAW:
                    g_drawMode = SendMessage(g_hBtnDraw, BM_GETCHECK, 0, 0) == BST_CHECKED;
                    if (g_drawMode) {
                        g_cropMode = 0;
                        SendMessage(g_hBtnCrop, BM_SETCHECK, BST_UNCHECKED, 0);
                    }
                    break;
                case ID_BTN_PREV:
                    if (g_fileCount > 0) {
                        g_fileIndex = (g_fileIndex - 1 + g_fileCount) % g_fileCount;
                        LoadBitmapFile(hwnd, g_fileList[g_fileIndex]);
                    }
                    break;
                case ID_BTN_NEXT:
                    if (g_fileCount > 0) {
                        g_fileIndex = (g_fileIndex + 1) % g_fileCount;
                        LoadBitmapFile(hwnd, g_fileList[g_fileIndex]);
                    }
                    break;
                case ID_BTN_PLAY:
                    g_slideshowPlaying = !g_slideshowPlaying;
                    SetWindowTextA(g_hBtnPlay, g_slideshowPlaying ? "⏸ Pause" : "▶ Play");
                    if (g_slideshowPlaying) {
                        SetTimer(hwnd, TIMER_SLIDESHOW, 2500, NULL);
                    } else {
                        KillTimer(hwnd, TIMER_SLIDESHOW);
                    }
                    break;
            }
            break;
        }
        case WM_TIMER: {
            if (wParam == TIMER_SLIDESHOW && g_fileCount > 0) {
                g_fileIndex = (g_fileIndex + 1) % g_fileCount;
                LoadBitmapFile(hwnd, g_fileList[g_fileIndex]);
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (!g_hBmpWork) break;
            int mx = (short)LOWORD(lParam);
            int my = (short)HIWORD(lParam);
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            int canvasW = rcClient.right - rcClient.left - SIDEBAR_WIDTH;
            int canvasH = rcClient.bottom - rcClient.top - TOOLBAR_HEIGHT;
            int drawW = (int)(g_bmpW * g_zoom);
            int drawH = (int)(g_bmpH * g_zoom);
            int ox = (canvasW - drawW) / 2;
            int oy = TOOLBAR_HEIGHT + (canvasH - drawH) / 2;

            int ix = (mx - ox) * g_bmpW / (drawW > 0 ? drawW : 1);
            int iy = (my - oy) * g_bmpH / (drawH > 0 ? drawH : 1);

            if (g_drawMode) {
                g_isDrawing = 1;
                g_lastPt.x = ix;
                g_lastPt.y = iy;
                SetCapture(hwnd);
            } else if (g_cropMode) {
                g_isCropping = 1;
                g_cropRect.left = ix;
                g_cropRect.top = iy;
                g_cropRect.right = ix;
                g_cropRect.bottom = iy;
                SetCapture(hwnd);
            }
            break;
        }
        case WM_MOUSEMOVE: {
            if (!g_hBmpWork) break;
            int mx = (short)LOWORD(lParam);
            int my = (short)HIWORD(lParam);
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            int canvasW = rcClient.right - rcClient.left - SIDEBAR_WIDTH;
            int canvasH = rcClient.bottom - rcClient.top - TOOLBAR_HEIGHT;
            int drawW = (int)(g_bmpW * g_zoom);
            int drawH = (int)(g_bmpH * g_zoom);
            int ox = (canvasW - drawW) / 2;
            int oy = TOOLBAR_HEIGHT + (canvasH - drawH) / 2;

            int ix = (mx - ox) * g_bmpW / (drawW > 0 ? drawW : 1);
            int iy = (my - oy) * g_bmpH / (drawH > 0 ? drawH : 1);

            if (g_isDrawing && g_pBitsWork) {
                HDC hdcMem = CreateCompatibleDC(NULL);
                HGDIOBJ oldBmp = SelectObject(hdcMem, g_hBmpWork);
                HGDIOBJ oldPen = SelectObject(hdcMem, g_hPenDraw);
                MoveToEx(hdcMem, g_lastPt.x, g_lastPt.y, NULL);
                LineTo(hdcMem, ix, iy);
                SelectObject(hdcMem, oldPen);
                SelectObject(hdcMem, oldBmp);
                DeleteDC(hdcMem);
                g_lastPt.x = ix;
                g_lastPt.y = iy;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (g_isCropping) {
                g_cropRect.right = ix;
                g_cropRect.bottom = iy;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_LBUTTONUP: {
            if (g_isDrawing) {
                g_isDrawing = 0;
                ReleaseCapture();
            } else if (g_isCropping) {
                g_isCropping = 0;
                ReleaseCapture();
                CropImageToRect(g_cropRect);
                g_cropMode = 0;
                SendMessage(g_hBtnCrop, BM_SETCHECK, BST_UNCHECKED, 0);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            int canvasW = rcClient.right - rcClient.left - SIDEBAR_WIDTH;
            int canvasH = rcClient.bottom - rcClient.top - TOOLBAR_HEIGHT;

            // Draw Image Area
            if (g_hBmpWork) {
                HDC hdcMem = CreateCompatibleDC(hdc);
                HGDIOBJ oldBmp = SelectObject(hdcMem, g_hBmpWork);

                int drawW = (int)(g_bmpW * g_zoom);
                int drawH = (int)(g_bmpH * g_zoom);
                int ox = (canvasW - drawW) / 2;
                int oy = TOOLBAR_HEIGHT + (canvasH - drawH) / 2;

                SetStretchBltMode(hdc, HALFTONE);
                StretchBlt(hdc, ox, oy, drawW, drawH, hdcMem, 0, 0, g_bmpW, g_bmpH, SRCCOPY);

                // Draw Crop Overlay Selection Rectangle
                if (g_isCropping) {
                    HPEN hPenCrop = CreatePen(PS_DASH, 1, RGB(59, 130, 246));
                    HGDIOBJ oldPen = SelectObject(hdc, hPenCrop);
                    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

                    int rx1 = ox + g_cropRect.left * drawW / g_bmpW;
                    int ry1 = oy + g_cropRect.top * drawH / g_bmpH;
                    int rx2 = ox + g_cropRect.right * drawW / g_bmpW;
                    int ry2 = oy + g_cropRect.bottom * drawH / g_bmpH;

                    Rectangle(hdc, rx1, ry1, rx2, ry2);

                    SelectObject(hdc, oldBrush);
                    SelectObject(hdc, oldPen);
                    DeleteObject(hPenCrop);
                }

                SelectObject(hdcMem, oldBmp);
                DeleteDC(hdcMem);
            }

            // Draw Sidebar (Inspector Panel)
            RECT rcSide;
            rcSide.left = rcClient.right - SIDEBAR_WIDTH;
            rcSide.top = TOOLBAR_HEIGHT;
            rcSide.right = rcClient.right;
            rcSide.bottom = rcClient.bottom;

            HBRUSH hSideBg = CreateSolidBrush(RGB(15, 23, 42));
            FillRect(hdc, &rcSide, hSideBg);
            DeleteObject(hSideBg);

            // Draw Sidebar Header & Metadata Text
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(248, 250, 252));
            
            HFONT hFont = (HFONT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            HGDIOBJ oldFont = SelectObject(hdc, hFont);

            TextOutA(hdc, rcSide.left + 10, rcSide.top + 10, "RGB Histogram", 13);

            // Draw RGB Histogram Box (Height 100px)
            RECT rcHist;
            rcHist.left = rcSide.left + 10;
            rcHist.top = rcSide.top + 30;
            rcHist.right = rcSide.right - 10;
            rcHist.bottom = rcSide.top + 130;
            DrawRGBHistogram(hdc, rcHist);

            // Image Metadata Text
            int ty = rcHist.bottom + 20;
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutA(hdc, rcSide.left + 10, ty, "Image Metadata:", 15); ty += 20;

            char buf[128];
            wsprintfA(buf, "Size: %d x %d px", g_bmpW, g_bmpH);
            SetTextColor(hdc, RGB(248, 250, 252));
            TextOutA(hdc, rcSide.left + 10, ty, buf, lstrlenA(buf)); ty += 18;

            wsprintfA(buf, "Zoom: %d%%", (int)(g_zoom * 100));
            TextOutA(hdc, rcSide.left + 10, ty, buf, lstrlenA(buf)); ty += 18;

            wsprintfA(buf, "Depth: 32-bit BGRA");
            TextOutA(hdc, rcSide.left + 10, ty, buf, lstrlenA(buf)); ty += 22;

            if (g_fileCount > 0) {
                SetTextColor(hdc, RGB(148, 163, 184));
                TextOutA(hdc, rcSide.left + 10, ty, "Slideshow Playlist:", 19); ty += 20;
                wsprintfA(buf, "Image %d of %d", g_fileIndex + 1, g_fileCount);
                SetTextColor(hdc, RGB(96, 165, 250));
                TextOutA(hdc, rcSide.left + 10, ty, buf, lstrlenA(buf));
            }

            SelectObject(hdc, oldFont);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBRUSH brush = CreateSolidBrush(RGB(30, 41, 59));
            FillRect(hdc, &rc, brush);
            DeleteObject(brush);
            return 1;
        }
        case WM_DESTROY: {
            if (g_hBmpWork) DeleteObject(g_hBmpWork);
            if (g_hBmpOrig) DeleteObject(g_hBmpOrig);
            if (g_hPenDraw) DeleteObject(g_hPenDraw);
            HFONT hFont = (HFONT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (hFont) DeleteObject(hFont);
            PostQuitMessage(0);
            break;
        }
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
    wc.lpszClassName = "KImageApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KImageApp", "KImage Pro (Native C)", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
