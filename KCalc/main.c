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
    while (s && s[len]) len++;
    return len;
}

void my_strcpy(char* dest, const char* src) {
    while (*src) *dest++ = *src++;
    *dest = 0;
}

void my_strcat(char* dest, const char* src) {
    while (*dest) dest++;
    while (*src) *dest++ = *src++;
    *dest = 0;
}

int my_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// MSVCRT Function Pointers
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
typedef double (__cdecl *fmod_t)(double, double);
typedef int (__cdecl *rand_t)(void);

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
fmod_t m_fmod;
rand_t m_rand;

#define PI 3.14159265358979323846
#define E  2.71828182845904523536

// Scientific Constants
#define CONST_C   299792458.0
#define CONST_H   6.62607015e-34
#define CONST_G   6.67430e-11
#define CONST_NA  6.02214076e23
#define CONST_R   8.314462618

int isDeg = 1;
HWND hDisplay, hSubDisplay, hStatusText;
HFONT hFontMain, hFontSub, hFontSmall;
HBRUSH hDisplayBgBrush, hEditBgBrush;
int dpiX = 96;
#define S(x) MulDiv((x), dpiX, 96)

char displayBuffer[64] = "0";
char subDisplayBuffer[64] = "";
double operand1 = 0;
int operator = 0;
double memoryStore = 0.0;
int isNewOperand = 1;
int currentMode = 0; // 0=Sci, 1=Fin, 2=Stats, 3=Const, 4=History
double statLastMean = 0.0;
double statLastSlope = 0.0;

// History structure
typedef struct {
    char expr[64];
    char res[32];
} HistoryEntry;

HistoryEntry historyTape[50];
int historyCount = 0;

// View Handles
HWND hModeBtns[5];
HWND hHelpBtn;
HWND hSciBtns[42];
HWND hFinControls[14];
HWND hStatsControls[14];
HWND hConstBtns[8];
HWND hHistControls[4];

// Control IDs
#define ID_MODE_SCI   3001
#define ID_MODE_FIN   3002
#define ID_MODE_STATS 3005
#define ID_MODE_CONST 3003
#define ID_MODE_HIST  3004
#define ID_HELP       3006

#define ID_FIN_CALC_PMT  4001
#define ID_FIN_CALC_FV   4002
#define ID_FIN_CALC_MARG 4003

#define ID_STATS_CALC_1VAR 4101
#define ID_STATS_CALC_2VAR 4102
#define ID_STATS_USE_MEAN  4103
#define ID_STATS_USE_SLOPE 4104

#define ID_HIST_LIST   5001
#define ID_HIST_RECALL 5002
#define ID_HIST_CLEAR  5003
#define ID_HIST_EXPORT 5004

int ParseDoubleList(const char* s, double* outArr, int maxCount) {
    int count = 0;
    while (*s && count < maxCount) {
        while (*s && (*s == ' ' || *s == ',' || *s == ';' || *s == '\t' || *s == '\r' || *s == '\n')) s++;
        if (!*s) break;
        outArr[count++] = m_atof(s);
        while (*s && *s != ' ' && *s != ',' && *s != ';' && *s != '\t' && *s != '\r' && *s != '\n') s++;
    }
    return count;
}

int ParsePointList(const char* s, double* outX, double* outY, int maxCount) {
    int count = 0;
    while (*s && count < maxCount) {
        while (*s && (*s == ' ' || *s == ';' || *s == ',' || *s == '\t' || *s == '\r' || *s == '\n' || *s == '(')) s++;
        if (!*s) break;
        outX[count] = m_atof(s);
        while (*s && *s != ',' && *s != ':' && *s != ' ' && *s != '\t' && *s != ';' && *s != '\n') s++;
        while (*s && (*s == ',' || *s == ':' || *s == ' ' || *s == '\t')) s++;
        if (!*s) break;
        outY[count] = m_atof(s);
        count++;
        while (*s && *s != ';' && *s != '\n' && *s != '\r' && *s != ')') s++;
        if (*s) s++;
    }
    return count;
}

void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd,
        "KCalc Pro — Scientific & Financial Suite\n\n"
        "Calculator Modes:\n"
        "  [1] Scientific Calculator & Keypad\n"
        "  [2] Financial (PMT Loans, FV, Margin)\n"
        "  [3] Statistics & Linear Regression\n"
        "  [4] Scientific Constants Library\n"
        "  [5] Calculation History Tape\n\n"
        "Keyboard Shortcuts:\n"
        "  0-9, .        : Input numbers and decimals\n"
        "  +, -, *, /, ^ : Arithmetic and powers\n"
        "  Enter or =    : Calculate result\n"
        "  Backspace     : Delete last character\n"
        "  Esc           : Clear all / Reset\n"
        "  D             : Toggle DEG / RAD mode\n"
        "  F1 or H       : Open this Help Guide\n"
        "  Ctrl+1 to 5   : Switch Calculator Modes\n\n"
        "Memory Registers:\n"
        "  MS=Store, MR=Recall, M+=Add, M-=Sub, MC=Clear",
        "KCalc Pro Help", MB_OK | MB_ICONINFORMATION);
}

