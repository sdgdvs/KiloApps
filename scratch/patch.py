import re
import sys

with open("MicroBBS/main.c", "r") as f:
    src = f.read()

# 1. Globals
src = src.replace(
    "struct Cell screen[TERM_ROWS][TERM_COLS];\n",
    "#define MAX_LINES 1000\nstruct Cell screen[MAX_LINES][TERM_COLS];\nint activeTop = 0;\nint scrollOffset = 0;\n"
)

# 2. ClearScreen
src = src.replace(
    "void ClearScreen(void) {\n    int r, c;\n    for (r = 0; r < TERM_ROWS; r++) {",
    "void ClearScreen(void) {\n    int r, c;\n    for (r = 0; r < MAX_LINES; r++) {"
)
src = src.replace(
    "    curX = 0;\n    curY = 0;\n}",
    "    activeTop = 0;\n    scrollOffset = 0;\n    curX = 0;\n    curY = 0;\n}"
)

# 3. ScrollUp
scroll_up_new = """void ScrollUp(void) {
    int r, c;
    if (activeTop + TERM_ROWS < MAX_LINES) {
        activeTop++;
    } else {
        int shift = 100;
        memmove(&screen[0][0], &screen[shift][0], sizeof(struct Cell) * (MAX_LINES - shift) * TERM_COLS);
        activeTop -= shift;
        for (r = MAX_LINES - shift; r < MAX_LINES; r++) {
            for (c = 0; c < TERM_COLS; c++) {
                screen[r][c].ch = ' ';
                screen[r][c].fg = 7;
                screen[r][c].bg = 0;
            }
        }
        activeTop++;
    }
    for (c = 0; c < TERM_COLS; c++) {
        screen[activeTop + TERM_ROWS - 1][c].ch = ' ';
        screen[activeTop + TERM_ROWS - 1][c].fg = 7;
        screen[activeTop + TERM_ROWS - 1][c].bg = 0;
    }
    if (scrollOffset > 0) {
        scrollOffset++;
    }
}"""
src = re.sub(r"void ScrollUp\(void\) \{.*?\n\}", scroll_up_new, src, flags=re.DOTALL)

# 4. PutChar
src = src.replace("screen[curY][curX].ch = ch;", "screen[activeTop + curY][curX].ch = ch;")
src = src.replace("screen[curY][curX].fg = boldFlag ? (curFg | 8) : curFg;", "screen[activeTop + curY][curX].fg = boldFlag ? (curFg | 8) : curFg;")
src = src.replace("screen[curY][curX].bg = curBg;", "screen[activeTop + curY][curX].bg = curBg;")

# 5. ProcessCSI array indices
src = src.replace("screen[r][c]", "screen[activeTop + r][c]")
src = src.replace("screen[curY][c]", "screen[activeTop + curY][c]")

# 6. WM_PAINT drawing
src = src.replace(
    "                    char ch = screen[r][c].ch;\n                    SetTextColor(memDC, ansiColors[screen[r][c].fg & 0x0F]);\n                    SetBkColor(memDC, ansiColors[screen[r][c].bg & 0x0F]);",
    "                    int actR = activeTop - scrollOffset + r;\n                    if (actR < 0) actR = 0;\n                    char ch = screen[actR][c].ch;\n                    SetTextColor(memDC, ansiColors[screen[actR][c].fg & 0x0F]);\n                    SetBkColor(memDC, ansiColors[screen[actR][c].bg & 0x0F]);"
)

src = src.replace(
    "            if (curX >= 0 && curX < TERM_COLS && curY >= 0 && curY < TERM_ROWS) {",
    "            if (scrollOffset == 0 && curX >= 0 && curX < TERM_COLS && curY >= 0 && curY < TERM_ROWS) {"
)

# Scaling in WM_PAINT
old_bitblt = "            BitBlt(hdc, termX, termY, termW, termH, memDC, 0, 0, SRCCOPY);"
new_bitblt = """            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int drawW = clientRect.right - clientRect.left - termX * 2;
            int drawH = clientRect.bottom - clientRect.top - termY - 24;
            if (drawW < termW) drawW = termW;
            if (drawH < termH) drawH = termH;
            StretchBlt(hdc, termX, termY, drawW, drawH, memDC, 0, 0, termW, termH, SRCCOPY);"""
src = src.replace(old_bitblt, new_bitblt)

# 7. Add WM_VSCROLL, WM_MOUSEWHEEL, WM_SIZE before WM_LBUTTONDOWN
new_handlers = """        case WM_SIZE: {
            int cx = LOWORD(lParam);
            int cy = HIWORD(lParam);
            SetWindowPos(hStatus, NULL, 5, cy - 20, cx - 10, 18, SWP_NOZORDER);
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
            si.nMin = 0;
            si.nMax = activeTop + TERM_ROWS - 1;
            si.nPage = TERM_ROWS;
            si.nPos = activeTop - scrollOffset;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        case WM_MOUSEWHEEL: {
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (zDelta > 0) scrollOffset += 3;
            else scrollOffset -= 3;
            if (scrollOffset > activeTop) scrollOffset = activeTop;
            if (scrollOffset < 0) scrollOffset = 0;
            
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SIF_POS;
            si.nPos = activeTop - scrollOffset;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_VSCROLL: {
            int action = LOWORD(wParam);
            if (action == SB_LINEUP) scrollOffset++;
            else if (action == SB_LINEDOWN) scrollOffset--;
            else if (action == SB_PAGEUP) scrollOffset += TERM_ROWS;
            else if (action == SB_PAGEDOWN) scrollOffset -= TERM_ROWS;
            else if (action == SB_THUMBTRACK) {
                SCROLLINFO si;
                si.cbSize = sizeof(si);
                si.fMask = SIF_TRACKPOS;
                GetScrollInfo(hwnd, SB_VERT, &si);
                scrollOffset = activeTop - si.nTrackPos;
            }
            if (scrollOffset > activeTop) scrollOffset = activeTop;
            if (scrollOffset < 0) scrollOffset = 0;
            
            SCROLLINFO si;
            si.cbSize = sizeof(si);
            si.fMask = SIF_POS;
            si.nPos = activeTop - scrollOffset;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_LBUTTONDOWN:"""

src = src.replace("        case WM_LBUTTONDOWN:", new_handlers)

# Reset scroll offset when typing
src = src.replace(
    "                char ch = (char)wParam;\n                send(sock, &ch, 1, 0);",
    "                char ch = (char)wParam;\n                scrollOffset = 0;\n                send(sock, &ch, 1, 0);"
)

# Update scrollbar continuously in ProcessByte/ProcessCSI (just invalidate usually, but let's do it in WM_PAINT indirectly or after connection)
src = src.replace(
    "                telState = TEL_STATE_NORMAL;\n                InvalidateRect(hwnd, NULL, FALSE);\n            }",
    "                telState = TEL_STATE_NORMAL;\n                InvalidateRect(hwnd, NULL, FALSE);\n                PostMessage(hwnd, WM_SIZE, 0, MAKELPARAM(650, 490));\n            }"
)

# Call WM_SIZE at startup to set scrollbar
src = src.replace(
    "    ShowWindow(hMain, SW_SHOW);",
    "    ShowWindow(hMain, SW_SHOW);\n    PostMessage(hMain, WM_SIZE, 0, MAKELPARAM(winW, winH));"
)

# WinMain style
src = src.replace(
    "        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,",
    "        WS_OVERLAPPEDWINDOW | WS_VSCROLL,"
)

with open("MicroBBS/main.c", "w") as f:
    f.write(src)
print("Patched main.c successfully.")
