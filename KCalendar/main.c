#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#define ID_MONTHCAL        1000
#define ID_BTN_TODAY       1001
#define ID_LIST_EVENTS     1002
#define ID_EDIT_EVENT      1003
#define ID_BTN_ADD         1004
#define ID_BTN_DEL         1005
#define ID_COMBO_CATEGORY  1006
#define ID_COMBO_RECUR     1007
#define ID_COMBO_FILTER    1008
#define ID_EDIT_SEARCH     1009
#define ID_BTN_EXPORT_ICS  1010
#define ID_BTN_EXPORT_CSV  1011
#define ID_BTN_HELP        1012

#define MAX_EVENTS 1000

typedef struct {
    int year, month, day;
    char category[32]; // Work, Personal, Health, Important, Other
    int recurring;     // 0=None, 1=Daily, 2=Weekly, 3=Monthly, 4=Yearly
    char text[128];
} Event;

static Event events[MAX_EVENTS];
static int event_count = 0;
static SYSTEMTIME selected_date;

static HWND hMonthCal, hBtnToday, hListEvents, hEditEvent, hBtnAdd, hBtnDel;
static HWND hComboCategory, hComboRecur, hComboFilter, hEditSearch;
static HWND hBtnExportIcs, hBtnExportCsv;
static HBRUSH hBgBrush = NULL;
static HBRUSH hEditBrush = NULL;
static WNDPROC oldEditProc = NULL;
static HFONT hFont = NULL;

const char* CATEGORIES[] = { "Work", "Personal", "Health", "Important", "Other" };
const char* RECURRENCES[] = { "None", "Daily", "Weekly", "Monthly", "Yearly" };

static BOOL CALLBACK SetFontCallback(HWND hwnd, LPARAM lParam) {
    SendMessage(hwnd, WM_SETFONT, (WPARAM)lParam, TRUE);
    return TRUE;
}

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

