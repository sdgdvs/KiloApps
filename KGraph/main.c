#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E  2.71828182845904523536
#endif

// --- Math Expression Evaluator ---
static const char* expr_ptr;
static double eval_x_val;

static double expr(void);

static void skip_whitespace(void) {
    while (*expr_ptr == ' ' || *expr_ptr == '\t') expr_ptr++;
}

static double make_nan(void) {
    double z = 0.0;
    return z / z;
}

static double factor(void) {
    skip_whitespace();
    if (*expr_ptr == '-') {
        expr_ptr++;
        return -factor();
    }
    if (*expr_ptr == '+') {
        expr_ptr++;
        return factor();
    }
    if (*expr_ptr == '(') {
        expr_ptr++;
        double res = expr();
        skip_whitespace();
        if (*expr_ptr == ')') expr_ptr++;
        return res;
    }
    if ((*expr_ptr >= 'a' && *expr_ptr <= 'z') || (*expr_ptr >= 'A' && *expr_ptr <= 'Z')) {
        char id[32];
        int len = 0;
        while (((*expr_ptr >= 'a' && *expr_ptr <= 'z') || (*expr_ptr >= 'A' && *expr_ptr <= 'Z') || (*expr_ptr >= '0' && *expr_ptr <= '9')) && len < 31) {
            id[len++] = *expr_ptr++;
        }
        id[len] = '\0';
        
        if (_stricmp(id, "x") == 0) return eval_x_val;
        if (_stricmp(id, "pi") == 0) return M_PI;
        if (_stricmp(id, "e") == 0) return M_E;
        
        skip_whitespace();
        if (*expr_ptr == '(') {
            expr_ptr++;
            double arg = expr();
            skip_whitespace();
            if (*expr_ptr == ')') expr_ptr++;
            
            if (_stricmp(id, "sin") == 0) return sin(arg);
            if (_stricmp(id, "cos") == 0) return cos(arg);
            if (_stricmp(id, "tan") == 0) return tan(arg);
            if (_stricmp(id, "sqrt") == 0) return (arg >= 0) ? sqrt(arg) : make_nan();
            if (_stricmp(id, "abs") == 0 || _stricmp(id, "fabs") == 0) return fabs(arg);
            if (_stricmp(id, "exp") == 0) return exp(arg);
            if (_stricmp(id, "log") == 0 || _stricmp(id, "ln") == 0) return (arg > 0) ? log(arg) : make_nan();
        }
        return 0.0;
    }
    
    char* endp;
    double val = strtod(expr_ptr, &endp);
    if (endp != expr_ptr) {
        expr_ptr = endp;
        return val;
    }
    return 0.0;
}

static double power(void) {
    double base = factor();
    skip_whitespace();
    if (*expr_ptr == '^') {
        expr_ptr++;
        double exp_val = power();
        return pow(base, exp_val);
    }
    return base;
}

static double term(void) {
    double res = power();
    skip_whitespace();
    while (*expr_ptr == '*' || *expr_ptr == '/') {
        char op = *expr_ptr++;
        double val = power();
        if (op == '*') res *= val;
        else if (val != 0.0) res /= val;
        else res = make_nan();
        skip_whitespace();
    }
    return res;
}

static double expr(void) {
    double res = term();
    skip_whitespace();
    while (*expr_ptr == '+' || *expr_ptr == '-') {
        char op = *expr_ptr++;
        double val = term();
        if (op == '+') res += val;
        else res -= val;
        skip_whitespace();
    }
    return res;
}

static double evaluate(const char* e, double x) {
    if (!e || !*e) return make_nan();
    expr_ptr = e;
    eval_x_val = x;
    return expr();
}

static double eval_derivative(const char* e, double x) {
    double h = 1e-5;
    double y1 = evaluate(e, x + h);
    double y0 = evaluate(e, x - h);
    if (isnan(y1) || isnan(y0)) return make_nan();
    return (y1 - y0) / (2.0 * h);
}

// --- Data Structures & State ---
#define MAX_FUNCS 3
typedef struct {
    char expr[128];
    int enabled;
    COLORREF color;
} FuncState;

