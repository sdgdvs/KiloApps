#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <process.h>

#define W 800
#define H 600

int _fltused = 0;

HBITMAP hBitmap = NULL;
DWORD* pixels = NULL;
int bmpW = 0, bmpH = 0;

double minRe = -2.0, maxRe = 1.0;
double minIm = -1.2, maxIm = 1.2;
unsigned int max_iter = 100;
int theme = 0; // 0: Fire, 1: Ocean, 2: Cyberpunk, 3: BW, 4: Custom
int isJulia = 0;
double juliaCRe = 0.0, juliaCIm = 0.0;

unsigned char customColor1[3] = {255, 0, 0};
unsigned char customColor2[3] = {0, 0, 255};

#define MAX_HISTORY 256
typedef struct {
    double minRe, maxRe, minIm, maxIm;
    unsigned int max_iter;
    int isJulia;
    double juliaCRe, juliaCIm;
    int theme;
    unsigned char cc1[3], cc2[3];
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
    history[history_idx].cc1[0] = customColor1[0];
    history[history_idx].cc1[1] = customColor1[1];
    history[history_idx].cc1[2] = customColor1[2];
    history[history_idx].cc2[0] = customColor2[0];
    history[history_idx].cc2[1] = customColor2[1];
    history[history_idx].cc2[2] = customColor2[2];
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
        customColor1[0] = history[idx].cc1[0];
        customColor1[1] = history[idx].cc1[1];
        customColor1[2] = history[idx].cc1[2];
        customColor2[0] = history[idx].cc2[0];
        customColor2[1] = history[idx].cc2[1];
        customColor2[2] = history[idx].cc2[2];
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
        *r = (unsigned char)((n * 5) % 256);
        *g = (unsigned char)((n * 2) % 128);
        *b = (unsigned char)(255 - ((n * 3) % 256));
    } else if (t == 3) { // BW
        unsigned char v = ((n % 20) > 10) ? 255 : 0;
        *r = v; *g = v; *b = v;
    } else if (t == 4) { // Custom
        double f = (double)n / iter;
        *r = (unsigned char)(customColor1[0] + f * (customColor2[0] - customColor1[0]));
        *g = (unsigned char)(customColor1[1] + f * (customColor2[1] - customColor1[1]));
        *b = (unsigned char)(customColor1[2] + f * (customColor2[2] - customColor1[2]));
    }
}

typedef struct {
    DWORD* buffer;
    int width;
    int height;
    int startY;
    int endY;
    double mRe, mxRe, mIm, mxIm;
    unsigned int mIter;
    int isJ;
    double jCRe, jCIm;
    int th;
} RenderTask;

DWORD WINAPI RenderThreadProc(LPVOID lpParam) {
    RenderTask* task = (RenderTask*)lpParam;
    double re_factor = (task->mxRe - task->mRe) / (task->width - 1);
    double im_factor = (task->mxIm - task->mIm) / (task->height - 1);
    
    for (int y = task->startY; y < task->endY; ++y) {
        double c_im_view = task->mxIm - y * im_factor;
        for (int x = 0; x < task->width; ++x) {
            double c_re_view = task->mRe + x * re_factor;
            double c_re = task->isJ ? task->jCRe : c_re_view;
            double c_im = task->isJ ? task->jCIm : c_im_view;
            double Z_re = c_re_view, Z_im = c_im_view;
            int isInside = 1;
            unsigned int n = 0;
            for (n = 0; n < task->mIter; ++n) {
                double Z_re2 = Z_re * Z_re, Z_im2 = Z_im * Z_im;
                if (Z_re2 + Z_im2 > 4) {
                    isInside = 0;
                    break;
                }
                Z_im = 2 * Z_re * Z_im + c_im;
                Z_re = Z_re2 - Z_im2 + c_re;
            }
            if (isInside) {
                task->buffer[y * task->width + x] = 0; // Black
            } else {
                unsigned char r, g, b;
                GetColors(n, task->mIter, task->th, &r, &g, &b);
                task->buffer[y * task->width + x] = (r << 16) | (g << 8) | b;
            }
        }
    }
    return 0;
}

