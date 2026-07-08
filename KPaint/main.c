#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

#pragma function(memset)

int isPainting = 0;
HDC hdcMem = NULL;
HBITMAP hbmCanvas = NULL;
int lastX = 0, lastY = 0;
HPEN hPen = NULL;

COLORREF curColor = RGB(0,0,0);
int curSize = 2;

HWND hBtnBlack, hBtnRed, hBtnGreen, hBtnBlue, hBtnEraser;
HWND hBtnSize1, hBtnSize5, hBtnSave, hBtnClear;

void UpdatePen() {
    if (hPen) DeleteObject(hPen);
    hPen = CreatePen(PS_SOLID, curSize, curColor);
    if (hdcMem) SelectObject(hdcMem, hPen);
}

int SaveBitmap(const char* path, HBITMAP hbm) {
    HDC hdc = GetDC(NULL);
    BITMAP bmp;
    GetObject(hbm, sizeof(BITMAP), &bmp);

    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = 800; // Hardcode size for export to avoid huge white space
    bi.biHeight = 600;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    DWORD dwBmpSize = ((800 * 24 + 31) / 32) * 4 * 600;
    
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char* lpbitmap = (char*)GlobalLock(hDIB);
    
    // Create a temporary mem dc to get the right dimensions
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
        bmfHeader.bfType = 0x4D42; // "BM"

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
            
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hBtnBlack = CreateWindowA("BUTTON", "Black", WS_CHILD | WS_VISIBLE, 5, 5, 70, 25, hwnd, (HMENU)101, NULL, NULL);
            hBtnRed = CreateWindowA("BUTTON", "Red", WS_CHILD | WS_VISIBLE, 5, 35, 70, 25, hwnd, (HMENU)102, NULL, NULL);
            hBtnGreen = CreateWindowA("BUTTON", "Green", WS_CHILD | WS_VISIBLE, 5, 65, 70, 25, hwnd, (HMENU)103, NULL, NULL);
            hBtnBlue = CreateWindowA("BUTTON", "Blue", WS_CHILD | WS_VISIBLE, 5, 95, 70, 25, hwnd, (HMENU)104, NULL, NULL);
            hBtnEraser = CreateWindowA("BUTTON", "Eraser", WS_CHILD | WS_VISIBLE, 5, 125, 70, 25, hwnd, (HMENU)105, NULL, NULL);
            
            hBtnSize1 = CreateWindowA("BUTTON", "Size: 1", WS_CHILD | WS_VISIBLE, 5, 165, 70, 25, hwnd, (HMENU)201, NULL, NULL);
            hBtnSize5 = CreateWindowA("BUTTON", "Size: 5", WS_CHILD | WS_VISIBLE, 5, 195, 70, 25, hwnd, (HMENU)202, NULL, NULL);
            
            hBtnClear = CreateWindowA("BUTTON", "Clear", WS_CHILD | WS_VISIBLE, 5, 235, 70, 25, hwnd, (HMENU)301, NULL, NULL);
            hBtnSave = CreateWindowA("BUTTON", "Save BMP", WS_CHILD | WS_VISIBLE, 5, 265, 70, 25, hwnd, (HMENU)302, NULL, NULL);
            
            SendMessage(hBtnBlack, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnRed, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnGreen, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnBlue, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnEraser, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSize1, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSize5, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnClear, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnSave, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 101) curColor = RGB(0,0,0);
            if (id == 102) curColor = RGB(231,76,60);
            if (id == 103) curColor = RGB(46,204,113);
            if (id == 104) curColor = RGB(52,152,219);
            if (id == 105) curColor = RGB(255,255,255);
            
            if (id == 201) curSize = 2;
            if (id == 202) curSize = 6;
            
            if (id >= 101 && id <= 202) UpdatePen();
            
            if (id == 301) {
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
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam) - 80;
            int y = HIWORD(lParam);
            if (x >= 0) {
                isPainting = 1;
                lastX = x;
                lastY = y;
                MoveToEx(hdcMem, x, y, NULL);
            }
            break;
        }
        case WM_MOUSEMOVE: {
            int x = LOWORD(lParam) - 80;
            int y = HIWORD(lParam);
            if (isPainting && x >= 0) {
                HDC hdc = GetDC(hwnd);
                SelectObject(hdc, hPen);
                MoveToEx(hdc, lastX + 80, lastY, NULL);
                LineTo(hdc, x + 80, y);
                ReleaseDC(hwnd, hdc);

                LineTo(hdcMem, x, y);
                
                lastX = x;
                lastY = y;
            }
            break;
        }
        case WM_LBUTTONUP: {
            isPainting = 0;
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT r;
            GetClientRect(hwnd, &r);
            // Sidebar background
            RECT sidebar = {0, 0, 80, r.bottom};
            FillRect(hdc, &sidebar, (HBRUSH)(COLOR_BTNFACE + 1));
            // Canvas
            BitBlt(hdc, 80, 0, r.right - 80, r.bottom, hdcMem, 0, 0, SRCCOPY);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            if (hPen) DeleteObject(hPen);
            if (hdcMem) DeleteDC(hdcMem);
            if (hbmCanvas) DeleteObject(hbmCanvas);
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
    wc.hbrBackground = NULL; // hand painted

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KPaintClass", "KPaint", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 700, 500, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