static FuncState funcs[MAX_FUNCS] = {
    {"sin(x)", 1, RGB(56, 189, 248)},
    {"cos(x)", 1, RGB(52, 211, 153)},
    {"x^2/4 - 2", 0, RGB(244, 63, 94)}
};

static double view_scale = 10.0;
static double view_cx = 0.0;
static double view_cy = 0.0;

static int is_dragging = 0;
static POINT last_mouse;
static POINT hover_mouse = {-1, -1};

typedef struct {
    double x;
    double y;
    int func_idx;
} RootPoint;

static RootPoint roots[64];
static int root_count = 0;

// Win32 Control Handles
static HWND hInputs[MAX_FUNCS];
static HWND hChecks[MAX_FUNCS];
static HWND hPlotBtn, hZoomIn, hZoomOut, hResetBtn, hRootsBtn, hPresetBtn, hStatus;
static HFONT hFontSmall, hFontBold;

static void FindRootsInView(void) {
    root_count = 0;
    double minX = view_cx - view_scale;
    double maxX = view_cx + view_scale;
    int steps = 200;
    double dx = (maxX - minX) / steps;

    for (int idx = 0; idx < MAX_FUNCS; idx++) {
        if (!funcs[idx].enabled) continue;
        for (int i = 0; i < steps && root_count < 64; i++) {
            double x1 = minX + i * dx;
            double x2 = x1 + dx;
            double y1 = evaluate(funcs[idx].expr, x1);
            double y2 = evaluate(funcs[idx].expr, x2);

            if (isnan(y1) || isnan(y2)) continue;

            if (y1 * y2 <= 0.0) {
                double low = x1, high = x2;
                for (int iter = 0; iter < 20; iter++) {
                    double mid = (low + high) / 2.0;
                    double yMid = evaluate(funcs[idx].expr, mid);
                    if (evaluate(funcs[idx].expr, low) * yMid <= 0.0) high = mid;
                    else low = mid;
                }
                double rootX = (low + high) / 2.0;
                double rootY = evaluate(funcs[idx].expr, rootX);
                if (fabs(rootY) < 1e-3) {
                    roots[root_count].x = rootX;
                    roots[root_count].y = rootY;
                    roots[root_count].func_idx = idx;
                    root_count++;
                }
            }
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hFontSmall = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            int topY = 8;
            for (int i = 0; i < MAX_FUNCS; i++) {
                char label[16];
                sprintf(label, "y%d =", i + 1);
                CreateWindowA("STATIC", label, WS_CHILD | WS_VISIBLE, 10, topY + 3, 30, 20, hwnd, NULL, NULL, NULL);
                hInputs[i] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", funcs[i].expr, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 42, topY, 140, 22, hwnd, NULL, NULL, NULL);
                hChecks[i] = CreateWindowA("BUTTON", "Show", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 188, topY + 2, 50, 20, hwnd, (HMENU)(HMENU)(1100 + i), NULL, NULL);
                SendMessageA(hChecks[i], BM_SETCHECK, funcs[i].enabled ? BST_CHECKED : BST_UNCHECKED, 0);

                SendMessageA(hInputs[i], WM_SETFONT, (WPARAM)hFontSmall, TRUE);
                SendMessageA(hChecks[i], WM_SETFONT, (WPARAM)hFontSmall, TRUE);

                topY += 26;
            }

            hPlotBtn  = CreateWindowA("BUTTON", "Plot", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 245, 8, 55, 22, hwnd, (HMENU)1001, NULL, NULL);
            hZoomIn   = CreateWindowA("BUTTON", "+", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 305, 8, 30, 22, hwnd, (HMENU)1002, NULL, NULL);
            hZoomOut  = CreateWindowA("BUTTON", "-", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 340, 8, 30, 22, hwnd, (HMENU)1003, NULL, NULL);
            hResetBtn = CreateWindowA("BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 375, 8, 55, 22, hwnd, (HMENU)1004, NULL, NULL);
            hRootsBtn = CreateWindowA("BUTTON", "Roots", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 245, 34, 55, 22, hwnd, (HMENU)1005, NULL, NULL);
            hPresetBtn= CreateWindowA("BUTTON", "Presets", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 305, 34, 70, 22, hwnd, (HMENU)1006, NULL, NULL);

            hStatus = CreateWindowA("STATIC", "Ready. Hover mouse over graph to trace values.", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, topY + 5, 480, 20, hwnd, NULL, NULL, NULL);
            SendMessageA(hStatus, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

            SendMessageA(hPlotBtn, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hZoomIn, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hZoomOut, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hResetBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hRootsBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hPresetBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 1001) { // Plot
                for (int i = 0; i < MAX_FUNCS; i++) {
                    GetWindowTextA(hInputs[i], funcs[i].expr, 127);
                    funcs[i].enabled = (SendMessageA(hChecks[i], BM_GETCHECK, 0, 0) == BST_CHECKED);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (id == 1002) { // Zoom In
                view_scale *= 0.75;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (id == 1003) { // Zoom Out
                view_scale *= 1.3333;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (id == 1004) { // Reset
                view_cx = 0.0; view_cy = 0.0; view_scale = 10.0;
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (id == 1005) { // Find Roots
                FindRootsInView();
                char msgBuf[128];
                sprintf(msgBuf, "Found %d root(s) in view range.", root_count);
                SetWindowTextA(hStatus, msgBuf);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (id == 1006) { // Presets
                SetWindowTextA(hInputs[0], "exp(-x^2)");
                SetWindowTextA(hInputs[1], "sin(3*x)");
                SetWindowTextA(hInputs[2], "x^3 - 3*x");
                SendMessageA(hChecks[0], BM_SETCHECK, BST_CHECKED, 0);
                SendMessageA(hChecks[1], BM_SETCHECK, BST_CHECKED, 0);
                SendMessageA(hChecks[2], BM_SETCHECK, BST_CHECKED, 0);
                SendMessageA(hwnd, WM_COMMAND, 1001, 0);
            } else if (id >= 1100 && id < 1100 + MAX_FUNCS) {
                int idx = id - 1100;
                funcs[idx].enabled = (SendMessageA(hChecks[idx], BM_GETCHECK, 0, 0) == BST_CHECKED);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }

        case WM_LBUTTONDOWN: {
            if (HIWORD(lParam) > 105) {
                is_dragging = 1;
                last_mouse.x = LOWORD(lParam);
                last_mouse.y = HIWORD(lParam);
                SetCapture(hwnd);
            }
            break;
        }

        case WM_MOUSEMOVE: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            hover_mouse.x = mx;
            hover_mouse.y = my;

            RECT rect; GetClientRect(hwnd, &rect);
            int canvasTop = 105;
            int w = rect.right - rect.left;
            int h = rect.bottom - canvasTop;

            if (is_dragging && w > 0 && h > 0) {
                view_cx -= (double)(mx - last_mouse.x) / w * (2.0 * view_scale);
                view_cy += (double)(my - last_mouse.y) / h * (2.0 * view_scale);
                last_mouse.x = mx;
                last_mouse.y = my;
            }

            if (my >= canvasTop && w > 0) {
                double worldX = view_cx - view_scale + ((double)mx / w) * (2.0 * view_scale);
                char statusText[256];
                int offset = sprintf(statusText, "x = %.4f | ", worldX);

                for (int i = 0; i < MAX_FUNCS; i++) {
                    if (funcs[i].enabled) {
                        double yVal = evaluate(funcs[i].expr, worldX);
                        double dyVal = eval_derivative(funcs[i].expr, worldX);
                        offset += sprintf(statusText + offset, "y%d=%.3f (y%d'=%.2f)  ", i + 1, yVal, i + 1, dyVal);
                    }
                }
                SetWindowTextA(hStatus, statusText);
            }

            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        case WM_LBUTTONUP: {
            if (is_dragging) {
                is_dragging = 0;
                ReleaseCapture();
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect);
            int canvasTop = 105;
            RECT canvasRect = {rect.left, canvasTop, rect.right, rect.bottom};

            // Double Buffering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBM = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

            // Fill canvas background
            HBRUSH bgBrush = CreateSolidBrush(RGB(15, 17, 26));
            FillRect(memDC, &canvasRect, bgBrush);
            DeleteObject(bgBrush);

            int w = rect.right - rect.left;
            int h = rect.bottom - canvasTop;

            if (w > 0 && h > 0) {
                // Grid Pens
                HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(35, 42, 60));
                HPEN axisPen = CreatePen(PS_SOLID, 2, RGB(90, 105, 130));

                SelectObject(memDC, gridPen);
                for (int i = -10; i <= 10; i++) {
                    int px = w / 2 + (i * w / 20);
                    MoveToEx(memDC, px, canvasTop, NULL);
                    LineTo(memDC, px, rect.bottom);

                    int py = canvasTop + h / 2 - (i * h / 20);
                    MoveToEx(memDC, rect.left, py, NULL);
                    LineTo(memDC, rect.right, py);
                }

                // Axes
                SelectObject(memDC, axisPen);
                int axis_x = rect.left + (int)((0.0 - (view_cx - view_scale)) / (2.0 * view_scale) * w);
                int axis_y = canvasTop + (int)(h - (0.0 - (view_cy - view_scale)) / (2.0 * view_scale) * h);

                if (axis_x >= rect.left && axis_x <= rect.right) {
                    MoveToEx(memDC, axis_x, canvasTop, NULL); LineTo(memDC, axis_x, rect.bottom);
                }
                if (axis_y >= canvasTop && axis_y <= rect.bottom) {
                    MoveToEx(memDC, rect.left, axis_y, NULL); LineTo(memDC, rect.right, axis_y);
                }

                // Plot Curves
                for (int idx = 0; idx < MAX_FUNCS; idx++) {
                    if (!funcs[idx].enabled) continue;

                    HPEN plotPen = CreatePen(PS_SOLID, 2, funcs[idx].color);
                    SelectObject(memDC, plotPen);

                    int first = 1;
                    for (int px = 0; px <= w; px += 2) {
                        double x = view_cx - view_scale + ((double)px / w) * (2.0 * view_scale);
                        double y = evaluate(funcs[idx].expr, x);
                        if (isnan(y)) {
                            first = 1;
                            continue;
                        }
                        int py = canvasTop + (int)(h - (y - (view_cy - view_scale)) / (2.0 * view_scale) * h);

                        if (py >= canvasTop - 500 && py <= rect.bottom + 500) {
                            if (first) {
                                MoveToEx(memDC, rect.left + px, py, NULL);
                                first = 0;
                            } else {
                                LineTo(memDC, rect.left + px, py);
                            }
                        } else {
                            first = 1;
                        }
                    }
                    DeleteObject(plotPen);
                }

                // Draw Root Markers
                HBRUSH rootBrush = CreateSolidBrush(RGB(52, 211, 153));
                SelectObject(memDC, rootBrush);
                for (int r = 0; r < root_count; r++) {
                    int rpx = rect.left + (int)((roots[r].x - (view_cx - view_scale)) / (2.0 * view_scale) * w);
                    int rpy = canvasTop + (int)(h - (roots[r].y - (view_cy - view_scale)) / (2.0 * view_scale) * h);
                    Ellipse(memDC, rpx - 4, rpy - 4, rpx + 5, rpy + 5);
                }
                DeleteObject(rootBrush);

                // Hover Crosshairs
                if (hover_mouse.y >= canvasTop) {
                    HPEN crossPen = CreatePen(PS_DOT, 1, RGB(150, 150, 150));
                    SelectObject(memDC, crossPen);
                    MoveToEx(memDC, hover_mouse.x, canvasTop, NULL);
                    LineTo(memDC, hover_mouse.x, rect.bottom);
                    DeleteObject(crossPen);
                }

                DeleteObject(gridPen);
                DeleteObject(axisPen);
            }

            BitBlt(hdc, 0, canvasTop, rect.right, rect.bottom - canvasTop, memDC, 0, canvasTop, SRCCOPY);

            SelectObject(memDC, oldBM);
            DeleteObject(memBM);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_DESTROY:
            if (hFontSmall) DeleteObject(hFontSmall);
            if (hFontBold) DeleteObject(hFontBold);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry(void) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KGraphClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KGraphClass", "KGraph Studio", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 520, 520, NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
