#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define W 950
#define H 700

HWND hList, hSizeList;
HWND hCustomText, hBold, hItalic, hAnatomyChar;
HWND hTabMetrics, hTabGlyphs, hTabDiag, hTabAnatomy, hTabSample, hHelpBtn, hCopyBtn;
HWND hPanel, hRangeList;

HFONT hCurrentFont = NULL;
int currentSize = 24;
BOOL isBold = FALSE;
BOOL isItalic = FALSE;
char currentFontName[64] = "Arial";
int currentTab = 0;
char currentCustomText[512] = "The quick brown fox jumps over the lazy dog.\r\n\r\n0123456789\r\n\r\nAa Bb Cc Dd Ee Ff";
WCHAR anatomyChar = L'A';

HFONT hFont = NULL;
HBRUSH hBrush = NULL;
HBRUSH hBgBrush = NULL;
HBRUSH hPanelBrush = NULL;

static unsigned int ParseHexW(const WCHAR* s) {
    unsigned int val = 0;
    while (*s) {
        WCHAR c = *s++;
        if (c >= L'0' && c <= L'9') val = (val << 4) | (c - L'0');
        else if (c >= L'a' && c <= L'f') val = (val << 4) | (c - L'a' + 10);
        else if (c >= L'A' && c <= L'F') val = (val << 4) | (c - L'A' + 10);
        else break;
    }
    return val;
}

const char* GetHtmlEntity(WCHAR ch) {
    switch(ch) {
        case '&': return "&amp;";
        case '<': return "&lt;";
        case '>': return "&gt;";
        case '"': return "&quot;";
        case '\'': return "&apos;";
        case 0x00A9: return "&copy;";
        case 0x00AE: return "&reg;";
        case 0x20AC: return "&euro;";
        case 0x00A3: return "&pound;";
        case 0x00A5: return "&yen;";
        case 0x00B0: return "&deg;";
        default: return "-";
    }
}

void GetUtf8String(WCHAR ch, char* out, int maxLen) {
    unsigned int val = (unsigned int)(unsigned short)ch;
    if (val <= 0x7F) {
        if (val >= 32 && val <= 126) {
            wsprintfA(out, "0x%02X ('%c')", val, (char)val);
        } else {
            wsprintfA(out, "0x%02X", val);
        }
    } else if (val <= 0x7FF) {
        unsigned char b1 = (unsigned char)(0xC0 | (val >> 6));
        unsigned char b2 = (unsigned char)(0x80 | (val & 0x3F));
        wsprintfA(out, "0x%02X 0x%02X", b1, b2);
    } else {
        unsigned char b1 = (unsigned char)(0xE0 | (val >> 12));
        unsigned char b2 = (unsigned char)(0x80 | ((val >> 6) & 0x3F));
        unsigned char b3 = (unsigned char)(0x80 | (val & 0x3F));
        wsprintfA(out, "0x%02X 0x%02X 0x%02X", b1, b2, b3);
    }
}

