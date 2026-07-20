#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define W 450
#define H 350

HWND hList, hSizeList, hSample;
HWND hCustomText, hBold, hItalic;
HFONT hCurrentFont = NULL;
int currentSize = 24;
BOOL isBold = FALSE;
BOOL isItalic = FALSE;
HFONT hFont = NULL;
HBRUSH hBrush = NULL;
HBRUSH hBgBrush = NULL;

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
            hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            CreateWindowEx(0, "STATIC", "System Fonts:", WS_CHILD | WS_VISIBLE, 10, 10, 150, 20, hwnd, NULL, NULL, NULL);
            hList = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_SORT, 10, 30, 150, 160, hwnd, (HMENU)1, NULL, NULL);
            
            CreateWindowEx(0, "STATIC", "Size:", WS_CHILD | WS_VISIBLE, 10, 195, 150, 20, hwnd, NULL, NULL, NULL);
            hSizeList = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 10, 215, 150, 200, hwnd, (HMENU)2, NULL, NULL);
            const char* sizes[] = {"12", "16", "24", "32", "48", "64"};
            for (int i = 0; i < 6; i++) {
                SendMessage(hSizeList, CB_ADDSTRING, 0, (LPARAM)sizes[i]);
            }
            SendMessage(hSizeList, CB_SETCURSEL, 2, 0);
            
            CreateWindowEx(0, "STATIC", "Custom Text:", WS_CHILD | WS_VISIBLE, 10, 240, 150, 20, hwnd, NULL, NULL, NULL);
            hCustomText = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 260, 150, 20, hwnd, (HMENU)3, NULL, NULL);

            hBold = CreateWindowEx(0, "BUTTON", "Bold", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 10, 285, 70, 20, hwnd, (HMENU)4, NULL, NULL);
            hItalic = CreateWindowEx(0, "BUTTON", "Italic", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 80, 285, 70, 20, hwnd, (HMENU)5, NULL, NULL);
            
            hSample = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "The quick brown fox jumps over the lazy dog.\n\n0123456789\n\nAa Bb Cc Dd Ee Ff", WS_CHILD | WS_VISIBLE | SS_CENTER, 170, 30, 250, 275, hwnd, NULL, NULL, NULL);
            
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
            if ((LOWORD(wParam) == 1 && HIWORD(wParam) == LBN_SELCHANGE) ||
                (LOWORD(wParam) == 2 && HIWORD(wParam) == CBN_SELCHANGE) ||
                (LOWORD(wParam) == 3 && HIWORD(wParam) == EN_CHANGE) ||
                (LOWORD(wParam) == 4 && HIWORD(wParam) == BN_CLICKED) ||
                (LOWORD(wParam) == 5 && HIWORD(wParam) == BN_CLICKED)) {
                
                isBold = SendMessage(hBold, BM_GETCHECK, 0, 0) == BST_CHECKED;
                isItalic = SendMessage(hItalic, BM_GETCHECK, 0, 0) == BST_CHECKED;
                
                int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    char fontName[64];
                    SendMessage(hList, LB_GETTEXT, sel, (LPARAM)fontName);
                    
                    int sizeSel = SendMessage(hSizeList, CB_GETCURSEL, 0, 0);
                    if (sizeSel != CB_ERR) {
                        char sizeStr[16];
                        SendMessage(hSizeList, CB_GETLBTEXT, sizeSel, (LPARAM)sizeStr);
                        currentSize = 0;
                        for (int i = 0; sizeStr[i]; i++) currentSize = currentSize * 10 + (sizeStr[i] - '0');
                    }
                    
                    if (hCurrentFont) DeleteObject(hCurrentFont);
                    hCurrentFont = CreateFontA(currentSize, 0, 0, 0, isBold ? FW_BOLD : FW_NORMAL, isItalic, 0, 0, DEFAULT_CHARSET, 0, 0, DEFAULT_QUALITY, DEFAULT_PITCH, fontName);
                    SendMessage(hSample, WM_SETFONT, (WPARAM)hCurrentFont, TRUE);
                }
                
                char customText[256];
                GetWindowText(hCustomText, customText, 256);
                if (customText[0] != '\0') {
                    SetWindowText(hSample, customText);
                } else {
                    SetWindowText(hSample, "The quick brown fox jumps over the lazy dog.\n\n0123456789\n\nAa Bb Cc Dd Ee Ff");
                }
            }
            break;
        }
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORBTN: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(224, 224, 224));
            SetBkColor(hdcStatic, RGB(30, 30, 30));
            if (!hBrush) hBrush = CreateSolidBrush(RGB(30, 30, 30));
            return (LRESULT)hBrush;
        }
        case WM_DESTROY:
            if (hCurrentFont) DeleteObject(hCurrentFont);
            if (hFont) DeleteObject(hFont);
            if (hBrush) DeleteObject(hBrush);
            if (hBgBrush) DeleteObject(hBgBrush);
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
    hBgBrush = CreateSolidBrush(RGB(18, 18, 18));
    wc.hbrBackground = hBgBrush;
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
