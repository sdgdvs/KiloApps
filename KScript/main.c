#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>

#define W 980
#define H 640

HWND hMainWnd = NULL;
HWND hInput, hOutput, hMemory, hRegexFind, hRegexRep;
HWND hBtnRun, hBtnLoad, hBtnSave, hBtnStep, hBtnBench, hBtnRec, hBtnPlay, hBtnRep, hBtnHelp, hBtnClear;

#define MAX_VARS 256
#define MAX_VAR_NAME 32

typedef struct {
    char name[MAX_VAR_NAME];
    int val;
    int assigned;
} VarEntry;

VarEntry g_vars[MAX_VARS];
int g_varCount = 0;

int nodeCount = 0;
int dpi = 96;
#define S(x) MulDiv(x, dpi, 96)

const char* debugPtr = NULL;
char debugInput[65536];
char outStr[65536];
char memStr[65536];
int lastExecMicros = 0;

int isRecording = 0;
char macroBuf[8192];
int macroLen = 0;
WNDPROC oldEditProc;
WNDPROC oldFindProc;
WNDPROC oldRepProc;

// String helpers
int StrLen(const char* s) { int c = 0; while (*s++) c++; return c; }
int IsAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
int IsAlnum(char c) { return IsAlpha(c) || (c >= '0' && c <= '9'); }
int IsHexDigit(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
int HexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

int StrEqualCase(const char* a, const char* b) {
    while (*a && *b) {
        char ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a;
        char cb = (*b >= 'A' && *b <= 'Z') ? *b + 32 : *b;
        if (ca != cb) return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

void IntToStr(int val, char* buf) {
    wsprintfA(buf, "%d", val);
}

// Math built-ins
int MathAbs(int x) { return x < 0 ? -x : x; }
int MathMin(int a, int b) { return a < b ? a : b; }
int MathMax(int a, int b) { return a > b ? a : b; }
int MathClamp(int x, int lo, int hi) { return x < lo ? lo : (x > hi ? hi : x); }
int MathSign(int x) { return x > 0 ? 1 : (x < 0 ? -1 : 0); }
int MathGcd(int a, int b) {
    a = MathAbs(a); b = MathAbs(b);
    while (b != 0) { int t = b; b = a % b; a = t; }
    return a;
}
int MathSqrt(int x) {
    if (x <= 0) return 0;
    int res = 0;
    int bit = 1 << 30;
    while (bit > x) bit >>= 2;
    while (bit != 0) {
        if (x >= res + bit) {
            x -= res + bit;
            res = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }
    return res;
}
static unsigned int g_randSeed = 123456789;
int MathRand(int minVal, int maxVal) {
    if (minVal >= maxVal) return minVal;
    g_randSeed = g_randSeed * 1103515245 + 12345;
    unsigned int r = (g_randSeed >> 16) & 0x7FFF;
    return minVal + (int)(r % (unsigned int)(maxVal - minVal + 1));
}
int MathPow(int base, int exp) {
    if (exp <= 0) return 1;
    int res = 1;
    while (exp > 0) {
        if (exp & 1) res *= base;
        base *= base;
        exp >>= 1;
    }
    return res;
}

// Variable Table
int FindVariable(const char* name) {
    for (int i = 0; i < g_varCount; i++) {
        if (StrEqualCase(g_vars[i].name, name)) return i;
    }
    return -1;
}

int GetOrCreateVariable(const char* name) {
    int idx = FindVariable(name);
    if (idx != -1) return idx;
    if (g_varCount < MAX_VARS) {
        idx = g_varCount++;
        int i = 0;
        while (name[i] && i < MAX_VAR_NAME - 1) {
            g_vars[idx].name[i] = name[i];
            i++;
        }
        g_vars[idx].name[i] = '\0';
        g_vars[idx].val = 0;
        g_vars[idx].assigned = 0;
        return idx;
    }
    return -1;
}

void ClearVariables() {
    g_varCount = 0;
    for (int i = 0; i < MAX_VARS; i++) {
        g_vars[i].name[0] = '\0';
        g_vars[i].val = 0;
        g_vars[i].assigned = 0;
    }
}

// Parser prototypes
int ParseExpr(const char** p);
int ParseLogicalOr(const char** p);
int ParseLogicalAnd(const char** p);
int ParseBitwiseOr(const char** p);
int ParseBitwiseXor(const char** p);
int ParseBitwiseAnd(const char** p);
int ParseEquality(const char** p);
int ParseRelational(const char** p);
int ParseShift(const char** p);
int ParseAdditive(const char** p);
int ParseMultiplicative(const char** p);
int ParsePower(const char** p);
int ParseUnary(const char** p);
int ParsePrimary(const char** p);

void SkipWhitespace(const char** p) {
    while (1) {
        if (**p == ' ' || **p == '\t' || **p == '\r' || **p == '\n' || **p == ';') {
            (*p)++;
        } else if (**p == '/' && *(*p + 1) == '/') {
            while (**p != '\0' && **p != '\r' && **p != '\n') (*p)++;
        } else {
            break;
        }
    }
}

int ParsePrimary(const char** p) {
    nodeCount++;
    SkipWhitespace(p);
    if (**p == '\0') return 0;

    // Parentheses
    if (**p == '(') {
        (*p)++;
        int val = ParseExpr(p);
        SkipWhitespace(p);
        if (**p == ')') (*p)++;
        return val;
    }

    // Hex literal: 0x or 0X
    if (**p == '0' && (*(*p + 1) == 'x' || *(*p + 1) == 'X')) {
        *p += 2;
        int val = 0;
        while (IsHexDigit(**p)) {
            val = (val << 4) | HexVal(**p);
            (*p)++;
        }
        return val;
    }

    // Binary literal: 0b or 0B
    if (**p == '0' && (*(*p + 1) == 'b' || *(*p + 1) == 'B')) {
        *p += 2;
        int val = 0;
        while (**p == '0' || **p == '1') {
            val = (val << 1) | (**p - '0');
            (*p)++;
        }
        return val;
    }

    // Decimal numbers
    if (**p >= '0' && **p <= '9') {
        int val = 0;
        while (**p >= '0' && **p <= '9') {
            val = val * 10 + (**p - '0');
            (*p)++;
        }
        return val;
    }

    // Identifiers or built-in function calls
    if (IsAlpha(**p)) {
        char ident[MAX_VAR_NAME];
        int len = 0;
        while (IsAlnum(**p) && len < MAX_VAR_NAME - 1) {
            ident[len++] = **p;
            (*p)++;
        }
        ident[len] = '\0';

        SkipWhitespace(p);
        // Check if function call
        if (**p == '(') {
            (*p)++; // eat '('
            int arg1 = 0, arg2 = 0, arg3 = 0;
            SkipWhitespace(p);
            if (**p != ')') arg1 = ParseExpr(p);
            SkipWhitespace(p);
            if (**p == ',') {
                (*p)++;
                arg2 = ParseExpr(p);
                SkipWhitespace(p);
            }
            if (**p == ',') {
                (*p)++;
                arg3 = ParseExpr(p);
                SkipWhitespace(p);
            }
            if (**p == ')') (*p)++;

            if (StrEqualCase(ident, "abs")) return MathAbs(arg1);
            if (StrEqualCase(ident, "min")) return MathMin(arg1, arg2);
            if (StrEqualCase(ident, "max")) return MathMax(arg1, arg2);
            if (StrEqualCase(ident, "clamp")) return MathClamp(arg1, arg2, arg3);
            if (StrEqualCase(ident, "sqrt")) return MathSqrt(arg1);
            if (StrEqualCase(ident, "gcd")) return MathGcd(arg1, arg2);
            if (StrEqualCase(ident, "sign")) return MathSign(arg1);
            if (StrEqualCase(ident, "rand")) return MathRand(arg1, arg2);
            if (StrEqualCase(ident, "pow")) return MathPow(arg1, arg2);
            return arg1;
        }

        // Variable lookup
        int idx = FindVariable(ident);
        if (idx != -1) return g_vars[idx].val;
        return 0;
    }

    if (**p != '\0' && **p != ';' && **p != '\n' && **p != '\r') (*p)++;
    return 0;
}

int ParseUnary(const char** p) {
    nodeCount++;
    SkipWhitespace(p);
    if (**p == '+') {
        (*p)++;
        return ParseUnary(p);
    }
    if (**p == '-') {
        (*p)++;
        return -ParseUnary(p);
    }
    if (**p == '~') {
        (*p)++;
        return ~ParseUnary(p);
    }
    if (**p == '!') {
        (*p)++;
        return !ParseUnary(p);
    }
    return ParsePrimary(p);
}

int ParsePower(const char** p) {
    nodeCount++;
    int val = ParseUnary(p);
    SkipWhitespace(p);
    if (**p == '*' && *(*p + 1) == '*') {
        *p += 2;
        int exp = ParsePower(p);
        val = MathPow(val, exp);
    }
    return val;
}

int ParseMultiplicative(const char** p) {
    nodeCount++;
    int val = ParsePower(p);
    while (1) {
        SkipWhitespace(p);
        char c = **p;
        if ((c == '*' && *(*p + 1) != '*') || c == '/' || c == '%') {
            (*p)++;
            int nextVal = ParsePower(p);
            if (c == '*') val *= nextVal;
            else if (c == '/') {
                if (nextVal != 0) {
                    if (val == (int)0x80000000 && nextVal == -1) val = (int)0x80000000;
                    else val /= nextVal;
                } else val = 0;
            }
            else if (c == '%') {
                if (nextVal != 0) {
                    if (val == (int)0x80000000 && nextVal == -1) val = 0;
                    else val %= nextVal;
                } else val = 0;
            }
        } else {
            break;
        }
    }
    return val;
}

int ParseAdditive(const char** p) {
    nodeCount++;
    int val = ParseMultiplicative(p);
    while (1) {
        SkipWhitespace(p);
        char c = **p;
        if (c == '+' || c == '-') {
            (*p)++;
            int nextVal = ParseMultiplicative(p);
            if (c == '+') val += nextVal;
            else val -= nextVal;
        } else {
            break;
        }
    }
    return val;
}

int ParseShift(const char** p) {
    nodeCount++;
    int val = ParseAdditive(p);
    while (1) {
        SkipWhitespace(p);
        if (**p == '<' && *(*p + 1) == '<') {
            *p += 2;
            int nextVal = ParseAdditive(p);
            val = val << (nextVal & 31);
        } else if (**p == '>' && *(*p + 1) == '>') {
            *p += 2;
            int nextVal = ParseAdditive(p);
            val = val >> (nextVal & 31);
        } else {
            break;
        }
    }
    return val;
}

int ParseRelational(const char** p) {
    nodeCount++;
    int val = ParseShift(p);
    while (1) {
        SkipWhitespace(p);
        if (**p == '<' && *(*p + 1) == '=') {
            *p += 2;
            int nextVal = ParseShift(p);
            val = (val <= nextVal) ? 1 : 0;
        } else if (**p == '>' && *(*p + 1) == '=') {
            *p += 2;
            int nextVal = ParseShift(p);
            val = (val >= nextVal) ? 1 : 0;
        } else if (**p == '<' && *(*p + 1) != '<') {
            (*p)++;
            int nextVal = ParseShift(p);
            val = (val < nextVal) ? 1 : 0;
        } else if (**p == '>' && *(*p + 1) != '>') {
            (*p)++;
            int nextVal = ParseShift(p);
            val = (val > nextVal) ? 1 : 0;
        } else {
            break;
        }
    }
    return val;
}

int ParseEquality(const char** p) {
    nodeCount++;
    int val = ParseRelational(p);
    while (1) {
        SkipWhitespace(p);
        if (**p == '=' && *(*p + 1) == '=') {
            *p += 2;
            int nextVal = ParseRelational(p);
            val = (val == nextVal) ? 1 : 0;
        } else if (**p == '!' && *(*p + 1) == '=') {
            *p += 2;
            int nextVal = ParseRelational(p);
            val = (val != nextVal) ? 1 : 0;
        } else {
            break;
        }
    }
    return val;
}

int ParseBitwiseAnd(const char** p) {
    nodeCount++;
    int val = ParseEquality(p);
    while (1) {
        SkipWhitespace(p);
        if (**p == '&' && *(*p + 1) != '&') {
            (*p)++;
            int nextVal = ParseEquality(p);
            val &= nextVal;
        } else {
            break;
        }
    }
    return val;
}

int ParseBitwiseXor(const char** p) {
    nodeCount++;
    int val = ParseBitwiseAnd(p);
    while (1) {
        SkipWhitespace(p);
        if (**p == '^') {
            (*p)++;
            int nextVal = ParseBitwiseAnd(p);
            val ^= nextVal;
        } else {
            break;
        }
    }
    return val;
}

int ParseBitwiseOr(const char** p) {
    nodeCount++;
    int val = ParseBitwiseXor(p);
    while (1) {
        SkipWhitespace(p);
        if (**p == '|' && *(*p + 1) != '|') {
            (*p)++;
            int nextVal = ParseBitwiseXor(p);
            val |= nextVal;
        } else {
            break;
        }
    }
    return val;
}

int ParseLogicalAnd(const char** p) {
    nodeCount++;
    int val = ParseBitwiseOr(p);
    while (1) {
        SkipWhitespace(p);
        if (**p == '&' && *(*p + 1) == '&') {
            *p += 2;
            int nextVal = ParseBitwiseOr(p);
            val = (val && nextVal) ? 1 : 0;
        } else {
            break;
        }
    }
    return val;
}

int ParseLogicalOr(const char** p) {
    nodeCount++;
    int val = ParseLogicalAnd(p);
    while (1) {
        SkipWhitespace(p);
        if (**p == '|' && *(*p + 1) == '|') {
            *p += 2;
            int nextVal = ParseLogicalAnd(p);
            val = (val || nextVal) ? 1 : 0;
        } else {
            break;
        }
    }
    return val;
}

int ParseExpr(const char** p) {
    return ParseLogicalOr(p);
}

void FormatBin16(unsigned int v, char* out) {
    for (int i = 15; i >= 0; i--) {
        *out++ = ((v >> i) & 1) ? '1' : '0';
        if (i == 12 || i == 8 || i == 4) *out++ = ' ';
    }
    *out = '\0';
}

void UpdateMemoryUI() {
    memStr[0] = '\0';
    char header[128];
    wsprintfA(header, "AST Nodes: %d  |  Vars: %d\r\nLast Exec: %d us\r\n\r\n", nodeCount, g_varCount, lastExecMicros);
    lstrcatA(memStr, header);
    lstrcatA(memStr, "VARIABLE        DECIMAL    HEX       BIN (16-bit)\r\n");
    lstrcatA(memStr, "---------------------------------------------------------\r\n");

    int active = 0;
    for (int i = 0; i < g_varCount; i++) {
        if (g_vars[i].assigned) {
            char binBuf[24];
            FormatBin16((unsigned int)g_vars[i].val, binBuf);
            char line[128];
            wsprintfA(line, "%-14s  %-9d  0x%04X    %s\r\n", g_vars[i].name, g_vars[i].val, (unsigned int)(g_vars[i].val & 0xFFFF), binBuf);
            if (StrLen(memStr) + StrLen(line) < sizeof(memStr) - 100) {
                lstrcatA(memStr, line);
            }
            active++;
        }
    }
    if (active == 0) {
        lstrcatA(memStr, "(no assigned variables)");
    }
    SetWindowTextA(hMemory, memStr);
    SetWindowTextA(hOutput, outStr);
}

// Single step execution
int StepScript(int* lastVal) {
    if (!debugPtr) {
        GetWindowTextA(hInput, debugInput, sizeof(debugInput));
        debugPtr = debugInput;
        ClearVariables();
        outStr[0] = '\0';
        nodeCount = 0;
    }

    SkipWhitespace(&debugPtr);
    if (!*debugPtr) {
        char resStr[32];
        IntToStr(*lastVal, resStr);
        int l = StrLen(outStr);
        if (l < sizeof(outStr) - 64) wsprintfA(outStr + l, "\r\nReturn: %s", resStr);
        UpdateMemoryUI();
        debugPtr = NULL;
        return 0;
    }

    const char* q = debugPtr;

    // Print statement: print "string" or print expr, expr
    if ((q[0] == 'p' || q[0] == 'P') &&
        (q[1] == 'r' || q[1] == 'R') &&
        (q[2] == 'i' || q[2] == 'I') &&
        (q[3] == 'n' || q[3] == 'N') &&
        (q[4] == 't' || q[4] == 'T')) {
        char after = q[5];
        if (after == ' ' || after == '\t' || after == '(' || after == '\"' || after == '\0' || after == '\r' || after == '\n' || after == ';') {
            debugPtr += 5;
            SkipWhitespace(&debugPtr);
            int outL = StrLen(outStr);
            if (outL < sizeof(outStr) - 64) wsprintfA(outStr + outL, "Print: ");

            int first = 1;
            while (*debugPtr != '\0' && *debugPtr != '\r' && *debugPtr != '\n' && *debugPtr != ';') {
                SkipWhitespace(&debugPtr);
                if (*debugPtr == ',') {
                    debugPtr++;
                    SkipWhitespace(&debugPtr);
                }
                if (*debugPtr == '\"') {
                    // String literal
                    debugPtr++;
                    char strPart[256];
                    int sLen = 0;
                    while (*debugPtr != '\0' && *debugPtr != '\"' && *debugPtr != '\r' && *debugPtr != '\n' && sLen < sizeof(strPart) - 1) {
                        strPart[sLen++] = *debugPtr++;
                    }
                    if (*debugPtr == '\"') debugPtr++;
                    strPart[sLen] = '\0';
                    outL = StrLen(outStr);
                    if (outL + sLen + 2 < sizeof(outStr)) {
                        if (!first) lstrcatA(outStr, " ");
                        lstrcatA(outStr, strPart);
                    }
                    first = 0;
                } else if (*debugPtr != '\0' && *debugPtr != '\r' && *debugPtr != '\n' && *debugPtr != ';') {
                    int val = ParseExpr(&debugPtr);
                    *lastVal = val;
                    char vstr[32];
                    IntToStr(val, vstr);
                    outL = StrLen(outStr);
                    if (outL + 36 < sizeof(outStr)) {
                        if (!first) lstrcatA(outStr, " ");
                        lstrcatA(outStr, vstr);
                    }
                    first = 0;
                }
            }
            outL = StrLen(outStr);
            if (outL < sizeof(outStr) - 4) lstrcatA(outStr, "\r\n");
            UpdateMemoryUI();
            return 1;
        }
    }

    // Assignment or standalone expression
    int isAssign = 0;
    char varName[MAX_VAR_NAME] = {0};

    if (IsAlpha(*q)) {
        int vlen = 0;
        const char* scan = q;
        while (IsAlnum(*scan) && vlen < MAX_VAR_NAME - 1) {
            varName[vlen++] = *scan++;
        }
        varName[vlen] = '\0';
        while (*scan == ' ' || *scan == '\t') scan++;
        if (*scan == '=' && *(scan + 1) != '=') {
            isAssign = 1;
            debugPtr = scan + 1;
        }
    }

    int val = ParseExpr(&debugPtr);
    if (isAssign) {
        int idx = GetOrCreateVariable(varName);
        if (idx != -1) {
            g_vars[idx].val = val;
            g_vars[idx].assigned = 1;
        }
        char vstr[32];
        IntToStr(val, vstr);
        int l = StrLen(outStr);
        if (l + StrLen(varName) + 40 < sizeof(outStr)) {
            wsprintfA(outStr + l, "%s = %s\r\n", varName, vstr);
        }
    }
    *lastVal = val;
    UpdateMemoryUI();
    return 1;
}

void RunScript() {
    LARGE_INTEGER freq, tStart, tEnd;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&tStart);

    debugPtr = NULL;
    int lastVal = 0;
    while (StepScript(&lastVal)) {}

    QueryPerformanceCounter(&tEnd);
    if (freq.QuadPart > 0) {
        lastExecMicros = (int)((tEnd.QuadPart - tStart.QuadPart) * 1000000 / freq.QuadPart);
    }
    UpdateMemoryUI();
}

void BenchmarkScript() {
    LARGE_INTEGER freq, tStart, tEnd;
    QueryPerformanceFrequency(&freq);

    GetWindowTextA(hInput, debugInput, sizeof(debugInput));
    int iterations = 500;
    int totalNodes = 0;
    int lastVal = 0;

    QueryPerformanceCounter(&tStart);
    for (int iter = 0; iter < iterations; iter++) {
        debugPtr = debugInput;
        ClearVariables();
        nodeCount = 0;
        outStr[0] = '\0';
        while (StepScript(&lastVal)) {}
        totalNodes += nodeCount;
    }
    QueryPerformanceCounter(&tEnd);

    int totalMicros = (int)((tEnd.QuadPart - tStart.QuadPart) * 1000000 / freq.QuadPart);
    if (totalMicros <= 0) totalMicros = 1;
    int avgMicros = totalMicros / iterations;
    int runsPerSec = (int)(((long long)iterations * 1000000) / totalMicros);
    int nodesPerSec = (int)(((long long)totalNodes * 1000000) / totalMicros);

    lastExecMicros = avgMicros;
    outStr[0] = '\0';
    wsprintfA(outStr,
        "=========================================\r\n"
        "      KSCRIPT PERFORMANCE BENCHMARK      \r\n"
        "=========================================\r\n"
        "Completed:        %d runs\r\n"
        "Total Time:       %d us (%d ms)\r\n"
        "Avg Time / Run:   %d us\r\n"
        "Throughput:       %d runs/sec\r\n"
        "Total AST Nodes:  %d\r\n"
        "Node Throughput:  %d nodes/sec\r\n"
        "Active Variables: %d\r\n"
        "Last Result:      %d\r\n"
        "=========================================\r\n",
        iterations, totalMicros, totalMicros / 1000, avgMicros, runsPerSec, totalNodes, nodesPerSec, g_varCount, lastVal);

    UpdateMemoryUI();
}

void SimpleRegexReplace() {
    char f[256], r[256];
    char inBuf[65536], outBuf[65536];
    GetWindowTextA(hRegexFind, f, sizeof(f));
    GetWindowTextA(hRegexRep, r, sizeof(r));
    GetWindowTextA(hInput, inBuf, sizeof(inBuf));
    if (f[0] == '\0') return;

    outBuf[0] = '\0';
    char* src = inBuf;
    char* dst = outBuf;
    int fLen = StrLen(f);
    int rLen = StrLen(r);

    while (*src) {
        int match = 1;
        for (int i = 0; i < fLen; i++) {
            if (src[i] == '\0' || src[i] != f[i]) { match = 0; break; }
        }
        if (match) {
            for (int i = 0; i < rLen; i++) {
                if (dst - outBuf < sizeof(outBuf) - 1) {
                    *dst++ = r[i];
                }
            }
            src += fLen;
        } else {
            if (dst - outBuf < sizeof(outBuf) - 1) {
                *dst++ = *src;
            }
            src++;
        }
    }
    *dst = '\0';
    SetWindowTextA(hInput, outBuf);
}

void SaveJsonDump(HWND hwnd) {
    char szFile[MAX_PATH] = "script_dump.json";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = "json";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (GetSaveFileNameA(&ofn)) {
        HANDLE hFile = CreateFileA(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            char* json = (char*)VirtualAlloc(NULL, 65536, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (json) {
                wsprintfA(json, "{\r\n  \"app\": \"KScript\",\r\n  \"astNodes\": %d,\r\n  \"executionMicroseconds\": %d,\r\n  \"variables\": {\r\n", nodeCount, lastExecMicros);
                int first = 1;
                for (int i = 0; i < g_varCount; i++) {
                    if (g_vars[i].assigned) {
                        char entry[256];
                        wsprintfA(entry, "%s    \"%s\": { \"dec\": %d, \"hex\": \"0x%X\" }", first ? "" : ",\r\n", g_vars[i].name, g_vars[i].val, (unsigned int)g_vars[i].val);
                        lstrcatA(json, entry);
                        first = 0;
                    }
                }
                lstrcatA(json, "\r\n  }\r\n}\r\n");
                DWORD dwWritten;
                WriteFile(hFile, json, StrLen(json), &dwWritten, NULL);
                VirtualFree(json, 0, MEM_RELEASE);
            }
            CloseHandle(hFile);
        }
    }
}

void ShowHelpDialog(HWND hwnd) {
    const char* helpText =
        "====================================================\n"
        "           KSCRIPT ADVANCED USER GUIDE              \n"
        "====================================================\n\n"
        "[SYNTAX & VARIABLES]\n"
        " - Variables: Multi-character identifiers (e.g. counter = 42, mask = 0xFF)\n"
        " - Literals: Decimal (100), Hex (0x1F, 0xFF00), Binary (0b1010)\n"
        " - Comments: Lines starting with // are ignored\n\n"
        "[OPERATORS & PRECEDENCE]\n"
        " - Arithmetic: +, -, *, /, %, ** (Power)\n"
        " - Bitwise: & (AND), | (OR), ^ (XOR), ~ (NOT), << (SHL), >> (SHR)\n"
        " - Comparison: ==, !=, <, <=, >, >=\n"
        " - Logical: &&, ||, !\n\n"
        "[BUILT-IN FUNCTIONS]\n"
        " abs(x), min(a,b), max(a,b), clamp(x,lo,hi), sqrt(x), gcd(a,b), sign(x), rand(min,max), pow(b,e)\n\n"
        "[OUTPUT & CONSOLE]\n"
        " print \"Result is:\", val\n"
        " print \"Bitmask:\", mask, \"Flags:\", flags\n\n"
        "[SHORTCUTS]\n"
        " - [F5] / [Ctrl+Enter] : Run Entire Script\n"
        " - [F6]                : Run Benchmark (500 iterations)\n"
        " - [F10] / [Alt+S]     : Step Line-by-Line\n"
        " - [Ctrl+S]            : Save Script File (.ksc / .json)\n"
        " - [Ctrl+O]            : Load Script File (.ksc)\n"
        " - [Ctrl+K]            : Clear Console & Reset State\n"
        " - [Alt+M] / [Ctrl+M]  : Toggle Macro Recording\n"
        " - [Alt+P] / [Ctrl+P]  : Play Recorded Macro\n"
        " - [F1] / [Alt+H]      : Open this Help Guide";

    MessageBoxA(hwnd, helpText, "KScript - Help & Reference", MB_OK | MB_ICONINFORMATION);
}

void UpdateWindowTitle() {
    if (isRecording) {
        SetWindowTextA(hMainWnd, "KScript [● RECORDING MACRO] - Press Alt+M to Stop");
    } else {
        SetWindowTextA(hMainWnd, "KScript - [F5] Run | [F6] Bench | [F10] Step | [F1] Help");
    }
}

LRESULT CALLBACK InputEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CHAR && isRecording) {
        if (macroLen < sizeof(macroBuf) - 1) {
            macroBuf[macroLen++] = (char)wp;
        }
    }
    if (msg == WM_KEYDOWN) {
        if (wp == VK_F5 || (GetKeyState(VK_CONTROL) < 0 && wp == VK_RETURN)) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 1, 0);
            return 0;
        }
        if (wp == VK_F6) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 10, 0);
            return 0;
        }
        if (wp == VK_F10 || (GetKeyState(VK_MENU) < 0 && (wp == 'S' || wp == 's'))) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 6, 0);
            return 0;
        }
        if (GetKeyState(VK_CONTROL) < 0 && (wp == 'S' || wp == 's')) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 3, 0);
            return 0;
        }
        if (GetKeyState(VK_CONTROL) < 0 && (wp == 'O' || wp == 'o')) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 2, 0);
            return 0;
        }
        if (GetKeyState(VK_CONTROL) < 0 && (wp == 'K' || wp == 'k')) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 9, 0);
            return 0;
        }
        if ((GetKeyState(VK_MENU) < 0 || GetKeyState(VK_CONTROL) < 0) && (wp == 'M' || wp == 'm')) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 4, 0);
            return 0;
        }
        if ((GetKeyState(VK_MENU) < 0 || GetKeyState(VK_CONTROL) < 0) && (wp == 'P' || wp == 'p')) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 5, 0);
            return 0;
        }
        if (wp == VK_F1) {
            SendMessage(GetParent(hwnd), WM_COMMAND, 8, 0);
            return 0;
        }
    }
    if (msg == WM_SYSKEYDOWN && (wp == 'H' || wp == 'h')) {
        SendMessage(GetParent(hwnd), WM_COMMAND, 8, 0);
        return 0;
    }
    return CallWindowProc(oldEditProc, hwnd, msg, wp, lp);
}

