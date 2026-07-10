#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 400
#define H 300

HWND hList, hSample;
HFONT hCurrentFont = NULL;

int CALLBACK EnumFontFamExProc(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam) {
    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)lpelfe->lfFaceName);
    return 1;
}

BOOL CALLBACK SetFontProc(HWND child, LPARAM hFont) {
    SendMessage(child, WM_SETFONT, hFont, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            CreateWindowEx(0, "STATIC", "System Fonts:", WS_CHILD | WS_VISIBLE, 10, 10, 150, 20, hwnd, NULL, NULL, NULL);
            hList = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_SORT, 10, 30, 150, 220, hwnd, (HMENU)1, NULL, NULL);
            
            hSample = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "The quick brown fox jumps over the lazy dog.\n\n0123456789\n\nAa Bb Cc Dd Ee Ff", WS_CHILD | WS_VISIBLE | SS_CENTER, 170, 30, 200, 220, hwnd, NULL, NULL, NULL);
            
            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            
            HDC hdc = GetDC(hwnd);
            LOGFONTA lf = {0};
            lf.lfCharSet = DEFAULT_CHARSET;
            EnumFontFamiliesExA(hdc, &lf, (FONTENUMPROCA)EnumFontFamExProc, 0, 0);
            ReleaseDC(hwnd, hdc);
            
            SendMessage(hList, LB_SETCURSEL, 0, 0);
            SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(1, LBN_SELCHANGE), (LPARAM)hList);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1 && HIWORD(wParam) == LBN_SELCHANGE) {
                int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    char fontName[64];
                    SendMessage(hList, LB_GETTEXT, sel, (LPARAM)fontName);
                    
                    if (hCurrentFont) DeleteObject(hCurrentFont);
                    hCurrentFont = CreateFontA(24, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, fontName);
                    SendMessage(hSample, WM_SETFONT, (WPARAM)hCurrentFont, TRUE);
                }
            }
            break;
        }
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORLISTBOX: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(224, 224, 224));
            SetBkColor(hdcStatic, RGB(30, 30, 30));
            static HBRUSH hBrush = NULL;
            if (!hBrush) hBrush = CreateSolidBrush(RGB(30, 30, 30));
            return (LRESULT)hBrush;
        }
        case WM_DESTROY:
            if (hCurrentFont) DeleteObject(hCurrentFont);
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
    wc.lpszClassName = "KFontApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = CreateSolidBrush(RGB(18, 18, 18));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KFontApp", "KFont", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
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
