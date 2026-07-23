#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

#pragma function(memset)

// Canvas & Graphics state
int isPainting = 0;
HDC hdcMem = NULL;
HBITMAP hbmCanvas = NULL;
int lastX = 0, lastY = 0;
HPEN hPen = NULL;
HBRUSH hBrush = NULL;

COLORREF curColor = RGB(0,0,0);
int curSize = 4;
int brushShape = 0; // 0 = Round, 1 = Square
int currentTool = 0; // 0=Freehand, 1=Line, 2=Rect, 3=Ellipse, 4=Spray, 5=Eraser
int startX = 0, startY = 0;

// History Stack (Undo / Redo)
#define MAX_HISTORY 10
HBITMAP hbmUndoStack[MAX_HISTORY];
int undoCount = 0;
HBITMAP hbmRedoStack[MAX_HISTORY];
int redoCount = 0;

// Controls
HWND hBtnBlack, hBtnRed, hBtnGreen, hBtnBlue, hBtnYellow, hBtnPurple, hBtnEraser, hBtnCustomColor;
HWND hBtnSizeSmall, hBtnSizeMed, hBtnSizeLarge, hBtnShapeToggle;
HWND hBtnFreehand, hBtnLine, hBtnRect, hBtnEllipse, hBtnSpray;
HWND hBtnUndo, hBtnRedo, hBtnInvert, hBtnGray, hBtnBright, hBtnFlipH, hBtnRotate90;
HWND hBtnClear, hBtnSave, hBtnOpen;
HFONT hFont = NULL;

void PushUndo();
void PerformUndo();
void PerformRedo();
void UpdatePen();

void UpdatePen() {
    if (hPen) DeleteObject(hPen);
    if (hBrush) DeleteObject(hBrush);

    COLORREF penColor = (currentTool == 5) ? RGB(255,255,255) : curColor;
    
    if (brushShape == 1) { // Square pen
        LOGBRUSH lb;
        lb.lbStyle = BS_SOLID;
        lb.lbColor = penColor;
        lb.lbHatch = 0;
        hPen = ExtCreatePen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_SQUARE | PS_JOIN_MITER, curSize, &lb, 0, NULL);
    } else {
        hPen = CreatePen(PS_SOLID, curSize, penColor);
    }
    
    hBrush = CreateSolidBrush(penColor);
    
    if (hdcMem) {
        SelectObject(hdcMem, hPen);
        SelectObject(hdcMem, hBrush);
    }
}

// Push state to undo stack
void PushUndo() {
    if (!hdcMem || !hbmCanvas) return;
    
    HDC hdcScreen = GetDC(NULL);
    HBITMAP hbmCopy = CreateCompatibleBitmap(hdcScreen, 2000, 2000);
    HDC hdcCopy = CreateCompatibleDC(hdcScreen);
    SelectObject(hdcCopy, hbmCopy);
    BitBlt(hdcCopy, 0, 0, 2000, 2000, hdcMem, 0, 0, SRCCOPY);
    
    DeleteDC(hdcCopy);
    ReleaseDC(NULL, hdcScreen);

    if (undoCount >= MAX_HISTORY) {
        if (hbmUndoStack[0]) DeleteObject(hbmUndoStack[0]);
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            hbmUndoStack[i] = hbmUndoStack[i + 1];
        }
        undoCount--;
    }
    
    hbmUndoStack[undoCount++] = hbmCopy;

    // Clear Redo stack on new draw action
    for (int i = 0; i < redoCount; i++) {
        if (hbmRedoStack[i]) DeleteObject(hbmRedoStack[i]);
    }
    redoCount = 0;
}

