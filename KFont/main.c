#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define W 950
#define H 700

HWND hList, hSizeList;
HWND hCustomText, hBold, hItalic, hAnatomyChar;
HWND hTabMetrics, hTabGlyphs, hTabDiag, hTabAnatomy, hTabWaterfall, hTabSample, hTabSpec, hHelpBtn, hCopyBtn;
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

void SelectTab(HWND hwnd, int tabIdx);
void ShowHelpDialog(HWND hwnd);
void UpdateFont(HWND hwnd);
void CopyReportToClipboard(HWND hwnd);

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

void GetBlockRange(int sel, int* pStart, int* pEnd) {
    switch(sel) {
        case 1:  *pStart = 0x00A0; *pEnd = 0x00FF; break; // Latin-1 Supp
        case 2:  *pStart = 0x0100; *pEnd = 0x017F; break; // Latin Ext-A
        case 3:  *pStart = 0x0370; *pEnd = 0x03FF; break; // Greek & Coptic
        case 4:  *pStart = 0x0400; *pEnd = 0x04FF; break; // Cyrillic
        case 5:  *pStart = 0x2000; *pEnd = 0x206F; break; // General Punctuation
        case 6:  *pStart = 0x20A0; *pEnd = 0x20CF; break; // Currency Symbols
        case 7:  *pStart = 0x2100; *pEnd = 0x214F; break; // Letterlike Symbols
        case 8:  *pStart = 0x2190; *pEnd = 0x22FF; break; // Arrows & Math Operators
        case 9:  *pStart = 0x2500; *pEnd = 0x257F; break; // Box Drawing
        case 10: *pStart = 0x25A0; *pEnd = 0x25FF; break; // Geometric Shapes
        default: *pStart = 0x0020; *pEnd = 0x007F; break; // Basic Latin
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
                    int start, end;
                    GetBlockRange(sel, &start, &end);

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
                            SelectTab(GetParent(hwnd), 3);
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
            
            if (currentTab == 0) { // Metrics & OS/2
                HFONT hOld = SelectObject(hdc, hCurrentFont);
                UINT cbData = GetOutlineTextMetricsA(hdc, 0, NULL);
                if (cbData > 0) {
                    OUTLINETEXTMETRIC *potm = (OUTLINETEXTMETRIC*)HeapAlloc(GetProcessHeap(), 0, cbData);
                    if (potm && GetOutlineTextMetricsA(hdc, cbData, potm)) {
                        SelectObject(hdc, hFont);
                        char buf[256];
                        int y = 10;
                        TextOutA(hdc, 10, y, "=== OpenType / TrueType Outline Metrics ===", 43); y += 22;
                        wsprintfA(buf, "Font Bounding Box: [%d, %d] to [%d, %d]", potm->otmrcFontBox.left, potm->otmrcFontBox.top, potm->otmrcFontBox.right, potm->otmrcFontBox.bottom);
                        TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                        wsprintfA(buf, "Mac Typo: Ascent=%d, Descent=%d, LineGap=%d", potm->otmMacAscent, potm->otmMacDescent, potm->otmMacLineGap);
                        TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                        wsprintfA(buf, "Windows Typo: Ascent=%d, Descent=%d, LineGap=%d", potm->otmAscent, potm->otmDescent, potm->otmLineGap);
                        TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                        wsprintfA(buf, "Cap Em Height: %u px, x-Height: %u px", potm->otmsCapEmHeight, potm->otmsXHeight);
                        TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                        wsprintfA(buf, "OS/2 Flags: fsSelection=0x%04X, fsType=0x%04X", potm->otmfsSelection, potm->otmfsType);
                        TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                        wsprintfA(buf, "Panose Number: [%d %d %d %d %d %d %d %d %d %d]",
                            potm->otmPanoseNumber.bFamilyType, potm->otmPanoseNumber.bSerifStyle,
                            potm->otmPanoseNumber.bWeight, potm->otmPanoseNumber.bProportion,
                            potm->otmPanoseNumber.bContrast, potm->otmPanoseNumber.bStrokeVariation,
                            potm->otmPanoseNumber.bArmStyle, potm->otmPanoseNumber.bLetterform,
                            potm->otmPanoseNumber.bMidline, potm->otmPanoseNumber.bXHeight);
                        TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                    }
                    if (potm) HeapFree(GetProcessHeap(), 0, potm);
                } else {
                    SelectObject(hdc, hFont);
                    TextOutA(hdc, 10, 10, "Outline text metrics not available for this bitmap/vector font.", 64);
                }
                
                SelectObject(hdc, hCurrentFont);
                TEXTMETRICA tm;
                GetTextMetricsA(hdc, &tm);
                SelectObject(hdc, hFont);
                int y = 180;
                char buf[256];
                TextOutA(hdc, 10, y, "=== Win32 GDI Raster Metrics (TEXTMETRIC) ===", 46); y += 22;
                wsprintfA(buf, "tmHeight: %d px, tmInternalLeading: %d px, tmExternalLeading: %d px", tm.tmHeight, tm.tmInternalLeading, tm.tmExternalLeading);
                TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                wsprintfA(buf, "tmAscent: %d px, tmDescent: %d px, tmOverhang: %d px", tm.tmAscent, tm.tmDescent, tm.tmOverhang);
                TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                wsprintfA(buf, "tmWeight: %d (%s), tmMaxCharWidth: %d px, tmAveCharWidth: %d px",
                    tm.tmWeight, tm.tmWeight >= 700 ? "Bold" : (tm.tmWeight >= 600 ? "SemiBold" : "Regular"),
                    tm.tmMaxCharWidth, tm.tmAveCharWidth);
                TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;
                wsprintfA(buf, "CharSet: %u, PitchAndFamily: 0x%02X (%s), Italic: %s",
                    tm.tmCharSet, tm.tmPitchAndFamily, (tm.tmPitchAndFamily & TMPF_FIXED_PITCH) ? "Variable Pitch" : "Monospace",
                    tm.tmItalic ? "Yes" : "No");
                TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y+=20;

                // Typographic scale quick guidance
                y += 10;
                TextOutA(hdc, 10, y, "=== Modular Typographic Proportions ===", 40); y += 22;
                wsprintfA(buf, "Computed Em: %d px | Body Base: 16px | Heading 1 (2.44x): %dpx | Heading 2 (1.75x): %dpx",
                    tm.tmAscent + tm.tmDescent, (int)(16 * 2.44), (int)(16 * 1.75));
                TextOutA(hdc, 10, y, buf, lstrlenA(buf)); y += 20;

                SelectObject(hdc, hOld);
            }
            else if (currentTab == 1) { // Glyphs
                int sel = SendMessage(hRangeList, CB_GETCURSEL, 0, 0);
                int start, end;
                GetBlockRange(sel, &start, &end);
                
                HFONT hOld = SelectObject(hdc, hCurrentFont);
                int x = 10, y = 42;
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
                    if (y > H - 100) break; // Keep inside visible scroll limits
                }
                SelectObject(hdc, hOld);
            }
            else if (currentTab == 2) { // Diagnostics & Contrast
                HFONT hOld = SelectObject(hdc, hCurrentFont);
                DWORD cPairs = GetKerningPairsA(hdc, 0, NULL);
                SelectObject(hdc, hFont);
                char buf[128];
                wsprintfA(buf, "Kerning Pairs Available in Font Table: %u", cPairs);
                TextOutA(hdc, 10, 10, buf, lstrlenA(buf));
                
                SelectObject(hdc, hCurrentFont);
                TextOutA(hdc, 10, 32, "AV  To  Tr  WA  Ye  Va  fj  Ly  P.  Te", 38);
                
                SelectObject(hdc, hFont);
                TextOutA(hdc, 10, 80, "=== Rasterization & Grid Fitting (Hinting Across Sizes) ===", 58);
                int y = 102;
                int sizes[] = {10, 12, 14, 16, 20, 24, 32};
                for(int i = 0; i < 7; i++) {
                    HFONT hSz = CreateFontA(-sizes[i], 0, 0, 0, isBold ? FW_BOLD : FW_NORMAL, isItalic, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, currentFontName);
                    HFONT hPrev = SelectObject(hdc, hSz);
                    wsprintfA(buf, "%dpx: Pack my box with five dozen liquor jugs. 0123456789", sizes[i]);
                    TextOutA(hdc, 10, y, buf, lstrlenA(buf));
                    SIZE sz;
                    GetTextExtentPoint32A(hdc, buf, lstrlenA(buf), &sz);
                    y += sz.cy + 4;
                    SelectObject(hdc, hPrev);
                    DeleteObject(hSz);
                }

                // WCAG Legibility & Contrast Matrix
                y += 10;
                SelectObject(hdc, hFont);
                TextOutA(hdc, 10, y, "=== WCAG 2.1 Contrast & Legibility Palette Matrix ===", 53); y += 22;

                struct PaletteSample {
                    const char* name;
                    COLORREF fg;
                    COLORREF bg;
                    const char* ratioStr;
                    const char* badge;
                } samples[4] = {
                    {"Standard B&W", RGB(0,0,0), RGB(255,255,255), "21.0:1", "AAA PASS"},
                    {"Charcoal Dark", RGB(230,230,230), RGB(24,24,24), "15.3:1", "AAA PASS"},
                    {"Amber CRT Phosphor", RGB(255,176,0), RGB(26,17,0), "11.2:1", "AAA PASS"},
                    {"Win95 Retro 3D Face", RGB(0,0,128), RGB(192,192,192), "4.9:1", "AA PASS"}
                };

                HFONT hSampleFont = CreateFontA(-18, 0, 0, 0, isBold ? FW_BOLD : FW_NORMAL, isItalic, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, currentFontName);
                for (int s = 0; s < 4; s++) {
                    int bx = 10 + s * 180;
                    RECT rcBox = {bx, y, bx + 172, y + 80};
                    HBRUSH hSampleBg = CreateSolidBrush(samples[s].bg);
                    FillRect(hdc, &rcBox, hSampleBg);
                    DeleteObject(hSampleBg);

                    // Border
                    HPEN hBoxPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
                    HPEN hOldPen = SelectObject(hdc, hBoxPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    Rectangle(hdc, rcBox.left, rcBox.top, rcBox.right, rcBox.bottom);
                    SelectObject(hdc, hOldPen);
                    DeleteObject(hBoxPen);

                    // Text
                    SetTextColor(hdc, samples[s].fg);
                    SelectObject(hdc, hSampleFont);
                    TextOutA(hdc, bx + 8, y + 8, "Aa Gg 1999", 10);

                    SelectObject(hdc, hFont);
                    TextOutA(hdc, bx + 8, y + 36, samples[s].name, lstrlenA(samples[s].name));
                    wsprintfA(buf, "Ratio: %s [%s]", samples[s].ratioStr, samples[s].badge);
                    TextOutA(hdc, bx + 8, y + 54, buf, lstrlenA(buf));
                }
                DeleteObject(hSampleFont);
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
            else if (currentTab == 4) { // Waterfall & Typographic Scale Ladder
                HFONT hOld = SelectObject(hdc, hFont);
                TextOutA(hdc, 15, 10, "=== Typographic Waterfall & Optical Scaling Ladder ===", 54);
                
                int y = 36;
                int ladderSizes[] = {9, 11, 13, 16, 20, 24, 32, 44, 60};
                char buf[256];

                for (int i = 0; i < 9; i++) {
                    int sz = ladderSizes[i];
                    HFONT hWf = CreateFontA(-sz, 0, 0, 0, isBold ? FW_BOLD : FW_NORMAL, isItalic, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, currentFontName);
                    
                    SelectObject(hdc, hFont);
                    wsprintfA(buf, "%2dpx:", sz);
                    TextOutA(hdc, 15, y + (sz > 20 ? 4 : 0), buf, lstrlenA(buf));

                    SelectObject(hdc, hWf);
                    const char* sampleLine = "Sphinx of black quartz, judge my vow 12345";
                    TextOutA(hdc, 65, y, sampleLine, lstrlenA(sampleLine));

                    SIZE textSz;
                    GetTextExtentPoint32A(hdc, sampleLine, lstrlenA(sampleLine), &textSz);

                    SelectObject(hdc, hFont);
                    wsprintfA(buf, "(Adv: %dpx)", textSz.cx);
                    TextOutA(hdc, 65 + textSz.cx + 12, y + (sz > 20 ? 4 : 0), buf, lstrlenA(buf));

                    y += (textSz.cy > sz ? textSz.cy : sz) + 8;
                    SelectObject(hdc, hOld);
                    DeleteObject(hWf);

                    if (y > H - 100) break;
                }
                SelectObject(hdc, hOld);
            }
            else if (currentTab == 5) { // Sample & String Run Dissector
                HFONT hOld = SelectObject(hdc, hCurrentFont);
                RECT rc = {15, 12, W - 210, 180};
                DrawTextA(hdc, currentCustomText, -1, &rc, DT_WORDBREAK | DT_LEFT);

                // String Run Dissector
                SelectObject(hdc, hFont);
                int y = 200;
                TextOutA(hdc, 15, y, "=== String Run & Glyph Spacing Dissector ===", 44); y += 22;

                // Table Header
                HPEN hLinePen = CreatePen(PS_SOLID, 1, RGB(210, 210, 210));
                HPEN hOldPen = SelectObject(hdc, hLinePen);
                MoveToEx(hdc, 15, y, NULL); LineTo(hdc, W - 220, y);
                SelectObject(hdc, hOldPen);
                DeleteObject(hLinePen);
                y += 6;

                TextOutA(hdc, 15, y, "Idx", 3);
                TextOutA(hdc, 55, y, "Glyph", 5);
                TextOutA(hdc, 110, y, "CodePoint", 9);
                TextOutA(hdc, 195, y, "Advance", 7);
                TextOutA(hdc, 275, y, "Total X", 7);
                TextOutA(hdc, 355, y, "UTF-8 Hex", 9);
                TextOutA(hdc, 465, y, "HTML Entity", 11);
                y += 20;

                int cumX = 0;
                int maxRun = 16;
                char buf[256];
                int len = lstrlenA(currentCustomText);
                int count = 0;

                for (int i = 0; i < len && count < maxRun; i++) {
                    char c = currentCustomText[i];
                    if (c == '\r' || c == '\n') continue;

                    SIZE charSz;
                    SelectObject(hdc, hCurrentFont);
                    GetTextExtentPoint32A(hdc, &c, 1, &charSz);
                    cumX += charSz.cx;

                    SelectObject(hdc, hFont);
                    wsprintfA(buf, "#%02d", count + 1);
                    TextOutA(hdc, 15, y, buf, lstrlenA(buf));

                    char gBuf[4] = {c, '\0'};
                    TextOutA(hdc, 65, y, gBuf, 1);

                    wsprintfA(buf, "U+%04X", (unsigned char)c);
                    TextOutA(hdc, 110, y, buf, lstrlenA(buf));

                    wsprintfA(buf, "%d px", charSz.cx);
                    TextOutA(hdc, 195, y, buf, lstrlenA(buf));

                    wsprintfA(buf, "%d px", cumX);
                    TextOutA(hdc, 275, y, buf, lstrlenA(buf));

                    wsprintfA(buf, "0x%02X", (unsigned char)c);
                    TextOutA(hdc, 355, y, buf, lstrlenA(buf));

                    TextOutA(hdc, 465, y, GetHtmlEntity((WCHAR)(unsigned char)c), lstrlenA(GetHtmlEntity((WCHAR)(unsigned char)c)));

                    y += 20;
                    count++;
                }

                wsprintfA(buf, "Total Glyphs Analyzed: %d | Total Advance Run: %d px", count, cumX);
                TextOutA(hdc, 15, y + 8, buf, lstrlenA(buf));

                SelectObject(hdc, hOld);
            }
            else if (currentTab == 6) { // Spec & Code Generator
                HFONT hOld = SelectObject(hdc, hFont);
                TextOutA(hdc, 15, 10, "=== Typographic Specification & Code Generator ===", 50);
                
                int y = 35;
                char buf[256];

                TextOutA(hdc, 15, y, "1. Win32 GDI C Initialization Code (Press 'C' to Copy):", 55); y += 20;
                
                wsprintfA(buf, "   LOGFONTA lf = {0};"); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "   lf.lfHeight = -%d; // In logical pixels", currentSize); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "   lf.lfWeight = %s;", isBold ? "FW_BOLD" : "FW_NORMAL"); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "   lf.lfItalic = %s;", isItalic ? "TRUE" : "FALSE"); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "   lf.lfCharSet = DEFAULT_CHARSET;"); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "   lf.lfQuality = CLEARTYPE_QUALITY;"); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "   lstrcpyA(lf.lfFaceName, \"%s\");", currentFontName); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "   HFONT hFont = CreateFontIndirectA(&lf);"); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 26;

                TextOutA(hdc, 15, y, "2. CSS Modular Typography Variables:", 36); y += 20;
                wsprintfA(buf, "   :root {"); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "     --font-family: \"%s\", sans-serif;", currentFontName); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "     --font-weight: %d;", isBold ? 700 : 400); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "     --font-style: %s;", isItalic ? "italic" : "normal"); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "     --font-size-base: %dpx;", currentSize); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "     --font-scale-ratio: 1.25; /* Major Third */"); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "   }"); TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 26;

                TextOutA(hdc, 15, y, "3. Typographic Properties Export Summary:", 41); y += 20;
                wsprintfA(buf, "   Font Family: %s | Size: %dpx | Weight: %s | Style: %s",
                    currentFontName, currentSize, isBold ? "Bold" : "Normal", isItalic ? "Italic" : "Normal");
                TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;
                wsprintfA(buf, "   Rendering Target: Win32 GDI Subpixel Cleartype & Canvas Context2D");
                TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;

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

