#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define W 950
#define H 700

HWND hList, hSizeList;
HWND hCustomText, hBold, hItalic, hAnatomyChar;
HWND hTabMetrics, hTabGlyphs, hTabDiag, hTabAnatomy, hTabWaterfall, hTabSample, hTabSpec, hTabEditor, hHelpBtn, hCopyBtn;
HWND hPanel, hRangeList;
HWND hEditorCode;
HWND hBtnEdPrev, hBtnEdNext, hBtnEdRasterize, hBtnEdClear, hBtnEdInvert, hBtnEdFmt, hBtnEdCopy, hBtnEdExport;

unsigned char edChar = 'A';
int edFormat = 0; // 0 = BDF 2.1, 1 = TTF TTX 3.0, 2 = C ROM Table
unsigned char edBank[95][16];
BOOL edBankInitialized = FALSE;

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
void RasterizeGlyph(HDC hdcRef, unsigned char c, unsigned char* outRows);
void InitEditorBank(HDC hdcRef);
void GenerateEditorExport(char* outBuf, int maxLen, int format);
void UpdateEditorCode(void);
void UpdateEditorFormatBtnText(void);
void SaveEditorExportToFile(HWND hwnd);
void CopyEditorCodeToClipboard(HWND hwnd);

static void AppendStr(char* dest, int* pOffset, const char* src) {
    while (*src) {
        dest[(*pOffset)++] = *src++;
    }
    dest[*pOffset] = '\0';
}

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

void RasterizeGlyph(HDC hdcRef, unsigned char c, unsigned char* outRows) {
    for (int i = 0; i < 16; i++) outRows[i] = 0;
    HDC hMemDC = CreateCompatibleDC(hdcRef);
    HBITMAP hBmp = CreateBitmap(8, 16, 1, 1, NULL);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hBmp);
    HFONT hRasterFont = CreateFontA(-14, 0, 0, 0, isBold ? FW_BOLD : FW_NORMAL, isItalic, 0, 0, DEFAULT_CHARSET, 0, 0, NONANTIALIASED_QUALITY, DEFAULT_PITCH, currentFontName);
    HFONT hOldFont = (HFONT)SelectObject(hMemDC, hRasterFont);

    RECT rc = {0, 0, 8, 16};
    FillRect(hMemDC, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
    SetTextColor(hMemDC, RGB(255, 255, 255));
    SetBkColor(hMemDC, RGB(0, 0, 0));
    SetBkMode(hMemDC, OPAQUE);

    char chStr[2] = {(char)c, 0};
    TextOutA(hMemDC, 0, 0, chStr, 1);

    for (int y = 0; y < 16; y++) {
        unsigned char byteVal = 0;
        for (int x = 0; x < 8; x++) {
            COLORREF pix = GetPixel(hMemDC, x, y);
            if (pix != RGB(0, 0, 0)) {
                byteVal |= (unsigned char)(1 << (7 - x));
            }
        }
        outRows[y] = byteVal;
    }

    SelectObject(hMemDC, hOldFont);
    DeleteObject(hRasterFont);
    SelectObject(hMemDC, hOldBmp);
    DeleteObject(hBmp);
    DeleteDC(hMemDC);
}

void InitEditorBank(HDC hdcRef) {
    if (edBankInitialized) return;
    for (int c = 32; c <= 126; c++) {
        RasterizeGlyph(hdcRef, (unsigned char)c, edBank[c - 32]);
    }
    edBankInitialized = TRUE;
}