void RenderMandelbrotToBuffer(DWORD* buffer, int width, int height) {
    if (!buffer || width <= 1 || height <= 1) return;
    
    int numThreads = 8;
    HANDLE threads[8];
    RenderTask tasks[8];
    
    int chunkH = height / numThreads;
    for (int i = 0; i < numThreads; i++) {
        tasks[i].buffer = buffer;
        tasks[i].width = width;
        tasks[i].height = height;
        tasks[i].startY = i * chunkH;
        tasks[i].endY = (i == numThreads - 1) ? height : (i + 1) * chunkH;
        tasks[i].mRe = minRe; tasks[i].mxRe = maxRe;
        tasks[i].mIm = minIm; tasks[i].mxIm = maxIm;
        tasks[i].mIter = max_iter;
        tasks[i].isJ = isJulia;
        tasks[i].jCRe = juliaCRe; tasks[i].jCIm = juliaCIm;
        tasks[i].th = theme;
        
        threads[i] = CreateThread(NULL, 0, RenderThreadProc, &tasks[i], 0, NULL);
    }
    
    WaitForMultipleObjects(numThreads, threads, TRUE, INFINITE);
    for (int i = 0; i < numThreads; i++) {
        CloseHandle(threads[i]);
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
    
    RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
}

void Zoom(double factor, int mouseX, int mouseY) {
    if (bmpW <= 1 || bmpH <= 1) return;
    
    double re_factor = (maxRe - minRe) / (bmpW - 1);
    double im_factor = (maxIm - minIm) / (bmpH - 1);
    
    double centerRe = minRe + mouseX * re_factor;
    double centerIm = maxIm - mouseY * im_factor;
    
    double newWRe = (maxRe - minRe) * factor;
    double newWIm = (maxIm - minIm) * factor;
    
    if (newWRe < 1e-13 || newWRe > 10.0) return;

    
    minRe = centerRe - ((double)mouseX / bmpW) * newWRe;
    maxRe = minRe + newWRe;
    
    maxIm = centerIm + ((double)mouseY / bmpH) * newWIm;
    minIm = maxIm - newWIm;
    
    double zoomLevel = 3.0 / newWRe;
    unsigned int needed_iter = 100;
    double temp_z = zoomLevel;
    int limit = 0;
    while (temp_z > 5.0 && limit < 50) {
        needed_iter += 30;
        temp_z /= 2.0;
        limit++;
    }
    if (needed_iter > max_iter && zoomLevel > 5.0) {
        max_iter = needed_iter;
        if (max_iter > 2000) max_iter = 2000;
    }
    
    RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
}

void SaveImage4K(HWND hwnd) {
    int expW = 3840;
    int expH = 2160;
    int imageSize32 = expW * expH * 4;
    DWORD* buffer = (DWORD*)HeapAlloc(GetProcessHeap(), 0, imageSize32);
    if (!buffer) {
        MessageBox(hwnd, "Failed to allocate memory for 4K export.", "Error", MB_OK);
        return;
    }
    
    SetWindowText(hwnd, "KMandelApp - Rendering 4K image...");
    RenderMandelbrotToBuffer(buffer, expW, expH);
    SetWindowText(hwnd, "KMandel - Press F1 or H for Help");
    
    OPENFILENAME ofn = {0};
    char szFileName[MAX_PATH] = "kmandel_4k.bmp";
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
            
            bfh.bfType = 0x4D42;
            bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageSize32;
            bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
            
            bih.biSize = sizeof(BITMAPINFOHEADER);
            bih.biWidth = expW;
            bih.biHeight = -expH;
            bih.biPlanes = 1;
            bih.biBitCount = 32;
            bih.biCompression = BI_RGB;
            bih.biSizeImage = imageSize32;
            
            DWORD dwWritten;
            WriteFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
            WriteFile(hFile, &bih, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
            WriteFile(hFile, buffer, imageSize32, &dwWritten, NULL);
            CloseHandle(hFile);
        }
    }
    HeapFree(GetProcessHeap(), 0, buffer);
}

