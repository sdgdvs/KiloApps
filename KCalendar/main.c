#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#define ID_MONTHCAL    1000
#define ID_BTN_TODAY   1001
#define ID_LIST_EVENTS 1002
#define ID_EDIT_EVENT  1003
#define ID_BTN_ADD     1004
#define ID_BTN_DEL     1005

#define MAX_EVENTS 1000

typedef struct {
    int year, month, day;
    char text[128];
} Event;

static Event events[MAX_EVENTS];
static int event_count = 0;
static SYSTEMTIME selected_date;

static HWND hMonthCal, hBtnToday, hListEvents, hEditEvent, hBtnAdd, hBtnDel;
static HBRUSH hBgBrush = NULL;
static HBRUSH hEditBrush = NULL;
static WNDPROC oldEditProc = NULL;

// --- CRT-free Helper Functions ---
void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

static int my_strlen(const char* s) {
    if (!s) return 0;
    int len = 0;
    while (s[len]) len++;
    return len;
}

static void my_strcpy(char* dest, const char* src) {
    if (!dest || !src) return;
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

static void LoadEvents() {
    HANDLE hFile = CreateFileA("kcal_events.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == 0 || fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return;
    }

    char* buf = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
    if (!buf) {
        CloseHandle(hFile);
        return;
    }

    DWORD bytesRead = 0;
    if (ReadFile(hFile, buf, fileSize, &bytesRead, NULL)) {
        buf[bytesRead] = '\0';
        event_count = 0;
        char* ptr = buf;
        while (*ptr && event_count < MAX_EVENTS) {
            int y = 0, m = 0, d = 0;
            while (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n') ptr++;
            if (!*ptr) break;

            while (*ptr >= '0' && *ptr <= '9') { y = y * 10 + (*ptr - '0'); ptr++; }
            while (*ptr == ' ') ptr++;
            while (*ptr >= '0' && *ptr <= '9') { m = m * 10 + (*ptr - '0'); ptr++; }
            while (*ptr == ' ') ptr++;
            while (*ptr >= '0' && *ptr <= '9') { d = d * 10 + (*ptr - '0'); ptr++; }
            while (*ptr == ' ') ptr++;

            int textIdx = 0;
            while (*ptr && *ptr != '\r' && *ptr != '\n' && textIdx < 127) {
                events[event_count].text[textIdx++] = *ptr++;
            }
            events[event_count].text[textIdx] = '\0';

            while (*ptr && *ptr != '\r' && *ptr != '\n') ptr++;

            if (y > 0 && m >= 1 && m <= 12 && d >= 1 && d <= 31 && textIdx > 0) {
                events[event_count].year = y;
                events[event_count].month = m;
                events[event_count].day = d;
                event_count++;
            }
        }
    }

    HeapFree(GetProcessHeap(), 0, buf);
    CloseHandle(hFile);
}

static void SaveEvents() {
    HANDLE hFile = CreateFileA("kcal_events.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    for (int i = 0; i < event_count; i++) {
        char line[256];
        int len = wsprintfA(line, "%d %d %d %s\r\n", events[i].year, events[i].month, events[i].day, events[i].text);
        DWORD written = 0;
        WriteFile(hFile, line, len, &written, NULL);
    }
    CloseHandle(hFile);
}

static void RefreshList() {
    SendMessage(hListEvents, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < event_count; i++) {
        if (events[i].year == selected_date.wYear && 
            events[i].month == selected_date.wMonth && 
            events[i].day == selected_date.wDay) {
            SendMessage(hListEvents, LB_ADDSTRING, 0, (LPARAM)events[i].text);
        }
    }
}

static int GetEventIndexFromListIndex(int selIndex) {
    if (selIndex < 0) return -1;
    int current = 0;
    for (int i = 0; i < event_count; i++) {
        if (events[i].year == selected_date.wYear && 
            events[i].month == selected_date.wMonth && 
            events[i].day == selected_date.wDay) {
            if (current == selIndex) return i;
            current++;
        }
    }
    return -1;
}

static void DeleteSelectedEvent() {
    int sel = (int)SendMessage(hListEvents, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) return;
    int idx = GetEventIndexFromListIndex(sel);
    if (idx >= 0 && idx < event_count) {
        for (int i = idx; i < event_count - 1; i++) {
            events[i] = events[i + 1];
        }
        event_count--;
        SaveEvents();
        RefreshList();
        InvalidateRect(hMonthCal, NULL, TRUE);
    }
}

static void AddEventFromInput() {
    char buf[128];
    GetWindowText(hEditEvent, buf, 128);
    if (my_strlen(buf) > 0 && event_count < MAX_EVENTS) {
        events[event_count].year = selected_date.wYear;
        events[event_count].month = selected_date.wMonth;
        events[event_count].day = selected_date.wDay;
        my_strcpy(events[event_count].text, buf);
        event_count++;
        SaveEvents();
        SetWindowText(hEditEvent, "");
        RefreshList();
        InvalidateRect(hMonthCal, NULL, TRUE);
    }
}

static LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_RETURN) {
        AddEventFromInput();
        return 0;
    }
    return CallWindowProc(oldEditProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex;
            icex.dwSize = sizeof(icex);
            icex.dwICC = ICC_DATE_CLASSES;
            InitCommonControlsEx(&icex);

            hBgBrush = CreateSolidBrush(RGB(15, 23, 42));
            hEditBrush = CreateSolidBrush(RGB(15, 23, 42));

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
            int listW = 220;
            int listH = rc.bottom - 40;

            hListEvents = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                listX, 10, listW, listH, hwnd, (HMENU)ID_LIST_EVENTS, GetModuleHandle(NULL), NULL);

            hEditEvent = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                listX, 10 + listH + 5, listW, 25, hwnd, (HMENU)ID_EDIT_EVENT, GetModuleHandle(NULL), NULL);

            int btnW = (listW - 5) / 2;
            hBtnAdd = CreateWindowEx(0, "BUTTON", "Add",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                listX, 10 + rc.bottom + 10, btnW, 30, hwnd, (HMENU)ID_BTN_ADD, GetModuleHandle(NULL), NULL);

            hBtnDel = CreateWindowEx(0, "BUTTON", "Delete",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                listX + btnW + 5, 10 + rc.bottom + 10, btnW, 30, hwnd, (HMENU)ID_BTN_DEL, GetModuleHandle(NULL), NULL);

            oldEditProc = (WNDPROC)SetWindowLongPtr(hEditEvent, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

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
                                if (events[e].day >= 1 && events[e].day <= 31) {
                                    state |= (1 << (events[e].day - 1));
                                }
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
                AddEventFromInput();
            } else if (LOWORD(wParam) == ID_BTN_DEL) {
                DeleteSelectedEvent();
            } else if (LOWORD(wParam) == ID_LIST_EVENTS && HIWORD(wParam) == LBN_DBLCLK) {
                DeleteSelectedEvent();
            }
            break;
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(15, 23, 42));
            SetTextColor(hdc, RGB(255, 255, 255));
            return (LRESULT)hEditBrush;
        }
        case WM_DESTROY:
            if (hBgBrush) { DeleteObject(hBgBrush); hBgBrush = NULL; }
            if (hEditBrush) { DeleteObject(hEditBrush); hEditBrush = NULL; }
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
        CW_USEDEFAULT, CW_USEDEFAULT, 460, 320, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}

