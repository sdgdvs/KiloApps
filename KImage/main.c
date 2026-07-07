#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#define W 500
#define H 500

HWND hBtnOpen;
HBITMAP hBmp = NULL;
int bmpW = 0;
int bmpH = 0;

void OpenFileDlg(HWND hwnd) {
    OPENFILENAMEA ofn = {0};
    char szFile[MAX_PATH] = {0};
    
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Bitmap Files\0*.bmp\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileNameA(&ofn) == TRUE) {
        if (hBmp) DeleteObject(hBmp);
        hBmp = (HBITMAP)LoadImageA(NULL, szFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        if (hBmp) {
            BITMAP bm;
            GetObject(hBmp, sizeof(bm), &bm);
            bmpW = bm.bmWidth;
            bmpH = bm.bmHeight;
            SetWindowTextA(hwnd, szFile);
        } else {
            MessageBoxA(hwnd, "Failed to load bitmap.", "Error", MB_OK);
        }
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            hBtnOpen = CreateWindowEx(0, "BUTTON", "Open Bitmap",
                WS_CHILD | WS_VISIBLE,
                10, 10, 100, 30, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hBtnOpen, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) {
                OpenFileDlg(hwnd);
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rc;
            GetClientRect(hwnd, &rc);
            
            if (hBmp) {
                HDC hdcMem = CreateCompatibleDC(hdc);
                HGDIOBJ oldBmp = SelectObject(hdcMem, hBmp);
                
                // Draw centered or scaled down
                int drawW = bmpW;
                int drawH = bmpH;
                int canvasW = rc.right - rc.left;
                int canvasH = rc.bottom - rc.top - 50;
                
                if (drawW > canvasW || drawH > canvasH) {
                    float ratio = (float)drawW / (float)drawH;
                    if (canvasW / ratio <= canvasH) {
                        drawW = canvasW;
                        drawH = (int)(canvasW / ratio);
                    } else {
                        drawH = canvasH;
                        drawW = (int)(canvasH * ratio);
                    }
                }
                
                int x = (canvasW - drawW) / 2;
                int y = 50 + (canvasH - drawH) / 2;
                
                SetStretchBltMode(hdc, HALFTONE);
                StretchBlt(hdc, x, y, drawW, drawH, hdcMem, 0, 0, bmpW, bmpH, SRCCOPY);
                
                SelectObject(hdcMem, oldBmp);
                DeleteDC(hdcMem);
            }
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBRUSH brush = CreateSolidBrush(RGB(200, 200, 200));
            FillRect(hdc, &rc, brush);
            DeleteObject(brush);
            return 1;
        }
        case WM_DESTROY:
            if (hBmp) DeleteObject(hBmp);
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

int _fltused = 1;
long _ftol2_sse(float f) { return (long)f; }
long _ftol2(float f) { return (long)f; }

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KImageApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KImageApp", "KImage", WS_OVERLAPPEDWINDOW,
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
