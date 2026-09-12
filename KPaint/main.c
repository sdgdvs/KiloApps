#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>

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
int currentTool = 0; // 0=Freehand, 1=Line, 2=Rect, 3=Ellipse, 4=Spray, 5=Eraser, 6=Fill, 7=Pick
int startX = 0, startY = 0;
int scrollX = 0, scrollY = 0;

// History Stack (Undo / Redo)
#define MAX_HISTORY 10
HBITMAP hbmUndoStack[MAX_HISTORY];
int undoCount = 0;
HBITMAP hbmRedoStack[MAX_HISTORY];
int redoCount = 0;

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

// Controls
HWND hBtnBlack, hBtnRed, hBtnGreen, hBtnBlue, hBtnYellow, hBtnPurple, hBtnEraser, hBtnCustomColor;
HWND hBtnSizeSmall, hBtnSizeMed, hBtnSizeLarge, hBtnShapeToggle;
HWND hBtnFreehand, hBtnLine, hBtnRect, hBtnEllipse, hBtnSpray, hBtnFill, hBtnPipette;
HWND hBtnUndo, hBtnRedo, hBtnInvert, hBtnGray, hBtnBright, hBtnDark, hBtnFlipH, hBtnFlipV, hBtnRotate90, hBtnRotateCCW;
HWND hBtnClear, hBtnSave, hBtnOpen, hBtnHelp, hBtnDemoArt;
HWND hBtnEdge, hBtnSharpen, hBtnEmboss;

HFONT hFont = NULL;
static HBITMAP hbmStockOld = NULL;

void PushUndo();
void PerformUndo();
void PerformRedo();
void UpdatePen();
void UpdateTitleStatus(HWND hwnd);
void GenerateDemoArtwork(HWND hwnd);

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

void UpdateTitleStatus(HWND hwnd) {
    const char* toolNames[] = {"Brush", "Line", "Rect", "Circle", "Spray", "Eraser", "Fill", "Pick"};
    const char* toolName = (currentTool >= 0 && currentTool <= 7) ? toolNames[currentTool] : "Brush";
    char title[180];
    wsprintfA(title, "KPaint Pro - Tool: %s | Size: %dpx (%s) | Color: #%02X%02X%02X | Press F1 for Help, D for Demo",
        toolName, curSize, brushShape ? "Square" : "Round",
        GetRValue(curColor), GetGValue(curColor), GetBValue(curColor));
    SetWindowTextA(hwnd, title);
}

// Push state to undo stack
void PushUndo() {
    if (!hdcMem || !hbmCanvas) return;
    
    HDC hdcScreen = GetDC(NULL);
    HBITMAP hbmCopy = CreateCompatibleBitmap(hdcScreen, 2000, 2000);
    HDC hdcCopy = CreateCompatibleDC(hdcScreen);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcCopy, hbmCopy);
    BitBlt(hdcCopy, 0, 0, 2000, 2000, hdcMem, 0, 0, SRCCOPY);
    
    SelectObject(hdcCopy, hOld);
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
    HBITMAP hOld = (HBITMAP)SelectObject(hdcCopy, hbmCurrent);
    BitBlt(hdcCopy, 0, 0, 2000, 2000, hdcMem, 0, 0, SRCCOPY);
    SelectObject(hdcCopy, hOld);
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
    HBITMAP hOldTemp = (HBITMAP)SelectObject(hdcTemp, hbmRestore);
    BitBlt(hdcMem, 0, 0, 2000, 2000, hdcTemp, 0, 0, SRCCOPY);
    SelectObject(hdcTemp, hOldTemp);
    DeleteDC(hdcTemp);
    DeleteObject(hbmRestore);
}

void PerformRedo() {
    if (redoCount <= 0) return;

    // Save current to undo stack
    HDC hdcScreen = GetDC(NULL);
    HBITMAP hbmCurrent = CreateCompatibleBitmap(hdcScreen, 2000, 2000);
    HDC hdcCopy = CreateCompatibleDC(hdcScreen);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcCopy, hbmCurrent);
    BitBlt(hdcCopy, 0, 0, 2000, 2000, hdcMem, 0, 0, SRCCOPY);
    SelectObject(hdcCopy, hOld);
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
    HBITMAP hOldTemp = (HBITMAP)SelectObject(hdcTemp, hbmRestore);
    BitBlt(hdcMem, 0, 0, 2000, 2000, hdcTemp, 0, 0, SRCCOPY);
    SelectObject(hdcTemp, hOldTemp);
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
}

void FilterBrightness(int delta) {
    PushUndo();
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
}

