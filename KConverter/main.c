int _fltused = 1;
#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

#pragma function(memset)
#pragma function(memcpy)

typedef int (__cdecl *sprintf_t)(char*, const char*, ...);
typedef double (__cdecl *atof_t)(const char*);
typedef void* (__cdecl *fopen_t)(const char*, const char*);
typedef int (__cdecl *fputs_t)(const char*, void*);
typedef int (__cdecl *fclose_t)(void*);

sprintf_t m_sprintf;
atof_t m_atof;
fopen_t m_fopen;
fputs_t m_fputs;
fclose_t m_fclose;

HWND hCategory, hInput, hOutput, hFrom, hTo, hPrecision, hFormat;
HWND hBatchOutput, hHistoryOutput, hFavCombo, hFormulaStatic;
HWND hBtnSingle, hBtnBatch, hBtnFavs, hBtnHistory;
HFONT hFont, hFontBold;

char buffer[1024];
char historyBuffer[4096];
int currentMode = 0; // 0=Single, 1=Batch, 2=Favs, 3=History

// Categories and Units
const char* catNames[] = {"Length", "Weight", "Temperature", "Data", "Speed", "Area", "Volume", "Time"};
const int numCats = 8;

const char* lenUnits[] = {"Meters", "Kilometers", "Centimeters", "Millimeters", "Miles", "Yards", "Feet", "Inches"};
const double lenFactors[] = {1.0, 1000.0, 0.01, 0.001, 1609.344, 0.9144, 0.3048, 0.0254};
const int lenCount = 8;

const char* wtUnits[] = {"Kilograms", "Grams", "Milligrams", "Pounds", "Ounces", "Stone"};
const double wtFactors[] = {1.0, 0.001, 0.000001, 0.45359237, 0.028349523, 6.35029};
const int wtCount = 6;

const char* tempUnits[] = {"Celsius", "Fahrenheit", "Kelvin"};
const int tempCount = 3;

const char* dataUnits[] = {"Bytes", "Kilobytes (KB)", "Megabytes (MB)", "Gigabytes (GB)", "Terabytes (TB)"};
const double dataFactors[] = {1.0, 1024.0, 1048576.0, 1073741824.0, 1099511627776.0};
const int dataCount = 5;

const char* speedUnits[] = {"Meters/sec", "Km/hour", "Miles/hour", "Knots", "Feet/sec"};
const double speedFactors[] = {1.0, 0.277778, 0.44704, 0.514444, 0.3048};
const int speedCount = 5;

const char* areaUnits[] = {"Sq Meters", "Sq Kilometers", "Sq Feet", "Acres", "Hectares"};
const double areaFactors[] = {1.0, 1000000.0, 0.092903, 4046.856, 10000.0};
const int areaCount = 5;

const char* volUnits[] = {"Liters", "Milliliters", "Cubic Meters", "Gallons (US)", "Fluid Oz"};
const double volFactors[] = {1.0, 0.001, 1000.0, 3.78541, 0.0295735};
const int volCount = 5;

const char* timeUnits[] = {"Seconds", "Minutes", "Hours", "Days", "Weeks", "Years"};
const double timeFactors[] = {1.0, 60.0, 3600.0, 86400.0, 604800.0, 31536000.0};
const int timeCount = 6;

double GetFactor(int cat, int index) {
    if (cat == 0 && index < lenCount) return lenFactors[index];
    if (cat == 1 && index < wtCount) return wtFactors[index];
    if (cat == 3 && index < dataCount) return dataFactors[index];
    if (cat == 4 && index < speedCount) return speedFactors[index];
    if (cat == 5 && index < areaCount) return areaFactors[index];
    if (cat == 6 && index < volCount) return volFactors[index];
    if (cat == 7 && index < timeCount) return timeFactors[index];
    return 1.0;
}

