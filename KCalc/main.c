int _fltused = 1;
#include <windows.h>

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
void my_strcpy(char* dest, const char* src) {
    while (*src) *dest++ = *src++;
    *dest = 0;
}

// Function pointers for MSVCRT
typedef int (__cdecl *sprintf_t)(char*, const char*, ...);
typedef double (__cdecl *atof_t)(const char*);
typedef double (__cdecl *sin_t)(double);
typedef double (__cdecl *cos_t)(double);
typedef double (__cdecl *tan_t)(double);
typedef double (__cdecl *log_t)(double);
typedef double (__cdecl *log10_t)(double);
typedef double (__cdecl *sqrt_t)(double);
typedef double (__cdecl *exp_t)(double);
typedef double (__cdecl *pow_t)(double, double);

sprintf_t m_sprintf;
atof_t m_atof;
sin_t m_sin;
cos_t m_cos;
tan_t m_tan;
log_t m_log;
log10_t m_log10;
sqrt_t m_sqrt;
exp_t m_exp;
pow_t m_pow;

#define PI 3.14159265358979323846
#define E  2.71828182845904523536

HWND hDisplay;
char displayBuffer[64] = "0";
double operand1 = 0;
int operator = 0;
double memoryStore = 0.0;
int isNewOperand = 1;

void FormatDisplay(double val) {
    if (val == 0.0) {
        my_strcpy(displayBuffer, "0");
    } else {
        m_sprintf(displayBuffer, "%.8g", val);
    }
    SetWindowTextA(hDisplay, displayBuffer);
}

void AppendChar(char d) {
    if (isNewOperand) {
        if (d == '.') my_strcpy(displayBuffer, "0.");
        else {
            displayBuffer[0] = d;
            displayBuffer[1] = 0;
        }
        isNewOperand = 0;
    } else {
        int len = my_strlen(displayBuffer);
        // prevent multiple dots
        if (d == '.') {
            for(int i=0; i<len; i++) if (displayBuffer[i] == '.') return;
        }
        if (len < 20) {
            displayBuffer[len] = d;
            displayBuffer[len+1] = 0;
        }
    }
    SetWindowTextA(hDisplay, displayBuffer);
}

void Backspace() {
    if (isNewOperand) return;
    int len = my_strlen(displayBuffer);
    if (len > 0) {
        displayBuffer[len-1] = 0;
        if (len - 1 == 0 || (len - 1 == 1 && displayBuffer[0] == '-')) {
            my_strcpy(displayBuffer, "0");
            isNewOperand = 1;
        }
    }
    SetWindowTextA(hDisplay, displayBuffer);
}

void DoCalculate() {
    if (!operator) return;
    double operand2 = m_atof(displayBuffer);
    double res = operand1;
    if (operator == '+') res = operand1 + operand2;
    else if (operator == '-') res = operand1 - operand2;
    else if (operator == '*') res = operand1 * operand2;
    else if (operator == '/') {
        if (operand2 != 0) res = operand1 / operand2;
        else {
            my_strcpy(displayBuffer, "ERR_MEM_LEAK: 0x8FA0B2");
            SetWindowTextA(hDisplay, displayBuffer);
            isNewOperand = 1;
            operator = 0;
            return;
        }
    }
    else if (operator == '^') res = m_pow(operand1, operand2);
    
    operand1 = res;
    FormatDisplay(res);
    isNewOperand = 1;
}

