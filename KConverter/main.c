int _fltused = 1;
#include <windows.h>
typedef BOOL(WINAPI *SetProcessDPIAwareFunc)();

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

HMODULE hMsvcrt = NULL;
HWND hCategory, hInput, hOutput, hFrom, hTo, hPrecision, hFormat;
HWND hBatchOutput, hHistoryOutput, hFavCombo, hFormulaStatic;
HWND hBtnSingle, hBtnBatch, hBtnFavs, hBtnHistory, hBtnExpress, hBtnHelp;
HWND hBtnLoadFav = NULL, hBtnRemoveFav = NULL;
HWND hExpressInput, hExpressOutput, hExpressPresetBtns[6];
HFONT hFont = NULL, hFontBold = NULL;
HBRUSH hStaticBkBrush = NULL;
WNDPROC OldEditProc = NULL;

char buffer[1024];
char historyBuffer[4096];
int currentMode = 0; // 0=Single, 1=Batch, 2=Favs, 3=History, 4=Express

void ShowHelpDialog(HWND hwnd);

// Categories and Units
const char* catNames[] = {"Length", "Weight", "Temperature", "Data Storage", "Speed", "Area", "Volume", "Time", "Pressure"};
const int numCats = 9;

const char* lenUnits[] = {"Meters", "Kilometers", "Centimeters", "Millimeters", "Miles", "Yards", "Feet", "Inches", "Nautical Miles"};
const double lenFactors[] = {1.0, 1000.0, 0.01, 0.001, 1609.344, 0.9144, 0.3048, 0.0254, 1852.0};
const int lenCount = 9;

const char* wtUnits[] = {"Kilograms", "Grams", "Milligrams", "Metric Tons", "Pounds", "Ounces", "Stone"};
const double wtFactors[] = {1.0, 0.001, 0.000001, 1000.0, 0.45359237, 0.028349523125, 6.35029318};
const int wtCount = 7;

const char* tempUnits[] = {"Celsius", "Fahrenheit", "Kelvin"};
const int tempCount = 3;

const char* dataUnits[] = {"Bytes", "Kilobytes (KB)", "Megabytes (MB)", "Gigabytes (GB)", "Terabytes (TB)", "Petabytes (PB)"};
const double dataFactors[] = {1.0, 1024.0, 1048576.0, 1073741824.0, 1099511627776.0, 1125899906842624.0};
const int dataCount = 6;

const char* speedUnits[] = {"Meters/sec", "Km/hour", "Miles/hour", "Knots", "Feet/sec"};
const double speedFactors[] = {1.0, 0.2777777777777778, 0.44704, 0.5144444444444445, 0.3048};
const int speedCount = 5;

const char* areaUnits[] = {"Sq Meters", "Sq Kilometers", "Sq Feet", "Acres", "Hectares"};
const double areaFactors[] = {1.0, 1000000.0, 0.09290304, 4046.8564224, 10000.0};
const int areaCount = 5;

const char* volUnits[] = {"Liters", "Milliliters", "Cubic Meters", "Gallons (US)", "Quarts (US)", "Fluid Oz"};
const double volFactors[] = {1.0, 0.001, 1000.0, 3.785411784, 0.946352946, 0.0295735295625};
const int volCount = 6;

const char* timeUnits[] = {"Seconds", "Minutes", "Hours", "Days", "Weeks", "Years"};
const double timeFactors[] = {1.0, 60.0, 3600.0, 86400.0, 604800.0, 31536000.0};
const int timeCount = 6;

const char* pressUnits[] = {"Pascal (Pa)", "Kilopascal (kPa)", "Bar", "PSI (lb/in\xC2\xB2)", "Atmosphere (atm)", "mmHg (Torr)"};
const double pressFactors[] = {1.0, 1000.0, 100000.0, 6894.757293, 101325.0, 133.322368};
const int pressCount = 6;

typedef struct {
    const char* token;
    int cat;
    int index;
    const char* dim;
} UnitAlias;

