#include <windows.h>

#define BTN_DRAW 101
#define BTN_RESET 102

typedef struct {
    char name[32];
    int cost;
} CardDef;

CardDef sampleCards[] = {
    {"Fireball", 3},
    {"Ice Shard", 2},
    {"Arcane Missiles", 1},
    {"Healing Touch", 2},
    {"Lightning Strike", 4}
};
#define NUM_SAMPLE_CARDS (sizeof(sampleCards)/sizeof(CardDef))

int playerHand[7];
int opponentHand[7];
int playerCount = 0;
int opponentCount = 0;

HWND hwndDraw, hwndReset;

unsigned int seed = 0;
int my_rand() {
    seed = seed * 1664525 + 1013904223;
    return (seed >> 16) & 0x7FFF;
}

void DrawCard(int isOpponent) {
    if (isOpponent) {
        if (opponentCount < 7) {
            opponentHand[opponentCount++] = my_rand() % NUM_SAMPLE_CARDS;
        }
    } else {
        if (playerCount < 7) {
            playerHand[playerCount++] = my_rand() % NUM_SAMPLE_CARDS;
        }
    }
}

void ResetGame() {
    playerCount = 0;
    opponentCount = 0;
    for (int i = 0; i < 3; i++) {
        DrawCard(0);
        DrawCard(1);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hwndDraw = CreateWindow("BUTTON", "Draw Card",
                                    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                    300, 10, 100, 30,
                                    hwnd, (HMENU)BTN_DRAW, NULL, NULL);
            hwndReset = CreateWindow("BUTTON", "Reset Game",
                                     WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                     410, 10, 100, 30,
                                     hwnd, (HMENU)BTN_RESET, NULL, NULL);
            seed = GetTickCount();
            ResetGame();
            return 0;

        case WM_COMMAND:
            if (LOWORD(wParam) == BTN_DRAW) {
                DrawCard(0);
                DrawCard(1);
                InvalidateRect(hwnd, NULL, TRUE);
            } else if (LOWORD(wParam) == BTN_RESET) {
                ResetGame();
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;

        case WM_LBUTTONDOWN: {
            int xPos = (short)LOWORD(lParam);
            int yPos = (short)HIWORD(lParam);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int cw = clientRect.right - clientRect.left;
            int ch = clientRect.bottom - clientRect.top;
            
            int cardW = 100;
            int cardH = 140;
            int gap = 10;
            
            int playerW = playerCount * cardW + (playerCount > 0 ? playerCount - 1 : 0) * gap;
            int playerX = (cw - playerW) / 2;
            int playerY = ch - cardH - 20;

            for (int i = 0; i < playerCount; i++) {
                int cx = playerX + i * (cardW + gap);
                if (xPos >= cx && xPos <= cx + cardW && yPos >= playerY && yPos <= playerY + cardH) {
                    for (int j = i; j < playerCount - 1; j++) {
                        playerHand[j] = playerHand[j + 1];
                    }
                    playerCount--;
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                }
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int cw = clientRect.right - clientRect.left;
            int ch = clientRect.bottom - clientRect.top;

            HBRUSH bgBrush = CreateSolidBrush(RGB(26, 26, 26));
            FillRect(hdc, &clientRect, bgBrush);
            DeleteObject(bgBrush);

            RECT arenaRect = {20, 220, cw - 20, ch - 220};
            HBRUSH arenaBrush = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(hdc, &arenaRect, arenaBrush);
            DeleteObject(arenaBrush);
            
            HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(51, 51, 51));
            HGDIOBJ oldPen = SelectObject(hdc, borderPen);
            HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HGDIOBJ oldBrush = SelectObject(hdc, nullBrush);
            Rectangle(hdc, arenaRect.left, arenaRect.top, arenaRect.right, arenaRect.bottom);
            SelectObject(hdc, oldBrush);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(102, 102, 102));
            const char* arenaText = "Spells and effects go here";
            DrawText(hdc, arenaText, -1, &arenaRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            int cardW = 100;
            int cardH = 140;
            int gap = 10;
            
            HPEN cardBorderPen = CreatePen(PS_SOLID, 2, RGB(85, 85, 85));
            SelectObject(hdc, cardBorderPen);

            int oppW = opponentCount * cardW + (opponentCount > 0 ? opponentCount - 1 : 0) * gap;
            int oppX = (cw - oppW) / 2;
            int oppY = 60;
            HBRUSH oppBrush = CreateSolidBrush(RGB(58, 26, 26));

            for (int i = 0; i < opponentCount; i++) {
                int cx = oppX + i * (cardW + gap);
                SelectObject(hdc, oppBrush);
                Rectangle(hdc, cx, oppY, cx + cardW, oppY + cardH);
                
                SetTextColor(hdc, RGB(212, 212, 212));
                RECT textRect = {cx, oppY, cx + cardW, oppY + cardH};
                DrawText(hdc, "Card", -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            DeleteObject(oppBrush);

            int playerW = playerCount * cardW + (playerCount > 0 ? playerCount - 1 : 0) * gap;
            int playerX = (cw - playerW) / 2;
            int playerY = ch - cardH - 20;
            HBRUSH playerBrush = CreateSolidBrush(RGB(42, 42, 42));

            for (int i = 0; i < playerCount; i++) {
                int cx = playerX + i * (cardW + gap);
                SelectObject(hdc, playerBrush);
                Rectangle(hdc, cx, playerY, cx + cardW, playerY + cardH);
                
                CardDef cd = sampleCards[playerHand[i]];
                
                SetTextColor(hdc, RGB(212, 212, 212));
                RECT nameRect = {cx, playerY + 50, cx + cardW, playerY + 70};
                DrawText(hdc, cd.name, -1, &nameRect, DT_CENTER | DT_SINGLELINE);
                
                SetTextColor(hdc, RGB(77, 166, 255));
                char costStr[32];
                wsprintf(costStr, "Mana: %d", cd.cost);
                RECT costRect = {cx, playerY + 80, cx + cardW, playerY + 100};
                DrawText(hdc, costStr, -1, &costRect, DT_CENTER | DT_SINGLELINE);
            }
            DeleteObject(playerBrush);
            
            SelectObject(hdc, oldPen);
            DeleteObject(borderPen);
            DeleteObject(cardBorderPen);

            SetTextColor(hdc, RGB(212, 212, 212));
            RECT lblOpp = {0, oppY - 20, cw, oppY};
            DrawText(hdc, "Opponent Hand", -1, &lblOpp, DT_CENTER | DT_SINGLELINE);
            
            RECT lblPlayer = {0, playerY - 20, cw, playerY};
            DrawText(hdc, "Player Hand", -1, &lblPlayer, DT_CENTER | DT_SINGLELINE);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "KWizardClass";

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
        "KWizard",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