void PerformUndo() {
    if (undoCount <= 0) return;

    // Save current to redo stack
    HDC hdcScreen = GetDC(NULL);
    HBITMAP hbmCurrent = CreateCompatibleBitmap(hdcScreen, 2000, 2000);
    HDC hdcCopy = CreateCompatibleDC(hdcScreen);
    SelectObject(hdcCopy, hbmCurrent);
    BitBlt(hdcCopy, 0, 0, 2000, 2000, hdcMem, 0, 0, SRCCOPY);
    DeleteDC(hdcCopy);
    ReleaseDC(NULL, hdcScreen);

    if (redoCount < MAX_HISTORY) {
        hbmRedoStack[redoCount++] = hbmCurrent;
    } else {
        DeleteObject(hbmCurrent);
    }

    // Restore from undo stack
    HBITMAP hbmRestore = hbmUndoStack[--undoCount];
    HDC hdcTemp = CreateCompatibleDC(hdcMem);
    SelectObject(hdcTemp, hbmRestore);
    BitBlt(hdcMem, 0, 0, 2000, 2000, hdcTemp, 0, 0, SRCCOPY);
    DeleteDC(hdcTemp);
    DeleteObject(hbmRestore);
}

void PerformRedo() {
    if (redoCount <= 0) return;

    // Save current to undo stack
    HDC hdcScreen = GetDC(NULL);
    HBITMAP hbmCurrent = CreateCompatibleBitmap(hdcScreen, 2000, 2000);
    HDC hdcCopy = CreateCompatibleDC(hdcScreen);
    SelectObject(hdcCopy, hbmCurrent);
    BitBlt(hdcCopy, 0, 0, 2000, 2000, hdcMem, 0, 0, SRCCOPY);
    DeleteDC(hdcCopy);
    ReleaseDC(NULL, hdcScreen);

    if (undoCount < MAX_HISTORY) {
        hbmUndoStack[undoCount++] = hbmCurrent;
    } else {
        DeleteObject(hbmCurrent);
    }

    // Restore from redo stack
    HBITMAP hbmRestore = hbmRedoStack[--redoCount];
    HDC hdcTemp = CreateCompatibleDC(hdcMem);
    SelectObject(hdcTemp, hbmRestore);
    BitBlt(hdcMem, 0, 0, 2000, 2000, hdcTemp, 0, 0, SRCCOPY);
    DeleteDC(hdcTemp);
    DeleteObject(hbmRestore);
}

// Image Filters
void FilterInvert() {
    PushUndo();
    BitBlt(hdcMem, 0, 0, 2000, 2000, hdcMem, 0, 0, NOTSRCCOPY);
}

void FilterGrayscale() {
    PushUndo();
    HDC hdc = GetDC(NULL);
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = 2000;
    bi.bmiHeader.biHeight = -2000;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    
    DWORD bufSize = 2000 * 2000 * 3;
    BYTE* pBits = (BYTE*)GlobalAlloc(GPTR, bufSize);
    if (pBits) {
        GetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
        for (DWORD i = 0; i < bufSize; i += 3) {
            BYTE b = pBits[i];
            BYTE g = pBits[i+1];
            BYTE r = pBits[i+2];
            BYTE gray = (BYTE)((r * 299 + g * 587 + b * 114) / 1000);
            pBits[i] = pBits[i+1] = pBits[i+2] = gray;
        }
        SetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
        GlobalFree(pBits);
    }
    ReleaseDC(NULL, hdc);
}

void FilterBrightness(int delta) {
    PushUndo();
    HDC hdc = GetDC(NULL);
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = 2000;
    bi.bmiHeader.biHeight = -2000;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    
    DWORD bufSize = 2000 * 2000 * 3;
    BYTE* pBits = (BYTE*)GlobalAlloc(GPTR, bufSize);
    if (pBits) {
        GetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
        for (DWORD i = 0; i < bufSize; i++) {
            int val = pBits[i] + delta;
            pBits[i] = (BYTE)(val < 0 ? 0 : (val > 255 ? 255 : val));
        }
        SetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
        GlobalFree(pBits);
    }
    ReleaseDC(NULL, hdc);
}

