#include <windows.h>
#include <stdio.h>
#include <string.h>

#define ID_BTN_DEST1 101
#define ID_BTN_DEST2 102
#define ID_LIST_LOG  103

typedef struct {
    int credits;
    int fuel;
    int maxFuel;
    int cargo;
    int maxCargo;
    int location; // 0 = Earth, 1 = Mars, 2 = Venus
} GameState;

GameState state = { 1000, 100, 100, 0, 20, 0 };

const char* planetNames[] = { "Earth", "Mars", "Venus" };
int distances[3][3] = {
    {0, 20, 15}, // Earth
    {20, 0, 25}, // Mars
    {15, 25, 0}  // Venus
};

HWND hStatCredits, hStatFuel, hStatCargo;
HWND hStatLoc;
HWND hBtnDest1, hBtnDest2;
HWND hListLog;

int destTarget[2];
int destCost[2];

void LogMessage(const char* msg) {
    SendMessage(hListLog, LB_ADDSTRING, 0, (LPARAM)msg);
    int count = SendMessage(hListLog, LB_GETCOUNT, 0, 0);
    SendMessage(hListLog, LB_SETTOPINDEX, count - 1, 0);
}

void UpdateUI(HWND hwnd) {
    char buf[128];
    wsprintf(buf, "Credits: %d", state.credits);
    SetWindowText(hStatCredits, buf);
    wsprintf(buf, "Fuel: %d / %d", state.fuel, state.maxFuel);
    SetWindowText(hStatFuel, buf);
    wsprintf(buf, "Cargo: %d / %d", state.cargo, state.maxCargo);
    SetWindowText(hStatCargo, buf);

    wsprintf(buf, "Current Location: %s", planetNames[state.location]);
    SetWindowText(hStatLoc, buf);

    // Update buttons
    int btnIdx = 0;
    for (int i = 0; i < 3; i++) {
        if (i == state.location) continue;
        destTarget[btnIdx] = i;
        destCost[btnIdx] = distances[state.location][i];
        
        wsprintf(buf, "Travel to %s (%d fuel)", planetNames[i], destCost[btnIdx]);
        SetWindowText(btnIdx == 0 ? hBtnDest1 : hBtnDest2, buf);
        EnableWindow(btnIdx == 0 ? hBtnDest1 : hBtnDest2, state.fuel >= destCost[btnIdx]);
        
        btnIdx++;
    }
}

void Travel(int btnIdx, HWND hwnd) {
    int cost = destCost[btnIdx];
    int target = destTarget[btnIdx];
    
    if (state.fuel >= cost) {
        state.fuel -= cost;
        state.location = target;
        char buf[256];
        wsprintf(buf, "> Hyperspace jump complete. Arrived at %s. Used %d fuel.", planetNames[target], cost);
        LogMessage(buf);
        UpdateUI(hwnd);
    } else {
        LogMessage("> Insufficient fuel!");
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, 
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
            
            CreateWindow("STATIC", "KTrader Space Trading Sim", WS_CHILD | WS_VISIBLE,
                20, 20, 300, 20, hwnd, NULL, NULL, NULL);

            hStatCredits = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 20, 60, 150, 20, hwnd, NULL, NULL, NULL);
            hStatFuel = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 180, 60, 150, 20, hwnd, NULL, NULL, NULL);
            hStatCargo = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 340, 60, 150, 20, hwnd, NULL, NULL, NULL);

            CreateWindow("STATIC", "Navigation", WS_CHILD | WS_VISIBLE, 20, 100, 100, 20, hwnd, NULL, NULL, NULL);
            hStatLoc = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE, 20, 130, 200, 20, hwnd, NULL, NULL, NULL);

            hBtnDest1 = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 160, 250, 30, hwnd, (HMENU)ID_BTN_DEST1, NULL, NULL);
            hBtnDest2 = CreateWindow("BUTTON", "", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 200, 250, 30, hwnd, (HMENU)ID_BTN_DEST2, NULL, NULL);

            hListLog = CreateWindow("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
                20, 250, 440, 100, hwnd, (HMENU)ID_LIST_LOG, NULL, NULL);

            // Set fonts
            SendDlgItemMessage(hwnd, ID_BTN_DEST1, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendDlgItemMessage(hwnd, ID_BTN_DEST2, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hListLog, WM_SETFONT, (WPARAM)hFont, TRUE);

            LogMessage("> Welcome to KTrader, Captain.");
            UpdateUI(hwnd);
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_DEST1) {
                Travel(0, hwnd);
            } else if (LOWORD(wParam) == ID_BTN_DEST2) {
                Travel(1, hwnd);
            }
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void __stdcall MainEntry() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    const char CLASS_NAME[] = "KTraderClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "KTrader",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 420,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd != NULL) {
        ShowWindow(hwnd, SW_SHOWDEFAULT);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    ExitProcess(0);
}
