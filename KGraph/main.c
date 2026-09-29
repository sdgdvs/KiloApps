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

#pragma function(memcpy)
void* memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

// --- Math Expression Evaluator ---
static const char* expr_ptr;
static double eval_var_val;

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
        
        if (_stricmp(id, "x") == 0 || _stricmp(id, "t") == 0 || _stricmp(id, "theta") == 0 || _stricmp(id, "th") == 0) return eval_var_val;
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

static double evaluate(const char* e, double v) {
    if (!e || !*e) return make_nan();
    expr_ptr = e;
    eval_var_val = v;
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

typedef enum {
    MODE_CARTESIAN = 0,
    MODE_POLAR = 1,
    MODE_PARAMETRIC = 2
} PlotMode;

static PlotMode g_mode = MODE_CARTESIAN;

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

// Win32 Control Handles & State
static HWND g_hWnd = NULL;
static HWND hInputs[MAX_FUNCS];
static HWND hChecks[MAX_FUNCS];
static HWND hLabels[MAX_FUNCS];
static HWND hClrBtn[MAX_FUNCS];
static HWND hPlotBtn, hZoomIn, hZoomOut, hResetBtn, hRootsBtn, hPresetBtn, hHelpBtn, hModeBtn, hSaveBtn, hCopyBtn, hStatus;
static HWND hIntegralBtn, hTangentBtn, hSonifyBtn, hExportCsvBtn;
static HFONT hFontSmall, hFontBold;
static HBRUSH hTopBgBrush = NULL;
static HBRUSH hEditBgBrush = NULL;
static WNDPROC g_oldEditProc = NULL;
static int g_dpi = 96;
static int g_canvasTop = 125;
static int g_showIntegral = 0;
static int g_showTangent = 0;

static void ShowNativeStatus(const char* text) {
    if (hStatus) {
        SetWindowTextA(hStatus, text);
        if (g_hWnd) {
            SetTimer(g_hWnd, 2001, 4000, NULL);
        }
    }
}

static double EvalSimpsonIntegral(const char* expr_str, double a, double b, int n) {
    if (a >= b || n <= 0) return 0.0;
    if (n % 2 != 0) n++;
    double h = (b - a) / (double)n;
    double sum = evaluate(expr_str, a) + evaluate(expr_str, b);
    if (isnan(sum)) sum = 0.0;
    for (int i = 1; i < n; i++) {
        double x = a + i * h;
        double y = evaluate(expr_str, x);
        if (!isnan(y)) {
            sum += (i % 2 == 1 ? 4.0 : 2.0) * y;
        }
    }
    return (h / 3.0) * sum;
}

static DWORD WINAPI SonifyThreadProc(LPVOID lpParam) {
    WAVEFORMATEX wfx = {0};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 22050;
    wfx.wBitsPerSample = 8;
    wfx.nBlockAlign = 1;
    wfx.nAvgBytesPerSec = 22050;
    wfx.cbSize = 0;

    HWAVEOUT hWaveOut = NULL;
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        return 0;
    }

    const int totalSamples = 26460; // 1.2 seconds duration
    BYTE* pBuffer = (BYTE*)malloc(totalSamples);
    if (!pBuffer) {
        waveOutClose(hWaveOut);
        return 0;
    }

    double minX = view_cx - view_scale;
    double maxX = view_cx + view_scale;
    int steps = 36;
    int samplesPerStep = totalSamples / steps;
    double phase = 0.0;

    for (int s = 0; s < steps; s++) {
        double x = minX + ((double)s / steps) * (maxX - minX);
        double y = evaluate(funcs[0].expr, x);
        if (isnan(y)) y = 0.0;

        double normY = (y - (view_cy - view_scale)) / (2.0 * view_scale);
        if (normY < 0.0) normY = 0.0;
        if (normY > 1.0) normY = 1.0;
        double freq = 180.0 + normY * 720.0;
        double phaseInc = (2.0 * M_PI * freq) / 22050.0;

        for (int i = 0; i < samplesPerStep; i++) {
            int bufIdx = s * samplesPerStep + i;
            if (bufIdx < totalSamples) {
                double sampleVal = sin(phase) * 55.0;
                double env = 1.0;
                if (i < 80) env = (double)i / 80.0;
                else if (i > samplesPerStep - 80) env = (double)(samplesPerStep - i) / 80.0;
                pBuffer[bufIdx] = (BYTE)(128 + (int)(sampleVal * env));
                phase += phaseInc;
                if (phase > 2.0 * M_PI) phase -= 2.0 * M_PI;
            }
        }
    }

    WAVEHDR wh = {0};
    wh.lpData = (LPSTR)pBuffer;
    wh.dwBufferLength = totalSamples;
    wh.dwFlags = 0;

    waveOutPrepareHeader(hWaveOut, &wh, sizeof(wh));
    waveOutWrite(hWaveOut, &wh, sizeof(wh));

    while ((wh.dwFlags & WHDR_DONE) == 0) {
        Sleep(25);
    }

    waveOutUnprepareHeader(hWaveOut, &wh, sizeof(wh));
    waveOutClose(hWaveOut);
    free(pBuffer);
    return 0;
}

static void SonifyActiveCurve(HWND hwnd) {
    if (g_mode != MODE_CARTESIAN && g_mode != MODE_POLAR) {
        ShowNativeStatus("Curve Sonification active in Cartesian & Polar modes.");
        return;
    }
    ShowNativeStatus("Playing curve sonification audio [Space]...");
    HANDLE hThread = CreateThread(NULL, 0, SonifyThreadProc, NULL, 0, NULL);
    if (hThread) CloseHandle(hThread);
}