const UnitAlias aliasTable[] = {
    // Length
    {"m", 0, 0, "Length [L]"}, {"meter", 0, 0, "Length [L]"}, {"meters", 0, 0, "Length [L]"},
    {"km", 0, 1, "Length [L]"}, {"kilometer", 0, 1, "Length [L]"}, {"kilometers", 0, 1, "Length [L]"},
    {"cm", 0, 2, "Length [L]"}, {"centimeter", 0, 2, "Length [L]"}, {"centimeters", 0, 2, "Length [L]"},
    {"mm", 0, 3, "Length [L]"}, {"millimeter", 0, 3, "Length [L]"}, {"millimeters", 0, 3, "Length [L]"},
    {"mi", 0, 4, "Length [L]"}, {"mile", 0, 4, "Length [L]"}, {"miles", 0, 4, "Length [L]"},
    {"yd", 0, 5, "Length [L]"}, {"yard", 0, 5, "Length [L]"}, {"yards", 0, 5, "Length [L]"},
    {"ft", 0, 6, "Length [L]"}, {"feet", 0, 6, "Length [L]"}, {"foot", 0, 6, "Length [L]"},
    {"in", 0, 7, "Length [L]"}, {"inch", 0, 7, "Length [L]"}, {"inches", 0, 7, "Length [L]"},
    {"nmi", 0, 8, "Length [L]"},

    // Weight/Mass
    {"kg", 1, 0, "Mass [M]"}, {"kilogram", 1, 0, "Mass [M]"}, {"kilograms", 1, 0, "Mass [M]"},
    {"g", 1, 1, "Mass [M]"}, {"gram", 1, 1, "Mass [M]"}, {"grams", 1, 1, "Mass [M]"},
    {"mg", 1, 2, "Mass [M]"}, {"milligram", 1, 2, "Mass [M]"}, {"milligrams", 1, 2, "Mass [M]"},
    {"t", 1, 3, "Mass [M]"}, {"ton", 1, 3, "Mass [M]"}, {"tons", 1, 3, "Mass [M]"},
    {"lb", 1, 4, "Mass [M]"}, {"lbs", 1, 4, "Mass [M]"}, {"pound", 1, 4, "Mass [M]"}, {"pounds", 1, 4, "Mass [M]"},
    {"oz", 1, 5, "Mass [M]"}, {"ounce", 1, 5, "Mass [M]"}, {"ounces", 1, 5, "Mass [M]"},
    {"st", 1, 6, "Mass [M]"}, {"stone", 1, 6, "Mass [M]"},

    // Temp
    {"c", 2, 0, "Temperature [\xCE\x98]"}, {"celsius", 2, 0, "Temperature [\xCE\x98]"}, {"degc", 2, 0, "Temperature [\xCE\x98]"},
    {"f", 2, 1, "Temperature [\xCE\x98]"}, {"fahrenheit", 2, 1, "Temperature [\xCE\x98]"}, {"degf", 2, 1, "Temperature [\xCE\x98]"},
    {"k", 2, 2, "Temperature [\xCE\x98]"}, {"kelvin", 2, 2, "Temperature [\xCE\x98]"},

    // Data
    {"b", 3, 0, "Information [Bits]"}, {"byte", 3, 0, "Information [Bits]"}, {"bytes", 3, 0, "Information [Bits]"},
    {"kb", 3, 1, "Information [Bits]"},
    {"mb", 3, 2, "Information [Bits]"},
    {"gb", 3, 3, "Information [Bits]"},
    {"tb", 3, 4, "Information [Bits]"},
    {"pb", 3, 5, "Information [Bits]"},

    // Speed
    {"m/s", 4, 0, "Velocity [L \xC2\xB7 T\xE2\x81\xBB\xC2\xB9]"}, {"mps", 4, 0, "Velocity [L \xC2\xB7 T\xE2\x81\xBB\xC2\xB9]"},
    {"km/h", 4, 1, "Velocity [L \xC2\xB7 T\xE2\x81\xBB\xC2\xB9]"}, {"kph", 4, 1, "Velocity [L \xC2\xB7 T\xE2\x81\xBB\xC2\xB9]"}, {"kmh", 4, 1, "Velocity [L \xC2\xB7 T\xE2\x81\xBB\xC2\xB9]"},
    {"mph", 4, 2, "Velocity [L \xC2\xB7 T\xE2\x81\xBB\xC2\xB9]"},
    {"knot", 4, 3, "Velocity [L \xC2\xB7 T\xE2\x81\xBB\xC2\xB9]"}, {"knots", 4, 3, "Velocity [L \xC2\xB7 T\xE2\x81\xBB\xC2\xB9]"},
    {"ft/s", 4, 4, "Velocity [L \xC2\xB7 T\xE2\x81\xBB\xC2\xB9]"}, {"fps", 4, 4, "Velocity [L \xC2\xB7 T\xE2\x81\xBB\xC2\xB9]"},

    // Area
    {"m^2", 5, 0, "Area [L\xC2\xB2]"}, {"sqm", 5, 0, "Area [L\xC2\xB2]"}, {"m2", 5, 0, "Area [L\xC2\xB2]"},
    {"km^2", 5, 1, "Area [L\xC2\xB2]"}, {"sqkm", 5, 1, "Area [L\xC2\xB2]"}, {"km2", 5, 1, "Area [L\xC2\xB2]"},
    {"ft^2", 5, 2, "Area [L\xC2\xB2]"}, {"sqft", 5, 2, "Area [L\xC2\xB2]"}, {"ft2", 5, 2, "Area [L\xC2\xB2]"},
    {"acre", 5, 3, "Area [L\xC2\xB2]"}, {"acres", 5, 3, "Area [L\xC2\xB2]"},
    {"ha", 5, 4, "Area [L\xC2\xB2]"}, {"hectare", 5, 4, "Area [L\xC2\xB2]"},

    // Volume
    {"l", 6, 0, "Volume [L\xC2\xB3]"}, {"liter", 6, 0, "Volume [L\xC2\xB3]"}, {"liters", 6, 0, "Volume [L\xC2\xB3]"},
    {"ml", 6, 1, "Volume [L\xC2\xB3]"}, {"milliliter", 6, 1, "Volume [L\xC2\xB3]"},
    {"m^3", 6, 2, "Volume [L\xC2\xB3]"}, {"cbm", 6, 2, "Volume [L\xC2\xB3]"}, {"m3", 6, 2, "Volume [L\xC2\xB3]"},
    {"gal", 6, 3, "Volume [L\xC2\xB3]"}, {"gallon", 6, 3, "Volume [L\xC2\xB3]"}, {"gallons", 6, 3, "Volume [L\xC2\xB3]"},
    {"qt", 6, 4, "Volume [L\xC2\xB3]"}, {"quart", 6, 4, "Volume [L\xC2\xB3]"},
    {"floz", 6, 5, "Volume [L\xC2\xB3]"},

    // Time
    {"s", 7, 0, "Time [T]"}, {"sec", 7, 0, "Time [T]"}, {"second", 7, 0, "Time [T]"}, {"seconds", 7, 0, "Time [T]"},
    {"min", 7, 1, "Time [T]"}, {"minute", 7, 1, "Time [T]"}, {"minutes", 7, 1, "Time [T]"},
    {"h", 7, 2, "Time [T]"}, {"hr", 7, 2, "Time [T]"}, {"hour", 7, 2, "Time [T]"}, {"hours", 7, 2, "Time [T]"},
    {"d", 7, 3, "Time [T]"}, {"day", 7, 3, "Time [T]"}, {"days", 7, 3, "Time [T]"},
    {"wk", 7, 4, "Time [T]"}, {"week", 7, 4, "Time [T]"}, {"weeks", 7, 4, "Time [T]"},
    {"yr", 7, 5, "Time [T]"}, {"year", 7, 5, "Time [T]"}, {"years", 7, 5, "Time [T]"},

    // Pressure
    {"pa", 8, 0, "Pressure [M \xC2\xB7 L\xE2\x81\xBB\xC2\xB9 \xC2\xB7 T\xE2\x81\xBB\xC2\xB2]"}, {"pascal", 8, 0, "Pressure [M \xC2\xB7 L\xE2\x81\xBB\xC2\xB9 \xC2\xB7 T\xE2\x81\xBB\xC2\xB2]"},
    {"kpa", 8, 1, "Pressure [M \xC2\xB7 L\xE2\x81\xBB\xC2\xB9 \xC2\xB7 T\xE2\x81\xBB\xC2\xB2]"},
    {"bar", 8, 2, "Pressure [M \xC2\xB7 L\xE2\x81\xBB\xC2\xB9 \xC2\xB7 T\xE2\x81\xBB\xC2\xB2]"},
    {"psi", 8, 3, "Pressure [M \xC2\xB7 L\xE2\x81\xBB\xC2\xB9 \xC2\xB7 T\xE2\x81\xBB\xC2\xB2]"},
    {"atm", 8, 4, "Pressure [M \xC2\xB7 L\xE2\x81\xBB\xC2\xB9 \xC2\xB7 T\xE2\x81\xBB\xC2\xB2]"},
    {"mmhg", 8, 5, "Pressure [M \xC2\xB7 L\xE2\x81\xBB\xC2\xB9 \xC2\xB7 T\xE2\x81\xBB\xC2\xB2]"}, {"torr", 8, 5, "Pressure [M \xC2\xB7 L\xE2\x81\xBB\xC2\xB9 \xC2\xB7 T\xE2\x81\xBB\xC2\xB2]"}
};
const int aliasCount = sizeof(aliasTable) / sizeof(aliasTable[0]);

