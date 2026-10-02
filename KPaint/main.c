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
COLORREF curColorSec = RGB(255,255,255);
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
HWND hBtnSizeSmall, hBtnSizeMed, hBtnSizeLarge, hBtnShapeToggle, hBtnMirror;
HWND hBtnFreehand, hBtnLine, hBtnRect, hBtnEllipse, hBtnSpray, hBtnFill, hBtnPipette;
HWND hBtnUndo, hBtnRedo, hBtnInvert, hBtnGray, hBtnBright, hBtnDark, hBtnFlipH, hBtnFlipV, hBtnRotate90, hBtnRotateCCW;
HWND hBtnClear, hBtnSave, hBtnOpen, hBtnQuickSave, hBtnQuickLoad, hBtnHelp, hBtnDemoArt;
HWND hBtnEdge, hBtnSharpen, hBtnEmboss, hBtnDither, hBtnScanlines;
HWND hBtnSepia, hBtnCGA, hBtnExportC;
HWND hBtnGameBoy, hBtnPixelize, hBtnSolarize, hBtnExportPPM, hBtnSwapColor, hBtnSignalArt;
HWND hBtnExportPCX, hBtnExportXBM;
int mirrorMode = 0; // 0 = Normal, 1 = Horizontal Mirror

HFONT hFont = NULL;
static HBITMAP hbmStockOld = NULL;

#define QUICKSAVE_BMP "kpaint_quicksave.bmp"
#define QUICKSAVE_DAT "kpaint_quicksave.dat"

typedef struct {
    DWORD magic; // 0x4B504153 'KPAS'
    COLORREF curColor;
    int curSize;
    int brushShape;
    int currentTool;
    int mirrorMode;
} KPaintQuickState;

void PushUndo();
void PerformUndo();
void PerformRedo();
void UpdatePen();
void UpdateTitleStatus(HWND hwnd);
void GenerateDemoArtwork(HWND hwnd);
void GenerateSignalArtwork(HWND hwnd);
void FilterDither1Bit();
void FilterScanlines();
void FilterSepia();
void FilterCGADither();
void FilterGameBoyDither();
void FilterPixelize();
void FilterSolarize();
void ExportCHeader(HWND hwnd);
void ExportPPM(HWND hwnd);
void ExportPCX(HWND hwnd);
void ExportXBM(HWND hwnd);
void QuicksaveState(HWND hwnd);
void QuickloadState(HWND hwnd);

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
    char title[320];
    wsprintfA(title, "KPaint Pro - %s | %dpx (%s) | FG:#%02X%02X%02X BG:#%02X%02X%02X [X swap] | Mirror: %s | F5 QSave, F9 QLoad",
        toolName, curSize, brushShape ? "Square" : "Round",
        GetRValue(curColor), GetGValue(curColor), GetBValue(curColor),
        GetRValue(curColorSec), GetGValue(curColorSec), GetBValue(curColorSec),
        mirrorMode ? "ON" : "OFF");
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

void FilterDither1Bit() {
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
    int* pCurErr = (int*)GlobalAlloc(GPTR, 2002 * sizeof(int));
    int* pNextErr = (int*)GlobalAlloc(GPTR, 2002 * sizeof(int));

    if (pBits && pCurErr && pNextErr) {
        GetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);

        for (int y = 0; y < 2000; y++) {
            memset(pNextErr, 0, 2002 * sizeof(int));
            for (int x = 0; x < 2000; x++) {
                int idx = (y * 2000 + x) * 3;
                int b = pBits[idx];
                int g = pBits[idx+1];
                int r = pBits[idx+2];
                int gray = (r * 77 + g * 151 + b * 28) >> 8;
                int val = gray + pCurErr[x + 1];
                BYTE outVal = (val > 127) ? 255 : 0;
                int err = val - outVal;

                pBits[idx]   = outVal;
                pBits[idx+1] = outVal;
                pBits[idx+2] = outVal;

                pCurErr[x + 2] += (err * 7) / 16;
                pNextErr[x]     += (err * 3) / 16;
                pNextErr[x + 1] += (err * 5) / 16;
                pNextErr[x + 2] += (err * 1) / 16;
            }
            int* temp = pCurErr;
            pCurErr = pNextErr;
            pNextErr = temp;
        }

        SetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
    }
    if (pBits) GlobalFree(pBits);
    if (pCurErr) GlobalFree(pCurErr);
    if (pNextErr) GlobalFree(pNextErr);
}

void FilterScanlines() {
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
        for (int y = 0; y < 2000; y++) {
            if ((y % 3) == 0) {
                for (int x = 0; x < 2000; x++) {
                    int idx = (y * 2000 + x) * 3;
                    pBits[idx]   = (BYTE)((pBits[idx] * 130) / 255);
                    pBits[idx+1] = (BYTE)((pBits[idx+1] * 130) / 255);
                    pBits[idx+2] = (BYTE)((pBits[idx+2] * 130) / 255);
                }
            }
        }
        SetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
        GlobalFree(pBits);
    }
}

void FilterSepia() {
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
            int b = pBits[i];
            int g = pBits[i+1];
            int r = pBits[i+2];
            int outR = (r * 393 + g * 769 + b * 189) / 1000;
            int outG = (r * 349 + g * 686 + b * 168) / 1000;
            int outB = (r * 272 + g * 534 + b * 131) / 1000;
            pBits[i]   = (BYTE)(outB > 255 ? 255 : outB);
            pBits[i+1] = (BYTE)(outG > 255 ? 255 : outG);
            pBits[i+2] = (BYTE)(outR > 255 ? 255 : outR);
        }
        SetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
        GlobalFree(pBits);
    }
}

void FilterCGADither() {
    PushUndo();
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = 2000;
    bi.bmiHeader.biHeight = -2000;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;

    static const BYTE cgaR[4] = {0, 0, 255, 255};
    static const BYTE cgaG[4] = {0, 255, 0, 255};
    static const BYTE cgaB[4] = {0, 255, 255, 255};

    DWORD bufSize = 2000 * 2000 * 3;
    BYTE* pBits = (BYTE*)GlobalAlloc(GPTR, bufSize);
    if (pBits) {
        GetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
        for (DWORD i = 0; i < bufSize; i += 3) {
            int b = pBits[i];
            int g = pBits[i+1];
            int r = pBits[i+2];
            int bestIdx = 0;
            int bestDist = 9999999;
            for (int k = 0; k < 4; k++) {
                int dr = r - cgaR[k];
                int dg = g - cgaG[k];
                int db = b - cgaB[k];
                int dist = dr*dr + dg*dg + db*db;
                if (dist < bestDist) {
                    bestDist = dist;
                    bestIdx = k;
                }
            }
            pBits[i]   = cgaB[bestIdx];
            pBits[i+1] = cgaG[bestIdx];
            pBits[i+2] = cgaR[bestIdx];
        }
        SetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
        GlobalFree(pBits);
    }
}