// Transforms
void FlipHorizontal() {
    PushUndo();
    HDC hdcTemp = CreateCompatibleDC(hdcMem);
    HBITMAP hbmTemp = CreateCompatibleBitmap(hdcMem, 2000, 2000);
    SelectObject(hdcTemp, hbmTemp);
    StretchBlt(hdcTemp, 0, 0, 2000, 2000, hdcMem, 1999, 0, -2000, 2000, SRCCOPY);
    BitBlt(hdcMem, 0, 0, 2000, 2000, hdcTemp, 0, 0, SRCCOPY);
    DeleteDC(hdcTemp);
    DeleteObject(hbmTemp);
}

void Rotate90CW() {
    PushUndo();
    HDC hdc = GetDC(NULL);
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = 2000;
    bi.bmiHeader.biHeight = -2000;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    
    BYTE* pSrc = (BYTE*)GlobalAlloc(GPTR, 2000 * 2000 * 3);
    BYTE* pDst = (BYTE*)GlobalAlloc(GPTR, 2000 * 2000 * 3);
    if (pSrc && pDst) {
        GetDIBits(hdcMem, hbmCanvas, 0, 2000, pSrc, &bi, DIB_RGB_COLORS);
        for (int y = 0; y < 2000; y++) {
            for (int x = 0; x < 2000; x++) {
                int srcIdx = (y * 2000 + x) * 3;
                int dstIdx = (x * 2000 + (1999 - y)) * 3;
                pDst[dstIdx]   = pSrc[srcIdx];
                pDst[dstIdx+1] = pSrc[srcIdx+1];
                pDst[dstIdx+2] = pSrc[srcIdx+2];
            }
        }
        SetDIBits(hdcMem, hbmCanvas, 0, 2000, pDst, &bi, DIB_RGB_COLORS);
    }
    if (pSrc) GlobalFree(pSrc);
    if (pDst) GlobalFree(pDst);
    ReleaseDC(NULL, hdc);
}