void GenerateEditorExport(char* outBuf, int maxLen, int format) {
    int off = 0;
    outBuf[0] = '\0';
    char chunk[256];

    if (format == 0) { // Adobe BDF 2.1
        wsprintfA(chunk, "STARTFONT 2.1\r\nFONT -KiloApps-%s-Medium-R-Normal--16-160-75-75-C-80-ISO8859-1\r\n", currentFontName);
        AppendStr(outBuf, &off, chunk);
        AppendStr(outBuf, &off, "SIZE 16 75 75\r\nFONTBOUNDINGBOX 8 16 0 -2\r\nSTARTPROPERTIES 2\r\nFONT_ASCENT 14\r\nFONT_DESCENT 2\r\nENDPROPERTIES\r\nCHARS 95\r\n");

        for (int c = 32; c <= 126; c++) {
            wsprintfA(chunk, "STARTCHAR char_%02X\r\nENCODING %d\r\nSWIDTH 1000 0\r\nDWIDTH 8 0\r\nBBX 8 16 0 -2\r\nBITMAP\r\n", c, c);
            AppendStr(outBuf, &off, chunk);
            for (int r = 0; r < 16; r++) {
                wsprintfA(chunk, "%02X\r\n", (UINT)edBank[c - 32][r]);
                AppendStr(outBuf, &off, chunk);
            }
            AppendStr(outBuf, &off, "ENDCHAR\r\n");
        }
        AppendStr(outBuf, &off, "ENDFONT\r\n");
    } else if (format == 1) { // TrueType TTX 3.0 XML
        wsprintfA(chunk, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<!-- TrueType / OpenType TTX 3.0 Font Extract (%s) -->\r\n<TTFont sfntVersion=\"\\x00\\x01\\x00\\x00\" ttLibVersion=\"3.0\">\r\n", currentFontName);
        AppendStr(outBuf, &off, chunk);
        AppendStr(outBuf, &off, "  <GlyphOrder>\r\n    <GlyphID id=\"0\" name=\".notdef\"/>\r\n    <GlyphID id=\"1\" name=\"space\"/>\r\n");
        for (int c = 33; c <= 126; c++) {
            wsprintfA(chunk, "    <GlyphID id=\"%d\" name=\"glyph%04X\"/>\r\n", c - 31, c);
            AppendStr(outBuf, &off, chunk);
        }
        AppendStr(outBuf, &off, "  </GlyphOrder>\r\n  <head>\r\n    <tableVersion value=\"1.0\"/>\r\n    <fontRevision value=\"1.0\"/>\r\n    <unitsPerEm value=\"1000\"/>\r\n    <xMin value=\"0\" yMin=\"-200\" xMax=\"800\" yMax=\"800\"/>\r\n  </head>\r\n  <hhea>\r\n    <ascent value=\"800\" descent=\"-200\" lineGap=\"90\"/>\r\n    <advanceWidthMax value=\"800\" numberOfHMetrics=\"95\"/>\r\n  </hhea>\r\n  <glyf>\r\n");
        wsprintfA(chunk, "    <!-- Active Selected Glyph: '%c' (U+%04X) Matrix Vector -->\r\n    <TTGlyph name=\"glyph%04X\" xMin=\"0\" yMin=\"-200\" xMax=\"800\" yMax=\"800\">\r\n", (edChar >= 32 && edChar <= 126) ? edChar : ' ', (UINT)edChar, (UINT)edChar);
        AppendStr(outBuf, &off, chunk);
        for (int r = 0; r < 16; r++) {
            for (int col = 0; col < 8; col++) {
                if ((edBank[edChar - 32][r] >> (7 - col)) & 1) {
                    int x0 = col * 100;
                    int y0 = 800 - (r + 1) * 62;
                    wsprintfA(chunk, "      <contour><pt x=\"%d\" y=\"%d\" on=\"1\"/><pt x=\"%d\" y=\"%d\" on=\"1\"/></contour>\r\n", x0, y0, x0 + 100, y0 + 62);
                    AppendStr(outBuf, &off, chunk);
                }
            }
        }
        AppendStr(outBuf, &off, "    </TTGlyph>\r\n  </glyf>\r\n</TTFont>\r\n");
    } else { // C Header ROM Table
        wsprintfA(chunk, "/* === KFont 8x16 Bitmap Font ROM Table === */\r\n/* Font: %s | Size: 16px | 95 ASCII Glyphs (32..126) */\r\n\r\nconst unsigned char font_rom_8x16[95][16] = {\r\n", currentFontName);
        AppendStr(outBuf, &off, chunk);
        for (int c = 32; c <= 126; c++) {
            char chDisp = (c >= 32 && c <= 126 && c != '\\' && c != '\'') ? (char)c : ' ';
            wsprintfA(chunk, "  /* [%3d '%c'] */ { ", c, chDisp);
            AppendStr(outBuf, &off, chunk);
            for (int r = 0; r < 16; r++) {
                wsprintfA(chunk, "0x%02X%s", (UINT)edBank[c - 32][r], (r == 15) ? "" : ", ");
                AppendStr(outBuf, &off, chunk);
            }
            wsprintfA(chunk, " }%s\r\n", (c == 126) ? "" : ",");
            AppendStr(outBuf, &off, chunk);
        }
        AppendStr(outBuf, &off, "};\r\n");
    }
}

