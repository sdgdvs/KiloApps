#include <windows.h>
#include <stdio.h>

#define ID_BUY_BUTTON 101
#define ID_MARKET_LIST 102
#define ID_OWNED_LIST 103
#define ID_FUNDS_LABEL 104

typedef struct {
    int id;
    char name[32];
    int cost;
    char desc[64];
} Gladiator;

Gladiator market[] = {
    {1, "Titus", 300, "A strong but slow fighter."},
    {2, "Flamma", 500, "Fierce veteran with many scars."},
    {3, "Spiculus", 700, "Highly agile, crowd favorite."}
};
int market_count = 3;

Gladiator owned[10];
int owned_count = 0;

int funds = 1000;

HWND hMarketList, hOwnedList, hFundsLabel, hBuyButton;

void UpdateUI() {
    char buf[256];
    wsprintfA(buf, "Treasury: %d Denarii", funds);
    SetWindowTextA(hFundsLabel, buf);

    SendMessageA(hMarketList, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < market_count; i++) {
        wsprintfA(buf, "%s - %s (%dD)", market[i].name, market[i].desc, market[i].cost);
        SendMessageA(hMarketList, LB_ADDSTRING, 0, (LPARAM)buf);
    }

    SendMessageA(hOwnedList, LB_RESETCONTENT, 0, 0);
    if (owned_count == 0) {
        SendMessageA(hOwnedList, LB_ADDSTRING, 0, (LPARAM)"No gladiators owned.");
    } else {
        for (int i = 0; i < owned_count; i++) {
            wsprintfA(buf, "%s - %s", owned[i].name, owned[i].desc);
            SendMessageA(hOwnedList, LB_ADDSTRING, 0, (LPARAM)buf);
        }
    }
}

void BuyGladiator(int index) {
    if (index >= 0 && index < market_count && funds >= market[index].cost) {
        funds -= market[index].cost;
        owned[owned_count++] = market[index];
        for (int i = index; i < market_count - 1; i++) {
            market[i] = market[i + 1];
        }
        market_count--;
        UpdateUI();
    } else if (index >= 0 && index < market_count) {
        MessageBoxA(NULL, "Not enough funds!", "Error", MB_OK | MB_ICONWARNING);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            CreateWindowA("STATIC", "KColosseum - Ludus Management", WS_VISIBLE | WS_CHILD | SS_CENTER,
                          10, 10, 560, 20, hwnd, NULL, NULL, NULL);

            hFundsLabel = CreateWindowA("STATIC", "Treasury: 1000 Denarii", WS_VISIBLE | WS_CHILD | SS_CENTER,
                          10, 40, 560, 20, hwnd, (HMENU)ID_FUNDS_LABEL, NULL, NULL);

            CreateWindowA("STATIC", "Available Recruits", WS_VISIBLE | WS_CHILD,
                          10, 70, 270, 20, hwnd, NULL, NULL, NULL);

            hMarketList = CreateWindowA("LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY,
                          10, 90, 270, 200, hwnd, (HMENU)ID_MARKET_LIST, NULL, NULL);

            hBuyButton = CreateWindowA("BUTTON", "Buy Selected", WS_VISIBLE | WS_CHILD,
                          10, 300, 270, 30, hwnd, (HMENU)ID_BUY_BUTTON, NULL, NULL);

            CreateWindowA("STATIC", "Your Gladiators", WS_VISIBLE | WS_CHILD,
                          290, 70, 270, 20, hwnd, NULL, NULL, NULL);

            hOwnedList = CreateWindowA("LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER,
                          290, 90, 270, 200, hwnd, (HMENU)ID_OWNED_LIST, NULL, NULL);

            UpdateUI();
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BUY_BUTTON) {
                int sel = SendMessageA(hMarketList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    BuyGladiator(sel);
                } else {
                    MessageBoxA(hwnd, "Select a gladiator to buy.", "Info", MB_OK | MB_ICONINFORMATION);
                }
            }
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void MainEntry() {
    HINSTANCE hInstance = GetModuleHandleA(NULL);
    const char CLASS_NAME[] = "KColosseumClass";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0, CLASS_NAME, "KColosseum", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd) {
        ShowWindow(hwnd, SW_SHOW);
        MSG msg = {0};
        while (GetMessageA(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    ExitProcess(0);
}