void FilterConvolve(int type) {
    PushUndo();
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = 2000;
    bi.bmiHeader.biHeight = -2000;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    
    DWORD bufSize = 2000 * 2000 * 3;
    BYTE* pSrc = (BYTE*)GlobalAlloc(GPTR, bufSize);
    BYTE* pDst = (BYTE*)GlobalAlloc(GPTR, bufSize);
    if (pSrc && pDst) {
        GetDIBits(hdcMem, hbmCanvas, 0, 2000, pSrc, &bi, DIB_RGB_COLORS);
        
        int matrix[9];
        int div = 1;
        int offset = 0;
        
        if (type == 0) { // Edge
            int m[] = {-1,-1,-1, -1,8,-1, -1,-1,-1};
            for(int i=0;i<9;i++) matrix[i]=m[i];
        } else if (type == 1) { // Sharpen
            int m[] = {0,-1,0, -1,5,-1, 0,-1,0};
            for(int i=0;i<9;i++) matrix[i]=m[i];
        } else if (type == 2) { // Emboss
            int m[] = {-2,-1,0, -1,1,1, 0,1,2};
            for(int i=0;i<9;i++) matrix[i]=m[i];
        }

        for (int y = 1; y < 1999; y++) {
            for (int x = 1; x < 1999; x++) {
                int r=0,g=0,b=0;
                for(int cy=0; cy<3; cy++) {
                    for(int cx=0; cx<3; cx++) {
                        int idx = ((y+cy-1)*2000 + (x+cx-1))*3;
                        int wt = matrix[cy*3+cx];
                        b += pSrc[idx] * wt;
                        g += pSrc[idx+1] * wt;
                        r += pSrc[idx+2] * wt;
                    }
                }
                int dstIdx = (y*2000 + x)*3;
                pDst[dstIdx]   = (BYTE)(b/div + offset < 0 ? 0 : (b/div + offset > 255 ? 255 : b/div + offset));
                pDst[dstIdx+1] = (BYTE)(g/div + offset < 0 ? 0 : (g/div + offset > 255 ? 255 : g/div + offset));
                pDst[dstIdx+2] = (BYTE)(r/div + offset < 0 ? 0 : (r/div + offset > 255 ? 255 : r/div + offset));
            }
        }
        SetDIBits(hdcMem, hbmCanvas, 0, 2000, pDst, &bi, DIB_RGB_COLORS);
    }
    if (pSrc) GlobalFree(pSrc);
    if (pDst) GlobalFree(pDst);
}

// Transforms
void FlipHorizontal() {
    PushUndo();
    HDC hdcTemp = CreateCompatibleDC(hdcMem);
    HBITMAP hbmTemp = CreateCompatibleBitmap(hdcMem, 2000, 2000);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcTemp, hbmTemp);
    StretchBlt(hdcTemp, 0, 0, 2000, 2000, hdcMem, 1999, 0, -2000, 2000, SRCCOPY);
    BitBlt(hdcMem, 0, 0, 2000, 2000, hdcTemp, 0, 0, SRCCOPY);
    SelectObject(hdcTemp, hOld);
    DeleteDC(hdcTemp);
    DeleteObject(hbmTemp);
}

void FlipVertical() {
    PushUndo();
    HDC hdcTemp = CreateCompatibleDC(hdcMem);
    HBITMAP hbmTemp = CreateCompatibleBitmap(hdcMem, 2000, 2000);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcTemp, hbmTemp);
    StretchBlt(hdcTemp, 0, 0, 2000, 2000, hdcMem, 0, 1999, 2000, -2000, SRCCOPY);
    BitBlt(hdcMem, 0, 0, 2000, 2000, hdcTemp, 0, 0, SRCCOPY);
    SelectObject(hdcTemp, hOld);
    DeleteDC(hdcTemp);
    DeleteObject(hbmTemp);
}

void Rotate90CW() {
    PushUndo();
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
}

void Rotate90CCW() {
    PushUndo();
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
                int dstIdx = ((1999 - x) * 2000 + y) * 3;
                pDst[dstIdx]   = pSrc[srcIdx];
                pDst[dstIdx+1] = pSrc[srcIdx+1];
                pDst[dstIdx+2] = pSrc[srcIdx+2];
            }
        }
        SetDIBits(hdcMem, hbmCanvas, 0, 2000, pDst, &bi, DIB_RGB_COLORS);
    }
    if (pSrc) GlobalFree(pSrc);
    if (pDst) GlobalFree(pDst);
}

int SaveBitmap(const char* path, HBITMAP hbm) {
    HDC hdc = GetDC(NULL);
    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = 2000;
    bi.biHeight = 2000;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    DWORD dwBmpSize = ((2000 * 24 + 31) / 32) * 4 * 2000;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    if (!hDIB) { ReleaseDC(NULL, hdc); return 0; }
    char* lpbitmap = (char*)GlobalLock(hDIB);
    if (!lpbitmap) { GlobalFree(hDIB); ReleaseDC(NULL, hdc); return 0; }
    
    HDC tempDC = CreateCompatibleDC(hdc);
    HBITMAP tempBmp = CreateCompatibleBitmap(hdc, 2000, 2000);
    HBITMAP hOld = (HBITMAP)SelectObject(tempDC, tempBmp);
    RECT tr = {0,0,2000,2000}; FillRect(tempDC, &tr, (HBRUSH)GetStockObject(WHITE_BRUSH));
    BitBlt(tempDC, 0, 0, 2000, 2000, hdcMem, 0, 0, SRCCOPY);
    SelectObject(tempDC, hOld);
    
    GetDIBits(hdc, tempBmp, 0, 2000, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    int success = 0;
    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD dwBytesWritten = 0;
        BITMAPFILEHEADER bmfHeader = {0};
        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
        bmfHeader.bfSize = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bmfHeader.bfType = 0x4D42;

        if (WriteFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL) &&
            WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL) &&
            WriteFile(hFile, lpbitmap, dwBmpSize, &dwBytesWritten, NULL)) {
            success = 1;
        }
        CloseHandle(hFile);
    }
    
    DeleteDC(tempDC);
    DeleteObject(tempBmp);
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    ReleaseDC(NULL, hdc);
    return success;
}