static void SaveCSVData(HWND hwnd) {
    FILE* f = fopen("KGraph_points.csv", "w");
    if (!f) {
        ShowNativeStatus("Failed to create KGraph_points.csv");
        return;
    }
    if (g_mode == MODE_CARTESIAN) {
        fprintf(f, "x,y1,y2,y3\n");
        double minX = view_cx - view_scale;
        double maxX = view_cx + view_scale;
        double dx = (maxX - minX) / 100.0;
        for (int i = 0; i <= 100; i++) {
            double x = minX + i * dx;
            double y1 = funcs[0].enabled ? evaluate(funcs[0].expr, x) : 0.0;
            double y2 = funcs[1].enabled ? evaluate(funcs[1].expr, x) : 0.0;
            double y3 = funcs[2].enabled ? evaluate(funcs[2].expr, x) : 0.0;
            fprintf(f, "%.5f,%.5f,%.5f,%.5f\n", x, y1, y2, y3);
        }
    } else if (g_mode == MODE_POLAR) {
        fprintf(f, "theta,r1,r2,r3\n");
        for (int i = 0; i <= 100; i++) {
            double th = (4.0 * M_PI * i) / 100.0;
            double r1 = funcs[0].enabled ? evaluate(funcs[0].expr, th) : 0.0;
            double r2 = funcs[1].enabled ? evaluate(funcs[1].expr, th) : 0.0;
            double r3 = funcs[2].enabled ? evaluate(funcs[2].expr, th) : 0.0;
            fprintf(f, "%.5f,%.5f,%.5f,%.5f\n", th, r1, r2, r3);
        }
    } else {
        fprintf(f, "t,x1,y1,x2,y2\n");
        for (int i = 0; i <= 100; i++) {
            double t = (2.0 * M_PI * i) / 100.0;
            double x1 = funcs[0].enabled ? evaluate(funcs[0].expr, t) : 0.0;
            double y1 = funcs[1].enabled ? evaluate(funcs[1].expr, t) : 0.0;
            double x2 = funcs[2].enabled ? evaluate(funcs[2].expr, t) : 0.0;
            double y2 = 3.0 * sin(t);
            fprintf(f, "%.5f,%.5f,%.5f,%.5f\n", t, x1, y1, x2, y2);
        }
    }
    fclose(f);
    ShowNativeStatus("Exported 101 sample points to KGraph_points.csv [E]!");
}

static void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd,
        "KGraph Studio - Usage Guide & Shortcuts:\n\n"
        "[Modes & Navigation]\n"
        "• Mode [M] / Button : Cycle Cartesian y(x), Polar r(th), Parametric (x,y)(t)\n"
        "• Arrow Keys (Left/Right/Up/Down) : Smoothly pan graph viewport\n"
        "• Left Click & Drag : Pan viewport across canvas\n"
        "• Mouse Scroll Wheel or +/- Keys : Smooth zoom in / out\n"
        "• Reset [R] / Home Key : Restore origin & default [-10, 10] viewport\n"
        "• Hover Mouse : Instant coordinates and numerical derivative tracer f'(x)\n\n"
        "[Calculus, Audio & Analysis]\n"
        "• Space : Sonify active curve with authentic PCM retro audio\n"
        "• Tangent [T] : Toggle instantaneous tangent & normal lines with slope m\n"
        "• Integral [I] : Compute Simpson's definite integral with shaded region\n"
        "• Roots : Find and highlight numerical roots in view\n"
        "• Presets [P] : Cycle popular mathematical curves & formulas\n"
        "• Save BMP [S] / Ctrl+S : Save graph snapshot to KGraph_snapshot.bmp\n"
        "• Export CSV [E] : Save sampled coordinates to KGraph_points.csv\n"
        "• Copy [C] / Ctrl+C : Copy sampled points to Windows clipboard\n"
        "• Clr Buttons : Quick 1-click expression clear\n"
        "• F1 or H : Open this Help guide",
        "KGraph Studio Help & Hotkeys", MB_OK | MB_ICONINFORMATION);
}

static LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_RETURN) {
            HWND hParent = GetParent(hwnd);
            if (hParent) SendMessageA(hParent, WM_COMMAND, 1001, 0);
            return 0;
        } else if (wParam == VK_ESCAPE) {
            HWND hParent = GetParent(hwnd);
            if (hParent) SetFocus(hParent);
            return 0;
        } else if (wParam == VK_F1) {
            HWND hParent = GetParent(hwnd);
            if (hParent) ShowHelpDialog(hParent);
            return 0;
        }
    }
    return CallWindowProcA(g_oldEditProc, hwnd, msg, wParam, lParam);
}

static void SaveGraphBMP(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    int canvasTop = g_canvasTop;
    int w = rect.right - rect.left;
    int h = rect.bottom - canvasTop;
    if (w <= 0 || h <= 0) return;

    HDC hdc = GetDC(hwnd);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBm = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP oldBm = (HBITMAP)SelectObject(memDC, hBm);

    BitBlt(memDC, 0, 0, w, h, hdc, 0, canvasTop, SRCCOPY);

    BITMAPINFOHEADER bih = {0};
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = w;
    bih.biHeight = h;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;

    int rowSize = ((w * 3 + 3) & ~3);
    int imageBytes = rowSize * h;
    BYTE* pPixels = (BYTE*)malloc(imageBytes);
    if (pPixels) {
        GetDIBits(memDC, hBm, 0, h, pPixels, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

        BITMAPFILEHEADER bfh = {0};
        bfh.bfType = 0x4D42; // 'BM'
        bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bfh.bfSize = bfh.bfOffBits + imageBytes;

        FILE* f = fopen("KGraph_snapshot.bmp", "wb");
        if (f) {
            fwrite(&bfh, sizeof(bfh), 1, f);
            fwrite(&bih, sizeof(bih), 1, f);
            fwrite(pPixels, 1, imageBytes, f);
            fclose(f);
            ShowNativeStatus("Saved snapshot to KGraph_snapshot.bmp [S]!");
        } else {
            ShowNativeStatus("Failed to save KGraph_snapshot.bmp");
        }
        free(pPixels);
    }
    SelectObject(memDC, oldBm);
    DeleteObject(hBm);
    DeleteDC(memDC);
    ReleaseDC(hwnd, hdc);
}

static void CopyPointsToClipboard(HWND hwnd) {
    char buf[4096];
    int len = 0;
    if (g_mode == MODE_CARTESIAN) {
        len += sprintf(buf + len, "x,y1,y2,y3\r\n");
        double minX = view_cx - view_scale;
        double maxX = view_cx + view_scale;
        double dx = (maxX - minX) / 25.0;
        for (int i = 0; i <= 25 && len < 3800; i++) {
            double x = minX + i * dx;
            double y1 = funcs[0].enabled ? evaluate(funcs[0].expr, x) : 0.0;
            double y2 = funcs[1].enabled ? evaluate(funcs[1].expr, x) : 0.0;
            double y3 = funcs[2].enabled ? evaluate(funcs[2].expr, x) : 0.0;
            len += sprintf(buf + len, "%.4f,%.4f,%.4f,%.4f\r\n", x, y1, y2, y3);
        }
    } else {
        len += sprintf(buf + len, "KGraph State:\r\nCenter=(%.4f, %.4f)\r\nScale=%.4f\r\n", view_cx, view_cy, view_scale);
    }

    if (OpenClipboard(hwnd)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len + 1);
        if (hMem) {
            char* pMem = (char*)GlobalLock(hMem);
            if (pMem) {
                for (int ci = 0; ci <= len; ci++) pMem[ci] = buf[ci];
                GlobalUnlock(hMem);
                SetClipboardData(CF_TEXT, hMem);
            }
        }
        CloseClipboard();
        ShowNativeStatus("Copied sample coordinates to clipboard [C]!");
    }
}

