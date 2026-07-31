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

HWND hDisplay, hSubDisplay, hStatusText;
HFONT hFontMain, hFontSub, hFontSmall;
HBRUSH hDisplayBgBrush;

char displayBuffer[64] = "0";
char subDisplayBuffer[64] = "";
double operand1 = 0;
int operator = 0;
double memoryStore = 0.0;
int isNewOperand = 1;
int currentMode = 0; // 0=Sci, 1=Fin, 2=Const, 3=History

// History structure
typedef struct {
    char expr[64];
    char res[32];
} HistoryEntry;

HistoryEntry historyTape[50];
int historyCount = 0;

// View Handles
HWND hModeBtns[4];
HWND hSciBtns[42];
HWND hFinControls[12];
HWND hConstBtns[8];
HWND hHistControls[4];

// Control IDs
#define ID_MODE_SCI   3001
#define ID_MODE_FIN   3002
#define ID_MODE_CONST 3003
#define ID_MODE_HIST  3004

#define ID_FIN_CALC_PMT  4001
#define ID_FIN_CALC_FV   4002
#define ID_FIN_CALC_MARG 4003

#define ID_HIST_LIST   5001
#define ID_HIST_RECALL 5002
#define ID_HIST_CLEAR  5003
#define ID_HIST_EXPORT 5004

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
    char statusBuf[64] = "Press 'H' for Help";
    if (memoryStore != 0.0) {
        m_sprintf(statusBuf, "[M: %.6g] | Press 'H' for Help", memoryStore);
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
    
    AddHistoryEntry(exprBuf, resBuf);

    operand1 = res;
    isNewOperand = 1;
}