int SaveBitmap(const char* path, HBITMAP hbm) {
    HDC hdc = GetDC(NULL);
    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = 800;
    bi.biHeight = 600;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    DWORD dwBmpSize = ((800 * 24 + 31) / 32) * 4 * 600;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char* lpbitmap = (char*)GlobalLock(hDIB);
    
    HDC tempDC = CreateCompatibleDC(hdc);
    HBITMAP tempBmp = CreateCompatibleBitmap(hdc, 800, 600);
    SelectObject(tempDC, tempBmp);
    RECT tr = {0,0,800,600}; FillRect(tempDC, &tr, (HBRUSH)GetStockObject(WHITE_BRUSH));
    BitBlt(tempDC, 0, 0, 800, 600, hdcMem, 0, 0, SRCCOPY);
    
    GetDIBits(hdc, tempBmp, 0, 600, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD dwBytesWritten = 0;
        BITMAPFILEHEADER bmfHeader = {0};
        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
        bmfHeader.bfSize = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bmfHeader.bfType = 0x4D42;

        WriteFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
        WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
        WriteFile(hFile, lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
        CloseHandle(hFile);
    }
    
    DeleteDC(tempDC);
    DeleteObject(tempBmp);
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    ReleaseDC(NULL, hdc);
    return 1;
}

void LoadBitmapFile(HWND hwnd, const char* path) {
    HBITMAP hLoaded = (HBITMAP)LoadImageA(NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (hLoaded) {
        PushUndo();
        HDC hdcTemp = CreateCompatibleDC(hdcMem);
        SelectObject(hdcTemp, hLoaded);
        BITMAP bmp;
        GetObject(hLoaded, sizeof(BITMAP), &bmp);
        RECT r = {0, 0, 2000, 2000};
        FillRect(hdcMem, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
        BitBlt(hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcTemp, 0, 0, SRCCOPY);
        DeleteDC(hdcTemp);
        DeleteObject(hLoaded);
        InvalidateRect(hwnd, NULL, FALSE);
    } else {
        MessageBoxA(hwnd, "Could not open BMP image file.", "KPaint Error", MB_OK | MB_ICONERROR);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HDC hdc = GetDC(hwnd);
            hdcMem = CreateCompatibleDC(hdc);
            hbmCanvas = CreateCompatibleBitmap(hdc, 2000, 2000);
            SelectObject(hdcMem, hbmCanvas);
            
            RECT r = {0, 0, 2000, 2000};
            FillRect(hdcMem, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
            ReleaseDC(hwnd, hdc);
            
            UpdatePen();
            
            hFont = CreateFontA(13, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            // Colors
            hBtnBlack = CreateWindowA("BUTTON", "Black", WS_CHILD | WS_VISIBLE, 5, 5, 50, 22, hwnd, (HMENU)101, NULL, NULL);
            hBtnRed = CreateWindowA("BUTTON", "Red", WS_CHILD | WS_VISIBLE, 60, 5, 50, 22, hwnd, (HMENU)102, NULL, NULL);
            hBtnGreen = CreateWindowA("BUTTON", "Green", WS_CHILD | WS_VISIBLE, 5, 30, 50, 22, hwnd, (HMENU)103, NULL, NULL);
            hBtnBlue = CreateWindowA("BUTTON", "Blue", WS_CHILD | WS_VISIBLE, 60, 30, 50, 22, hwnd, (HMENU)104, NULL, NULL);
            hBtnYellow = CreateWindowA("BUTTON", "Yellow", WS_CHILD | WS_VISIBLE, 5, 55, 50, 22, hwnd, (HMENU)105, NULL, NULL);
            hBtnPurple = CreateWindowA("BUTTON", "Purple", WS_CHILD | WS_VISIBLE, 60, 55, 50, 22, hwnd, (HMENU)106, NULL, NULL);
            hBtnCustomColor = CreateWindowA("BUTTON", "Custom...", WS_CHILD | WS_VISIBLE, 5, 80, 105, 22, hwnd, (HMENU)107, NULL, NULL);

            // Tools & Eraser
            hBtnFreehand = CreateWindowA("BUTTON", "Brush", WS_CHILD | WS_VISIBLE, 5, 110, 50, 22, hwnd, (HMENU)401, NULL, NULL);
            hBtnLine = CreateWindowA("BUTTON", "Line", WS_CHILD | WS_VISIBLE, 60, 110, 50, 22, hwnd, (HMENU)402, NULL, NULL);
            hBtnRect = CreateWindowA("BUTTON", "Rect", WS_CHILD | WS_VISIBLE, 5, 135, 50, 22, hwnd, (HMENU)403, NULL, NULL);
            hBtnEllipse = CreateWindowA("BUTTON", "Ellipse", WS_CHILD | WS_VISIBLE, 60, 135, 50, 22, hwnd, (HMENU)404, NULL, NULL);
            hBtnSpray = CreateWindowA("BUTTON", "Spray", WS_CHILD | WS_VISIBLE, 5, 160, 50, 22, hwnd, (HMENU)405, NULL, NULL);
            hBtnEraser = CreateWindowA("BUTTON", "Eraser", WS_CHILD | WS_VISIBLE, 60, 160, 50, 22, hwnd, (HMENU)406, NULL, NULL);

            // Size & Shape
            hBtnSizeSmall = CreateWindowA("BUTTON", "2px", WS_CHILD | WS_VISIBLE, 5, 190, 33, 22, hwnd, (HMENU)201, NULL, NULL);
            hBtnSizeMed = CreateWindowA("BUTTON", "6px", WS_CHILD | WS_VISIBLE, 41, 190, 33, 22, hwnd, (HMENU)202, NULL, NULL);
            hBtnSizeLarge = CreateWindowA("BUTTON", "14px", WS_CHILD | WS_VISIBLE, 77, 190, 33, 22, hwnd, (HMENU)203, NULL, NULL);
            hBtnShapeToggle = CreateWindowA("BUTTON", "Shape: Round", WS_CHILD | WS_VISIBLE, 5, 215, 105, 22, hwnd, (HMENU)204, NULL, NULL);

            // Filters & Transforms
            hBtnInvert = CreateWindowA("BUTTON", "Invert", WS_CHILD | WS_VISIBLE, 5, 245, 50, 22, hwnd, (HMENU)501, NULL, NULL);
            hBtnGray = CreateWindowA("BUTTON", "Gray", WS_CHILD | WS_VISIBLE, 60, 245, 50, 22, hwnd, (HMENU)502, NULL, NULL);
            hBtnBright = CreateWindowA("BUTTON", "+Bright", WS_CHILD | WS_VISIBLE, 5, 270, 50, 22, hwnd, (HMENU)503, NULL, NULL);
            hBtnFlipH = CreateWindowA("BUTTON", "Flip H", WS_CHILD | WS_VISIBLE, 60, 270, 50, 22, hwnd, (HMENU)504, NULL, NULL);
            hBtnRotate90 = CreateWindowA("BUTTON", "Rotate 90", WS_CHILD | WS_VISIBLE, 5, 295, 105, 22, hwnd, (HMENU)505, NULL, NULL);

            // History & Actions
            hBtnUndo = CreateWindowA("BUTTON", "Undo", WS_CHILD | WS_VISIBLE, 5, 325, 50, 22, hwnd, (HMENU)601, NULL, NULL);
            hBtnRedo = CreateWindowA("BUTTON", "Redo", WS_CHILD | WS_VISIBLE, 60, 325, 50, 22, hwnd, (HMENU)602, NULL, NULL);
            hBtnOpen = CreateWindowA("BUTTON", "Open BMP", WS_CHILD | WS_VISIBLE, 5, 355, 105, 22, hwnd, (HMENU)303, NULL, NULL);
            hBtnSave = CreateWindowA("BUTTON", "Save BMP", WS_CHILD | WS_VISIBLE, 5, 380, 105, 22, hwnd, (HMENU)302, NULL, NULL);
            hBtnClear = CreateWindowA("BUTTON", "Clear Canvas", WS_CHILD | WS_VISIBLE, 5, 405, 105, 22, hwnd, (HMENU)301, NULL, NULL);

            // Set Fonts
            HWND controls[] = {
                hBtnBlack, hBtnRed, hBtnGreen, hBtnBlue, hBtnYellow, hBtnPurple, hBtnCustomColor,
                hBtnFreehand, hBtnLine, hBtnRect, hBtnEllipse, hBtnSpray, hBtnEraser,
                hBtnSizeSmall, hBtnSizeMed, hBtnSizeLarge, hBtnShapeToggle,
                hBtnInvert, hBtnGray, hBtnBright, hBtnFlipH, hBtnRotate90,
                hBtnUndo, hBtnRedo, hBtnOpen, hBtnSave, hBtnClear
            };
            for (int i = 0; i < sizeof(controls)/sizeof(controls[0]); i++) {
                SendMessage(controls[i], WM_SETFONT, (WPARAM)hFont, TRUE);
            }
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 101) curColor = RGB(0,0,0);
            if (id == 102) curColor = RGB(231,76,60);
            if (id == 103) curColor = RGB(46,204,113);
            if (id == 104) curColor = RGB(52,152,219);
            if (id == 105) curColor = RGB(241,196,15);
            if (id == 106) curColor = RGB(155,89,182);
            if (id == 107) {
                CHOOSECOLORA cc = {0};
                static COLORREF custColors[16] = {0};
                cc.lStructSize = sizeof(cc);
                cc.hwndOwner = hwnd;
                cc.lpCustColors = custColors;
                cc.rgbResult = curColor;
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                if (ChooseColorA(&cc)) curColor = cc.rgbResult;
            }

            if (id == 201) curSize = 2;
            if (id == 202) curSize = 6;
            if (id == 203) curSize = 14;
            if (id == 204) {
                brushShape = 1 - brushShape;
                SetWindowTextA(hBtnShapeToggle, brushShape ? "Shape: Square" : "Shape: Round");
            }
            
            if (id >= 101 && id <= 204) UpdatePen();
            
            if (id == 301) {
                PushUndo();
                RECT r = {0, 0, 2000, 2000};
                FillRect(hdcMem, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
                InvalidateRect(hwnd, NULL, FALSE);
            }
            if (id == 302) {
                char file[260] = {0};
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = file;
                ofn.nMaxFile = 260;
                ofn.lpstrFilter = "BMP Files\0*.bmp\0All Files\0*.*\0";
                ofn.lpstrDefExt = "bmp";
                if (GetSaveFileNameA(&ofn)) {
                    if (SaveBitmap(file, hbmCanvas)) {
                        MessageBoxA(hwnd, "Saved successfully!", "KPaint", MB_OK | MB_ICONINFORMATION);
                    } else {
                        MessageBoxA(hwnd, "Failed to save.", "Error", MB_OK | MB_ICONERROR);
                    }
                }
            }
            if (id == 303) {
                char file[260] = {0};
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = file;
                ofn.nMaxFile = 260;
                ofn.lpstrFilter = "BMP Files\0*.bmp\0All Files\0*.*\0";
                if (GetOpenFileNameA(&ofn)) {
                    LoadBitmapFile(hwnd, file);
                }
            }
            
            if (id == 401) { currentTool = 0; UpdatePen(); }
            if (id == 402) { currentTool = 1; UpdatePen(); }
            if (id == 403) { currentTool = 2; UpdatePen(); }
            if (id == 404) { currentTool = 3; UpdatePen(); }
            if (id == 405) { currentTool = 4; UpdatePen(); }
            if (id == 406) { currentTool = 5; UpdatePen(); }

            if (id == 501) { FilterInvert(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 502) { FilterGrayscale(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 503) { FilterBrightness(25); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 504) { FlipHorizontal(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 505) { Rotate90CW(); InvalidateRect(hwnd, NULL, FALSE); }

            if (id == 601) { PerformUndo(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 602) { PerformRedo(); InvalidateRect(hwnd, NULL, FALSE); }
            break;
        }
        case WM_KEYDOWN: {
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                if (wParam == 'Z' || wParam == 'z') {
                    PerformUndo();
                    InvalidateRect(hwnd, NULL, FALSE);
                } else if (wParam == 'Y' || wParam == 'y') {
                    PerformRedo();
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = (short)LOWORD(lParam) - 115;
            int y = (short)HIWORD(lParam);
            if (x >= 0) {
                PushUndo();
                isPainting = 1;
                startX = x; startY = y;
                lastX = x; lastY = y;
                if (currentTool == 0 || currentTool == 5) {
                    HDC hdc = GetDC(hwnd);
                    SelectObject(hdc, hPen);
                    MoveToEx(hdc, x + 115, y, NULL);
                    LineTo(hdc, x + 115, y);
                    ReleaseDC(hwnd, hdc);
                    MoveToEx(hdcMem, x, y, NULL);
                    LineTo(hdcMem, x, y);
                } else if (currentTool == 4) { // Spray
                    for (int i = 0; i < 15; i++) {
                        int rx = (x + (i * 7) % curSize) - curSize / 2;
                        int ry = (y + (i * 13) % curSize) - curSize / 2;
                        SetPixel(hdcMem, rx, ry, curColor);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                } else {
                    HDC hdc = GetDC(hwnd);
                    SetROP2(hdc, R2_NOTXORPEN);
                    SelectObject(hdc, hPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    if (currentTool == 1) { MoveToEx(hdc, startX + 115, startY, NULL); LineTo(hdc, x + 115, y); }
                    else if (currentTool == 2) { Rectangle(hdc, startX + 115, startY, x + 115, y); }
                    else if (currentTool == 3) { Ellipse(hdc, startX + 115, startY, x + 115, y); }
                    ReleaseDC(hwnd, hdc);
                }
            }
            break;
        }
        case WM_MOUSEMOVE: {
            int x = (short)LOWORD(lParam) - 115;
            int y = (short)HIWORD(lParam);
            if (isPainting && x >= 0) {
                HDC hdc = GetDC(hwnd);
                if (currentTool == 0 || currentTool == 5) {
                    SelectObject(hdc, hPen);
                    MoveToEx(hdc, lastX + 115, lastY, NULL);
                    LineTo(hdc, x + 115, y);
                    MoveToEx(hdcMem, lastX, lastY, NULL);
                    LineTo(hdcMem, x, y);
                } else if (currentTool == 4) { // Spray
                    for (int i = 0; i < 15; i++) {
                        int rx = (x + (i * 7) % curSize) - curSize / 2;
                        int ry = (y + (i * 13) % curSize) - curSize / 2;
                        SetPixel(hdcMem, rx, ry, curColor);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                } else {
                    SetROP2(hdc, R2_NOTXORPEN);
                    SelectObject(hdc, hPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    if (currentTool == 1) { MoveToEx(hdc, startX + 115, startY, NULL); LineTo(hdc, lastX + 115, lastY); }
                    else if (currentTool == 2) { Rectangle(hdc, startX + 115, startY, lastX + 115, lastY); }
                    else if (currentTool == 3) { Ellipse(hdc, startX + 115, startY, lastX + 115, lastY); }
                    
                    if (currentTool == 1) { MoveToEx(hdc, startX + 115, startY, NULL); LineTo(hdc, x + 115, y); }
                    else if (currentTool == 2) { Rectangle(hdc, startX + 115, startY, x + 115, y); }
                    else if (currentTool == 3) { Ellipse(hdc, startX + 115, startY, x + 115, y); }
                }
                ReleaseDC(hwnd, hdc);
                lastX = x;
                lastY = y;
            }
            break;
        }
        case WM_LBUTTONUP: {
            if (isPainting) {
                int x = (short)LOWORD(lParam) - 115;
                int y = (short)HIWORD(lParam);
                if (currentTool != 0 && currentTool != 5 && currentTool != 4) {
                    HDC hdc = GetDC(hwnd);
                    SetROP2(hdc, R2_NOTXORPEN);
                    SelectObject(hdc, hPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    if (currentTool == 1) { MoveToEx(hdc, startX + 115, startY, NULL); LineTo(hdc, lastX + 115, lastY); }
                    else if (currentTool == 2) { Rectangle(hdc, startX + 115, startY, lastX + 115, lastY); }
                    else if (currentTool == 3) { Ellipse(hdc, startX + 115, startY, lastX + 115, lastY); }
                    ReleaseDC(hwnd, hdc);

                    SelectObject(hdcMem, hPen);
                    SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
                    if (currentTool == 1) { MoveToEx(hdcMem, startX, startY, NULL); LineTo(hdcMem, x, y); }
                    else if (currentTool == 2) { Rectangle(hdcMem, startX, startY, x, y); }
                    else if (currentTool == 3) { Ellipse(hdcMem, startX, startY, x, y); }
                    
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                isPainting = 0;
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT r;
            GetClientRect(hwnd, &r);
            RECT sidebar = {0, 0, 115, r.bottom};
            FillRect(hdc, &sidebar, (HBRUSH)(COLOR_BTNFACE + 1));
            BitBlt(hdc, 115, 0, r.right - 115, r.bottom, hdcMem, 0, 0, SRCCOPY);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            if (hPen) DeleteObject(hPen);
            if (hBrush) DeleteObject(hBrush);
            if (hdcMem) DeleteDC(hdcMem);
            if (hbmCanvas) DeleteObject(hbmCanvas);
            if (hFont) DeleteObject(hFont);
            for (int i = 0; i < undoCount; i++) if (hbmUndoStack[i]) DeleteObject(hbmUndoStack[i]);
            for (int i = 0; i < redoCount; i++) if (hbmRedoStack[i]) DeleteObject(hbmRedoStack[i]);
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
    wc.hbrBackground = NULL;

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KPaintClass", "KPaint Pro", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 850, 560, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