void ExportCHeader(HWND hwnd) {
    char file[260] = "kpaint_sprite.h";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = file;
    ofn.nMaxFile = 260;
    ofn.lpstrFilter = "C Header Files (*.h)\0*.h\0All Files\0*.*\0";
    ofn.lpstrDefExt = "h";
    ofn.Flags = OFN_OVERWRITEPROMPT;
    if (!GetSaveFileNameA(&ofn)) return;

    HANDLE hFile = CreateFileA(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(hwnd, "Failed to create C header file.", "KPaint Error", MB_OK | MB_ICONERROR);
        return;
    }

    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = 320;
    bi.bmiHeader.biHeight = -240;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;

    DWORD sampleSize = 320 * 240 * 3;
    BYTE* pBits = (BYTE*)GlobalAlloc(GPTR, sampleSize);
    if (pBits) {
        HDC hdcThumb = CreateCompatibleDC(hdcMem);
        HBITMAP hbmThumb = CreateCompatibleBitmap(hdcMem, 320, 240);
        HBITMAP hOld = (HBITMAP)SelectObject(hdcThumb, hbmThumb);
        StretchBlt(hdcThumb, 0, 0, 320, 240, hdcMem, 0, 0, 640, 480, SRCCOPY);
        GetDIBits(hdcThumb, hbmThumb, 0, 240, pBits, &bi, DIB_RGB_COLORS);
        SelectObject(hdcThumb, hOld);
        DeleteObject(hbmThumb);
        DeleteDC(hdcThumb);

        char headerBuf[512];
        DWORD written = 0;
        wsprintfA(headerBuf,
            "/* KPaint Pro - Win32 Exported Sprite Array */\n"
            "#ifndef KPAINT_SPRITE_H\n"
            "#define KPAINT_SPRITE_H\n\n"
            "#define SPRITE_WIDTH  320\n"
            "#define SPRITE_HEIGHT 240\n\n"
            "static const unsigned long sprite_pixels[76800] = {\n  ");
        WriteFile(hFile, headerBuf, lstrlenA(headerBuf), &written, NULL);

        char numBuf[32];
        for (int i = 0; i < 320 * 240; i++) {
            BYTE b = pBits[i*3];
            BYTE g = pBits[i*3+1];
            BYTE r = pBits[i*3+2];
            DWORD color = (0xFF000000) | (r << 16) | (g << 8) | b;
            wsprintfA(numBuf, "0x%08X%s%s", (unsigned int)color,
                (i < 76799 ? ", " : "\n};\n\n#endif\n"),
                ((i + 1) % 8 == 0 && i < 76799 ? "\n  " : ""));
            WriteFile(hFile, numBuf, lstrlenA(numBuf), &written, NULL);
        }
        GlobalFree(pBits);
        CloseHandle(hFile);
        MessageBoxA(hwnd, "Exported 320x240 sprite array to C Header successfully!", "KPaint Export", MB_OK | MB_ICONINFORMATION);
    } else {
        CloseHandle(hFile);
    }
}

void FilterGameBoyDither() {
    PushUndo();
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = 2000;
    bi.bmiHeader.biHeight = -2000;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;

    static const BYTE gbR[4] = {15, 48, 139, 155};
    static const BYTE gbG[4] = {56, 98, 172, 188};
    static const BYTE gbB[4] = {15, 48, 15, 15};

    DWORD bufSize = 2000 * 2000 * 3;
    BYTE* pBits = (BYTE*)GlobalAlloc(GPTR, bufSize);
    if (pBits) {
        GetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
        for (DWORD i = 0; i < bufSize; i += 3) {
            int b = pBits[i];
            int g = pBits[i+1];
            int r = pBits[i+2];
            int bestIdx = 0;
            int bestDist = 9999999;
            for (int k = 0; k < 4; k++) {
                int dr = r - gbR[k];
                int dg = g - gbG[k];
                int db = b - gbB[k];
                int dist = dr*dr + dg*dg + db*db;
                if (dist < bestDist) {
                    bestDist = dist;
                    bestIdx = k;
                }
            }
            pBits[i]   = gbB[bestIdx];
            pBits[i+1] = gbG[bestIdx];
            pBits[i+2] = gbR[bestIdx];
        }
        SetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
        GlobalFree(pBits);
    }
}

void FilterPixelize() {
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
        const int bs = 8;
        for (int by = 0; by < 2000; by += bs) {
            for (int bx = 0; bx < 2000; bx += bs) {
                int sumR = 0, sumG = 0, sumB = 0, count = 0;
                for (int dy = 0; dy < bs && by + dy < 2000; dy++) {
                    for (int dx = 0; dx < bs && bx + dx < 2000; dx++) {
                        int idx = ((by + dy) * 2000 + (bx + dx)) * 3;
                        sumB += pBits[idx];
                        sumG += pBits[idx+1];
                        sumR += pBits[idx+2];
                        count++;
                    }
                }
                BYTE avgB = (BYTE)(sumB / count);
                BYTE avgG = (BYTE)(sumG / count);
                BYTE avgR = (BYTE)(sumR / count);
                for (int dy = 0; dy < bs && by + dy < 2000; dy++) {
                    for (int dx = 0; dx < bs && bx + dx < 2000; dx++) {
                        int idx = ((by + dy) * 2000 + (bx + dx)) * 3;
                        pBits[idx] = avgB;
                        pBits[idx+1] = avgG;
                        pBits[idx+2] = avgR;
                    }
                }
            }
        }
        SetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
        GlobalFree(pBits);
    }
}

void FilterSolarize() {
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
            if (pBits[i] > 128) {
                pBits[i] = (BYTE)(255 - pBits[i]);
            }
        }
        SetDIBits(hdcMem, hbmCanvas, 0, 2000, pBits, &bi, DIB_RGB_COLORS);
        GlobalFree(pBits);
    }
}

