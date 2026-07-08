#include <windows.h>
#include <math.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

int my_strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

// Simple recursive descent parser for math expressions with 'x'
const char* expr_ptr;
double eval_x_val;

double expr();

void skip_whitespace() {
    while (*expr_ptr == ' ') expr_ptr++;
}

double factor() {
    skip_whitespace();
    if (*expr_ptr == '(') {
        expr_ptr++;
        double res = expr();
        if (*expr_ptr == ')') expr_ptr++;
        return res;
    } else if (*expr_ptr == 'x' || *expr_ptr == 'X') {
        expr_ptr++;
        return eval_x_val;
    } else if (*expr_ptr == '-' || *expr_ptr == '+') {
        char op = *expr_ptr++;
        double res = factor();
        return (op == '-') ? -res : res;
    } else {
        // Simple float parse
        double res = 0.0;
        double frac = 1.0;
        int is_frac = 0;
        while ((*expr_ptr >= '0' && *expr_ptr <= '9') || *expr_ptr == '.') {
            if (*expr_ptr == '.') {
                is_frac = 1;
            } else {
                if (is_frac) {
                    frac *= 10.0;
                    res += (*expr_ptr - '0') / frac;
                } else {
                    res = res * 10.0 + (*expr_ptr - '0');
                }
            }
            expr_ptr++;
        }
        return res;
    }
}

double term() {
    double res = factor();
    skip_whitespace();
    while (*expr_ptr == '*' || *expr_ptr == '/') {
        char op = *expr_ptr++;
        double val = factor();
        if (op == '*') res *= val;
        else if (val != 0.0) res /= val;
        skip_whitespace();
    }
    return res;
}

double expr() {
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

double evaluate(const char* e, double x) {
    expr_ptr = e;
    eval_x_val = x;
    return expr();
}

HWND hInput, hPlotBtn;
char current_expr[256] = "x*x";

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "x*x", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 10, 200, 24, hwnd, NULL, NULL, NULL);
            hPlotBtn = CreateWindowA("BUTTON", "Plot", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 220, 10, 60, 24, hwnd, (HMENU)1001, NULL, NULL);
            
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Tahoma");
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hPlotBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) {
                GetWindowTextA(hInput, current_expr, 255);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            rect.top = 40; // Avoid top controls
            
            // Draw background
            HBRUSH bgBrush = CreateSolidBrush(RGB(10, 10, 10));
            FillRect(hdc, &rect, bgBrush);
            DeleteObject(bgBrush);
            
            int w = rect.right - rect.left;
            int h = rect.bottom - rect.top;
            int cx = rect.left + w / 2;
            int cy = rect.top + h / 2;
            
            HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));
            HPEN axisPen = CreatePen(PS_SOLID, 2, RGB(100, 100, 100));
            HPEN plotPen = CreatePen(PS_SOLID, 2, RGB(90, 139, 212));
            
            // Grid
            SelectObject(hdc, gridPen);
            for(int i = -10; i <= 10; i++) {
                int px = cx + (i * w / 20);
                MoveToEx(hdc, px, rect.top, NULL);
                LineTo(hdc, px, rect.bottom);
                
                int py = cy - (i * h / 20);
                MoveToEx(hdc, rect.left, py, NULL);
                LineTo(hdc, rect.right, py);
            }
            
            // Axes
            SelectObject(hdc, axisPen);
            MoveToEx(hdc, cx, rect.top, NULL); LineTo(hdc, cx, rect.bottom);
            MoveToEx(hdc, rect.left, cy, NULL); LineTo(hdc, rect.right, cy);
            
            // Plot
            SelectObject(hdc, plotPen);
            int first = 1;
            for(int px = 0; px <= w; px += 2) {
                double x = ((double)px / w) * 20.0 - 10.0;
                double y = evaluate(current_expr, x);
                int py = cy - (int)((y / 10.0) * (h / 2));
                
                if (first) {
                    MoveToEx(hdc, rect.left + px, py, NULL);
                    first = 0;
                } else {
                    LineTo(hdc, rect.left + px, py);
                }
            }
            
            DeleteObject(gridPen);
            DeleteObject(axisPen);
            DeleteObject(plotPen);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KGraphClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KGraphClass", "KGraph", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