void LoadBitmapFile(HWND hwnd, const char* path) {
    HBITMAP hLoaded = (HBITMAP)LoadImageA(NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (hLoaded) {
        PushUndo();
        HDC hdcTemp = CreateCompatibleDC(hdcMem);
        HBITMAP hOld = (HBITMAP)SelectObject(hdcTemp, hLoaded);
        BITMAP bmp;
        GetObject(hLoaded, sizeof(BITMAP), &bmp);
        RECT r = {0, 0, 2000, 2000};
        FillRect(hdcMem, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
        BitBlt(hdcMem, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcTemp, 0, 0, SRCCOPY);
        SelectObject(hdcTemp, hOld);
        DeleteDC(hdcTemp);
        DeleteObject(hLoaded);
        InvalidateRect(hwnd, NULL, FALSE);
    } else {
        MessageBoxA(hwnd, "Could not open BMP image file.", "KPaint Error", MB_OK | MB_ICONERROR);
    }
}

void GenerateDemoArtwork(HWND hwnd) {
    if (!hdcMem) return;
    PushUndo();
    
    // Sky background
    RECT rSky = {0, 0, 2000, 2000};
    HBRUSH hSky = CreateSolidBrush(RGB(15, 20, 36));
    FillRect(hdcMem, &rSky, hSky);
    DeleteObject(hSky);

    // Warm sunset gradient band
    for (int y = 140; y < 440; y++) {
        int t = y - 140;
        int r = min(255, 240 - t / 3);
        int g = min(255, 80 + t / 3);
        int b = min(255, 50 + t / 2);
        HPEN hBand = CreatePen(PS_SOLID, 1, RGB(r, g, b));
        HPEN hOld = (HPEN)SelectObject(hdcMem, hBand);
        MoveToEx(hdcMem, 0, y, NULL);
        LineTo(hdcMem, 2000, y);
        SelectObject(hdcMem, hOld);
        DeleteObject(hBand);
    }

    // Radiant Sun
    HBRUSH hSun = CreateSolidBrush(RGB(255, 225, 95));
    HPEN hSunPen = CreatePen(PS_SOLID, 2, RGB(255, 245, 140));
    HBRUSH hOldB = (HBRUSH)SelectObject(hdcMem, hSun);
    HPEN hOldP = (HPEN)SelectObject(hdcMem, hSunPen);
    Ellipse(hdcMem, 440, 180, 640, 380);
    SelectObject(hdcMem, hOldB);
    SelectObject(hdcMem, hOldP);
    DeleteObject(hSun);
    DeleteObject(hSunPen);

    // Distant Purple Mountains
    POINT mtn1[3] = { {0, 540}, {260, 280}, {520, 540} };
    HBRUSH hMtn1 = CreateSolidBrush(RGB(65, 40, 85));
    HPEN hPenMtn1 = CreatePen(PS_SOLID, 1, RGB(65, 40, 85));
    SelectObject(hdcMem, hMtn1);
    SelectObject(hdcMem, hPenMtn1);
    Polygon(hdcMem, mtn1, 3);
    DeleteObject(hMtn1); DeleteObject(hPenMtn1);
    
    POINT mtn2[3] = { {380, 540}, {720, 240}, {1060, 540} };
    HBRUSH hMtn2 = CreateSolidBrush(RGB(82, 50, 105));
    HPEN hPenMtn2 = CreatePen(PS_SOLID, 1, RGB(82, 50, 105));
    SelectObject(hdcMem, hMtn2);
    SelectObject(hdcMem, hPenMtn2);
    Polygon(hdcMem, mtn2, 3);
    DeleteObject(hMtn2); DeleteObject(hPenMtn2);

    // Midground Dark Ridge
    POINT mtn3[3] = { {140, 600}, {460, 350}, {820, 600} };
    HBRUSH hMtn3 = CreateSolidBrush(RGB(32, 42, 68));
    HPEN hPenMtn3 = CreatePen(PS_SOLID, 1, RGB(32, 42, 68));
    SelectObject(hdcMem, hMtn3);
    SelectObject(hdcMem, hPenMtn3);
    Polygon(hdcMem, mtn3, 3);
    DeleteObject(hMtn3); DeleteObject(hPenMtn3);

    // Lake with Sunset Reflection
    RECT rLake = {0, 540, 2000, 780};
    HBRUSH hLake = CreateSolidBrush(RGB(20, 30, 52));
    FillRect(hdcMem, &rLake, hLake);
    DeleteObject(hLake);

    for (int y = 560; y < 740; y += 16) {
        HPEN hRipple = CreatePen(PS_SOLID, 2, RGB(255, 175, 90));
        HPEN hOldR = (HPEN)SelectObject(hdcMem, hRipple);
        MoveToEx(hdcMem, 490 - (y - 540), y, NULL);
        LineTo(hdcMem, 590 + (y - 540), y);
        SelectObject(hdcMem, hOldR);
        DeleteObject(hRipple);
    }

    // Foreground Meadow
    RECT rGrass = {0, 780, 2000, 2000};
    HBRUSH hGrass = CreateSolidBrush(RGB(16, 28, 22));
    FillRect(hdcMem, &rGrass, hGrass);
    DeleteObject(hGrass);

    // Pine Trees
    int treeX[] = {70, 180, 300, 780, 920, 1060};
    for (int i = 0; i < 6; i++) {
        int tx = treeX[i];
        int ty = 720 + (i % 3) * 20;
        RECT rTrunk = {tx - 4, ty, tx + 4, ty + 40};
        HBRUSH hTrunk = CreateSolidBrush(RGB(55, 38, 25));
        FillRect(hdcMem, &rTrunk, hTrunk);
        DeleteObject(hTrunk);
        
        POINT pPine[3] = { {tx - 28, ty + 10}, {tx, ty - 65}, {tx + 28, ty + 10} };
        HBRUSH hPine = CreateSolidBrush(RGB(22, 52, 32));
        HPEN hPinePen = CreatePen(PS_SOLID, 1, RGB(22, 52, 32));
        SelectObject(hdcMem, hPine);
        SelectObject(hdcMem, hPinePen);
        Polygon(hdcMem, pPine, 3);
        DeleteObject(hPine);
        DeleteObject(hPinePen);
    }

    // Retro Title Badge on Canvas
    HFONT hTitleFont = CreateFontA(-22, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 5, DEFAULT_PITCH, "Segoe UI");
    HFONT hOldF = (HFONT)SelectObject(hdcMem, hTitleFont);
    SetTextColor(hdcMem, RGB(245, 245, 255));
    SetBkMode(hdcMem, TRANSPARENT);
    TextOutA(hdcMem, 40, 40, "KPaint Demo Art (Mountain Sunset)", 33);
    HFONT hSubFont = CreateFontA(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 5, DEFAULT_PITCH, "Segoe UI");
    SelectObject(hdcMem, hSubFont);
    SetTextColor(hdcMem, RGB(180, 205, 230));
    TextOutA(hdcMem, 40, 72, "Test tools: Pick [I], Fill [G], Spray [A], Invert [V], Gray [Y], Size [1-3], F1 Help", 84);
    SelectObject(hdcMem, hOldF);
    DeleteObject(hTitleFont);
    DeleteObject(hSubFont);

    UpdatePen();
    UpdateTitleStatus(hwnd);
    InvalidateRect(hwnd, NULL, FALSE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HDC hdc = GetDC(hwnd);
            hdcMem = CreateCompatibleDC(hdc);
            hbmCanvas = CreateCompatibleBitmap(hdc, 2000, 2000);
            hbmStockOld = (HBITMAP)SelectObject(hdcMem, hbmCanvas);
            
            RECT r = {0, 0, 2000, 2000};
            FillRect(hdcMem, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
            
            HFONT hWelcomeFont = CreateFontA(-24, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 5, DEFAULT_PITCH, "Segoe UI");
            SetTextColor(hdcMem, RGB(150, 150, 150));
            SetBkMode(hdcMem, TRANSPARENT);
            HFONT hOldF = (HFONT)SelectObject(hdcMem, hWelcomeFont);
            TextOutA(hdcMem, 20, 20, "Welcome to KPaint Pro! Press F1 for Help | D for Demo Art", 58);
            SelectObject(hdcMem, hOldF);
            DeleteObject(hWelcomeFont);
            
            ReleaseDC(hwnd, hdc);
            
            UpdatePen();
            
            hFont = CreateFontA(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 5, DEFAULT_PITCH, "Segoe UI");
            
            // Colors
            hBtnBlack = CreateWindowA("BUTTON", "Black", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 5, 58, 22, hwnd, (HMENU)101, NULL, NULL);
            hBtnRed = CreateWindowA("BUTTON", "Red", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 5, 58, 22, hwnd, (HMENU)102, NULL, NULL);
            hBtnGreen = CreateWindowA("BUTTON", "Green", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 30, 58, 22, hwnd, (HMENU)103, NULL, NULL);
            hBtnBlue = CreateWindowA("BUTTON", "Blue", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 30, 58, 22, hwnd, (HMENU)104, NULL, NULL);
            hBtnYellow = CreateWindowA("BUTTON", "Yellow", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 55, 58, 22, hwnd, (HMENU)105, NULL, NULL);
            hBtnPurple = CreateWindowA("BUTTON", "Purple", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 55, 58, 22, hwnd, (HMENU)106, NULL, NULL);
            hBtnCustomColor = CreateWindowA("BUTTON", "Custom...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 80, 121, 22, hwnd, (HMENU)107, NULL, NULL);

            // Tools & Eraser
            hBtnFreehand = CreateWindowA("BUTTON", "Brush (B)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 107, 58, 22, hwnd, (HMENU)401, NULL, NULL);
            hBtnLine = CreateWindowA("BUTTON", "Line (L)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 107, 58, 22, hwnd, (HMENU)402, NULL, NULL);
            hBtnRect = CreateWindowA("BUTTON", "Rect (R)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 132, 58, 22, hwnd, (HMENU)403, NULL, NULL);
            hBtnEllipse = CreateWindowA("BUTTON", "Circle (C)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 132, 58, 22, hwnd, (HMENU)404, NULL, NULL);
            hBtnSpray = CreateWindowA("BUTTON", "Spray (A)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 157, 58, 22, hwnd, (HMENU)405, NULL, NULL);
            hBtnEraser = CreateWindowA("BUTTON", "Eraser (E)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 157, 58, 22, hwnd, (HMENU)406, NULL, NULL);
            hBtnFill = CreateWindowA("BUTTON", "Fill (G)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 182, 58, 22, hwnd, (HMENU)407, NULL, NULL);
            hBtnPipette = CreateWindowA("BUTTON", "Pick (I)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 182, 58, 22, hwnd, (HMENU)408, NULL, NULL);

            // Size & Shape
            hBtnSizeSmall = CreateWindowA("BUTTON", "2px [1]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 209, 38, 22, hwnd, (HMENU)201, NULL, NULL);
            hBtnSizeMed = CreateWindowA("BUTTON", "6px [2]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 46, 209, 38, 22, hwnd, (HMENU)202, NULL, NULL);
            hBtnSizeLarge = CreateWindowA("BUTTON", "14px [3]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 87, 209, 39, 22, hwnd, (HMENU)203, NULL, NULL);
            hBtnShapeToggle = CreateWindowA("BUTTON", "Shape: Round", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 234, 121, 22, hwnd, (HMENU)204, NULL, NULL);

            // Filters & Transforms
            hBtnInvert = CreateWindowA("BUTTON", "Invert [V]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 261, 58, 22, hwnd, (HMENU)501, NULL, NULL);
            hBtnGray = CreateWindowA("BUTTON", "Gray [Y]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 261, 58, 22, hwnd, (HMENU)502, NULL, NULL);
            hBtnBright = CreateWindowA("BUTTON", "+Bright [+]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 286, 58, 22, hwnd, (HMENU)503, NULL, NULL);
            hBtnDark = CreateWindowA("BUTTON", "-Bright [-]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 286, 58, 22, hwnd, (HMENU)509, NULL, NULL);
            hBtnFlipH = CreateWindowA("BUTTON", "Flip H", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 311, 58, 22, hwnd, (HMENU)504, NULL, NULL);
            hBtnFlipV = CreateWindowA("BUTTON", "Flip V", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 311, 58, 22, hwnd, (HMENU)510, NULL, NULL);
            hBtnRotate90 = CreateWindowA("BUTTON", "Rot CW", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 336, 58, 22, hwnd, (HMENU)505, NULL, NULL);
            hBtnRotateCCW = CreateWindowA("BUTTON", "Rot CCW", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 336, 58, 22, hwnd, (HMENU)511, NULL, NULL);

            // Convolve Filters
            hBtnEdge = CreateWindowA("BUTTON", "Edge", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 363, 38, 22, hwnd, (HMENU)506, NULL, NULL);
            hBtnSharpen = CreateWindowA("BUTTON", "Sharp", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 46, 363, 38, 22, hwnd, (HMENU)507, NULL, NULL);
            hBtnEmboss = CreateWindowA("BUTTON", "Emboss", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 87, 363, 39, 22, hwnd, (HMENU)508, NULL, NULL);

            // History & Actions
            hBtnUndo = CreateWindowA("BUTTON", "Undo ^Z", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 390, 58, 22, hwnd, (HMENU)601, NULL, NULL);
            hBtnRedo = CreateWindowA("BUTTON", "Redo ^Y", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 390, 58, 22, hwnd, (HMENU)602, NULL, NULL);
            hBtnOpen = CreateWindowA("BUTTON", "Open ^O", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 415, 58, 22, hwnd, (HMENU)303, NULL, NULL);
            hBtnSave = CreateWindowA("BUTTON", "Save ^S", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 415, 58, 22, hwnd, (HMENU)302, NULL, NULL);
            hBtnClear = CreateWindowA("BUTTON", "Clear Canvas", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 440, 121, 22, hwnd, (HMENU)301, NULL, NULL);
            hBtnHelp = CreateWindowA("BUTTON", "Help (F1)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 465, 121, 22, hwnd, (HMENU)701, NULL, NULL);
            hBtnDemoArt = CreateWindowA("BUTTON", "Demo Art (D)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 490, 121, 22, hwnd, (HMENU)304, NULL, NULL);

            HWND controls[] = {
                hBtnBlack, hBtnRed, hBtnGreen, hBtnBlue, hBtnYellow, hBtnPurple, hBtnCustomColor,
                hBtnFreehand, hBtnLine, hBtnRect, hBtnEllipse, hBtnSpray, hBtnEraser, hBtnFill, hBtnPipette,
                hBtnSizeSmall, hBtnSizeMed, hBtnSizeLarge, hBtnShapeToggle,
                hBtnInvert, hBtnGray, hBtnBright, hBtnDark, hBtnFlipH, hBtnFlipV, hBtnRotate90, hBtnRotateCCW,
                hBtnEdge, hBtnSharpen, hBtnEmboss,
                hBtnUndo, hBtnRedo, hBtnOpen, hBtnSave, hBtnClear, hBtnHelp, hBtnDemoArt
            };
            for (int i = 0; i < sizeof(controls)/sizeof(controls[0]); i++) {
                SendMessage(controls[i], WM_SETFONT, (WPARAM)hFont, TRUE);
            }

            DragAcceptFiles(hwnd, TRUE);
            UpdateTitleStatus(hwnd);
            break;
        }
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            char droppedFile[MAX_PATH] = {0};
            if (DragQueryFileA(hDrop, 0, droppedFile, MAX_PATH)) {
                LoadBitmapFile(hwnd, droppedFile);
            }
            DragFinish(hDrop);
            break;
        }
        case WM_ERASEBKGND:
            return 1;
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
            
            if (id >= 101 && id <= 204) {
                UpdatePen();
                UpdateTitleStatus(hwnd);
            }
            
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
                        MessageBoxA(hwnd, "Failed to save bitmap.", "KPaint Error", MB_OK | MB_ICONERROR);
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
            
            if (id == 401) { currentTool = 0; UpdatePen(); UpdateTitleStatus(hwnd); }
            if (id == 402) { currentTool = 1; UpdatePen(); UpdateTitleStatus(hwnd); }
            if (id == 403) { currentTool = 2; UpdatePen(); UpdateTitleStatus(hwnd); }
            if (id == 404) { currentTool = 3; UpdatePen(); UpdateTitleStatus(hwnd); }
            if (id == 405) { currentTool = 4; UpdatePen(); UpdateTitleStatus(hwnd); }
            if (id == 406) { currentTool = 5; UpdatePen(); UpdateTitleStatus(hwnd); }
            if (id == 407) { currentTool = 6; UpdatePen(); UpdateTitleStatus(hwnd); }
            if (id == 408) { currentTool = 7; UpdatePen(); UpdateTitleStatus(hwnd); }

            if (id == 501) { FilterInvert(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 502) { FilterGrayscale(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 503) { FilterBrightness(25); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 509) { FilterBrightness(-25); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 504) { FlipHorizontal(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 510) { FlipVertical(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 505) { Rotate90CW(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 511) { Rotate90CCW(); InvalidateRect(hwnd, NULL, FALSE); }

            if (id == 506) { FilterConvolve(0); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 507) { FilterConvolve(1); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 508) { FilterConvolve(2); InvalidateRect(hwnd, NULL, FALSE); }

            if (id == 304) { GenerateDemoArtwork(hwnd); }
            if (id == 601) { PerformUndo(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 602) { PerformRedo(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 701) {
                MessageBoxA(hwnd,
                    "KPaint Pro - Help & Shortcuts Guide\n\n"
                    "Drawing Tools:\n"
                    "  [B]  Brush (Freehand drawing)\n"
                    "  [L]  Line\n"
                    "  [R]  Rectangle\n"
                    "  [C]  Circle / Ellipse\n"
                    "  [A]  Spray / Airbrush\n"
                    "  [E]  Eraser\n"
                    "  [G]  Bucket Fill\n"
                    "  [I]  Eyedropper / Color Pick\n\n"
                    "Brush & Shapes:\n"
                    "  [1]  Small (2px)    [2] Medium (6px)    [3] Large (14px)\n"
                    "  [ [ ] / [ ] ]  Decrease / Increase Brush Size\n"
                    "  Shape Toggle: Round / Square\n\n"
                    "Image Filters & Transforms:\n"
                    "  [V]  Invert Colors    [Y] Grayscale\n"
                    "  [+]  +Brightness     [-] -Brightness\n"
                    "  Edge Detect, Sharpen, Emboss, Flip H/V, Rotate CW/CCW\n\n"
                    "Quick Starter & Actions:\n"
                    "  [D]  Generate Demo Artwork (Mountain Sunset)\n"
                    "  Ctrl+Z  Undo          Ctrl+Y  Redo\n"
                    "  Ctrl+S  Save BMP      Ctrl+O  Open BMP\n"
                    "  F1 / H  This Help Guide\n\n"
                    "Drag & drop any BMP file onto the window to open!",
                    "KPaint Help", MB_OK | MB_ICONINFORMATION);
            }
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
                } else if (wParam == 'S' || wParam == 's') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(302, 0), 0);
                } else if (wParam == 'O' || wParam == 'o') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(303, 0), 0);
                }
            } else if (wParam == VK_F1 || wParam == 'H' || wParam == 'h') {
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(701, 0), 0);
            } else if (wParam == 'B' || wParam == 'b') {
                currentTool = 0; UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == 'L' || wParam == 'l') {
                currentTool = 1; UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == 'R' || wParam == 'r') {
                currentTool = 2; UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == 'C' || wParam == 'c') {
                currentTool = 3; UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == 'A' || wParam == 'a' || wParam == 'S' || wParam == 's') {
                currentTool = 4; UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == 'E' || wParam == 'e') {
                currentTool = 5; UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == 'G' || wParam == 'g') {
                currentTool = 6; UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == 'I' || wParam == 'i') {
                currentTool = 7; UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == 'D' || wParam == 'd') {
                GenerateDemoArtwork(hwnd);
            } else if (wParam == '1') {
                curSize = 2; UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == '2') {
                curSize = 6; UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == '3') {
                curSize = 14; UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == 'V' || wParam == 'v') {
                FilterInvert(); InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'Y' || wParam == 'y') {
                FilterGrayscale(); InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == VK_OEM_4 /* [ */) {
                curSize = max(1, curSize - 2);
                UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == VK_OEM_6 /* ] */) {
                curSize = min(80, curSize + 2);
                UpdatePen(); UpdateTitleStatus(hwnd);
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = (short)LOWORD(lParam) - 130 + scrollX;
            int y = (short)HIWORD(lParam) + scrollY;
            if ((short)LOWORD(lParam) >= 130) {
                if (currentTool == 7) { // Pipette / Pick
                    curColor = GetPixel(hdcMem, x, y);
                    currentTool = 0; // return to brush
                    UpdatePen();
                    UpdateTitleStatus(hwnd);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (currentTool == 6) { // Bucket Fill
                    PushUndo();
                    SelectObject(hdcMem, hBrush);
                    COLORREF target = GetPixel(hdcMem, x, y);
                    if (target != curColor) {
                        ExtFloodFill(hdcMem, x, y, target, FLOODFILLSURFACE);
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                PushUndo();
                isPainting = 1;
                startX = x; startY = y;
                lastX = x; lastY = y;
                if (currentTool == 0 || currentTool == 5) {
                    HDC hdc = GetDC(hwnd);
                    SelectObject(hdc, hPen);
                    MoveToEx(hdc, x + 130 - scrollX, y - scrollY, NULL);
                    LineTo(hdc, x + 130 - scrollX, y - scrollY);
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
                    if (currentTool == 1) { MoveToEx(hdc, startX + 130 - scrollX, startY - scrollY, NULL); LineTo(hdc, x + 130 - scrollX, y - scrollY); }
                    else if (currentTool == 2) { Rectangle(hdc, startX + 130 - scrollX, startY - scrollY, x + 130 - scrollX, y - scrollY); }
                    else if (currentTool == 3) { Ellipse(hdc, startX + 130 - scrollX, startY - scrollY, x + 130 - scrollX, y - scrollY); }
                    ReleaseDC(hwnd, hdc);
                }
            }
            break;
        }
        case WM_MOUSEMOVE: {
            int x = (short)LOWORD(lParam) - 130 + scrollX;
            int y = (short)HIWORD(lParam) + scrollY;
            if (isPainting && (short)LOWORD(lParam) >= 130) {
                HDC hdc = GetDC(hwnd);
                if (currentTool == 0 || currentTool == 5) {
                    SelectObject(hdc, hPen);
                    MoveToEx(hdc, lastX + 130 - scrollX, lastY - scrollY, NULL);
                    LineTo(hdc, x + 130 - scrollX, y - scrollY);
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
                    if (currentTool == 1) { MoveToEx(hdc, startX + 130 - scrollX, startY - scrollY, NULL); LineTo(hdc, lastX + 130 - scrollX, lastY - scrollY); }
                    else if (currentTool == 2) { Rectangle(hdc, startX + 130 - scrollX, startY - scrollY, lastX + 130 - scrollX, lastY - scrollY); }
                    else if (currentTool == 3) { Ellipse(hdc, startX + 130 - scrollX, startY - scrollY, lastX + 130 - scrollX, lastY - scrollY); }
                    
                    if (currentTool == 1) { MoveToEx(hdc, startX + 130 - scrollX, startY - scrollY, NULL); LineTo(hdc, x + 130 - scrollX, y - scrollY); }
                    else if (currentTool == 2) { Rectangle(hdc, startX + 130 - scrollX, startY - scrollY, x + 130 - scrollX, y - scrollY); }
                    else if (currentTool == 3) { Ellipse(hdc, startX + 130 - scrollX, startY - scrollY, x + 130 - scrollX, y - scrollY); }
                }
                ReleaseDC(hwnd, hdc);
                lastX = x;
                lastY = y;
            }
            break;
        }
        case WM_LBUTTONUP: {
            if (isPainting) {
                int x = (short)LOWORD(lParam) - 130 + scrollX;
                int y = (short)HIWORD(lParam) + scrollY;
                if (currentTool != 0 && currentTool != 5 && currentTool != 4 && currentTool != 6 && currentTool != 7) {
                    HDC hdc = GetDC(hwnd);
                    SetROP2(hdc, R2_NOTXORPEN);
                    SelectObject(hdc, hPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    if (currentTool == 1) { MoveToEx(hdc, startX + 130 - scrollX, startY - scrollY, NULL); LineTo(hdc, lastX + 130 - scrollX, lastY - scrollY); }
                    else if (currentTool == 2) { Rectangle(hdc, startX + 130 - scrollX, startY - scrollY, lastX + 130 - scrollX, lastY - scrollY); }
                    else if (currentTool == 3) { Ellipse(hdc, startX + 130 - scrollX, startY - scrollY, lastX + 130 - scrollX, lastY - scrollY); }
                    ReleaseDC(hwnd, hdc);

                    SelectObject(hdcMem, hPen);
                    SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
                    if (currentTool == 1) { MoveToEx(hdcMem, startX, startY, NULL); LineTo(hdcMem, x, y); }
                    else if (currentTool == 2) { Rectangle(hdcMem, startX, startY, x, y); }
                    else if (currentTool == 3) { Ellipse(hdcMem, startX, startY, x, y); }
                    SelectObject(hdcMem, hBrush);
                    
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
            RECT sidebar = {0, 0, 130, r.bottom};
            FillRect(hdc, &sidebar, (HBRUSH)(COLOR_BTNFACE + 1));
            BitBlt(hdc, 130, 0, r.right - 130, r.bottom, hdcMem, scrollX, scrollY, SRCCOPY);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_SIZE: {
            SCROLLINFO si = {0};
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_RANGE | SIF_PAGE;
            si.nMin = 0;
            si.nMax = 1999;
            si.nPage = HIWORD(lParam);
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            si.nPage = max(0, LOWORD(lParam) - 130);
            SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
            break;
        }
        case WM_VSCROLL: {
            int action = LOWORD(wParam);
            if (action == SB_LINEUP) scrollY -= 20;
            else if (action == SB_LINEDOWN) scrollY += 20;
            else if (action == SB_PAGEUP) scrollY -= 100;
            else if (action == SB_PAGEDOWN) scrollY += 100;
            else if (action == SB_THUMBTRACK) scrollY = HIWORD(wParam);
            if (scrollY < 0) scrollY = 0;
            if (scrollY > 2000 - HIWORD(lParam)) scrollY = max(0, 2000 - HIWORD(lParam));
            SetScrollPos(hwnd, SB_VERT, scrollY, TRUE);
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_HSCROLL: {
            int action = LOWORD(wParam);
            if (action == SB_LINELEFT) scrollX -= 20;
            else if (action == SB_LINERIGHT) scrollX += 20;
            else if (action == SB_PAGELEFT) scrollX -= 100;
            else if (action == SB_PAGERIGHT) scrollX += 100;
            else if (action == SB_THUMBTRACK) scrollX = HIWORD(wParam);
            if (scrollX < 0) scrollX = 0;
            if (scrollX > 2000 - (LOWORD(lParam) - 130)) scrollX = max(0, 2000 - (LOWORD(lParam) - 130));
            SetScrollPos(hwnd, SB_HORZ, scrollX, TRUE);
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_DESTROY:
            SelectObject(hdcMem, GetStockObject(BLACK_PEN));
            SelectObject(hdcMem, GetStockObject(WHITE_BRUSH));
            if (hbmStockOld) SelectObject(hdcMem, hbmStockOld);
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
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *SetDPIFunc)(void);
        SetDPIFunc setDpi = (SetDPIFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDpi) setDpi();
    }

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KPaintClass";
    wc.hCursor = LoadCursorA(NULL, IDC_CROSS);
    wc.hbrBackground = NULL;

    RegisterClassA(&wc);
    RECT wr = {0, 0, 1100, 750};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN, FALSE);
    HWND hwnd = CreateWindowExA(0, "KPaintClass", "KPaint Pro - Press F1 or H for Help", WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            WPARAM wParam = msg.wParam;
            BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            if (bCtrl) {
                if (wParam == 'Z' || wParam == 'z') {
                    PerformUndo();
                    InvalidateRect(hwnd, NULL, FALSE);
                    continue;
                } else if (wParam == 'Y' || wParam == 'y') {
                    PerformRedo();
                    InvalidateRect(hwnd, NULL, FALSE);
                    continue;
                } else if (wParam == 'S' || wParam == 's') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(302, 0), 0);
                    continue;
                } else if (wParam == 'O' || wParam == 'o') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(303, 0), 0);
                    continue;
                }
            } else {
                if (wParam == VK_F1 || wParam == 'H' || wParam == 'h') {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(701, 0), 0);
                    continue;
                } else if (wParam == 'B' || wParam == 'b') {
                    currentTool = 0; UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == 'L' || wParam == 'l') {
                    currentTool = 1; UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == 'R' || wParam == 'r') {
                    currentTool = 2; UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == 'C' || wParam == 'c') {
                    currentTool = 3; UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == 'A' || wParam == 'a' || wParam == 'S' || wParam == 's') {
                    currentTool = 4; UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == 'E' || wParam == 'e') {
                    currentTool = 5; UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == 'G' || wParam == 'g') {
                    currentTool = 6; UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == 'I' || wParam == 'i') {
                    currentTool = 7; UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == 'D' || wParam == 'd') {
                    GenerateDemoArtwork(hwnd);
                    continue;
                } else if (wParam == '1') {
                    curSize = 2; UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == '2') {
                    curSize = 6; UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == '3') {
                    curSize = 14; UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == 'V' || wParam == 'v') {
                    FilterInvert(); InvalidateRect(hwnd, NULL, FALSE);
                    continue;
                } else if (wParam == 'Y' || wParam == 'y') {
                    FilterGrayscale(); InvalidateRect(hwnd, NULL, FALSE);
                    continue;
                } else if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
                    FilterBrightness(25); InvalidateRect(hwnd, NULL, FALSE);
                    continue;
                } else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
                    FilterBrightness(-25); InvalidateRect(hwnd, NULL, FALSE);
                    continue;
                } else if (wParam == VK_OEM_4 /* [ */) {
                    curSize = max(1, curSize - 2); UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == VK_OEM_6 /* ] */) {
                    curSize = min(80, curSize + 2); UpdatePen(); UpdateTitleStatus(hwnd);
                    continue;
                }
            }
        }
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    ExitProcess(0);
}