double GetFactor(int cat, int index) {
    if (cat == 0 && index >= 0 && index < lenCount) return lenFactors[index];
    if (cat == 1 && index >= 0 && index < wtCount) return wtFactors[index];
    if (cat == 3 && index >= 0 && index < dataCount) return dataFactors[index];
    if (cat == 4 && index >= 0 && index < speedCount) return speedFactors[index];
    if (cat == 5 && index >= 0 && index < areaCount) return areaFactors[index];
    if (cat == 6 && index >= 0 && index < volCount) return volFactors[index];
    if (cat == 7 && index >= 0 && index < timeCount) return timeFactors[index];
    if (cat == 8 && index >= 0 && index < pressCount) return pressFactors[index];
    return 1.0;
}

const char* GetUnitName(int cat, int index) {
    if (cat == 0 && index >= 0 && index < lenCount) return lenUnits[index];
    if (cat == 1 && index >= 0 && index < wtCount) return wtUnits[index];
    if (cat == 2 && index >= 0 && index < tempCount) return tempUnits[index];
    if (cat == 3 && index >= 0 && index < dataCount) return dataUnits[index];
    if (cat == 4 && index >= 0 && index < speedCount) return speedUnits[index];
    if (cat == 5 && index >= 0 && index < areaCount) return areaUnits[index];
    if (cat == 6 && index >= 0 && index < volCount) return volUnits[index];
    if (cat == 7 && index >= 0 && index < timeCount) return timeUnits[index];
    if (cat == 8 && index >= 0 && index < pressCount) return pressUnits[index];
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
    if (cat == 8) return pressCount;
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
    if (!out) return;
    if (formatIdx == 1) { // Scientific
        m_sprintf(out, "%.4e", val);
        return;
    }
    
    if (precIdx == 0) { // Auto
        if (val != 0.0 && (val > -1e-5 && val < 1e-5)) {
            m_sprintf(out, "%.4e", val);
        } else {
            m_sprintf(out, "%.6g", val);
        }
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
    if (!entry) return;
    char temp[4096];
    int entryLen = lstrlenA(entry);
    if (entryLen > 250) entryLen = 250;
    if (historyBuffer[0] != '\0') {
        int maxOld = 3800 - entryLen;
        if (maxOld < 0) maxOld = 0;
        historyBuffer[maxOld] = '\0';
        m_sprintf(temp, "%.250s\r\n%s", entry, historyBuffer);
    } else {
        m_sprintf(temp, "%.250s", entry);
    }
    temp[3900] = '\0';
    lstrcpyA(historyBuffer, temp);
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

    if (catIdx == CB_ERR || catIdx < 0 || catIdx >= numCats) catIdx = 0;
    if (fromIdx == CB_ERR || fromIdx < 0) fromIdx = 0;
    if (toIdx == CB_ERR || toIdx < 0) toIdx = 0;
    if (precIdx == CB_ERR || precIdx < 0) precIdx = 0;
    if (formatIdx == CB_ERR || formatIdx < 0) formatIdx = 0;

    double result = 0;
    BOOL isBelowAbsZero = FALSE;

    if (catIdx == 2) { // Temp
        double c = 0;
        if (fromIdx == 0) c = val;
        else if (fromIdx == 1) c = (val - 32.0) * 5.0 / 9.0;
        else if (fromIdx == 2) c = val - 273.15;
        
        if (c < -273.15) isBelowAbsZero = TRUE;

        if (toIdx == 0) result = c;
        else if (toIdx == 1) result = (c * 9.0 / 5.0) + 32.0;
        else if (toIdx == 2) result = c + 273.15;
    } else {
        double fromFactor = GetFactor(catIdx, fromIdx);
        double toFactor = GetFactor(catIdx, toIdx);
        if (toFactor <= 0.0) toFactor = 1.0;
        double baseVal = val * fromFactor;
        result = baseVal / toFactor;
    }

    char resStr[128];
    FormatValue(result, resStr, precIdx, formatIdx);
    SetWindowTextA(hOutput, resStr);

    // Formula label
    const char* fromName = GetUnitName(catIdx, fromIdx);
    const char* toName = GetUnitName(catIdx, toIdx);
    char eqStr[384];
    if (isBelowAbsZero) {
        m_sprintf(eqStr, "Formula: %g %s = %s %s (Below Abs Zero!)", val, fromName, resStr, toName);
    } else {
        m_sprintf(eqStr, "Formula: %g %s = %s %s", val, fromName, resStr, toName);
    }
    SetWindowTextA(hFormulaStatic, eqStr);

    // Log entry
    char logLine[256];
    m_sprintf(logLine, "[%s] %g %s -> %s %s", catNames[catIdx], val, fromName, resStr, toName);
    AppendHistory(logLine);

    // Update Batch View if visible
    int uCount = GetUnitCount(catIdx);
    char batchBuf[3500];
    batchBuf[0] = '\0';
    char line[256];
    for (int i = 0; i < uCount; i++) {
        double uRes = 0;
        if (catIdx == 2) {
            double c = 0;
            if (fromIdx == 0) c = val;
            else if (fromIdx == 1) c = (val - 32.0) * 5.0 / 9.0;
            else if (fromIdx == 2) c = val - 273.15;

            if (i == 0) uRes = c;
            else if (i == 1) uRes = (c * 9.0 / 5.0) + 32.0;
            else if (i == 2) uRes = c + 273.15;
        } else {
            double fTo = GetFactor(catIdx, i);
            if (fTo <= 0.0) fTo = 1.0;
            double baseVal = val * GetFactor(catIdx, fromIdx);
            uRes = baseVal / fTo;
        }
        char uStr[128];
        FormatValue(uRes, uStr, precIdx, formatIdx);
        m_sprintf(line, "%s: %s\r\n", GetUnitName(catIdx, i), uStr);
        if (lstrlenA(batchBuf) + lstrlenA(line) < 3400) {
            lstrcatA(batchBuf, line);
        }
    }
    SetWindowTextA(hBatchOutput, batchBuf);
}

int MatchAlias(const char* str, const UnitAlias** outAlias) {
    char clean[64];
    int len = 0;
    for (int i = 0; str[i] && len < 63; i++) {
        char c = str[i];
        if (c >= 'A' && c <= 'Z') c += 32;
        if (c != ' ' && c != '\t') {
            clean[len++] = c;
        }
    }
    clean[len] = '\0';

    for (int i = 0; i < aliasCount; i++) {
        if (lstrcmpiA(clean, aliasTable[i].token) == 0) {
            if (outAlias) *outAlias = &aliasTable[i];
            return 1;
        }
    }
    return 0;
}

void DoExpressParse() {
    char expr[256];
    GetWindowTextA(hExpressInput, expr, 255);
    
    char* p = expr;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0') {
        SetWindowTextA(hExpressOutput, "Enter an expression like: 100 km/h to m/s or 50 psi to bar");
        return;
    }

    double val = m_atof(p);
    
    while (*p && ((*p >= '0' && *p <= '9') || *p == '.' || *p == '-' || *p == '+' || 
                  ((*p == 'e' || *p == 'E') && (p[1] == '+' || p[1] == '-' || (p[1] >= '0' && p[1] <= '9'))) || 
                  *p == ' ' || *p == '\t')) {
        if (*p == ' ' || *p == '\t') {
            char* next = p;
            while (*next == ' ' || *next == '\t') next++;
            if (*next < '0' || *next > '9') {
                if (*next != '.' && *next != '-' && *next != '+') break;
            }
        }
        p++;
    }
    
    char uFromStr[64] = {0};
    char uToStr[64] = {0};
    
    int fLen = 0;
    while (*p) {
        if (((p[0] == ' ' || p[0] == '\t') && (p[1] == 't' || p[1] == 'T') && (p[2] == 'o' || p[2] == 'O') && (p[3] == ' ' || p[3] == '\t' || p[3] == '\0')) ||
            (p[0] == '-' && p[1] == '>') ||
            (p[0] == '=') ||
            ((p[0] == ' ' || p[0] == '\t') && (p[1] == 'i' || p[1] == 'I') && (p[2] == 'n' || p[2] == 'N') && (p[3] == ' ' || p[3] == '\t' || p[3] == '\0'))) {
            if (p[0] == '-') p += 2;
            else if (p[0] == '=') p += 1;
            else p += 3;
            break;
        }
        if (fLen < 63) uFromStr[fLen++] = *p;
        p++;
    }
    uFromStr[fLen] = '\0';
    
    while (*p == ' ' || *p == '\t') p++;
    int tLen = 0;
    while (*p && tLen < 63) {
        uToStr[tLen++] = *p++;
    }
    uToStr[tLen] = '\0';
    
    const UnitAlias* fromAlias = NULL;
    const UnitAlias* toAlias = NULL;
    
    int okFrom = MatchAlias(uFromStr, &fromAlias);
    int okTo = MatchAlias(uToStr, &toAlias);
    
    char outBuf[1024];
    if (!okFrom || !okTo) {
        m_sprintf(outBuf, 
            "=== EXPRESSION PARSE ERROR ===\r\n"
            "Unrecognized unit tokens in expression: '%s'\r\n\r\n"
            "From Token Recognized: %s (%s)\r\n"
            "To Token Recognized:   %s (%s)\r\n\r\n"
            "Supported Expression Format:\r\n"
            "  <value> <from_unit> to <to_unit>\r\n\r\n"
            "Supported Unit Tokens:\r\n"
            "  Speed: km/h, kph, m/s, mps, mph, knot, ft/s, fps\r\n"
            "  Pressure: psi, bar, kPa, Pa, atm, mmHg, torr\r\n"
            "  Temp: c, f, k, celsius, fahrenheit, kelvin\r\n"
            "  Length: m, km, cm, mm, mi, yd, ft, in, nmi\r\n"
            "  Weight: kg, g, mg, t, lb, oz, st\r\n"
            "  Data: b, kb, mb, gb, tb, pb\r\n"
            "  Area: m^2, sqm, km^2, sqft, ft^2, acre, ha\r\n"
            "  Volume: l, ml, m^3, cbm, gal, qt, floz\r\n"
            "  Time: sec, min, hr, day, wk, yr",
            expr, 
            okFrom ? "YES" : "NO", uFromStr,
            okTo ? "YES" : "NO", uToStr);
        SetWindowTextA(hExpressOutput, outBuf);
        return;
    }
    
    if (fromAlias->cat != toAlias->cat) {
        m_sprintf(outBuf,
            "=== DIMENSION MISMATCH ERROR ===\r\n"
            "Cannot convert between different physical dimensions!\r\n\r\n"
            "From Unit: '%s' -> Dimension: %s (%s)\r\n"
            "To Unit:   '%s' -> Dimension: %s (%s)\r\n\r\n"
            "Status: CONVERSION INVALID",
            uFromStr, fromAlias->dim, catNames[fromAlias->cat],
            uToStr, toAlias->dim, catNames[toAlias->cat]);
        SetWindowTextA(hExpressOutput, outBuf);
        return;
    }
    
    int cat = fromAlias->cat;
    double result = 0.0;
    
    if (cat == 2) { // Temp
        double c = 0;
        if (fromAlias->index == 0) c = val;
        else if (fromAlias->index == 1) c = (val - 32.0) * 5.0 / 9.0;
        else if (fromAlias->index == 2) c = val - 273.15;
        
        if (toAlias->index == 0) result = c;
        else if (toAlias->index == 1) result = (c * 9.0 / 5.0) + 32.0;
        else if (toAlias->index == 2) result = c + 273.15;
    } else {
        double fFrom = GetFactor(cat, fromAlias->index);
        double fTo = GetFactor(cat, toAlias->index);
        if (fTo <= 0.0) fTo = 1.0;
        result = (val * fFrom) / fTo;
    }
    
    char resStr[128];
    int precIdx = SendMessageA(hPrecision, CB_GETCURSEL, 0, 0);
    int formatIdx = SendMessageA(hFormat, CB_GETCURSEL, 0, 0);
    if (precIdx == CB_ERR || precIdx < 0) precIdx = 0;
    if (formatIdx == CB_ERR || formatIdx < 0) formatIdx = 0;
    FormatValue(result, resStr, precIdx, formatIdx);
    
    m_sprintf(outBuf,
        "=== SMART EXPRESSION EVALUATION ===\r\n"
        "Input Expression:   %s\r\n"
        "Evaluated Value:    %s %s\r\n\r\n"
        "Physical Dimension: %s\r\n"
        "Unit Category:      %s\r\n"
        "From Unit:          %s\r\n"
        "To Unit:            %s\r\n\r\n"
        "Formula Breakdown:  %g %s = %s %s\r\n"
        "Status:             VALID CONVERSION",
        expr, resStr, GetUnitName(cat, toAlias->index),
        fromAlias->dim, catNames[cat],
        GetUnitName(cat, fromAlias->index), GetUnitName(cat, toAlias->index),
        val, GetUnitName(cat, fromAlias->index), resStr, GetUnitName(cat, toAlias->index));
        
    SetWindowTextA(hExpressOutput, outBuf);
    
    char logLine[256];
    m_sprintf(logLine, "[Express:%s] %g %s -> %s %s", catNames[cat], val, GetUnitName(cat, fromAlias->index), resStr, GetUnitName(cat, toAlias->index));
    AppendHistory(logLine);
}

