#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define W 950
#define H 700

HWND hList, hSizeList;
HWND hCustomText, hBold, hItalic;
HWND hTabMetrics, hTabGlyphs, hTabDiag, hTabSample, hHelpBtn;
HWND hPanel, hRangeList;

HFONT hCurrentFont = NULL;
int currentSize = 24;
BOOL isBold = FALSE;
BOOL isItalic = FALSE;
char currentFontName[64] = "Arial";
int currentTab = 0;
char currentCustomText[512] = "The quick brown fox jumps over the lazy dog.\r\n\r\n0123456789\r\n\r\nAa Bb Cc Dd Ee Ff";

HFONT hFont = NULL;
HBRUSH hBrush = NULL;
HBRUSH hBgBrush = NULL;
HBRUSH hPanelBrush = NULL;

LRESULT CALLBACK PanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == 14 && HIWORD(wParam) == CBN_SELCHANGE) {
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0,0,0));
            
            if (currentTab == 0) { // Metrics
                HFONT hOld = SelectObject(hdc, hCurrentFont);
                UINT cbData = GetOutlineTextMetricsA(hdc, 0, NULL);
                if (cbData > 0) {
                    OUTLINETEXTMETRIC *potm = HeapAlloc(GetProcessHeap(), 0, cbData);
                    if (GetOutlineTextMetricsA(hdc, cbData, potm)) {
                        SelectObject(hdc, hFont);
                        char buf[256];
                        int y = 10;
                        wsprintfA(buf, "Font Box: [%d, %d] to [%d, %d]", potm->otmrcFontBox.left, potm->otmrcFontBox.top, potm->otmrcFontBox.right, potm->otmrcFontBox.bottom);
                        TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                        wsprintfA(buf, "Mac Ascent: %d, Mac Descent: %d, Mac LineGap: %d", potm->otmMacAscent, potm->otmMacDescent, potm->otmMacLineGap);
                        TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                        wsprintfA(buf, "Typo Ascent: %d, Typo Descent: %d, LineGap: %d", potm->otmAscent, potm->otmDescent, potm->otmLineGap);
                        TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                        wsprintfA(buf, "Cap Em Height: %u, x-Height: %u", potm->otmsCapEmHeight, potm->otmsXHeight);
                        TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                        wsprintfA(buf, "OS/2 fsSelection: 0x%04X, fsType: 0x%04X", potm->otmfsSelection, potm->otmfsType);
                        TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                        wsprintfA(buf, "Panose: %d %d %d %d %d %d %d %d %d %d", potm->otmPanoseNumber.bFamilyType, potm->otmPanoseNumber.bSerifStyle, potm->otmPanoseNumber.bWeight, potm->otmPanoseNumber.bProportion, potm->otmPanoseNumber.bContrast, potm->otmPanoseNumber.bStrokeVariation, potm->otmPanoseNumber.bArmStyle, potm->otmPanoseNumber.bLetterform, potm->otmPanoseNumber.bMidline, potm->otmPanoseNumber.bXHeight);
                        TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                    }
                    HeapFree(GetProcessHeap(), 0, potm);
                } else {
                    SelectObject(hdc, hFont);
                    TextOutA(hdc, 10, 10, "Outline text metrics not available for this font.", 49);
                }
                
                SelectObject(hdc, hCurrentFont);
                TEXTMETRICA tm;
                GetTextMetricsA(hdc, &tm);
                SelectObject(hdc, hFont);
                int y = 160;
                char buf[256];
                wsprintfA(buf, "tmAscent: %d, tmDescent: %d, tmInternalLeading: %d", tm.tmAscent, tm.tmDescent, tm.tmInternalLeading);
                TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                wsprintfA(buf, "tmWeight: %d, tmOverhang: %d, tmMaxCharWidth: %d", tm.tmWeight, tm.tmOverhang, tm.tmMaxCharWidth);
                TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                
                SelectObject(hdc, hOld);
            }
            else if (currentTab == 1) { // Glyphs
                int sel = SendMessage(hRangeList, CB_GETCURSEL, 0, 0);
                int start = 0x20, end = 0x7F;
                if(sel == 1) { start = 0xA0; end = 0xFF; }
                else if(sel == 2) { start = 0x100; end = 0x17F; }
                else if(sel == 3) { start = 0x370; end = 0x3FF; }
                else if(sel == 4) { start = 0x400; end = 0x4FF; }
                
                HFONT hOld = SelectObject(hdc, hCurrentFont);
                int x = 10, y = 40;
                for (int i = start; i <= end; i++) {
                    WCHAR wch = (WCHAR)i;
                    SIZE sz;
                    GetTextExtentPoint32W(hdc, &wch, 1, &sz);
                    TextOutW(hdc, x, y, &wch, 1);
                    
                    SelectObject(hdc, hFont);
                    char hexBuf[10];
                    wsprintfA(hexBuf, "%04X", i);
                    TextOutA(hdc, x, y + sz.cy + 2, hexBuf, lstrlenA(hexBuf));
                    SelectObject(hdc, hCurrentFont);
                    
                    x += (sz.cx > 35 ? sz.cx : 35) + 15;
                    if (x > W - 240) { x = 10; y += sz.cy + 20; }
                }
                SelectObject(hdc, hOld);
            }
            else if (currentTab == 2) { // Diagnostics
                HFONT hOld = SelectObject(hdc, hCurrentFont);
                DWORD cPairs = GetKerningPairsA(hdc, 0, NULL);
                SelectObject(hdc, hFont);
                char buf[128];
                wsprintfA(buf, "Kerning Pairs Available: %u", cPairs);
                TextOutA(hdc, 10, 10, buf, lstrlenA(buf));
                
                SelectObject(hdc, hCurrentFont);
                TextOutA(hdc, 10, 40, "AV To Tr WA Ye Va fj", 20);
                
                SelectObject(hdc, hFont);
                TextOutA(hdc, 10, 120, "Rasterization & Grid Fitting (Hinting):", 39);
                int y = 140;
                int sizes[] = {10, 12, 16, 20, 24, 32};
                for(int i=0; i<6; i++) {
                    HFONT hSz = CreateFontA(sizes[i], 0, 0, 0, isBold?FW_BOLD:FW_NORMAL, isItalic, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, currentFontName);
                    SelectObject(hdc, hSz);
                    wsprintfA(buf, "%dpx: The quick brown fox", sizes[i]);
                    TextOutA(hdc, 10, y, buf, lstrlenA(buf));
                    SIZE sz;
                    GetTextExtentPoint32A(hdc, buf, lstrlenA(buf), &sz);
                    y += sz.cy + 5;
                    DeleteObject(hSz);
                }
                
                SelectObject(hdc, hOld);
            }
            else if (currentTab == 3) { // Sample
                HFONT hOld = SelectObject(hdc, hCurrentFont);
                RECT rc = {10, 10, W - 210, H - 110};
                DrawTextA(hdc, currentCustomText, -1, &rc, DT_WORDBREAK | DT_LEFT);
                SelectObject(hdc, hOld);
            }
            
            EndPaint(hwnd, &ps);
            break;
        }
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int CALLBACK EnumFontFamExProc(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam) {
    if (SendMessage(hList, LB_FINDSTRINGEXACT, -1, (LPARAM)lpelfe->lfFaceName) == LB_ERR) {
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)lpelfe->lfFaceName);
    }
    return 1;
}