void UpdateEditorFormatBtnText(void) {
    if (!hBtnEdFmt) return;
    const char* txt = (edFormat == 0) ? "Fmt: BDF 2.1" : (edFormat == 1 ? "Fmt: TTF TTX" : "Fmt: C Table");
    SetWindowTextA(hBtnEdFmt, txt);
}

void UpdateEditorCode(void) {
    if (!hEditorCode) return;
    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 65536);
    if (buf) {
        GenerateEditorExport(buf, 65536, edFormat);
        SetWindowTextA(hEditorCode, buf);
        HeapFree(GetProcessHeap(), 0, buf);
    }
}

void SaveEditorExportToFile(HWND hwnd) {
    const char* filename = (edFormat == 0) ? "kfont_export.bdf" : (edFormat == 1 ? "kfont_export.ttx" : "kfont_export.h");
    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 65536);
    if (!buf) return;
    GenerateEditorExport(buf, 65536, edFormat);
    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, buf, lstrlenA(buf), &written, NULL);
        CloseHandle(hFile);
        char msg[128];
        wsprintfA(msg, "Font successfully exported to %s!", filename);
        MessageBoxA(hwnd, msg, "KFont Export", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(hwnd, "Failed to create export file.", "KFont Error", MB_OK | MB_ICONERROR);
    }
    HeapFree(GetProcessHeap(), 0, buf);
}

void CopyEditorCodeToClipboard(HWND hwnd) {
    int len = GetWindowTextLengthA(hEditorCode);
    if (len <= 0) return;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len + 1);
    if (hMem) {
        char* pMem = (char*)GlobalLock(hMem);
        if (pMem) {
            GetWindowTextA(hEditorCode, pMem, len + 1);
            GlobalUnlock(hMem);
            if (OpenClipboard(hwnd)) {
                EmptyClipboard();
                SetClipboardData(CF_TEXT, hMem);
                CloseClipboard();
                MessageBoxA(hwnd, "Export code copied to clipboard!", "KFont", MB_OK | MB_ICONINFORMATION);
                return;
            }
        }
        GlobalFree(hMem);
    }
}