LRESULT CALLBACK FindEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN && wp == VK_RETURN) {
        SendMessage(GetParent(hwnd), WM_COMMAND, 7, 0);
        return 0;
    }
    if (msg == WM_KEYDOWN && wp == VK_F1) {
        SendMessage(GetParent(hwnd), WM_COMMAND, 8, 0);
        return 0;
    }
    return CallWindowProc(oldFindProc, hwnd, msg, wp, lp);
}

LRESULT CALLBACK RepEditProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN && wp == VK_RETURN) {
        SendMessage(GetParent(hwnd), WM_COMMAND, 7, 0);
        return 0;
    }
    if (msg == WM_KEYDOWN && wp == VK_F1) {
        SendMessage(GetParent(hwnd), WM_COMMAND, 8, 0);
        return 0;
    }
    return CallWindowProc(oldRepProc, hwnd, msg, wp, lp);
}

HBRUSH hbrBg;
HFONT hFont;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_GETMINMAXINFO: {
            MINMAXINFO* mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = S(760);
            mmi->ptMinTrackSize.y = S(400);
            return 0;
        }
        case WM_CREATE: {
            hMainWnd = hwnd;
            hbrBg = CreateSolidBrush(RGB(20, 24, 38));
            int fontHeight = -MulDiv(11, dpi, 72);
            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 5 /* CLEARTYPE_QUALITY */, DEFAULT_PITCH, "Consolas");

            // Toolbar Buttons
            hBtnRec   = CreateWindowEx(0, "BUTTON", "Rec [Alt+M]", WS_CHILD | WS_VISIBLE, S(6), S(8), S(86), S(26), hwnd, (HMENU)4, NULL, NULL);
            hBtnPlay  = CreateWindowEx(0, "BUTTON", "Play [Alt+P]", WS_CHILD | WS_VISIBLE, S(96), S(8), S(86), S(26), hwnd, (HMENU)5, NULL, NULL);
            hBtnStep  = CreateWindowEx(0, "BUTTON", "Step [F10]", WS_CHILD | WS_VISIBLE, S(186), S(8), S(78), S(26), hwnd, (HMENU)6, NULL, NULL);
            hBtnRun   = CreateWindowEx(0, "BUTTON", "Run [F5]", WS_CHILD | WS_VISIBLE, S(268), S(8), S(74), S(26), hwnd, (HMENU)1, NULL, NULL);
            hBtnBench = CreateWindowEx(0, "BUTTON", "Bench [F6]", WS_CHILD | WS_VISIBLE, S(346), S(8), S(80), S(26), hwnd, (HMENU)10, NULL, NULL);
            hBtnLoad  = CreateWindowEx(0, "BUTTON", "Load [Ctrl+O]", WS_CHILD | WS_VISIBLE, S(430), S(8), S(90), S(26), hwnd, (HMENU)2, NULL, NULL);
            hBtnSave  = CreateWindowEx(0, "BUTTON", "Save [Ctrl+S]", WS_CHILD | WS_VISIBLE, S(524), S(8), S(90), S(26), hwnd, (HMENU)3, NULL, NULL);
            hBtnClear = CreateWindowEx(0, "BUTTON", "Clear [Ctrl+K]", WS_CHILD | WS_VISIBLE, S(618), S(8), S(90), S(26), hwnd, (HMENU)9, NULL, NULL);
            hBtnHelp  = CreateWindowEx(0, "BUTTON", "Help [F1]", WS_CHILD | WS_VISIBLE, S(712), S(8), S(74), S(26), hwnd, (HMENU)8, NULL, NULL);

            hRegexFind = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, S(790), S(9), S(60), S(24), hwnd, NULL, NULL, NULL);
            hRegexRep  = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, S(854), S(9), S(60), S(24), hwnd, NULL, NULL, NULL);
            hBtnRep    = CreateWindowEx(0, "BUTTON", "Rep", WS_CHILD | WS_VISIBLE, S(918), S(8), S(50), S(26), hwnd, (HMENU)7, NULL, NULL);

            HWND hwnds[] = {hBtnRec, hBtnPlay, hBtnStep, hBtnRun, hBtnBench, hBtnLoad, hBtnSave, hBtnClear, hRegexFind, hRegexRep, hBtnRep, hBtnHelp};
            for (int i = 0; i < 12; i++) SendMessage(hwnds[i], WM_SETFONT, (WPARAM)hFont, TRUE);

            oldFindProc = (WNDPROC)SetWindowLongPtr(hRegexFind, GWLP_WNDPROC, (LONG_PTR)FindEditProc);
            oldRepProc  = (WNDPROC)SetWindowLongPtr(hRegexRep, GWLP_WNDPROC, (LONG_PTR)RepEditProc);

            DragAcceptFiles(hwnd, TRUE);

            // Panels
            const char* defaultCode =
                "// Welcome to KScript Advanced!\r\n"
                "// Press F5 to Run, F6 to Benchmark, F10 to Step, F1 for Help\r\n"
                "mask = 0xFF00\r\n"
                "flag = 0b10101010\r\n"
                "combined = (mask >> 8) | flag\r\n"
                "val = sqrt(144) + min(25, 40)\r\n"
                "print \"Combined mask:\", combined\r\n"
                "print \"Math result:\", val\r\n"
                "is_active = (combined > 100) && (val == 37)\r\n"
                "print \"Condition check:\", is_active";

            hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", defaultCode,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
                S(8), S(42), S(310), S(H) - S(90), hwnd, NULL, NULL, NULL);
            SendMessage(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            oldEditProc = (WNDPROC)SetWindowLongPtr(hInput, GWLP_WNDPROC, (LONG_PTR)InputEditProc);

            hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                S(326), S(42), S(290), S(H) - S(90), hwnd, NULL, NULL, NULL);
            SendMessage(hOutput, WM_SETFONT, (WPARAM)hFont, TRUE);

            hMemory = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Memory Inspector",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                S(624), S(42), S(340), S(H) - S(90), hwnd, NULL, NULL, NULL);
            SendMessage(hMemory, WM_SETFONT, (WPARAM)hFont, TRUE);

            RunScript();
            break;
        }
        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            char szFile[MAX_PATH] = {0};
            if (DragQueryFileA(hDrop, 0, szFile, sizeof(szFile))) {
                HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD dwSize = GetFileSize(hFile, NULL);
                    if (dwSize > 0 && dwSize < sizeof(debugInput)) {
                        char* buf = (char*)VirtualAlloc(NULL, dwSize + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                        if (buf) {
                            DWORD dwRead;
                            if (ReadFile(hFile, buf, dwSize, &dwRead, NULL)) {
                                buf[dwRead] = '\0';
                                SetWindowTextA(hInput, buf);
                                RunScript();
                            }
                            VirtualFree(buf, 0, MEM_RELEASE);
                        }
                    }
                    CloseHandle(hFile);
                }
            }
            DragFinish(hDrop);
            return 0;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if (wmId == 1) {
                RunScript();
            }
            else if (wmId == 10) {
                BenchmarkScript();
            }
            else if (wmId == 6) {
                int lv = 0;
                StepScript(&lv);
            }
            else if (wmId == 4) {
                isRecording = !isRecording;
                SetWindowTextA(hBtnRec, isRecording ? "Stop [Alt+M]" : "Rec [Alt+M]");
                if (isRecording) macroLen = 0;
                UpdateWindowTitle();
            }
            else if (wmId == 5) {
                if (isRecording) {
                    isRecording = 0;
                    SetWindowTextA(hBtnRec, "Rec [Alt+M]");
                    UpdateWindowTitle();
                }
                SetFocus(hInput);
                for (int i = 0; i < macroLen; i++) {
                    SendMessage(hInput, WM_CHAR, (WPARAM)(unsigned char)macroBuf[i], 0);
                }
            }
            else if (wmId == 7) {
                SimpleRegexReplace();
            }
            else if (wmId == 8) {
                ShowHelpDialog(hwnd);
            }
            else if (wmId == 9) {
                debugPtr = NULL;
                ClearVariables();
                outStr[0] = '\0';
                nodeCount = 0;
                lastExecMicros = 0;
                UpdateMemoryUI();
            }
            else if (wmId == 2) {
                char szFile[MAX_PATH] = {0};
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "KScript Files (*.ksc)\0*.ksc\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                if (GetOpenFileNameA(&ofn)) {
                    HANDLE hFile = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile != INVALID_HANDLE_VALUE) {
                        DWORD dwSize = GetFileSize(hFile, NULL);
                        if (dwSize > 0 && dwSize < sizeof(debugInput)) {
                            char* buf = (char*)VirtualAlloc(NULL, dwSize + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                            if (buf) {
                                DWORD dwRead;
                                if (ReadFile(hFile, buf, dwSize, &dwRead, NULL)) {
                                    buf[dwRead] = '\0';
                                    SetWindowTextA(hInput, buf);
                                    RunScript();
                                }
                                VirtualFree(buf, 0, MEM_RELEASE);
                            }
                        }
                        CloseHandle(hFile);
                    }
                }
            }
            else if (wmId == 3) {
                char szFile[MAX_PATH] = "script.ksc";
                OPENFILENAMEA ofn = {0};
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "KScript Files (*.ksc)\0*.ksc\0JSON State Dump (*.json)\0*.json\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrDefExt = "ksc";
                ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
                if (GetSaveFileNameA(&ofn)) {
                    int flen = StrLen(szFile);
                    if (flen > 5 && StrEqualCase(szFile + flen - 5, ".json")) {
                        SaveJsonDump(hwnd);
                    } else {
                        HANDLE hFile = CreateFileA(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hFile != INVALID_HANDLE_VALUE) {
                            int len = GetWindowTextLengthA(hInput);
                            if (len > 0) {
                                char* buf = (char*)VirtualAlloc(NULL, len + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                                if (buf) {
                                    GetWindowTextA(hInput, buf, len + 1);
                                    DWORD dwWritten;
                                    WriteFile(hFile, buf, len, &dwWritten, NULL);
                                    VirtualFree(buf, 0, MEM_RELEASE);
                                }
                            }
                            CloseHandle(hFile);
                        }
                    }
                }
            }
            break;
        }
        case WM_SYSKEYDOWN: {
            if (wParam == 'H' || wParam == 'h') {
                ShowHelpDialog(hwnd);
                return 0;
            }
            if (wParam == 'M' || wParam == 'm') {
                SendMessage(hwnd, WM_COMMAND, 4, 0);
                return 0;
            }
            if (wParam == 'P' || wParam == 'p') {
                SendMessage(hwnd, WM_COMMAND, 5, 0);
                return 0;
            }
            if (wParam == 'S' || wParam == 's') {
                SendMessage(hwnd, WM_COMMAND, 6, 0);
                return 0;
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        case WM_KEYDOWN: {
            if (wParam == VK_F1) {
                ShowHelpDialog(hwnd);
                return 0;
            }
            if (wParam == VK_F5) {
                RunScript();
                return 0;
            }
            if (wParam == VK_F6) {
                BenchmarkScript();
                return 0;
            }
            if (wParam == VK_F10) {
                int lv = 0;
                StepScript(&lv);
                return 0;
            }
            if (GetKeyState(VK_CONTROL) < 0 && (wParam == 'K' || wParam == 'k')) {
                SendMessage(hwnd, WM_COMMAND, 9, 0);
                return 0;
            }
            break;
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hCtl = (HWND)lParam;
            SetBkMode(hdc, OPAQUE);
            SetBkColor(hdc, RGB(20, 24, 38));
            if (hCtl == hMemory) SetTextColor(hdc, RGB(56, 189, 248));
            else if (hCtl == hOutput) SetTextColor(hdc, RGB(196, 181, 253));
            else SetTextColor(hdc, RGB(241, 245, 249));
            return (LRESULT)hbrBg;
        }
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, hbrBg);
            return 1;
        }
        case WM_SIZE: {
            int nw = LOWORD(lParam);
            int nh = HIWORD(lParam);
            int topY = S(40);
            int bottomH = nh - topY - S(10);
            if (bottomH < S(50)) bottomH = S(50);
            int panelGap = S(8);
            int panelW = (nw - S(16) - panelGap * 2) / 3;
            if (panelW < S(100)) panelW = S(100);

            MoveWindow(hInput, S(8), topY, panelW, bottomH, TRUE);
            MoveWindow(hOutput, S(8) + panelW + panelGap, topY, panelW, bottomH, TRUE);
            MoveWindow(hMemory, S(8) + (panelW + panelGap) * 2, topY, nw - S(8) - (S(8) + (panelW + panelGap) * 2), bottomH, TRUE);

            int repW = S(50);
            int repInputW = S(58);
            int rx3 = nw - S(8) - repW;
            int rx2 = rx3 - S(6) - repInputW;
            int rx1 = rx2 - S(6) - repInputW;
            if (rx1 > S(790)) {
                MoveWindow(hRegexFind, rx1, S(9), repInputW, S(24), TRUE);
                MoveWindow(hRegexRep, rx2, S(9), repInputW, S(24), TRUE);
                MoveWindow(hBtnRep, rx3, S(8), repW, S(26), TRUE);
            }
            break;
        }
        case WM_DESTROY:
            DeleteObject(hFont);
            DeleteObject(hbrBg);
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

#pragma function(memcpy)
void* __cdecl memcpy(void* dest, const void* src, size_t count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (count--) *d++ = *s++;
    return dest;
}

void MainEntry() {
    SetProcessDPIAware();
    HDC hdc = GetDC(NULL);
    dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KScriptApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hbrBackground = NULL;
    RegisterClass(&wc);

    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    RECT rect = {0, 0, S(W), S(H)};
    AdjustWindowRect(&rect, style, FALSE);
    HWND hwnd = CreateWindowEx(0, "KScriptApp", "KScript - [F5] Run | [F6] Bench | [F10] Step | [F1] Help", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