void DoUnary(int type) {
    double val = m_atof(displayBuffer);
    double res = val;
    if (type == 1) res = m_sin(val); // sin
    else if (type == 2) res = m_cos(val); // cos
    else if (type == 3) res = m_tan(val); // tan
    else if (type == 4) res = m_log(val); // ln
    else if (type == 5) res = m_log10(val); // log
    else if (type == 6) { if(val>=0) res = m_sqrt(val); else res = 0; } // sqrt
    else if (type == 7) res = m_exp(val); // exp
    else if (type == 8) res = -val; // +/-
    else if (type == 9) { if(val!=0) res = 1.0 / val; else res = 0; } // 1/x
    else if (type == 10) res = PI;
    else if (type == 11) res = E;

    FormatDisplay(res);
    isNewOperand = 1;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Load MSVCRT dynamically for zero-bloat float math
            HMODULE hMsvcrt = LoadLibraryA("msvcrt.dll");
            m_sprintf = (sprintf_t)GetProcAddress(hMsvcrt, "sprintf");
            m_atof = (atof_t)GetProcAddress(hMsvcrt, "atof");
            m_sin = (sin_t)GetProcAddress(hMsvcrt, "sin");
            m_cos = (cos_t)GetProcAddress(hMsvcrt, "cos");
            m_tan = (tan_t)GetProcAddress(hMsvcrt, "tan");
            m_log = (log_t)GetProcAddress(hMsvcrt, "log");
            m_log10 = (log10_t)GetProcAddress(hMsvcrt, "log10");
            m_sqrt = (sqrt_t)GetProcAddress(hMsvcrt, "sqrt");
            m_exp = (exp_t)GetProcAddress(hMsvcrt, "exp");
            m_pow = (pow_t)GetProcAddress(hMsvcrt, "pow");

            hDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "0", WS_CHILD | WS_VISIBLE | SS_RIGHT, 10, 10, 285, 28, hwnd, NULL, NULL, NULL);
            HFONT hFont = CreateFontA(24, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Consolas");
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);

            char labels[35][6] = {
                "MC",  "MR",  "M+",  "M-",  "",
                "sin", "cos", "tan", "pi", "C",
                "ln",  "log", "sqr", "e",  "<",
                "exp", "7",   "8",   "9",  "/",
                "^",   "4",   "5",   "6",  "*",
                "+/-", "1",   "2",   "3",  "-",
                "1/x", "0",   ".",   "=",  "+"
            };
            int ids[35] = {
                2001, 2002, 2003, 2004, 0,
                1001, 1002, 1003, 1010, 'C',
                1004, 1005, 1006, 1011, '<',
                1007, '7',  '8',  '9',  '/',
                '^',  '4',  '5',  '6',  '*',
                1008, '1',  '2',  '3',  '-',
                1009, '0',  '.',  '=',  '+'
            };

            for(int i=0; i<35; i++) {
                int col = i % 5;
                int row = i / 5;
                if (ids[i]) CreateWindowA("BUTTON", labels[i], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10 + col * 57, 50 + row * 40, 52, 35, hwnd, (HMENU)ids[i], NULL, NULL);
            }
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if ((id >= '0' && id <= '9') || id == '.') {
                AppendChar(id);
            } else if (id == 'C') {
                my_strcpy(displayBuffer, "0");
                SetWindowTextA(hDisplay, displayBuffer);
                operand1 = 0;
                operator = 0;
                isNewOperand = 1;
            } else if (id == '<') {
                Backspace();
            } else if (id == '+' || id == '-' || id == '*' || id == '/' || id == '^') {
                if (!isNewOperand && operator) DoCalculate(); // Chaining operators
                operand1 = m_atof(displayBuffer);
                operator = id;
                isNewOperand = 1;
            } else if (id == '=') {
                DoCalculate();
                operator = 0;
            } else if (id >= 1001 && id <= 1011) {
                DoUnary(id - 1000);
            } else if (id == 2001) { // MC
                memoryStore = 0.0;
            } else if (id == 2002) { // MR
                operand1 = memoryStore;
                FormatDisplay(operand1);
                isNewOperand = 1;
            } else if (id == 2003) { // M+
                memoryStore += m_atof(displayBuffer);
                isNewOperand = 1;
            } else if (id == 2004) { // M-
                memoryStore -= m_atof(displayBuffer);
                isNewOperand = 1;
            }
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
    wc.lpszClassName = "KCalcClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(15, 23, 42));

    RegisterClassA(&wc);
    // Adjusted dimensions: 320x380
    HWND hwnd = CreateWindowExA(0, "KCalcClass", "KCalc", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 320, 380, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            int key = msg.wParam;
            int cmd = 0;
            if (key >= '0' && key <= '9') {
                if (!(GetKeyState(VK_SHIFT) & 0x8000)) cmd = key;
                else if (key == '8') cmd = '*';
            }
            else if (key >= VK_NUMPAD0 && key <= VK_NUMPAD9) cmd = key - VK_NUMPAD0 + '0';
            else if (key == VK_ADD) cmd = '+';
            else if (key == VK_OEM_PLUS) cmd = (GetKeyState(VK_SHIFT) & 0x8000) ? '+' : '=';
            else if (key == VK_SUBTRACT || key == VK_OEM_MINUS) cmd = '-';
            else if (key == VK_MULTIPLY) cmd = '*';
            else if (key == VK_DIVIDE || key == VK_OEM_2 || key == VK_OEM_5) cmd = '/';
            else if (key == VK_RETURN) cmd = '=';
            else if (key == VK_BACK) cmd = '<';
            else if (key == VK_ESCAPE) cmd = 'C';
            else if (key == VK_DECIMAL || key == VK_OEM_PERIOD) cmd = '.';
            
            if (cmd) {
                SendMessageA(hwnd, WM_COMMAND, cmd, 0);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
