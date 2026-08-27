#include <windows.h>
#include <stdio.h>
#include <string.h>

#define ID_MARKET_LIST 101
#define ID_OWNED_LIST 102
#define ID_BUY_BUTTON 103

typedef struct {
    int id;
    char name[32];
    int cost;
    char desc[64];
} Gladiator;

Gladiator market[3] = {
    {1, "Titus", 300, "A strong but slow fighter."},
    {2, "Flamma", 500, "Fierce veteran with many scars."},
    {3, "Spiculus", 700, "Highly agile, crowd favorite."}
};
int market_count = 3;

Gladiator owned[10];
int owned_count = 0;

int funds = 1000;

HWND hFundsLabel, hMarketList, hOwnedList, hBuyButton;

void UpdateUI() {
    char buf[128];
    sprintf(buf, "Treasury: %d Denarii", funds);
    SetWindowText(hFundsLabel, buf);

    SendMessage(hMarketList, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < market_count; i++) {
        sprintf(buf, "%s - %dD (%s)", market[i].name, market[i].cost, market[i].desc);
        SendMessage(hMarketList, LB_ADDSTRING, 0, (LPARAM)buf);
    }

    SendMessage(hOwnedList, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < owned_count; i++) {
        sprintf(buf, "%s - %s", owned[i].name, owned[i].desc);
        SendMessage(hOwnedList, LB_ADDSTRING, 0, (LPARAM)buf);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            
            HWND hLbl1 = CreateWindow("STATIC", "Available Recruits", WS_VISIBLE | WS_CHILD, 20, 50, 300, 20, hwnd, NULL, NULL, NULL);
            hMarketList = CreateWindow("LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY, 20, 70, 350, 150, hwnd, (HMENU)ID_MARKET_LIST, NULL, NULL);
            
            HWND hLbl2 = CreateWindow("STATIC", "Your Gladiators", WS_VISIBLE | WS_CHILD, 400, 50, 300, 20, hwnd, NULL, NULL, NULL);
            hOwnedList = CreateWindow("LISTBOX", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER, 400, 70, 350, 150, hwnd, (HMENU)ID_OWNED_LIST, NULL, NULL);

            hFundsLabel = CreateWindow("STATIC", "Treasury: 1000 Denarii", WS_VISIBLE | WS_CHILD, 20, 20, 300, 20, hwnd, NULL, NULL, NULL);
            hBuyButton = CreateWindow("BUTTON", "Buy Selected", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 20, 230, 150, 30, hwnd, (HMENU)ID_BUY_BUTTON, NULL, NULL);
            
            SendMessage(hLbl1, WM_SETFONT, (WPARAM)hFont, FALSE);
            SendMessage(hMarketList, WM_SETFONT, (WPARAM)hFont, FALSE);
            SendMessage(hLbl2, WM_SETFONT, (WPARAM)hFont, FALSE);
            SendMessage(hOwnedList, WM_SETFONT, (WPARAM)hFont, FALSE);
            SendMessage(hFundsLabel, WM_SETFONT, (WPARAM)hFont, FALSE);
            SendMessage(hBuyButton, WM_SETFONT, (WPARAM)hFont, FALSE);

            UpdateUI();
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BUY_BUTTON) {
                int sel = SendMessage(hMarketList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR && sel >= 0 && sel < market_count) {
                    if (funds >= market[sel].cost) {
                        funds -= market[sel].cost;
                        owned[owned_count++] = market[sel];
                        
                        for (int i = sel; i < market_count - 1; i++) {
                            market[i] = market[i + 1];
                        }
                        market_count--;
                        UpdateUI();
                    } else {
                        MessageBox(hwnd, "Not enough funds!", "Error", MB_OK | MB_ICONERROR);
                    }
                }
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KColosseumClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "KColosseum - Ludus Management",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 350,
        NULL, NULL, hInstance, NULL
    );
    
    if (hwnd == NULL) return 0;
    
    ShowWindow(hwnd, nCmdShow);
    
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}