LRESULT CALLBACK PanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == 20 && HIWORD(wParam) == CBN_SELCHANGE) {
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case WM_LBUTTONDOWN: {
            if (currentTab == 1) { // Glyphs tab - click glyph to inspect in Anatomy tab
                int mx = LOWORD(lParam);
                int my = HIWORD(lParam);
                if (my >= 40) {
                    int sel = SendMessage(hRangeList, CB_GETCURSEL, 0, 0);
                    int start = 0x20, end = 0x7F;
                    if(sel == 1) { start = 0xA0; end = 0xFF; }
                    else if(sel == 2) { start = 0x100; end = 0x17F; }
                    else if(sel == 3) { start = 0x370; end = 0x3FF; }
                    else if(sel == 4) { start = 0x400; end = 0x4FF; }

                    HDC hdc = GetDC(hwnd);
                    HFONT hOld = SelectObject(hdc, hCurrentFont);
                    int x = 10, y = 40;
                    for (int i = start; i <= end; i++) {
                        WCHAR wch = (WCHAR)i;
                        SIZE sz;
                        GetTextExtentPoint32W(hdc, &wch, 1, &sz);
                        int cellW = (sz.cx > 35 ? sz.cx : 35) + 15;
                        int cellH = sz.cy + 20;
                        if (mx >= x && mx <= x + cellW && my >= y && my <= y + cellH) {
                            anatomyChar = (WCHAR)i;
                            WCHAR wBuf[16];
                            wsprintfW(wBuf, L"U+%04X", (UINT)i);
                            SetWindowTextW(hAnatomyChar, wBuf);
                            currentTab = 3;
                            SendMessage(hTabAnatomy, BM_SETCHECK, BST_CHECKED, 0);
                            SendMessage(hTabGlyphs, BM_SETCHECK, BST_UNCHECKED, 0);
                            ShowWindow(hRangeList, SW_HIDE);
                            InvalidateRect(hwnd, NULL, TRUE);
                            break;
                        }
                        x += cellW;
                        if (x > W - 240) { x = 10; y += cellH; }
                    }
                    SelectObject(hdc, hOld);
                    ReleaseDC(hwnd, hdc);
                }
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdcScreen = BeginPaint(hwnd, &ps);
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);
            HDC hdc = CreateCompatibleDC(hdcScreen);
            HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, rcClient.right, rcClient.bottom);
            HBITMAP hOldBmp = (HBITMAP)SelectObject(hdc, hBmp);

            FillRect(hdc, &rcClient, hPanelBrush);
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
                    HFONT hSz = CreateFontA(-sizes[i], 0, 0, 0, isBold?FW_BOLD:FW_NORMAL, isItalic, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, currentFontName);
                    HFONT hPrev = SelectObject(hdc, hSz);
                    wsprintfA(buf, "%dpx: The quick brown fox", sizes[i]);
                    TextOutA(hdc, 10, y, buf, lstrlenA(buf));
                    SIZE sz;
                    GetTextExtentPoint32A(hdc, buf, lstrlenA(buf), &sz);
                    y += sz.cy + 5;
                    SelectObject(hdc, hPrev);
                    DeleteObject(hSz);
                }
                
                SelectObject(hdc, hOld);
            }
            else if (currentTab == 3) { // Anatomy & Vector Metrics Inspector
                HFONT hOld = SelectObject(hdc, hFont);
                
                // Header Info
                char headerBuf[128];
                wsprintfA(headerBuf, "Glyph Anatomy & Vector Metrics: U+%04X (Dec: %u)", (unsigned int)anatomyChar, (unsigned int)anatomyChar);
                TextOutA(hdc, 15, 10, headerBuf, lstrlenA(headerBuf));

                // Big Font for Vector Dissection
                int bigSize = 160;
                HFONT hBigFont = CreateFontA(-bigSize, 0, 0, 0, isBold ? FW_BOLD : FW_NORMAL, isItalic, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, currentFontName);
                SelectObject(hdc, hBigFont);
                
                TEXTMETRICW tmBig;
                GetTextMetricsW(hdc, &tmBig);

                ABC abc = {0};
                GetCharABCWidthsW(hdc, (UINT)anatomyChar, (UINT)anatomyChar, &abc);

                GLYPHMETRICS gm = {0};
                MAT2 mat2 = {{0,1}, {0,0}, {0,0}, {0,1}};
                GetGlyphOutlineW(hdc, (UINT)anatomyChar, GGO_METRICS, &gm, 0, NULL, &mat2);

                int gridX = 40;
                int baselineY = 220;
                int gridW = 660;

                // Draw Vector Guidelines
                HPEN hPenBase = CreatePen(PS_SOLID, 2, RGB(220, 50, 50));     // Red baseline
                HPEN hPenCap = CreatePen(PS_DASH, 1, RGB(40, 120, 240));      // Blue cap
                HPEN hPenX = CreatePen(PS_DOT, 1, RGB(0, 160, 160));          // Cyan x-height
                HPEN hPenAsc = CreatePen(PS_DOT, 1, RGB(40, 180, 40));        // Green ascent
                HPEN hPenDesc = CreatePen(PS_DOT, 1, RGB(180, 40, 180));      // Magenta descent
                HPEN hPenBearing = CreatePen(PS_DASH, 1, RGB(230, 140, 0));   // Orange bearings
                HPEN hPenBox = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));

                // Grid background box
                HPEN hOldPen = SelectObject(hdc, hPenBox);
                Rectangle(hdc, gridX - 20, 35, gridX + gridW, baselineY + tmBig.tmDescent + 20);

                // Ascent line
                SelectObject(hdc, hPenAsc);
                MoveToEx(hdc, gridX - 10, baselineY - tmBig.tmAscent, NULL);
                LineTo(hdc, gridX + gridW - 10, baselineY - tmBig.tmAscent);

                // Cap-Height approximation
                int capH = (int)(tmBig.tmAscent * 0.72);
                SelectObject(hdc, hPenCap);
                MoveToEx(hdc, gridX - 10, baselineY - capH, NULL);
                LineTo(hdc, gridX + gridW - 10, baselineY - capH);

                // X-Height approximation
                int xH = (int)(tmBig.tmAscent * 0.50);
                SelectObject(hdc, hPenX);
                MoveToEx(hdc, gridX - 10, baselineY - xH, NULL);
                LineTo(hdc, gridX + gridW - 10, baselineY - xH);

                // Baseline
                SelectObject(hdc, hPenBase);
                MoveToEx(hdc, gridX - 10, baselineY, NULL);
                LineTo(hdc, gridX + gridW - 10, baselineY);

                // Descent line
                SelectObject(hdc, hPenDesc);
                MoveToEx(hdc, gridX - 10, baselineY + tmBig.tmDescent, NULL);
                LineTo(hdc, gridX + gridW - 10, baselineY + tmBig.tmDescent);

                // Glyph Origin & Bearings
                int originX = gridX + 160;
                int lsbX = originX + abc.abcA;
                int rsbX = lsbX + (int)abc.abcB;

                SelectObject(hdc, hPenBearing);
                MoveToEx(hdc, originX, 40, NULL); LineTo(hdc, originX, baselineY + tmBig.tmDescent + 15);
                MoveToEx(hdc, lsbX, 40, NULL); LineTo(hdc, lsbX, baselineY + tmBig.tmDescent + 15);
                MoveToEx(hdc, rsbX, 40, NULL); LineTo(hdc, rsbX, baselineY + tmBig.tmDescent + 15);
                MoveToEx(hdc, originX + (abc.abcA + (int)abc.abcB + abc.abcC), 40, NULL); LineTo(hdc, originX + (abc.abcA + (int)abc.abcB + abc.abcC), baselineY + tmBig.tmDescent + 15);

                // Render the character
                SetTextColor(hdc, RGB(20, 20, 20));
                TextOutW(hdc, originX, baselineY - tmBig.tmAscent, &anatomyChar, 1);

                // Annotations on grid
                SelectObject(hdc, hFont);
                SetTextColor(hdc, RGB(220, 50, 50));
                TextOutA(hdc, gridX + gridW - 140, baselineY - 14, "Baseline (Y:0)", 14);

                SetTextColor(hdc, RGB(40, 120, 240));
                TextOutA(hdc, gridX + gridW - 140, baselineY - capH - 14, "Cap Height", 10);

                SetTextColor(hdc, RGB(0, 140, 140));
                TextOutA(hdc, gridX + gridW - 140, baselineY - xH - 14, "x-Height", 8);

                SetTextColor(hdc, RGB(40, 160, 40));
                TextOutA(hdc, gridX + gridW - 140, baselineY - tmBig.tmAscent - 14, "Ascent", 6);

                SetTextColor(hdc, RGB(160, 40, 160));
                TextOutA(hdc, gridX + gridW - 140, baselineY + tmBig.tmDescent - 14, "Descent", 7);

                SetTextColor(hdc, RGB(200, 110, 0));
                TextOutA(hdc, originX - 40, baselineY + tmBig.tmDescent + 3, "Origin", 6);
                TextOutA(hdc, lsbX - 15, 38, "LSB", 3);
                TextOutA(hdc, rsbX - 15, 38, "RSB", 3);

                // Cleanup Guidelines
                SelectObject(hdc, hOldPen);
                DeleteObject(hPenBase);
                DeleteObject(hPenCap);
                DeleteObject(hPenX);
                DeleteObject(hPenAsc);
                DeleteObject(hPenDesc);
                DeleteObject(hPenBearing);
                DeleteObject(hPenBox);
                DeleteObject(hBigFont);

                // Bottom Diagnostic Cards
                SetTextColor(hdc, RGB(0, 0, 0));
                int cardY = 320;
                char buf[256];

                // Card 1: GDI ABC Metrics
                TextOutA(hdc, 20, cardY, "[ GDI ABC Spacing & Bearings ]", 30);
                wsprintfA(buf, "A (Left Bearing LSB): %d px", abc.abcA);
                TextOutA(hdc, 20, cardY + 22, buf, lstrlenA(buf));
                wsprintfA(buf, "B (Black Box Body):  %u px", abc.abcB);
                TextOutA(hdc, 20, cardY + 42, buf, lstrlenA(buf));
                wsprintfA(buf, "C (Right Bearing RSB): %d px", abc.abcC);
                TextOutA(hdc, 20, cardY + 62, buf, lstrlenA(buf));
                wsprintfA(buf, "Total Advance Width:   %d px", abc.abcA + (int)abc.abcB + abc.abcC);
                TextOutA(hdc, 20, cardY + 82, buf, lstrlenA(buf));

                // Card 2: Outline Metrics (GGO_METRICS)
                TextOutA(hdc, 260, cardY, "[ GLYPHMETRICS Outline ]", 24);
                wsprintfA(buf, "Black Box (W x H): %u x %u", gm.gmBlackBoxX, gm.gmBlackBoxY);
                TextOutA(hdc, 260, cardY + 22, buf, lstrlenA(buf));
                wsprintfA(buf, "Glyph Origin (X, Y): (%d, %d)", gm.gmptGlyphOrigin.x, gm.gmptGlyphOrigin.y);
                TextOutA(hdc, 260, cardY + 42, buf, lstrlenA(buf));
                wsprintfA(buf, "Cell Inc (X, Y):   (%d, %d)", gm.gmCellIncX, gm.gmCellIncY);
                TextOutA(hdc, 260, cardY + 62, buf, lstrlenA(buf));
                wsprintfA(buf, "Em Ascent/Descent: %d / %d", tmBig.tmAscent, tmBig.tmDescent);
                TextOutA(hdc, 260, cardY + 82, buf, lstrlenA(buf));

                // Card 3: Multi-Encoding / Codec Dissector
                TextOutA(hdc, 500, cardY, "[ Multi-Encoding Codec ]", 24);
                char utf8Buf[64];
                GetUtf8String(anatomyChar, utf8Buf, sizeof(utf8Buf));
                wsprintfA(buf, "UTF-8 Bytes: %s", utf8Buf);
                TextOutA(hdc, 500, cardY + 22, buf, lstrlenA(buf));
                wsprintfA(buf, "UTF-16 Code Unit: 0x%04X", (unsigned int)anatomyChar);
                TextOutA(hdc, 500, cardY + 42, buf, lstrlenA(buf));
                wsprintfA(buf, "HTML: &#%u; (%s)", (unsigned int)anatomyChar, GetHtmlEntity(anatomyChar));
                TextOutA(hdc, 500, cardY + 62, buf, lstrlenA(buf));
                wsprintfA(buf, "C/C++ Escape: \\u%04X", (unsigned int)anatomyChar);
                TextOutA(hdc, 500, cardY + 82, buf, lstrlenA(buf));

                SelectObject(hdc, hOld);
            }
            else if (currentTab == 4) { // Sample
                HFONT hOld = SelectObject(hdc, hCurrentFont);
                RECT rc = {10, 10, W - 210, H - 70};
                DrawTextA(hdc, currentCustomText, -1, &rc, DT_WORDBREAK | DT_LEFT);
                SelectObject(hdc, hOld);
            }
            
            BitBlt(hdcScreen, 0, 0, rcClient.right, rcClient.bottom, hdc, 0, 0, SRCCOPY);
            SelectObject(hdc, hOldBmp);
            DeleteObject(hBmp);
            DeleteDC(hdc);
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