void PickColor(HWND hwnd, unsigned char* color) {
    CHOOSECOLOR cc;
    static COLORREF acrCustClr[16]; 
    memset(&cc, 0, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwnd;
    cc.lpCustColors = (LPDWORD)acrCustClr;
    cc.rgbResult = RGB(color[0], color[1], color[2]);
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;
    if (ChooseColor(&cc)) {
        color[0] = GetRValue(cc.rgbResult);
        color[1] = GetGValue(cc.rgbResult);
        color[2] = GetBValue(cc.rgbResult);
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
            
            if ((wParam & MK_SHIFT) && !isJulia) {
                // Interactive Julia bridging
                double re_factor = (maxRe - minRe) / (bmpW - 1);
                double im_factor = (maxIm - minIm) / (bmpH - 1);
                juliaCRe = minRe + x * re_factor;
                juliaCIm = maxIm - y * im_factor;
                
                HDC hdc = GetDC(hwnd);
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 0));
                HPEN hOld = (HPEN)SelectObject(hdc, hPen);
                MoveToEx(hdc, x - 15, y, NULL); LineTo(hdc, x + 16, y);
                MoveToEx(hdc, x, y - 15, NULL); LineTo(hdc, x, y + 16);
                SelectObject(hdc, hOld);
                DeleteObject(hPen);
                ReleaseDC(hwnd, hdc);
                Sleep(50);
                
                isJulia = 1;
                minRe = -2.0; maxRe = 2.0;
                minIm = -2.0; maxIm = 2.0;
                max_iter = 100;
                SaveState();
                RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else {
                HDC hdc = GetDC(hwnd);
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                HPEN hOld = (HPEN)SelectObject(hdc, hPen);
                HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hBrush);
                Rectangle(hdc, x - 50, y - 37, x + 50, y + 37);
                SelectObject(hdc, hOld);
                SelectObject(hdc, hOldB);
                DeleteObject(hPen);
                ReleaseDC(hwnd, hdc);
                Sleep(50);

                Zoom(0.5, x, y); // Zoom in
                SaveState();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_RBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            HDC hdc = GetDC(hwnd);
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
            HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HPEN hOld = (HPEN)SelectObject(hdc, hPen);
            HBRUSH hOldB = (HBRUSH)SelectObject(hdc, hBrush);
            Rectangle(hdc, x - 100, y - 75, x + 100, y + 75);
            SelectObject(hdc, hOld);
            SelectObject(hdc, hOldB);
            DeleteObject(hPen);
            ReleaseDC(hwnd, hdc);
            Sleep(50);
            
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
                RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'T') {
                theme = (theme + 1) % 5;
                SaveState();
                RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'C') {
                theme = 4; // Switch to Custom
                PickColor(hwnd, customColor1);
                PickColor(hwnd, customColor2);
                SaveState();
                RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
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
                RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 'Z') {
                if (history_idx > 0) {
                    history_idx--;
                    LoadState(history_idx);
                    RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 'Y') {
                if (history_idx < history_max) {
                    history_idx++;
                    LoadState(history_idx);
                    RenderMandelbrotToBuffer(pixels, bmpW, bmpH);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == 'S') {
                SaveImage4K(hwnd);
            } else if (wParam == 'H' || wParam == VK_F1) {
                MessageBox(hwnd, "KMandel Help\n\nL/R Click: Zoom\nShift+L: Pick Julia\nZ/Y: Undo/Redo\nS: Save 4K\nR: Reset\nT: Theme\nC: Colors\nJ: Julia\nF1/H: Help", "Help", MB_OK);
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
            
            // Draw Help Background with a stylized outline
            RECT textBg = { 10, bmpH - 46, 220, bmpH - 10 };
            HBRUSH hBrush = CreateSolidBrush(RGB(20, 30, 50));
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(100, 150, 255));
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            RoundRect(hdc, textBg.left, textBg.top, textBg.right, textBg.bottom, 12, 12);
            
            // Draw a GDI composed vector icon (Stylized 'i' in a circle)
            HPEN iconPen = CreatePen(PS_SOLID, 2, RGB(100, 200, 255));
            SelectObject(hdc, iconPen);
            Arc(hdc, 20, bmpH - 38, 36, bmpH - 22, 0, 0, 0, 0); // Circle
            MoveToEx(hdc, 28, bmpH - 34, NULL); // 'i' dot
            LineTo(hdc, 28, bmpH - 33);
            MoveToEx(hdc, 28, bmpH - 30, NULL); // 'i' body
            LineTo(hdc, 28, bmpH - 25);
            
            SelectObject(hdc, hOldBrush);
            SelectObject(hdc, hOldPen);
            DeleteObject(hBrush);
            DeleteObject(hPen);
            DeleteObject(iconPen);
            
            // Draw Help Text
            SetBkMode(hdc, TRANSPARENT);
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            int fontHeight = -MulDiv(12, dpi, 72);
            HFONT hFont = CreateFont(fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                     DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            SetTextColor(hdc, RGB(255, 255, 255));
            TextOut(hdc, 44, bmpH - 36, "Press F1 or H for Help", 22);
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            
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

void MainEntry() {
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KMandelApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    RECT rect = { 0, 0, W, H };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, FALSE);

    HWND hwnd = CreateWindowEx(0, "KMandelApp", "KMandel - Press F1 or H for Help", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    SaveState(); // Save initial state

    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
