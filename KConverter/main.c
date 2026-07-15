int _fltused = 1;
#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}

#pragma function(memset)

typedef int (__cdecl *sprintf_t)(char*, const char*, ...);
typedef double (__cdecl *atof_t)(const char*);
sprintf_t m_sprintf;
atof_t m_atof;

HWND hCategory, hInput, hOutput, hFrom, hTo;
HFONT hFont;
char buffer[256];

const char* catNames[] = {"Length", "Weight", "Temperature", "Data"};
const char* lenUnits[] = {"Meters", "Kilometers", "Miles", "Feet", "Inches"};
const char* wtUnits[] = {"Kilograms", "Grams", "Pounds", "Ounces"};
const char* tempUnits[] = {"Celsius", "Fahrenheit", "Kelvin"};
const char* dataUnits[] = {"Bytes", "KB", "MB", "GB", "TB"};

double GetFactor(int cat, int index) {
    if (cat == 0) {
        if (index == 0) return 1.0;
        if (index == 1) return 1000.0;
        if (index == 2) return 1609.34;
        if (index == 3) return 0.3048;
        if (index == 4) return 0.0254;
    } else if (cat == 1) {
        if (index == 0) return 1.0;
        if (index == 1) return 0.001;
        if (index == 2) return 0.453592;
        if (index == 3) return 0.0283495;
    } else if (cat == 3) {
        if (index == 0) return 1.0;
        if (index == 1) return 1024.0;
        if (index == 2) return 1048576.0;
        if (index == 3) return 1073741824.0;
        if (index == 4) return 1099511627776.0;
    }
    return 1.0;
}

void PopulateUnits(int catIdx) {
    SendMessage(hFrom, CB_RESETCONTENT, 0, 0);
    SendMessage(hTo, CB_RESETCONTENT, 0, 0);
    const char** units = NULL;
    int count = 0;
    if (catIdx == 0) { units = lenUnits; count = 5; }
    else if (catIdx == 1) { units = wtUnits; count = 4; }
    else if (catIdx == 2) { units = tempUnits; count = 3; }
    else if (catIdx == 3) { units = dataUnits; count = 5; }
    
    for (int i=0; i<count; i++) {
        SendMessageA(hFrom, CB_ADDSTRING, 0, (LPARAM)units[i]);
        SendMessageA(hTo, CB_ADDSTRING, 0, (LPARAM)units[i]);
    }
    SendMessage(hFrom, CB_SETCURSEL, 0, 0);
    SendMessage(hTo, CB_SETCURSEL, count > 1 ? 1 : 0);
}

void DoConvert() {
    GetWindowTextA(hInput, buffer, 255);
    double val = m_atof(buffer);
    int catIdx = SendMessage(hCategory, CB_GETCURSEL, 0, 0);
    int fromIdx = SendMessage(hFrom, CB_GETCURSEL, 0, 0);
    int toIdx = SendMessage(hTo, CB_GETCURSEL, 0, 0);
    if (catIdx == CB_ERR) catIdx = 0;
    if (fromIdx == CB_ERR) fromIdx = 0;
    if (toIdx == CB_ERR) toIdx = 0;
    
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
    
    m_sprintf(buffer, "%.6g", result);
    SetWindowTextA(hOutput, buffer);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HMODULE hMsvcrt = LoadLibraryA("msvcrt.dll");
            m_sprintf = (sprintf_t)GetProcAddress(hMsvcrt, "sprintf");
            m_atof = (atof_t)GetProcAddress(hMsvcrt, "atof");

            CreateWindowA("STATIC", "Type:", WS_CHILD | WS_VISIBLE, 10, 10, 50, 20, hwnd, NULL, NULL, NULL);
            hCategory = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 70, 10, 100, 100, hwnd, (HMENU)1002, NULL, NULL);

            CreateWindowA("STATIC", "Value:", WS_CHILD | WS_VISIBLE, 10, 40, 50, 20, hwnd, NULL, NULL, NULL);
            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 70, 40, 100, 20, hwnd, NULL, NULL, NULL);

            CreateWindowA("STATIC", "From:", WS_CHILD | WS_VISIBLE, 10, 70, 50, 20, hwnd, NULL, NULL, NULL);
            hFrom = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 70, 70, 100, 100, hwnd, NULL, NULL, NULL);

            CreateWindowA("STATIC", "To:", WS_CHILD | WS_VISIBLE, 10, 100, 50, 20, hwnd, NULL, NULL, NULL);
            hTo = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 70, 100, 100, 100, hwnd, NULL, NULL, NULL);

            CreateWindowA("BUTTON", "Convert", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 180, 70, 80, 50, hwnd, (HMENU)1001, NULL, NULL);

            CreateWindowA("STATIC", "Result:", WS_CHILD | WS_VISIBLE, 10, 130, 50, 20, hwnd, NULL, NULL, NULL);
            hOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_READONLY | ES_AUTOHSCROLL, 70, 130, 190, 20, hwnd, NULL, NULL, NULL);

            for (int i = 0; i < 4; i++) {
                SendMessageA(hCategory, CB_ADDSTRING, 0, (LPARAM)catNames[i]);
            }
            SendMessageA(hCategory, CB_SETCURSEL, 0, 0);
            PopulateUnits(0);
            
            hFont = CreateFontA(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Tahoma");
            SendMessageA(hCategory, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hFrom, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hTo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) {
                DoConvert();
            } else if (LOWORD(wParam) == 1002 && HIWORD(wParam) == CBN_SELCHANGE) {
                int catIdx = SendMessage(hCategory, CB_GETCURSEL, 0, 0);
                PopulateUnits(catIdx);
                DoConvert();
            }
            break;
        }
        case WM_DESTROY:
            if (hFont) DeleteObject(hFont);
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KConvClass", "KConverter", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 290, 200, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