void ExportPPM(HWND hwnd) {
    char file[260] = "kpaint_artwork.ppm";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = file;
    ofn.nMaxFile = 260;
    ofn.lpstrFilter = "PPM Portable Pixmap (*.ppm)\0*.ppm\0All Files\0*.*\0";
    ofn.lpstrDefExt = "ppm";
    ofn.Flags = OFN_OVERWRITEPROMPT;
    if (!GetSaveFileNameA(&ofn)) return;

    HANDLE hFile = CreateFileA(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(hwnd, "Failed to create PPM file.", "KPaint Error", MB_OK | MB_ICONERROR);
        return;
    }

    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = 800;
    bi.bmiHeader.biHeight = -600;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;

    DWORD dwSize = 800 * 600 * 3;
    BYTE* pBits = (BYTE*)GlobalAlloc(GPTR, dwSize);
    if (pBits) {
        HDC hdcThumb = CreateCompatibleDC(hdcMem);
        HBITMAP hbmThumb = CreateCompatibleBitmap(hdcMem, 800, 600);
        HBITMAP hOld = (HBITMAP)SelectObject(hdcThumb, hbmThumb);
        BitBlt(hdcThumb, 0, 0, 800, 600, hdcMem, 0, 0, SRCCOPY);
        GetDIBits(hdcThumb, hbmThumb, 0, 600, pBits, &bi, DIB_RGB_COLORS);
        SelectObject(hdcThumb, hOld);
        DeleteObject(hbmThumb);
        DeleteDC(hdcThumb);

        char header[128];
        wsprintfA(header, "P6\n# KPaint Pro Netpbm Export\n800 600\n255\n");
        DWORD written = 0;
        WriteFile(hFile, header, lstrlenA(header), &written, NULL);

        BYTE* pPpm = (BYTE*)GlobalAlloc(GPTR, dwSize);
        if (pPpm) {
            for (int i = 0; i < 800 * 600; i++) {
                pPpm[i * 3]     = pBits[i * 3 + 2]; // R
                pPpm[i * 3 + 1] = pBits[i * 3 + 1]; // G
                pPpm[i * 3 + 2] = pBits[i * 3];     // B
            }
            WriteFile(hFile, pPpm, dwSize, &written, NULL);
            GlobalFree(pPpm);
        }
        GlobalFree(pBits);
        CloseHandle(hFile);
        MessageBoxA(hwnd, "Exported 800x600 PPM graphic successfully!", "KPaint Export", MB_OK | MB_ICONINFORMATION);
    } else {
        CloseHandle(hFile);
    }
}

void ExportPCX(HWND hwnd) {
    char file[260] = "kpaint_artwork.pcx";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = file;
    ofn.nMaxFile = 260;
    ofn.lpstrFilter = "PCX Paintbrush Image (*.pcx)\0*.pcx\0All Files\0*.*\0";
    ofn.lpstrDefExt = "pcx";
    ofn.Flags = OFN_OVERWRITEPROMPT;
    if (!GetSaveFileNameA(&ofn)) return;

    HANDLE hFile = CreateFileA(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(hwnd, "Failed to create PCX file.", "KPaint Error", MB_OK | MB_ICONERROR);
        return;
    }

    int width = 800;
    int height = 600;
    int bytesPerLine = (width % 2 == 0) ? width : width + 1;

    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = -height;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;

    DWORD dwSize = width * height * 3;
    BYTE* pBits = (BYTE*)GlobalAlloc(GPTR, dwSize);
    if (!pBits) {
        CloseHandle(hFile);
        return;
    }

    HDC hdcThumb = CreateCompatibleDC(hdcMem);
    HBITMAP hbmThumb = CreateCompatibleBitmap(hdcMem, width, height);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcThumb, hbmThumb);
    BitBlt(hdcThumb, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
    GetDIBits(hdcThumb, hbmThumb, 0, height, pBits, &bi, DIB_RGB_COLORS);
    SelectObject(hdcThumb, hOld);
    DeleteObject(hbmThumb);
    DeleteDC(hdcThumb);

    BYTE hdr[128];
    for (int k = 0; k < 128; k++) hdr[k] = 0;
    hdr[0] = 10; // Manufacturer: ZSoft .PCX
    hdr[1] = 5;  // Version: 3.0+ (24-bit support)
    hdr[2] = 1;  // Encoding: RLE
    hdr[3] = 8;  // BitsPerPixel: 8 per plane
    short xmax = (short)(width - 1);
    short ymax = (short)(height - 1);
    hdr[8] = (BYTE)(xmax & 0xFF); hdr[9] = (BYTE)((xmax >> 8) & 0xFF);
    hdr[10] = (BYTE)(ymax & 0xFF); hdr[11] = (BYTE)((ymax >> 8) & 0xFF);
    hdr[12] = 0x2C; hdr[13] = 0x01; // HDpi 300
    hdr[14] = 0x2C; hdr[15] = 0x01; // VDpi 300
    hdr[65] = 3; // NPlanes: 3 for RGB
    hdr[66] = (BYTE)(bytesPerLine & 0xFF);
    hdr[67] = (BYTE)((bytesPerLine >> 8) & 0xFF);
    hdr[68] = 1; // PaletteInfo
    hdr[70] = (BYTE)(width & 0xFF); hdr[71] = (BYTE)((width >> 8) & 0xFF);
    hdr[72] = (BYTE)(height & 0xFF); hdr[73] = (BYTE)((height >> 8) & 0xFF);

    DWORD written = 0;
    WriteFile(hFile, hdr, 128, &written, NULL);

    BYTE* planeBuf = (BYTE*)GlobalAlloc(GPTR, bytesPerLine);
    BYTE* outBuf = (BYTE*)GlobalAlloc(GPTR, bytesPerLine * 2);

    if (planeBuf && outBuf) {
        for (int y = 0; y < height; y++) {
            for (int p = 0; p < 3; p++) {
                for (int x = 0; x < width; x++) {
                    int srcIdx = (y * width + x) * 3;
                    if (p == 0) planeBuf[x] = pBits[srcIdx + 2];      // Red
                    else if (p == 1) planeBuf[x] = pBits[srcIdx + 1];  // Green
                    else planeBuf[x] = pBits[srcIdx];                  // Blue
                }
                for (int x = width; x < bytesPerLine; x++) planeBuf[x] = 0;

                int outLen = 0;
                int i = 0;
                while (i < bytesPerLine) {
                    BYTE val = planeBuf[i];
                    int run = 1;
                    while (i + run < bytesPerLine && planeBuf[i + run] == val && run < 63) {
                        run++;
                    }
                    if (run > 1 || (val & 0xC0) == 0xC0) {
                        outBuf[outLen++] = (BYTE)(0xC0 | run);
                        outBuf[outLen++] = val;
                    } else {
                        outBuf[outLen++] = val;
                    }
                    i += run;
                }
                WriteFile(hFile, outBuf, outLen, &written, NULL);
            }
        }
        GlobalFree(planeBuf);
        GlobalFree(outBuf);
        MessageBoxA(hwnd, "Exported 800x600 24-bit TrueColor PCX image successfully!", "KPaint Export", MB_OK | MB_ICONINFORMATION);
    }
    GlobalFree(pBits);
    CloseHandle(hFile);
}