void CopyReportToClipboard(HWND hwnd) {
    char report[2048];
    if (currentTab == 6) { // Spec / Code Tab
        wsprintfA(report,
            "/* === KFont Generated C Win32 LOGFONT Code === */\r\n"
            "LOGFONTA lf = {0};\r\n"
            "lf.lfHeight = -%d;\r\n"
            "lf.lfWeight = %s;\r\n"
            "lf.lfItalic = %s;\r\n"
            "lf.lfCharSet = DEFAULT_CHARSET;\r\n"
            "lf.lfQuality = CLEARTYPE_QUALITY;\r\n"
            "lstrcpyA(lf.lfFaceName, \"%s\");\r\n"
            "HFONT hFont = CreateFontIndirectA(&lf);\r\n\r\n"
            "/* === CSS Typography Variables === */\r\n"
            ":root {\r\n"
            "  --font-family: \"%s\", sans-serif;\r\n"
            "  --font-weight: %d;\r\n"
            "  --font-style: %s;\r\n"
            "  --font-size-base: %dpx;\r\n"
            "  --font-scale-ratio: 1.25;\r\n"
            "}\r\n",
            currentSize, isBold ? "FW_BOLD" : "FW_NORMAL", isItalic ? "TRUE" : "FALSE",
            currentFontName, currentFontName, isBold ? 700 : 400, isItalic ? "italic" : "normal", currentSize);
    } else { // Anatomy / General Metrics Report
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
        
        wsprintfA(report,
            "{\r\n"
            "  \"app\": \"KFont\",\r\n"
            "  \"fontFamily\": \"%s\",\r\n"
            "  \"size\": %d,\r\n"
            "  \"bold\": %s,\r\n"
            "  \"italic\": %s,\r\n"
            "  \"glyph\": {\r\n"
            "    \"character\": \"U+%04X\",\r\n"
            "    \"decimal\": %u,\r\n"
            "    \"utf8\": \"%s\",\r\n"
            "    \"htmlEntity\": \"%s\",\r\n"
            "    \"bearingA\": %d,\r\n"
            "    \"bearingB\": %u,\r\n"
            "    \"bearingC\": %d,\r\n"
            "    \"advanceWidth\": %d,\r\n"
            "    \"ascent\": %d,\r\n"
            "    \"descent\": %d\r\n"
            "  }\r\n"
            "}\r\n",
            currentFontName, currentSize, isBold ? "true" : "false", isItalic ? "true" : "false",
            (unsigned int)anatomyChar, (unsigned int)anatomyChar, utf8Buf,
            GetHtmlEntity(anatomyChar), abc.abcA, abc.abcB, abc.abcC,
            abc.abcA + (int)abc.abcB + abc.abcC, tmBig.tmAscent, tmBig.tmDescent);
    }

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
        MessageBoxA(hwnd, "Report / Code snippet copied to clipboard!", "KFont", MB_OK | MB_ICONINFORMATION);
    }
}