const char* GetUnitName(int cat, int index) {
    if (cat == 0 && index < lenCount) return lenUnits[index];
    if (cat == 1 && index < wtCount) return wtUnits[index];
    if (cat == 2 && index < tempCount) return tempUnits[index];
    if (cat == 3 && index < dataCount) return dataUnits[index];
    if (cat == 4 && index < speedCount) return speedUnits[index];
    if (cat == 5 && index < areaCount) return areaUnits[index];
    if (cat == 6 && index < volCount) return volUnits[index];
    if (cat == 7 && index < timeCount) return timeUnits[index];
    return "";
}

int GetUnitCount(int cat) {
    if (cat == 0) return lenCount;
    if (cat == 1) return wtCount;
    if (cat == 2) return tempCount;
    if (cat == 3) return dataCount;
    if (cat == 4) return speedCount;
    if (cat == 5) return areaCount;
    if (cat == 6) return volCount;
    if (cat == 7) return timeCount;
    return 0;
}

void PopulateUnits(int catIdx) {
    SendMessageA(hFrom, CB_RESETCONTENT, 0, 0);
    SendMessageA(hTo, CB_RESETCONTENT, 0, 0);
    int count = GetUnitCount(catIdx);
    for (int i = 0; i < count; i++) {
        const char* name = GetUnitName(catIdx, i);
        SendMessageA(hFrom, CB_ADDSTRING, 0, (LPARAM)name);
        SendMessageA(hTo, CB_ADDSTRING, 0, (LPARAM)name);
    }
    SendMessageA(hFrom, CB_SETCURSEL, 0, 0);
    SendMessageA(hTo, CB_SETCURSEL, count > 1 ? 1 : 0, 0);
}

void FormatValue(double val, char* out, int precIdx, int formatIdx) {
    if (formatIdx == 1) { // Scientific
        m_sprintf(out, "%.4e", val);
        return;
    }
    
    if (precIdx == 0) { // Auto
        m_sprintf(out, "%.6g", val);
    } else if (precIdx == 1) { // 0 dec
        m_sprintf(out, "%.0f", val);
    } else if (precIdx == 2) { // 2 dec
        m_sprintf(out, "%.2f", val);
    } else if (precIdx == 3) { // 4 dec
        m_sprintf(out, "%.4f", val);
    } else if (precIdx == 4) { // 6 dec
        m_sprintf(out, "%.6f", val);
    } else {
        m_sprintf(out, "%.6g", val);
    }
}

void AppendHistory(const char* entry) {
    if (historyBuffer[0] != '\0') {
        char temp[4096];
        m_sprintf(temp, "%s\r\n%s", entry, historyBuffer);
        lstrcpyA(historyBuffer, temp);
    } else {
        m_sprintf(historyBuffer, "%s", entry);
    }
    SetWindowTextA(hHistoryOutput, historyBuffer);
}

