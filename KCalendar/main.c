#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

#define ID_MONTHCAL 1000
#define ID_BTN_TODAY 1001
#define ID_LIST_EVENTS 1002
#define ID_EDIT_EVENT 1003
#define ID_BTN_ADD 1004

#define MAX_EVENTS 1000
typedef struct {
    int year, month, day;
    char text[128];
} Event;

Event events[MAX_EVENTS];
int event_count = 0;
SYSTEMTIME selected_date;

HWND hMonthCal, hBtnToday, hListEvents, hEditEvent, hBtnAdd;

void LoadEvents() {
    FILE *f = fopen("kcal_events.dat", "r");
    if (!f) return;
    event_count = 0;
    while (fscanf(f, "%d %d %d %[^\n]\n", &events[event_count].year, &events[event_count].month, &events[event_count].day, events[event_count].text) == 4) {
        event_count++;
        if (event_count >= MAX_EVENTS) break;
    }
    fclose(f);
}

void SaveEvents() {
    FILE *f = fopen("kcal_events.dat", "w");
    if (!f) return;
    for (int i = 0; i < event_count; i++) {
        fprintf(f, "%d %d %d %s\n", events[i].year, events[i].month, events[i].day, events[i].text);
    }
    fclose(f);
}

void RefreshList() {
    SendMessage(hListEvents, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < event_count; i++) {
        if (events[i].year == selected_date.wYear && 
            events[i].month == selected_date.wMonth && 
            events[i].day == selected_date.wDay) {
            SendMessage(hListEvents, LB_ADDSTRING, 0, (LPARAM)events[i].text);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(icex);
            icex.dwICC = ICC_DATE_CLASSES;
            InitCommonControlsEx(&icex);

            LoadEvents();
            GetLocalTime(&selected_date);

            hMonthCal = CreateWindowEx(0, MONTHCAL_CLASS, "",
                WS_BORDER | WS_CHILD | WS_VISIBLE | MCS_DAYSTATE | MCS_NOTODAY,
                10, 10, 0, 0, hwnd, (HMENU)ID_MONTHCAL, GetModuleHandle(NULL), NULL);

            RECT rc;
            SendMessage(hMonthCal, MCM_GETMINREQRECT, 0, (LPARAM)&rc);
            SetWindowPos(hMonthCal, NULL, 10, 10, rc.right, rc.bottom, SWP_NOZORDER);

            hBtnToday = CreateWindowEx(0, "BUTTON", "Go to Today",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, 10 + rc.bottom + 10, rc.right, 30, hwnd, (HMENU)ID_BTN_TODAY, GetModuleHandle(NULL), NULL);

            int listX = 10 + rc.right + 10;
            int listW = 200;
            hListEvents = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                listX, 10, listW, rc.bottom - 40, hwnd, (HMENU)ID_LIST_EVENTS, GetModuleHandle(NULL), NULL);

            hEditEvent = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                listX, 10 + rc.bottom - 30, listW, 25, hwnd, (HMENU)ID_EDIT_EVENT, GetModuleHandle(NULL), NULL);

            hBtnAdd = CreateWindowEx(0, "BUTTON", "Add Event",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                listX, 10 + rc.bottom + 10, listW, 30, hwnd, (HMENU)ID_BTN_ADD, GetModuleHandle(NULL), NULL);

            RECT winRc = {0, 0, listX + listW + 10, 10 + rc.bottom + 10 + 30 + 10};
            AdjustWindowRect(&winRc, GetWindowLong(hwnd, GWL_STYLE), FALSE);
            SetWindowPos(hwnd, NULL, 0, 0, winRc.right - winRc.left, winRc.bottom - winRc.top, SWP_NOMOVE | SWP_NOZORDER);

            SendMessage(hMonthCal, MCM_SETCOLOR, MCSC_BACKGROUND, RGB(15, 23, 42));
            SendMessage(hMonthCal, MCM_SETCOLOR, MCSC_TEXT, RGB(255, 255, 255));
            SendMessage(hMonthCal, MCM_SETCOLOR, MCSC_TITLEBK, RGB(9, 9, 11));
            SendMessage(hMonthCal, MCM_SETCOLOR, MCSC_TITLETEXT, RGB(245, 158, 11));
            SendMessage(hMonthCal, MCM_SETCOLOR, MCSC_MONTHBK, RGB(15, 23, 42));
            SendMessage(hMonthCal, MCM_SETCOLOR, MCSC_TRAILINGTEXT, RGB(100, 116, 139));
            
            RefreshList();
            break;
        }
        case WM_NOTIFY: {
            LPNMHDR nmhdr = (LPNMHDR)lParam;
            if (nmhdr->idFrom == ID_MONTHCAL) {
                if (nmhdr->code == MCN_SELCHANGE || nmhdr->code == MCN_SELECT) {
                    LPNMSELCHANGE lpNMSelChange = (LPNMSELCHANGE)lParam;
                    selected_date = lpNMSelChange->stSelStart;
                    RefreshList();
                } else if (nmhdr->code == MCN_GETDAYSTATE) {
                    LPNMDAYSTATE lpNMDayState = (LPNMDAYSTATE)lParam;
                    int cDayState = lpNMDayState->cDayState;
                    SYSTEMTIME st = lpNMDayState->stStart;
                    for (int i = 0; i < cDayState; i++) {
                        MONTHDAYSTATE state = 0;
                        for (int e = 0; e < event_count; e++) {
                            if (events[e].year == st.wYear && events[e].month == st.wMonth) {
                                state |= (1 << (events[e].day - 1));
                            }
                        }
                        lpNMDayState->prgDayState[i] = state;
                        st.wMonth++;
                        if (st.wMonth > 12) { st.wMonth = 1; st.wYear++; }
                    }
                }
            }
            break;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BTN_TODAY) {
                GetLocalTime(&selected_date);
                SendMessage(hMonthCal, MCM_SETCURSEL, 0, (LPARAM)&selected_date);
                RefreshList();
            } else if (LOWORD(wParam) == ID_BTN_ADD) {
                char buf[128];
                GetWindowText(hEditEvent, buf, 128);
                if (strlen(buf) > 0 && event_count < MAX_EVENTS) {
                    events[event_count].year = selected_date.wYear;
                    events[event_count].month = selected_date.wMonth;
                    events[event_count].day = selected_date.wDay;
                    strcpy(events[event_count].text, buf);
                    event_count++;
                    SaveEvents();
                    SetWindowText(hEditEvent, "");
                    RefreshList();
                    InvalidateRect(hMonthCal, NULL, TRUE);
                }
            }
            break;
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(15, 23, 42));
            SetTextColor(hdc, RGB(255, 255, 255));
            static HBRUSH hBrush = NULL;
            if (!hBrush) hBrush = CreateSolidBrush(RGB(15, 23, 42));
            return (LRESULT)hBrush;
        }
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
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