static void UpdateModeUI(void) {
    if (g_mode == MODE_CARTESIAN) {
        SetWindowTextA(hModeBtn, "Mode: Cartesian [M]");
        SetWindowTextA(hLabels[0], "y1 =");
        SetWindowTextA(hLabels[1], "y2 =");
        SetWindowTextA(hLabels[2], "y3 =");
        SetWindowTextA(hInputs[0], "sin(x)");
        SetWindowTextA(hInputs[1], "cos(x)");
        SetWindowTextA(hInputs[2], "x^2/4 - 2");
        ShowNativeStatus("Cartesian Mode y(x). Scroll or Arrow keys to pan. F1 for Help.");
        if (g_hWnd) SetWindowTextA(g_hWnd, "KGraph Studio - Cartesian [M: Mode | P: Presets | R: Reset | F1: Help]");
    } else if (g_mode == MODE_POLAR) {
        SetWindowTextA(hModeBtn, "Mode: Polar [M]");
        SetWindowTextA(hLabels[0], "r1 =");
        SetWindowTextA(hLabels[1], "r2 =");
        SetWindowTextA(hLabels[2], "r3 =");
        SetWindowTextA(hInputs[0], "3*cos(4*t)");
        SetWindowTextA(hInputs[1], "2*(1 - cos(t))");
        SetWindowTextA(hInputs[2], "t / 2");
        ShowNativeStatus("Polar Mode r(th). Concentric polar grid with radial spokes. F1 for Help.");
        if (g_hWnd) SetWindowTextA(g_hWnd, "KGraph Studio - Polar [M: Mode | P: Presets | R: Reset | F1: Help]");
    } else if (g_mode == MODE_PARAMETRIC) {
        SetWindowTextA(hModeBtn, "Mode: Parametric [M]");
        SetWindowTextA(hLabels[0], "x1 =");
        SetWindowTextA(hLabels[1], "y1 =");
        SetWindowTextA(hLabels[2], "x2 =");
        SetWindowTextA(hInputs[0], "4*cos(3*t)");
        SetWindowTextA(hInputs[1], "4*sin(2*t)");
        SetWindowTextA(hInputs[2], "3*cos(t)");
        ShowNativeStatus("Parametric Mode (x(t), y(t)). Trajectories and Lissajous curves. F1 for Help.");
        if (g_hWnd) SetWindowTextA(g_hWnd, "KGraph Studio - Parametric [M: Mode | P: Presets | R: Reset | F1: Help]");
    }
    for (int i = 0; i < MAX_FUNCS; i++) {
        GetWindowTextA(hInputs[i], funcs[i].expr, 127);
        funcs[i].enabled = (SendMessageA(hChecks[i], BM_GETCHECK, 0, 0) == BST_CHECKED);
    }
}

