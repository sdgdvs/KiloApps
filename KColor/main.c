#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define W 300
#define H 200

HWND hScrollR, hScrollG, hScrollB;
HWND hLabelR, hLabelG, hLabelB;
HWND hColorBox;
int r = 100, g = 150, b = 200;
HBRUSH colorBrush;

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFont) {
    SendMessage(child, WM_SETFONT, hFont, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            CreateWindowEx(0, "STATIC", "R:", WS_CHILD | WS_VISIBLE, 10, 15, 20, 20, hwnd, NULL, NULL, NULL);
            hScrollR = CreateWindowEx(0, "SCROLLBAR", "", WS_CHILD | WS_VISIBLE | SBS_HORZ, 35, 15, 120, 20, hwnd, (HMENU)1, NULL, NULL);
            SetScrollRange(hScrollR, SB_CTL, 0, 255, FALSE);
            SetScrollPos(hScrollR, SB_CTL, r, TRUE);
            hLabelR = CreateWindowEx(0, "STATIC", "100", WS_CHILD | WS_VISIBLE, 160, 15, 30, 20, hwnd, NULL, NULL, NULL);
            
            CreateWindowEx(0, "STATIC", "G:", WS_CHILD | WS_VISIBLE, 10, 45, 20, 20, hwnd, NULL, NULL, NULL);
            hScrollG = CreateWindowEx(0, "SCROLLBAR", "", WS_CHILD | WS_VISIBLE | SBS_HORZ, 35, 45, 120, 20, hwnd, (HMENU)2, NULL, NULL);
            SetScrollRange(hScrollG, SB_CTL, 0, 255, FALSE);
            SetScrollPos(hScrollG, SB_CTL, g, TRUE);
            hLabelG = CreateWindowEx(0, "STATIC", "150", WS_CHILD | WS_VISIBLE, 160, 45, 30, 20, hwnd, NULL, NULL, NULL);
            
            CreateWindowEx(0, "STATIC", "B:", WS_CHILD | WS_VISIBLE, 10, 75, 20, 20, hwnd, NULL, NULL, NULL);
            hScrollB = CreateWindowEx(0, "SCROLLBAR", "", WS_CHILD | WS_VISIBLE | SBS_HORZ, 35, 75, 120, 20, hwnd, (HMENU)3, NULL, NULL);
            SetScrollRange(hScrollB, SB_CTL, 0, 255, FALSE);
            SetScrollPos(hScrollB, SB_CTL, b, TRUE);
            hLabelB = CreateWindowEx(0, "STATIC", "200", WS_CHILD | WS_VISIBLE, 160, 75, 30, 20, hwnd, NULL, NULL, NULL);
            
            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            
            colorBrush = CreateSolidBrush(RGB(r, g, b));
            break;
        }
        case WM_HSCROLL: {
            HWND hScroll = (HWND)lParam;
            int pos = GetScrollPos(hScroll, SB_CTL);
            int code = LOWORD(wParam);
            if (code == SB_LINELEFT) pos -= 1;
            else if (code == SB_LINERIGHT) pos += 1;
            else if (code == SB_PAGELEFT) pos -= 10;
            else if (code == SB_PAGERIGHT) pos += 10;
            else if (code == SB_THUMBPOSITION || code == SB_THUMBTRACK) pos = HIWORD(wParam);
            
            if (pos < 0) pos = 0;
            if (pos > 255) pos = 255;
            
            SetScrollPos(hScroll, SB_CTL, pos, TRUE);
            
            char buf[16];
            if (hScroll == hScrollR) {
                r = pos;
                wsprintfA(buf, "%d", r);
                SetWindowTextA(hLabelR, buf);
            } else if (hScroll == hScrollG) {
                g = pos;
                wsprintfA(buf, "%d", g);
                SetWindowTextA(hLabelG, buf);
            } else if (hScroll == hScrollB) {
                b = pos;
                wsprintfA(buf, "%d", b);
                SetWindowTextA(hLabelB, buf);
            }
            
            if (colorBrush) DeleteObject(colorBrush);
            colorBrush = CreateSolidBrush(RGB(r, g, b));
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rect = { 200, 15, 270, 95 };
            FillRect(hdc, &rect, colorBrush);
            
            // Draw hex value
            char hex[16];
            wsprintfA(hex, "#%02X%02X%02X", r, g, b);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0,0,0));
            TextOutA(hdc, 205, 105, hex, lstrlenA(hex));
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            if (colorBrush) DeleteObject(colorBrush);
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

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KColorApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KColorApp", "KColor", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