void UpdateViewVisibility() {
    BOOL isSingle = (currentMode == 0);
    BOOL isBatch = (currentMode == 1);
    BOOL isFav = (currentMode == 2);
    BOOL isHistory = (currentMode == 3);
    BOOL isExpress = (currentMode == 4);

    ShowWindow(hInput, isSingle ? SW_SHOW : SW_HIDE);
    ShowWindow(hFrom, isSingle ? SW_SHOW : SW_HIDE);
    ShowWindow(hTo, isSingle ? SW_SHOW : SW_HIDE);
    ShowWindow(hOutput, isSingle ? SW_SHOW : SW_HIDE);
    ShowWindow(hFormulaStatic, isSingle ? SW_SHOW : SW_HIDE);

    ShowWindow(hBatchOutput, isBatch ? SW_SHOW : SW_HIDE);
    ShowWindow(hFavCombo, isFav ? SW_SHOW : SW_HIDE);
    if (hBtnLoadFav) ShowWindow(hBtnLoadFav, isFav ? SW_SHOW : SW_HIDE);
    if (hBtnRemoveFav) ShowWindow(hBtnRemoveFav, isFav ? SW_SHOW : SW_HIDE);
    ShowWindow(hHistoryOutput, isHistory ? SW_SHOW : SW_HIDE);

    ShowWindow(hExpressInput, isExpress ? SW_SHOW : SW_HIDE);
    ShowWindow(hExpressOutput, isExpress ? SW_SHOW : SW_HIDE);
    for (int i = 0; i < 6; i++) {
        if (hExpressPresetBtns[i]) ShowWindow(hExpressPresetBtns[i], isExpress ? SW_SHOW : SW_HIDE);
    }
}

