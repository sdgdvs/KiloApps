#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

HWND hDisplay, hBtnGen;
const char* charPool = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+";

void GeneratePassword() {
    char pwd[17] = {0};
    for(int i = 0; i < 16; i++) {
        pwd[i] = charPool[GetTickCount() % (72 + i) % 74];
        Sleep(1); // Mix up tick count a bit for crude randomness
    }
    SetWindowTextA(hDisplay, pwd);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Click Generate...", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, 20, 20, 240, 30, hwnd, NULL, NULL, NULL);
            hBtnGen = CreateWindowA("BUTTON", "Generate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 90, 70, 100, 30, hwnd, (HMENU)1001, NULL, NULL);
            
            HFONT hFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
            SendMessageA(hDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);
            HFONT hBtnFont = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Tahoma");
            SendMessageA(hBtnGen, WM_SETFONT, (WPARAM)hBtnFont, TRUE);
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) {
                GeneratePassword();
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(20, 20, 20));
            SetTextColor(hdc, RGB(231, 76, 60)); // Red-ish for KPass
            return (LRESULT)CreateSolidBrush(RGB(20, 20, 20));
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
    wc.lpszClassName = "KPassClass";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, "KPassClass", "KPass", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 160, NULL, NULL, wc.hInstance, NULL);
    
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ExitProcess(0);
}