void CopyAnatomyToClipboard(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    int bigSize = 160;
    HFONT hBigFont = CreateFontA(-bigSize, 0, 0, 0, isBold ? FW_BOLD : FW_NORMAL, isItalic, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, currentFontName);
    HFONT hOld = SelectObject(hdc, hBigFont);
    
    TEXTMETRICW tmBig;
    GetTextMetricsW(hdc, &tmBig);
    ABC abc = {0};
    GetCharABCWidthsW(hdc, (UINT)anatomyChar, (UINT)anatomyChar, &abc);
    GLYPHMETRICS gm = {0};
    MAT2 mat2 = {{0,1}, {0,0}, {0,0}, {0,1}};
    GetGlyphOutlineW(hdc, (UINT)anatomyChar, GGO_METRICS, &gm, 0, NULL, &mat2);
    
    SelectObject(hdc, hOld);
    DeleteObject(hBigFont);
    ReleaseDC(hwnd, hdc);
    
    char utf8Buf[64];
    GetUtf8String(anatomyChar, utf8Buf, sizeof(utf8Buf));
    
    char report[1024];
    wsprintfA(report,
        "=== KFont Glyph Anatomy Report ===\r\n"
        "Font: %s (%d px%s%s)\r\n"
        "Glyph: U+%04X (Dec: %u)\r\n"
        "UTF-8 Bytes: %s\r\n"
        "UTF-16 Code Unit: 0x%04X\r\n"
        "HTML Entities: &#%u; (%s)\r\n"
        "C/C++ Literal: \\u%04X\r\n\r\n"
        "[ ABC Spacing & Bearings ]\r\n"
        "Left Side Bearing (LSB): %d px\r\n"
        "Black Box (Body): %u px (W: %u x H: %u)\r\n"
        "Right Side Bearing (RSB): %d px\r\n"
        "Total Advance Width: %d px\r\n"
        "Em Ascent: %d px, Descent: %d px\r\n",
        currentFontName, currentSize, isBold ? ", Bold" : "", isItalic ? ", Italic" : "",
        (unsigned int)anatomyChar, (unsigned int)anatomyChar,
        utf8Buf, (unsigned int)anatomyChar, (unsigned int)anatomyChar, GetHtmlEntity(anatomyChar),
        (unsigned int)anatomyChar,
        abc.abcA, abc.abcB, gm.gmBlackBoxX, gm.gmBlackBoxY, abc.abcC,
        abc.abcA + (int)abc.abcB + abc.abcC,
        tmBig.tmAscent, tmBig.tmDescent);

    if (OpenClipboard(hwnd)) {
        EmptyClipboard();
        int len = lstrlenA(report) + 1;
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
        if (hMem) {
            char* pMem = (char*)GlobalLock(hMem);
            if (pMem) {
                lstrcpyA(pMem, report);
                GlobalUnlock(hMem);
                SetClipboardData(CF_TEXT, hMem);
            }
        }
        CloseClipboard();
        MessageBoxA(hwnd, "Anatomy report copied to clipboard!", "KFont", MB_OK | MB_ICONINFORMATION);
    }
}