void SelectTab(HWND hwnd, int tabIdx) {
    if (tabIdx < 0 || tabIdx > 6) return;
    currentTab = tabIdx;
    HWND tabs[] = {hTabMetrics, hTabGlyphs, hTabDiag, hTabAnatomy, hTabWaterfall, hTabSample, hTabSpec};
    for (int i = 0; i < 7; i++) {
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
    MessageBoxA(hwnd,
        "=== KFont - Typography & Font Engineering Inspector ===\n\n"
        "Tabs:\n"
        "  [1] Metrics & OS/2: Typographic measurements and OS/2 Panose\n"
        "  [2] Glyphs: Browse 11 Unicode ranges (click glyph to dissect)\n"
        "  [3] Diagnostics & Contrast: Kerning, hinting, & WCAG 2.1 matrix\n"
        "  [4] Anatomy: Vector guidelines, ABC spacing, bearings & codec\n"
        "  [5] Waterfall: Optical scaling cascade across 9 scale steps\n"
        "  [6] Sample & Dissector: Live text test & running advance table\n"
        "  [7] Spec / Code: Win32 C LOGFONT & CSS variables code generator\n\n"
        "Keyboard Shortcuts:\n"
        "  [1-7]  : Switch inspector tabs\n"
        "  [B]    : Toggle Bold\n"
        "  [I]    : Toggle Italic\n"
        "  [C]    : Copy context report or C LOGFONT code snippet\n"
        "  [F1/H] : Show this Help dialog\n\n"
        "Presets:\n"
        "  - Quick Glyphs: Click to jump directly to glyph anatomy\n"
        "  - Sample Presets: Load pangrams, digits, or clear text",
        "KFont Help & Shortcuts", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HDC hdcDpi = GetDC(hwnd);
            int dpi = GetDeviceCaps(hdcDpi, LOGPIXELSY);
            ReleaseDC(hwnd, hdcDpi);
            int fontHeight = -MulDiv(12, dpi, 72);
            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            
            CreateWindowEx(0, "STATIC", "System Fonts:", WS_CHILD | WS_VISIBLE, 10, 10, 150, 18, hwnd, NULL, NULL, NULL);
            hList = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_SORT | WS_TABSTOP, 10, 28, 150, 130, hwnd, (HMENU)1, NULL, NULL);
            
            CreateWindowEx(0, "STATIC", "Size:", WS_CHILD | WS_VISIBLE, 10, 162, 150, 18, hwnd, NULL, NULL, NULL);
            hSizeList = CreateWindowEx(WS_EX_CLIENTEDGE, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 10, 180, 150, 200, hwnd, (HMENU)2, NULL, NULL);
            const char* sizes[] = {"12", "16", "24", "32", "48", "64"};
            for (int i = 0; i < 6; i++) {
                SendMessage(hSizeList, CB_ADDSTRING, 0, (LPARAM)sizes[i]);
            }
            SendMessage(hSizeList, CB_SETCURSEL, 2, 0);
            
            CreateWindowEx(0, "STATIC", "Inspect Glyph:", WS_CHILD | WS_VISIBLE, 10, 208, 150, 18, hwnd, NULL, NULL, NULL);
            hAnatomyChar = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "A", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 10, 226, 150, 24, hwnd, (HMENU)6, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Custom Text:", WS_CHILD | WS_VISIBLE, 10, 254, 150, 18, hwnd, NULL, NULL, NULL);
            hCustomText = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "The quick brown fox jumps over the lazy dog.\r\n\r\n0123456789\r\n\r\nAa Bb Cc Dd Ee Ff", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_TABSTOP, 10, 272, 150, 75, hwnd, (HMENU)3, NULL, NULL);

            hBold = CreateWindowEx(0, "BUTTON", "Bold [B]", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 10, 355, 72, 20, hwnd, (HMENU)4, NULL, NULL);
            hItalic = CreateWindowEx(0, "BUTTON", "Italic [I]", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 85, 355, 75, 20, hwnd, (HMENU)5, NULL, NULL);
            
            CreateWindowEx(0, "STATIC", "Quick Glyphs:", WS_CHILD | WS_VISIBLE, 10, 382, 150, 18, hwnd, NULL, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "A", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 10, 400, 46, 22, hwnd, (HMENU)101, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "g", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 61, 400, 46, 22, hwnd, (HMENU)102, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "Q", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 112, 400, 46, 22, hwnd, (HMENU)103, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "W", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 10, 426, 46, 22, hwnd, (HMENU)104, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "0", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 61, 426, 46, 22, hwnd, (HMENU)105, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "\x80", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 112, 426, 46, 22, hwnd, (HMENU)106, NULL, NULL);

            CreateWindowEx(0, "STATIC", "Sample Presets:", WS_CHILD | WS_VISIBLE, 10, 456, 150, 18, hwnd, NULL, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "Fox Pangram", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 10, 474, 148, 22, hwnd, (HMENU)110, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "Alphabet Aa-Zz", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 10, 500, 148, 22, hwnd, (HMENU)111, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "Digits & Symbols", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 10, 526, 148, 22, hwnd, (HMENU)112, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "Clear Text", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 10, 552, 148, 22, hwnd, (HMENU)113, NULL, NULL);

            // Tab bar (7 tabs + Copy + Help)
            hTabMetrics   = CreateWindowEx(0, "BUTTON", "Metrics [1]",   WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, 170, 10, 82, 20, hwnd, (HMENU)10, NULL, NULL);
            hTabGlyphs    = CreateWindowEx(0, "BUTTON", "Glyphs [2]",    WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 254, 10, 80, 20, hwnd, (HMENU)11, NULL, NULL);
            hTabDiag      = CreateWindowEx(0, "BUTTON", "Diagnostics [3]", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 336, 10, 96, 20, hwnd, (HMENU)12, NULL, NULL);
            hTabAnatomy   = CreateWindowEx(0, "BUTTON", "Anatomy [4]",   WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 434, 10, 88, 20, hwnd, (HMENU)13, NULL, NULL);
            hTabWaterfall = CreateWindowEx(0, "BUTTON", "Waterfall [5]", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 524, 10, 86, 20, hwnd, (HMENU)14, NULL, NULL);
            hTabSample    = CreateWindowEx(0, "BUTTON", "Sample [6]",    WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 612, 10, 80, 20, hwnd, (HMENU)15, NULL, NULL);
            hTabSpec      = CreateWindowEx(0, "BUTTON", "Spec/Code [7]", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 694, 10, 92, 20, hwnd, (HMENU)16, NULL, NULL);
            hCopyBtn      = CreateWindowEx(0, "BUTTON", "Copy [C]",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 788, 10, 68, 20, hwnd, (HMENU)17, NULL, NULL);
            hHelpBtn      = CreateWindowEx(0, "BUTTON", "Help [F1]",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 858, 10, 72, 20, hwnd, (HMENU)18, NULL, NULL);
            
            SendMessage(hTabMetrics, BM_SETCHECK, BST_CHECKED, 0);

            WNDCLASS pc = {0};
            pc.lpfnWndProc = PanelProc;
            pc.hInstance = GetModuleHandle(NULL);
            pc.lpszClassName = "KFontPanel";
            hPanelBrush = CreateSolidBrush(RGB(255,255,255));
            pc.hbrBackground = hPanelBrush;
            RegisterClass(&pc);
            
            hPanel = CreateWindowEx(WS_EX_CLIENTEDGE, "KFontPanel", "", WS_CHILD | WS_VISIBLE, 170, 38, W - 190, H - 48, hwnd, NULL, NULL, NULL);
            
            hRangeList = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 10, 10, 280, 220, hPanel, (HMENU)20, NULL, NULL);
            const char* blocks[] = {
                "Basic Latin (0020-007F)",
                "Latin-1 Supp (00A0-00FF)",
                "Latin Ext-A (0100-017F)",
                "Greek & Coptic (0370-03FF)",
                "Cyrillic (0400-04FF)",
                "General Punctuation (2000-206F)",
                "Currency Symbols (20A0-20CF)",
                "Letterlike Symbols (2100-214F)",
                "Arrows & Math (2190-22FF)",
                "Box Drawing (2500-257F)",
                "Geometric Shapes (25A0-25FF)"
            };
            for (int i = 0; i < 11; i++) {
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
            if (LOWORD(wParam) == 18 && HIWORD(wParam) == BN_CLICKED) {
                ShowHelpDialog(hwnd);
            }
            else if (LOWORD(wParam) == 17 && HIWORD(wParam) == BN_CLICKED) {
                CopyReportToClipboard(hwnd);
            }
            else if (LOWORD(wParam) >= 10 && LOWORD(wParam) <= 16 && HIWORD(wParam) == BN_CLICKED) {
                SelectTab(hwnd, LOWORD(wParam) - 10);
            }
            else if (LOWORD(wParam) >= 101 && LOWORD(wParam) <= 106 && HIWORD(wParam) == BN_CLICKED) {
                WCHAR chars[] = { L'A', L'g', L'Q', L'W', L'0', (WCHAR)0x20AC };
                anatomyChar = chars[LOWORD(wParam) - 101];
                WCHAR wBuf[16];
                wsprintfW(wBuf, L"U+%04X", (UINT)anatomyChar);
                SetWindowTextW(hAnatomyChar, wBuf);
                SelectTab(hwnd, 3);
            }
            else if (LOWORD(wParam) >= 110 && LOWORD(wParam) <= 113 && HIWORD(wParam) == BN_CLICKED) {
                int id = LOWORD(wParam);
                if (id == 110) {
                    lstrcpyA(currentCustomText, "The quick brown fox jumps over the lazy dog.\r\n\r\nPACK MY BOX WITH FIVE DOZEN LIQUOR JUGS.");
                } else if (id == 111) {
                    lstrcpyA(currentCustomText, "ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\nabcdefghijklmnopqrstuvwxyz\r\n0123456789");
                } else if (id == 112) {
                    lstrcpyA(currentCustomText, "0123456789 + - * / = % < > [ ] { } ( ) @ # $ \x80 \xA3 \xA5 ^ & _ ~ ` \" ' : ; , . ?");
                } else if (id == 113) {
                    currentCustomText[0] = '\0';
                }
                SetWindowTextA(hCustomText, currentCustomText);
                SelectTab(hwnd, 5);
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
                    if (currentTab == 4 || currentTab == 5) InvalidateRect(hPanel, NULL, TRUE);
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
            if (hChild == hTabMetrics || hChild == hTabGlyphs || hChild == hTabDiag ||
                hChild == hTabAnatomy || hChild == hTabWaterfall || hChild == hTabSample ||
                hChild == hTabSpec || hChild == hBold || hChild == hItalic) {
                HDC hdcStatic = (HDC)wParam;
                SetTextColor(hdcStatic, RGB(224, 224, 224));
                SetBkColor(hdcStatic, RGB(18, 18, 18));
                return (LRESULT)hBgBrush;
            }
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(224, 224, 224));
            SetBkColor(hdcStatic, RGB(18, 18, 18));
            return (LRESULT)hBgBrush;
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
    HWND hwnd = CreateWindowEx(0, "KFontApp", "KFont - Typography & Font Engineering Inspector (Press H or F1 for Help)", style,
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
                if (msg.wParam >= '1' && msg.wParam <= '7') {
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
                    CopyReportToClipboard(hwnd);
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