static void FindRootsInView(void) {
    root_count = 0;
    if (g_mode != MODE_CARTESIAN) return;

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
            HDC hdc = GetDC(hwnd);
            g_dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(hwnd, hdc);
            
            g_canvasTop = MulDiv(135, g_dpi, 96);
            int fontSize = -MulDiv(15, g_dpi, 96);

            hFontSmall = CreateFontA(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hFontBold = CreateFontA(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            hTopBgBrush = CreateSolidBrush(RGB(23, 27, 44));
            hEditBgBrush = CreateSolidBrush(RGB(15, 23, 42));

            g_hWnd = hwnd;
            int topY = MulDiv(6, g_dpi, 96);
            for (int i = 0; i < MAX_FUNCS; i++) {
                char label[16];
                sprintf(label, "y%d =", i + 1);
                hLabels[i] = CreateWindowA("STATIC", label, WS_CHILD | WS_VISIBLE, MulDiv(10, g_dpi, 96), topY + MulDiv(4, g_dpi, 96), MulDiv(30, g_dpi, 96), MulDiv(20, g_dpi, 96), hwnd, NULL, NULL, NULL);
                hInputs[i] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", funcs[i].expr, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, MulDiv(44, g_dpi, 96), topY, MulDiv(196, g_dpi, 96), MulDiv(24, g_dpi, 96), hwnd, NULL, NULL, NULL);
                hChecks[i] = CreateWindowA("BUTTON", "Show", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, MulDiv(245, g_dpi, 96), topY + MulDiv(2, g_dpi, 96), MulDiv(52, g_dpi, 96), MulDiv(20, g_dpi, 96), hwnd, (HMENU)(1100 + i), NULL, NULL);
                hClrBtn[i] = CreateWindowA("BUTTON", "Clr", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(300, g_dpi, 96), topY + MulDiv(1, g_dpi, 96), MulDiv(34, g_dpi, 96), MulDiv(22, g_dpi, 96), hwnd, (HMENU)(1120 + i), NULL, NULL);
                SendMessageA(hChecks[i], BM_SETCHECK, funcs[i].enabled ? BST_CHECKED : BST_UNCHECKED, 0);

                SendMessageA(hLabels[i], WM_SETFONT, (WPARAM)hFontSmall, TRUE);
                SendMessageA(hInputs[i], WM_SETFONT, (WPARAM)hFontSmall, TRUE);
                SendMessageA(hChecks[i], WM_SETFONT, (WPARAM)hFontSmall, TRUE);
                SendMessageA(hClrBtn[i], WM_SETFONT, (WPARAM)hFontSmall, TRUE);

                if (!g_oldEditProc) {
                    g_oldEditProc = (WNDPROC)GetWindowLongPtrA(hInputs[i], GWLP_WNDPROC);
                }
                SetWindowLongPtrA(hInputs[i], GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

                topY += MulDiv(28, g_dpi, 96);
            }

            // Right side toolbar row 1 (Y=6)
            hModeBtn  = CreateWindowA("BUTTON", "Mode: Cartesian [M]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(344, g_dpi, 96), MulDiv(6, g_dpi, 96), MulDiv(140, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1008, NULL, NULL);
            hPlotBtn  = CreateWindowA("BUTTON", "Plot [Enter]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(488, g_dpi, 96), MulDiv(6, g_dpi, 96), MulDiv(78, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1001, NULL, NULL);
            hZoomIn   = CreateWindowA("BUTTON", "+", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(570, g_dpi, 96), MulDiv(6, g_dpi, 96), MulDiv(26, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1002, NULL, NULL);
            hZoomOut  = CreateWindowA("BUTTON", "-", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(600, g_dpi, 96), MulDiv(6, g_dpi, 96), MulDiv(26, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1003, NULL, NULL);
            hResetBtn = CreateWindowA("BUTTON", "Reset [R]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(630, g_dpi, 96), MulDiv(6, g_dpi, 96), MulDiv(68, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1004, NULL, NULL);
            hHelpBtn  = CreateWindowA("BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(702, g_dpi, 96), MulDiv(6, g_dpi, 96), MulDiv(68, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1007, NULL, NULL);

            // Right side toolbar row 2 (Y=35)
            hRootsBtn    = CreateWindowA("BUTTON", "Roots", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(344, g_dpi, 96), MulDiv(35, g_dpi, 96), MulDiv(58, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1005, NULL, NULL);
            hIntegralBtn = CreateWindowA("BUTTON", "Integral [I]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(406, g_dpi, 96), MulDiv(35, g_dpi, 96), MulDiv(78, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1011, NULL, NULL);
            hTangentBtn  = CreateWindowA("BUTTON", "Tangent [T]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(488, g_dpi, 96), MulDiv(35, g_dpi, 96), MulDiv(82, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1012, NULL, NULL);
            hSonifyBtn   = CreateWindowA("BUTTON", "Sonify [Space]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(574, g_dpi, 96), MulDiv(35, g_dpi, 96), MulDiv(96, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1013, NULL, NULL);
            hPresetBtn   = CreateWindowA("BUTTON", "Presets [P]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(674, g_dpi, 96), MulDiv(35, g_dpi, 96), MulDiv(96, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1006, NULL, NULL);

            // Right side toolbar row 3 (Y=64)
            hSaveBtn      = CreateWindowA("BUTTON", "Save BMP [S]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(344, g_dpi, 96), MulDiv(64, g_dpi, 96), MulDiv(96, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1009, NULL, NULL);
            hExportCsvBtn = CreateWindowA("BUTTON", "Export CSV [E]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(444, g_dpi, 96), MulDiv(64, g_dpi, 96), MulDiv(104, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1014, NULL, NULL);
            hCopyBtn      = CreateWindowA("BUTTON", "Copy [C]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, MulDiv(552, g_dpi, 96), MulDiv(64, g_dpi, 96), MulDiv(68, g_dpi, 96), MulDiv(26, g_dpi, 96), hwnd, (HMENU)1010, NULL, NULL);

            // Status bar at row 4 (Y=94)
            hStatus = CreateWindowA("STATIC", "Welcome to KGraph Studio! Drag or use Arrow keys to pan, scroll wheel or +/- to zoom, Space to Sonify, F1 for Help.", WS_CHILD | WS_VISIBLE | SS_LEFT, MulDiv(10, g_dpi, 96), MulDiv(94, g_dpi, 96), MulDiv(760, g_dpi, 96), MulDiv(22, g_dpi, 96), hwnd, NULL, NULL, NULL);
            SendMessageA(hStatus, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

            SendMessageA(hModeBtn, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hPlotBtn, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hZoomIn, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hZoomOut, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hResetBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hRootsBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hIntegralBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hTangentBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hSonifyBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hPresetBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hHelpBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hSaveBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hExportCsvBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            SendMessageA(hCopyBtn, WM_SETFONT, (WPARAM)hFontSmall, TRUE);
            
            SetTimer(hwnd, 2001, 5000, NULL);
            break;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(226, 232, 240));
            SetBkColor(hdcStatic, RGB(23, 27, 44));
            return (INT_PTR)hTopBgBrush;
        }

        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            SetTextColor(hdcEdit, RGB(248, 250, 252));
            SetBkColor(hdcEdit, RGB(15, 23, 42));
            return (INT_PTR)hEditBgBrush;
        }

        case WM_GETMINMAXINFO: {
            MINMAXINFO* mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = MulDiv(760, g_dpi, 96);
            mmi->ptMinTrackSize.y = MulDiv(480, g_dpi, 96);
            return 0;
        }

        case WM_SIZE: {
            RECT rc;
            GetClientRect(hwnd, &rc);
            int clientW = rc.right - rc.left;
            if (hStatus && clientW > 0) {
                SetWindowPos(hStatus, NULL, MulDiv(10, g_dpi, 96), MulDiv(94, g_dpi, 96), clientW - MulDiv(20, g_dpi, 96), MulDiv(22, g_dpi, 96), SWP_NOZORDER);
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_TIMER: {
            if (wParam == 2001) {
                KillTimer(hwnd, 2001);
                if (g_mode == MODE_CARTESIAN) {
                    SetWindowTextA(hStatus, "Cartesian Mode y(x). Scroll or Arrow keys to pan. F1 for Help.");
                } else if (g_mode == MODE_POLAR) {
                    SetWindowTextA(hStatus, "Polar Mode r(th). Scroll or Arrow keys to pan. F1 for Help.");
                } else {
                    SetWindowTextA(hStatus, "Parametric Mode (x,y)(t). Scroll or Arrow keys to pan. F1 for Help.");
                }
            }
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 1001) { // Plot
                for (int i = 0; i < MAX_FUNCS; i++) {
                    GetWindowTextA(hInputs[i], funcs[i].expr, 127);
                    funcs[i].enabled = (SendMessageA(hChecks[i], BM_GETCHECK, 0, 0) == BST_CHECKED);
                }
                ShowNativeStatus("Graph plotted successfully!");
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (id == 1002) { // Zoom In
                view_scale *= 0.75;
                ShowNativeStatus("Zoom In (+)");
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (id == 1003) { // Zoom Out
                view_scale *= 1.3333;
                ShowNativeStatus("Zoom Out (-)");
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (id == 1004) { // Reset
                view_cx = 0.0; view_cy = 0.0; view_scale = 10.0;
                ShowNativeStatus("Viewport reset to [-10, 10] at origin.");
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (id == 1005) { // Find Roots
                if (g_mode == MODE_CARTESIAN) {
                    FindRootsInView();
                    char msgBuf[128];
                    sprintf(msgBuf, "Found %d root(s) in current view range.", root_count);
                    ShowNativeStatus(msgBuf);
                } else {
                    ShowNativeStatus("Root finder is active in Cartesian mode.");
                }
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (id == 1006) { // Presets
                if (g_mode == MODE_CARTESIAN) {
                    static int cartPresetIdx = 0;
                    const char* p1[6] = {"exp(-x^2)", "sin(x)*cos(2*x)", "1/(1+x^2)", "3*exp(-abs(x)/3)*cos(4*x)", "sin(x)+sin(3*x)/3+sin(5*x)/5", "8/(x^2+4)"};
                    const char* p2[6] = {"sin(3*x)", "x^2/4 - 2", "cos(x)", "sin(2*x)", "cos(3*x)", "exp(-x^2/2)"};
                    const char* p3[6] = {"x^3 - 3*x", "exp(-abs(x)/3)", "sin(x)", "cos(x)/2", "sin(x)", "x^2/6 - 3"};
                    const char* names[6] = {"Gaussian & Waves", "Harmonics & Parabola", "Cubic & Witch of Agnesi", "Damped Oscillation", "Fourier Square Wave", "Witch & Bell Curve"};
                    SetWindowTextA(hInputs[0], p1[cartPresetIdx]);
                    SetWindowTextA(hInputs[1], p2[cartPresetIdx]);
                    SetWindowTextA(hInputs[2], p3[cartPresetIdx]);
                    char pMsg[128];
                    sprintf(pMsg, "Loaded Cartesian Preset [%d/6]: %s", cartPresetIdx + 1, names[cartPresetIdx]);
                    ShowNativeStatus(pMsg);
                    cartPresetIdx = (cartPresetIdx + 1) % 6;
                } else if (g_mode == MODE_POLAR) {
                    static int polarPresetIdx = 0;
                    const char* p1[6] = {"4*sin(5*t)", "3*cos(4*t)", "2*(1-cos(t))", "sqrt(t)", "sqrt(abs(9*cos(2*t)))", "2+3*cos(t)"};
                    const char* p2[6] = {"3*(1 - sin(t))", "t/2", "3+2*cos(t)", "t/3", "2*cos(3*t)", "1-cos(t)"};
                    const char* p3[6] = {"sqrt(abs(9*cos(2*t)))", "sin(3*t)", "cos(2*t)", "sin(5*t)", "cos(t)", "2*sin(4*t)"};
                    const char* names[6] = {"Penta-Rose & Cardioid", "Cardioid & Archimedes", "Lemniscate & Rose", "Fermat & Archimedes Spiral", "Lemniscate & Trefoil", "Limaçon & Cardioid"};
                    SetWindowTextA(hInputs[0], p1[polarPresetIdx]);
                    SetWindowTextA(hInputs[1], p2[polarPresetIdx]);
                    SetWindowTextA(hInputs[2], p3[polarPresetIdx]);
                    char pMsg[128];
                    sprintf(pMsg, "Loaded Polar Preset [%d/6]: %s", polarPresetIdx + 1, names[polarPresetIdx]);
                    ShowNativeStatus(pMsg);
                    polarPresetIdx = (polarPresetIdx + 1) % 6;
                } else if (g_mode == MODE_PARAMETRIC) {
                    static int paramPresetIdx = 0;
                    const char* p1[6] = {"sin(t)*(exp(cos(t))-2*cos(4*t))", "4*cos(3*t)", "4*cos(t)^3", "4*sin(3*t)", "sin(t)+2*sin(2*t)", "4*cos(t)+2*cos(3*t)"};
                    const char* p2[6] = {"cos(t)*(exp(cos(t))-2*cos(4*t))", "4*sin(2*t)", "4*sin(t)^3", "4*cos(4*t)", "cos(t)-2*cos(2*t)", "4*sin(t)-2*sin(3*t)"};
                    const char* p3[6] = {"4*cos(t)^3", "3*cos(t)", "sin(2*t)", "cos(2*t)", "3*sin(t)", "sin(3*t)"};
                    const char* names[6] = {"Butterfly Curve", "Lissajous 3:2", "Astroid", "Lissajous 3:4", "Trefoil Projection", "Hypotrochoid"};
                    SetWindowTextA(hInputs[0], p1[paramPresetIdx]);
                    SetWindowTextA(hInputs[1], p2[paramPresetIdx]);
                    SetWindowTextA(hInputs[2], p3[paramPresetIdx]);
                    char pMsg[128];
                    sprintf(pMsg, "Loaded Parametric Preset [%d/6]: %s", paramPresetIdx + 1, names[paramPresetIdx]);
                    ShowNativeStatus(pMsg);
                    paramPresetIdx = (paramPresetIdx + 1) % 6;
                }
                SendMessageA(hChecks[0], BM_SETCHECK, BST_CHECKED, 0);
                SendMessageA(hChecks[1], BM_SETCHECK, BST_CHECKED, 0);
                SendMessageA(hChecks[2], BM_SETCHECK, BST_CHECKED, 0);
                SendMessageA(hwnd, WM_COMMAND, 1001, 0);
            } else if (id == 1007) { // Help
                ShowHelpDialog(hwnd);
            } else if (id == 1008) { // Mode Toggle
                g_mode = (PlotMode)((g_mode + 1) % 3);
                UpdateModeUI();
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (id == 1009) { // Save BMP
                SaveGraphBMP(hwnd);
            } else if (id == 1010) { // Copy Points
                CopyPointsToClipboard(hwnd);
            } else if (id == 1011) { // Definite Integral
                if (g_mode == MODE_CARTESIAN) {
                    g_showIntegral = !g_showIntegral;
                    if (g_showIntegral) {
                        double a = view_cx - 0.5 * view_scale;
                        double b = view_cx + 0.5 * view_scale;
                        double area = EvalSimpsonIntegral(funcs[0].expr, a, b, 200);
                        char msgBuf[128];
                        sprintf(msgBuf, "Simpson's Integral ∫[%.2f, %.2f] y1(x) dx = %.5f", a, b, area);
                        ShowNativeStatus(msgBuf);
                    } else {
                        ShowNativeStatus("Definite integral shading hidden.");
                    }
                } else {
                    ShowNativeStatus("Definite integral calculation active in Cartesian mode.");
                }
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (id == 1012) { // Tangent Line
                if (g_mode == MODE_CARTESIAN) {
                    g_showTangent = !g_showTangent;
                    ShowNativeStatus(g_showTangent ? "Tangent & Normal line tracer enabled [T]." : "Tangent line disabled.");
                } else {
                    ShowNativeStatus("Tangent & slope tracer active in Cartesian mode.");
                }
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (id == 1013) { // Sonify
                SonifyActiveCurve(hwnd);
            } else if (id == 1014) { // Export CSV
                SaveCSVData(hwnd);
            } else if (id >= 1100 && id < 1100 + MAX_FUNCS) {
                int idx = id - 1100;
                funcs[idx].enabled = (SendMessageA(hChecks[idx], BM_GETCHECK, 0, 0) == BST_CHECKED);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (id >= 1120 && id < 1120 + MAX_FUNCS) {
                int idx = id - 1120;
                SetWindowTextA(hInputs[idx], "");
                funcs[idx].expr[0] = '\0';
                SetFocus(hInputs[idx]);
                char clrMsg[64];
                sprintf(clrMsg, "Cleared expression %d", idx + 1);
                ShowNativeStatus(clrMsg);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }

        case WM_MOUSEWHEEL: {
            short zDelta = (short)HIWORD(wParam);
            if (zDelta > 0) {
                view_scale *= 0.85;
            } else {
                view_scale *= 1.18;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_KEYDOWN: {
            if (wParam == 'H' || wParam == 'h' || wParam == VK_F1) {
                ShowHelpDialog(hwnd);
            } else if (wParam == 'M' || wParam == 'm') {
                SendMessageA(hwnd, WM_COMMAND, 1008, 0);
            } else if (wParam == 'R' || wParam == 'r' || wParam == VK_HOME) {
                SendMessageA(hwnd, WM_COMMAND, 1004, 0);
            } else if (wParam == 'P' || wParam == 'p') {
                SendMessageA(hwnd, WM_COMMAND, 1006, 0);
            } else if (wParam == 'S' || wParam == 's') {
                SendMessageA(hwnd, WM_COMMAND, 1009, 0);
            } else if (wParam == 'C' || wParam == 'c') {
                SendMessageA(hwnd, WM_COMMAND, 1010, 0);
            } else if (wParam == 'I' || wParam == 'i') {
                SendMessageA(hwnd, WM_COMMAND, 1011, 0);
            } else if (wParam == 'T' || wParam == 't') {
                SendMessageA(hwnd, WM_COMMAND, 1012, 0);
            } else if (wParam == VK_SPACE) {
                SendMessageA(hwnd, WM_COMMAND, 1013, 0);
            } else if (wParam == 'E' || wParam == 'e') {
                SendMessageA(hwnd, WM_COMMAND, 1014, 0);
            } else if (wParam == VK_ADD || wParam == VK_OEM_PLUS || wParam == VK_PRIOR) {
                SendMessageA(hwnd, WM_COMMAND, 1002, 0);
            } else if (wParam == VK_SUBTRACT || wParam == VK_OEM_MINUS || wParam == VK_NEXT) {
                SendMessageA(hwnd, WM_COMMAND, 1003, 0);
            } else if (wParam == VK_LEFT) {
                view_cx -= 0.15 * view_scale;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == VK_RIGHT) {
                view_cx += 0.15 * view_scale;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == VK_UP) {
                view_cy += 0.15 * view_scale;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == VK_DOWN) {
                view_cy -= 0.15 * view_scale;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == '1' || wParam == '2' || wParam == '3') {
                int idx = wParam - '1';
                funcs[idx].enabled = !funcs[idx].enabled;
                SendMessageA(hChecks[idx], BM_SETCHECK, funcs[idx].enabled ? BST_CHECKED : BST_UNCHECKED, 0);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == VK_RETURN) {
                SendMessageA(hwnd, WM_COMMAND, 1001, 0);
            }
            break;
        }

        case WM_LBUTTONDOWN: {
            if (HIWORD(lParam) > g_canvasTop) {
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
            int canvasTop = g_canvasTop;
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
                double worldY = view_cy + view_scale - ((double)(my - canvasTop) / h) * (2.0 * view_scale);
                char statusText[256];
                
                if (g_mode == MODE_CARTESIAN) {
                    int offset = sprintf(statusText, "x = %.4f | ", worldX);
                    for (int i = 0; i < MAX_FUNCS; i++) {
                        if (funcs[i].enabled) {
                            double yVal = evaluate(funcs[i].expr, worldX);
                            double dyVal = eval_derivative(funcs[i].expr, worldX);
                            offset += sprintf(statusText + offset, "y%d=%.3f (f'=%.2f)  ", i + 1, yVal, dyVal);
                        }
                    }
                    SetWindowTextA(hStatus, statusText);
                } else if (g_mode == MODE_POLAR) {
                    double rVal = sqrt(worldX * worldX + worldY * worldY);
                    double thVal = atan2(worldY, worldX);
                    if (thVal < 0) thVal += 2.0 * M_PI;
                    sprintf(statusText, "Polar Hover | (x, y) = (%.3f, %.3f) | r = %.3f, th = %.3f rad (%.1f deg)", worldX, worldY, rVal, thVal, thVal * 180.0 / M_PI);
                    SetWindowTextA(hStatus, statusText);
                } else if (g_mode == MODE_PARAMETRIC) {
                    sprintf(statusText, "Parametric Hover | Canvas (x, y) = (%.3f, %.3f)", worldX, worldY);
                    SetWindowTextA(hStatus, statusText);
                }
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
            int canvasTop = g_canvasTop;
            RECT topRect = {rect.left, 0, rect.right, canvasTop};
            RECT canvasRect = {rect.left, canvasTop, rect.right, rect.bottom};

            // Paint top toolbar background
            FillRect(hdc, &topRect, hTopBgBrush);

            // Double Buffering for canvas
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
                HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(35, 42, 60));
                HPEN axisPen = CreatePen(PS_SOLID, 2, RGB(90, 105, 130));

                int axis_x = rect.left + (int)((0.0 - (view_cx - view_scale)) / (2.0 * view_scale) * w);
                int axis_y = canvasTop + (int)(h - (0.0 - (view_cy - view_scale)) / (2.0 * view_scale) * h);

                if (g_mode == MODE_POLAR) {
                    // Polar Grid: Concentric rings and radial spokes
                    SelectObject(memDC, gridPen);
                    for (int r = 1; r <= 20; r++) {
                        int rx = (int)((double)r / (2.0 * view_scale) * w);
                        int ry = (int)((double)r / (2.0 * view_scale) * h);
                        if (rx > 0 && axis_x + rx >= rect.left && axis_x - rx <= rect.right) {
                            HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                            HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, nullBrush);
                            Ellipse(memDC, axis_x - rx, axis_y - ry, axis_x + rx, axis_y + ry);
                            SelectObject(memDC, oldBrush);
                        }
                    }

                    // Radial Spokes
                    for (int i = 0; i < 12; i++) {
                        double th = i * (M_PI / 6.0);
                        int endX = axis_x + (int)(cos(th) * w);
                        int endY = axis_y - (int)(sin(th) * h);
                        MoveToEx(memDC, axis_x, axis_y, NULL);
                        LineTo(memDC, endX, endY);
                    }
                } else {
                    // Cartesian Grid
                    SelectObject(memDC, gridPen);
                    for (int i = -10; i <= 10; i++) {
                        int px = w / 2 + (i * w / 20);
                        MoveToEx(memDC, px, canvasTop, NULL);
                        LineTo(memDC, px, rect.bottom);

                        int py = canvasTop + h / 2 - (i * h / 20);
                        MoveToEx(memDC, rect.left, py, NULL);
                        LineTo(memDC, rect.right, py);
                    }
                }

                // Axes
                SelectObject(memDC, axisPen);
                if (axis_x >= rect.left && axis_x <= rect.right) {
                    MoveToEx(memDC, axis_x, canvasTop, NULL); LineTo(memDC, axis_x, rect.bottom);
                }
                if (axis_y >= canvasTop && axis_y <= rect.bottom) {
                    MoveToEx(memDC, rect.left, axis_y, NULL); LineTo(memDC, rect.right, axis_y);
                }

                // Plot Curves according to Mode
                if (g_mode == MODE_CARTESIAN) {
                    // Shaded Definite Integral Region
                    if (g_showIntegral && funcs[0].enabled) {
                        double intA = view_cx - 0.5 * view_scale;
                        double intB = view_cx + 0.5 * view_scale;
                        int startPx = rect.left + (int)((intA - (view_cx - view_scale)) / (2.0 * view_scale) * w);
                        int endPx = rect.left + (int)((intB - (view_cx - view_scale)) / (2.0 * view_scale) * w);
                        if (startPx < rect.left) startPx = rect.left;
                        if (endPx > rect.right) endPx = rect.right;

                        if (startPx < endPx) {
                            POINT poly[320];
                            int polyCount = 0;
                            poly[polyCount].x = startPx;
                            poly[polyCount].y = axis_y;
                            polyCount++;

                            for (int px = startPx; px <= endPx && polyCount < 310; px += 2) {
                                double x = view_cx - view_scale + ((double)(px - rect.left) / w) * (2.0 * view_scale);
                                double y = evaluate(funcs[0].expr, x);
                                if (!isnan(y)) {
                                    int py = canvasTop + (int)(h - (y - (view_cy - view_scale)) / (2.0 * view_scale) * h);
                                    poly[polyCount].x = px;
                                    poly[polyCount].y = py;
                                    polyCount++;
                                }
                            }
                            poly[polyCount].x = endPx;
                            poly[polyCount].y = axis_y;
                            polyCount++;

                            HBRUSH intBrush = CreateSolidBrush(RGB(18, 48, 76));
                            HPEN intPen = CreatePen(PS_SOLID, 1, RGB(56, 189, 248));
                            SelectObject(memDC, intBrush);
                            SelectObject(memDC, intPen);
                            Polygon(memDC, poly, polyCount);
                            DeleteObject(intBrush);
                            DeleteObject(intPen);
                        }
                    }

                    for (int idx = 0; idx < MAX_FUNCS; idx++) {
                        if (!funcs[idx].enabled) continue;
                        HPEN plotPen = CreatePen(PS_SOLID, 2, funcs[idx].color);
                        SelectObject(memDC, plotPen);
                        int first = 1;

                        for (int px = 0; px <= w; px += 2) {
                            double x = view_cx - view_scale + ((double)px / w) * (2.0 * view_scale);
                            double y = evaluate(funcs[idx].expr, x);
                            if (isnan(y)) { first = 1; continue; }
                            int py = canvasTop + (int)(h - (y - (view_cy - view_scale)) / (2.0 * view_scale) * h);

                            if (py >= canvasTop - 500 && py <= rect.bottom + 500) {
                                if (first) { MoveToEx(memDC, rect.left + px, py, NULL); first = 0; }
                                else { LineTo(memDC, rect.left + px, py); }
                            } else { first = 1; }
                        }
                        DeleteObject(plotPen);
                    }

                    // Tangent and Normal Line Overlay
                    if (g_showTangent && hover_mouse.y >= canvasTop && funcs[0].enabled) {
                        double hoverX = view_cx - view_scale + ((double)hover_mouse.x / w) * (2.0 * view_scale);
                        double hoverY = evaluate(funcs[0].expr, hoverX);
                        double slope = eval_derivative(funcs[0].expr, hoverX);
                        if (!isnan(hoverY) && !isnan(slope)) {
                            double xLeft = view_cx - view_scale;
                            double xRight = view_cx + view_scale;
                            double yLeft = hoverY + slope * (xLeft - hoverX);
                            double yRight = hoverY + slope * (xRight - hoverX);
                            int pyLeft = canvasTop + (int)(h - (yLeft - (view_cy - view_scale)) / (2.0 * view_scale) * h);
                            int pyRight = canvasTop + (int)(h - (yRight - (view_cy - view_scale)) / (2.0 * view_scale) * h);

                            HPEN tanPen = CreatePen(PS_SOLID, 2, RGB(251, 191, 36));
                            SelectObject(memDC, tanPen);
                            MoveToEx(memDC, rect.left, pyLeft, NULL);
                            LineTo(memDC, rect.right, pyRight);
                            DeleteObject(tanPen);

                            if (fabs(slope) > 1e-4) {
                                HPEN normPen = CreatePen(PS_DOT, 1, RGB(6, 182, 212));
                                SelectObject(memDC, normPen);
                                double normSlope = -1.0 / slope;
                                double ynLeft = hoverY + normSlope * (xLeft - hoverX);
                                double ynRight = hoverY + normSlope * (xRight - hoverX);
                                int pynLeft = canvasTop + (int)(h - (ynLeft - (view_cy - view_scale)) / (2.0 * view_scale) * h);
                                int pynRight = canvasTop + (int)(h - (ynRight - (view_cy - view_scale)) / (2.0 * view_scale) * h);
                                MoveToEx(memDC, rect.left, pynLeft, NULL);
                                LineTo(memDC, rect.right, pynRight);
                                DeleteObject(normPen);
                            }

                            int ptX = rect.left + (int)((hoverX - (view_cx - view_scale)) / (2.0 * view_scale) * w);
                            int ptY = canvasTop + (int)(h - (hoverY - (view_cy - view_scale)) / (2.0 * view_scale) * h);
                            HBRUSH ptBrush = CreateSolidBrush(RGB(251, 191, 36));
                            SelectObject(memDC, ptBrush);
                            Ellipse(memDC, ptX - 5, ptY - 5, ptX + 6, ptY + 6);
                            DeleteObject(ptBrush);
                        }
                    }

                    // Root Markers
                    HBRUSH rootBrush = CreateSolidBrush(RGB(52, 211, 153));
                    SelectObject(memDC, rootBrush);
                    for (int r = 0; r < root_count; r++) {
                        int rpx = rect.left + (int)((roots[r].x - (view_cx - view_scale)) / (2.0 * view_scale) * w);
                        int rpy = canvasTop + (int)(h - (roots[r].y - (view_cy - view_scale)) / (2.0 * view_scale) * h);
                        Ellipse(memDC, rpx - 4, rpy - 4, rpx + 5, rpy + 5);
                    }
                    DeleteObject(rootBrush);

                } else if (g_mode == MODE_POLAR) {
                    for (int idx = 0; idx < MAX_FUNCS; idx++) {
                        if (!funcs[idx].enabled) continue;
                        HPEN plotPen = CreatePen(PS_SOLID, 2, funcs[idx].color);
                        SelectObject(memDC, plotPen);
                        int first = 1;
                        int steps = 600;

                        for (int i = 0; i <= steps; i++) {
                            double th = (4.0 * M_PI * i) / steps;
                            double r = evaluate(funcs[idx].expr, th);
                            if (isnan(r)) { first = 1; continue; }
                            double x = r * cos(th);
                            double y = r * sin(th);
                            int px = rect.left + (int)((x - (view_cx - view_scale)) / (2.0 * view_scale) * w);
                            int py = canvasTop + (int)(h - (y - (view_cy - view_scale)) / (2.0 * view_scale) * h);

                            if (px >= rect.left - 500 && px <= rect.right + 500 && py >= canvasTop - 500 && py <= rect.bottom + 500) {
                                if (first) { MoveToEx(memDC, px, py, NULL); first = 0; }
                                else { LineTo(memDC, px, py); }
                            } else { first = 1; }
                        }
                        DeleteObject(plotPen);
                    }
                } else if (g_mode == MODE_PARAMETRIC) {
                    // Pair 1 (x1, y1) if both or x1 enabled
                    if (funcs[0].enabled) {
                        HPEN plotPen = CreatePen(PS_SOLID, 2, funcs[0].color);
                        SelectObject(memDC, plotPen);
                        int first = 1;
                        int steps = 600;
                        for (int i = 0; i <= steps; i++) {
                            double t = (2.0 * M_PI * i) / steps;
                            double x = evaluate(funcs[0].expr, t);
                            double y = evaluate(funcs[1].expr, t);
                            if (isnan(x) || isnan(y)) { first = 1; continue; }
                            int px = rect.left + (int)((x - (view_cx - view_scale)) / (2.0 * view_scale) * w);
                            int py = canvasTop + (int)(h - (y - (view_cy - view_scale)) / (2.0 * view_scale) * h);

                            if (first) { MoveToEx(memDC, px, py, NULL); first = 0; }
                            else { LineTo(memDC, px, py); }
                        }
                        DeleteObject(plotPen);
                    }
                    // Pair 2 (x2, 3*sin(t))
                    if (funcs[2].enabled) {
                        HPEN plotPen = CreatePen(PS_SOLID, 2, funcs[2].color);
                        SelectObject(memDC, plotPen);
                        int first = 1;
                        int steps = 600;
                        for (int i = 0; i <= steps; i++) {
                            double t = (2.0 * M_PI * i) / steps;
                            double x = evaluate(funcs[2].expr, t);
                            double y = 3.0 * sin(t);
                            if (isnan(x) || isnan(y)) { first = 1; continue; }
                            int px = rect.left + (int)((x - (view_cx - view_scale)) / (2.0 * view_scale) * w);
                            int py = canvasTop + (int)(h - (y - (view_cy - view_scale)) / (2.0 * view_scale) * h);

                            if (first) { MoveToEx(memDC, px, py, NULL); first = 0; }
                            else { LineTo(memDC, px, py); }
                        }
                        DeleteObject(plotPen);
                    }
                }

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
            if (hTopBgBrush) DeleteObject(hTopBgBrush);
            if (hEditBgBrush) DeleteObject(hEditBgBrush);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry(void) {
    SetProcessDPIAware();
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KGraphClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

    RegisterClassA(&wc);
    HDC screenDC = GetDC(NULL);
    int initial_dpi = GetDeviceCaps(screenDC, LOGPIXELSY);
    ReleaseDC(NULL, screenDC);
    
    RECT winRect = {0, 0, MulDiv(1024, initial_dpi, 96), MulDiv(768, initial_dpi, 96)};
    AdjustWindowRect(&winRect, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, FALSE);
    HWND hwnd = CreateWindowExA(0, "KGraphClass", "KGraph Studio - Cartesian [M: Mode | P: Presets | R: Reset | F1: Help]", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, winRect.right - winRect.left, winRect.bottom - winRect.top, NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1) {
                ShowHelpDialog(hwnd);
                continue;
            }
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                if (msg.wParam == 'S' || msg.wParam == 's') {
                    SendMessageA(hwnd, WM_COMMAND, 1009, 0);
                    continue;
                } else if (msg.wParam == 'C' || msg.wParam == 'c') {
                    SendMessageA(hwnd, WM_COMMAND, 1010, 0);
                    continue;
                }
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}