void DoConvert() {
    GetWindowTextA(hInput, buffer, 255);
    double val = m_atof(buffer);
    int catIdx = SendMessageA(hCategory, CB_GETCURSEL, 0, 0);
    int fromIdx = SendMessageA(hFrom, CB_GETCURSEL, 0, 0);
    int toIdx = SendMessageA(hTo, CB_GETCURSEL, 0, 0);
    int precIdx = SendMessageA(hPrecision, CB_GETCURSEL, 0, 0);
    int formatIdx = SendMessageA(hFormat, CB_GETCURSEL, 0, 0);

    if (catIdx == CB_ERR) catIdx = 0;
    if (fromIdx == CB_ERR) fromIdx = 0;
    if (toIdx == CB_ERR) toIdx = 0;
    if (precIdx == CB_ERR) precIdx = 0;
    if (formatIdx == CB_ERR) formatIdx = 0;

    double result = 0;
    if (catIdx == 2) { // Temp
        double c = 0;
        if (fromIdx == 0) c = val;
        else if (fromIdx == 1) c = (val - 32.0) * 5.0/9.0;
        else if (fromIdx == 2) c = val - 273.15;
        
        if (toIdx == 0) result = c;
        else if (toIdx == 1) result = (c * 9.0/5.0) + 32.0;
        else if (toIdx == 2) result = c + 273.15;
    } else {
        double baseVal = val * GetFactor(catIdx, fromIdx);
        result = baseVal / GetFactor(catIdx, toIdx);
    }

    char resStr[128];
    FormatValue(result, resStr, precIdx, formatIdx);
    SetWindowTextA(hOutput, resStr);

    // Formula label
    const char* fromName = GetUnitName(catIdx, fromIdx);
    const char* toName = GetUnitName(catIdx, toIdx);
    char eqStr[256];
    m_sprintf(eqStr, "Formula: %g %s = %s %s", val, fromName, resStr, toName);
    SetWindowTextA(hFormulaStatic, eqStr);

    // Log entry
    char logLine[256];
    m_sprintf(logLine, "[%s] %g %s -> %s %s", catNames[catIdx], val, fromName, resStr, toName);
    AppendHistory(logLine);

    // Update Batch View if visible
    int uCount = GetUnitCount(catIdx);
    char batchBuf[2048];
    batchBuf[0] = '\0';
    char line[256];
    for (int i = 0; i < uCount; i++) {
        double uRes = 0;
        if (catIdx == 2) {
            double c = 0;
            if (fromIdx == 0) c = val;
            else if (fromIdx == 1) c = (val - 32.0) * 5.0/9.0;
            else if (fromIdx == 2) c = val - 273.15;

            if (i == 0) uRes = c;
            else if (i == 1) uRes = (c * 9.0/5.0) + 32.0;
            else if (i == 2) uRes = c + 273.15;
        } else {
            double baseVal = val * GetFactor(catIdx, fromIdx);
            uRes = baseVal / GetFactor(catIdx, i);
        }
        char uStr[128];
        FormatValue(uRes, uStr, precIdx, formatIdx);
        m_sprintf(line, "%s: %s\r\n", GetUnitName(catIdx, i), uStr);
        lstrcatA(batchBuf, line);
    }
    SetWindowTextA(hBatchOutput, batchBuf);
}

