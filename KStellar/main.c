#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define ID_BTN_SET_COURSE 101

typedef struct {
    int id;
    char name[32];
    int x;
    int y;
    char desc[256];
} System;

System systems[] = {
    {1, "Sol", 50, 50, "Cradle of humanity. Highly developed planetary system with a stable economy."},
    {2, "Alpha Centauri", 30, 70, "Primary mining colony. Resource rich, but known for pirate activity on the outskirts."},
    {3, "Sirius", 75, 35, "Major trading hub. High tech goods are cheap here."},
    {4, "Proxima", 20, 80, "Remote outpost. Dangerous but offers high rewards for smugglers."},
    {5, "Vega", 80, 80, "Agricultural world. Supplies food to neighboring industrial systems."}
};

HWND hMapArea, hInfoArea, hBtnCourse, hFuelText, hCreditsText;
int selectedSystem = -1;
int currentSystemId = 0;
int fuel = 100;
int credits = 1000;
static HFONT hFont = NULL;
static HBRUSH hBgBrush = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            hFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
            hBgBrush = CreateSolidBrush(RGB(5, 5, 15));
            
            HWND h1 = CreateWindow("STATIC", "FUEL: 100%", WS_VISIBLE | WS_CHILD, 20, 20, 100, 20, hwnd, NULL, NULL, NULL);
            hFuelText = h1;
            SendMessage(h1, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND h2 = CreateWindow("STATIC", "CREDITS: 1,000", WS_VISIBLE | WS_CHILD, 140, 20, 150, 20, hwnd, NULL, NULL, NULL);
            hCreditsText = h2;
            SendMessage(h2, WM_SETFONT, (WPARAM)hFont, TRUE);
            HWND h3 = CreateWindow("STATIC", "CARGO: 0/50 TONS", WS_VISIBLE | WS_CHILD, 300, 20, 150, 20, hwnd, NULL, NULL, NULL);
            SendMessage(h3, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND h4 = CreateWindow("STATIC", "LOCAL SYSTEMS", WS_VISIBLE | WS_CHILD, 440, 60, 150, 20, hwnd, NULL, NULL, NULL);
            SendMessage(h4, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hInfoArea = CreateWindow("STATIC", "Select a system on the map for details.", WS_VISIBLE | WS_CHILD, 440, 90, 200, 150, hwnd, NULL, NULL, NULL);
            SendMessage(hInfoArea, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            hBtnCourse = CreateWindow("BUTTON", "Set Course", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 440, 250, 150, 30, hwnd, (HMENU)ID_BTN_SET_COURSE, NULL, NULL);
            ShowWindow(hBtnCourse, SW_HIDE);
            return 0;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(0, 255, 204));
            SetBkColor(hdcStatic, RGB(5, 5, 15));
            return (INT_PTR)hBgBrush;
        }
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
            if (pdis->CtlID == ID_BTN_SET_COURSE) {
                HDC hdc = pdis->hDC;
                RECT rect = pdis->rcItem;
                int state = pdis->itemState;
                
                if (state & ODS_SELECTED) {
                    FillRect(hdc, &rect, CreateSolidBrush(RGB(0, 255, 204)));
                    SetTextColor(hdc, RGB(0, 0, 0));
                    SetBkColor(hdc, RGB(0, 255, 204));
                } else {
                    FillRect(hdc, &rect, CreateSolidBrush(RGB(0, 0, 0)));
                    HBRUSH hBorder = CreateSolidBrush(RGB(0, 255, 204));
                    FrameRect(hdc, &rect, hBorder);
                    DeleteObject(hBorder);
                    SetTextColor(hdc, RGB(0, 255, 204));
                    SetBkColor(hdc, RGB(0, 0, 0));
                }
                char text[32];
                GetWindowText(pdis->hwndItem, text, sizeof(text));
                SelectObject(hdc, hFont);
                DrawText(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }
            return FALSE;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            HBRUSH hMapBg = CreateSolidBrush(RGB(2, 10, 16));
            SelectObject(hdc, hMapBg);
            HPEN hBorderPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 204));
            SelectObject(hdc, hBorderPen);
            Rectangle(hdc, 20, 60, 420, 460);
            
            for(int y = 62; y < 458; y += 4) {
                HPEN hScanLine = CreatePen(PS_SOLID, 1, RGB(0, 50, 50));
                SelectObject(hdc, hScanLine);
                MoveToEx(hdc, 22, y, NULL);
                LineTo(hdc, 418, y);
                DeleteObject(hScanLine);
            }
            
            DeleteObject(hMapBg);
            DeleteObject(hBorderPen);
            
            HBRUSH hGreenBrush = CreateSolidBrush(RGB(0, 255, 0));
            HBRUSH hCyanBrush = CreateSolidBrush(RGB(0, 255, 255));
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 255, 204));
            SelectObject(hdc, hFont);
            
            for (int i = 0; i < 5; i++) {
                int px = 20 + (systems[i].x * 400 / 100);
                int py = 60 + (systems[i].y * 400 / 100);
                
                if (i == selectedSystem) {
                    SelectObject(hdc, hCyanBrush);
                    SelectObject(hdc, GetStockObject(WHITE_PEN));
                } else if (i == currentSystemId) {
                    SelectObject(hdc, hCyanBrush);
                    SelectObject(hdc, GetStockObject(NULL_PEN));
                } else {
                    SelectObject(hdc, hGreenBrush);
                    SelectObject(hdc, GetStockObject(NULL_PEN));
                }
                
                int r = (i == selectedSystem || i == currentSystemId) ? 8 : 6;
                Ellipse(hdc, px - r, py - r, px + r, py + r);
                
                char nameBuf[64];
                if (i == currentSystemId) {
                    sprintf(nameBuf, "%s (HERE)", systems[i].name);
                } else {
                    strcpy(nameBuf, systems[i].name);
                }
                
                if (i == currentSystemId) {
                    SetTextColor(hdc, RGB(0, 255, 255));
                } else {
                    SetTextColor(hdc, RGB(0, 255, 204));
                }
                TextOut(hdc, px + 12, py - 8, nameBuf, strlen(nameBuf));
            }
            
            DeleteObject(hGreenBrush);
            DeleteObject(hCyanBrush);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);
            
            for (int i = 0; i < 5; i++) {
                int px = 20 + (systems[i].x * 400 / 100);
                int py = 60 + (systems[i].y * 400 / 100);
                
                if (abs(mx - px) < 15 && abs(my - py) < 15) {
                    selectedSystem = i;
                    
                    char infoText[512];
                    sprintf(infoText, "%s\n\n%s\n\nCoordinates: X:%d Y:%d", 
                        systems[i].name, systems[i].desc, systems[i].x, systems[i].y);
                        
                    SetWindowText(hInfoArea, infoText);
                    if (selectedSystem != currentSystemId) {
                        ShowWindow(hBtnCourse, SW_SHOW);
                    } else {
                        ShowWindow(hBtnCourse, SW_HIDE);
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                }
            }
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_SET_COURSE && selectedSystem != -1) {
                int dx = systems[selectedSystem].x - systems[currentSystemId].x;
                int dy = systems[selectedSystem].y - systems[currentSystemId].y;
                int dist = (int)sqrt((double)(dx * dx + dy * dy));
                int fuelCost = dist / 2;
                
                if (fuel < fuelCost) {
                    char msg[128];
                    sprintf(msg, "Not enough fuel! You need %d%% fuel.", fuelCost);
                    MessageBox(hwnd, msg, "Warning", MB_OK | MB_ICONWARNING);
                } else {
                    fuel -= fuelCost;
                    currentSystemId = selectedSystem;
                    
                    char fuelStr[32];
                    sprintf(fuelStr, "FUEL: %d%%", fuel);
                    SetWindowText(hFuelText, fuelStr);
                    
                    char resultMsg[512];
                    sprintf(resultMsg, "Traveled to %s.\nFuel consumed: %d%%.", systems[currentSystemId].name, fuelCost);
                    
                    int randomEncounter = rand() % 100;
                    if (randomEncounter < 30) {
                        strcat(resultMsg, "\n\nAlert: Hostile pirate vessel encountered!");
                        int stolen = (rand() % 50) + 10;
                        if (credits >= stolen) {
                            credits -= stolen;
                            char stealMsg[64];
                            sprintf(stealMsg, "\nThey extorted %d credits.", stolen);
                            strcat(resultMsg, stealMsg);
                        } else {
                            strcat(resultMsg, "\nThey found you have no money and left.");
                        }
                    } else if (randomEncounter < 50) {
                        strcat(resultMsg, "\n\nNotice: You found drifting debris.");
                        int found = (rand() % 100) + 20;
                        credits += found;
                        char foundMsg[64];
                        sprintf(foundMsg, "\nSalvaged %d credits.", found);
                        strcat(resultMsg, foundMsg);
                    } else {
                        strcat(resultMsg, "\n\nThe journey was uneventful.");
                    }
                    
                    char credStr[32];
                    sprintf(credStr, "CREDITS: %d", credits);
                    SetWindowText(hCreditsText, credStr);
                    
                    MessageBox(hwnd, resultMsg, "Navigation Log", MB_OK);
                    
                    SetWindowText(hInfoArea, "Select a system on the map for details.");
                    ShowWindow(hBtnCourse, SW_HIDE);
                    selectedSystem = -1;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return 0;
        }
        case WM_DESTROY: {
            if (hFont) DeleteObject(hFont);
            if (hBgBrush) DeleteObject(hBgBrush);
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KStellar Window Class";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(5, 5, 15));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "KStellar Phase 3",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 680, 520,
        NULL,
        NULL,
        hInstance,
        NULL
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
