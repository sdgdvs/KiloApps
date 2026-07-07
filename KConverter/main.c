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

HWND hInput, hOutput, hFrom, hTo;
char buffer[256];

double GetFactor(int index) {
    if (index == 0) return 1.0;
    if (index == 1) return 1000.0;
    if (index == 2) return 1609.34;
    if (index == 3) return 0.3048;
    if (index == 4) return 0.0254;
    return 1.0;
}

void DoConvert() {
    GetWindowTextA(hInput, buffer, 255);
    double val = m_atof(buffer);
    int fromIdx = SendMessage(hFrom, CB_GETCURSEL, 0, 0);
    int toIdx = SendMessage(hTo, CB_GETCURSEL, 0, 0);
    if (fromIdx == CB_ERR) fromIdx = 0;
    if (toIdx == CB_ERR) toIdx = 0;
    
    double baseVal = val * GetFactor(fromIdx);
    double result = baseVal / GetFactor(toIdx);
    
    m_sprintf(buffer, "%.6g", result);
    SetWindowTextA(hOutput, buffer);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HMODULE hMsvcrt = LoadLibraryA("msvcrt.dll");
            m_sprintf = (sprintf_t)GetProcAddress(hMsvcrt, "sprintf");
            m_atof = (atof_t)GetProcAddress(hMsvcrt, "atof");

            CreateWindowA("STATIC", "Value:", WS_CHILD | WS_VISIBLE, 10, 10, 50, 20, hwnd, NULL, NULL, NULL);
            hInput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 70, 10, 100, 20, hwnd, NULL, NULL, NULL);

            CreateWindowA("STATIC", "From:", WS_CHILD | WS_VISIBLE, 10, 40, 50, 20, hwnd, NULL, NULL, NULL);
            hFrom = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 70, 40, 100, 100, hwnd, NULL, NULL, NULL);

            CreateWindowA("STATIC", "To:", WS_CHILD | WS_VISIBLE, 10, 70, 50, 20, hwnd, NULL, NULL, NULL);
            hTo = CreateWindowA("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 70, 70, 100, 100, hwnd, NULL, NULL, NULL);

            CreateWindowA("BUTTON", "Convert", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 180, 40, 80, 50, hwnd, (HMENU)1001, NULL, NULL);

            CreateWindowA("STATIC", "Result:", WS_CHILD | WS_VISIBLE, 10, 100, 50, 20, hwnd, NULL, NULL, NULL);
            hOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_READONLY | ES_AUTOHSCROLL, 70, 100, 190, 20, hwnd, NULL, NULL, NULL);

            const char* units[] = {"Meters", "Kilometers", "Miles", "Feet", "Inches"};
            for (int i = 0; i < 5; i++) {
                SendMessageA(hFrom, CB_ADDSTRING, 0, (LPARAM)units[i]);
                SendMessageA(hTo, CB_ADDSTRING, 0, (LPARAM)units[i]);
            }
            SendMessageA(hFrom, CB_SETCURSEL, 0, 0);
            SendMessageA(hTo, CB_SETCURSEL, 0, 0);
            
            HFONT hFont = CreateFontA(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Tahoma");
            SendMessageA(hInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hFrom, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hTo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) {
                DoConvert();
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
    wc.lpszClassName = "KConvClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KConvClass", "KConverter", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 290, 170, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
