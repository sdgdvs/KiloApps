#include <windows.h>

void* __cdecl memset(void* p, int c, size_t sz) {
    char* pb = (char*)p;
    while (sz--) *pb++ = (char)c;
    return p;
}
#pragma function(memset)

int my_strlen(const char* s) {
    if (!s) return 0;
    int len = 0;
    while (s[len]) len++;
    return len;
}

void my_strcpy(char* dest, const char* src) {
    if (!dest || !src) return;
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

#define ID_BTN_SEARCH 1001
#define ID_BTN_TRAVEL_OFFICE 1002
#define ID_BTN_TRAVEL_MANOR 1003
#define ID_BTN_TRAVEL_DOCKS 1004
#define ID_BTN_START 1005
#define ID_LIST_SUSPECTS 1006
#define ID_LIST_CLUES 1007

HWND hTitle, hLocName, hLocDesc, hBtnSearch, hBtnTravelOffice, hBtnTravelManor, hBtnTravelDocks;
HWND hSuspectTitle, hListSuspects, hClueTitle, hListClues;
HWND hStartPanel, hBtnStart, hStartDesc;
HFONT hFont, hFontBold, hFontTitle;

int currentState = 0; // 0 = start, 1 = playing
int currentLocation = 0; // 0 = Office, 1 = Manor, 2 = Docks

typedef struct {
    char name[32];
    char desc[256];
    char clue[128];
    int searched;
    int clueFound;
} Location;

Location locations[3] = {
    {"Office", "Your dingy office. Dust motes dance in the light filtering through the blinds.", "A note slipped under the door: \"Check The Manor.\"", 0, 0},
    {"The Manor", "The sprawling estate where the victim was found. Yellow police tape blocks the study.", "A muddy footprint near the window matching a size 10 boot.", 0, 0},
    {"The Docks", "Smells like salt and secrets. The fog here is thick enough to cut with a knife.", "A torn shipping manifest listing illegal cargo.", 0, 0}
};

char* suspects[] = {
    "Mr. Black",
    "Miss Scarlet",
    "Colonel Mustard"
};

void UpdateUI() {
    if (currentState == 0) {
        ShowWindow(hStartPanel, SW_SHOW);
        ShowWindow(hStartDesc, SW_SHOW);
        ShowWindow(hBtnStart, SW_SHOW);
        
        ShowWindow(hTitle, SW_HIDE);
        ShowWindow(hLocName, SW_HIDE);
        ShowWindow(hLocDesc, SW_HIDE);
        ShowWindow(hBtnSearch, SW_HIDE);
        ShowWindow(hBtnTravelOffice, SW_HIDE);
        ShowWindow(hBtnTravelManor, SW_HIDE);
        ShowWindow(hBtnTravelDocks, SW_HIDE);
        ShowWindow(hSuspectTitle, SW_HIDE);
        ShowWindow(hListSuspects, SW_HIDE);
        ShowWindow(hClueTitle, SW_HIDE);
        ShowWindow(hListClues, SW_HIDE);
    } else {
        ShowWindow(hStartPanel, SW_HIDE);
        ShowWindow(hStartDesc, SW_HIDE);
        ShowWindow(hBtnStart, SW_HIDE);
        
        ShowWindow(hTitle, SW_SHOW);
        ShowWindow(hLocName, SW_SHOW);
        ShowWindow(hLocDesc, SW_SHOW);
        ShowWindow(hBtnSearch, SW_SHOW);
        ShowWindow(hBtnTravelOffice, SW_SHOW);
        ShowWindow(hBtnTravelManor, SW_SHOW);
        ShowWindow(hBtnTravelDocks, SW_SHOW);
        ShowWindow(hSuspectTitle, SW_SHOW);
        ShowWindow(hListSuspects, SW_SHOW);
        ShowWindow(hClueTitle, SW_SHOW);
        ShowWindow(hListClues, SW_SHOW);
        
        char locNameBuf[64];
        wsprintfA(locNameBuf, "Location: %s", locations[currentLocation].name);
        SetWindowTextA(hLocName, locNameBuf);
        SetWindowTextA(hLocDesc, locations[currentLocation].desc);
        
        if (locations[currentLocation].searched) {
            EnableWindow(hBtnSearch, FALSE);
            SetWindowTextA(hBtnSearch, "Already searched");
        } else {
            EnableWindow(hBtnSearch, TRUE);
            SetWindowTextA(hBtnSearch, "Search for Clues");
        }
        
        EnableWindow(hBtnTravelOffice, currentLocation != 0);
        EnableWindow(hBtnTravelManor, currentLocation != 1);
        EnableWindow(hBtnTravelDocks, currentLocation != 2);
    }
}

void StartGame() {
    currentState = 1;
    SendMessageA(hListSuspects, LB_RESETCONTENT, 0, 0);
    SendMessageA(hListClues, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < 3; i++) {
        SendMessageA(hListSuspects, LB_ADDSTRING, 0, (LPARAM)suspects[i]);
    }
    UpdateUI();
}

void SearchLocation(HWND hwnd) {
    if (!locations[currentLocation].searched) {
        locations[currentLocation].searched = 1;
        if (my_strlen(locations[currentLocation].clue) > 0) {
            locations[currentLocation].clueFound = 1;
            char clueEntry[256];
            wsprintfA(clueEntry, "[%s] %s", locations[currentLocation].name, locations[currentLocation].clue);
            SendMessageA(hListClues, LB_ADDSTRING, 0, (LPARAM)clueEntry);
            
            char msgBuf[256];
            wsprintfA(msgBuf, "You found a clue: %s", locations[currentLocation].clue);
            MessageBoxA(hwnd, msgBuf, "Clue Found", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBoxA(hwnd, "You didn't find anything useful here.", "Nothing Found", MB_OK | MB_ICONINFORMATION);
        }
        UpdateUI();
    }
}

void Travel(int loc) {
    currentLocation = loc;
    UpdateUI();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hFont = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Courier New");
            hFontBold = CreateFontA(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Courier New");
            hFontTitle = CreateFontA(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, "Courier New");

            // Start Screen
            hStartPanel = CreateWindowA("STATIC", "", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hStartDesc = CreateWindowA("STATIC", "Detective, a murder has occurred.", WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnStart = CreateWindowA("BUTTON", "Start Investigation", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_START, NULL, NULL);

            // Game Screen
            hTitle = CreateWindowA("STATIC", "KMystery", WS_CHILD | SS_CENTER, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hLocName = CreateWindowA("STATIC", "Location:", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hLocDesc = CreateWindowA("STATIC", "", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hBtnSearch = CreateWindowA("BUTTON", "Search for Clues", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_SEARCH, NULL, NULL);
            
            hBtnTravelOffice = CreateWindowA("BUTTON", "Travel to Office", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_OFFICE, NULL, NULL);
            hBtnTravelManor = CreateWindowA("BUTTON", "Travel to The Manor", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_MANOR, NULL, NULL);
            hBtnTravelDocks = CreateWindowA("BUTTON", "Travel to The Docks", WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ID_BTN_TRAVEL_DOCKS, NULL, NULL);
            
            hSuspectTitle = CreateWindowA("STATIC", "Notebook - Suspects", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hListSuspects = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VSCROLL | LBS_HASSTRINGS, 0, 0, 0, 0, hwnd, (HMENU)ID_LIST_SUSPECTS, NULL, NULL);
            
            hClueTitle = CreateWindowA("STATIC", "Notebook - Clues", WS_CHILD, 0, 0, 0, 0, hwnd, NULL, NULL, NULL);
            hListClues = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", NULL, WS_CHILD | WS_VSCROLL | LBS_HASSTRINGS | WS_HSCROLL, 0, 0, 0, 0, hwnd, (HMENU)ID_LIST_CLUES, NULL, NULL);

            SendMessageA(hStartDesc, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnStart, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hTitle, WM_SETFONT, (WPARAM)hFontTitle, TRUE);
            SendMessageA(hLocName, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hLocDesc, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnSearch, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hBtnTravelOffice, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnTravelManor, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hBtnTravelDocks, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hSuspectTitle, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hListSuspects, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(hClueTitle, WM_SETFONT, (WPARAM)hFontBold, TRUE);
            SendMessageA(hListClues, WM_SETFONT, (WPARAM)hFont, TRUE);

            UpdateUI();
            break;
        }

        case WM_SIZE: {
            int cx = LOWORD(lParam);
            int cy = HIWORD(lParam);
            
            if (currentState == 0) {
                MoveWindow(hStartPanel, 0, 0, cx, cy, TRUE);
                MoveWindow(hStartDesc, cx/2 - 150, cy/2 - 50, 300, 30, TRUE);
                MoveWindow(hBtnStart, cx/2 - 100, cy/2 - 10, 200, 40, TRUE);
            } else {
                int headerH = 40;
                int pad = 20;
                int leftW = (cx - pad*3) / 2;
                int rightW = leftW;
                
                MoveWindow(hTitle, 0, 0, cx, headerH, TRUE);
                
                int top = headerH + pad;
                MoveWindow(hLocName, pad, top, leftW, 24, TRUE);
                MoveWindow(hLocDesc, pad, top + 30, leftW, 60, TRUE);
                MoveWindow(hBtnSearch, pad, top + 100, 150, 30, TRUE);
                
                int travelY = top + 150;
                MoveWindow(hBtnTravelOffice, pad, travelY, 200, 30, TRUE);
                MoveWindow(hBtnTravelManor, pad, travelY + 40, 200, 30, TRUE);
                MoveWindow(hBtnTravelDocks, pad, travelY + 80, 200, 30, TRUE);
                
                int rightX = pad*2 + leftW;
                int listH = (cy - top - pad*2 - 60) / 2;
                
                MoveWindow(hSuspectTitle, rightX, top, rightW, 24, TRUE);
                MoveWindow(hListSuspects, rightX, top + 30, rightW, listH, TRUE);
                
                int clueY = top + 30 + listH + 20;
                MoveWindow(hClueTitle, rightX, clueY, rightW, 24, TRUE);
                MoveWindow(hListClues, rightX, clueY + 30, rightW, cy - (clueY + 30) - pad, TRUE);
            }
            break;
        }

        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            if (id == ID_BTN_START) {
                StartGame();
                RECT r;
                GetClientRect(hwnd, &r);
                SendMessageA(hwnd, WM_SIZE, 0, MAKELPARAM(r.right, r.bottom));
            } else if (id == ID_BTN_SEARCH) {
                SearchLocation(hwnd);
            } else if (id == ID_BTN_TRAVEL_OFFICE) {
                Travel(0);
            } else if (id == ID_BTN_TRAVEL_MANOR) {
                Travel(1);
            } else if (id == ID_BTN_TRAVEL_DOCKS) {
                Travel(2);
            }
            break;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(26, 26, 26));
            SetTextColor(hdc, RGB(208, 208, 208));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
        }
        
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            HBRUSH hBrush = CreateSolidBrush(RGB(26, 26, 26));
            FillRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
            return 1;
        }

        case WM_DESTROY:
            if (hFont) DeleteObject(hFont);
            if (hFontBold) DeleteObject(hFontBold);
            if (hFontTitle) DeleteObject(hFontTitle);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void __stdcall MainEntry() {
    HMODULE hUser32 = LoadLibraryA("user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *SETPROCESSDPIAWARE)();
        SETPROCESSDPIAWARE pSetProcessDPIAware = (SETPROCESSDPIAWARE)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (pSetProcessDPIAware) pSetProcessDPIAware();
    }

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "KMysteryClass";
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0, "KMysteryClass", "KMystery",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, wc.hInstance, NULL
    );

    if (hwnd) {
        ShowWindow(hwnd, SW_SHOWDEFAULT);
        MSG msg;
        while (GetMessageA(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    ExitProcess(0);
}