void ExportXBM(HWND hwnd) {
    char file[260] = "kpaint_artwork.xbm";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = file;
    ofn.nMaxFile = 260;
    ofn.lpstrFilter = "XBM X11 Bitmap (*.xbm)\0*.xbm\0All Files\0*.*\0";
    ofn.lpstrDefExt = "xbm";
    ofn.Flags = OFN_OVERWRITEPROMPT;
    if (!GetSaveFileNameA(&ofn)) return;

    HANDLE hFile = CreateFileA(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxA(hwnd, "Failed to create XBM file.", "KPaint Error", MB_OK | MB_ICONERROR);
        return;
    }

    int width = 320;
    int height = 240;
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = width;
    bi.bmiHeader.biHeight = -height;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;

    DWORD dwSize = width * height * 3;
    BYTE* pBits = (BYTE*)GlobalAlloc(GPTR, dwSize);
    if (!pBits) {
        CloseHandle(hFile);
        return;
    }

    HDC hdcThumb = CreateCompatibleDC(hdcMem);
    HBITMAP hbmThumb = CreateCompatibleBitmap(hdcMem, width, height);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcThumb, hbmThumb);
    StretchBlt(hdcThumb, 0, 0, width, height, hdcMem, 0, 0, 640, 480, SRCCOPY);
    GetDIBits(hdcThumb, hbmThumb, 0, height, pBits, &bi, DIB_RGB_COLORS);
    SelectObject(hdcThumb, hOld);
    DeleteObject(hbmThumb);
    DeleteDC(hdcThumb);

    char header[256];
    wsprintfA(header,
        "#define kpaint_width %d\n"
        "#define kpaint_height %d\n"
        "static unsigned char kpaint_bits[] = {\n  ", width, height);
    DWORD written = 0;
    WriteFile(hFile, header, lstrlenA(header), &written, NULL);

    int bytesPerRow = (width + 7) / 8;
    int totalBytes = bytesPerRow * height;
    char byteStr[16];
    int count = 0;

    for (int y = 0; y < height; y++) {
        for (int b = 0; b < bytesPerRow; b++) {
            BYTE val = 0;
            for (int bit = 0; bit < 8; bit++) {
                int x = b * 8 + bit;
                if (x < width) {
                    int idx = (y * width + x) * 3;
                    int luma = (pBits[idx + 2] * 77 + pBits[idx + 1] * 150 + pBits[idx] * 29) >> 8;
                    if (luma < 128) {
                        val |= (BYTE)(1 << bit);
                    }
                }
            }
            count++;
            wsprintfA(byteStr, "0x%02X%s%s", (unsigned int)val,
                (count < totalBytes ? ", " : "\n};\n"),
                (count % 12 == 0 && count < totalBytes ? "\n  " : ""));
            WriteFile(hFile, byteStr, lstrlenA(byteStr), &written, NULL);
        }
    }

    GlobalFree(pBits);
    CloseHandle(hFile);
    MessageBoxA(hwnd, "Exported 320x240 XBM bitmap successfully!", "KPaint Export", MB_OK | MB_ICONINFORMATION);
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

void QuicksaveState(HWND hwnd) {
    if (!hdcMem || !hbmCanvas) return;
    if (!SaveBitmap(QUICKSAVE_BMP, hbmCanvas)) {
        MessageBoxA(hwnd, "Failed to save quicksave canvas snapshot.", "KPaint Error", MB_OK | MB_ICONERROR);
        return;
    }
    KPaintQuickState st;
    st.magic = 0x4B504153;
    st.curColor = curColor;
    st.curSize = curSize;
    st.brushShape = brushShape;
    st.currentTool = currentTool;
    st.mirrorMode = mirrorMode;

    HANDLE hFile = CreateFileA(QUICKSAVE_DAT, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD dwWritten = 0;
        WriteFile(hFile, &st, sizeof(st), &dwWritten, NULL);
        CloseHandle(hFile);
    }

    HANDLE hTut = CreateFileA("kpaint_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTut != INVALID_HANDLE_VALUE) {
        DWORD dw = 0;
        WriteFile(hTut, "1", 1, &dw, NULL);
        CloseHandle(hTut);
    }

    SetWindowTextA(hwnd, "KPaint Pro - Quicksave Snapshot Saved! [F5] | Press F9 to Restore");
    MessageBeep(MB_OK);
}

