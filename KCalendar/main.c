#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#define ID_MONTHCAL 1000
#define ID_BTN_TODAY 1001

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(icex);
            icex.dwICC = ICC_DATE_CLASSES;
            InitCommonControlsEx(&icex);

            HWND hMonthCal = CreateWindowEx(0, MONTHCAL_CLASS, "",
                WS_BORDER | WS_CHILD | WS_VISIBLE | MCS_DAYSTATE,
                0, 0, 0, 0, hwnd, (HMENU)ID_MONTHCAL, GetModuleHandle(NULL), NULL);

            HWND hBtnToday = CreateWindowEx(0, "BUTTON", "Go to Today",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TODAY, GetModuleHandle(NULL), NULL);

            // Get required size for the month calendar
            RECT rc;
            SendMessage(hMonthCal, MCM_GETMINREQRECT, 0, (LPARAM)&rc);
            
            int btnHeight = 30;
            // Adjust the window size to fit the calendar exactly + button
            RECT winRc = {0, 0, rc.right, rc.bottom + btnHeight};
            AdjustWindowRect(&winRc, GetWindowLong(hwnd, GWL_STYLE), FALSE);
            
            SetWindowPos(hwnd, NULL, 0, 0, winRc.right - winRc.left, winRc.bottom - winRc.top, SWP_NOMOVE | SWP_NOZORDER);
            SetWindowPos(hMonthCal, NULL, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOZORDER);
            SetWindowPos(hBtnToday, NULL, 0, rc.bottom, rc.right, btnHeight, SWP_NOMOVE | SWP_NOZORDER);

            // Apply dark theme colors
            SendMessage(hMonthCal, MCM_SETCOLOR, MCSC_BACKGROUND, RGB(15, 23, 42));
            SendMessage(hMonthCal, MCM_SETCOLOR, MCSC_TEXT, RGB(255, 255, 255));
            SendMessage(hMonthCal, MCM_SETCOLOR, MCSC_TITLEBK, RGB(9, 9, 11));
            SendMessage(hMonthCal, MCM_SETCOLOR, MCSC_TITLETEXT, RGB(245, 158, 11));
            SendMessage(hMonthCal, MCM_SETCOLOR, MCSC_MONTHBK, RGB(15, 23, 42));
            SendMessage(hMonthCal, MCM_SETCOLOR, MCSC_TRAILINGTEXT, RGB(100, 116, 139));
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BTN_TODAY) {
                SYSTEMTIME st;
                GetLocalTime(&st);
                HWND hMonthCal = GetDlgItem(hwnd, ID_MONTHCAL);
                SendMessage(hMonthCal, MCM_SETCURSEL, 0, (LPARAM)&st);
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KCalendarApp";
    wc.hbrBackground = CreateSolidBrush(RGB(15, 23, 42));
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "KCalendarApp", "KCalendar", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 250, 200, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