void SelectTab(HWND hwnd, int tabIdx) {
    if (tabIdx < 0 || tabIdx > 4) return;
    currentTab = tabIdx;
    HWND tabs[] = {hTabMetrics, hTabGlyphs, hTabDiag, hTabAnatomy, hTabSample};
    for (int i = 0; i < 5; i++) {
        SendMessage(tabs[i], BM_SETCHECK, (i == currentTab) ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    ShowWindow(hRangeList, currentTab == 1 ? SW_SHOW : SW_HIDE);
    InvalidateRect(hPanel, NULL, TRUE);
}

void UpdateFont(HWND hwnd) {
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
        hCurrentFont = CreateFontA(-currentSize, 0, 0, 0, isBold ? FW_BOLD : FW_NORMAL, isItalic, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, currentFontName);
        if (hOldFont) DeleteObject(hOldFont);
        InvalidateRect(hPanel, NULL, TRUE);
    }
}

void ShowHelpDialog(HWND hwnd) {
    MessageBox(hwnd,
        "KFont - Font Metrics & Anatomy Inspector\n\n"
        "Tabs:\n"
        "1. Metrics & OS/2: Inspect font bounds and text metrics\n"
        "2. Unicode Ranges: Browse glyphs by block (click to inspect in Anatomy)\n"
        "3. Diagnostics: View visual kerning pairs and hinting across sizes\n"
        "4. Anatomy: Vector metrics, ABC spacing, bearings, and codecs\n"
        "5. Live Sample: Test font with custom editable text\n\n"
        "Shortcuts:\n"
        "[1-5] : Switch Tabs\n"
        "[B]   : Toggle Bold\n"
        "[I]   : Toggle Italic\n"
        "[C]   : Copy Anatomy Report to Clipboard\n"
        "[H/F1]: Show this Help dialog",
        "KFont Help", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HDC hdcDpi = GetDC(hwnd);
            int dpi = GetDeviceCaps(hdcDpi, LOGPIXELSY);
            ReleaseDC(hwnd, hdcDpi);
            int fontHeight = -MulDiv(12, dpi, 72);
            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            CreateWindowEx(0, "STATIC", "System Fonts:", WS_CHILD | WS_VISIBLE, 10, 10, 150, 20, hwnd, NULL, NULL, NULL);
            hList = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_SORT | WS_TABSTOP, 10, 30, 150, 140, hwnd, (HMENU)1, NULL, NULL);
            
            CreateWindowEx(0, "STATIC", "Size:", WS_CHILD | WS_VISIBLE, 10, 175, 150, 20, hwnd, NULL, NULL, NULL);
            hSizeList = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 10, 195, 150, 200, hwnd, (HMENU)2, NULL, NULL);
            const char* sizes[] = {"12", "16", "24", "32", "48", "64"};
            for (int i = 0; i < 6; i++) {
                SendMessage(hSizeList, CB_ADDSTRING, 0, (LPARAM)sizes[i]);
            }
            SendMessage(hSizeList, CB_SETCURSEL, 2, 0);
            
            CreateWindowEx(0, "STATIC", "Inspect Glyph / Char:", WS_CHILD | WS_VISIBLE, 10, 225, 150, 20, hwnd, NULL, NULL, NULL);
            hAnatomyChar = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "A", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 10, 245, 150, 24, hwnd, (HMENU)6, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Custom Text:", WS_CHILD | WS_VISIBLE, 10, 275, 150, 20, hwnd, NULL, NULL, NULL);
            hCustomText = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "The quick brown fox jumps over the lazy dog.\r\n\r\n0123456789\r\n\r\nAa Bb Cc Dd Ee Ff", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_TABSTOP, 10, 295, 150, 90, hwnd, (HMENU)3, NULL, NULL);

            hBold = CreateWindowEx(0, "BUTTON", "Bold", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 10, 395, 70, 20, hwnd, (HMENU)4, NULL, NULL);
            hItalic = CreateWindowEx(0, "BUTTON", "Italic", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 80, 395, 70, 20, hwnd, (HMENU)5, NULL, NULL);
            
            hTabMetrics = CreateWindowEx(0, "BUTTON", "Metrics", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, 170, 10, 75, 20, hwnd, (HMENU)10, NULL, NULL);
            hTabGlyphs = CreateWindowEx(0, "BUTTON", "Glyphs", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 250, 10, 75, 20, hwnd, (HMENU)11, NULL, NULL);
            hTabDiag = CreateWindowEx(0, "BUTTON", "Diagnostics", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 330, 10, 95, 20, hwnd, (HMENU)12, NULL, NULL);
            hTabAnatomy = CreateWindowEx(0, "BUTTON", "Anatomy", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 430, 10, 85, 20, hwnd, (HMENU)13, NULL, NULL);
            hTabSample = CreateWindowEx(0, "BUTTON", "Sample", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 520, 10, 80, 20, hwnd, (HMENU)14, NULL, NULL);
            hCopyBtn = CreateWindowEx(0, "BUTTON", "Copy (C)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 610, 10, 80, 20, hwnd, (HMENU)16, NULL, NULL);
            hHelpBtn = CreateWindowEx(0, "BUTTON", "Help (H)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 700, 10, 80, 20, hwnd, (HMENU)15, NULL, NULL);
            
            SendMessage(hTabMetrics, BM_SETCHECK, BST_CHECKED, 0);

            WNDCLASS pc = {0};
            pc.lpfnWndProc = PanelProc;
            pc.hInstance = GetModuleHandle(NULL);
            pc.lpszClassName = "KFontPanel";
            hPanelBrush = CreateSolidBrush(RGB(255,255,255));
            pc.hbrBackground = hPanelBrush;
            RegisterClass(&pc);
            
            hPanel = CreateWindowEx(WS_EX_CLIENTEDGE, "KFontPanel", "", WS_CHILD | WS_VISIBLE, 170, 40, W - 190, H - 50, hwnd, NULL, NULL, NULL);
            
            hRangeList = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 10, 10, 250, 200, hPanel, (HMENU)20, NULL, NULL);
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
                ShowHelpDialog(hwnd);
            }
            else if (LOWORD(wParam) == 16 && HIWORD(wParam) == BN_CLICKED) {
                CopyAnatomyToClipboard(hwnd);
            }
            else if (LOWORD(wParam) >= 10 && LOWORD(wParam) <= 14 && HIWORD(wParam) == BN_CLICKED) {
                SelectTab(hwnd, LOWORD(wParam) - 10);
            }
            else if (LOWORD(wParam) == 6 && HIWORD(wParam) == EN_CHANGE) {
                WCHAR wText[32] = {0};
                GetWindowTextW(hAnatomyChar, wText, 32);
                if (wText[0]) {
                    if ((wText[0] == L'U' || wText[0] == L'u') && wText[1] == L'+') {
                        anatomyChar = (WCHAR)ParseHexW(wText + 2);
                    } else if (wText[0] == L'0' && (wText[1] == L'x' || wText[1] == L'X')) {
                        anatomyChar = (WCHAR)ParseHexW(wText + 2);
                    } else if (lstrlenW(wText) == 4 && ParseHexW(wText) != 0) {
                        anatomyChar = (WCHAR)ParseHexW(wText);
                    } else {
                        anatomyChar = wText[0];
                    }
                    if (currentTab == 3) InvalidateRect(hPanel, NULL, TRUE);
                }
            }
            else if ((LOWORD(wParam) == 1 && HIWORD(wParam) == LBN_SELCHANGE) ||
                (LOWORD(wParam) == 2 && HIWORD(wParam) == CBN_SELCHANGE) ||
                (LOWORD(wParam) == 3 && HIWORD(wParam) == EN_CHANGE) ||
                (LOWORD(wParam) == 4 && HIWORD(wParam) == BN_CLICKED) ||
                (LOWORD(wParam) == 5 && HIWORD(wParam) == BN_CLICKED)) {
                
                if (LOWORD(wParam) == 3 && HIWORD(wParam) == EN_CHANGE) {
                    GetWindowText(hCustomText, currentCustomText, 512);
                    if (currentTab == 4) InvalidateRect(hPanel, NULL, TRUE);
                    break;
                }
                
                UpdateFont(hwnd);
            }
            break;
        }
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORBTN: {
            HWND hChild = (HWND)lParam;
            if (hChild == hTabMetrics || hChild == hTabGlyphs || hChild == hTabDiag || hChild == hTabAnatomy || hChild == hTabSample || hChild == hBold || hChild == hItalic) {
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

    DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN;
    RECT rcWin = {0, 0, W, H};
    AdjustWindowRect(&rcWin, style, FALSE);
    HWND hwnd = CreateWindowEx(0, "KFontApp", "KFont (Press H or F1 for Help)", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN) {
            HWND hFocus = GetFocus();
            BOOL inEdit = (hFocus == hCustomText || hFocus == hAnatomyChar);
            if (msg.wParam == 'H' || msg.wParam == 'h' || msg.wParam == VK_F1) {
                if (!inEdit) {
                    ShowHelpDialog(hwnd);
                    continue;
                }
            } else if (!inEdit) {
                if (msg.wParam >= '1' && msg.wParam <= '5') {
                    SelectTab(hwnd, (int)(msg.wParam - '1'));
                    continue;
                } else if (msg.wParam == 'B' || msg.wParam == 'b') {
                    isBold = !isBold;
                    SendMessage(hBold, BM_SETCHECK, isBold ? BST_CHECKED : BST_UNCHECKED, 0);
                    UpdateFont(hwnd);
                    continue;
                } else if (msg.wParam == 'I' || msg.wParam == 'i') {
                    isItalic = !isItalic;
                    SendMessage(hItalic, BM_SETCHECK, isItalic ? BST_CHECKED : BST_UNCHECKED, 0);
                    UpdateFont(hwnd);
                    continue;
                } else if (msg.wParam == 'C' || msg.wParam == 'c') {
                    CopyAnatomyToClipboard(hwnd);
                    continue;
                }
            }
        }
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    ExitProcess(0);
}