LRESULT CALLBACK PanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == 20 && HIWORD(wParam) == CBN_SELCHANGE) {
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 201) { // Prev glyph
                if (edChar > 32) edChar--;
                else edChar = 126;
                UpdateEditorCode();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 202) { // Next glyph
                if (edChar < 126) edChar++;
                else edChar = 32;
                UpdateEditorCode();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 203) { // Rasterize from font
                HDC hdcRef = GetDC(hwnd);
                RasterizeGlyph(hdcRef, edChar, edBank[edChar - 32]);
                ReleaseDC(hwnd, hdcRef);
                UpdateEditorCode();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 204) { // Clear
                for (int i = 0; i < 16; i++) edBank[edChar - 32][i] = 0;
                UpdateEditorCode();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 205) { // Invert
                for (int i = 0; i < 16; i++) edBank[edChar - 32][i] = (unsigned char)~edBank[edChar - 32][i];
                UpdateEditorCode();
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == 206) { // Toggle format: BDF -> TTF -> C -> BDF
                edFormat = (edFormat + 1) % 3;
                UpdateEditorFormatBtnText();
                UpdateEditorCode();
            } else if (LOWORD(wParam) == 207) { // Copy Code
                CopyEditorCodeToClipboard(hwnd);
            } else if (LOWORD(wParam) == 208) { // Save Export File
                SaveEditorExportToFile(hwnd);
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
            } else if (currentTab == 7) { // Font Editor & BDF/TTF Studio
                int mx = LOWORD(lParam);
                int my = HIWORD(lParam);
                // 8x16 Grid: x=15, y=50, 8 cols x 16 rows, 16x16 px cells
                if (mx >= 15 && mx < 15 + 8 * 16 && my >= 50 && my < 50 + 16 * 16) {
                    int col = (mx - 15) / 16;
                    int row = (my - 50) / 16;
                    if (col >= 0 && col < 8 && row >= 0 && row < 16) {
                        edBank[edChar - 32][row] ^= (unsigned char)(1 << (7 - col));
                        UpdateEditorCode();
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
                // ASCII Palette: x=15, y=410, 16 cols x 6 rows, 17x18 px cells
                else if (mx >= 15 && mx < 15 + 16 * 17 && my >= 410 && my < 410 + 6 * 18) {
                    int col = (mx - 15) / 17;
                    int row = (my - 410) / 18;
                    int idx = row * 16 + col;
                    if (idx >= 0 && idx < 95) {
                        edChar = (unsigned char)(32 + idx);
                        UpdateEditorCode();
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            }
            break;
        }
        case WM_RBUTTONDOWN: {
            if (currentTab == 7) {
                int mx = LOWORD(lParam);
                int my = HIWORD(lParam);
                if (mx >= 15 && mx < 15 + 8 * 16 && my >= 50 && my < 50 + 16 * 16) {
                    int col = (mx - 15) / 16;
                    int row = (my - 50) / 16;
                    if (col >= 0 && col < 8 && row >= 0 && row < 16) {
                        edBank[edChar - 32][row] &= (unsigned char)~(1 << (7 - col));
                        UpdateEditorCode();
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
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
                    {"Win95 Retro 3D Face", RGB(0,0,128), RGB(192,192,192), "8.8:1", "AAA PASS"}
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
                TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 24;

                TextOutA(hdc, 15, y, "4. 1-Bit Embedded ROM C Array (8x8 Glyph Snippet):", 50); y += 20;
                wsprintfA(buf, "   const uint8_t glyph_8x8[8] = { 0x3C, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C };");
                TextOutA(hdc, 15, y, buf, lstrlenA(buf)); y += 18;

                SelectObject(hdc, hOld);
            }
            else if (currentTab == 7) { // Font Editor & BDF/TTF Studio
                SelectObject(hdc, hFont);
                char buf[128];
                wsprintfA(buf, "Active Glyph: '%c' (ASCII %d / 0x%02X)", (edChar >= 32 && edChar <= 126) ? edChar : ' ', (UINT)edChar, (UINT)edChar);
                SetTextColor(hdc, RGB(0, 102, 204));
                TextOutA(hdc, 15, 8, buf, lstrlenA(buf));
                SetTextColor(hdc, RGB(0, 0, 0));

                TextOutA(hdc, 15, 34, "Pixel Grid (L-Click Toggle / R-Click Clear):", 44);
                HBRUSH hCellOn = CreateSolidBrush(RGB(14, 165, 233));
                HBRUSH hCellOff = CreateSolidBrush(RGB(241, 245, 249));
                HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(203, 213, 225));
                HPEN hOldPen = (HPEN)SelectObject(hdc, hGridPen);

                for (int r = 0; r < 16; r++) {
                    unsigned char byteVal = edBank[edChar - 32][r];
                    for (int c = 0; c < 8; c++) {
                        BOOL on = (byteVal >> (7 - c)) & 1;
                        RECT rcCell = { 15 + c * 16, 50 + r * 16, 15 + (c + 1) * 16, 50 + (r + 1) * 16 };
                        FillRect(hdc, &rcCell, on ? hCellOn : hCellOff);
                        MoveToEx(hdc, rcCell.left, rcCell.top, NULL);
                        LineTo(hdc, rcCell.right, rcCell.top);
                        LineTo(hdc, rcCell.right, rcCell.bottom);
                        LineTo(hdc, rcCell.left, rcCell.bottom);
                        LineTo(hdc, rcCell.left, rcCell.top);
                    }
                }
                DeleteObject(hCellOn);
                DeleteObject(hCellOff);

                // Previews at x=160
                int px = 160;
                TextOutA(hdc, px, 50, "1x Preview:", 11);
                for (int r = 0; r < 16; r++) {
                    unsigned char b = edBank[edChar - 32][r];
                    for (int c = 0; c < 8; c++) {
                        if ((b >> (7 - c)) & 1) SetPixel(hdc, px + c, 68 + r, RGB(0, 0, 0));
                    }
                }

                TextOutA(hdc, px, 95, "2x Preview:", 11);
                HBRUSH hBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
                for (int r = 0; r < 16; r++) {
                    unsigned char b = edBank[edChar - 32][r];
                    for (int c = 0; c < 8; c++) {
                        if ((b >> (7 - c)) & 1) {
                            RECT r2 = { px + c * 2, 115 + r * 2, px + (c + 1) * 2, 115 + (r + 1) * 2 };
                            FillRect(hdc, &r2, hBlack);
                        }
                    }
                }

                TextOutA(hdc, px, 155, "4x Preview:", 11);
                for (int r = 0; r < 16; r++) {
                    unsigned char b = edBank[edChar - 32][r];
                    for (int c = 0; c < 8; c++) {
                        if ((b >> (7 - c)) & 1) {
                            RECT r4 = { px + c * 4, 175 + r * 4, px + (c + 1) * 4, 175 + (r + 1) * 4 };
                            FillRect(hdc, &r4, hBlack);
                        }
                    }
                }

                TextOutA(hdc, 15, 318, "Live String Preview (Banked 8x16 Bitmap Font):", 46);
                const char* sampleStr = "THE QUICK BROWN FOX 123";
                int sx = 15, sy = 338;
                for (int i = 0; sampleStr[i]; i++) {
                    unsigned char ch = (unsigned char)sampleStr[i];
                    if (ch >= 32 && ch <= 126) {
                        for (int r = 0; r < 16; r++) {
                            unsigned char b = edBank[ch - 32][r];
                            for (int c = 0; c < 8; c++) {
                                if ((b >> (7 - c)) & 1) {
                                    SetPixel(hdc, sx + c, sy + r, RGB(15, 23, 42));
                                }
                            }
                        }
                    }
                    sx += 9;
                    if (sx > 275) break;
                }

                TextOutA(hdc, 15, 390, "ASCII Bank (Click glyph to edit):", 33);
                HBRUSH hSelBrush = CreateSolidBrush(RGB(186, 230, 253));
                for (int idx = 0; idx < 95; idx++) {
                    unsigned char ch = (unsigned char)(32 + idx);
                    int col = idx % 16;
                    int row = idx / 16;
                    int gx = 15 + col * 17;
                    int gy = 410 + row * 18;
                    RECT rcG = { gx, gy, gx + 17, gy + 18 };
                    if (ch == edChar) {
                        FillRect(hdc, &rcG, hSelBrush);
                    }
                    char s[2] = { (char)ch, 0 };
                    TextOutA(hdc, gx + 4, gy + 1, s, 1);
                    MoveToEx(hdc, rcG.left, rcG.top, NULL);
                    LineTo(hdc, rcG.right, rcG.top);
                    LineTo(hdc, rcG.right, rcG.bottom);
                    LineTo(hdc, rcG.left, rcG.bottom);
                    LineTo(hdc, rcG.left, rcG.top);
                }
                DeleteObject(hSelBrush);

                SelectObject(hdc, hOldPen);
                DeleteObject(hGridPen);
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
    if (currentTab == 7) {
        CopyEditorCodeToClipboard(hwnd);
        return;
    }
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
    if (tabIdx < 0 || tabIdx > 7) return;
    currentTab = tabIdx;
    HWND tabs[] = {hTabMetrics, hTabGlyphs, hTabDiag, hTabAnatomy, hTabWaterfall, hTabSample, hTabSpec, hTabEditor};
    for (int i = 0; i < 8; i++) {
        SendMessage(tabs[i], BM_SETCHECK, (i == currentTab) ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    ShowWindow(hRangeList, currentTab == 1 ? SW_SHOW : SW_HIDE);
    int swEd = (currentTab == 7) ? SW_SHOW : SW_HIDE;
    ShowWindow(hEditorCode, swEd);
    ShowWindow(hBtnEdPrev, swEd);
    ShowWindow(hBtnEdNext, swEd);
    ShowWindow(hBtnEdRasterize, swEd);
    ShowWindow(hBtnEdClear, swEd);
    ShowWindow(hBtnEdInvert, swEd);
    ShowWindow(hBtnEdFmt, swEd);
    ShowWindow(hBtnEdCopy, swEd);
    ShowWindow(hBtnEdExport, swEd);
    if (currentTab == 7) {
        UpdateEditorFormatBtnText();
        UpdateEditorCode();
    }
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

#define QUICKSAVE_FILE "kfont_quicksave.dat"
#define TUTORIAL_FILE  "kfont_tutorial.dat"

typedef struct {
    char fontName[64];
    int size;
    BOOL bold;
    BOOL italic;
    int tab;
    WCHAR anatomyChar;
    char customText[512];
    int rangeIndex;
    unsigned char edChar;
    int edFormat;
    unsigned char edBank[95][16];
} KFontSaveData;

void QuicksaveNative(HWND hwnd) {
    KFontSaveData data = {0};
    lstrcpynA(data.fontName, currentFontName, sizeof(data.fontName));
    data.size = currentSize;
    data.bold = isBold;
    data.italic = isItalic;
    data.tab = currentTab;
    data.anatomyChar = anatomyChar;
    GetWindowTextA(hCustomText, data.customText, sizeof(data.customText));
    data.rangeIndex = (int)SendMessage(hRangeList, CB_GETCURSEL, 0, 0);
    data.edChar = edChar;
    data.edFormat = edFormat;
    for (int i = 0; i < 95; i++) {
        for (int j = 0; j < 16; j++) {
            data.edBank[i][j] = edBank[i][j];
        }
    }

    HANDLE hFile = CreateFileA(QUICKSAVE_FILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(hFile, &data, sizeof(data), &written, NULL);
        CloseHandle(hFile);
        MessageBoxA(hwnd, "Workspace state quicksaved to kfont_quicksave.dat [F5].", "KFont", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(hwnd, "Failed to create quicksave file (kfont_quicksave.dat).", "KFont Error", MB_OK | MB_ICONERROR);
    }
}

void QuickloadNative(HWND hwnd) {
    HANDLE hFile = CreateFileA(QUICKSAVE_FILE, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        KFontSaveData data = {0};
        DWORD read = 0;
        ReadFile(hFile, &data, sizeof(data), &read, NULL);
        CloseHandle(hFile);
        if (read >= 588) { // Backward compatible with previous struct (592 bytes) and current struct (596 bytes)
            isBold = data.bold;
            isItalic = data.italic;
            SendMessage(hBold, BM_SETCHECK, isBold ? BST_CHECKED : BST_UNCHECKED, 0);
            SendMessage(hItalic, BM_SETCHECK, isItalic ? BST_CHECKED : BST_UNCHECKED, 0);

            int lbIdx = (int)SendMessageA(hList, LB_FINDSTRINGEXACT, -1, (LPARAM)data.fontName);
            if (lbIdx != LB_ERR) {
                SendMessage(hList, LB_SETCURSEL, lbIdx, 0);
                lstrcpyA(currentFontName, data.fontName);
            }

            char szBuf[16];
            wsprintfA(szBuf, "%d", data.size);
            int cbIdx = (int)SendMessageA(hSizeList, CB_FINDSTRINGEXACT, -1, (LPARAM)szBuf);
            if (cbIdx != CB_ERR) {
                SendMessage(hSizeList, CB_SETCURSEL, cbIdx, 0);
            }

            anatomyChar = data.anatomyChar;
            WCHAR wBuf[16];
            wsprintfW(wBuf, L"U+%04X", (UINT)anatomyChar);
            SetWindowTextW(hAnatomyChar, wBuf);

            lstrcpynA(currentCustomText, data.customText, sizeof(currentCustomText));
            SetWindowTextA(hCustomText, currentCustomText);

            if (data.rangeIndex >= 0 && data.rangeIndex < 11) {
                SendMessage(hRangeList, CB_SETCURSEL, data.rangeIndex, 0);
            }

            if (read >= sizeof(KFontSaveData)) {
                if (data.edChar >= 32 && data.edChar <= 126) edChar = data.edChar;
                if (data.edFormat >= 0 && data.edFormat <= 2) edFormat = data.edFormat;
                for (int i = 0; i < 95; i++) {
                    for (int j = 0; j < 16; j++) {
                        edBank[i][j] = data.edBank[i][j];
                    }
                }
                edBankInitialized = TRUE;
            }

            UpdateFont(hwnd);
            SelectTab(hwnd, data.tab);

            // Ensure tutorial flag is marked so restored states never prompt
            HANDLE hTut = CreateFileA(TUTORIAL_FILE, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hTut != INVALID_HANDLE_VALUE) CloseHandle(hTut);

            MessageBoxA(hwnd, "Quicksave snapshot restored from kfont_quicksave.dat [F9].", "KFont", MB_OK | MB_ICONINFORMATION);
            return;
        }
    }
    MessageBoxA(hwnd, "No quicksave snapshot found (kfont_quicksave.dat).\nPress F5 to save snapshot.", "KFont", MB_OK | MB_ICONWARNING);
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
        "  [7] Spec / Code: Win32 C LOGFONT & CSS variables code generator\n"
        "  [8] Editor: Bitmap font studio, pixel glyph grid, & BDF/TTF/C export\n\n"
        "Keyboard Shortcuts:\n"
        "  [1-8]  : Switch inspector tabs\n"
        "  [F5]   : Quicksave workspace snapshot (kfont_quicksave.dat)\n"
        "  [F9]   : Quickload snapshot (kfont_quicksave.dat)\n"
        "  [B]    : Toggle Bold\n"
        "  [I]    : Toggle Italic\n"
        "  [C]    : Copy context report or C LOGFONT code snippet\n"
        "  [ / ]  : Cycle Previous / Next font family\n"
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

            CreateWindowEx(0, "STATIC", "Workspace State:", WS_CHILD | WS_VISIBLE, 10, 582, 150, 18, hwnd, NULL, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "Save [F5]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 10, 602, 70, 24, hwnd, (HMENU)120, NULL, NULL);
            CreateWindowEx(0, "BUTTON", "Load [F9]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 86, 602, 72, 24, hwnd, (HMENU)121, NULL, NULL);

            // Tab bar (8 tabs + Copy + Help)
            hTabMetrics   = CreateWindowEx(0, "BUTTON", "Metrics [1]",   WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, 170, 10, 76, 20, hwnd, (HMENU)10, NULL, NULL);
            hTabGlyphs    = CreateWindowEx(0, "BUTTON", "Glyphs [2]",    WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 248, 10, 70, 20, hwnd, (HMENU)11, NULL, NULL);
            hTabDiag      = CreateWindowEx(0, "BUTTON", "Diagnostics [3]", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 320, 10, 88, 20, hwnd, (HMENU)12, NULL, NULL);
            hTabAnatomy   = CreateWindowEx(0, "BUTTON", "Anatomy [4]",   WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 410, 10, 76, 20, hwnd, (HMENU)13, NULL, NULL);
            hTabWaterfall = CreateWindowEx(0, "BUTTON", "Waterfall [5]", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 488, 10, 74, 20, hwnd, (HMENU)14, NULL, NULL);
            hTabSample    = CreateWindowEx(0, "BUTTON", "Sample [6]",    WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 564, 10, 66, 20, hwnd, (HMENU)15, NULL, NULL);
            hTabSpec      = CreateWindowEx(0, "BUTTON", "Spec/Code [7]", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 632, 10, 72, 20, hwnd, (HMENU)16, NULL, NULL);
            hTabEditor    = CreateWindowEx(0, "BUTTON", "Editor [8]",    WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 706, 10, 74, 20, hwnd, (HMENU)17, NULL, NULL);
            hCopyBtn      = CreateWindowEx(0, "BUTTON", "Copy [C]",      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 784, 10, 64, 20, hwnd, (HMENU)18, NULL, NULL);
            hHelpBtn      = CreateWindowEx(0, "BUTTON", "Help [F1]",     WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 852, 10, 68, 20, hwnd, (HMENU)19, NULL, NULL);
            
            SendMessage(hTabMetrics, BM_SETCHECK, BST_CHECKED, 0);

            WNDCLASS pc = {0};
            pc.lpfnWndProc = PanelProc;
            pc.hInstance = GetModuleHandle(NULL);
            pc.lpszClassName = "KFontPanel";
            hPanelBrush = CreateSolidBrush(RGB(255,255,255));
            pc.hbrBackground = hPanelBrush;
            RegisterClass(&pc);
            
            hPanel = CreateWindowEx(WS_EX_CLIENTEDGE, "KFontPanel", "", WS_CHILD | WS_VISIBLE, 170, 38, W - 190, H - 48, hwnd, NULL, NULL, NULL);
            
            hBtnEdPrev      = CreateWindowEx(0, "BUTTON", "< Prev", WS_CHILD, 15, 8, 50, 22, hPanel, (HMENU)201, NULL, NULL);
            hBtnEdNext      = CreateWindowEx(0, "BUTTON", "Next >", WS_CHILD, 68, 8, 50, 22, hPanel, (HMENU)202, NULL, NULL);
            hBtnEdRasterize = CreateWindowEx(0, "BUTTON", "Rasterize", WS_CHILD, 122, 8, 64, 22, hPanel, (HMENU)203, NULL, NULL);
            hBtnEdClear     = CreateWindowEx(0, "BUTTON", "Clear", WS_CHILD, 190, 8, 46, 22, hPanel, (HMENU)204, NULL, NULL);
            hBtnEdInvert    = CreateWindowEx(0, "BUTTON", "Invert", WS_CHILD, 240, 8, 46, 22, hPanel, (HMENU)205, NULL, NULL);
            hBtnEdFmt       = CreateWindowEx(0, "BUTTON", "Fmt: BDF 2.1", WS_CHILD, 290, 8, 88, 22, hPanel, (HMENU)206, NULL, NULL);
            hBtnEdCopy      = CreateWindowEx(0, "BUTTON", "Copy Code", WS_CHILD, 382, 8, 72, 22, hPanel, (HMENU)207, NULL, NULL);
            hBtnEdExport    = CreateWindowEx(0, "BUTTON", "Save Export", WS_CHILD, 458, 8, 80, 22, hPanel, (HMENU)208, NULL, NULL);
            hEditorCode     = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL, 290, 34, 450, 600, hPanel, (HMENU)210, NULL, NULL);

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
            InitEditorBank(hdc);
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
            else if (LOWORD(wParam) == 18 && HIWORD(wParam) == BN_CLICKED) {
                CopyReportToClipboard(hwnd);
            }
            else if (LOWORD(wParam) == 19 && HIWORD(wParam) == BN_CLICKED) {
                ShowHelpDialog(hwnd);
            }
            else if (LOWORD(wParam) == 120 && HIWORD(wParam) == BN_CLICKED) {
                QuicksaveNative(hwnd);
            }
            else if (LOWORD(wParam) == 121 && HIWORD(wParam) == BN_CLICKED) {
                QuickloadNative(hwnd);
            }
            else if (LOWORD(wParam) >= 10 && LOWORD(wParam) <= 17 && HIWORD(wParam) == BN_CLICKED) {
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
                hChild == hTabSpec || hChild == hTabEditor || hChild == hBold || hChild == hItalic) {
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

    // First-run tutorial onboarding flag
    DWORD tutAttr = GetFileAttributesA(TUTORIAL_FILE);
    if (tutAttr == INVALID_FILE_ATTRIBUTES) {
        HANDLE hTut = CreateFileA(TUTORIAL_FILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hTut != INVALID_HANDLE_VALUE) {
            char tutFlag[] = "1\n";
            DWORD wr = 0;
            WriteFile(hTut, tutFlag, sizeof(tutFlag) - 1, &wr, NULL);
            CloseHandle(hTut);
        }
        if (GetFileAttributesA(QUICKSAVE_FILE) == INVALID_FILE_ATTRIBUTES) {
            MessageBoxA(hwnd,
                "Welcome to KFont Typography & Font Engineering Inspector!\n\n"
                "Explore font outline metrics, Unicode blocks, kerning tables, vector anatomy, and code generation.\n\n"
                "Key Controls:\n"
                "  - [1-8]  : Switch inspector tabs\n"
                "  - [F5]   : Quicksave workspace snapshot (kfont_quicksave.dat)\n"
                "  - [F9]   : Quickload snapshot\n"
                "  - [F1/H] : Comprehensive Help & Shortcut Reference\n"
                "  - [B/I]  : Toggle Bold / Italic weights\n"
                "  - [C]    : Copy context report / Win32 LOGFONT snippet",
                "KFont - Initial Setup", MB_OK | MB_ICONINFORMATION);
        }
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN) {
            HWND hFocus = GetFocus();
            BOOL inEdit = (hFocus == hCustomText || hFocus == hAnatomyChar || hFocus == hEditorCode);
            if (msg.wParam == VK_ESCAPE) {
                if (inEdit) {
                    SetFocus(hwnd);
                    continue;
                }
            } else if (msg.wParam == VK_F5) {
                QuicksaveNative(hwnd);
                continue;
            } else if (msg.wParam == VK_F9) {
                QuickloadNative(hwnd);
                continue;
            } else if (msg.wParam == 'H' || msg.wParam == 'h' || msg.wParam == VK_F1) {
                if (!inEdit) {
                    ShowHelpDialog(hwnd);
                    continue;
                }
            } else if (!inEdit) {
                if (msg.wParam >= '1' && msg.wParam <= '8') {
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
                } else if (msg.wParam == VK_OEM_4) {
                    int cur = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
                    int count = (int)SendMessage(hList, LB_GETCOUNT, 0, 0);
                    if (count > 0) {
                        cur = (cur - 1 + count) % count;
                        SendMessage(hList, LB_SETCURSEL, cur, 0);
                        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(1, LBN_SELCHANGE), (LPARAM)hList);
                    }
                    continue;
                } else if (msg.wParam == VK_OEM_6) {
                    int cur = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
                    int count = (int)SendMessage(hList, LB_GETCOUNT, 0, 0);
                    if (count > 0) {
                        cur = (cur + 1) % count;
                        SendMessage(hList, LB_SETCURSEL, cur, 0);
                        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(1, LBN_SELCHANGE), (LPARAM)hList);
                    }
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