static int my_tolower(int c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

static int my_stristr(const char* haystack, const char* needle) {
    if (!haystack || !needle || !*needle) return 1;
    for (; *haystack; haystack++) {
        const char *h = haystack, *n = needle;
        while (*h && *n && my_tolower((unsigned char)*h) == my_tolower((unsigned char)*n)) {
            h++;
            n++;
        }
        if (!*n) return 1;
    }
    return 0;
}

static int DayOfWeek(int y, int m, int d) {
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) y -= 1;
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

static int IsEventOnDate(const Event* e, int y, int m, int d) {
    if (e->year == y && e->month == m && e->day == d) return 1;
    if (e->recurring == 0) return 0;
    
    if (y < e->year) return 0;
    if (y == e->year && m < e->month) return 0;
    if (y == e->year && m == e->month && d < e->day) return 0;
    
    if (e->recurring == 1) return 1; // Daily
    if (e->recurring == 2) { // Weekly
        return DayOfWeek(e->year, e->month, e->day) == DayOfWeek(y, m, d);
    }
    if (e->recurring == 3) { // Monthly
        return e->day == d;
    }
    if (e->recurring == 4) { // Yearly
        return e->month == m && e->day == d;
    }
    return 0;
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
            int y = 0, m = 0, d = 0, recur = 0;
            char cat[32] = "Work";
            while (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n') ptr++;
            if (!*ptr) break;

            while (*ptr >= '0' && *ptr <= '9') { y = y * 10 + (*ptr - '0'); ptr++; }
            while (*ptr == ' ') ptr++;
            while (*ptr >= '0' && *ptr <= '9') { m = m * 10 + (*ptr - '0'); ptr++; }
            while (*ptr == ' ') ptr++;
            while (*ptr >= '0' && *ptr <= '9') { d = d * 10 + (*ptr - '0'); ptr++; }
            while (*ptr == ' ') ptr++;

            // Optional category
            if (*ptr >= 'A' && *ptr <= 'Z') {
                int cidx = 0;
                while (*ptr && *ptr != ' ' && cidx < 31) { cat[cidx++] = *ptr++; }
                cat[cidx] = '\0';
                while (*ptr == ' ') ptr++;
            }

            // Optional recurrence
            if (*ptr >= '0' && *ptr <= '9') {
                recur = *ptr - '0';
                ptr++;
                while (*ptr == ' ') ptr++;
            }

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
                my_strcpy(events[event_count].category, cat);
                events[event_count].recurring = (recur >= 0 && recur <= 4) ? recur : 0;
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
        int len = wsprintfA(line, "%d %d %d %s %d %s\r\n", 
            events[i].year, events[i].month, events[i].day, 
            events[i].category[0] ? events[i].category : "Work", 
            events[i].recurring, events[i].text);
        DWORD written = 0;
        WriteFile(hFile, line, len, &written, NULL);
    }
    CloseHandle(hFile);
}

static void ExportToIcs() {
    HANDLE hFile = CreateFileA("kcalendar_export.ics", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    const char* header = "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nPRODID:-//KCalendar Native//EN\r\n";
    DWORD written = 0;
    WriteFile(hFile, header, my_strlen(header), &written, NULL);

    for (int i = 0; i < event_count; i++) {
        char buf[512];
        int len = wsprintfA(buf, "BEGIN:VEVENT\r\nUID:native_%d@kcalendar\r\nDTSTART:%04d%02d%02dT090000\r\nSUMMARY:%s\r\nCATEGORIES:%s\r\nEND:VEVENT\r\n",
            i, events[i].year, events[i].month, events[i].day, events[i].text, events[i].category);
        WriteFile(hFile, buf, len, &written, NULL);
    }

    const char* footer = "END:VCALENDAR\r\n";
    WriteFile(hFile, footer, my_strlen(footer), &written, NULL);
    CloseHandle(hFile);
    MessageBoxA(NULL, "Exported events to kcalendar_export.ics", "Export Complete", MB_OK | MB_ICONINFORMATION);
}

static void ExportToCsv() {
    HANDLE hFile = CreateFileA("kcalendar_export.csv", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    const char* header = "Year,Month,Day,Category,Recurrence,Title\r\n";
    DWORD written = 0;
    WriteFile(hFile, header, my_strlen(header), &written, NULL);

    for (int i = 0; i < event_count; i++) {
        char buf[512];
        int len = wsprintfA(buf, "%d,%d,%d,%s,%s,\"%s\"\r\n",
            events[i].year, events[i].month, events[i].day, 
            events[i].category, RECURRENCES[events[i].recurring], events[i].text);
        WriteFile(hFile, buf, len, &written, NULL);
    }

    CloseHandle(hFile);
    MessageBoxA(NULL, "Exported events to kcalendar_export.csv", "Export Complete", MB_OK | MB_ICONINFORMATION);
}

static void RefreshList() {
    SendMessage(hListEvents, LB_RESETCONTENT, 0, 0);

    char searchBuf[64] = "";
    GetWindowTextA(hEditSearch, searchBuf, 64);

    int filterSel = (int)SendMessage(hComboFilter, CB_GETCURSEL, 0, 0);

    for (int i = 0; i < event_count; i++) {
        if (IsEventOnDate(&events[i], selected_date.wYear, selected_date.wMonth, selected_date.wDay)) {
            // Category filter
            if (filterSel > 0 && filterSel <= 5) {
                if (lstrcmpiA(events[i].category, CATEGORIES[filterSel - 1]) != 0) {
                    continue;
                }
            }

            // Search filter
            if (searchBuf[0] && !my_stristr(events[i].text, searchBuf)) {
                continue;
            }

            char displayStr[256];
            wsprintfA(displayStr, "[%s] %s %s", 
                events[i].category, 
                events[i].text, 
                events[i].recurring > 0 ? "(🔄)" : "");
            SendMessage(hListEvents, LB_ADDSTRING, 0, (LPARAM)displayStr);
        }
    }
}

static int GetEventIndexFromListIndex(int selIndex) {
    if (selIndex < 0) return -1;
    char searchBuf[64] = "";
    GetWindowTextA(hEditSearch, searchBuf, 64);
    int filterSel = (int)SendMessage(hComboFilter, CB_GETCURSEL, 0, 0);

    int current = 0;
    for (int i = 0; i < event_count; i++) {
        if (IsEventOnDate(&events[i], selected_date.wYear, selected_date.wMonth, selected_date.wDay)) {
            if (filterSel > 0 && filterSel <= 5) {
                if (lstrcmpiA(events[i].category, CATEGORIES[filterSel - 1]) != 0) continue;
            }
            if (searchBuf[0] && !my_stristr(events[i].text, searchBuf)) continue;

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
    GetWindowTextA(hEditEvent, buf, 128);
    if (my_strlen(buf) > 0 && event_count < MAX_EVENTS) {
        events[event_count].year = selected_date.wYear;
        events[event_count].month = selected_date.wMonth;
        events[event_count].day = selected_date.wDay;
        my_strcpy(events[event_count].text, buf);

        int catSel = (int)SendMessage(hComboCategory, CB_GETCURSEL, 0, 0);
        if (catSel >= 0 && catSel < 5) my_strcpy(events[event_count].category, CATEGORIES[catSel]);
        else my_strcpy(events[event_count].category, "Work");

        int recurSel = (int)SendMessage(hComboRecur, CB_GETCURSEL, 0, 0);
        events[event_count].recurring = (recurSel >= 0 && recurSel <= 4) ? recurSel : 0;

        event_count++;
        SaveEvents();
        SetWindowTextA(hEditEvent, "");
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

            HDC hdc = GetDC(NULL);
            int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(NULL, hdc);
            #define SCALE(x) MulDiv((x), dpiY, 96)

            int fontHeight = -MulDiv(12, dpiY, 72);
            hFont = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            hBgBrush = CreateSolidBrush(RGB(15, 23, 42));
            hEditBrush = CreateSolidBrush(RGB(15, 23, 42));

            LoadEvents();
            GetLocalTime(&selected_date);

            hMonthCal = CreateWindowEx(0, MONTHCAL_CLASS, "",
                WS_BORDER | WS_CHILD | WS_VISIBLE | MCS_DAYSTATE | MCS_NOTODAY,
                10, 10, 0, 0, hwnd, (HMENU)ID_MONTHCAL, GetModuleHandle(NULL), NULL);

            RECT rc;
            SendMessage(hMonthCal, MCM_GETMINREQRECT, 0, (LPARAM)&rc);
            SetWindowPos(hMonthCal, NULL, SCALE(10), SCALE(10), rc.right, rc.bottom, SWP_NOZORDER);

            int winWidth = SCALE(800);
            int winHeight = SCALE(600);
            int pad = SCALE(10);
            int btnH = SCALE(28);
            int editH = SCALE(24);
            int spacing = SCALE(6);
            
            int listX = pad + rc.right + SCALE(15);
            int rightW = winWidth - listX - SCALE(15);
            int listH = winHeight - SCALE(40) - SCALE(130);

            // Left column buttons
            int btnY = pad + rc.bottom + SCALE(15);
            hBtnToday = CreateWindowEx(0, "BUTTON", "Go to Today",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                pad, btnY, rc.right, btnH, hwnd, (HMENU)ID_BTN_TODAY, GetModuleHandle(NULL), NULL);

            int exportW = (rc.right - SCALE(5)) / 2;
            hBtnExportIcs = CreateWindowEx(0, "BUTTON", "Export .ics",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                pad, btnY + btnH + spacing, exportW, btnH, hwnd, (HMENU)ID_BTN_EXPORT_ICS, GetModuleHandle(NULL), NULL);

            hBtnExportCsv = CreateWindowEx(0, "BUTTON", "Export CSV",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                pad + exportW + SCALE(5), btnY + btnH + spacing, exportW, btnH, hwnd, (HMENU)ID_BTN_EXPORT_CSV, GetModuleHandle(NULL), NULL);

            CreateWindowEx(0, "BUTTON", "Help (F1/H)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, pad, btnY + (btnH + spacing) * 2, rc.right, btnH, hwnd, (HMENU)ID_BTN_HELP, GetModuleHandle(NULL), NULL);

            // Search & Category Filter bar
            int searchW = rightW - SCALE(165);
            hEditSearch = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                listX, pad, searchW, editH, hwnd, (HMENU)ID_EDIT_SEARCH, GetModuleHandle(NULL), NULL);

            hComboFilter = CreateWindowEx(0, "COMBOBOX", "",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                listX + searchW + SCALE(5), pad, SCALE(160), SCALE(120), hwnd, (HMENU)ID_COMBO_FILTER, GetModuleHandle(NULL), NULL);
            
            SendMessage(hComboFilter, CB_ADDSTRING, 0, (LPARAM)"All Categories");
            for (int i = 0; i < 5; i++) SendMessage(hComboFilter, CB_ADDSTRING, 0, (LPARAM)CATEGORIES[i]);
            SendMessage(hComboFilter, CB_SETCURSEL, 0, 0);

            // Event List Box
            hListEvents = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                listX, pad + editH + spacing, rightW, listH, hwnd, (HMENU)ID_LIST_EVENTS, GetModuleHandle(NULL), NULL);

            // New event input controls
            int btmY = pad + editH + spacing + listH + SCALE(10);
            hEditEvent = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                listX, btmY, rightW, editH, hwnd, (HMENU)ID_EDIT_EVENT, GetModuleHandle(NULL), NULL);

            btmY += editH + SCALE(10);
            int comboW = (rightW - SCALE(5)) / 2;
            hComboCategory = CreateWindowEx(0, "COMBOBOX", "",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                listX, btmY, comboW, SCALE(120), hwnd, (HMENU)ID_COMBO_CATEGORY, GetModuleHandle(NULL), NULL);
            for (int i = 0; i < 5; i++) SendMessage(hComboCategory, CB_ADDSTRING, 0, (LPARAM)CATEGORIES[i]);
            SendMessage(hComboCategory, CB_SETCURSEL, 0, 0);

            hComboRecur = CreateWindowEx(0, "COMBOBOX", "",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                listX + comboW + SCALE(5), btmY, comboW, SCALE(120), hwnd, (HMENU)ID_COMBO_RECUR, GetModuleHandle(NULL), NULL);
            for (int i = 0; i < 5; i++) SendMessage(hComboRecur, CB_ADDSTRING, 0, (LPARAM)RECURRENCES[i]);
            SendMessage(hComboRecur, CB_SETCURSEL, 0, 0);

            btmY += SCALE(34);
            int btnW = (rightW - SCALE(5)) / 2;
            hBtnAdd = CreateWindowEx(0, "BUTTON", "Add Event",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                listX, btmY, btnW, btnH, hwnd, (HMENU)ID_BTN_ADD, GetModuleHandle(NULL), NULL);

            hBtnDel = CreateWindowEx(0, "BUTTON", "Delete",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                listX + btnW + SCALE(5), btmY, btnW, btnH, hwnd, (HMENU)ID_BTN_DEL, GetModuleHandle(NULL), NULL);

            oldEditProc = (WNDPROC)SetWindowLongPtr(hEditEvent, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

            EnumChildWindows(hwnd, SetFontCallback, (LPARAM)hFont);

            DWORD style = GetWindowLong(hwnd, GWL_STYLE) | WS_CLIPCHILDREN;
            SetWindowLong(hwnd, GWL_STYLE, style);
            RECT winRc = {0, 0, winWidth, winHeight};
            AdjustWindowRect(&winRc, style, FALSE);
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
                            if (IsEventOnDate(&events[e], st.wYear, st.wMonth, events[e].day)) {
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
            if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == ID_EDIT_SEARCH) {
                RefreshList();
            } else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == ID_COMBO_FILTER) {
                RefreshList();
            } else if (LOWORD(wParam) == ID_BTN_TODAY) {
                GetLocalTime(&selected_date);
                SendMessage(hMonthCal, MCM_SETCURSEL, 0, (LPARAM)&selected_date);
                RefreshList();
            } else if (LOWORD(wParam) == ID_BTN_ADD) {
                AddEventFromInput();
            } else if (LOWORD(wParam) == ID_BTN_DEL) {
                DeleteSelectedEvent();
            } else if (LOWORD(wParam) == ID_BTN_EXPORT_ICS) {
                ExportToIcs();
            } else if (LOWORD(wParam) == ID_BTN_EXPORT_CSV) {
                ExportToCsv();
            } else if (LOWORD(wParam) == ID_BTN_HELP) {
                MessageBoxA(hwnd, "KCalendar Help:\n- Use controls on left to pick date/export\n- Search or filter on top right\n- Double-click to delete\n- Add events at bottom right\n- Press F1 or H anytime outside edit boxes for this help", "Help", MB_OK | MB_ICONINFORMATION);
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
            if (hFont) { DeleteObject(hFont); hFont = NULL; }
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void MainEntry() {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *PSETPROCESSDPIAWARENESSCONTEXT)(HANDLE);
        PSETPROCESSDPIAWARENESSCONTEXT setDPIContext = (PSETPROCESSDPIAWARENESSCONTEXT)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (setDPIContext) {
            setDPIContext((HANDLE)-4); // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
        } else {
            typedef BOOL (WINAPI *PSETPROCESSDPIAWARE)(void);
            PSETPROCESSDPIAWARE setDPI = (PSETPROCESSDPIAWARE)GetProcAddress(hUser32, "SetProcessDPIAware");
            if (setDPI) setDPI();
        }
    }
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KCalendarApp";
    wc.hbrBackground = CreateSolidBrush(RGB(15, 23, 42));
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    RegisterClass(&wc);

    DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_CLIPCHILDREN;
    RECT rc = {0, 0, 800, 600};
    AdjustWindowRect(&rc, style, FALSE);

    HWND hwnd = CreateWindowEx(0, "KCalendarApp", "KCalendar (Press H for Help)", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_KEYDOWN && (msg.wParam == 'H' || msg.wParam == 'h' || msg.wParam == VK_F1)) {
            char cls[32] = {0};
            GetClassNameA(msg.hwnd, cls, 32);
            if (lstrcmpiA(cls, "EDIT") != 0) {
                MessageBoxA(hwnd, "KCalendar Help:\n- Use controls on left to pick date/export\n- Search or filter on top right\n- Double-click to delete\n- Add events at bottom right\n- Press F1 or H anytime outside edit boxes for this help", "Help", MB_OK | MB_ICONINFORMATION);
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ExitProcess(0);
}