void UpdateViewVisibility() {
    BOOL isSingle = (currentMode == 0);
    BOOL isBatch = (currentMode == 1);
    BOOL isFav = (currentMode == 2);
    BOOL isHistory = (currentMode == 3);

    ShowWindow(hInput, isSingle ? SW_SHOW : SW_HIDE);
    ShowWindow(hFrom, isSingle ? SW_SHOW : SW_HIDE);
    ShowWindow(hTo, isSingle ? SW_SHOW : SW_HIDE);
    ShowWindow(hOutput, isSingle ? SW_SHOW : SW_HIDE);
    ShowWindow(hFormulaStatic, isSingle ? SW_SHOW : SW_HIDE);

    ShowWindow(hBatchOutput, isBatch ? SW_SHOW : SW_HIDE);
    ShowWindow(hFavCombo, isFav ? SW_SHOW : SW_HIDE);
    ShowWindow(hHistoryOutput, isHistory ? SW_SHOW : SW_HIDE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HMODULE hMsvcrt = LoadLibraryA("msvcrt.dll");
            m_sprintf = (sprintf_t)GetProcAddress(hMsvcrt, "sprintf");
            m_atof = (atof_t)GetProcAddress(hMsvcrt, "atof");
            m_fopen = (fopen_t)GetProcAddress(hMsvcrt, "fopen");
            m_fputs = (fputs_t)GetProcAddress(hMsvcrt, "fputs");
            m_fclose = (fclose_t)GetProcAddress(hMsvcrt, "fclose");

            historyBuffer[0] = '\0';

            // Top bar controls
            CreateWindowA("STATIC", "Category:", WS_CHILD | WS_VISIBLE, 10, 10, 60, 20, hwnd, NULL, NULL, NULL);
            hCategory = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 75, 8, 120, 150, hwnd, (HMENU)1002, NULL, NULL);

            CreateWindowA("STATIC", "Prec:", WS_CHILD | WS_VISIBLE, 210, 10, 35, 20, hwnd, NULL, NULL, NULL);
            hPrecision = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 250, 8, 70, 150, hwnd, (HMENU)1003, NULL, NULL);

            CreateWindowA("STATIC", "Fmt:", WS_CHILD | WS_VISIBLE, 335, 10, 30, 20, hwnd, NULL, NULL, NULL);
            hFormat = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 370, 8, 100, 150, hwnd, (HMENU)1004, NULL, NULL);

            // Mode Tab Buttons
            hBtnSingle = CreateWindowA("BUTTON", "Single", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 38, 70, 24, hwnd, (HMENU)2001, NULL, NULL);
            hBtnBatch = CreateWindowA("BUTTON", "Batch Mode", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 85, 38, 85, 24, hwnd, (HMENU)2002, NULL, NULL);
            hBtnFavs = CreateWindowA("BUTTON", "Favorites", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 175, 38, 75, 24, hwnd, (HMENU)2003, NULL, NULL);
            hBtnHistory = CreateWindowA("BUTTON", "History Log", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 255, 38, 85, 24, hwnd, (HMENU)2004, NULL, NULL);

            // Single View Controls
            CreateWindowA("STATIC", "Input:", WS_CHILD | WS_VISIBLE, 10, 72, 45, 20, hwnd, NULL, NULL, NULL);
            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 60, 70, 100, 22, hwnd, (HMENU)1005, NULL, NULL);

            CreateWindowA("STATIC", "From:", WS_CHILD | WS_VISIBLE, 170, 72, 40, 20, hwnd, NULL, NULL, NULL);
            hFrom = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 210, 70, 120, 150, hwnd, (HMENU)1006, NULL, NULL);

            CreateWindowA("BUTTON", "⇄ Swap", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 340, 70, 60, 22, hwnd, (HMENU)3001, NULL, NULL);
            CreateWindowA("BUTTON", "⭐ Pin", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 405, 70, 50, 22, hwnd, (HMENU)3002, NULL, NULL);

            CreateWindowA("STATIC", "To:", WS_CHILD | WS_VISIBLE, 170, 102, 40, 20, hwnd, NULL, NULL, NULL);
            hTo = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 210, 100, 120, 150, hwnd, (HMENU)1007, NULL, NULL);

            CreateWindowA("STATIC", "Result:", WS_CHILD | WS_VISIBLE, 10, 132, 50, 20, hwnd, NULL, NULL, NULL);
            hOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_READONLY | ES_AUTOHSCROLL, 60, 130, 270, 22, hwnd, NULL, NULL, NULL);
            CreateWindowA("BUTTON", "Convert", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 340, 100, 115, 52, hwnd, (HMENU)1001, NULL, NULL);

            hFormulaStatic = CreateWindowA("STATIC", "Formula: 1 Meter = 1 Meter", WS_CHILD | WS_VISIBLE, 10, 160, 460, 20, hwnd, NULL, NULL, NULL);

            // Batch View Output
            hBatchOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, 10, 70, 460, 110, hwnd, NULL, NULL, NULL);

            // Favorites View
            hFavCombo = CreateWindowA("COMBOBOX", NULL, WS_CHILD | CBS_DROPDOWNLIST, 10, 70, 300, 150, hwnd, (HMENU)3003, NULL, NULL);

            // History Log View
            hHistoryOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, 10, 70, 460, 110, hwnd, NULL, NULL, NULL);
            CreateWindowA("BUTTON", "Export History Log", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 185, 130, 24, hwnd, (HMENU)4001, NULL, NULL);

            // Populate Category Combo
            for (int i = 0; i < numCats; i++) {
                SendMessageA(hCategory, CB_ADDSTRING, 0, (LPARAM)catNames[i]);
            }
            SendMessageA(hCategory, CB_SETCURSEL, 0, 0);

            // Populate Precision Combo
            SendMessageA(hPrecision, CB_ADDSTRING, 0, (LPARAM)"Auto");
            SendMessageA(hPrecision, CB_ADDSTRING, 0, (LPARAM)"0 Dec");
            SendMessageA(hPrecision, CB_ADDSTRING, 0, (LPARAM)"2 Dec");
            SendMessageA(hPrecision, CB_ADDSTRING, 0, (LPARAM)"4 Dec");
            SendMessageA(hPrecision, CB_ADDSTRING, 0, (LPARAM)"6 Dec");
            SendMessageA(hPrecision, CB_SETCURSEL, 0, 0);

            // Populate Format Combo
            SendMessageA(hFormat, CB_ADDSTRING, 0, (LPARAM)"Standard");
            SendMessageA(hFormat, CB_ADDSTRING, 0, (LPARAM)"Scientific");
            SendMessageA(hFormat, CB_SETCURSEL, 0, 0);

            PopulateUnits(0);

            hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            SendMessageA(hCategory, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hPrecision, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hFormat, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hFrom, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hTo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hOutput, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hFormulaStatic, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBatchOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hHistoryOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hFavCombo, WM_SETFONT, (WPARAM)hFont, TRUE);

            UpdateViewVisibility();
            DoConvert();
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == 1001) { // Convert Button
                DoConvert();
            } else if (wmId == 3001) { // Swap Button
                int fIdx = SendMessageA(hFrom, CB_GETCURSEL, 0, 0);
                int tIdx = SendMessageA(hTo, CB_GETCURSEL, 0, 0);
                SendMessageA(hFrom, CB_SETCURSEL, tIdx, 0);
                SendMessageA(hTo, CB_SETCURSEL, fIdx, 0);
                DoConvert();
            } else if (wmId == 3002) { // Pin Fav Button
                int cIdx = SendMessageA(hCategory, CB_GETCURSEL, 0, 0);
                int fIdx = SendMessageA(hFrom, CB_GETCURSEL, 0, 0);
                int tIdx = SendMessageA(hTo, CB_GETCURSEL, 0, 0);
                char favItem[128];
                m_sprintf(favItem, "%s: %s -> %s", catNames[cIdx], GetUnitName(cIdx, fIdx), GetUnitName(cIdx, tIdx));
                SendMessageA(hFavCombo, CB_ADDSTRING, 0, (LPARAM)favItem);
                SendMessageA(hFavCombo, CB_SETCURSEL, 0, 0);
                MessageBoxA(hwnd, "Pinned to Favorites list!", "KConverter", MB_OK | MB_ICONINFORMATION);
            } else if (wmId >= 2001 && wmId <= 2004) { // Mode tabs
                currentMode = wmId - 2001;
                UpdateViewVisibility();
                DoConvert();
            } else if ((wmId == 1002 || wmId == 1003 || wmId == 1004 || wmId == 1006 || wmId == 1007) && wmEvent == CBN_SELCHANGE) {
                if (wmId == 1002) {
                    int catIdx = SendMessageA(hCategory, CB_GETCURSEL, 0, 0);
                    PopulateUnits(catIdx);
                }
                DoConvert();
            } else if (wmId == 4001) { // Export History
                if (historyBuffer[0] == '\0') {
                    MessageBoxA(hwnd, "History log is empty!", "KConverter", MB_OK | MB_ICONWARNING);
                } else {
                    void* f = m_fopen("kconverter_history.txt", "w");
                    if (f) {
                        m_fputs(historyBuffer, f);
                        m_fclose(f);
                        MessageBoxA(hwnd, "History exported to kconverter_history.txt", "KConverter", MB_OK | MB_ICONINFORMATION);
                    }
                }
            }
            break;
        }
        case WM_DESTROY:
            if (hFont) DeleteObject(hFont);
            if (hFontBold) DeleteObject(hFontBold);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KConvClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KConvClass", "KConverter Pro", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 495, 260, NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