void QuickloadState(HWND hwnd) {
    DWORD attrBmp = GetFileAttributesA(QUICKSAVE_BMP);
    if (attrBmp == INVALID_FILE_ATTRIBUTES) {
        MessageBoxA(hwnd, "No quicksave snapshot found. Press F5 to Quicksave.", "KPaint Pro", MB_OK | MB_ICONINFORMATION);
        return;
    }

    LoadBitmapFile(hwnd, QUICKSAVE_BMP);

    DWORD attrDat = GetFileAttributesA(QUICKSAVE_DAT);
    if (attrDat != INVALID_FILE_ATTRIBUTES) {
        HANDLE hFile = CreateFileA(QUICKSAVE_DAT, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            KPaintQuickState st = {0};
            DWORD dwRead = 0;
            if (ReadFile(hFile, &st, sizeof(st), &dwRead, NULL) && dwRead == sizeof(st) && st.magic == 0x4B504153) {
                curColor = st.curColor;
                curSize = st.curSize;
                brushShape = st.brushShape;
                currentTool = st.currentTool;
                mirrorMode = st.mirrorMode;
                UpdatePen();
                if (hBtnMirror) SetWindowTextA(hBtnMirror, mirrorMode ? "Mirror ON" : "Mirror [M]");
            }
            CloseHandle(hFile);
        }
    }

    HANDLE hTut = CreateFileA("kpaint_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTut != INVALID_HANDLE_VALUE) {
        DWORD dw = 0;
        WriteFile(hTut, "1", 1, &dw, NULL);
        CloseHandle(hTut);
    }

    UpdateTitleStatus(hwnd);
    InvalidateRect(hwnd, NULL, FALSE);
    MessageBeep(MB_OK);
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

static int IntSin16(int angleIdx) {
    static const int sinTab[16] = {0, 38, 71, 92, 100, 92, 71, 38, 0, -38, -71, -92, -100, -92, -71, -38};
    int idx = (angleIdx % 16 + 16) % 16;
    return sinTab[idx];
}

void GenerateSignalArtwork(HWND hwnd) {
    if (!hdcMem) return;
    PushUndo();

    // Dark subterranean backdrop
    RECT rBg = {0, 0, 2000, 2000};
    HBRUSH hBg = CreateSolidBrush(RGB(6, 10, 20));
    FillRect(hdcMem, &rBg, hBg);
    DeleteObject(hBg);

    // Cyan frequency grid lines
    HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(18, 38, 58));
    HPEN hOldP = (HPEN)SelectObject(hdcMem, hGridPen);
    for (int y = 60; y < 1200; y += 30) {
        MoveToEx(hdcMem, 40, y, NULL);
        LineTo(hdcMem, 1600, y);
    }
    for (int x = 80; x < 1600; x += 60) {
        MoveToEx(hdcMem, x, 60, NULL);
        LineTo(hdcMem, x, 1200);
    }
    SelectObject(hdcMem, hOldP);
    DeleteObject(hGridPen);

    // 1999Hz carrier wave (Bright Cyan)
    HPEN hWavePen = CreatePen(PS_SOLID, 2, RGB(56, 189, 248));
    SelectObject(hdcMem, hWavePen);
    for (int x = 40; x < 1600; x++) {
        int y = 280 + (IntSin16(x / 4) * 32) / 100 + (IntSin16(x / 2) * 12) / 100;
        if (x == 40) MoveToEx(hdcMem, x, y, NULL);
        else LineTo(hdcMem, x, y);
    }
    SelectObject(hdcMem, hOldP);
    DeleteObject(hWavePen);

    // Modulated data burst (Amber 10.19.99.4 beacon)
    HPEN hBurstPen = CreatePen(PS_SOLID, 2, RGB(245, 158, 11));
    SelectObject(hdcMem, hBurstPen);
    for (int x = 320; x < 680; x++) {
        int progress = (x - 320);
        int envelope = (progress * (360 - progress)) / 324;
        int wave = (IntSin16(progress / 3) * envelope) / 100;
        int y = 460 + wave;
        if (x == 320) MoveToEx(hdcMem, x, y, NULL);
        else LineTo(hdcMem, x, y);
    }
    SelectObject(hdcMem, hOldP);
    DeleteObject(hBurstPen);

    // Telemetry text
    HFONT hTitleFont = CreateFontA(-20, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 5, DEFAULT_PITCH, "Segoe UI");
    HFONT hOldF = (HFONT)SelectObject(hdcMem, hTitleFont);
    SetTextColor(hdcMem, RGB(56, 189, 248));
    SetBkMode(hdcMem, TRANSPARENT);
    TextOutA(hdcMem, 42, 32, "RF SPECTRUM DIAGNOSTIC // NODE-7F MONITOR", 41);

    HFONT hSubFont = CreateFontA(-13, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 5, DEFAULT_PITCH, "Segoe UI");
    SelectObject(hdcMem, hSubFont);
    SetTextColor(hdcMem, RGB(148, 163, 184));
    TextOutA(hdcMem, 42, 560, "SUB-CARRIER: 1999.04 Hz [LOCKED]   HEX OFFSET: 0x00402000   GATEWAY: 10.19.99.4/classified", 88);
    TextOutA(hdcMem, 42, 582, "DATA BURST DETECTED [320px..680px] - ECHO SUBNET SYNCHRONIZED", 61);

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
            hBtnShapeToggle = CreateWindowA("BUTTON", "Shape", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 234, 58, 22, hwnd, (HMENU)204, NULL, NULL);
            hBtnMirror = CreateWindowA("BUTTON", "Mirror [M]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 234, 58, 22, hwnd, (HMENU)205, NULL, NULL);

            // Filters & Transforms
            hBtnInvert = CreateWindowA("BUTTON", "Invert [V]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 261, 58, 22, hwnd, (HMENU)501, NULL, NULL);
            hBtnGray = CreateWindowA("BUTTON", "Gray [Y]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 261, 58, 22, hwnd, (HMENU)502, NULL, NULL);
            hBtnBright = CreateWindowA("BUTTON", "+Bright [+]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 286, 58, 22, hwnd, (HMENU)503, NULL, NULL);
            hBtnDark = CreateWindowA("BUTTON", "-Bright [-]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 286, 58, 22, hwnd, (HMENU)509, NULL, NULL);
            hBtnFlipH = CreateWindowA("BUTTON", "Flip H", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 311, 58, 22, hwnd, (HMENU)504, NULL, NULL);
            hBtnFlipV = CreateWindowA("BUTTON", "Flip V", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 311, 58, 22, hwnd, (HMENU)510, NULL, NULL);
            hBtnRotate90 = CreateWindowA("BUTTON", "Rot CW", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 336, 58, 22, hwnd, (HMENU)505, NULL, NULL);
            hBtnRotateCCW = CreateWindowA("BUTTON", "Rot CCW", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 336, 58, 22, hwnd, (HMENU)511, NULL, NULL);

            // Convolve & DSP Filters
            hBtnEdge = CreateWindowA("BUTTON", "Edge", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 363, 38, 22, hwnd, (HMENU)506, NULL, NULL);
            hBtnSharpen = CreateWindowA("BUTTON", "Sharp", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 46, 363, 38, 22, hwnd, (HMENU)507, NULL, NULL);
            hBtnEmboss = CreateWindowA("BUTTON", "Emboss", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 87, 363, 39, 22, hwnd, (HMENU)508, NULL, NULL);
            hBtnDither = CreateWindowA("BUTTON", "Dither", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 388, 58, 22, hwnd, (HMENU)512, NULL, NULL);
            hBtnScanlines = CreateWindowA("BUTTON", "CRT Lines", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 388, 58, 22, hwnd, (HMENU)513, NULL, NULL);
            hBtnSepia = CreateWindowA("BUTTON", "Sepia [P]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 413, 58, 22, hwnd, (HMENU)514, NULL, NULL);
            hBtnCGA = CreateWindowA("BUTTON", "CGA 4-Col", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 413, 58, 22, hwnd, (HMENU)515, NULL, NULL);
            hBtnExportC = CreateWindowA("BUTTON", "Export C Header (.H)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 438, 121, 22, hwnd, (HMENU)516, NULL, NULL);

            // History & Actions
            hBtnUndo = CreateWindowA("BUTTON", "Undo ^Z", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 463, 58, 22, hwnd, (HMENU)601, NULL, NULL);
            hBtnRedo = CreateWindowA("BUTTON", "Redo ^Y", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 463, 58, 22, hwnd, (HMENU)602, NULL, NULL);
            hBtnOpen = CreateWindowA("BUTTON", "Open ^O", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 488, 58, 22, hwnd, (HMENU)303, NULL, NULL);
            hBtnSave = CreateWindowA("BUTTON", "Save ^S", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 488, 58, 22, hwnd, (HMENU)302, NULL, NULL);
            hBtnQuickSave = CreateWindowA("BUTTON", "QSave F5", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 513, 58, 22, hwnd, (HMENU)305, NULL, NULL);
            hBtnQuickLoad = CreateWindowA("BUTTON", "QLoad F9", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 513, 58, 22, hwnd, (HMENU)306, NULL, NULL);
            hBtnClear = CreateWindowA("BUTTON", "Clear Canvas", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 538, 121, 22, hwnd, (HMENU)301, NULL, NULL);
            hBtnHelp = CreateWindowA("BUTTON", "Help (F1)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 563, 121, 22, hwnd, (HMENU)701, NULL, NULL);
            hBtnDemoArt = CreateWindowA("BUTTON", "Demo Art (D)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 588, 121, 22, hwnd, (HMENU)304, NULL, NULL);
            hBtnGameBoy = CreateWindowA("BUTTON", "GBoy Dither", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 613, 58, 22, hwnd, (HMENU)517, NULL, NULL);
            hBtnPixelize = CreateWindowA("BUTTON", "Mosaic", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 613, 58, 22, hwnd, (HMENU)518, NULL, NULL);
            hBtnSolarize = CreateWindowA("BUTTON", "Solarize", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 638, 58, 22, hwnd, (HMENU)519, NULL, NULL);
            hBtnExportPPM = CreateWindowA("BUTTON", "Export PPM", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 638, 58, 22, hwnd, (HMENU)520, NULL, NULL);
            hBtnSwapColor = CreateWindowA("BUTTON", "Swap [X]", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 663, 58, 22, hwnd, (HMENU)108, NULL, NULL);
            hBtnSignalArt = CreateWindowA("BUTTON", "Signal '99", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 663, 58, 22, hwnd, (HMENU)307, NULL, NULL);
            hBtnExportPCX = CreateWindowA("BUTTON", "PCX (24b)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 5, 688, 58, 22, hwnd, (HMENU)521, NULL, NULL);
            hBtnExportXBM = CreateWindowA("BUTTON", "XBM (1b)", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 68, 688, 58, 22, hwnd, (HMENU)522, NULL, NULL);

            HWND controls[] = {
                hBtnBlack, hBtnRed, hBtnGreen, hBtnBlue, hBtnYellow, hBtnPurple, hBtnCustomColor,
                hBtnFreehand, hBtnLine, hBtnRect, hBtnEllipse, hBtnSpray, hBtnEraser, hBtnFill, hBtnPipette,
                hBtnSizeSmall, hBtnSizeMed, hBtnSizeLarge, hBtnShapeToggle, hBtnMirror,
                hBtnInvert, hBtnGray, hBtnBright, hBtnDark, hBtnFlipH, hBtnFlipV, hBtnRotate90, hBtnRotateCCW,
                hBtnEdge, hBtnSharpen, hBtnEmboss, hBtnDither, hBtnScanlines, hBtnSepia, hBtnCGA, hBtnExportC,
                hBtnUndo, hBtnRedo, hBtnOpen, hBtnSave, hBtnQuickSave, hBtnQuickLoad, hBtnClear, hBtnHelp, hBtnDemoArt,
                hBtnGameBoy, hBtnPixelize, hBtnSolarize, hBtnExportPPM, hBtnSwapColor, hBtnSignalArt,
                hBtnExportPCX, hBtnExportXBM
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
            if (id >= 101 && id <= 106) {
                COLORREF c = RGB(0,0,0);
                if (id == 101) c = RGB(0,0,0);
                else if (id == 102) c = RGB(231,76,60);
                else if (id == 103) c = RGB(46,204,113);
                else if (id == 104) c = RGB(52,152,219);
                else if (id == 105) c = RGB(241,196,15);
                else if (id == 106) c = RGB(155,89,182);
                if (GetKeyState(VK_SHIFT) & 0x8000) {
                    curColorSec = c;
                } else {
                    curColor = c;
                }
            }
            if (id == 107) {
                CHOOSECOLORA cc = {0};
                static COLORREF custColors[16] = {0};
                cc.lStructSize = sizeof(cc);
                cc.hwndOwner = hwnd;
                cc.lpCustColors = custColors;
                cc.rgbResult = (GetKeyState(VK_SHIFT) & 0x8000) ? curColorSec : curColor;
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                if (ChooseColorA(&cc)) {
                    if (GetKeyState(VK_SHIFT) & 0x8000) curColorSec = cc.rgbResult;
                    else curColor = cc.rgbResult;
                }
            }
            if (id == 108) {
                COLORREF tmp = curColor;
                curColor = curColorSec;
                curColorSec = tmp;
            }

            if (id == 201) curSize = 2;
            if (id == 202) curSize = 6;
            if (id == 203) curSize = 14;
            if (id == 204) {
                brushShape = 1 - brushShape;
                SetWindowTextA(hBtnShapeToggle, brushShape ? "Square" : "Round");
            }
            if (id == 205) {
                mirrorMode = 1 - mirrorMode;
                SetWindowTextA(hBtnMirror, mirrorMode ? "Mirror ON" : "Mirror [M]");
            }
            
            if (id >= 101 && id <= 205) {
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
            if (id == 305) { QuicksaveState(hwnd); }
            if (id == 306) { QuickloadState(hwnd); }
            
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
            if (id == 512) { FilterDither1Bit(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 513) { FilterScanlines(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 514) { FilterSepia(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 515) { FilterCGADither(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 516) { ExportCHeader(hwnd); }
            if (id == 517) { FilterGameBoyDither(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 518) { FilterPixelize(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 519) { FilterSolarize(); InvalidateRect(hwnd, NULL, FALSE); }
            if (id == 520) { ExportPPM(hwnd); }
            if (id == 521) { ExportPCX(hwnd); }
            if (id == 522) { ExportXBM(hwnd); }

            if (id == 304) { GenerateDemoArtwork(hwnd); }
            if (id == 307) { GenerateSignalArtwork(hwnd); }
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
                    "  [I]  Eyedropper / Color Pick\n"
                    "  [X]  Swap FG & BG Colors (Right-click draws with BG)\n\n"
                    "Brush & Shapes:\n"
                    "  [1]  Small (2px)    [2] Medium (6px)    [3] Large (14px)\n"
                    "  [ [ ] / [ ] ]  Decrease / Increase Brush Size\n"
                    "  [M]  Mirror Drawing Mode\n"
                    "  Shape Toggle: Round / Square\n\n"
                    "Image Filters & Transforms:\n"
                    "  [V]  Invert Colors    [Y] Grayscale\n"
                    "  [P]  Sepia Tone\n"
                    "  [+]  +Brightness     [-] -Brightness\n"
                    "  GBoy Dither, Mosaic (Pixelize), Solarize, CRT Lines, CGA 4-Col, Edge, Sharp, Emboss\n"
                    "  Flip H/V, Rotate CW/CCW\n\n"
                    "State & File Actions:\n"
                    "  [F5] Quicksave Snapshot\n"
                    "  [F9] Quickload Snapshot\n"
                    "  [D]  Generate Demo Artwork (Mountain Sunset)\n"
                    "  [0]  Signal '99 Spectrogram Art\n"
                    "  Export C Header (.H) sprite array for Win32/demoscene\n"
                    "  Export PPM (.PPM) Portable Pixmap image\n"
                    "  Export PCX (.PCX) 24-bit TrueColor RLE Paintbrush image\n"
                    "  Export XBM (.XBM) X11 monochrome C bitmap\n"
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
                    if (GetKeyState(VK_SHIFT) & 0x8000) {
                        PerformRedo();
                    } else {
                        PerformUndo();
                    }
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
            } else if (wParam == VK_F5) {
                QuicksaveState(hwnd);
            } else if (wParam == VK_F9) {
                QuickloadState(hwnd);
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
            } else if (wParam == 'M' || wParam == 'm') {
                mirrorMode = 1 - mirrorMode;
                SetWindowTextA(hBtnMirror, mirrorMode ? "Mirror ON" : "Mirror [M]");
                UpdateTitleStatus(hwnd);
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
            } else if (wParam == 'P' || wParam == 'p') {
                FilterSepia(); InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == VK_OEM_4 /* [ */) {
                curSize = max(1, curSize - 2);
                UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == VK_OEM_6 /* ] */) {
                curSize = min(80, curSize + 2);
                UpdatePen(); UpdateTitleStatus(hwnd);
            } else if (wParam == 'X' || wParam == 'x') {
                COLORREF tmp = curColor;
                curColor = curColorSec;
                curColorSec = tmp;
                UpdatePen();
                UpdateTitleStatus(hwnd);
            } else if (wParam == '0') {
                GenerateSignalArtwork(hwnd);
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
                    if (mirrorMode) {
                        int mx = 1999 - x;
                        MoveToEx(hdc, mx + 130 - scrollX, y - scrollY, NULL);
                        LineTo(hdc, mx + 130 - scrollX, y - scrollY);
                    }
                    ReleaseDC(hwnd, hdc);
                    MoveToEx(hdcMem, x, y, NULL);
                    LineTo(hdcMem, x, y);
                    if (mirrorMode) {
                        int mx = 1999 - x;
                        MoveToEx(hdcMem, mx, y, NULL);
                        LineTo(hdcMem, mx, y);
                    }
                } else if (currentTool == 4) { // Spray
                    for (int i = 0; i < 15; i++) {
                        int rx = (x + (i * 7) % curSize) - curSize / 2;
                        int ry = (y + (i * 13) % curSize) - curSize / 2;
                        SetPixel(hdcMem, rx, ry, curColor);
                        if (mirrorMode) {
                            int mx = 1999 - x;
                            int mrx = (mx + (i * 7) % curSize) - curSize / 2;
                            SetPixel(hdcMem, mrx, ry, curColor);
                        }
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
        case WM_RBUTTONDOWN: {
            int x = (short)LOWORD(lParam) - 130 + scrollX;
            int y = (short)HIWORD(lParam) + scrollY;
            if ((short)LOWORD(lParam) >= 130) {
                if (currentTool == 7) { // Pipette: pick secondary color
                    curColorSec = GetPixel(hdcMem, x, y);
                    currentTool = 0;
                    UpdateTitleStatus(hwnd);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (currentTool == 6) { // Bucket fill with secondary color
                    PushUndo();
                    HBRUSH hSecBrush = CreateSolidBrush(curColorSec);
                    SelectObject(hdcMem, hSecBrush);
                    COLORREF target = GetPixel(hdcMem, x, y);
                    if (target != curColorSec) {
                        ExtFloodFill(hdcMem, x, y, target, FLOODFILLSURFACE);
                    }
                    SelectObject(hdcMem, hBrush);
                    DeleteObject(hSecBrush);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                PushUndo();
                isPainting = 2; // 2 = painting with secondary color
                startX = x; startY = y;
                lastX = x; lastY = y;
                HPEN hSecPen = (brushShape == 1) ?
                    ExtCreatePen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_SQUARE | PS_JOIN_MITER, curSize, &(LOGBRUSH){BS_SOLID, curColorSec, 0}, 0, NULL) :
                    CreatePen(PS_SOLID, curSize, curColorSec);
                if (currentTool == 0 || currentTool == 5) {
                    HDC hdc = GetDC(hwnd);
                    SelectObject(hdc, hSecPen);
                    MoveToEx(hdc, x + 130 - scrollX, y - scrollY, NULL);
                    LineTo(hdc, x + 130 - scrollX, y - scrollY);
                    if (mirrorMode) {
                        int mx = 1999 - x;
                        MoveToEx(hdc, mx + 130 - scrollX, y - scrollY, NULL);
                        LineTo(hdc, mx + 130 - scrollX, y - scrollY);
                    }
                    ReleaseDC(hwnd, hdc);
                    SelectObject(hdcMem, hSecPen);
                    MoveToEx(hdcMem, x, y, NULL);
                    LineTo(hdcMem, x, y);
                    if (mirrorMode) {
                        int mx = 1999 - x;
                        MoveToEx(hdcMem, mx, y, NULL);
                        LineTo(hdcMem, mx, y);
                    }
                    SelectObject(hdcMem, hPen);
                } else if (currentTool == 4) { // Spray
                    for (int i = 0; i < 15; i++) {
                        int rx = (x + (i * 7) % curSize) - curSize / 2;
                        int ry = (y + (i * 13) % curSize) - curSize / 2;
                        SetPixel(hdcMem, rx, ry, curColorSec);
                        if (mirrorMode) {
                            int mx = 1999 - x;
                            int mrx = (mx + (i * 7) % curSize) - curSize / 2;
                            SetPixel(hdcMem, mrx, ry, curColorSec);
                        }
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                } else {
                    HDC hdc = GetDC(hwnd);
                    SetROP2(hdc, R2_NOTXORPEN);
                    SelectObject(hdc, hSecPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    if (currentTool == 1) { MoveToEx(hdc, startX + 130 - scrollX, startY - scrollY, NULL); LineTo(hdc, x + 130 - scrollX, y - scrollY); }
                    else if (currentTool == 2) { Rectangle(hdc, startX + 130 - scrollX, startY - scrollY, x + 130 - scrollX, y - scrollY); }
                    else if (currentTool == 3) { Ellipse(hdc, startX + 130 - scrollX, startY - scrollY, x + 130 - scrollX, y - scrollY); }
                    ReleaseDC(hwnd, hdc);
                }
                DeleteObject(hSecPen);
            }
            break;
        }
        case WM_MOUSEMOVE: {
            int x = (short)LOWORD(lParam) - 130 + scrollX;
            int y = (short)HIWORD(lParam) + scrollY;
            if (isPainting && (short)LOWORD(lParam) >= 130) {
                COLORREF drawColor = (isPainting == 2) ? curColorSec : ((currentTool == 5) ? RGB(255,255,255) : curColor);
                HPEN hMovePen = (brushShape == 1) ?
                    ExtCreatePen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_SQUARE | PS_JOIN_MITER, curSize, &(LOGBRUSH){BS_SOLID, drawColor, 0}, 0, NULL) :
                    CreatePen(PS_SOLID, curSize, drawColor);
                HDC hdc = GetDC(hwnd);
                if (currentTool == 0 || currentTool == 5) {
                    SelectObject(hdc, hMovePen);
                    MoveToEx(hdc, lastX + 130 - scrollX, lastY - scrollY, NULL);
                    LineTo(hdc, x + 130 - scrollX, y - scrollY);
                    SelectObject(hdcMem, hMovePen);
                    MoveToEx(hdcMem, lastX, lastY, NULL);
                    LineTo(hdcMem, x, y);
                    if (mirrorMode) {
                        int m_lastX = 1999 - lastX;
                        int m_x = 1999 - x;
                        MoveToEx(hdc, m_lastX + 130 - scrollX, lastY - scrollY, NULL);
                        LineTo(hdc, m_x + 130 - scrollX, y - scrollY);
                        MoveToEx(hdcMem, m_lastX, lastY, NULL);
                        LineTo(hdcMem, m_x, y);
                    }
                    SelectObject(hdcMem, hPen);
                } else if (currentTool == 4) { // Spray
                    for (int i = 0; i < 15; i++) {
                        int rx = (x + (i * 7) % curSize) - curSize / 2;
                        int ry = (y + (i * 13) % curSize) - curSize / 2;
                        SetPixel(hdcMem, rx, ry, drawColor);
                        if (mirrorMode) {
                            int mx = 1999 - x;
                            int mrx = (mx + (i * 7) % curSize) - curSize / 2;
                            SetPixel(hdcMem, mrx, ry, drawColor);
                        }
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                } else {
                    SetROP2(hdc, R2_NOTXORPEN);
                    SelectObject(hdc, hMovePen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    if (currentTool == 1) { MoveToEx(hdc, startX + 130 - scrollX, startY - scrollY, NULL); LineTo(hdc, lastX + 130 - scrollX, lastY - scrollY); }
                    else if (currentTool == 2) { Rectangle(hdc, startX + 130 - scrollX, startY - scrollY, lastX + 130 - scrollX, lastY - scrollY); }
                    else if (currentTool == 3) { Ellipse(hdc, startX + 130 - scrollX, startY - scrollY, lastX + 130 - scrollX, lastY - scrollY); }
                    
                    if (currentTool == 1) { MoveToEx(hdc, startX + 130 - scrollX, startY - scrollY, NULL); LineTo(hdc, x + 130 - scrollX, y - scrollY); }
                    else if (currentTool == 2) { Rectangle(hdc, startX + 130 - scrollX, startY - scrollY, x + 130 - scrollX, y - scrollY); }
                    else if (currentTool == 3) { Ellipse(hdc, startX + 130 - scrollX, startY - scrollY, x + 130 - scrollX, y - scrollY); }
                }
                ReleaseDC(hwnd, hdc);
                DeleteObject(hMovePen);
                lastX = x;
                lastY = y;
            }
            break;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP: {
            if (isPainting) {
                int x = (short)LOWORD(lParam) - 130 + scrollX;
                int y = (short)HIWORD(lParam) + scrollY;
                if (currentTool != 0 && currentTool != 5 && currentTool != 4 && currentTool != 6 && currentTool != 7) {
                    COLORREF drawColor = (isPainting == 2) ? curColorSec : curColor;
                    HPEN hFinalPen = (brushShape == 1) ?
                        ExtCreatePen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_SQUARE | PS_JOIN_MITER, curSize, &(LOGBRUSH){BS_SOLID, drawColor, 0}, 0, NULL) :
                        CreatePen(PS_SOLID, curSize, drawColor);

                    HDC hdc = GetDC(hwnd);
                    SetROP2(hdc, R2_NOTXORPEN);
                    SelectObject(hdc, hFinalPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    if (currentTool == 1) { MoveToEx(hdc, startX + 130 - scrollX, startY - scrollY, NULL); LineTo(hdc, lastX + 130 - scrollX, lastY - scrollY); }
                    else if (currentTool == 2) { Rectangle(hdc, startX + 130 - scrollX, startY - scrollY, lastX + 130 - scrollX, lastY - scrollY); }
                    else if (currentTool == 3) { Ellipse(hdc, startX + 130 - scrollX, startY - scrollY, lastX + 130 - scrollX, lastY - scrollY); }
                    ReleaseDC(hwnd, hdc);

                    SelectObject(hdcMem, hFinalPen);
                    SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
                    if (currentTool == 1) { MoveToEx(hdcMem, startX, startY, NULL); LineTo(hdcMem, x, y); }
                    else if (currentTool == 2) { Rectangle(hdcMem, startX, startY, x, y); }
                    else if (currentTool == 3) { Ellipse(hdcMem, startX, startY, x, y); }

                    if (mirrorMode) {
                        int mStartX = 1999 - startX;
                        int mX = 1999 - x;
                        if (currentTool == 1) { MoveToEx(hdcMem, mStartX, startY, NULL); LineTo(hdcMem, mX, y); }
                        else if (currentTool == 2) { Rectangle(hdcMem, mX, startY, mStartX, y); }
                        else if (currentTool == 3) { Ellipse(hdcMem, mX, startY, mStartX, y); }
                    }
                    SelectObject(hdcMem, hPen);
                    SelectObject(hdcMem, hBrush);
                    DeleteObject(hFinalPen);
                    
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
    RECT wr = {0, 0, 1140, 780};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN, FALSE);
    HWND hwnd = CreateWindowExA(0, "KPaintClass", "KPaint Pro - Press F1 or H for Help", WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    DWORD attrTut = GetFileAttributesA("kpaint_tutorial.dat");
    DWORD attrQuick = GetFileAttributesA(QUICKSAVE_DAT);
    if (attrTut == INVALID_FILE_ATTRIBUTES && attrQuick == INVALID_FILE_ATTRIBUTES) {
        HANDLE hTut = CreateFileA("kpaint_tutorial.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hTut != INVALID_HANDLE_VALUE) {
            DWORD dw = 0;
            WriteFile(hTut, "1", 1, &dw, NULL);
            CloseHandle(hTut);
        }
        MessageBoxA(hwnd,
            "Welcome to KPaint Pro!\n\n"
            "Quick Tips:\n"
            "- [B] Brush, [E] Eraser, [L] Line, [R] Rect, [C] Circle, [G] Fill\n"
            "- [F5] Quicksave Snapshot, [F9] Quickload Snapshot\n"
            "- [D] Load Demo Artwork, [F1] / [H] Full Help Guide\n"
            "- Drag & drop any BMP onto the canvas to open!",
            "KPaint Pro - Quick Guide", MB_OK | MB_ICONINFORMATION);
    }

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            WPARAM wParam = msg.wParam;
            BOOL bCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            if (bCtrl) {
                if (wParam == 'Z' || wParam == 'z') {
                    if (GetKeyState(VK_SHIFT) & 0x8000) {
                        PerformRedo();
                    } else {
                        PerformUndo();
                    }
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
                } else if (wParam == VK_F5) {
                    QuicksaveState(hwnd);
                    continue;
                } else if (wParam == VK_F9) {
                    QuickloadState(hwnd);
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
                } else if (wParam == 'X' || wParam == 'x') {
                    COLORREF tmp = curColor;
                    curColor = curColorSec;
                    curColorSec = tmp;
                    UpdatePen();
                    UpdateTitleStatus(hwnd);
                    continue;
                } else if (wParam == '0') {
                    GenerateSignalArtwork(hwnd);
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
