#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#define ID_BTN_PREV 101
#define ID_BTN_NEXT 102
#define ID_BTN_FLIP 103
#define ID_BTN_ADD  104
#define ID_LBL_CARD 105
#define ID_LBL_INFO 106

// Dialog Add Card IDs
#define IDD_ADDCARD 200
#define IDC_FRONT   201
#define IDC_BACK    202
#define IDC_ADD     203
#define IDC_CANCEL  204

typedef struct {
    char front[256];
    char back[256];
} FlashCard;

FlashCard deck[100];
int deckCount = 0;
int currentIndex = 0;
int isFlipped = 0;

HWND hLblCard, hLblInfo, hBtnPrev, hBtnNext, hBtnFlip, hBtnAdd;
HBRUSH hbgBrush, hCardBrush;
HFONT hFont, hCardFont;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddCardProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void UpdateCardDisplay() {
    if (deckCount == 0) {
        SetWindowText(hLblCard, "Deck is empty\n\nClick 'Add Card' to begin.");
        SetWindowText(hLblInfo, "0/0");
        EnableWindow(hBtnPrev, FALSE);
        EnableWindow(hBtnNext, FALSE);
        EnableWindow(hBtnFlip, FALSE);
        return;
    }

    EnableWindow(hBtnFlip, TRUE);
    EnableWindow(hBtnPrev, currentIndex > 0);
    EnableWindow(hBtnNext, currentIndex < deckCount - 1);

    char info[32];
    sprintf(info, "Card %d/%d", currentIndex + 1, deckCount);
    SetWindowText(hLblInfo, info);

    if (isFlipped) {
        SetWindowText(hLblCard, deck[currentIndex].back);
    } else {
        SetWindowText(hLblCard, deck[currentIndex].front);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KFlashWindowClass";
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(13, 17, 23));

    RegisterClass(&wc);

    hbgBrush = CreateSolidBrush(RGB(13, 17, 23));
    hCardBrush = CreateSolidBrush(RGB(22, 27, 34));

    // Initial deck
    strcpy(deck[0].front, "What is KiloOS?");
    strcpy(deck[0].back, "An advanced, multi-agent operating system environment.");
    strcpy(deck[1].front, "What does the 'K' stand for?");
    strcpy(deck[1].back, "Kilo!");
    deckCount = 2;

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "KFlash",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 450,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateCardDisplay();

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(hbgBrush);
    DeleteObject(hCardBrush);
    if(hFont) DeleteObject(hFont);
    if(hCardFont) DeleteObject(hCardFont);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                               DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            hCardFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                               DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            hBtnAdd = CreateWindow("BUTTON", "+ Add Card", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   460, 10, 100, 30, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL);

            hLblInfo = CreateWindow("STATIC", "1/1", WS_CHILD | WS_VISIBLE | SS_LEFT,
                                    20, 15, 100, 20, hwnd, (HMENU)ID_LBL_INFO, NULL, NULL);

            hLblCard = CreateWindow("STATIC", "Card Content", WS_CHILD | WS_VISIBLE | SS_CENTER,
                                    50, 60, 480, 240, hwnd, (HMENU)ID_LBL_CARD, NULL, NULL);

            hBtnPrev = CreateWindow("BUTTON", "<- Prev", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    150, 320, 80, 30, hwnd, (HMENU)ID_BTN_PREV, NULL, NULL);

            hBtnFlip = CreateWindow("BUTTON", "Flip", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    250, 320, 80, 30, hwnd, (HMENU)ID_BTN_FLIP, NULL, NULL);

            hBtnNext = CreateWindow("BUTTON", "Next ->", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    350, 320, 80, 30, hwnd, (HMENU)ID_BTN_NEXT, NULL, NULL);

            SendMessage(hBtnAdd, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblInfo, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hLblCard, WM_SETFONT, (WPARAM)hCardFont, TRUE);
            SendMessage(hBtnPrev, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnFlip, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBtnNext, WM_SETFONT, (WPARAM)hFont, TRUE);

            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            HWND hStatic = (HWND)lParam;
            if (hStatic == hLblCard) {
                SetTextColor(hdcStatic, RGB(201, 209, 217));
                SetBkColor(hdcStatic, RGB(22, 27, 34));
                return (INT_PTR)hCardBrush;
            } else {
                SetTextColor(hdcStatic, RGB(139, 148, 158));
                SetBkColor(hdcStatic, RGB(13, 17, 23));
                return (INT_PTR)hbgBrush;
            }
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case ID_BTN_PREV:
                    if (currentIndex > 0) {
                        currentIndex--;
                        isFlipped = 0;
                        UpdateCardDisplay();
                    }
                    break;
                case ID_BTN_NEXT:
                    if (currentIndex < deckCount - 1) {
                        currentIndex++;
                        isFlipped = 0;
                        UpdateCardDisplay();
                    }
                    break;
                case ID_BTN_FLIP:
                    if (deckCount > 0) {
                        isFlipped = !isFlipped;
                        UpdateCardDisplay();
                    }
                    break;
                case ID_BTN_ADD:
                    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ADDCARD), hwnd, AddCardProc);
                    UpdateCardDisplay();
                    break;
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK AddCardProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_ADD) {
                if (deckCount < 100) {
                    GetDlgItemText(hwndDlg, IDC_FRONT, deck[deckCount].front, 255);
                    GetDlgItemText(hwndDlg, IDC_BACK, deck[deckCount].back, 255);
                    if (strlen(deck[deckCount].front) > 0 && strlen(deck[deckCount].back) > 0) {
                        deckCount++;
                    }
                }
                EndDialog(hwndDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            else if (LOWORD(wParam) == IDC_CANCEL || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}