BOOL CALLBACK SetFontProc(HWND child, LPARAM font) {
    SendMessage(child, WM_SETFONT, font, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
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
            hCustomText = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "The quick brown fox jumps over the lazy dog.\r\n\r\n0123456789\r\n\r\nAa Bb Cc Dd Ee Ff", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, 10, 260, 150, 120, hwnd, (HMENU)3, NULL, NULL);

            hBold = CreateWindowEx(0, "BUTTON", "Bold", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 10, 390, 70, 20, hwnd, (HMENU)4, NULL, NULL);
            hItalic = CreateWindowEx(0, "BUTTON", "Italic", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 80, 390, 70, 20, hwnd, (HMENU)5, NULL, NULL);
            
            hTabMetrics = CreateWindowEx(0, "BUTTON", "Metrics", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 170, 10, 100, 20, hwnd, (HMENU)10, NULL, NULL);
            hTabGlyphs = CreateWindowEx(0, "BUTTON", "Glyphs", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 280, 10, 100, 20, hwnd, (HMENU)11, NULL, NULL);
            hTabDiag = CreateWindowEx(0, "BUTTON", "Diagnostics", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 390, 10, 120, 20, hwnd, (HMENU)12, NULL, NULL);
            hTabSample = CreateWindowEx(0, "BUTTON", "Sample", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 520, 10, 100, 20, hwnd, (HMENU)13, NULL, NULL);
            hHelpBtn = CreateWindowEx(0, "BUTTON", "Help (H)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 630, 10, 100, 20, hwnd, (HMENU)15, NULL, NULL);
            
            SendMessage(hTabMetrics, BM_SETCHECK, BST_CHECKED, 0);

            WNDCLASS pc = {0};
            pc.lpfnWndProc = PanelProc;
            pc.hInstance = GetModuleHandle(NULL);
            pc.lpszClassName = "KFontPanel";
            hPanelBrush = CreateSolidBrush(RGB(255,255,255));
            pc.hbrBackground = hPanelBrush;
            RegisterClass(&pc);
            
            hPanel = CreateWindowEx(WS_EX_CLIENTEDGE, "KFontPanel", "", WS_CHILD | WS_VISIBLE, 170, 40, W - 190, H - 90, hwnd, NULL, NULL, NULL);
            
            hRangeList = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 10, 10, 250, 200, hPanel, (HMENU)14, NULL, NULL);
            const char* blocks[] = {
                "Basic Latin (0020-007F)",
                "Latin-1 Supp (00A0-00FF)",
                "Latin Ext-A (0100-017F)",
                "Greek (0370-03FF)",
                "Cyrillic (0400-04FF)"
            };
            for (int i = 0; i < 5; i++) {
                SendMessage(hRangeList, CB_ADDSTRING, 0, (LPARAM)blocks[i]);
            }
            SendMessage(hRangeList, CB_SETCURSEL, 0, 0);
            ShowWindow(hRangeList, SW_HIDE); // hide on startup (Metrics tab active)
            
            EnumChildWindows(hwnd, SetFontProc, (LPARAM)hFont);
            SendMessage(hRangeList, WM_SETFONT, (WPARAM)hFont, TRUE);
            
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
            if (LOWORD(wParam) == 15 && HIWORD(wParam) == BN_CLICKED) {
                MessageBox(hwnd, "Metrics & OS/2: Inspect font measurements and bounds.\nUnicode Ranges: View glyphs organized by unicode block.\nDiagnostics: Check visual kerning and hinting at various sizes.\nSample: Test how the font looks with custom text.", "KFont Help", MB_OK | MB_ICONINFORMATION);
            }
            else if (LOWORD(wParam) >= 10 && LOWORD(wParam) <= 13 && HIWORD(wParam) == BN_CLICKED) {
                currentTab = LOWORD(wParam) - 10;
                ShowWindow(hRangeList, currentTab == 1 ? SW_SHOW : SW_HIDE);
                InvalidateRect(hPanel, NULL, TRUE);
            }
            else if ((LOWORD(wParam) == 1 && HIWORD(wParam) == LBN_SELCHANGE) ||
                (LOWORD(wParam) == 2 && HIWORD(wParam) == CBN_SELCHANGE) ||
                (LOWORD(wParam) == 3 && HIWORD(wParam) == EN_CHANGE) ||
                (LOWORD(wParam) == 4 && HIWORD(wParam) == BN_CLICKED) ||
                (LOWORD(wParam) == 5 && HIWORD(wParam) == BN_CLICKED)) {
                
                if (LOWORD(wParam) == 3 && HIWORD(wParam) == EN_CHANGE) {
                    GetWindowText(hCustomText, currentCustomText, 512);
                    if (currentTab == 3) InvalidateRect(hPanel, NULL, TRUE);
                    break;
                }
                
                isBold = SendMessage(hBold, BM_GETCHECK, 0, 0) == BST_CHECKED;
                isItalic = SendMessage(hItalic, BM_GETCHECK, 0, 0) == BST_CHECKED;
                
                int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    SendMessage(hList, LB_GETTEXT, sel, (LPARAM)currentFontName);
                    
                    int sizeSel = SendMessage(hSizeList, CB_GETCURSEL, 0, 0);
                    if (sizeSel != CB_ERR) {
                        char sizeStr[16];
                        SendMessage(hSizeList, CB_GETLBTEXT, sizeSel, (LPARAM)sizeStr);
                        currentSize = 0;
                        for (int i = 0; sizeStr[i]; i++) currentSize = currentSize * 10 + (sizeStr[i] - '0');
                    }
                    
                    HFONT hOldFont = hCurrentFont;
                    hCurrentFont = CreateFontA(currentSize, 0, 0, 0, isBold ? FW_BOLD : FW_NORMAL, isItalic, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, currentFontName);
                    if (hOldFont) DeleteObject(hOldFont);
                    InvalidateRect(hPanel, NULL, TRUE);
                }
            }
            break;
        }
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORBTN: {
            HWND hChild = (HWND)lParam;
            if (hChild == hTabMetrics || hChild == hTabGlyphs || hChild == hTabDiag || hChild == hTabSample || hChild == hBold || hChild == hItalic) {
                HDC hdcStatic = (HDC)wParam;
                SetTextColor(hdcStatic, RGB(224, 224, 224));
                SetBkColor(hdcStatic, RGB(18, 18, 18));
                return (LRESULT)hBgBrush;
            }
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
            if (hPanelBrush) DeleteObject(hPanelBrush);
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
    SetProcessDPIAware();
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KFontApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    hBgBrush = CreateSolidBrush(RGB(18, 18, 18));
    wc.hbrBackground = hBgBrush;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KFontApp", "KFont (Press H for Help)", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, W, H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN && (msg.wParam == 'H' || msg.wParam == 'h')) {
            if (GetFocus() != hCustomText) {
                MessageBox(hwnd, "Metrics & OS/2: Inspect font measurements and bounds.\nUnicode Ranges: View glyphs organized by unicode block.\nDiagnostics: Check visual kerning and hinting at various sizes.\nSample: Test how the font looks with custom text.", "KFont Help", MB_OK | MB_ICONINFORMATION);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