void FormatDisplay(double val) {
    if (val != val || val > 1e308 || val < -1e308) {
        my_strcpy(displayBuffer, "Error");
    } else if (val == 0.0) {
        my_strcpy(displayBuffer, "0");
    } else {
        m_sprintf(displayBuffer, "%.10g", val);
    }
    SetWindowTextA(hDisplay, displayBuffer);
}

void UpdateStatusText() {
    char statusBuf[64];
    if (memoryStore != 0.0) {
        m_sprintf(statusBuf, "[%s] [M: %.6g] | Press 'H' or F1 for Help", isDeg ? "DEG" : "RAD", memoryStore);
    } else {
        m_sprintf(statusBuf, "[%s] | Press 'H' or F1 for Help", isDeg ? "DEG" : "RAD");
    }
    SetWindowTextA(hStatusText, statusBuf);
}

void AddHistoryEntry(const char* exprStr, const char* resStr) {
    if (historyCount < 50) {
        my_strcpy(historyTape[historyCount].expr, exprStr);
        my_strcpy(historyTape[historyCount].res, resStr);
        historyCount++;
    } else {
        for (int i = 0; i < 49; i++) {
            historyTape[i] = historyTape[i+1];
        }
        my_strcpy(historyTape[49].expr, exprStr);
        my_strcpy(historyTape[49].res, resStr);
    }

    if (hHistControls[0]) {
        char itemBuf[128];
        m_sprintf(itemBuf, "%s = %s", exprStr, resStr);
        SendMessageA(hHistControls[0], LB_ADDSTRING, 0, (LPARAM)itemBuf);
    }
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
        if (d == '.') {
            for (int i = 0; i < len; i++) if (displayBuffer[i] == '.') return;
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
    char opChar = (char)operator;
    
    if (operator == '+') res = operand1 + operand2;
    else if (operator == '-') res = operand1 - operand2;
    else if (operator == '*') res = operand1 * operand2;
    else if (operator == '/') {
        if (operand2 != 0.0) res = operand1 / operand2;
        else {
            my_strcpy(displayBuffer, "Error");
            SetWindowTextA(hDisplay, displayBuffer);
            isNewOperand = 1;
            operator = 0;
            return;
        }
    }
    else if (operator == '^') res = m_pow(operand1, operand2);
    else if (operator == '%') {
        if (operand2 != 0.0) res = m_fmod(operand1, operand2);
        else {
            my_strcpy(displayBuffer, "Error");
            SetWindowTextA(hDisplay, displayBuffer);
            isNewOperand = 1;
            operator = 0;
            return;
        }
    }
    
    char exprBuf[64], resBuf[32];
    m_sprintf(exprBuf, "%.6g %c %.6g", operand1, opChar, operand2);
    FormatDisplay(res);
    my_strcpy(resBuf, displayBuffer);

    m_sprintf(subDisplayBuffer, "%.6g %c %.6g =", operand1, opChar, operand2);
    SetWindowTextA(hSubDisplay, subDisplayBuffer);
    
    AddHistoryEntry(exprBuf, resBuf);

    operand1 = res;
    isNewOperand = 1;
}

void DoUnary(int type) {
    double val = m_atof(displayBuffer);
    double res = val;
    const char* funcName = "";

    if (type == 1) {
        double rad = isDeg ? (val * (PI / 180.0)) : val;
        res = m_sin(rad);
        funcName = "sin";
    }
    else if (type == 2) {
        double rad = isDeg ? (val * (PI / 180.0)) : val;
        res = m_cos(rad);
        funcName = "cos";
    }
    else if (type == 3) {
        double rad = isDeg ? (val * (PI / 180.0)) : val;
        res = m_tan(rad);
        funcName = "tan";
    }
    else if (type == 4) {
        if (val <= 0) { my_strcpy(displayBuffer, "Error"); SetWindowTextA(hDisplay, displayBuffer); isNewOperand = 1; return; }
        res = m_log(val); funcName = "ln";
    }
    else if (type == 5) {
        if (val <= 0) { my_strcpy(displayBuffer, "Error"); SetWindowTextA(hDisplay, displayBuffer); isNewOperand = 1; return; }
        res = m_log10(val); funcName = "log";
    }
    else if (type == 6) {
        if (val < 0) { my_strcpy(displayBuffer, "Error"); SetWindowTextA(hDisplay, displayBuffer); isNewOperand = 1; return; }
        res = m_sqrt(val); funcName = "sqrt";
    }
    else if (type == 7) { res = m_exp(val); funcName = "exp"; }
    else if (type == 8) {
        if (!isNewOperand && displayBuffer[0]) {
            if (displayBuffer[0] == '-') {
                for (int i = 0; displayBuffer[i]; i++) displayBuffer[i] = displayBuffer[i+1];
            } else {
                int len = my_strlen(displayBuffer);
                if (len < 60) {
                    for (int i = len; i >= 0; i--) displayBuffer[i+1] = displayBuffer[i];
                    displayBuffer[0] = '-';
                }
            }
            SetWindowTextA(hDisplay, displayBuffer);
            return;
        }
        res = -val;
    }
    else if (type == 9) {
        if (val == 0) { my_strcpy(displayBuffer, "Error"); SetWindowTextA(hDisplay, displayBuffer); isNewOperand = 1; return; }
        res = 1.0 / val; funcName = "1/";
    }
    else if (type == 10) res = PI;
    else if (type == 11) res = E;
    else if (type == 12) {
        if (val < 0 || val > 170) { my_strcpy(displayBuffer, "Error"); SetWindowTextA(hDisplay, displayBuffer); isNewOperand = 1; return; }
        int n = (int)val;
        double f = 1;
        for (int i = 2; i <= n; i++) f *= i;
        res = f; funcName = "fact";
    }
    else if (type == 13) { res = val < 0 ? -val : val; funcName = "abs"; }
    else if (type == 14) res = (double)m_rand() / 32767.0;
    else if (type == 15) { res = val * val; funcName = "sqr"; }
    else if (type == 16) { res = m_pow(10.0, val); funcName = "10^"; }

    char exprBuf[64], resBuf[32];
    if (funcName[0]) m_sprintf(exprBuf, "%s(%.6g)", funcName, val);
    else m_sprintf(exprBuf, "%.6g", val);

    FormatDisplay(res);
    my_strcpy(resBuf, displayBuffer);
    AddHistoryEntry(exprBuf, resBuf);

    isNewOperand = 1;
}

void SetViewMode(int mode) {
    currentMode = mode;
    // Show/Hide Scientific Keypad
    for (int i = 0; i < 40; i++) {
        if (hSciBtns[i]) ShowWindow(hSciBtns[i], mode == 0 ? SW_SHOW : SW_HIDE);
    }
    // Show/Hide Financial Controls
    for (int i = 0; i < 14; i++) {
        if (hFinControls[i]) ShowWindow(hFinControls[i], mode == 1 ? SW_SHOW : SW_HIDE);
    }
    // Show/Hide Statistics Controls
    for (int i = 0; i < 14; i++) {
        if (hStatsControls[i]) ShowWindow(hStatsControls[i], mode == 2 ? SW_SHOW : SW_HIDE);
    }
    // Show/Hide Constants Controls
    for (int i = 0; i < 7; i++) {
        if (hConstBtns[i]) ShowWindow(hConstBtns[i], mode == 3 ? SW_SHOW : SW_HIDE);
    }
    // Show/Hide History Controls
    for (int i = 0; i < 4; i++) {
        if (hHistControls[i]) ShowWindow(hHistControls[i], mode == 4 ? SW_SHOW : SW_HIDE);
    }
}

void ExportHistoryToFile() {
    HANDLE hFile = CreateFileA("kcalc_history.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char header[] = "=== KCalc History Tape ===\r\n\r\n";
        DWORD written;
        WriteFile(hFile, header, my_strlen(header), &written, NULL);
        
        for (int i = 0; i < historyCount; i++) {
            char line[128];
            m_sprintf(line, "[%d] %s = %s\r\n", i + 1, historyTape[i].expr, historyTape[i].res);
            WriteFile(hFile, line, my_strlen(line), &written, NULL);
        }
        CloseHandle(hFile);
        MessageBoxA(NULL, "History exported to kcalc_history.txt!", "KCalc", MB_OK | MB_ICONINFORMATION);
    }
}

BOOL CALLBACK SetFontCallback(HWND hwnd, LPARAM lParam) {
    SendMessageA(hwnd, WM_SETFONT, (WPARAM)lParam, TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HDC hdcScreen = GetDC(NULL);
            if (hdcScreen) {
                dpiX = GetDeviceCaps(hdcScreen, 88); // LOGPIXELSX
                ReleaseDC(NULL, hdcScreen);
            }
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
            m_fmod = (fmod_t)GetProcAddress(hMsvcrt, "fmod");
            m_rand = (rand_t)GetProcAddress(hMsvcrt, "rand");

            hDisplayBgBrush = CreateSolidBrush(RGB(15, 23, 42));
            hEditBgBrush = CreateSolidBrush(RGB(30, 41, 59));

            // Mode Selector Bar & Help Button
            hModeBtns[0] = CreateWindowA("BUTTON", "[1] Sci",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, S(10), S(8), S(52), S(26), hwnd, (HMENU)ID_MODE_SCI, NULL, NULL);
            hModeBtns[1] = CreateWindowA("BUTTON", "[2] Fin",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, S(64), S(8), S(52), S(26), hwnd, (HMENU)ID_MODE_FIN, NULL, NULL);
            hModeBtns[2] = CreateWindowA("BUTTON", "[3] Stat", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, S(118), S(8), S(54), S(26), hwnd, (HMENU)ID_MODE_STATS, NULL, NULL);
            hModeBtns[3] = CreateWindowA("BUTTON", "[4] Const",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, S(174), S(8), S(56), S(26), hwnd, (HMENU)ID_MODE_CONST, NULL, NULL);
            hModeBtns[4] = CreateWindowA("BUTTON", "[5] Hist", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, S(232), S(8), S(52), S(26), hwnd, (HMENU)ID_MODE_HIST, NULL, NULL);
            hHelpBtn     = CreateWindowA("BUTTON", "? [F1]",   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, S(286), S(8), S(54), S(26), hwnd, (HMENU)ID_HELP, NULL, NULL);

            // Displays
            hSubDisplay = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_RIGHT, S(10), S(38), S(324), S(18), hwnd, NULL, NULL, NULL);
            hDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "0", WS_CHILD | WS_VISIBLE | SS_RIGHT, S(10), S(58), S(324), S(34), hwnd, NULL, NULL, NULL);
            hStatusText = CreateWindowExA(0, "STATIC", "[DEG] | Press 'H' or F1 for Help", WS_CHILD | WS_VISIBLE | SS_LEFT, S(10), S(94), S(300), S(16), hwnd, NULL, NULL, NULL);

            hFontMain = CreateFontA(-MulDiv(24, dpiX, 72), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5, DEFAULT_PITCH | FF_SWISS, "Consolas");
            hFontSub = CreateFontA(-MulDiv(14, dpiX, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hFontSmall = CreateFontA(-MulDiv(12, dpiX, 72), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFontMain, TRUE);
            SendMessageA(hSubDisplay, WM_SETFONT, (WPARAM)hFontSub, TRUE);
            SendMessageA(hStatusText, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

            // Mode 0: Scientific Keypad
            char labels[40][6] = {
                "MC",  "MR",  "MS",  "M+",  "M-",
                "sin", "cos", "tan", "pi",  "C",
                "ln",  "log", "sqrt", "e",  "<",
                "n!",  "abs", "rnd", "x^2", "10^x",
                "exp", "7",   "8",   "9",   "/",
                "^",   "4",   "5",   "6",   "*",
                "+/-", "1",   "2",   "3",   "-",
                "1/x", "0",   ".",   "=",   "+"
            };
            int ids[40] = {
                2001, 2002, 2005, 2003, 2004,
                1001, 1002, 1003, 1010, 'C',
                1004, 1005, 1006, 1011, '<',
                1012, 1013, 1014, 1015, 1016,
                1007, '7',  '8',  '9',  '/',
                '^',  '4',  '5',  '6',  '*',
                1008, '1',  '2',  '3',  '-',
                1009, '0',  '.',  '=',  '+'
            };

            for(int i=0; i<40; i++) {
                int col = i % 5;
                int row = i / 5;
                if (ids[i]) {
                    hSciBtns[i] = CreateWindowA("BUTTON", labels[i], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, S(10 + col * 66), S(115 + row * 38), S(60), S(34), hwnd, (HMENU)(INT_PTR)ids[i], NULL, NULL);
                }
            }

            // Mode 1: Financial Controls
            hFinControls[0] = CreateWindowA("STATIC", "Loan Amount ($):", WS_CHILD | SS_LEFT, S(15), S(115), S(120), S(20), hwnd, NULL, NULL, NULL);
            hFinControls[1] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "250000", WS_CHILD | ES_NUMBER, S(140), S(112), S(180), S(24), hwnd, NULL, NULL, NULL);
            
            hFinControls[2] = CreateWindowA("STATIC", "Interest Rate (%):", WS_CHILD | SS_LEFT, S(15), S(145), S(120), S(20), hwnd, NULL, NULL, NULL);
            hFinControls[3] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "6.5", WS_CHILD, S(140), S(142), S(180), S(24), hwnd, NULL, NULL, NULL);

            hFinControls[4] = CreateWindowA("STATIC", "Term (Years):", WS_CHILD | SS_LEFT, S(15), S(175), S(120), S(20), hwnd, NULL, NULL, NULL);
            hFinControls[5] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "30", WS_CHILD | ES_NUMBER, S(140), S(172), S(180), S(24), hwnd, NULL, NULL, NULL);

            hFinControls[6] = CreateWindowA("BUTTON", "Calculate PMT", WS_CHILD | BS_PUSHBUTTON, S(140), S(205), S(180), S(30), hwnd, (HMENU)ID_FIN_CALC_PMT, NULL, NULL);
            hFinControls[7] = CreateWindowExA(0, "STATIC", "Monthly PMT: $0.00", WS_CHILD | SS_LEFT, S(15), S(245), S(300), S(24), hwnd, NULL, NULL, NULL);

            hFinControls[8] = CreateWindowA("STATIC", "Cost ($):", WS_CHILD | SS_LEFT, S(15), S(285), S(60), S(20), hwnd, NULL, NULL, NULL);
            hFinControls[9] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "50", WS_CHILD, S(80), S(282), S(80), S(24), hwnd, NULL, NULL, NULL);

            hFinControls[10] = CreateWindowA("STATIC", "Sell ($):", WS_CHILD | SS_LEFT, S(170), S(285), S(60), S(20), hwnd, NULL, NULL, NULL);
            hFinControls[11] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "80", WS_CHILD, S(235), S(282), S(85), S(24), hwnd, NULL, NULL, NULL);

            hFinControls[12] = CreateWindowA("BUTTON", "Calc Margin", WS_CHILD | BS_PUSHBUTTON, S(140), S(315), S(180), S(30), hwnd, (HMENU)ID_FIN_CALC_MARG, NULL, NULL);
            hFinControls[13] = CreateWindowExA(0, "STATIC", "Margin: 0.00%", WS_CHILD | SS_LEFT, S(15), S(355), S(300), S(24), hwnd, NULL, NULL, NULL);

            // Mode 2: Statistics Controls
            hStatsControls[0] = CreateWindowA("STATIC", "1-Var Dataset (comma/space separated):", WS_CHILD | SS_LEFT, S(15), S(110), S(314), S(16), hwnd, NULL, NULL, NULL);
            hStatsControls[1] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "14, 25, 33, 42, 18, 29, 35, 42, 58, 67", WS_CHILD | ES_AUTOHSCROLL, S(15), S(128), S(314), S(22), hwnd, NULL, NULL, NULL);
            hStatsControls[2] = CreateWindowA("BUTTON", "Compute 1-Var", WS_CHILD | BS_PUSHBUTTON, S(15), S(154), S(150), S(26), hwnd, (HMENU)ID_STATS_CALC_1VAR, NULL, NULL);
            hStatsControls[10] = CreateWindowA("BUTTON", "Use Mean", WS_CHILD | BS_PUSHBUTTON, S(170), S(154), S(75), S(26), hwnd, (HMENU)ID_STATS_USE_MEAN, NULL, NULL);
            hStatsControls[3] = CreateWindowExA(0, "STATIC", "N=10 | Mean=36.3 | Med=34 | s=16.67", WS_CHILD | SS_LEFT, S(15), S(184), S(314), S(16), hwnd, NULL, NULL, NULL);
            hStatsControls[4] = CreateWindowExA(0, "STATIC", "Min=14 | Max=67 | Range=53 | SE=5.27", WS_CHILD | SS_LEFT, S(15), S(202), S(314), S(16), hwnd, NULL, NULL, NULL);

            hStatsControls[5] = CreateWindowA("STATIC", "2-Var Regress (x,y e.g. 1,2.5; 2,4.8; 3,7.1):", WS_CHILD | SS_LEFT, S(15), S(226), S(314), S(16), hwnd, NULL, NULL, NULL);
            hStatsControls[6] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1,2.5; 2,4.8; 3,7.1; 4,9.4; 5,11.9", WS_CHILD | ES_AUTOHSCROLL, S(15), S(244), S(314), S(22), hwnd, NULL, NULL, NULL);
            hStatsControls[7] = CreateWindowA("BUTTON", "Compute Regression", WS_CHILD | BS_PUSHBUTTON, S(15), S(270), S(150), S(26), hwnd, (HMENU)ID_STATS_CALC_2VAR, NULL, NULL);
            hStatsControls[9] = CreateWindowA("BUTTON", "Use Slope", WS_CHILD | BS_PUSHBUTTON, S(170), S(270), S(75), S(26), hwnd, (HMENU)ID_STATS_USE_SLOPE, NULL, NULL);
            hStatsControls[8] = CreateWindowExA(0, "STATIC", "y = 2.34x + 0.12 | r = 0.9998 | R2 = 0.9996", WS_CHILD | SS_LEFT, S(15), S(300), S(314), S(32), hwnd, NULL, NULL, NULL);

            // Mode 3: Scientific Constants
            char constLabels[7][24] = { "c (Speed of Light)", "h (Planck)", "G (Gravitational)", "N_A (Avogadro)", "R (Gas Constant)", "pi (Pi)", "e (Euler)" };
            for (int i = 0; i < 7; i++) {
                hConstBtns[i] = CreateWindowA("BUTTON", constLabels[i], WS_CHILD | BS_PUSHBUTTON, S(20), S(115 + i * 38), S(304), S(32), hwnd, (HMENU)(INT_PTR)(6001 + i), NULL, NULL);
            }

            // Mode 4: History Tape Controls
            hHistControls[0] = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | LBS_NOTIFY | WS_VSCROLL, S(15), S(115), S(314), S(230), hwnd, (HMENU)ID_HIST_LIST, NULL, NULL);
            hHistControls[1] = CreateWindowA("BUTTON", "Recall Entry", WS_CHILD | BS_PUSHBUTTON, S(15), S(355), S(95), S(30), hwnd, (HMENU)ID_HIST_RECALL, NULL, NULL);
            hHistControls[2] = CreateWindowA("BUTTON", "Export TXT", WS_CHILD | BS_PUSHBUTTON, S(125), S(355), S(95), S(30), hwnd, (HMENU)ID_HIST_EXPORT, NULL, NULL);
            hHistControls[3] = CreateWindowA("BUTTON", "Clear Tape", WS_CHILD | BS_PUSHBUTTON, S(235), S(355), S(94), S(30), hwnd, (HMENU)ID_HIST_CLEAR, NULL, NULL);

            SetViewMode(0);

            EnumChildWindows(hwnd, SetFontCallback, (LPARAM)hFontSmall);
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFontMain, TRUE);
            SendMessageA(hSubDisplay, WM_SETFONT, (WPARAM)hFontSub, TRUE);
            
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            if ((HWND)lParam == hDisplay) {
                SetTextColor(hdcStatic, RGB(255, 255, 255));
            } else {
                SetTextColor(hdcStatic, RGB(148, 163, 184));
            }
            SetBkColor(hdcStatic, RGB(15, 23, 42));
            return (INT_PTR)hDisplayBgBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            SetTextColor(hdcEdit, RGB(248, 250, 252));
            SetBkColor(hdcEdit, RGB(30, 41, 59));
            return (INT_PTR)hEditBgBrush;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);

            if (id == ID_MODE_SCI) SetViewMode(0);
            else if (id == ID_MODE_FIN) SetViewMode(1);
            else if (id == ID_MODE_STATS) SetViewMode(2);
            else if (id == ID_MODE_CONST) SetViewMode(3);
            else if (id == ID_MODE_HIST) SetViewMode(4);
            else if (id == ID_HELP) ShowHelpDialog(hwnd);

            else if ((id >= '0' && id <= '9') || id == '.') {
                AppendChar(id);
            } else if (id == 'C') {
                my_strcpy(displayBuffer, "0");
                subDisplayBuffer[0] = 0;
                SetWindowTextA(hDisplay, displayBuffer);
                SetWindowTextA(hSubDisplay, "");
                operand1 = 0;
                operator = 0;
                isNewOperand = 1;
            } else if (id == '<') {
                Backspace();
            } else if (id == '+' || id == '-' || id == '*' || id == '/' || id == '^' || id == '%') {
                if (!isNewOperand && operator) DoCalculate();
                operand1 = m_atof(displayBuffer);
                operator = id;
                isNewOperand = 1;
                m_sprintf(subDisplayBuffer, "%.6g %c", operand1, (char)operator);
                SetWindowTextA(hSubDisplay, subDisplayBuffer);
            } else if (id == '=') {
                DoCalculate();
                operator = 0;
            } else if (id >= 1001 && id <= 1016) {
                DoUnary(id - 1000);
            } else if (id == 2001) { // MC
                memoryStore = 0.0;
                UpdateStatusText();
            } else if (id == 2002) { // MR
                operand1 = memoryStore;
                FormatDisplay(operand1);
                isNewOperand = 1;
            } else if (id == 2003) { // M+
                memoryStore += m_atof(displayBuffer);
                isNewOperand = 1;
                UpdateStatusText();
            } else if (id == 2004) { // M-
                memoryStore -= m_atof(displayBuffer);
                isNewOperand = 1;
                UpdateStatusText();
            } else if (id == 2005) { // MS
                memoryStore = m_atof(displayBuffer);
                isNewOperand = 1;
                UpdateStatusText();
            }
            // Financial PMT Calculation
            else if (id == ID_FIN_CALC_PMT) {
                char pBuf[32], rBuf[32], yBuf[32];
                GetWindowTextA(hFinControls[1], pBuf, 32);
                GetWindowTextA(hFinControls[3], rBuf, 32);
                GetWindowTextA(hFinControls[5], yBuf, 32);
                double P = m_atof(pBuf);
                double rPct = m_atof(rBuf);
                double r = (rPct / 100.0) / 12.0;
                double n = m_atof(yBuf) * 12.0;
                if (P > 0 && n > 0 && rPct >= 0) {
                    double pmt = 0;
                    if (r == 0) pmt = P / n;
                    else pmt = (P * r * m_pow(1.0 + r, n)) / (m_pow(1.0 + r, n) - 1.0);
                    char outBuf[64];
                    m_sprintf(outBuf, "Monthly PMT: $%.2f", pmt);
                    SetWindowTextA(hFinControls[7], outBuf);
                    FormatDisplay(pmt);
                    AddHistoryEntry("PMT Loan Calc", displayBuffer);
                }
            }
            // Financial Margin Calculation
            else if (id == ID_FIN_CALC_MARG) {
                char cBuf[32], sBuf[32];
                GetWindowTextA(hFinControls[9], cBuf, 32);
                GetWindowTextA(hFinControls[11], sBuf, 32);
                double cost = m_atof(cBuf);
                double sell = m_atof(sBuf);
                if (cost >= 0 && sell > 0) {
                    double profit = sell - cost;
                    double marginPct = (profit / sell) * 100.0;
                    char outBuf[64];
                    m_sprintf(outBuf, "Profit: $%.2f | Margin: %.2f%%", profit, marginPct);
                    SetWindowTextA(hFinControls[13], outBuf);
                    FormatDisplay(profit);
                    AddHistoryEntry("Margin Calc", displayBuffer);
                }
            }
            // Statistics 1-Var Calculation
            else if (id == ID_STATS_CALC_1VAR) {
                char buf[512];
                GetWindowTextA(hStatsControls[1], buf, 512);
                double nums[100];
                int n = ParseDoubleList(buf, nums, 100);
                if (n > 0) {
                    double sum = 0, minVal = nums[0], maxVal = nums[0];
                    for (int i = 0; i < n; i++) {
                        sum += nums[i];
                        if (nums[i] < minVal) minVal = nums[i];
                        if (nums[i] > maxVal) maxVal = nums[i];
                    }
                    double mean = sum / (double)n;
                    for (int i = 0; i < n - 1; i++) {
                        for (int j = i + 1; j < n; j++) {
                            if (nums[j] < nums[i]) {
                                double tmp = nums[i]; nums[i] = nums[j]; nums[j] = tmp;
                            }
                        }
                    }
                    double median = (n % 2 == 1) ? nums[n/2] : (nums[n/2 - 1] + nums[n/2]) / 2.0;
                    double sumDiffSq = 0;
                    for (int i = 0; i < n; i++) {
                        double diff = nums[i] - mean;
                        sumDiffSq += diff * diff;
                    }
                    double sampleVar = (n > 1) ? (sumDiffSq / (double)(n - 1)) : 0.0;
                    double sampleStdDev = m_sqrt(sampleVar);
                    double stdErr = sampleStdDev / m_sqrt((double)n);
                    statLastMean = mean;

                    char out1[128], out2[128];
                    m_sprintf(out1, "N=%d | Mean=%.4g | Med=%.4g | s=%.4g", n, mean, median, sampleStdDev);
                    m_sprintf(out2, "Min=%.4g | Max=%.4g | Range=%.4g | SE=%.4g", minVal, maxVal, maxVal - minVal, stdErr);
                    SetWindowTextA(hStatsControls[3], out1);
                    SetWindowTextA(hStatsControls[4], out2);

                    char histExpr[64], histRes[32];
                    m_sprintf(histExpr, "1-Var Stats (N=%d)", n);
                    m_sprintf(histRes, "Mean=%.4g, s=%.4g", mean, sampleStdDev);
                    AddHistoryEntry(histExpr, histRes);
                }
            }
            // Statistics 2-Var Regression Calculation
            else if (id == ID_STATS_CALC_2VAR) {
                char buf[512];
                GetWindowTextA(hStatsControls[6], buf, 512);
                double xs[50], ys[50];
                int n = ParsePointList(buf, xs, ys, 50);
                if (n >= 2) {
                    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0, sumY2 = 0;
                    for (int i = 0; i < n; i++) {
                        sumX += xs[i];
                        sumY += ys[i];
                        sumXY += xs[i] * ys[i];
                        sumX2 += xs[i] * xs[i];
                        sumY2 += ys[i] * ys[i];
                    }
                    double denomM = ((double)n * sumX2 - sumX * sumX);
                    if (denomM != 0.0) {
                        double m = ((double)n * sumXY - sumX * sumY) / denomM;
                        double b = (sumY - m * sumX) / (double)n;
                        double denomR = m_sqrt(((double)n * sumX2 - sumX * sumX) * ((double)n * sumY2 - sumY * sumY));
                        double r = (denomR != 0.0) ? (((double)n * sumXY - sumX * sumY) / denomR) : 0.0;
                        double r2 = r * r;
                        statLastSlope = m;

                        char outReg[128];
                        if (b >= 0) m_sprintf(outReg, "y = %.4gx + %.4g | r=%.4g | R2=%.4g", m, b, r, r2);
                        else m_sprintf(outReg, "y = %.4gx - %.4g | r=%.4g | R2=%.4g", m, -b, r, r2);
                        SetWindowTextA(hStatsControls[8], outReg);

                        char histExpr[64], histRes[32];
                        m_sprintf(histExpr, "LinReg (N=%d)", n);
                        m_sprintf(histRes, "m=%.4g, b=%.4g", m, b);
                        AddHistoryEntry(histExpr, histRes);
                    }
                }
            }
            // Use Mean / Slope buttons
            else if (id == ID_STATS_USE_MEAN) {
                FormatDisplay(statLastMean);
                isNewOperand = 1;
                SetViewMode(0);
            }
            else if (id == ID_STATS_USE_SLOPE) {
                FormatDisplay(statLastSlope);
                isNewOperand = 1;
                SetViewMode(0);
            }
            // Constants insertion
            else if (id >= 6001 && id <= 6007) {
                double val = 0;
                if (id == 6001) val = CONST_C;
                else if (id == 6002) val = CONST_H;
                else if (id == 6003) val = CONST_G;
                else if (id == 6004) val = CONST_NA;
                else if (id == 6005) val = CONST_R;
                else if (id == 6006) val = PI;
                else if (id == 6007) val = E;
                FormatDisplay(val);
                isNewOperand = 1;
                SetViewMode(0);
            }
            // History Tape buttons
            else if (id == ID_HIST_RECALL) {
                int sel = (int)SendMessageA(hHistControls[0], LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel < historyCount) {
                    my_strcpy(displayBuffer, historyTape[sel].res);
                    SetWindowTextA(hDisplay, displayBuffer);
                    isNewOperand = 1;
                    SetViewMode(0);
                }
            } else if (id == ID_HIST_CLEAR) {
                historyCount = 0;
                SendMessageA(hHistControls[0], LB_RESETCONTENT, 0, 0);
            } else if (id == ID_HIST_EXPORT) {
                ExportHistoryToFile();
            }
            break;
        }
        case WM_DESTROY:
            if (hFontMain) DeleteObject(hFontMain);
            if (hFontSub) DeleteObject(hFontSub);
            if (hFontSmall) DeleteObject(hFontSmall);
            if (hDisplayBgBrush) DeleteObject(hDisplayBgBrush);
            if (hEditBgBrush) DeleteObject(hEditBgBrush);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *SETPROCESSDPIAWARE_T)(void);
        SETPROCESSDPIAWARE_T setDPI = (SETPROCESSDPIAWARE_T)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDPI) setDPI();
    }

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KCalcClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(15, 23, 42));

    RegisterClassA(&wc);
    // Adjust window rect based on DPI-scaled client size
    RECT wr = {0, 0, S(350), S(430)};
    AdjustWindowRect(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    int winW = wr.right - wr.left;
    int winH = wr.bottom - wr.top;

    HWND hwnd = CreateWindowExA(0, "KCalcClass", "KCalc Pro - [Press F1 for Help]", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, winW, winH, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            HWND hFocus = GetFocus();
            char className[32] = {0};
            if (hFocus) GetClassNameA(hFocus, className, 32);
            int isEditFocused = (my_strcmp(className, "Edit") == 0 || my_strcmp(className, "EDIT") == 0);

            int key = msg.wParam;
            if (key == VK_F1 || ((key == 'H' || key == 'h') && !isEditFocused)) {
                ShowHelpDialog(hwnd);
                continue;
            }

            // Ctrl+1 through Ctrl+5 mode switching
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                if (key == '1') { SendMessageA(hwnd, WM_COMMAND, ID_MODE_SCI, 0); continue; }
                if (key == '2') { SendMessageA(hwnd, WM_COMMAND, ID_MODE_FIN, 0); continue; }
                if (key == '3') { SendMessageA(hwnd, WM_COMMAND, ID_MODE_STATS, 0); continue; }
                if (key == '4') { SendMessageA(hwnd, WM_COMMAND, ID_MODE_CONST, 0); continue; }
                if (key == '5') { SendMessageA(hwnd, WM_COMMAND, ID_MODE_HIST, 0); continue; }
            }

            if ((key == 'D' || key == 'd') && !isEditFocused && currentMode == 0) {
                isDeg = !isDeg;
                UpdateStatusText();
                continue;
            }

            if (!isEditFocused && currentMode == 0) {
                int cmd = 0;
                if (key >= '0' && key <= '9') {
                    if (!(GetKeyState(VK_SHIFT) & 0x8000)) cmd = key;
                    else if (key == '8') cmd = '*';
                    else if (key == '5') cmd = '%';
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
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    DeleteObject(wc.hbrBackground);
    ExitProcess(0);
}