void DoUnary(int type) {
    double val = m_atof(displayBuffer);
    double res = val;
    const char* funcName = "";

    if (type == 1) { res = m_sin(val); funcName = "sin"; }
    else if (type == 2) { res = m_cos(val); funcName = "cos"; }
    else if (type == 3) { res = m_tan(val); funcName = "tan"; }
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
    else if (type == 8) res = -val;
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
    for (int i = 0; i < 12; i++) {
        if (hFinControls[i]) ShowWindow(hFinControls[i], mode == 1 ? SW_SHOW : SW_HIDE);
    }
    // Show/Hide Constants Controls
    for (int i = 0; i < 7; i++) {
        if (hConstBtns[i]) ShowWindow(hConstBtns[i], mode == 2 ? SW_SHOW : SW_HIDE);
    }
    // Show/Hide History Controls
    for (int i = 0; i < 4; i++) {
        if (hHistControls[i]) ShowWindow(hHistControls[i], mode == 3 ? SW_SHOW : SW_HIDE);
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
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

            // Mode Selector Bar
            hModeBtns[0] = CreateWindowA("BUTTON", "Scientific", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 8, 78, 26, hwnd, (HMENU)ID_MODE_SCI, NULL, NULL);
            hModeBtns[1] = CreateWindowA("BUTTON", "Financial",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 92, 8, 78, 26, hwnd, (HMENU)ID_MODE_FIN, NULL, NULL);
            hModeBtns[2] = CreateWindowA("BUTTON", "Constants",  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 174, 8, 78, 26, hwnd, (HMENU)ID_MODE_CONST, NULL, NULL);
            hModeBtns[3] = CreateWindowA("BUTTON", "History",    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 256, 8, 78, 26, hwnd, (HMENU)ID_MODE_HIST, NULL, NULL);

            // Displays
            hSubDisplay = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_RIGHT, 10, 38, 324, 18, hwnd, NULL, NULL, NULL);
            hDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "STATIC", "0", WS_CHILD | WS_VISIBLE | SS_RIGHT, 10, 58, 324, 34, hwnd, NULL, NULL, NULL);
            hStatusText = CreateWindowExA(0, "STATIC", "Press 'H' for Help", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 94, 300, 16, hwnd, NULL, NULL, NULL);

            hFontMain = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Consolas");
            hFontSub = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hFontSmall = CreateFontA(12, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFontMain, TRUE);
            SendMessageA(hSubDisplay, WM_SETFONT, (WPARAM)hFontSub, TRUE);
            SendMessageA(hStatusText, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

            // Mode 0: Scientific Keypad
            char labels[40][6] = {
                "MC",  "MR",  "MS",  "M+",  "M-",
                "sin", "cos", "tan", "pi",  "C",
                "ln",  "log", "sqr", "e",   "<",
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
                    hSciBtns[i] = CreateWindowA("BUTTON", labels[i], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10 + col * 66, 115 + row * 38, 60, 34, hwnd, (HMENU)(INT_PTR)ids[i], NULL, NULL);
                }
            }

            // Mode 1: Financial Controls
            CreateWindowA("STATIC", "Loan Amount ($):", WS_CHILD | SS_LEFT, 15, 115, 120, 20, hwnd, NULL, NULL, NULL);
            hFinControls[0] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "250000", WS_CHILD | ES_NUMBER, 140, 112, 180, 24, hwnd, NULL, NULL, NULL);
            
            CreateWindowA("STATIC", "Interest Rate (%):", WS_CHILD | SS_LEFT, 15, 145, 120, 20, hwnd, NULL, NULL, NULL);
            hFinControls[1] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "6.5", WS_CHILD, 140, 142, 180, 24, hwnd, NULL, NULL, NULL);

            CreateWindowA("STATIC", "Term (Years):", WS_CHILD | SS_LEFT, 15, 175, 120, 20, hwnd, NULL, NULL, NULL);
            hFinControls[2] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "30", WS_CHILD | ES_NUMBER, 140, 172, 180, 24, hwnd, NULL, NULL, NULL);

            hFinControls[3] = CreateWindowA("BUTTON", "Calculate PMT", WS_CHILD | BS_PUSHBUTTON, 140, 205, 180, 30, hwnd, (HMENU)ID_FIN_CALC_PMT, NULL, NULL);
            hFinControls[4] = CreateWindowExA(0, "STATIC", "Monthly PMT: $0.00", WS_CHILD | SS_LEFT, 15, 245, 300, 24, hwnd, NULL, NULL, NULL);

            CreateWindowA("STATIC", "Cost ($):", WS_CHILD | SS_LEFT, 15, 285, 60, 20, hwnd, NULL, NULL, NULL);
            hFinControls[5] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "50", WS_CHILD, 80, 282, 80, 24, hwnd, NULL, NULL, NULL);

            CreateWindowA("STATIC", "Sell ($):", WS_CHILD | SS_LEFT, 170, 285, 60, 20, hwnd, NULL, NULL, NULL);
            hFinControls[6] = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "80", WS_CHILD, 235, 282, 85, 24, hwnd, NULL, NULL, NULL);

            hFinControls[7] = CreateWindowA("BUTTON", "Calc Margin", WS_CHILD | BS_PUSHBUTTON, 140, 315, 180, 30, hwnd, (HMENU)ID_FIN_CALC_MARG, NULL, NULL);
            hFinControls[8] = CreateWindowExA(0, "STATIC", "Margin: 0.00%", WS_CHILD | SS_LEFT, 15, 355, 300, 24, hwnd, NULL, NULL, NULL);

            // Mode 2: Scientific Constants
            char constLabels[7][24] = { "c (Speed of Light)", "h (Planck)", "G (Gravitational)", "N_A (Avogadro)", "R (Gas Constant)", "pi (Pi)", "e (Euler)" };
            for (int i = 0; i < 7; i++) {
                hConstBtns[i] = CreateWindowA("BUTTON", constLabels[i], WS_CHILD | BS_PUSHBUTTON, 20, 115 + i * 38, 304, 32, hwnd, (HMENU)(INT_PTR)(6001 + i), NULL, NULL);
            }

            // Mode 3: History Tape Controls
            hHistControls[0] = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | LBS_NOTIFY | WS_VSCROLL, 15, 115, 314, 230, hwnd, (HMENU)ID_HIST_LIST, NULL, NULL);
            hHistControls[1] = CreateWindowA("BUTTON", "Recall Entry", WS_CHILD | BS_PUSHBUTTON, 15, 355, 95, 30, hwnd, (HMENU)ID_HIST_RECALL, NULL, NULL);
            hHistControls[2] = CreateWindowA("BUTTON", "Export TXT", WS_CHILD | BS_PUSHBUTTON, 125, 355, 95, 30, hwnd, (HMENU)ID_HIST_EXPORT, NULL, NULL);
            hHistControls[3] = CreateWindowA("BUTTON", "Clear Tape", WS_CHILD | BS_PUSHBUTTON, 235, 355, 94, 30, hwnd, (HMENU)ID_HIST_CLEAR, NULL, NULL);

            SetViewMode(0);
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            if ((HWND)lParam == hDisplay) {
                SetTextColor(hdcStatic, RGB(255, 255, 255));
                SetBkColor(hdcStatic, RGB(15, 23, 42));
                return (INT_PTR)hDisplayBgBrush;
            } else if ((HWND)lParam == hSubDisplay || (HWND)lParam == hStatusText) {
                SetTextColor(hdcStatic, RGB(148, 163, 184));
                SetBkColor(hdcStatic, RGB(15, 23, 42));
                return (INT_PTR)hDisplayBgBrush;
            }
            break;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);

            if (id == ID_MODE_SCI) SetViewMode(0);
            else if (id == ID_MODE_FIN) SetViewMode(1);
            else if (id == ID_MODE_CONST) SetViewMode(2);
            else if (id == ID_MODE_HIST) SetViewMode(3);

            else if ((id >= '0' && id <= '9') || id == '.') {
                AppendChar(id);
            } else if (id == 'C') {
                my_strcpy(displayBuffer, "0");
                SetWindowTextA(hDisplay, displayBuffer);
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
                GetWindowTextA(hFinControls[0], pBuf, 32);
                GetWindowTextA(hFinControls[1], rBuf, 32);
                GetWindowTextA(hFinControls[2], yBuf, 32);
                double P = m_atof(pBuf);
                double r = m_atof(rBuf) / 100.0 / 12.0;
                double n = m_atof(yBuf) * 12.0;
                if (P > 0 && r > 0 && n > 0) {
                    double pmt = (P * r * m_pow(1.0 + r, n)) / (m_pow(1.0 + r, n) - 1.0);
                    char outBuf[64];
                    m_sprintf(outBuf, "Monthly PMT: $%.2f", pmt);
                    SetWindowTextA(hFinControls[4], outBuf);
                    FormatDisplay(pmt);
                    AddHistoryEntry("PMT Loan Calc", displayBuffer);
                }
            }
            // Financial Margin Calculation
            else if (id == ID_FIN_CALC_MARG) {
                char cBuf[32], sBuf[32];
                GetWindowTextA(hFinControls[5], cBuf, 32);
                GetWindowTextA(hFinControls[6], sBuf, 32);
                double cost = m_atof(cBuf);
                double sell = m_atof(sBuf);
                if (cost > 0 && sell > 0) {
                    double profit = sell - cost;
                    double marginPct = (profit / sell) * 100.0;
                    char outBuf[64];
                    m_sprintf(outBuf, "Profit: $%.2f | Margin: %.2f%%", profit, marginPct);
                    SetWindowTextA(hFinControls[8], outBuf);
                    FormatDisplay(profit);
                    AddHistoryEntry("Margin Calc", displayBuffer);
                }
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
    // Adjusted width and height for tab bar and controls (380x520)
    HWND hwnd = CreateWindowExA(0, "KCalcClass", "KCalc Pro", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 380, 520, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            int key = msg.wParam;
            int cmd = 0;
            if (key == 'H') {
                MessageBoxA(hwnd, "KCalc Pro Help:\n- Switch modes using the buttons above.\n- Keyboard shortcuts: Numpad/numbers, +, -, *, /, %, ^, Enter, Backspace, Esc.\n- History exports to kcalc_history.txt.", "Help", MB_OK | MB_ICONINFORMATION);
                continue;
            }
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
            
            if (cmd && currentMode == 0) {
                SendMessageA(hwnd, WM_COMMAND, cmd, 0);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    DeleteObject(wc.hbrBackground);
    ExitProcess(0);
}