// Subclass Edit Proc to handle ENTER and F1 keys in Input box
LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN) {
        if (wParam == VK_RETURN) {
            if (hwnd == hExpressInput) {
                DoExpressParse();
            } else {
                DoConvert();
            }
            return 0;
        } else if (wParam == VK_F1) {
            ShowHelpDialog(GetParent(hwnd));
            return 0;
        }
    }
    return CallWindowProcA(OldEditProc, hwnd, msg, wParam, lParam);
}

void ShowHelpDialog(HWND hwnd) {
    MessageBoxA(hwnd,
        "========================================\r\n"
        "   KCONVERTER PRO - QUICK GUIDE\r\n"
        "========================================\r\n\r\n"
        "NAVIGATION & SHORTCUTS:\r\n"
        "  [F1] or [H]  : Display this Help guide\r\n"
        "  [1] - [5]    : Switch Tab Modes:\r\n"
        "                 1: Single, 2: Batch, 3: Favs, 4: History, 5: Parser\r\n"
        "  [Enter]      : Calculate / Evaluate active expression\r\n"
        "  [⇄ Swap]    : Invert From and To units\r\n"
        "  [⭐ Pin]     : Save conversion pair to Favorites\r\n\r\n"
        "FEATURES:\r\n"
        "  - Single Convert : Bi-directional unit conversion with live formula\r\n"
        "  - Batch Mode     : See conversion to all units in the category at once\r\n"
        "  - Favorites      : Instant recall of pinned unit pairs\r\n"
        "  - History Log    : Conversion audit trail with export to text\r\n"
        "  - Smart Parser   : Evaluates phrases like '100 km/h to m/s' or '50 psi to bar'\r\n\r\n"
        "PHYSICAL CATEGORIES (9 total):\r\n"
        "  Length, Weight, Temperature, Data Storage, Speed, Area, Volume, Time, Pressure",
        "KConverter Pro Help", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hMsvcrt = LoadLibraryA("msvcrt.dll");
            if (hMsvcrt) {
                m_sprintf = (sprintf_t)GetProcAddress(hMsvcrt, "sprintf");
                m_atof = (atof_t)GetProcAddress(hMsvcrt, "atof");
                m_fopen = (fopen_t)GetProcAddress(hMsvcrt, "fopen");
                m_fputs = (fputs_t)GetProcAddress(hMsvcrt, "fputs");
                m_fclose = (fclose_t)GetProcAddress(hMsvcrt, "fclose");
            }

            historyBuffer[0] = '\0';

            // Top bar controls
            CreateWindowA("STATIC", "Category:", WS_CHILD | WS_VISIBLE, 10, 10, 60, 20, hwnd, NULL, NULL, NULL);
            hCategory = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 75, 8, 120, 150, hwnd, (HMENU)1002, NULL, NULL);

            CreateWindowA("STATIC", "Prec:", WS_CHILD | WS_VISIBLE, 205, 10, 35, 20, hwnd, NULL, NULL, NULL);
            hPrecision = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 245, 8, 70, 150, hwnd, (HMENU)1003, NULL, NULL);

            CreateWindowA("STATIC", "Fmt:", WS_CHILD | WS_VISIBLE, 325, 10, 30, 20, hwnd, NULL, NULL, NULL);
            hFormat = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 360, 8, 95, 150, hwnd, (HMENU)1004, NULL, NULL);

            hBtnHelp = CreateWindowA("BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 465, 8, 80, 24, hwnd, (HMENU)6001, NULL, NULL);

            // Mode Tab Buttons
            hBtnSingle = CreateWindowA("BUTTON", "[1] Single", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 10, 38, 75, 24, hwnd, (HMENU)2001, NULL, NULL);
            hBtnBatch = CreateWindowA("BUTTON", "[2] Batch", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 90, 38, 75, 24, hwnd, (HMENU)2002, NULL, NULL);
            hBtnFavs = CreateWindowA("BUTTON", "[3] Favs", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 170, 38, 70, 24, hwnd, (HMENU)2003, NULL, NULL);
            hBtnHistory = CreateWindowA("BUTTON", "[4] History", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 245, 38, 80, 24, hwnd, (HMENU)2004, NULL, NULL);
            hBtnExpress = CreateWindowA("BUTTON", "[5] Parser ⚡", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 330, 38, 95, 24, hwnd, (HMENU)2005, NULL, NULL);

            // Single View Controls
            CreateWindowA("STATIC", "Input:", WS_CHILD | WS_VISIBLE, 10, 72, 45, 20, hwnd, NULL, NULL, NULL);
            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 60, 70, 100, 24, hwnd, (HMENU)1005, NULL, NULL);
            OldEditProc = (WNDPROC)SetWindowLongPtrA(hInput, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

            CreateWindowA("STATIC", "From:", WS_CHILD | WS_VISIBLE, 170, 72, 40, 20, hwnd, NULL, NULL, NULL);
            hFrom = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 215, 70, 140, 150, hwnd, (HMENU)1006, NULL, NULL);

            CreateWindowA("BUTTON", "⇄ Swap", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 365, 70, 65, 24, hwnd, (HMENU)3001, NULL, NULL);
            CreateWindowA("BUTTON", "⭐ Pin", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 435, 70, 55, 24, hwnd, (HMENU)3002, NULL, NULL);

            CreateWindowA("STATIC", "To:", WS_CHILD | WS_VISIBLE, 170, 102, 40, 20, hwnd, NULL, NULL, NULL);
            hTo = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 215, 100, 140, 150, hwnd, (HMENU)1007, NULL, NULL);

            CreateWindowA("STATIC", "Result:", WS_CHILD | WS_VISIBLE, 10, 132, 50, 20, hwnd, NULL, NULL, NULL);
            hOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_READONLY | ES_AUTOHSCROLL | WS_TABSTOP, 60, 130, 295, 24, hwnd, NULL, NULL, NULL);
            CreateWindowA("BUTTON", "Convert", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 365, 100, 125, 54, hwnd, (HMENU)1001, NULL, NULL);

            hFormulaStatic = CreateWindowA("STATIC", "Formula: 1 Meter = 1 Meter", WS_CHILD | WS_VISIBLE, 10, 164, 520, 20, hwnd, NULL, NULL, NULL);

            // Batch View Output
            hBatchOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL | WS_TABSTOP, 10, 70, 560, 260, hwnd, NULL, NULL, NULL);

            // Favorites View
            hFavCombo = CreateWindowA("COMBOBOX", NULL, WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP, 10, 70, 360, 150, hwnd, (HMENU)3003, NULL, NULL);
            hBtnLoadFav = CreateWindowA("BUTTON", "Load", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, 380, 68, 80, 26, hwnd, (HMENU)3004, NULL, NULL);
            hBtnRemoveFav = CreateWindowA("BUTTON", "Remove", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, 470, 68, 80, 26, hwnd, (HMENU)3005, NULL, NULL);

            // History Log View
            hHistoryOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL | WS_TABSTOP, 10, 70, 560, 260, hwnd, NULL, NULL, NULL);
            CreateWindowA("BUTTON", "Export History Log", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP, 10, 340, 140, 26, hwnd, (HMENU)4001, NULL, NULL);

            // Smart Parser View Controls
            hExpressInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "100 km/h to m/s", WS_CHILD | ES_AUTOHSCROLL | WS_TABSTOP, 10, 70, 390, 24, hwnd, (HMENU)5010, NULL, NULL);
            SetWindowLongPtrA(hExpressInput, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
            CreateWindowA("BUTTON", "Evaluate", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, 410, 70, 80, 24, hwnd, (HMENU)5000, NULL, NULL);

            hExpressPresetBtns[0] = CreateWindowA("BUTTON", "100km/h->m/s", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, 10, 98, 90, 22, hwnd, (HMENU)5001, NULL, NULL);
            hExpressPresetBtns[1] = CreateWindowA("BUTTON", "50psi->bar", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, 105, 98, 75, 22, hwnd, (HMENU)5002, NULL, NULL);
            hExpressPresetBtns[2] = CreateWindowA("BUTTON", "250f->c", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, 185, 98, 60, 22, hwnd, (HMENU)5003, NULL, NULL);
            hExpressPresetBtns[3] = CreateWindowA("BUTTON", "1024mb->gb", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, 250, 98, 80, 22, hwnd, (HMENU)5004, NULL, NULL);
            hExpressPresetBtns[4] = CreateWindowA("BUTTON", "5000m2->acre", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, 335, 98, 90, 22, hwnd, (HMENU)5005, NULL, NULL);
            hExpressPresetBtns[5] = CreateWindowA("BUTTON", "5gal->l", WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP, 430, 98, 55, 22, hwnd, (HMENU)5006, NULL, NULL);

            hExpressOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL | WS_TABSTOP, 10, 126, 560, 204, hwnd, NULL, NULL, NULL);

            RegisterHotKey(hwnd, 1, 0, VK_F1);

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

            hFont = CreateFontA(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hFontBold = CreateFontA(-14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            SendMessageA(hCategory, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hPrecision, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hFormat, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnHelp, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnSingle, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnBatch, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnFavs, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnHistory, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnExpress, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hFrom, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hTo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hOutput, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hFormulaStatic, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBatchOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hHistoryOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hFavCombo, WM_SETFONT, (WPARAM)hFont, TRUE);
            if (hBtnLoadFav) SendMessageA(hBtnLoadFav, WM_SETFONT, (WPARAM)hFont, TRUE);
            if (hBtnRemoveFav) SendMessageA(hBtnRemoveFav, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hExpressInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hExpressOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
            for (int i = 0; i < 6; i++) {
                if (hExpressPresetBtns[i]) SendMessageA(hExpressPresetBtns[i], WM_SETFONT, (WPARAM)hFont, TRUE);
            }

            UpdateViewVisibility();
            DoConvert();
            DoExpressParse();
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == 6001) { // Help Button
                ShowHelpDialog(hwnd);
            } else if (wmId == 1001) { // Convert Button
                DoConvert();
            } else if (wmId == 1005 && wmEvent == EN_CHANGE) { // Live input update
                DoConvert();
            } else if (wmId == 3001) { // Swap Button
                int fIdx = SendMessageA(hFrom, CB_GETCURSEL, 0, 0);
                int tIdx = SendMessageA(hTo, CB_GETCURSEL, 0, 0);
                SendMessageA(hFrom, CB_SETCURSEL, tIdx, 0);
                SendMessageA(hTo, CB_SETCURSEL, fIdx, 0);
                DoConvert();
            } else if (wmId == 3002) { // Pin Fav Button
                int cIdx = (int)SendMessageA(hCategory, CB_GETCURSEL, 0, 0);
                int fIdx = (int)SendMessageA(hFrom, CB_GETCURSEL, 0, 0);
                int tIdx = (int)SendMessageA(hTo, CB_GETCURSEL, 0, 0);
                if (cIdx != CB_ERR && fIdx != CB_ERR && tIdx != CB_ERR) {
                    char favItem[128];
                    m_sprintf(favItem, "%s: %s -> %s", catNames[cIdx], GetUnitName(cIdx, fIdx), GetUnitName(cIdx, tIdx));
                    int idx = (int)SendMessageA(hFavCombo, CB_ADDSTRING, 0, (LPARAM)favItem);
                    LPARAM packed = (LPARAM)((cIdx & 0xFF) | ((fIdx & 0xFF) << 8) | ((tIdx & 0xFF) << 16));
                    SendMessageA(hFavCombo, CB_SETITEMDATA, (WPARAM)idx, packed);
                    SendMessageA(hFavCombo, CB_SETCURSEL, (WPARAM)idx, 0);
                    MessageBoxA(hwnd, "Pinned to Favorites list!", "KConverter", MB_OK | MB_ICONINFORMATION);
                }
            } else if (wmId == 3004 || (wmId == 3003 && wmEvent == CBN_SELCHANGE)) { // Load Favorite
                int sel = (int)SendMessageA(hFavCombo, CB_GETCURSEL, 0, 0);
                if (sel != CB_ERR) {
                    LPARAM data = SendMessageA(hFavCombo, CB_GETITEMDATA, (WPARAM)sel, 0);
                    int cIdx = (int)(data & 0xFF);
                    int fIdx = (int)((data >> 8) & 0xFF);
                    int tIdx = (int)((data >> 16) & 0xFF);
                    if (cIdx >= 0 && cIdx < numCats) {
                        SendMessageA(hCategory, CB_SETCURSEL, cIdx, 0);
                        PopulateUnits(cIdx);
                        SendMessageA(hFrom, CB_SETCURSEL, fIdx, 0);
                        SendMessageA(hTo, CB_SETCURSEL, tIdx, 0);
                        currentMode = 0;
                        UpdateViewVisibility();
                        DoConvert();
                    }
                }
            } else if (wmId == 3005) { // Remove Favorite
                int sel = (int)SendMessageA(hFavCombo, CB_GETCURSEL, 0, 0);
                if (sel != CB_ERR) {
                    SendMessageA(hFavCombo, CB_DELETESTRING, (WPARAM)sel, 0);
                    int count = (int)SendMessageA(hFavCombo, CB_GETCOUNT, 0, 0);
                    if (count > 0) {
                        int newSel = (sel >= count) ? count - 1 : sel;
                        SendMessageA(hFavCombo, CB_SETCURSEL, (WPARAM)newSel, 0);
                    }
                }
            } else if (wmId >= 2001 && wmId <= 2005) { // Mode tabs
                currentMode = wmId - 2001;
                UpdateViewVisibility();
                if (currentMode == 4) DoExpressParse();
                else DoConvert();
            } else if ((wmId == 1002 || wmId == 1003 || wmId == 1004 || wmId == 1006 || wmId == 1007) && wmEvent == CBN_SELCHANGE) {
                if (wmId == 1002) {
                    int catIdx = SendMessageA(hCategory, CB_GETCURSEL, 0, 0);
                    PopulateUnits(catIdx);
                }
                DoConvert();
                if (currentMode == 4) DoExpressParse();
            } else if (wmId == 4001) { // Export History
                if (historyBuffer[0] == '\0') {
                    MessageBoxA(hwnd, "History log is empty!", "KConverter", MB_OK | MB_ICONWARNING);
                } else {
                    if (m_fopen && m_fputs && m_fclose) {
                        void* f = m_fopen("kconverter_history.txt", "w");
                        if (f) {
                            m_fputs(historyBuffer, f);
                            m_fclose(f);
                            MessageBoxA(hwnd, "History exported to kconverter_history.txt", "KConverter", MB_OK | MB_ICONINFORMATION);
                        }
                    }
                }
            } else if (wmId == 5000) { // Evaluate Express
                DoExpressParse();
            } else if (wmId == 5010 && wmEvent == EN_CHANGE) { // Live express parse
                if (currentMode == 4) DoExpressParse();
            } else if (wmId >= 5001 && wmId <= 5006) { // Presets
                const char* pText = "100 km/h to m/s";
                if (wmId == 5002) pText = "50 psi to bar";
                else if (wmId == 5003) pText = "250 f to c";
                else if (wmId == 5004) pText = "1024 mb to gb";
                else if (wmId == 5005) pText = "5000 m^2 to acre";
                else if (wmId == 5006) pText = "5 gal to l";
                SetWindowTextA(hExpressInput, pText);
                DoExpressParse();
            }
            break;
        }
        case WM_KEYDOWN: {
            HWND hFocus = GetFocus();
            BOOL isEditing = (hFocus == hInput || hFocus == hExpressInput || hFocus == hHistoryOutput || hFocus == hBatchOutput);
            if (wParam == VK_F1) {
                ShowHelpDialog(hwnd);
                return 0;
            }
            if (!isEditing) {
                if (wParam == '1') {
                    currentMode = 0;
                    UpdateViewVisibility();
                    DoConvert();
                    return 0;
                } else if (wParam == '2') {
                    currentMode = 1;
                    UpdateViewVisibility();
                    DoConvert();
                    return 0;
                } else if (wParam == '3') {
                    currentMode = 2;
                    UpdateViewVisibility();
                    return 0;
                } else if (wParam == '4') {
                    currentMode = 3;
                    UpdateViewVisibility();
                    return 0;
                } else if (wParam == '5') {
                    currentMode = 4;
                    UpdateViewVisibility();
                    DoExpressParse();
                    return 0;
                } else if (wParam == 'H' || wParam == 'h') {
                    ShowHelpDialog(hwnd);
                    return 0;
                }
            }
            break;
        }
        case WM_HOTKEY: {
            if (wParam == 1) {
                ShowHelpDialog(hwnd);
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);
            if (!hStaticBkBrush) {
                hStaticBkBrush = GetSysColorBrush(COLOR_BTNFACE);
            }
            return (LRESULT)hStaticBkBrush;
        }
        case WM_DESTROY:
            if (hFont) { DeleteObject(hFont); hFont = NULL; }
            if (hFontBold) { DeleteObject(hFontBold); hFontBold = NULL; }
            if (hMsvcrt) { FreeLibrary(hMsvcrt); hMsvcrt = NULL; }
            UnregisterHotKey(hwnd, 1);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        SetProcessDPIAwareFunc setDpi = (SetProcessDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (setDpi) setDpi();
    }

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KConvClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    RegisterClassA(&wc);
    
    RECT rect = { 0, 0, 660, 480 };
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    HWND hwnd = CreateWindowExA(0, "KConvClass", "KConverter Pro - [F1] Help", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, wc.hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_F1) {
                ShowHelpDialog(hwnd);
                continue;
            }
            HWND hFoc = GetFocus();
            BOOL isEdit = (hFoc == hInput || hFoc == hExpressInput || hFoc == hHistoryOutput || hFoc == hBatchOutput);
            if (!isEdit) {
                if (msg.wParam >= '1' && msg.wParam <= '5') {
                    SendMessageA(hwnd, WM_COMMAND, MAKEWPARAM(2001 + (msg.wParam - '1'), 0), 0);
                    continue;
                }
                if (msg.wParam == 'H' || msg.wParam == 'h') {
                    ShowHelpDialog(hwnd);
                    continue;
                }
            }
        }
        if (!IsDialogMessageA(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    ExitProcess(0);
}